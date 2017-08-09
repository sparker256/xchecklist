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

double get_double_dataref(dataref_p dref)
{
  (void) dref;
  return 3.141592657;
}

double get_float_dataref(dataref_p dref)
{
  (void) dref;
  return 3.14;
}

double get_int_dataref(dataref_p dref)
{
  (void) dref;
  return 42;
}

double get_float_array_dataref(dataref_p dref)
{
  (void) dref;
  return 7.77;
}

double get_int_array_dataref(dataref_p dref)
{
  (void) dref;
  return 14;
}



bool find_array_dataref(const char *name, int index, dataref_p *dref, value_type_t preferred_type)
{

  (void)name;
  (void)index;
  *dref = (dataref_p)malloc(sizeof(struct dataref_struct_t));
  switch(preferred_type){
    case TYPE_INT:
      (*dref)->accessor = get_int_array_dataref;
      break;
    default:
      (*dref)->accessor = get_float_array_dataref;
      break;
  }
  return true;
}

bool find_dataref(const char *name, dataref_p *dref, value_type_t preferred_type)
{
  (void)name;
  (void)dref;
  *dref = (dataref_p)malloc(sizeof(struct dataref_struct_t));
  switch(preferred_type){
    case TYPE_INT:
      (*dref)->accessor = get_int_dataref;
      break;
    case TYPE_FLOAT:
      (*dref)->accessor = get_float_dataref;
      break;
    default:
      (*dref)->accessor = get_double_dataref;
      break;
  }
  return true;
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

