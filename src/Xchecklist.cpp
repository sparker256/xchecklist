// *********************************************************
//
//     Xchecklist Plugin
//
//     Michal Navratil
//     William Good
//
//     A plugin to display a clist.txt in widget window
//     Also use speach to prompt user
//
//     Supporting X-Plane 10.51r2 32/64bit
//     X-Plane 11.05r2+
//     Also suporting X-Plane 9.70
//
// *********************************************************

#define VERSION_NUMBER "1.27 build " __DATE__ " " __TIME__


#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPLMPlanes.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPWidgetUtils.h"
#include "XPLMDataAccess.h"

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <sstream>

#include "chkl_parser.h"
#include "interface.h"
#include "speech.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if IBM
#include <windows.h>
#endif

// to get VS2012 to be happy
#if IBM
#define strcasecmp( s1, s2 ) strcmpi( s1, s2 )
#define strncasecmp( s1, s2, n ) strnicmp( s1, s2, n )
#define snprintf sprintf_s
#endif

static int x = 10;
int y = 550;
int w = 300;
int h = 400;
//int x2, y2;

int outLeft, outTop, outRight, outBottom;

int max_items = -1;
int checkable = -1;
int loopnum = 0;

int Item;

//char FileName[256], AircraftPath[512];
//char prefsPath[512];

enum {NEXT_CHECKLIST_COMMAND, PREV_CHECKLIST_COMMAND, CHECK_ITEM_COMMAND, HIDE_CHECKLIST_COMMAND, RELOAD_CHECKLIST_COMMAND};

int MyCommandCallback(
                                   XPLMCommandRef       inCommand,
                                   XPLMCommandPhase     inPhase,
                                   void *               inRefcon);

FILE *my_stream;

XPWidgetID	xCheckListWidget = NULL;

XPWidgetID	setupWidget = NULL;

//XPWidgetID xCheckListCopilotInfoWidget;

XPWidgetID      xChecklistPreviousButton = NULL;
XPWidgetID      xChecklistNextButton = NULL;
std::vector<XPWidgetID>	xCheckListCopilotWidget;
std::vector<XPWidgetID>	xCheckListCheckWidget;
std::vector<XPWidgetID>	xCheckListTextWidget;
std::vector<XPWidgetID>	xCheckListTextAWidget;

XPWidgetID	setupCheckWidget[10] = {NULL};
XPWidgetID	setupTextWidget[10] = {NULL};

XPWidgetID      setupCheckItemButton = NULL;
XPWidgetID      setupNextChecklistButton = NULL;

XPWidgetID      setupMoveChecklistWindowDownButton = NULL;
XPWidgetID      setupSaveSettingsButton = NULL;



XPLMMenuID      PluginMenu     = NULL;
XPLMMenuID      checklistsMenu = NULL;

XPLMCommandRef nextchecklist = NULL;

XPLMCommandRef cmdcheckitem;
XPLMCommandRef cmdnextchecklist;
XPLMCommandRef cmdprevchecklist;
XPLMCommandRef cmdhidechecklist;
XPLMCommandRef cmdreloadchecklist;

static XPLMDataRef              ext_view = NULL;

static XPLMDataRef ParsedDataRef = NULL;

static XPLMDataTypeID ParsedDataRefType = 0;

int checklists_count = -1;

void xCheckListMenuHandler(void *, void *);

static void CreateSetupWidget(int xx1, int yy1, int ww, int hh);
static int xCheckListHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t  inParam2);
static int xSetupHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t  inParam2);
static float dataProcessingCallback(float inElapsed1, float inElapsed2, int cntr, void *ref);
static float xCheckListDeferredInitNewAircraftFLCB(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void * inRefcon);
static bool init_checklists();
static bool init_setup();
static bool do_cleanup();
static bool set_sound(bool enable);
static bool create_dictionary();
static int prev_external_view = false;
static bool restore_on_internal = false;

bool voice_state;
int win_pos_x1 = -1;
int win_pos_x2 = -1;
int win_pos_y1 = -1;
int win_pos_y2 = -1;

const char* setupText[] = {"Translucent Window", "Show Checklist if Checklist exist", \
                                 "Turn Copilot On", "Voice Prompt", "Auto Hide"};

#define SETUP_TEXT_ITEMS (sizeof(setupText) / sizeof(char*))

enum {TRANSLUCENT, SHOW_CHECKLIST, COPILOT_ON, VOICE, AUTO_HIDE};
bool state[SETUP_TEXT_ITEMS];

bool init_done = false;

PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
        int		PluginSubMenuItem;
	int             ChecklistsSubMenuItem;

         xcDebug("Xchecklist: ver " VERSION_NUMBER "\n");

        /* First set up our plugin info. */
        strcpy(outName, "Xchecklist ver " VERSION_NUMBER);
        strcpy(outSig, "Michal_Bill.Example.Xchecklist");
        strcpy(outDesc, "A plugin to display checklists in a widget window.");


  ext_view = XPLMFindDataRef("sim/graphics/view/view_is_external");

// Create our menu
        PluginSubMenuItem = XPLMAppendMenuItem(
                    XPLMFindPluginsMenu(),
                    "Xchecklist",
                    NULL,
                    1);

        PluginMenu = XPLMCreateMenu(
                    "Xchecklist",
                    XPLMFindPluginsMenu(),
                    PluginSubMenuItem,
                    xCheckListMenuHandler,
                    (void *)0);

        XPLMAppendMenuItem(PluginMenu, "Open CheckList", (void *) "checklist", 1);
        XPLMAppendMenuItem(PluginMenu, "Open Setup", (void *) "setup", 1);
        XPLMAppendMenuItem(PluginMenu, "Reload", (void *) "reload", 1);
        XPLMAppendMenuItem(PluginMenu, "Create Dictionary", (void *) "dictionary", 1);

        ChecklistsSubMenuItem = XPLMAppendMenuItem(
                    PluginMenu,
                    "CheckLists",
                    NULL,
                    1);

        checklistsMenu = XPLMCreateMenu(
                    "CheckLists",
                    PluginMenu,
                    ChecklistsSubMenuItem,
                    xCheckListMenuHandler,
                    (void *)1);

        XPLMAppendMenuItem(checklistsMenu, "CheckList1", (void *) 0, 1);
        XPLMAppendMenuItem(checklistsMenu, "CheckList2", (void *) 1, 1);


        cmdcheckitem = XPLMCreateCommand("bgood/xchecklist/check_item","Check Item");
        cmdnextchecklist = XPLMCreateCommand("bgood/xchecklist/next_checklist","Next Checklist");
        cmdprevchecklist = XPLMCreateCommand("bgood/xchecklist/prev_checklist","Prev Checklist");
        cmdhidechecklist = XPLMCreateCommand("bgood/xchecklist/hide_checklist","Hide Checklist");
        cmdreloadchecklist = XPLMCreateCommand("bgood/xchecklist/reload_checklist","Reload Checklist");

        XPLMRegisterCommandHandler(
                    cmdcheckitem,
                    MyCommandCallback,
                    true,
                    (void *)CHECK_ITEM_COMMAND);

        XPLMRegisterCommandHandler(
                    cmdnextchecklist,
                    MyCommandCallback,
                    true,
                    (void *)NEXT_CHECKLIST_COMMAND);

        XPLMRegisterCommandHandler(
                    cmdprevchecklist,
                    MyCommandCallback,
                    true,
                    (void *)PREV_CHECKLIST_COMMAND);

        XPLMRegisterCommandHandler(
                    cmdhidechecklist,
                    MyCommandCallback,
                    true,
                    (void *)HIDE_CHECKLIST_COMMAND);

        XPLMRegisterCommandHandler(
                    cmdreloadchecklist,
                    MyCommandCallback,
                    true,
                    (void *)RELOAD_CHECKLIST_COMMAND);

        init_setup();

        return 1;
}

PLUGIN_API void	XPluginStop(void)
{
        xcDebug("\nXchecklist: Shutting down so trying to save prefs.\n");
        save_prefs();
        stop_checklists();
        do_cleanup();
        XPLMUnregisterFlightLoopCallback(dataProcessingCallback, NULL);
        XPLMDestroyMenu(checklistsMenu);
        XPLMDestroyMenu(PluginMenu);
        xcClose();
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginDisable(void)
{
}

bool do_cleanup()
{
  XPLMClearAllMenuItems(checklistsMenu);
  checklists_count = -1;
  if(xCheckListWidget != NULL){
    if(XPIsWidgetVisible(xCheckListWidget)){
      XPHideWidget(xCheckListWidget);
    }
    XPDestroyWidget(xCheckListWidget, 1);
    xCheckListWidget = NULL;
  }
  if(setupWidget != NULL){
    if(XPIsWidgetVisible(setupWidget)){
      XPHideWidget(setupWidget);
    }
    XPDestroyWidget(setupWidget, 1);
    setupWidget = NULL;
  }
  return true;
}

bool create_checklists_menu(void)
{
  int size;
  int size_all;
  constname_t *names;
  int *indexes;
  XPLMClearAllMenuItems(checklistsMenu);
  if(get_checklist_names(&size_all, &size, &names, &indexes)){
    checklists_count = size_all;
    for(intptr_t i = 0; i < size; ++i){
      XPLMAppendMenuItem(checklistsMenu, names[i], (void *)((intptr_t) indexes[i]), 1);
    }
    free_checklist_names(size_all, size, &names, &indexes);
    return true;
  }
  return false;
}


bool init_checklists()
{
  bool res = false;
        char *clist = findChecklist();
        if(clist){
          res = start_checklists(clist, 0);
          free(clist);
        }else{
          discard_checklist();
        }
        checklists_count = -1; // to make it rebuild menus...
        return res;
}


static void readBoolean(std::fstream &str, bool &res)
{
  if(!str.good()){
    return;
  }
  std::string tmp;
  str>>tmp;
  std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
  if(tmp.find("true", 0) != std::string::npos){
    res = true;
  }else{
    res = false;
  }
}

bool try_open(char *name, std::fstream &fin)
{
  if(name == NULL){
    return false;
  }
  fin.open(name, std::ios::in);
  if(fin.is_open()){
    return true;
  }
  return false;
}

static int limit_coord(int min, int coord, int max)
{
  if(coord < min){
    return min;
  }
  if(coord > max){
    return max;
  }
  return coord;
}

static void fit_interval(int min, int max, int *l, int *r, int wdt)
{
  //printf("Fitting %d, %d to %d, %d (w = %d)\n", *l, *r, min, max, wdt);
  int l1 = limit_coord(min, *l, max);
  int r1 = limit_coord(min, *r, max);

  if(r1 - l1 < wdt){
    r1 = limit_coord(min, l1 + wdt, max);
  }
  if(r1 - l1 < wdt){
    l1 = r1 - wdt;
  }
  *l = l1;
  *r = r1;
  printf("%d - %d\n", l1, r1);
}

static void safe_window_defaults(void)
{
  int screen_w, screen_h;
  XPLMGetScreenSize(&screen_w, &screen_h);
  // Make sure the coords are on screen
  fit_interval(0, screen_w, &win_pos_x1, &win_pos_y1, 300);
  fit_interval(0, screen_h-30, &win_pos_y2, &win_pos_x2, 200);
}


bool save_prefs()
{
  char *prefs;

  if(!init_done){
    return true;
  }
  prefs = pluginPath("Xchecklist.prf");
  if(!prefs){
    xcDebug("Xchecklist: Can't save prefs (NULL plugin path received).\n");
    return false;
  }
  std::fstream fout;
  fout.open(prefs, std::ios::out);
  if(fout.is_open()){
    //Store prefs version first
    fout<<"1"<<std::endl;
    XPGetWidgetGeometry(xCheckListWidget, &win_pos_x1, &win_pos_x2, &win_pos_y1, &win_pos_y2);

    int screen_w, screen_h;
    int to_far_right, to_far_up;
    XPLMGetScreenSize(&screen_w, &screen_h);
    xcDebug("Xchecklist: XPLMGetScreenSize  screen_w = %d  screen_h = %d\n", screen_w, screen_h);
    xcDebug("Xchecklist: Checklist window position win_pos_x1 = %d win_pos_x2 = %d win_pos_y1 = %d win_pos_y2 = %d\n", win_pos_x1, win_pos_x2, win_pos_y1, win_pos_y2);

    if (win_pos_y1 > screen_w) {
        xcDebug("Xchecklist: Checklist to far to the right of the screen.\n");
        to_far_right = win_pos_y1 - screen_w;
        to_far_right = to_far_right + 30;
        win_pos_x1 = win_pos_x1 - to_far_right;
        win_pos_y1 = win_pos_y1 - to_far_right;
        xcDebug("Xchecklist: win_pos_x1 = %d  win_pos_y1 = %d  to_far_right = %d\n", win_pos_x1, win_pos_y1, to_far_right);
    }

    if (win_pos_x2 > screen_h) {
        xcDebug("Xchecklist: Checklist to far up on the screen.\n");
        to_far_up = win_pos_x2 - screen_h;
        to_far_up = to_far_up + 30;
        win_pos_x2 = win_pos_x2 - to_far_up;
        win_pos_y2 = win_pos_y2 - to_far_up;
        xcDebug("Xchecklist: win_pos_x2 = %d  win_pos_y2 = %d  to_far_up = %d\n", win_pos_x2, win_pos_y2, to_far_up);
    }

    XPSetWidgetGeometry(xCheckListWidget, win_pos_x1, win_pos_x2, win_pos_y1, win_pos_y2);

    fout<<win_pos_x1<<" "<<win_pos_x2<<" "<<win_pos_y1<<" "<<win_pos_y2<<std::endl;
    fout<<state[TRANSLUCENT]<<" "<<state[SHOW_CHECKLIST]<<" "<<state[COPILOT_ON]<<" "
        <<state[VOICE]<<" "<<state[AUTO_HIDE]<<std::endl;
    fout.close();
    xcDebug("\nXchecklist: prefs file found, Saving these values.\n");
    xcDebug("Xchecklist: Checklist window position win_pos_x1 = %d win_pos_x2 = %d win_pos_y1 = %d win_pos_y2 = %d\n", win_pos_x1, win_pos_x2, win_pos_y1, win_pos_y2);
    xcDebug("Xchecklist: TRANSLUCENT: %d \n", state[TRANSLUCENT]);
    xcDebug("Xchecklist: SHOW_CHECKLIST: %d\n", state[SHOW_CHECKLIST]);
    xcDebug("Xchecklist: COPILOT_ON: %d\n", state[COPILOT_ON]);
    xcDebug("Xchecklist: VOICE: %d\n", state[VOICE]);
    xcDebug("Xchecklist: AUTO_HIDE: %d\n", state[AUTO_HIDE]);
  }else{
    xcDebug("Xchecklist: Can't open prefs for writing.\n");
    free(prefs);
    return false;
  }
  free(prefs);
  return true;
}


bool init_setup()
{
  char *prefs = NULL;
  std::fstream fin;
  state[TRANSLUCENT] = true;
  state[SHOW_CHECKLIST] = true;
  state[COPILOT_ON] = true;
  state[VOICE] = true;
  voice_state = true;
  state[AUTO_HIDE] = true;
  prefs = pluginPath("Xchecklist.prf");
  if(try_open(prefs, fin)){
    //read new prefs from the fin
    int version = -1;
    fin>>version;
    switch(version){
      case 1:
	//Read the window position
	fin>>win_pos_x1>>win_pos_x2>>win_pos_y1>>win_pos_y2;
	//Read the rest of setup
        fin>>state[TRANSLUCENT]>>state[SHOW_CHECKLIST]>>state[COPILOT_ON]>>state[VOICE]>>state[AUTO_HIDE];
        xcDebug("\nXchecklist: During Startup inital prefs file found, using values found.\n");
        xcDebug("Xchecklist: Checklist window position win_pos_x1 = %d win_pos_x2 = %d win_pos_y1 = %d win_pos_y2 = %d\n", win_pos_x1, win_pos_x2, win_pos_y1, win_pos_y2);
        xcDebug("Xchecklist: TRANSLUCENT: %d \n", state[TRANSLUCENT]);
        xcDebug("Xchecklist: SHOW_CHECKLIST: %d\n", state[SHOW_CHECKLIST]);
        xcDebug("Xchecklist: COPILOT_ON: %d\n", state[COPILOT_ON]);
        xcDebug("Xchecklist: VOICE: %d\n", state[VOICE]);
        xcDebug("Xchecklist: AUTO_HIDE: %d\n", state[AUTO_HIDE]);
	break;
      default:
    xcDebug("Xchecklist: During Startup unknown preferences version, using defaults.\n");
	break;
    }
    //safe_window_defaults();
    // Set the upper left corner from the prefs file
    // Not sure if this is the corect place but it is working
    x = win_pos_x1;
    y = win_pos_x2;
  }else{
    free(prefs);
    prefs = prefsPath();
    if(try_open(prefs, fin)){
      //read old prefs from the fin
      for(size_t i = 0; i < SETUP_TEXT_ITEMS; ++i){
        readBoolean(fin, state[i]);
      }
      fin.close();
      //resave the prefs in the new format
      save_prefs();
    }else{
      //Just using defaults, no problem (maybe just log it)
      xcDebug("Xchecklist: During Startup no prefs file found, using defaults.\n");
    }
  }
  free(prefs);
  voice_state = (state[VOICE]);
  printf("\nTRANSLUCENT: %d \n", state[TRANSLUCENT]);
  printf("SHOW_CHECKLIST: %d\n", state[SHOW_CHECKLIST]);
  printf("COPILOT_ON: %d\n", state[COPILOT_ON]);
  printf("VOICE: %d\n", state[VOICE]);
  printf("AUTO_HIDE: %d\n", state[AUTO_HIDE]);
  return true;
}

bool create_dictionary()
{
    static int line_number = 0;
    static int parsed_array_size = 0;

    std::ofstream clist_dict_file("./Resources/plugins/Xchecklist/clist_dict_drt_last_run.txt");

    if (!clist_dict_file) {
         printf("XchecklistDictionaryCreator: Could not open clist_dict_drt_last_run.txt\n");
         return false;
    }

    std::ifstream drtfile("./Output/preferences/drt_last_run_datarefs.txt");
    std::string line;

    if (!drtfile) {
        printf("XchecklistDictionaryCreator: Could not open drt_last_run_datarefs.txt\n");
        return false;
    } else {
        while (std::getline(drtfile, line)) {
            line_number = line_number + 1;
            std::cout << "" << line << "\n";

            ParsedDataRef = XPLMFindDataRef(line.c_str());
            ParsedDataRefType = XPLMGetDataRefTypes(ParsedDataRef);
            if (ParsedDataRefType == 16) {
                parsed_array_size = XPLMGetDatavi(
                                        ParsedDataRef,
                                        NULL,    /* Can be NULL */
                                        0,
                                        8);
                std::ostringstream s;
                int array_size = 0;
                while (array_size < parsed_array_size) {
                    s.str("");
                    s.clear();
                    s << array_size;
                    std::string i_to_string(s.str());
                    clist_dict_file << line + "[" + i_to_string + "]\n";
                    array_size = array_size + 1;
                }
            }
            if (ParsedDataRefType == 8) {
                parsed_array_size = XPLMGetDatavf(
                                        ParsedDataRef,
                                        NULL,    /* Can be NULL */
                                        0,
                                        8);
                if (parsed_array_size > 1000) {
                    parsed_array_size = 1000;
                }
                std::ostringstream s;
                int array_size = 0;
                while (array_size < parsed_array_size) {
                    s.str("");
                    s.clear();
                    s << array_size;
                    const std::string i_to_string(s.str());
                    clist_dict_file << line + "[" + i_to_string + "]\n";
                    array_size = array_size + 1;
                }
            }

            if (ParsedDataRefType != 8 && ParsedDataRefType != 16) {
                clist_dict_file << line + "\n";
            }
        }
        drtfile.close();
        clist_dict_file.close();
        return true;
    }
}



PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam)
{
  (void) inFrom; // To get rid of warnings on unused variables
  if((inMsg == XPLM_MSG_PLANE_LOADED) && (inParam == 0)){
    //user plane loaded / reloaded, initiate deferred start to avoid
    //  race condition with plane's plugin creating custom datarefs
    XPLMRegisterFlightLoopCallback(xCheckListDeferredInitNewAircraftFLCB, -1, NULL);
    if(!init_done){
      XPLMRegisterFlightLoopCallback(dataProcessingCallback, 0.1f, NULL);
    }
  }
}

// ************************* Aircraft Loaded Deferred Init Callback  *************************
float xCheckListDeferredInitNewAircraftFLCB(float xCheckListelapsedMe, float xCheckListelapsedSim, int xCheckListcounter, void * xCheckListrefcon)
{
    (void) xCheckListelapsedMe; // To get rid of warnings on unused variables
    (void) xCheckListelapsedSim; // To get rid of warnings on unused variables
    (void) xCheckListcounter; // To get rid of warnings on unused variables
    (void) xCheckListrefcon; // To get rid of warnings on unused variables

    if(!init_done){
      set_sound(state[VOICE]);
      voice_state = (state[VOICE]);
      init_done = true;
      // Causes XP9 to segfault!!!
      //XPLMRegisterFlightLoopCallback(dataProcessingCallback, 0.1f, NULL);
    }
    
    do_cleanup();
    init_checklists();
    safe_window_defaults();
    save_prefs();

    return 0; // Returning 0 stops DeferredInitFLCB from being looped again.
}

float dataProcessingCallback(float inElapsed1, float inElapsed2, int cntr, void *ref)
{
  (void) inElapsed1;
  (void) inElapsed2;
  (void) cntr;
  (void) ref;
  static int hide_cntr;

  if(!init_done){
    return 0.1f;
  }


  int visible = XPIsWidgetVisible(xCheckListWidget);
  int external = XPLMGetDatai(ext_view);

  //hide the widget only when changing the view to external
  if(external && (!prev_external_view)){
    if(visible){
      XPHideWidget(xCheckListWidget);
      visible = 0;
      restore_on_internal = true;
    }
  }
  //show only when back in internal views and we remember to (restore_on_internal).
  if((!external) && prev_external_view){
    if((!visible) && restore_on_internal){
      XPShowWidget(xCheckListWidget);
      visible = 1;
      restore_on_internal = false;
    }
  }

  prev_external_view = external;
  do_processing(visible, state[COPILOT_ON]);

  bool switchNext = false;
  if(visible && checklist_finished(&switchNext)){
    hide_cntr = state[AUTO_HIDE] ? (hide_cntr + 1) : 0;
    if(switchNext){
      next_checklist(true);
    }else if(hide_cntr > 30){
      XPHideWidget(xCheckListWidget);
    }
  }else{
    hide_cntr = 0;
  }

  return 0.1f;
}

void xCheckListMenuHandler(void * inMenuRef, void * inItemRef)
{
  (void) inMenuRef;
  unsigned int pageSize = 0;
  const char *pageTitle = "Default Title";
  checklist_item_desc_t pageItems[1];

  if(((intptr_t)inMenuRef == 0) && ((intptr_t) inItemRef != 0)){
    if (!strcmp((char *) inItemRef, "checklist")){
      if (xCheckListWidget == NULL){
        create_checklist(pageSize, pageTitle, pageItems, 120, 0, 0);
      }else{
        if(!XPIsWidgetVisible(xCheckListWidget))
          XPShowWidget(xCheckListWidget);
      }
    }
    if (!strcmp((char *) inItemRef, "setup")){
      if (setupWidget == NULL){
        CreateSetupWidget(400, 550, 215, 200);	//left, top, right, bottom.
      }else{
        if(!XPIsWidgetVisible(setupWidget))
          XPShowWidget(setupWidget);
      }
    }
    if (!strcmp((char *) inItemRef, "reload")){
      do_cleanup();
      init_checklists();
    }
    if (!strcmp((char *) inItemRef, "dictionary")){
        create_dictionary();
    }
  }else if((intptr_t)inMenuRef == 1){
    open_checklist((intptr_t)inItemRef);
  }
}

// This will create our setup widget dialog.
void CreateSetupWidget(int xx, int yy, int ww, int hh)
{

        int xx2 = xx + ww;
        int yy2 = yy - hh;

        int yOffset;


// Create the Main Widget window.
        setupWidget = XPCreateWidget(xx, yy, xx2, yy2,
                      1,		  // Visible
                      "Xchecklist Setup",  // desc
                      1,			  // root
                      NULL,			  // no container
                      xpWidgetClass_MainWindow);


// Add Close Box to the Main Widget.  Other options are available.  See the SDK Documentation.
        XPSetWidgetProperty(setupWidget, xpProperty_MainWindowHasCloseBoxes, 1);

// Display each line of setup.

        for(size_t l = 0; l < SETUP_TEXT_ITEMS; ++l){

        yOffset = (5+18+(l*25));

        setupCheckWidget[l] = XPCreateWidget(xx-5, yy-yOffset, xx+15+20, yy-yOffset-20,
                       1,	// Visible
                       "",      // desc
                       0,	// root
                       setupWidget,
                       xpWidgetClass_Button);

        XPSetWidgetProperty(setupCheckWidget[l], xpProperty_ButtonType, xpRadioButton);
        XPSetWidgetProperty(setupCheckWidget[l], xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
        XPSetWidgetProperty(setupCheckWidget[l], xpProperty_ButtonState, state[l]);

        // Create a checklist item description text widget
        setupTextWidget[l] = XPCreateWidget(xx+30, yy-yOffset, xx+60+200, yy-yOffset-20,
                       1,	        // Visible
                       setupText[l],    // desc
                       0,		// root
                       setupWidget,
                       xpWidgetClass_Caption);
        }

        yOffset = (5+18+(5*25));

        setupMoveChecklistWindowDownButton = XPCreateWidget(xx+10, yy-yOffset, xx+5+200, yy-yOffset-20,
                                  1,
                                  "Move Checklist Window Down",
                                  0,
                                  setupWidget,
                                  xpWidgetClass_Button);

        XPSetWidgetProperty(setupMoveChecklistWindowDownButton, xpProperty_ButtonType, xpPushButton);

        yOffset = (6+18+(6*25));

        setupSaveSettingsButton = XPCreateWidget(xx+10, yy-yOffset, xx+5+200, yy-yOffset-20,
                                  1,
                                  "Save Settings",
                                  0,
                                  setupWidget,
                                  xpWidgetClass_Button);

        XPSetWidgetProperty(setupSaveSettingsButton, xpProperty_ButtonType, xpPushButton);


        // Register our widget handler
        XPAddWidgetCallback(setupWidget, xSetupHandler);


}

bool set_sound(bool enable)
{
    static bool prev = false;
    if(prev != enable){
        prev = enable;
        if(enable){
            return init_speech();
        }else{
            close_speech();
            return true;
        }
    }
    return true;
}



// This is our widget handler.  In this example we are only interested when the close box is pressed.
int	xSetupHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t  inParam2)
{
  (void) inParam1;
  (void) inParam2;


	if (inMessage == xpMessage_CloseButtonPushed)
	{
                if (inWidget == setupWidget)
                {
                      XPHideWidget(setupWidget);
                      return 1;

                }

        }

        if (inMessage == xpMsg_ButtonStateChanged)
        {
                printf("Got button state change message!\n");
                for (size_t ItemNo=0; ItemNo<SETUP_TEXT_ITEMS; ItemNo++)
                {
                        intptr_t tmp;
                        // If the setupCheckWidget check box is checked then set state[ItemNo] true
                        if ((tmp = XPGetWidgetProperty(setupCheckWidget[ItemNo], xpProperty_ButtonState, 0))){
                          state[ItemNo] = true;
                        }
                        else {
                          state[ItemNo] = false;
                        }

                }
                set_sound(state[VOICE]);
                voice_state = (state[VOICE]);

                if(state[TRANSLUCENT]){

                  XPSetWidgetProperty(xCheckListWidget, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);

		  for(size_t iii = 0; iii < xCheckListCopilotWidget.size(); ++iii){
                    XPSetWidgetProperty(xCheckListCopilotWidget[iii], xpProperty_CaptionLit, 1);
                    XPSetWidgetProperty(xCheckListTextWidget[iii], xpProperty_CaptionLit, 1);
                    XPSetWidgetProperty(xCheckListTextAWidget[iii], xpProperty_CaptionLit, 1);
		  }

                  //XPSetWidgetProperty(xCheckListCopilotInfoWidget, xpProperty_CaptionLit, 1);


	        }else{
                  XPSetWidgetProperty(xCheckListWidget, xpProperty_MainWindowType, xpMainWindowStyle_MainWindow);

		  for(size_t iii = 0; iii < xCheckListCopilotWidget.size(); ++iii){
                    XPSetWidgetProperty(xCheckListCopilotWidget[iii], xpProperty_CaptionLit, 0);
                    XPSetWidgetProperty(xCheckListTextWidget[iii], xpProperty_CaptionLit, 0);
                    XPSetWidgetProperty(xCheckListTextAWidget[iii], xpProperty_CaptionLit, 0);
		  }

                  //XPSetWidgetProperty(xCheckListCopilotInfoWidget, xpProperty_CaptionLit, 0);

		}
        }

        if (inMessage == xpMsg_PushButtonPressed)
        {
                if (inParam1 == (intptr_t)setupMoveChecklistWindowDownButton)
                {
                    XPUMoveWidgetBy(
                                xCheckListWidget,
                                0,
                                -25);
                    XPHideWidget(setupWidget);
                }

                if (inParam1 == (intptr_t)setupSaveSettingsButton)
                {
                    save_prefs();
                    XPHideWidget(setupWidget);
                }
        }


	return 0;
}

// This is our widget handler.  In this example we are only interested when the close box is pressed.
int	xCheckListHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t  inParam2)
{
  (void) inParam2;
  if(inMessage == xpMessage_CloseButtonPushed){
    if(inWidget == xCheckListWidget){
      XPHideWidget(xCheckListWidget);
      return 1;
    }
  }

  if(inMessage == xpMsg_PushButtonPressed){
    printf("Button pushed...\n");
    if(inParam1 == (intptr_t)xChecklistPreviousButton){
      prev_checklist();
      return 1;
    }
    if(inParam1 == (intptr_t)xChecklistNextButton){
      next_checklist(true);
      return 1;
    }
  }

  if(inMessage == xpMsg_ButtonStateChanged){
    for(int i = 0; i < max_items; ++i){
      if(inParam1 == (intptr_t)xCheckListCheckWidget[i]){
	    if(i == checkable){
	      if(item_checked(i)){
	        return 1;
	      }
	    }
        XPSetWidgetProperty(xCheckListCheckWidget[i],
        xpProperty_ButtonState, (i < checkable) ? 1 : 0);
	  }
	}
  }
  return 0;
}

bool create_checklist(unsigned int size, const char *title,
                      checklist_item_desc_t items[], int width,
                      int index, int force_show)
{
    size_t i;
    int x2, y2;
    int screen_w, screen_h;

    h = (5+18+(size*20)) + 50;

    restore_on_internal = false;
    if (checklists_count == -1) {
        create_checklists_menu();
    }

    for(i = 0; i < xCheckListCopilotWidget.size(); ++i){
      XPDestroyWidget(xCheckListCopilotWidget[i], 1);
      XPDestroyWidget(xCheckListCheckWidget[i], 1);
      XPDestroyWidget(xCheckListTextWidget[i], 1);
      XPDestroyWidget(xCheckListTextAWidget[i], 1);
    }

    xCheckListCopilotWidget.clear();
    xCheckListCheckWidget.clear();
    xCheckListTextWidget.clear();
    xCheckListTextAWidget.clear();

    checkable = 0;
    if (xCheckListWidget != NULL) {
        //get current window dimensions
        XPGetWidgetGeometry(xCheckListWidget, &outLeft, &outTop, &outRight, &outBottom);
        x = outLeft;
        y = outTop;
        x2 = outRight;
        y2 = outBottom;
        XPDestroyWidget(xCheckListWidget, 1);
    }
    float maxw_1 = 0;
    float maxw_2 = 0;
    float tmp_text, tmp_suffix;
    for (i = 0; i < size; ++i) {
        tmp_text = XPLMMeasureString(xplmFont_Proportional, items[i].text, strlen(items[i].text));
        //xcDebug("Xchecklist: text  %s   tmp_text = %f\n", items[i].text, tmp_text);
        if (tmp_text > maxw_1) {
            maxw_1 = tmp_text;
        }
        tmp_suffix = XPLMMeasureString(xplmFont_Proportional, items[i].suffix, strlen(items[i].suffix));
        //xcDebug("Xchecklist: suffix  %s   tmp_suffix = %f\n", items[i].suffix, tmp_suffix);
        if (tmp_suffix > maxw_2) {
            maxw_2 = tmp_suffix;
        }
    }
    //xcDebug("Xchecklist: maxw_1 = %f   maxw_2 = %f\n", maxw_1, maxw_2);
    //xcDebug("Xchecklist: int(maxw_1) = %d  int(maxw_2) = %d\n", int(maxw_1), int(maxw_2));

    w = int(maxw_1) + int(maxw_2) + 85;// original was 75
    //xcDebug("Xchecklist: width = %d w = %d\n", width, w);
    if (width > w) {
        w = width;
    }
    //xcDebug("Xchecklist: w = %d\n", w);
    XPLMGetScreenSize(&screen_w, &screen_h);
    //xcDebug("Xchecklist: screen_w = %d\n", screen_w);
    if (w > screen_w/2) {
        w = screen_w/2;
    }
    x2 = x + w;
    y2 = y - h;
    //int Index;  //unused
    int WindowCentre = x+w/2;
    int yOffset;
    //bool flip, cop;

    max_items = size;


    // Create the Main Widget window.

    xCheckListWidget = XPCreateWidget(x, y, x2, y2,
                       state[SHOW_CHECKLIST],	// Visible
                       title,	// desc
                       1,		// root
                       NULL,	// no container
                       xpWidgetClass_MainWindow);


// Add Close Box to the Main Widget.  Other options are available.  See the SDK Documentation.
    XPSetWidgetProperty(xCheckListWidget, xpProperty_MainWindowHasCloseBoxes, 1);

    printf("Button # %d has value %s \n", Item, (state[Item])?"true":"false");
    if (state[TRANSLUCENT] == true) {
        XPSetWidgetProperty(xCheckListWidget, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
    }

// Print each line of the checklist in widget window

     xCheckListCopilotWidget.resize(size);
     xCheckListCheckWidget.resize(size);
     xCheckListTextWidget.resize(size);
     xCheckListTextAWidget.resize(size);

     for(i = 0; i < size; ++i){

     yOffset = (5+18+(i*20));
     //flip = items[i].info_only;

            // Create a copilot text widget
            // cop = !(flip);
            xCheckListCopilotWidget[i] = XPCreateWidget(x+5, y-yOffset, x+60+200, y-yOffset-20,
                                         items[i].copilot_controlled,	// Visible
                                         "+",// desc
                                         0,		// root
                                         xCheckListWidget,
                                         xpWidgetClass_Caption);

            if (state[TRANSLUCENT] == true) {
              XPSetWidgetProperty(xCheckListCopilotWidget[i], xpProperty_CaptionLit, 1);
            }


             // Create a check box for a checklist item widget

             xCheckListCheckWidget[i] = XPCreateWidget(x+25, y-yOffset, x+15+20, y-yOffset-20,
                                        !(items[i].item_void),	// Visible
                                        "",// desc
                                        0,		// root
                                        xCheckListWidget,
                                        xpWidgetClass_Button);

             XPSetWidgetProperty(xCheckListCheckWidget[i], xpProperty_ButtonType, xpRadioButton);
             XPSetWidgetProperty(xCheckListCheckWidget[i], xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
             XPSetWidgetProperty(xCheckListCheckWidget[i], xpProperty_ButtonState, 0);

            // Create the description section for checklist item widget
            xCheckListTextWidget[i] = XPCreateWidget(x+40, y-yOffset, x+maxw_1+20, y-yOffset-20,
                                      1,	// Visible
                                      items[i].text,// desc
                                      0,		// root
                                      xCheckListWidget,
                                      xpWidgetClass_Caption);

            if (state[TRANSLUCENT] == true) {

               XPSetWidgetProperty(xCheckListTextWidget[i], xpProperty_CaptionLit, 1);

             }

             // Create the action for a checklist item widget   **  original x+maxw_1+40
             xCheckListTextAWidget[i] = XPCreateWidget(x+maxw_1+50, y-yOffset, x+maxw_1+maxw_2+40, y-yOffset-20,
                                        1,	// Visible
                                        items[i].suffix,// desc
                                        0,		// root
                                        xCheckListWidget,
                                        xpWidgetClass_Caption);

             if (state[TRANSLUCENT] == true) {

                XPSetWidgetProperty(xCheckListTextAWidget[i], xpProperty_CaptionLit, 1);

              }
     }


     // Create a checklist item sction description text widget
     yOffset = (5+18+(15*20));
/*
     xCheckListCopilotInfoWidget = XPCreateWidget(x+5, y-yOffset, x+60+200, y-yOffset-20,
                                   1,	// Visible
                                   "+ = Automaticly Checked Items (Copilot)",// desc
                                   0,		// root
                                   xCheckListWidget,
                                   xpWidgetClass_Caption);

     if (state[TRANSLUCENT] == true) {

      XPSetWidgetProperty(xCheckListCopilotInfoWidget, xpProperty_CaptionLit, 1);

      }
*/
     int bw = w / 2 - 10;
     xChecklistPreviousButton = XPCreateWidget(WindowCentre-bw, y2+40, WindowCentre-5, y2+10,
                                1,
                                "Previous",
                                0,
                                xCheckListWidget,
                                xpWidgetClass_Button);

     XPSetWidgetProperty(xChecklistPreviousButton, xpProperty_ButtonType, xpPushButton);
     XPSetWidgetProperty(xChecklistPreviousButton, xpProperty_Enabled, (index == 0) ? 0 : 1);

    xChecklistNextButton = XPCreateWidget(WindowCentre+5, y2+40, WindowCentre+bw, y2+10,
                                     1, "Next", 0, xCheckListWidget,
                                     xpWidgetClass_Button);

     XPSetWidgetProperty(xChecklistNextButton, xpProperty_ButtonType, xpPushButton);
     XPSetWidgetProperty(xChecklistNextButton, xpProperty_Enabled, (index < (checklists_count-1)) ? 1 : 0);
printf("Checklist index %d (of %d)\n", index, checklists_count);
     // Register our widget handler
     XPAddWidgetCallback(xCheckListWidget, xCheckListHandler);

     if(!XPIsWidgetVisible(xCheckListWidget)){
       XPShowWidget(xCheckListWidget);
     }

     if((!force_show) && (state[SHOW_CHECKLIST] == false)){
       XPHideWidget(xCheckListWidget);
     }
  return true;
}

bool check_item(int itemNo)
{
  printf("Checking item %d\n", itemNo);
  if(itemNo >= 0){
    XPSetWidgetProperty(xCheckListCheckWidget[itemNo], xpProperty_ButtonState, 1);
    item_checked(itemNo);
    return true;
  }
  return false;
}

bool activate_item(int itemNo)
{
  checkable = itemNo;
  printf("Activating item %d\n", itemNo);
  return true;
}

static double get_int_dataref(dataref_struct_t *dref)
{
  return (double)XPLMGetDatai(dref->dref);
}

static double get_float_dataref(dataref_struct_t *dref)
{
  return (double)XPLMGetDataf(dref->dref);
}

static double get_double_dataref(dataref_struct_t *dref)
{
  return XPLMGetDatad(dref->dref);
}

static double get_int_array_dataref(dataref_struct_t *dref)
{
  int val = 0;
  if(XPLMGetDatavi(dref->dref, &val, dref->index, 1) != 1){
    return 0.0;
  }
  return val;
}

static double get_float_array_dataref(dataref_struct_t *dref)
{
  float val = 0.0f;
  if(XPLMGetDatavf(dref->dref, &val, dref->index, 1) != 1){
    return 0.0;
  }
  return val;
}


bool find_dataref(const char *name, dataref_p *dref, value_type_t preferred_type)
{
  *dref= NULL;
  XPLMDataRef tmp = XPLMFindDataRef(name);
  if(tmp == NULL){
    xcDebug("Failed to find a dataref %s\n", name);
    return false;
  }
  int type = XPLMGetDataRefTypes(tmp);
  struct dataref_struct_t tmp_dref;
  tmp_dref.dref = tmp;
  tmp_dref.dref_type = type;
  tmp_dref.index = -1;

  if((preferred_type == TYPE_INT) && (type & xplmType_Int)){
    tmp_dref.accessor = get_int_dataref;
  }else if((preferred_type == TYPE_FLOAT) && (type & xplmType_Float)){
    tmp_dref.accessor = get_float_dataref;
  }else if((preferred_type == TYPE_DOUBLE) && (type & xplmType_Double)){
    tmp_dref.accessor = get_double_dataref;
  }else{
    //preferred type unavailable
    if(type & xplmType_Double){
      tmp_dref.accessor = get_double_dataref;
    }else if(type & xplmType_Float){
      tmp_dref.accessor = get_float_dataref;
    }else if(type & xplmType_Int){
      tmp_dref.accessor = get_int_dataref;
    }else{
      const char *t = NULL;
      switch(preferred_type){
        case TYPE_INT:
          t = "int";
          break;
        case TYPE_FLOAT:
          t = "float";
          break;
        case TYPE_DOUBLE:
          t = "double";
          break;
        default:
          t= "UNKNOWN";
          break;
      }

      xcDebug("Dataref %s doesn't have the needed type %s.\n", name, t);
      return false;
    }
  }
  *dref = new struct dataref_struct_t;
  **dref = tmp_dref;
  std::cout << "Dataref OK" << std::endl;
  return true;
}


bool find_array_dataref(const char *name, int index, dataref_p *dref, value_type_t preferred_type)
{
  *dref= NULL;
  XPLMDataRef tmp = XPLMFindDataRef(name);
  if(tmp == NULL){
    xcDebug("Failed to find a dataref %s\n", name);
    return false;
  }
  int type = XPLMGetDataRefTypes(tmp);
  struct dataref_struct_t tmp_dref;
  tmp_dref.dref = tmp;
  tmp_dref.dref_type = type;
  tmp_dref.index = index >= 0 ? index : 0;

  if(((preferred_type == TYPE_FLOAT) || (preferred_type == TYPE_DOUBLE)) && 
     (type & xplmType_FloatArray)){
    tmp_dref.accessor = get_float_array_dataref;
  }else if((preferred_type = TYPE_INT) && (type & xplmType_IntArray)){
    tmp_dref.accessor = get_int_array_dataref;
  }else{
    if(type & xplmType_FloatArray){
      tmp_dref.accessor = get_float_array_dataref;
    }else if(type & xplmType_IntArray){
      tmp_dref.accessor = get_int_array_dataref;
    }else{
      xcDebug("Array dataref %s doesn't have the needed type %s.\n", name,
              preferred_type == TYPE_INT?"int_array" : "float_array");
      return false;
    }
  }

  *dref = new struct dataref_struct_t;
  **dref = tmp_dref;
  return true;
}


bool dispose_dataref(dataref_p *dref)
{
  (void)dref;
  return true;
}

void get_sim_path(char *path)
{
  XPLMGetSystemPath(path);
}


int MyCommandCallback(XPLMCommandRef       inCommand,
                      XPLMCommandPhase     inPhase,
                      void *               inRefcon)
{
    (void) inCommand;
    //(void) inPhase;
    //(void) inRefcon;

    if (inPhase == xplm_CommandBegin) {
        switch((intptr_t)inRefcon){
        case CHECK_ITEM_COMMAND:
            printf ("trying to make check_item to work \n");
            if (XPIsWidgetVisible(xCheckListWidget)){
                if(item_checked(checkable)){
                  check_item(checkable);
                }
	    }else{
                XPShowWidget(xCheckListWidget);
	    }
            break;
        case NEXT_CHECKLIST_COMMAND:
            if (XPIsWidgetVisible(xCheckListWidget))
                next_checklist(true);
            else
                XPSetWidgetProperty(setupCheckWidget[1], xpProperty_ButtonState, 1);
                XPShowWidget(xCheckListWidget);
            break;
        case PREV_CHECKLIST_COMMAND:
            if (XPIsWidgetVisible(xCheckListWidget))
                prev_checklist();
            else
                XPSetWidgetProperty(setupCheckWidget[1], xpProperty_ButtonState, 1);
                XPShowWidget(xCheckListWidget);
            break;
        case RELOAD_CHECKLIST_COMMAND:
            if (XPIsWidgetVisible(xCheckListWidget)) {
                do_cleanup();
                init_checklists();

            }else{
                XPSetWidgetProperty(setupCheckWidget[1], xpProperty_ButtonState, 1);
                XPShowWidget(xCheckListWidget);
            }
            break;
        case HIDE_CHECKLIST_COMMAND:
            XPHideWidget(xCheckListWidget);
            break;
        }
    }

    return 1;
}

