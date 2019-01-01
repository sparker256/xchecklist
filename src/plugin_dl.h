#ifndef PLUGIN_DL__H
#define PLUGIN_DL__H

#include "XPLMDisplay.h"    // for window creation and manipulation
#include "XPWidgets.h"

#ifdef __cplusplus
extern "C" {
#endif


extern __typeof__(XPLMGetScreenBoundsGlobal) *XPLMGetScreenBoundsGlobal_ptr;
extern __typeof__(XPLMGetWindowGeometryOS) *XPLMGetWindowGeometryOS_ptr;
extern __typeof__(XPLMSetWindowGeometryOS) *XPLMSetWindowGeometryOS_ptr;
extern __typeof__(XPLMSetWindowGeometryVR) *XPLMSetWindowGeometryVR_ptr;
extern __typeof__(XPLMSetWindowPositioningMode) *XPLMSetWindowPositioningMode_ptr;
extern __typeof__(XPLMSetWindowResizingLimits) *XPLMSetWindowResizingLimits_ptr;
extern __typeof__(XPLMSetWindowTitle) *XPLMSetWindowTitle_ptr;
extern __typeof__(XPLMWindowIsPoppedOut) *XPLMWindowIsPoppedOut_ptr;
extern __typeof__(XPGetWidgetUnderlyingWindow) *XPGetWidgetUnderlyingWindow_ptr;
extern bool loadFunctions();


#ifdef __cplusplus
}
#endif

#endif
