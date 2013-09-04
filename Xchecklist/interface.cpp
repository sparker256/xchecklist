#include "chkl_parser.h"
#include "speech.h"
#include <string>

bool start_checklists(const std::string &fname)
{
  if(binder != NULL){
    delete binder;
    binder = NULL;
  }
  if(!parse_clist(fname, 0)){
    return false;
  }
  if(binder != NULL){
    return binder->select_checklist(0);
  }
  return false;
}

bool stop_checklists()
{
  close_speech();
  cleanup_speech();
  return true;
}

bool prev_checklist()
{
  if(binder == NULL){
    return false;
  }
  return binder->prev_checklist();
}

bool next_checklist()
{
  if(binder == NULL){
    return false;
  }
  return binder->next_checklist();
}

bool open_checklist(int number)
{
  if(binder == NULL){
    return false;
  }
  return binder->select_checklist((unsigned int) number);
}


bool item_checked(int item)
{
  if(binder == NULL){
    return false;
  }
  return binder->item_checked(item);
}

bool do_processing(bool visible)
{
  if(binder == NULL){
    return false;
  }
  return binder->do_processing(visible);
}

bool get_checklist_names(int *size, constname_t *names[])
{
  if(binder == NULL){
    return false;
  }
  return binder->get_checklist_names(size, names);
}

bool free_checklist_names(int size, constname_t *names[])
{
  if(binder == NULL){
    return false;
  }
  return binder->free_checklist_names(size, names);
}

bool checklist_finished()
{
    if(binder == NULL){
      return false;
    }
    return binder->checklist_finished();
}
