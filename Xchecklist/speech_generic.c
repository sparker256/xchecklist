#include "XPLMUtilities.h"
#include <speech.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

static const float speech_speed = 12.0f; //characters per second

static bool active = false;

bool init_speech()
{
  active = true;
  return true;
}

static size_t speech_length = 0;

void say(const char *text)
{
  printf("Simon says \"%s\"...\n", text);
  if(!active){
    return;
  }
  printf("Simon really says \"%s\"...\n", text);
  speech_length = strlen(text);
  XPLMSpeakString(text);
}

void close_speech()
{
  printf("Someone closed the speech!!!\n");
  active = false;
}

void cleanup_speech()
{
}

bool speech_active()
{
  return active;
}

bool spoken(float elapsed)
{
  if(!active){
    return true;
  }
  if(elapsed > speech_length / speech_speed){
    return true;
  }else{
    return false;
  }
}


