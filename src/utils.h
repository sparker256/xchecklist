#ifndef XC_UTILS__H
#define XC_UTILS__H

#include "XPLMDisplay.h"
#include <stdarg.h>
#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

void xcDebug(const char *format, ...);
void xcClose();
char *prefsPath(void);
char *pluginPath(const char *name);
char *findChecklist(void);
void xcWarn(const char *format, ...);
void xcErr(const char *format, ...);
void xcSummary(void);

extern int clist_dict_found;

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

extern int vr_is_enabled;
extern int is_popped_out;

extern void put_xcvr_gui_window_in_front();

#ifdef __cplusplus
}

#include <string>
#include <map>
void xcLoadDictionaries(std::map<std::string, std::string> &dict);

#endif

#endif
