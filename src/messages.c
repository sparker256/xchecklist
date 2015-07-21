#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include "messages.h"


static bool initialized = false;
static sigset_t pipe_mask;
static bool pipe_pending, pipe_unblock;

struct message_payload_t {
  uint32_t length;
  char msg[]; 
};

struct message_wrap_t {
  unsigned int allocated;
  struct message_payload_t *payload;
};


static void disable_sigpipe(void)
{
  sigset_t pending;
  if(!initialized){
    initialized = true;
    sigemptyset(&pipe_mask);
    sigaddset(&pipe_mask, SIGPIPE);
  }
  sigemptyset(&pending);
  sigpending(&pending);
  pipe_pending = sigismember(&pending, SIGPIPE);
  if(!pipe_pending){
    sigset_t blocked;
    sigemptyset(&blocked);
    pthread_sigmask(SIG_BLOCK, &pipe_mask, &blocked);
    pipe_unblock = !sigismember(&blocked, SIGPIPE);
  }
}

static void enable_sigpipe(void)
{
  //if it was pending, our new one would be merged and it also was blocked before
  if(!pipe_pending){
    sigset_t pending;
    sigemptyset(&pending);
    sigpending(&pending);
    if(sigismember(&pending, SIGPIPE)){
      int cleared, res;
      do{
        res = sigwait(&pipe_mask, &cleared);
      }while((res == -1) && (errno == EINTR));
    }
    if(pipe_unblock){
      pthread_sigmask(SIG_UNBLOCK, &pipe_mask, NULL);
    }
  }
}

bool new_message(message_t *msg)
{
  if(msg == NULL){
    return false;
  }
  (*msg) = (struct message_wrap_t *)malloc(sizeof(struct message_wrap_t));
  (*msg)->allocated = 0;
  (*msg)->payload = NULL;
  return true;
}

void dispose_message(message_t *msg)
{
  if(msg == NULL){
    return;
  }
  if((*msg) != NULL){
    if((*msg)->payload != NULL){
      free((*msg)->payload);
      (*msg)->payload = NULL;
    }
    free(*msg);
    (*msg) = NULL;
  }
}

bool msg_from_str(const char* str, message_t msg)
{
  ssize_t len = strlen(str)+1;//account for the \0
  if(msg == NULL){
    return false;
  }
  if((len + sizeof(uint32_t)) > msg->allocated){
    if(msg->payload != NULL){
      free(msg->payload);
    }
    msg->allocated *= 2;
    if(msg->allocated < len + sizeof(uint32_t)){
      msg->allocated = 2 * len + sizeof(uint32_t);
    }
    msg->payload = (struct message_payload_t *)malloc(msg->allocated);
  }
  msg->payload->length = len + sizeof(uint32_t);
  strncpy(msg->payload->msg, str, len);
  return true;
}

const char *str_from_msg(message_t msg)
{
  if((msg == NULL) || (msg->payload == NULL)){
    return NULL;
  }
  return msg->payload->msg;
}

bool write_message(int fd, message_t msg)
{
  uint32_t len = msg->payload->length;
  char *ptr = (char *)msg->payload;
  printf("Sending message '%s'!\n", msg->payload->msg);
  int res;
  disable_sigpipe();
  while(len > 0){
    res = write(fd, ptr, len);
    if((res < 0) && (errno != EINTR)){
      enable_sigpipe();
      return false;
    }
    len -= res;
    ptr += res;
    if(len <= 0){
      break;
    }
  }
  enable_sigpipe();
  return true;
}

static bool safe_read(int fd, char *buf, size_t len)
{
  struct pollfd pipe_poll;
  int fds, res;
  
  pipe_poll.fd = fd;
  pipe_poll.events = POLLIN;
  pipe_poll.revents = 0;
  while(1){
    //TODO: add check for end condition...
    pipe_poll.events = POLLIN;
    fds = poll(&pipe_poll, 1, 1000);
    if(fds > 0){
      if(pipe_poll.revents & POLLHUP){
        return false;
      }else{
        res = read(fd, buf, len);
        printf("Read %d bytes!\n", res);
        if((res < 0) && (errno != EINTR)){
          return false;
        }
        len -= res;
        buf += res;
        if((res == 0) || (len <= 0)){
          break;
        }
        return true;
      }
    }
  }  
  return true;
}

bool read_message(int fd, message_t msg)
{
  uint32_t new_len;
  if(!safe_read(fd, (char*)&new_len, sizeof(new_len))){
    return false;
  }
  if(new_len > msg->allocated){
    if(msg->payload != NULL){
      free(msg->payload);
    }
    msg->allocated *= 2;
    if(msg->allocated < new_len){
      msg->allocated = 2 * new_len + sizeof(uint32_t);
    }
    msg->payload = (struct message_payload_t *)malloc(msg->allocated);
  }
  msg->payload->length = new_len;
  return safe_read(fd, msg->payload->msg, new_len - sizeof(uint32_t));
}

