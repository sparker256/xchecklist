#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

static char *msg = NULL;
static size_t msgSize = 0;
int clist_dict_found = 0;

/*NOT THREAD SAFE!!!*/

#ifndef CHECKER

#include "XPLMUtilities.h"
#include "XPLMPlanes.h"

#if IBM
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#endif

#if IBM
  static const std::string dirSep = "\\";
#else
  static const std::string dirSep = "/";
#endif

static char *c_strFromString(const std::string str)
{
  char *res = NULL;
  size_t len = strlen(str.c_str()) + 1;
  res = (char *)malloc(len);
  if(res){
    strcpy(res, str.c_str());
  }
  return res;
}

static void fixDirSeparator(std::string &mypath)
{
    //Linux and Win are OK, just Mac needs special treatment...
    (void) mypath;
#if APL
    //On apple a core foundation "link" is used; this is a crude but
    //  effective way to get a path of it...
    std::replace(mypath.begin(), mypath.end(), ':', '/');
    mypath.insert(0, "/Volumes/");
#endif
}

static std::string processPath(char *path, std::string *acf_path = NULL)
{
    std::string mypath(path);
    fixDirSeparator(mypath);
    
    
    if(acf_path){
      size_t start = mypath.rfind(dirSep) + 1;
      *acf_path = mypath.substr(start, mypath.rfind('.') - start);
    }
    
    //equivalent of dirname(mypath)
    mypath.erase(mypath.rfind(dirSep));
    //printf("PATH: '%s'\n", mypath.c_str());

    return mypath;
}

char *prefsPath(void)
{
  char path[512];
  bzero(path, sizeof(path));
  XPLMGetPrefsPath(path);
  if(strlen(path) == 0){
    return NULL;
  }
  //To make sure I don't corrupt XPlane stuff
  std::string myPrefsPath = processPath(path);
  //Add xchecklist.prf to preferences path
  myPrefsPath += dirSep + "Xchecklist.prf";
  //printf("\nPrefs Path to initilize setup  %s \n\n", myPrefsPath.c_str());
  return c_strFromString(myPrefsPath);
}

char *pluginPath(const char *name)
{
  char path[512];
  bzero(path, sizeof(path));
  XPLMGetSystemPath(path);
  if(strlen(path) == 0){
    return NULL;
  }
  //To make sure I don't corrupt XPlane stuff
  std::string myPrefsPath = processPath(path);
  //Add xchecklist.prf to preferences path
  myPrefsPath += dirSep + "Resources" + dirSep + "plugins" + dirSep + 
    "Xchecklist" + dirSep + name;
  //printf("\nPrefs Path to initilize setup  %s \n\n", myPrefsPath.c_str());
  return c_strFromString(myPrefsPath);
}

char *findChecklist(void)
{
  char FileName[1024];
  char AircraftPath[1024];
  bzero(FileName, sizeof(FileName));
  bzero(AircraftPath, sizeof(AircraftPath));
  XPLMGetNthAircraftModel(0, FileName, AircraftPath);
  //To make sure I don't corrupt XPlane stuff
  if(strlen(AircraftPath) == 0){
    return NULL; 
  }
  //Aircraft path contains path to the *.acf file
  //  but we need only the directory name
  std::string myACF("");
  std::string myACFPath = processPath(AircraftPath, &myACF);
  
  //First try aircraft specific checklist
  std::string name0 = myACFPath + dirSep + myACF + "_clist.txt";
  std::string name1 = myACFPath + dirSep + "clist.txt";
  //std::string name2 = myACFPath + dirSep + "plane.txt";
  FILE *f;
  if((f = fopen(name0.c_str(), "r")) != NULL){
      fclose(f);
      xcDebug("Xchecklist: found %s \n", name0.c_str());
      return c_strFromString(name0);
  }

  if((f = fopen(name1.c_str(), "r")) != NULL){
      fclose(f);
      xcDebug("Xchecklist: found %s \n", name1.c_str());
      return c_strFromString(name1);
  }
  return NULL;
}
#endif //CHECKER

static void xcDebugInt(const char *format, va_list va)
{
  va_list vc;
  int res;
  if(msg == NULL){
    msgSize = 2;
    msg = (char *)malloc(msgSize);
  }
  if(msg == NULL){
#ifndef CHECKER    
    XPLMDebugString("Xchecklist: Couldn't allocate buffer for messages!\n");
#else
    printf("Xchecklist: Couldn't allocate buffer for messages!\n");
#endif
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
#ifndef CHECKER    
      XPLMDebugString(msg);
#else
      printf("%s", msg);
#endif
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
  
#ifndef CHECKER
  XPLMDebugString("Xchecklist: Problem with debug message formatting!\n");
#else
  printf("Xchecklist: Problem with debug message formatting!\n");
#endif
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


void xcLoadDictionaries(std::map<std::string, std::string> &dict)
{
  //Scan current directory for the dataref dictionaries
  DIR *dir = opendir(".");
  if(dir == NULL){
    std::cout << "Can't open '.' to scan for dataref dictionaries." << std::endl;
    return;
  }
  struct dirent *de;
  while((de = readdir(dir))){
    const std::string fname(de->d_name);
    if(fname.find("clist_dict_") != 0){
      continue;
    }
    std::cout << "Loading dataref dictionary \"" << fname << "\"." << std::endl;
    clist_dict_found = 1;
    std::ifstream inp(fname.c_str());
    std::string line_str;
    std::string field;
    while(std::getline(inp, line_str)){
      std::istringstream line(line_str);
      field = "";
      line >> field;
      if(field.size() > 0){
        dict[field] = field;
        //std::cout << "Dref: '" << field << "'" << std::endl;
      }
    }
  }
  if (clist_dict_found == 0) {
      printf("Xchecklist: dataref dictionary not found\n");
  }

}

