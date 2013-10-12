#include <stdlib.h>
#include "speech.h"
#include "utils.h"
#include "whisperer.h"


static bool active = false;

bool init_speech()
{
  char *simon = pluginPath("simon");
  active = false;
  if(simon){
    active = whisperer_init(simon);
  }
  free(simon);
  return active;
}

void say(const char *text)
{
  if(!active){
    return;
  }
  whisper(text);
}

void close_speech()
{
  whisperer_close();
  active = false;
}

void cleanup_speech()
{
}

bool speech_active()
{
  //printf("Speech: %s\n", active?"Active\n":"Inactive");
  return active;
}

bool spoken(float elapsed)
{
  (void) elapsed;
  if(!active){
    return true;
  }
  return whispered();
}




