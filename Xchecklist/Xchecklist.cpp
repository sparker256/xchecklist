// *********************************************************
//
//     Xchecklist Plugin
//
//     Michal Navratil
//     William Good
//
//     Ver 0.76 Working to 32/64bit multiplatform
//     X-Plane.org
//
//     A plugin to display a clist.txt in widget window
//
// *********************************************************

#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPLMPlanes.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMDataAccess.h"

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

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

// need to use _WIN32 to get VS2012 to be happy
#if _WIN32
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

enum {NEXT_CHECKLIST_COMMAND, CHECK_ITEM_COMMAND, HIDE_CHECKLIST_COMMAND};

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
XPWidgetID	xCheckListCopilotWidget[50] = {NULL};
XPWidgetID	xCheckListCheckWidget[50] = {NULL};
XPWidgetID	xCheckListTextWidget[50] = {NULL};
XPWidgetID	xCheckListTextAWidget[50] = {NULL};

XPWidgetID	setupCheckWidget[10] = {NULL};
XPWidgetID	setupTextWidget[10] = {NULL};

XPWidgetID      setupCheckItemButton = NULL;
XPWidgetID      setupNextChecklistButton = NULL;
XPWidgetID      setupSaveSettingsButton = NULL;

XPLMMenuID      PluginMenu     = NULL;
XPLMMenuID      checklistsMenu = NULL;

XPLMCommandRef nextchecklist = NULL;

XPLMCommandRef cmdcheckitem;
XPLMCommandRef cmdnextchecklist;
XPLMCommandRef cmdhidechecklist;

int checklists_count = -1;

void xCheckListMenuHandler(void *, void *);

static void CreateSetupWidget(int xx1, int yy1, int ww, int hh);
static int xCheckListHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t  inParam2);
static int xSetupHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t  inParam2);
static float dataProcessingCallback(float inElapsed1, float inElapsed2, int cntr, void *ref);
static bool init_checklists();
static bool init_setup();
static bool do_cleanup();
static bool set_sound(bool enable);
bool voice_state;

const char* setupText[] = {"Translucent Window", "Show Checklist if Checklist exist", \
                                 "Turn Copilot On", "Voice Prompt", "Auto Hide"};

#define SETUP_TEXT_ITEMS (sizeof(setupText) / sizeof(char*))

enum {TRANSLUCENT, SHOW_CHECKLIST, COPILOT_ON, VOICE, AUTO_HIDE};
bool state[SETUP_TEXT_ITEMS];



PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
        int		PluginSubMenuItem;
	int             ChecklistsSubMenuItem;

         xcDebug("Xchecklist: ver 0.76\n");

        /* First set up our plugin info. */
        strcpy(outName, "Xchecklist ver 0.76");
        strcpy(outSig, "Michal_Bill.Example.Xchecklist");
        strcpy(outDesc, "A plugin to display checklists in a widget window.");


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
        
        XPLMRegisterFlightLoopCallback(dataProcessingCallback, 0.1f, NULL);

        init_setup();
        set_sound(state[VOICE]);
        voice_state = (state[VOICE]);
        do_cleanup();
	//init_checklists();

        cmdcheckitem = XPLMCreateCommand("bgood/xchecklist/check_item","Check Item");
        cmdnextchecklist = XPLMCreateCommand("bgood/xchecklist/next_checklist","Next Checklist");
        cmdhidechecklist = XPLMCreateCommand("bgood/xchecklist/hide_checklist","Hide Checklist");

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
                    cmdhidechecklist,
                    MyCommandCallback,
                    true,
                    (void *)HIDE_CHECKLIST_COMMAND);

        return 1;
}

PLUGIN_API void	XPluginStop(void)
{
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
  constname_t *names;
  XPLMClearAllMenuItems(checklistsMenu);
  if(get_checklist_names(&size, &names)){
    checklists_count = size;
    for(intptr_t i = 0; i < size; ++i){
      XPLMAppendMenuItem(checklistsMenu, names[i], (void *) i, 1);
    }
    free_checklist_names(size, &names);
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


bool init_setup()
{
    char *prefs = prefsPath();
    if(!prefs){
      return false;
    }
    state[TRANSLUCENT] = true;
    state[SHOW_CHECKLIST] = true;
    state[COPILOT_ON] = true;
    state[VOICE] = true;
    voice_state = true;
    state[AUTO_HIDE] = true;
    
    std::fstream fin;
    fin.open(prefs, std::ios::in);
    if(fin.is_open()){
      for(size_t i = 0; i < SETUP_TEXT_ITEMS; ++i){
        readBoolean(fin, state[i]);
      }
      fin.close();
    }
    free(prefs);
    voice_state = (state[VOICE]);
    printf("\nTRANSLUCENT: %d \n", state[TRANSLUCENT]);
    printf("SHOW_CHECKLIST: %d\n", state[SHOW_CHECKLIST]);
    printf("COPILOT_ON: %d\n", state[COPILOT_ON]);
    printf("VOICE: %d\n", state[VOICE]);
    printf("AUTO_HIDE: %d\n", state[AUTO_HIDE]);
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam)
{
  (void) inFrom; // To get rid of warnings on unused variables
  if((inMsg == XPLM_MSG_PLANE_LOADED) && (inParam == 0)){
    //user plane loaded / reloaded
    do_cleanup();
    init_checklists();
  }
}

float dataProcessingCallback(float inElapsed1, float inElapsed2, int cntr, void *ref)
{
  (void) inElapsed1;
  (void) inElapsed2;
  (void) cntr;
  (void) ref;
  static int hide_cntr;        

  do_processing(XPIsWidgetVisible(xCheckListWidget), state[COPILOT_ON]);

  if((XPIsWidgetVisible(xCheckListWidget)) && (state[AUTO_HIDE])){
      if(checklist_finished()){
          ++hide_cntr;
          if(hide_cntr > 30){
              XPHideWidget(xCheckListWidget);
          }
      }else{
          hide_cntr = 0;
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
  checklist_item_desc_t pageItems[50];

  if((intptr_t)inMenuRef == 0){
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
        CreateSetupWidget(400, 550, 215, 175);	//left, top, right, bottom.
      }else{
        if(!XPIsWidgetVisible(setupWidget))
          XPShowWidget(setupWidget);
      }
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
                  
		  for(int iii = 0; iii < 16; ++iii){
                    XPSetWidgetProperty(xCheckListCopilotWidget[iii], xpProperty_CaptionLit, 1);
                    XPSetWidgetProperty(xCheckListTextWidget[iii], xpProperty_CaptionLit, 1);
                    XPSetWidgetProperty(xCheckListTextAWidget[iii], xpProperty_CaptionLit, 1);
		  }

                  //XPSetWidgetProperty(xCheckListCopilotInfoWidget, xpProperty_CaptionLit, 1);


	        }else{
                  XPSetWidgetProperty(xCheckListWidget, xpProperty_MainWindowType, xpMainWindowStyle_MainWindow);

		  for(int iii = 0; iii < 16; ++iii){
                    XPSetWidgetProperty(xCheckListCopilotWidget[iii], xpProperty_CaptionLit, 0);
                    XPSetWidgetProperty(xCheckListTextWidget[iii], xpProperty_CaptionLit, 0);
                    XPSetWidgetProperty(xCheckListTextAWidget[iii], xpProperty_CaptionLit, 0);
		  }

                  //XPSetWidgetProperty(xCheckListCopilotInfoWidget, xpProperty_CaptionLit, 0);

		}
        }

        if (inMessage == xpMsg_PushButtonPressed)
        {
                if (inParam1 == (intptr_t)setupSaveSettingsButton)
                {
                        char *prefs;
                        // ToDo Need to add saving settings to a file
                        XPHideWidget(setupWidget);
                        printf("Save settings pressed \n");

                        //Prefs Path  /home/bill/X-Plane_9.61/Output/preferences/Set X-Plane.prf
                        prefs = prefsPath();
                        if(!prefs){
                          return 1;
                        }
                        my_stream = fopen (prefs, "w");

                        for(size_t i = 0; i < SETUP_TEXT_ITEMS; ++i){
                            fprintf(my_stream, "%s ", ((state[i])?"true":"false"));
                            xcDebug("%s ", ((state[i])?"true":"false"));
                        }

                        fclose (my_stream);
                        free(prefs);
                        return 1;
                }


        }


	return 0;
}

// This is our widget handler.  In this example we are only interested when the close box is pressed.
int	xCheckListHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t  inParam2)
{
  (void) inParam2;
        if (inMessage == xpMessage_CloseButtonPushed)
        {

              if (inWidget == xCheckListWidget)
              {
                        XPHideWidget(xCheckListWidget);
                        return 1;
               }
        }


        if (inMessage == xpMsg_PushButtonPressed)
        {
	  printf("Button pushed...\n");
                if (inParam1 == (intptr_t)xChecklistPreviousButton)
                {
                        prev_checklist();
                        return 1;
                }

                if (inParam1 == (intptr_t)xChecklistNextButton)
                {
                        next_checklist();
                        return 1;
                }
        }
        if(inMessage == xpMsg_ButtonStateChanged)
	{
                for(int i = 0; i < max_items; ++i){
                  if(inParam1 == (intptr_t)xCheckListCheckWidget[i]){
		    if(i == checkable){
		      item_checked(i);
		      return 1;
		    }else{
                      XPSetWidgetProperty(xCheckListCheckWidget[i],
					  xpProperty_ButtonState, (i < checkable) ? 1 : 0);
            }
		  }
		}
	}


        return 0;
}

bool create_checklist(unsigned int size, const char *title,
                      checklist_item_desc_t items[], int width,
                      int index, int force_show)
{
  (void) width;
  unsigned int i;
  int x2;
  int y2;
  if(checklists_count == -1){
    create_checklists_menu();
  }
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
  float tmp;
  for(i = 0; i < size; ++i){
    tmp = XPLMMeasureString(xplmFont_Basic, items[i].text, strlen(items[i].text));
    if(tmp > maxw_1){
      maxw_1 = tmp;
    }
    tmp = XPLMMeasureString(xplmFont_Basic, items[i].suffix, strlen(items[i].suffix));
    if(tmp > maxw_2){
      maxw_2 = tmp;
    }
  }
  w = maxw_1 + maxw_2 + 75;
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

             // Create the action for a checklist item widget
             xCheckListTextAWidget[i] = XPCreateWidget(x+maxw_1+40, y-yOffset, x+maxw_1+maxw_2+40, y-yOffset-20,
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

struct dataref_t{
  XPLMDataRef dref;
  XPLMDataTypeID dref_type;
  int index;
};

bool find_dataref(const char *name, dataref_p *dref)
{
  XPLMDataRef tmp = XPLMFindDataRef(name);
  if(tmp == NULL){
    *dref= NULL;
    return false;
  }
  *dref = new struct dataref_t;
  (*dref)->dref = tmp;
  (*dref)->dref_type = XPLMGetDataRefTypes(tmp);
  (*dref)->index = -1;
  return true;
}


bool find_array_dataref(const char *name, int index, dataref_p *dref)
{
  XPLMDataRef tmp = XPLMFindDataRef(name);
  if(tmp == NULL){
    *dref= NULL;
    return false;
  }
  *dref = new struct dataref_t;
  (*dref)->dref = tmp;
  (*dref)->dref_type = XPLMGetDataRefTypes(tmp);
  (*dref)->index = index;
  return true;
}

//Get value of float dataref
//  name is the dataref name
//
//  Returns the float value of selected dataref
float get_float_dataref(dataref_p dref)
{
  XPLMDataTypeID type = dref->dref_type;
  if(type == xplmType_Unknown){
    return 0.0f;
  }
  if((type & xplmType_FloatArray) != 0){
    int index = (dref->index) < 0 ? 0 : dref->index; 
    float val = 0;
    if(XPLMGetDatavf(dref->dref, &val, index, 1) != 1){
      return 0.0f;
    }else{
      return val;
    }
  }
  if((type & xplmType_IntArray) != 0){
    int index = (dref->index) < 0 ? 0 : dref->index; 
    int val = 0;
    if(XPLMGetDatavi(dref->dref, &val, index, 1) != 1){
      return 0.0f;
    }else{
      return (float) val;
    }
  }
  if((type & xplmType_Int) != 0){
    return (float)XPLMGetDatai(dref->dref);
  }
  if((type & xplmType_Float) != 0){
    return XPLMGetDataf(dref->dref);
  }
  if((type & xplmType_Double) != 0){
    return XPLMGetDatad(dref->dref);
  }
  return 0.0f;
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
            if (XPIsWidgetVisible(xCheckListWidget))
                check_item(checkable);
            else
                XPShowWidget(xCheckListWidget);
            break;
        case NEXT_CHECKLIST_COMMAND:
            if (XPIsWidgetVisible(xCheckListWidget))
                next_checklist();
            else
                XPSetWidgetProperty(setupCheckWidget[1], xpProperty_ButtonState, 1);
                XPShowWidget(xCheckListWidget);
            break;
        case HIDE_CHECKLIST_COMMAND:
            XPHideWidget(xCheckListWidget);
            break;
         }
    }

    return 1;
}

