#ifndef GUI_WINDOW__H
#define GUI_WINDOW__H


#include "interface.h"
#include <vector>

// Variables to be passed to VR window
extern const char * xcvr_title; // title to be used for the VR window
extern int xcvr_width; // width of the widget checklist window
extern int xcvr_height; // height of the widget checklist window
extern unsigned int xcvr_size; // number of items per checklist page

extern checklist_item_desc_t xcvr_items[];

extern int xcvr_left_text_start;
extern int xcvr_right_text_start;
extern int xcvr_item_checked[];

extern int checkable;

extern int mouse_down_hide;
extern int mouse_down_previous;
extern int mouse_down_check_item;
extern int mouse_down_next;

extern int is_popped_out;
extern int was_popped_out;

extern bool isVREnabled();

extern void put_xcvr_gui_window_in_front();

typedef struct{
  char *str;
  float rgb[3];
  int len;
} t_c_str;
 
typedef std::vector<t_c_str> t_c_string;

#endif
