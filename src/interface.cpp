#include "chkl_parser.h"
#include "speech.h"
#include <string>

checklist_binder *binder = NULL;

void print_checklists(void)
{
  if(binder != NULL){
    std::cout << *binder << std::endl;
  }
}

bool start_checklists(const char *fname, int debug)
{
  if(binder != NULL){
    delete binder;
    binder = NULL;
  }
  parse_clist(std::string(fname), debug);
  if(binder != NULL){
    binder->check_references();
    return binder->select_checklist(0);
  }
  return false;
}

bool discard_checklist()
{
  if(current_checklist != NULL){
    current_checklist = NULL;
  }
  if(binder != NULL){
    delete binder;
    binder = NULL;
  }
  return true;
}


bool stop_checklists()
{
  close_speech();
  cleanup_speech();
  return discard_checklist();
}

bool prev_checklist()
{
  if(binder == NULL){
    return false;
  }
  return binder->prev_checklist();
}

bool next_checklist(bool follow_sw_cont)
{
  if(binder == NULL){
    return false;
  }
  return binder->next_checklist(follow_sw_cont);
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

