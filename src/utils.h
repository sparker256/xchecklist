#ifndef XC_UTILS__H
#define XC_UTILS__H

#include "XPLMDisplay.h"
#include <stdarg.h>

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

#ifdef __cplusplus
}

#include <string>
#include <map>
void xcLoadDictionaries(std::map<std::string, std::string> &dict);

#endif

#endif
