#include <stdio.h>
#include <stdlib.h>
#include "../src/interface.h"
#include "../src/speech.h"
#include "../src/utils.h"

bool voice_state = false;

bool dispose_dataref(dataref_p *dref)
{
  (void)dref;
  return true;
}

bool find_array_dataref(const char *name, int index, dataref_p *dref)
{
  (void)name;
  (void)index;
  (void)dref;
  return true;
}

bool find_dataref(const char *name, dataref_p *dref)
{
  (void)name;
  (void)dref;
  return true;
}

float get_float_dataref(dataref_p dref)
{
  (void) dref;
  return 42.43f;
}

bool create_checklist(unsigned int size, const char *title,
                      checklist_item_desc_t items[], int width,
                      int index, int force_show)
{
  (void)size;
  (void)title;
  (void)items;
  (void)width;
  (void)index;
  (void)force_show;
  return true;
}

bool activate_item(int itemNo)
{
  (void)itemNo;
  return true;
}

bool check_item(int itemNo)
{
  (void)itemNo;
  return true;
}

bool init_speech(){return true;}
void say(const char *text){(void)text;}
void close_speech(){}
void cleanup_speech(){}
bool speech_active(){return false;}
bool spoken(float elapsed){(void)elapsed;return true;}


int main(int argc, char *argv[])
{
  xcDebug("Hello %s!\n", "World");
  if(argc > 2){
    start_checklists(argv[1], atoi(argv[2]));
  }else if(argc > 1){
    start_checklists(argv[1], 0);
  }else{
    printf("Usage: %s checklist_name\n", argv[0]);
  }
  stop_checklists();
  xcClose();
  return 0;
}

