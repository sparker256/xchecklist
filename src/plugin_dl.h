#ifndef PLUGIN_DL__H
#define PLUGIN_DL__H

#include "XPLMDisplay.h"    // for window creation and manipulation
#include "XPWidgets.h"

#ifdef __cplusplus
extern "C" {
#endif


extern typeof(XPLMGetScreenBoundsGlobal) *XPLMGetScreenBoundsGlobal_ptr;
extern typeof(XPLMGetWindowGeometryOS) *XPLMGetWindowGeometryOS_ptr;
extern typeof(XPLMSetWindowGeometryOS) *XPLMSetWindowGeometryOS_ptr;
extern typeof(XPLMSetWindowGeometryVR) *XPLMSetWindowGeometryVR_ptr;
extern typeof(XPLMSetWindowPositioningMode) *XPLMSetWindowPositioningMode_ptr;
extern typeof(XPLMSetWindowResizingLimits) *XPLMSetWindowResizingLimits_ptr;
extern typeof(XPLMSetWindowTitle) *XPLMSetWindowTitle_ptr;
extern typeof(XPLMWindowIsPoppedOut) *XPLMWindowIsPoppedOut_ptr;
extern typeof(XPGetWidgetUnderlyingWindow) *XPGetWidgetUnderlyingWindow_ptr;
extern bool loadFunctions();


#ifdef __cplusplus
}
#endif

#endif
