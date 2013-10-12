#ifndef XC_UTILS__H
#define XC_UTILS__H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void xcDebug(const char *format, ...);
void xcClose();
char *prefsPath(void);
char *pluginPath(const char *name);
char *findChecklist(void);

#ifdef __cplusplus
}



#endif




#endif
