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
#elif __GNUC__
    #include <GL/gl.h>
#else
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

char * copilot_on = "+";
char * checkmark_off = "    ";
char * checkmark_on = " X ";
// char * checkmark_on = " \u2714\ ";


int checked [25];

int line_number = 2;


size_t ii;

void				xcvr_draw(XPLMWindowID in_window_id, void * in_refcon);
int					xcvr_handle_mouse(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon);


// bounds_lbrt  0 left,  1 bottom,  2 right,  3 top
static int	coord_in_rect(float x, float y, float * bounds_lbrt)  { return (((x - 10) >= bounds_lbrt[0]) && ((x - 20) < bounds_lbrt[2]) && (y < bounds_lbrt[3]) && (y >= bounds_lbrt[1])); }


void	xcvr_draw(XPLMWindowID xcvr_in_window_id, void * in_refcon)
{

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

    int l, t, r, b;
    XPLMGetWindowGeometry(xcvr_in_window_id, &l, &t, &r, &b);

        // Draw the main body of the checklist window.

        line_number = 1;

        for (ii = 0; ii < xcvr_size; ++ii) {

            // If we have a copilot draw the copilot symbol
            if (xcvr_copilot_controlled[ii]) {
                if (vr_is_enabled) {
                    // Draw text for what to be checked.
                    g_copilot_box_lbrt[ii][0] = l;
                    g_copilot_box_lbrt[ii][3] = t - (line_number * char_height) + 10;
                    g_copilot_box_lbrt[ii][2] = g_copilot_box_lbrt[ii][0] + XPLMMeasureString(xplmFont_Proportional, copilot_on, strlen(copilot_on)); // *just* wide enough to fit the button text
                    g_copilot_box_lbrt[ii][1] = g_copilot_box_lbrt[ii][3] - (1.25f * char_height); // a bit taller than the button text


                    // Draw the box around our rudimentary button
                    glColor4fv(light_green_60);
                    glBegin(GL_POLYGON);
                    {
                        glVertex2i(g_copilot_box_lbrt[ii][0], g_copilot_box_lbrt[ii][3]);
                        glVertex2i(g_copilot_box_lbrt[ii][2], g_copilot_box_lbrt[ii][3]);
                        glVertex2i(g_copilot_box_lbrt[ii][2], g_copilot_box_lbrt[ii][1]);
                        glVertex2i(g_copilot_box_lbrt[ii][0], g_copilot_box_lbrt[ii][1]);
                    }
                    glEnd();

                }

                XPLMDrawString(col_cyan, l, t - (line_number * char_height), copilot_on, NULL, xplmFont_Proportional);
            }

            // If checkable then draw a checkbox
            if (!xcvr_item_void[ii]) {
                // Draw the checkbox box.
                // Position the button in the upper left of the window (sized to fit the button text)
                g_check_box_lbrt[ii][0] = l + 16;
                g_check_box_lbrt[ii][3] = t - (line_number * char_height) + 10;
                g_check_box_lbrt[ii][2] = g_check_box_lbrt[ii][0] + XPLMMeasureString(xplmFont_Proportional, checkmark_off, strlen(checkmark_off)); // *just* wide enough to fit the button text
                g_check_box_lbrt[ii][1] = g_check_box_lbrt[ii][3] - (1.25f * char_height); // a bit taller than the button text

                // Draw the box around our rudimentary button
                glColor4fv(light_grey);
                glBegin(GL_POLYGON);
                {
                    glVertex2i(g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][3]);
                    glVertex2i(g_check_box_lbrt[ii][2], g_check_box_lbrt[ii][3]);
                    glVertex2i(g_check_box_lbrt[ii][2], g_check_box_lbrt[ii][1]);
                    glVertex2i(g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][1]);
                }
                glEnd();

                if (xcvr_item_checked[ii])
                {
                    glColor4fv(widget_green);
                    glBegin(GL_POLYGON);
                    {
                        glVertex2i(g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][3]);
                        glVertex2i(g_check_box_lbrt[ii][2], g_check_box_lbrt[ii][3]);
                        glVertex2i(g_check_box_lbrt[ii][2], g_check_box_lbrt[ii][1]);
                        glVertex2i(g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][1]);
                    }
                    glEnd();

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
                g_text_box_lbrt[ii][2] = g_text_box_lbrt[ii][0] + XPLMMeasureString(xplmFont_Proportional, xcvr_text[ii], strlen(xcvr_text[ii])); // *just* wide enough to fit the button text
                g_text_box_lbrt[ii][1] = g_text_box_lbrt[ii][3] - (1.25f * char_height); // a bit taller than the button text


                // Draw the box around our rudimentary button
                glColor4fv(light_green_60);
                glBegin(GL_POLYGON);
                {
                    glVertex2i(g_text_box_lbrt[ii][0], g_text_box_lbrt[ii][3]);
                    glVertex2i(g_text_box_lbrt[ii][2], g_text_box_lbrt[ii][3]);
                    glVertex2i(g_text_box_lbrt[ii][2], g_text_box_lbrt[ii][1]);
                    glVertex2i(g_text_box_lbrt[ii][0], g_text_box_lbrt[ii][1]);
                }
                glEnd();

            }

            XPLMDrawString(col_cyan, l + 40, t - line_number * char_height, (char *)xcvr_text[ii], NULL, xplmFont_Proportional);

            if (vr_is_enabled) {
                // Draw text for the result to be checked
                g_suffix_box_lbrt[ii][0] = l + xcvr_right_text_start;
                g_suffix_box_lbrt[ii][3] = t - (line_number * char_height) + 10;
                g_suffix_box_lbrt[ii][2] = g_suffix_box_lbrt[ii][0] + XPLMMeasureString(xplmFont_Proportional, xcvr_suffix[ii], strlen(xcvr_suffix[ii])); // *just* wide enough to fit the button text
                g_suffix_box_lbrt[ii][1] = g_suffix_box_lbrt[ii][3] - (1.25f * char_height); // a bit taller than the button text


                // Draw the box around our rudimentary button
                glColor4fv(light_green_60);
                glBegin(GL_POLYGON);
                {
                    glVertex2i(g_suffix_box_lbrt[ii][0], g_suffix_box_lbrt[ii][3]);
                    glVertex2i(g_suffix_box_lbrt[ii][2], g_suffix_box_lbrt[ii][3]);
                    glVertex2i(g_suffix_box_lbrt[ii][2], g_suffix_box_lbrt[ii][1]);
                    glVertex2i(g_suffix_box_lbrt[ii][0], g_suffix_box_lbrt[ii][1]);
                }
                glEnd();
            }

            XPLMDrawString(col_cyan, l + xcvr_right_text_start, t - line_number * char_height, (char *)xcvr_suffix[ii], NULL, xplmFont_Proportional);

            line_number = line_number + 2;
        }

        // Find out how big to make the buttons so they always fit on the window


        // Draw the Hide button
        line_number = line_number;
        const char * hide_btn_label = "Hide";

        // 0 left, 1 bottom, 2 right, 3 top
        // Position the button in the upper left of the window (sized to fit the button text)
        g_hide_button_lbrt[0] = l + (xcvr_width / 3) + 10;
        g_hide_button_lbrt[3] = t - (line_number * char_height);
        g_hide_button_lbrt[2] = g_hide_button_lbrt[0] + XPLMMeasureString(xplmFont_Proportional, hide_btn_label, strlen(hide_btn_label) + 9); // *just* wide enough to fit the button text
        g_hide_button_lbrt[1] = g_hide_button_lbrt[3] - (2.00f * char_height); // a bit taller than the button text

        // Draw the box around our rudimentary button
        if (mouse_down_hide) {
            glColor4fv(light_green);
        }
        else {
           glColor4fv(pail_green);
        }
        glBegin(GL_POLYGON);
        {
            glVertex2i(g_hide_button_lbrt[0], g_hide_button_lbrt[3]);
            glVertex2i(g_hide_button_lbrt[2], g_hide_button_lbrt[3]);
            glVertex2i(g_hide_button_lbrt[2], g_hide_button_lbrt[1]);
            glVertex2i(g_hide_button_lbrt[0], g_hide_button_lbrt[1]);
        }
        glEnd();

        // Draw the text on the previous button.
        // 0 left, 1 bottom, 2 right, 3 top
        g_hide_button_lbrt[0] = g_hide_button_lbrt[0] + 20;
        XPLMDrawString(col_black, g_hide_button_lbrt[0], g_hide_button_lbrt[1] + 8, (char *)hide_btn_label, NULL, xplmFont_Proportional);


        // Draw the In Front box
        line_number = line_number;
        const char * in_front_btn_label = "In Front";
        int in_front = XPLMIsWindowInFront(xcvr_in_window_id);

        // 0 left, 1 bottom, 2 right, 3 top
        // Position the button in the upper left of the window (sized to fit the button text)
        g_in_front_button_lbrt[0] = l + (2 * (xcvr_width / 3)) + 20;
        g_in_front_button_lbrt[3] = t - (line_number * char_height);
        g_in_front_button_lbrt[2] = g_in_front_button_lbrt[0] + XPLMMeasureString(xplmFont_Proportional, in_front_btn_label, strlen(in_front_btn_label) + 5); // *just* wide enough to fit the button text
        g_in_front_button_lbrt[1] = g_in_front_button_lbrt[3] - (2.00f * char_height); // a bit taller than the button text

        // Draw the box around our rudimentary button
        if (in_front) {
            glColor4fv(light_green);
        }
        else {
           glColor4fv(pail_green);
        }
        glBegin(GL_POLYGON);
        {
            glVertex2i(g_in_front_button_lbrt[0], g_in_front_button_lbrt[3]);
            glVertex2i(g_in_front_button_lbrt[2], g_in_front_button_lbrt[3]);
            glVertex2i(g_in_front_button_lbrt[2], g_in_front_button_lbrt[1]);
            glVertex2i(g_in_front_button_lbrt[0], g_in_front_button_lbrt[1]);
        }
        glEnd();

        // Draw the text on the previous button.
        // 0 left, 1 bottom, 2 right, 3 top
        g_in_front_button_lbrt[0] = g_in_front_button_lbrt[0] + 10;
        XPLMDrawString(col_black, g_in_front_button_lbrt[0], g_in_front_button_lbrt[1] + 8, (char *)in_front_btn_label, NULL, xplmFont_Proportional);





        // Draw the Previous button
        line_number = line_number + 3;
        const char * previous_btn_label = "Previous";

        // 0 left, 1 bottom, 2 right, 3 top
        // Position the button in the upper left of the window (sized to fit the button text)
        g_previous_button_lbrt[0] = l;
        g_previous_button_lbrt[3] = t - (line_number * char_height);
        g_previous_button_lbrt[2] = g_previous_button_lbrt[0] + XPLMMeasureString(xplmFont_Proportional, previous_btn_label, strlen(previous_btn_label) +5); // *just* wide enough to fit the button text
        g_previous_button_lbrt[1] = g_previous_button_lbrt[3] - (3.00f * char_height); // a bit taller than the button text

        // Draw the box around our rudimentary button
        if (mouse_down_previous) {
            glColor4fv(light_green);
        }
        else {
           glColor4fv(pail_green);
        }
        glBegin(GL_POLYGON);
        {
            glVertex2i(g_previous_button_lbrt[0], g_previous_button_lbrt[3]);
            glVertex2i(g_previous_button_lbrt[2], g_previous_button_lbrt[3]);
            glVertex2i(g_previous_button_lbrt[2], g_previous_button_lbrt[1]);
            glVertex2i(g_previous_button_lbrt[0], g_previous_button_lbrt[1]);
        }
        glEnd();

        // Draw the text on the previous button.
        // 0 left, 1 bottom, 2 right, 3 top
        g_previous_button_lbrt[0] = g_previous_button_lbrt[0] + 15;
        XPLMDrawString(col_black, g_previous_button_lbrt[0], g_previous_button_lbrt[1] + 12, (char *)previous_btn_label, NULL, xplmFont_Proportional);


        // Draw the Check Item button
        const char * check_item_btn_label = "Check Item";

        // 0 left, 1 bottom, 2 right, 3 top
        // Position the button in between the previous and next buttons
        g_check_item_button_lbrt[0] = l + (xcvr_width / 3) + 10;
        g_check_item_button_lbrt[3] = t - (line_number * char_height);
        g_check_item_button_lbrt[2] = g_check_item_button_lbrt[0] + XPLMMeasureString(xplmFont_Proportional, check_item_btn_label, strlen(check_item_btn_label) + 3); // *just* wide enough to fit the button text
        g_check_item_button_lbrt[1] = g_check_item_button_lbrt[3] - (3.00f * char_height); // a bit taller than the button text

        // Draw the box around our rudimentary button
        if (mouse_down_check_item) {
            glColor4fv(light_green);
        }
        else {
           glColor4fv(pail_green);
        }
        glBegin(GL_POLYGON);
        {
            glVertex2i(g_check_item_button_lbrt[0], g_check_item_button_lbrt[3]);
            glVertex2i(g_check_item_button_lbrt[2], g_check_item_button_lbrt[3]);
            glVertex2i(g_check_item_button_lbrt[2], g_check_item_button_lbrt[1]);
            glVertex2i(g_check_item_button_lbrt[0], g_check_item_button_lbrt[1]);
        }
        glEnd();

        // Draw the text on the check item button.
        // 0 left, 1 bottom, 2 right, 3 top
        g_check_item_button_lbrt[0] = g_check_item_button_lbrt[0] + 10;
        XPLMDrawString(col_black, g_check_item_button_lbrt[0], g_check_item_button_lbrt[1] + 12, (char *)check_item_btn_label, NULL, xplmFont_Proportional);


        // Draw the Next button
        const char * next_btn_label = "Next";

        // Position the button in the upper left of the window (sized to fit the button text)
        g_next_button_lbrt[0] = l + (2 * (xcvr_width / 3)) + 20;
        g_next_button_lbrt[3] = t - (line_number * char_height);
        g_next_button_lbrt[2] = g_next_button_lbrt[0] + XPLMMeasureString(xplmFont_Proportional, next_btn_label, strlen(next_btn_label) + 9); // *just* wide enough to fit the button text
        g_next_button_lbrt[1] = g_next_button_lbrt[3] - (3.00f * char_height); // a bit taller than the button text

        // Draw the box around our rudimentary button
        //float green2[] = {0.0, 1.0, 0.0, 1.0};
        if (mouse_down_next) {
            glColor4fv(light_green);
        }
        else {
           glColor4fv(pail_green);
        }
        glBegin(GL_POLYGON);
        {
            glVertex2i(g_next_button_lbrt[0], g_next_button_lbrt[3]);
            glVertex2i(g_next_button_lbrt[2], g_next_button_lbrt[3]);
            glVertex2i(g_next_button_lbrt[2], g_next_button_lbrt[1]);
            glVertex2i(g_next_button_lbrt[0], g_next_button_lbrt[1]);
        }
        glEnd();

        // Draw the text for the next button.
        g_next_button_lbrt[0] = g_next_button_lbrt[0] + 25;
        XPLMDrawString(col_black, g_next_button_lbrt[0], g_next_button_lbrt[1] + 12, (char *)next_btn_label, NULL, xplmFont_Proportional);

}


int	xcvr_handle_mouse(XPLMWindowID xcvr_in_window_id, int xcvr_x, int xcvr_y, XPLMMouseStatus xcvr_mouse_status, void * in_refcon)
{

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
                sprintf(scratch_buffer, "Check Item button has been clicked\n");
                XPLMDebugString(scratch_buffer);
                if(item_checked(checkable)){
                  check_item(checkable);
                }
            }

            else if (coord_in_rect(xcvr_x, xcvr_y, g_hide_button_lbrt))
            {
                mouse_down_hide = 1;
                sprintf(scratch_buffer, "Hide button has been clicked\n");
                XPLMDebugString(scratch_buffer);
                XPLMSetWindowIsVisible(xcvr_in_window_id, 0);
            }

            else if (coord_in_rect(xcvr_x, xcvr_y, g_previous_button_lbrt))
            {
                mouse_down_previous = 1;
                sprintf(scratch_buffer, "Previous button has been clicked\n");
                XPLMDebugString(scratch_buffer);
                prev_checklist();
            }


            else if (coord_in_rect(xcvr_x, xcvr_y, g_next_button_lbrt))
            {
                mouse_down_next = 1;
                sprintf(scratch_buffer, "Next button has been clicked\n");
                XPLMDebugString(scratch_buffer);
                next_checklist(true);
            }

            for (int iii = 0; iii < xcvr_size; ++iii) {

                if(coord_in_rect(xcvr_x, xcvr_y, g_check_box_lbrt[iii])) // user clicked the pop-in/pop-out button
                {
                    sprintf(scratch_buffer, "Clickbox %d clicked \n", iii);
                    XPLMDebugString(scratch_buffer);
                    if (checked[iii] == 0)
                    {
                        checked[iii] = 1;
                    }
                    else
                    {
                       checked[iii] = 0;
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
