#include <string>
#include "XPLMUtilities.h"
#include "XPLMPlanes.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
static char *msg = NULL;
static size_t msgSize = 0;

/*NOT THREAD SAFE!!!*/

#if IBM
  static const std::string dirSep = "\\";
#else
  static const std::string dirSep = "/";
#endif

static char *c_strFromString(const std::string str)
{
  char *res = NULL;
  res = (char *)malloc(str.size());
  if(res){
    strcpy(res, str.c_str());
  }
  return res;
}

static std::string processPath(char *path)
{
    std::string mypath(path);
#if APL
    //On apple a core foundation "link" is used; this is a crude but
    //  effective way to get a path of it...
    std::replace(mypath.begin(), mypath.end(), ':', '/');
    mypath.insert(0, "/Volumes/");
#endif
    //equivalent of dirname(mypath)
    mypath.erase(mypath.rfind(dirSep));
    
    //printf("PATH: '%s'\n", mypath.c_str());
    return mypath;
}

char *prefsPath(void)
{
  char path[512];
  XPLMGetPrefsPath(path);
  //To make sure I don't corrupt XPlane stuff
  std::string myPrefsPath = processPath(path);
  //Add xchecklist.prf to preferences path
  myPrefsPath += dirSep + "Xchecklist.prf";
  //printf("\nPrefs Path to initilize setup  %s \n\n", myPrefsPath.c_str());
  return c_strFromString(myPrefsPath);
}

char *findChecklist(void)
{
  char FileName[1024];
  char AircraftPath[1024];
  XPLMGetNthAircraftModel(0, FileName, AircraftPath);
  //To make sure I don't corrupt XPlane stuff
  if(strlen(AircraftPath) == 0){
    return false; 
  }
  //Aircraft path contains path to the *.acf file
  //  but we need only the directory name
  std::string myACFPath = processPath(AircraftPath);
	    
  std::string name1 = myACFPath + dirSep + "clist.txt";
  std::string name2 = myACFPath + dirSep + "plane.txt";
  FILE *f;
  if((f = fopen(name1.c_str(), "r")) != NULL){
      fclose(f);
      return c_strFromString(name1);
  }

  if((f = fopen(name2.c_str(), "r")) != NULL){
      fclose(f);
      return c_strFromString(name2);
  }
  return NULL;
}


static void xcDebugInt(const char *format, va_list va)
{
  va_list vc;
  int res;
  if(msg == NULL){
    msgSize = 2;
    msg = (char *)malloc(msgSize);
  }
  if(msg == NULL){
    XPLMDebugString("Xchecklist: Couldn't allocate buffer for messages!\n");
    return;
  }
  while(1){ /*looping once, in case of string too big*/
    /*copy, in case we need another go*/
#if IBM
    vc = va; /*no va_copy on VC*/
#else
    va_copy(vc, va);
#endif
    res = vsnprintf(msg, msgSize, format, vc);
    va_end(vc);
    
    if((res > -1) && ((size_t)res < msgSize)){
      XPLMDebugString(msg);
      return;
    }else{
      void *tmp;
      msgSize *= 2;
      if((tmp = realloc(msg, msgSize)) == NULL){
        break;
      }
      msg = (char *)tmp;
    }
  }
  XPLMDebugString("Xchecklist: Problem with debug message formatting!\n");
  msg = NULL;
  msgSize = 0;
  return;
}

void xcDebug(const char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  xcDebugInt(format, ap);
  va_end(ap);
}

void xcClose()
{
  free(msg);
  msg = NULL;
  msgSize = 0;
}

