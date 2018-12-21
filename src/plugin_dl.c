#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "XPLMUtilities.h"

#include "plugin_dl.h"
#include "utils.h"

typedef struct{
  const char *name;
  void **fun_ptr;
} t_fcn_info;

typeof(XPLMGetScreenBoundsGlobal) *XPLMGetScreenBoundsGlobal_ptr = NULL;
typeof(XPLMGetWindowGeometryOS) *XPLMGetWindowGeometryOS_ptr = NULL;
typeof(XPLMSetWindowGeometryOS) *XPLMSetWindowGeometryOS_ptr = NULL;
typeof(XPLMSetWindowGeometryVR) *XPLMSetWindowGeometryVR_ptr = NULL;
typeof(XPLMSetWindowPositioningMode) *XPLMSetWindowPositioningMode_ptr = NULL;
typeof(XPLMSetWindowResizingLimits) *XPLMSetWindowResizingLimits_ptr = NULL;
typeof(XPLMSetWindowTitle) *XPLMSetWindowTitle_ptr = NULL;
typeof(XPLMWindowIsPoppedOut) *XPLMWindowIsPoppedOut_ptr = NULL;
typeof(XPGetWidgetUnderlyingWindow) *XPGetWidgetUnderlyingWindow_ptr = NULL;

static t_fcn_info funcs[] = {
  {"XPLMGetScreenBoundsGlobal", (void *)&XPLMGetScreenBoundsGlobal_ptr},
  {"XPLMGetWindowGeometryOS", (void *)&XPLMGetWindowGeometryOS_ptr},
  {"XPLMSetWindowGeometryOS", (void *)&XPLMSetWindowGeometryOS_ptr},
  {"XPLMSetWindowGeometryVR", (void *)&XPLMSetWindowGeometryVR_ptr},
  {"XPLMSetWindowPositioningMode", (void *)&XPLMSetWindowPositioningMode_ptr},
  {"XPLMSetWindowResizingLimits", (void *)&XPLMSetWindowResizingLimits_ptr},
  {"XPLMSetWindowTitle", (void *)&XPLMSetWindowTitle_ptr},
  {"XPLMWindowIsPoppedOut", (void *)&XPLMWindowIsPoppedOut_ptr},
  {"XPGetWidgetUnderlyingWindow", (void *)&XPGetWidgetUnderlyingWindow_ptr},
  {NULL, NULL}
};

bool loadFunctions(void)
{
  t_fcn_info *ptr = funcs;
  void *fun_ptr;
  bool res = true;

  while(ptr->name != NULL){
    fun_ptr = XPLMFindSymbol(ptr->name);
    if(fun_ptr != NULL){
      *(ptr->fun_ptr) = fun_ptr;
    }else{
      xcDebug("Couldn't get address of function '%s'.\n", ptr->name);
      res = false;
    }
    ++ptr;
  }
  return res;
}

