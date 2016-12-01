#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "messages.h"
#include "whisperer.h"

static const char exec_fail[] = "Error: execv failed!\n";
static int to_child = -1;
static int from_child = -1;
static message_t message = NULL;


static enum {NOP, WHISPER, STOP} request;
static pthread_t whisperer_thread_id;
static pthread_mutex_t whisper_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t whisper_cond = PTHREAD_COND_INITIALIZER;
static bool done;
static char *target = NULL;
static bool initialized = false; 

static void *whisperer_thread(void *param);

static void whisperer_cleanup(void)
{
  if(to_child != -1){
    close(to_child);
    to_child = -1;
  }
  if(from_child != -1){
    close(from_child);
    from_child = -1;
  }
  if(message != NULL){
    dispose_message(&message);
  }
}

static bool whisperer_wait(void);

bool whisperer_init(const char *prog)
{
  ssize_t res;
  //Alloc new message
  if(!new_message(&message)){
    return false;
  }
  
  target = (char *)malloc(strlen(prog) + 1);
  if(target == NULL){
    return false;
  }
  strcpy(target, prog);
  
  //Open pipes and start child connected to them
  int pipefd_to_child[2];
  int pipefd_to_parent[2];
  pid_t pid;
  char *argv[] = {NULL, NULL};
  argv[0] = target;
  //Create two pipes - first will be parent's output and child's input,
  //  the second one will be the other way around 
  if(pipe(pipefd_to_child) != 0){
    printf("Problem creating pipe!\n");
    return false;
  }
  if(pipe(pipefd_to_parent ) != 0){
    close(pipefd_to_child[0]);
    close(pipefd_to_child[1]);
    printf("Problem creating pipe!\n");
    return false;
  }
  pid = fork();
  switch(pid){
    case -1:
      perror("Problem forking!\n");
      return false;
      break;
    case 0:
      //Child - close unneeded pipe ends and connect
      //  the rest to stdin/out
      printf("Hello World from child!\n");
      close(pipefd_to_child[1]);//child - close write end
      close(pipefd_to_parent[0]);
      dup2(pipefd_to_child[0], STDIN_FILENO);
      dup2(pipefd_to_parent[1], STDOUT_FILENO);
      
      execv(argv[0], argv);
      //fallthrough in case execv fails.
      res = write(STDERR_FILENO, exec_fail, sizeof(exec_fail));
      (void) res; //can't do much about it...
      exit(0);
      break;
    default:
      close(pipefd_to_child[0]);//parent - close read end
      close(pipefd_to_parent[1]);
      to_child = pipefd_to_child[1];
      from_child = pipefd_to_parent[0];
      break;
  }
  printf("Child process pid: %d\n", pid);
  //wait for the child to acknowledge its start.
  if(!whisperer_wait()){
    close(pipefd_to_child[0]);
    close(pipefd_to_child[1]);
    int status;
    pid_t wres = waitpid(pid, &status, 0);
    if(wres < 0){
      perror("waitpid");
    }
    printf("Child not responding! %d \n", wres);
    return false;
  }
  //Start the communicator thread
  request = NOP;
  pthread_create(&whisperer_thread_id, NULL, whisperer_thread, NULL);
  initialized = true;
  return true;
}

//Wait for a child to acknowldge
static bool whisperer_wait(void)
{
  char buf[1024];
  struct pollfd pipe_poll;
  int fds;
  ssize_t res;
  
  bzero(buf, sizeof(buf));
  pipe_poll.fd = from_child;
  pipe_poll.events = POLLIN;
  pipe_poll.revents = 0;
  while(1){
    //check if we shouldn't close
    pthread_mutex_lock(&whisper_mutex);
    if(request == STOP){
      pthread_mutex_unlock(&whisper_mutex);
      break;
    }
    pthread_mutex_unlock(&whisper_mutex);
    //wait for a movement 
    pipe_poll.events = POLLIN;
    fds = poll(&pipe_poll, 1, 1000);
    if(fds > 0){
      if(pipe_poll.revents & POLLHUP){
        printf("Hangup!\n");
        return false;
      }else{
        printf("Reading...\n");
        res = read(from_child, buf, sizeof(buf)-1);
        if(res < 0){
          perror("read@whisperer_wait");
          continue;
        }
        if(buf[0] != '\0'){
          printf("%s", buf);
        }
        printf("Reading done!\n");
        return true;
      }
    }
    if(fds < 0){
      if(errno != EINTR){
        perror("poll:");
        break;
      }
    }
  }
  return true;
}

static void *whisperer_thread(void *param)
{
  (void) param;
  done = true;
  while(1){
    pthread_mutex_lock(&whisper_mutex);
    done = true;
    while(request == NOP){
      pthread_cond_wait(&whisper_cond, &whisper_mutex);
    }
    if(request == STOP){
      pthread_mutex_unlock(&whisper_mutex);
      break;
    }
    request = NOP;
    pthread_mutex_unlock(&whisper_mutex);
    write_message(to_child, message);
    if(!whisperer_wait()){
      //child gone or other problem
      break;
    }
  }
  whisperer_cleanup();
  return NULL;
}

void whisperer_close(void)
{
  if(!initialized){
    return;
  } 
  pthread_mutex_lock(&whisper_mutex);
  request = STOP;
  pthread_cond_broadcast(&whisper_cond);
  pthread_mutex_unlock(&whisper_mutex);
  pthread_join(whisperer_thread_id, NULL);
}

bool whisper(const char *str)
{
  bool res = false;
  pthread_mutex_lock(&whisper_mutex);
  if(msg_from_str(str, message)){
    request = WHISPER;
    done = false;
    pthread_cond_broadcast(&whisper_cond);
    res = true;
  }
  pthread_mutex_unlock(&whisper_mutex);
  return res;
}

bool whispered(void)
{
  bool res;
  pthread_mutex_lock(&whisper_mutex);
  res = done;
  pthread_mutex_unlock(&whisper_mutex);
  return res;
}


