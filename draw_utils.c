#include "draw_utils.h"
#include "config.h"
#include <X11/Xft/Xft.h>  // Include Xft headers for modern font handling

// Draw menu with suggestions and highlights for the selected item using Xft for font rendering
void draw_menu(Display *display, Window window, GC gc, char *input, ResultList *result_list, XftFont *font, XftDraw *draw, XftColor *input_xft_color, XftColor *suggestion_bg_color) {
    // Clear the window and set the background color
    XSetWindowBackground(display, window, WINDOW_BG_COLOR);
    XClearWindow(display, window);

    // Calculate line height with padding (for each suggestion)
    int line_height = font->ascent + font->descent + TOP_PADDING + BOTTOM_PADDING;

    // Get the width of the input text using XftTextExtentsUtf8
    int input_len = strlen(input);
    XGlyphInfo extents;
    XftTextExtentsUtf8(display, font, (XftChar8 *)input, input_len, &extents);
    int input_width = extents.width;

    // Draw the input text at the top-left corner of the window using XftDrawStringUtf8
    XftDrawStringUtf8(draw, input_xft_color, font, 10, line_height/2 + TOP_PADDING/2, (XftChar8 *)input, input_len);

    if (result_list->count > 0) {
        // Starting position for suggestions (right of the input text, with extra gap)
        int x_pos = 10 + input_width + INPUT_TO_SUGGESTION_GAP;  // Increased gap here

        // Get screen width for boundary checks
        int screen_width = DisplayWidth(display, DefaultScreen(display));

        // Loop through suggestions
        for (int i = 0; i < result_list->count; i++) {
            int suggestion_len = strlen(result_list->items[i]);
            XftTextExtentsUtf8(display, font, (XftChar8 *)result_list->items[i], suggestion_len, &extents);
            int suggestion_width = extents.width;

            // Stop if the suggestion exceeds the screen width
            if (x_pos + suggestion_width + SUGGESTION_OFFSET > screen_width) {
                break;
            }

#ifdef ENABLE_HIGHLIGHT
            // Draw background for the selected suggestion (only if highlighting is enabled)
            if (i == result_list->selected) {
                // Set the background color for the selected suggestion using Xft
                XSetForeground(display, gc, SUGGESTION_BG_COLOR);

                // Draw a filled rectangle behind the selected suggestion (adjust y position and width)
                int rect_y = TOP_PADDING - 10; // Align with the top padding
                XFillRectangle(display, window, gc, x_pos - SUGGESTION_OFFSET, rect_y, suggestion_width + SUGGESTION_OFFSET * 2, line_height);
            }
#endif
            // Draw the suggestion text using XftDrawStringUtf8
            XftDrawStringUtf8(draw, suggestion_bg_color, font, x_pos, line_height/2 + TOP_PADDING/2, (XftChar8 *)result_list->items[i], suggestion_len);

            // Move the x position for the next suggestion (leave larger gap)
            x_pos += suggestion_width + SUGGESTION_OFFSET * 3;  // Increased gap between suggestions
        }
    }

    // Flush changes to the display
    XFlush(display);
}

// Ensure the window has focus and raise it to the top
void ensure_window_focus(Display *display, Window window) {
    XRaiseWindow(display, window);
    XSetInputFocus(display, window, RevertToParent, CurrentTime);
}