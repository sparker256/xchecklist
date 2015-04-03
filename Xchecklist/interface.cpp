#include "chkl_parser.h"
#include "speech.h"
#include <string>

checklist_binder *binder = NULL;

bool start_checklists(const char *fname, int debug)
{
  if(binder != NULL){
    delete binder;
    binder = NULL;
  }
  if(!parse_clist(std::string(fname), debug)){
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
  if(binder != NULL){
    delete binder;
    binder = NULL;
  }
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

bool do_processing(bool visible, bool copilotOn)
{
  if(binder == NULL){
    return false;
  }
  return binder->do_processing(visible, copilotOn);
}

bool get_checklist_names(int *all_checklists, int *menu_size, constname_t *names[], int *indexes[])
{
  if(binder == NULL){
    return false;
  }
  return binder->get_checklist_names(all_checklists, menu_size, names, indexes);
}

bool free_checklist_names(int all_checklists, int menu_size, constname_t *names[], int *indexes[])
{
  if(binder == NULL){
    return false;
  }
  return binder->free_checklist_names(all_checklists, menu_size, names, indexes);
}

bool checklist_finished(bool *switchNext)
{
    *switchNext = false;
    if(binder == NULL){
      return false;
    }
    return binder->checklist_finished(switchNext);
}

