#ifndef INTERFACE__H
#define INTERFACE__H

#include <stdbool.h>

typedef struct{
  //Text of the checklist item
  const char *text;
  //Text to be in the button (or to the right)
  const char *suffix;
  //True if the item has a controlling dataref attached
  bool copilot_controlled;
  //True if the item is not user checkable
  bool info_only;
  //True if the item is only a "label"
  bool item_void;
}checklist_item_desc_t;

typedef enum{
  XC_UNKNOWN     = 0,
  XC_INTEGER     = 1,
  XC_FLOAT       = 2,
  XC_DOUBLE      = 4,
  XC_FLOAT_ARRAY = 8,
  XC_INT_ARRAY   = 16
} dataref_type_t;

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
  Functions provided by the backend (parser + checklist logic)
 *******************************************************************************/

//Call in the plugin init phase
//  fname is the checklist file name
//
//  Returns true if no problems were encountered
bool start_checklists(const char *fname, int debug);
bool stop_checklists();

//Call by the plugin, go back to the previous checklist
//
// Returns true if on problems were encountered
bool prev_checklist();

//Call by the plugin, advance to the next checklist
//
// Returns true if on problems were encountered
bool next_checklist();

//Call by the plugin, open checklist number number
//
// Returns true if on problems were encountered
bool open_checklist(int number);

//Frontend informs us, that user has checked an item
//  item is the index of checked item (provided by checklist_item_desc_t.index)
//  
//  Return true if no problems were encountered.
bool item_checked(int item);

//This function is to be called periodically to allow the backend
//  check values of datarefs and act accordingly...
//
//  Returns true if no problems were encountered.
bool do_processing(bool visible, bool copilotOn);

//Returns/disposes of list of all checklist names
//
// Make sure you free the list when you don't need
//  it anymore to prevent resource leak
//
//Usage example:
//  constname_t *names = NULL;
//  int *indexes = NULL;
//  int names_size;
//  get_checklist_names(&names_size, &names, &indexes);
//  for(int i = 0; i < names_size; ++i){
//    printf("Checklist : %d: %s\n", indexes[i], names[i]);
//  }
//  free_checklist_names(names_size, &names, &indexes);
typedef const char *constname_t;

bool get_checklist_names(int *all_checklists, int *menu_size, constname_t *names[], int *indexes[]);
bool free_checklist_names(int all_checklists, int menu_size, constname_t *names[], int *indexes[]);
bool checklist_finished();

/*******************************************************************************
  Functions provided by the frontend (GUI)
 *******************************************************************************/

//Called by the backend to construct the checklist GUI
//  size denotes number of items in the items array
//  title specifies the name of the checklist
//  items is array of checklist item descriptions
//
//  Return true if no problems were encountered, false otherwise. 
bool create_checklist(unsigned int size, const char *title, 
                      checklist_item_desc_t items[], int width,
                      int index, int force_show);


//Inform frontend to check an item (to be used in copilot mode)
//  item is the index of an item to be checked(provided by checklist_item_desc_t.index)
//
//  Return true if no problems were encountered.
bool check_item(int item);

//Inform frontend to "prompt" user to attend to a specified item
//  item is the index of an item to be hilight
//
//  Return true if no problems were encountered.
bool activate_item(int item);


struct dataref_t;
typedef struct dataref_t* dataref_p;

bool find_dataref(const char *name, dataref_p *dref);
bool find_array_dataref(const char *name, int index, dataref_p *dref);

//Get value of float dataref
//  name is the dataref name
//
//  Returns the float value of selected dataref
float get_float_dataref(dataref_p dref);

bool dispose_dataref(dataref_p *dref);

void dataref_val(dataref_p dref, float f);
void get_sim_path(char *path);

#ifdef __cplusplus
}
#endif


#endif
