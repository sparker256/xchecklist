#include "XPLMDisplay.h"    // for window creation and manipulation
#include "XPLMGraphics.h"   // for window drawing
#include "XPLMDataAccess.h" // for the VR dataref
#include "XPLMPlugin.h"     // for XPLM_MSG_SCENERY_LOADED message
#include "XPLMUtilities.h"  // for Various utilities
#include <stdio.h>
#include <string.h>

#if IBM
#include <windows.h>
#endif
#if LIN
#include <GL/gl.h>
#endif
#if __GNUC__ && APL
#include <OpenGL/gl.h>
#endif
#if __GNUC__ && IBM
#include <GL/gl.h>
#endif

#ifndef XPLM301
    #error This is made to be compiled against the XPLM301 SDK
#endif



#include "utils.h"
#include "interface.h"

static float g_check_box_lbrt[25][4]; // left, bottom, right, top
static float g_copilot_box_lbrt[25][4]; // left, bottom, right, top
static float g_text_box_lbrt[25][4]; // left, bottom, right, top
static float g_suffix_box_lbrt[25][4]; // left, bottom, right, top


static float g_hide_button_lbrt[4]; // left, bottom, right, top
static float g_in_front_button_lbrt[4]; // left, bottom, right, top
static float g_previous_button_lbrt[4]; // left, bottom, right, top
static float g_next_button_lbrt[4]; // left, bottom, right, top
static float g_check_item_button_lbrt[4]; // left, bottom, right, top

char scratch_buffer[150];
float col_white[] = {1.0, 1.0, 1.0};
float col_green[] = {0.0, 1.0, 0.0};
float col_red[] = {1.0, 0.0, 0.0};
float col_fuchsia[] = {1.0, 0.0, 1.0};
float col_blue[] = {0.0, 0.0, 1.0};
float col_black[] = {0.0, 0.0, 0.0};
float col_cyan[] = {0.0, 1.0, 1.0};

float green[] = {0.0, 1.0, 0.0, 1.0};
float white[] = {1.0, 1.0, 1.0, 1.0};

float pail_green[] = {0.282, 0.45, 0.25, 1.0};
float light_green[] = {0.43, 0.58, 0.309, 1.0};
float light_green_50[] = {0.43, 0.58, 0.309, 0.5};
float light_green_60[] = {0.43, 0.58, 0.309, 0.6};
float widget_green[] = {0.5019, 1.0, 0.0, 1.0};

float gost_white[] = {0.9725, 0.9725, 1.0, 1.0};

float gainsboro[] = {0.8627, 0.8627, 0.8627, 1.0};
float light_grey[] = {0.8274, 0.8274, 0.8274, 1.0};


float cyan[] = {0.0, 1.0, 1.0, 1.0};

char copilot_on[] = "+";
char checkmark_off[] = "    ";
char checkmark_on[] = " X ";
// char * checkmark_on = " \u2714\ ";


int line_number = 2;


size_t ii;

void				xcvr_draw(XPLMWindowID in_window_id, void * in_refcon);
int					xcvr_handle_mouse(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon);


// bounds_lbrt  0 left,  1 bottom,  2 right,  3 top
//static int	coord_in_rect(float x, float y, float * bounds_lbrt)  { return (((x - 10) >= bounds_lbrt[0]) && ((x - 20) < bounds_lbrt[2]) && (y < bounds_lbrt[3]) && (y >= bounds_lbrt[1])); }
static int	coord_in_rect(float x, float y, float * bounds_lbrt)  { return ((x >= bounds_lbrt[0]) && (x < bounds_lbrt[2]) && (y < bounds_lbrt[3]) && (y >= bounds_lbrt[1])); }


void draw_button(float coords[], int x, int y, int w, int h, float color[], char *label, int char_height)
{
  coords[0] = x;      coords[3] = y;
  coords[2] = x + w;  coords[1] = y - h;
  glColor4fv(color);
  glRecti(coords[0], coords[3], coords[2], coords[1]);
  int tmp = w - XPLMMeasureString(xplmFont_Proportional, label, strlen(label));
  XPLMDrawString(col_black, x + tmp / 2, y - 1.5 * char_height, label, NULL, xplmFont_Proportional);
}

void	xcvr_draw(XPLMWindowID xcvr_in_window_id, void * in_refcon)
{
    (void) in_refcon;
    XPLMSetGraphicsState(
            0 /* no fog */,
            0 /* 0 texture units */,
            0 /* no lighting */,
            0 /* no alpha testing */,
            1 /* do alpha blend */,
            1 /* do depth testing */,
            0 /* no depth writing */
    );

    // We draw our rudimentary button boxes based on the height of the button text
    int char_height;
    XPLMGetFontDimensions(xplmFont_Proportional, NULL, &char_height, NULL);

    is_popped_out = XPLMWindowIsPoppedOut(xcvr_in_window_id);

    int l, t, r, b;
    XPLMGetWindowGeometry(xcvr_in_window_id, &l, &t, &r, &b);

        // Draw the main body of the checklist window.

        line_number = 1;

        for (ii = 0; ii < xcvr_size; ++ii) {

            // If we have a copilot draw the copilot symbol
            if (xcvr_items[ii].copilot_controlled) {
                if (vr_is_enabled) {
                    // Draw text for what to be checked.
                    g_copilot_box_lbrt[ii][0] = l;
                    g_copilot_box_lbrt[ii][3] = t - (line_number * char_height) + 10;
                    g_copilot_box_lbrt[ii][2] = g_copilot_box_lbrt[ii][0] + XPLMMeasureString(xplmFont_Proportional, copilot_on, strlen(copilot_on)); // *just* wide enough to fit the button text
                    g_copilot_box_lbrt[ii][1] = g_copilot_box_lbrt[ii][3] - (1.25f * char_height); // a bit taller than the button text


                    // Draw the box around our rudimentary button
                    glColor4fv(light_green_60);
                    glRecti(g_copilot_box_lbrt[ii][0], g_copilot_box_lbrt[ii][3],
                            g_copilot_box_lbrt[ii][2], g_copilot_box_lbrt[ii][1]);
                }

                XPLMDrawString(col_cyan, l, t - (line_number * char_height), copilot_on, NULL, xplmFont_Proportional);
            }

            // If checkable then draw a checkbox
            if (!xcvr_items[ii].item_void) {
                // Draw the checkbox box.
                // Position the button in the upper left of the window (sized to fit the button text)
                g_check_box_lbrt[ii][0] = l + 16;
                g_check_box_lbrt[ii][3] = t - (line_number * char_height) + 10;
                g_check_box_lbrt[ii][2] = g_check_box_lbrt[ii][0] + XPLMMeasureString(xplmFont_Proportional, checkmark_off, strlen(checkmark_off)); // *just* wide enough to fit the button text
                g_check_box_lbrt[ii][1] = g_check_box_lbrt[ii][3] - (1.25f * char_height); // a bit taller than the button text

                // Draw the box around our rudimentary button
                glColor4fv(light_grey);
                glRecti(g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][3],
                        g_check_box_lbrt[ii][2], g_check_box_lbrt[ii][1]);

                if (xcvr_item_checked[ii])
                {
                    glColor4fv(widget_green);
                    glRecti(g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][3],
                            g_check_box_lbrt[ii][2], g_check_box_lbrt[ii][1]);

                    XPLMDrawString(col_black, g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][1] + 4, (char *)checkmark_on, NULL, xplmFont_Proportional);
                }
                else
                {
                   XPLMDrawString(col_black, g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][1] + 4, (char *)checkmark_off, NULL, xplmFont_Proportional);
                }

                // Draw the checkmard on the checkmark box.
                // XPLMDrawString(col_black, g_check_box_lbrt[i][0], g_check_box_lbrt[i][1] + 4, (char *)Checkmark[i], NULL, xplmFont_Proportional);
            }

            if (vr_is_enabled) {
                // Draw text for what to be checked.
                g_text_box_lbrt[ii][0] = l + 40;
                g_text_box_lbrt[ii][3] = t - (line_number * char_height) + 10;
                g_text_box_lbrt[ii][2] = g_text_box_lbrt[ii][0] + XPLMMeasureString(xplmFont_Proportional, xcvr_items[ii].text, strlen(xcvr_items[ii].text)); // *just* wide enough to fit the button text
                g_text_box_lbrt[ii][1] = g_text_box_lbrt[ii][3] - (1.25f * char_height); // a bit taller than the button text


                // Draw the box around our rudimentary button
                glColor4fv(light_green_60);
                glRecti(g_text_box_lbrt[ii][0], g_text_box_lbrt[ii][3],
                        g_text_box_lbrt[ii][2], g_text_box_lbrt[ii][1]);

            }

            XPLMDrawString(col_cyan, l + 40, t - line_number * char_height, (char *)xcvr_items[ii].text, NULL, xplmFont_Proportional);

            if (vr_is_enabled) {
                // Draw text for the result to be checked
                g_suffix_box_lbrt[ii][0] = l + xcvr_right_text_start;
                g_suffix_box_lbrt[ii][3] = t - (line_number * char_height) + 10;
                g_suffix_box_lbrt[ii][2] = g_suffix_box_lbrt[ii][0] + XPLMMeasureString(xplmFont_Proportional, xcvr_items[ii].suffix, strlen(xcvr_items[ii].suffix)); // *just* wide enough to fit the button text
                g_suffix_box_lbrt[ii][1] = g_suffix_box_lbrt[ii][3] - (1.25f * char_height); // a bit taller than the button text


                // Draw the box around our rudimentary button
                glColor4fv(light_green_60);
                glRecti(g_suffix_box_lbrt[ii][0], g_suffix_box_lbrt[ii][3],
                        g_suffix_box_lbrt[ii][2], g_suffix_box_lbrt[ii][1]);

            }

            XPLMDrawString(col_cyan, l + xcvr_right_text_start, t - line_number * char_height, (char *)xcvr_items[ii].suffix, NULL, xplmFont_Proportional);

            line_number = line_number + 2;
        }

        // Find out how big to make the buttons so they always fit on the window
        line_number = line_number;
        typedef enum {HIDE_LABEL, IN_FRONT_LABEL, PREVIOUS_LABEL, CHECK_ITEM_LABEL, NEXT_LABEL, SENTINEL_LABEL} label_names;
        const char *labels[] = {"Hide", "In Front", "Previous", "Check Item", "Next", ""};

        const int c_BUTTON2BORDER = 20;
        const int c_BUTTON2BUTTON = 20;
        int button_w = (xcvr_width - 2 * (c_BUTTON2BORDER + c_BUTTON2BUTTON)) / 3;
        int button_h = 2.25 * char_height;

        draw_button(g_hide_button_lbrt, l + c_BUTTON2BORDER + button_w + c_BUTTON2BUTTON, t - (line_number * char_height),
                    button_w, button_h, mouse_down_hide ? light_green : pail_green, (char *)labels[HIDE_LABEL], char_height);

        draw_button(g_in_front_button_lbrt, l + c_BUTTON2BORDER + 2 * (button_w + c_BUTTON2BUTTON), t - (line_number * char_height),
                    button_w, button_h, XPLMIsWindowInFront(xcvr_in_window_id) ? light_green : pail_green, (char *)labels[IN_FRONT_LABEL], char_height);

        line_number = line_number + 3;
        draw_button(g_previous_button_lbrt, l + c_BUTTON2BORDER, t - (line_number * char_height),
                    button_w, button_h, mouse_down_previous ? light_green : pail_green, (char *)labels[PREVIOUS_LABEL], char_height);

        draw_button(g_check_item_button_lbrt, l + c_BUTTON2BORDER + button_w + c_BUTTON2BUTTON, t - (line_number * char_height),
                    button_w, button_h, mouse_down_check_item ? light_green : pail_green, (char *)labels[CHECK_ITEM_LABEL], char_height);

        draw_button(g_next_button_lbrt, l + c_BUTTON2BORDER + 2 * (button_w + c_BUTTON2BUTTON), t - (line_number * char_height),
                    button_w, button_h, mouse_down_next ? light_green : pail_green, (char *)labels[NEXT_LABEL], char_height);
}


int	xcvr_handle_mouse(XPLMWindowID xcvr_in_window_id, int xcvr_x, int xcvr_y, XPLMMouseStatus xcvr_mouse_status, void * in_refcon)
{
    (void) in_refcon;
    if(xcvr_mouse_status == xplm_MouseDown)
    {
        if(!XPLMIsWindowInFront(xcvr_in_window_id))
        {
            XPLMBringWindowToFront(xcvr_in_window_id);
        }
        else
        {
            if (coord_in_rect(xcvr_x, xcvr_y, g_check_item_button_lbrt)) {
                mouse_down_check_item = 1;
                if(item_checked(checkable)) {
                    check_item(checkable);
                }
                put_xcvr_gui_window_in_front();
            }

            else if (coord_in_rect(xcvr_x, xcvr_y, g_hide_button_lbrt))
            {
                mouse_down_hide = 1;
                XPLMSetWindowIsVisible(xcvr_in_window_id, 0);
            }

            else if (coord_in_rect(xcvr_x, xcvr_y, g_previous_button_lbrt))
            {
                mouse_down_previous = 1;
                prev_checklist();
                put_xcvr_gui_window_in_front();
            }


            else if (coord_in_rect(xcvr_x, xcvr_y, g_next_button_lbrt))
            {
                mouse_down_next = 1;
                next_checklist(true);
                put_xcvr_gui_window_in_front();
            }

            for (unsigned int iii = 0; iii < xcvr_size; ++iii) {
                if((iii == (unsigned int)checkable) && coord_in_rect(xcvr_x, xcvr_y, g_check_box_lbrt[iii])) // user clicked the pop-in/pop-out button
                {
                  if(item_checked(checkable)) {
                    check_item(checkable);
                  }
                }
            }

        }

    }

    if(xcvr_mouse_status == xplm_MouseUp)
    {
        mouse_down_hide = 0;
        mouse_down_previous = 0;
        mouse_down_check_item = 0;
        mouse_down_next = 0;
    }

    return 1;
}
