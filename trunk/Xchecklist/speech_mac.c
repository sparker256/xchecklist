#include <stdlib.h>
#include "speech.h"
#include "utils.h"
#include "whisperer.h"


static bool active = false;

static bool init_simon(char *name)
{
  char *simon = pluginPath(name);
  if(simon){
    active = whisperer_init(simon);
  }
  free((void*)simon);
  return active;
}

bool init_speech()
{
  active = false;
#if APL
  return init_simon("simon_mac");
#elif LIN
  bool res;
  res = init_simon("simon_lin32");
  if(!res){
    return init_simon("simon_lin64");
  }
#endif
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




