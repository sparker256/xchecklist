#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include "../src/messages.h"
#include "../src/speech.h"


const char cant_alloc_msg[] = "Can't alloc new message!\n";
const char cant_read_msg[] = "Can't read message!\n";
const char cant_send_msg[] = "Can't send ack message!\n";
const char cant_init_msg[] = "Can't initialize speech synthesis!\n";

message_t msg;


void xcDebug(const char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}


int main(int argc, char *argv[])
{
  (void) argc;
  (void) argv;
  ssize_t res;
  
  if(!new_message(&msg)){
    res = write(STDERR_FILENO, cant_alloc_msg, sizeof(cant_alloc_msg));
    (void) res;
    return 1;
  }

  if(!init_speech()){
    res = write(STDERR_FILENO, cant_init_msg, sizeof(cant_init_msg));
    (void) res;
    return 1;
  }
  res = write(STDOUT_FILENO, "\0", 1);
  if(res < 0){
    //would write problem message, but we can't send anyway
    return 1;
  }

  while(1){
    if(!read_message(STDIN_FILENO, msg)){
      res = write(STDERR_FILENO, cant_read_msg, sizeof(cant_read_msg));
      (void) res;
      break;
    }
    say(str_from_msg(msg));
    while(!spoken(0.0f)){
      usleep(200000);
    }
    res = write(STDOUT_FILENO, "\0", 1);
    if(res < 0){
      //would write problem message, but we can't send anyway
      break;
    }
  }
  
  close_speech();
  cleanup_speech();
  return 0;
}

