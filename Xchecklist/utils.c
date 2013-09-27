#include "XPLMUtilities.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

static char *msg = NULL;
static size_t msgSize = 0;

//NOT THREAD SAFE!!!

static void xcDebugInt(const char *format, va_list va)
{
  va_list vc;
  
  int cntr = 0;
  if(msg == NULL){
    msgSize = 2;
    msg = (char *)malloc(msgSize);
  }
  if(msg == NULL){
    XPLMDebugString("Xchecklist: Couldn't allocate buffer for messages!\n");
    return;
  }
  while(cntr < 2){//looping once, in case of string too big
    //copy, in case we need another go
    va_copy(vc, va);
    int res = vsnprintf(msg, msgSize, format, vc);
    va_end(vc);
    ++cntr;
    if((res > 0) && ((size_t)res < msgSize)){
      XPLMDebugString(msg);
      return;
    }else if((res < 0)){
      char *err = strerror(errno);
      XPLMDebugString("Xchecklist: Problem with debug message formatting!\n");
      XPLMDebugString(err);
      return;
    }else{
      free(msg);
      msgSize = 2 * res;
      msg = (char *)malloc(msgSize);
    }
  }
  XPLMDebugString("Xchecklist: Problem enlarging message buffer!\n");
}

void xcDebug(const char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  xcDebugInt(format, ap);
  va_end(ap);
}

void xcClose()
{
  free(msg);
  msg = NULL;
  msgSize = 0;
}

