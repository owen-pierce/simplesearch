#include "draw_utils.h"
#include "config.h"

// Draw menu with suggestions and highlights for the selected item
void draw_menu(Display *display, Window window, GC gc, char *input, ResultList *result_list, XFontStruct *font) {
    // Clear the window and set the background color
    XSetWindowBackground(display, window, WINDOW_BG_COLOR);
    XClearWindow(display, window);

    // Calculate line height with padding (for each suggestion)
    int line_height = font->ascent + font->descent + TOP_PADDING + BOTTOM_PADDING;

    // Get the width of the input text to place suggestions correctly
    int input_len = strlen(input);
    int input_width = XTextWidth(font, input, input_len);

    // Set the foreground color for input text
    XSetForeground(display, gc, INPUT_TEXT_COLOR);

    // Draw the input text at the top-left corner of the window
    XDrawString(display, window, gc, 10, TOP_PADDING + font->ascent, input, input_len);

    if (result_list->count > 0) {
        // Starting position for suggestions (right of the input text, with extra gap)
        int x_pos = 10 + input_width + INPUT_TO_SUGGESTION_GAP;  // Increased gap here

        // Get screen width for boundary checks
        int screen_width = DisplayWidth(display, DefaultScreen(display));

        // Loop through suggestions
        for (int i = 0; i < result_list->count; i++) {
            int suggestion_len = strlen(result_list->items[i]);
            int suggestion_width = XTextWidth(font, result_list->items[i], suggestion_len);

            // Stop if the suggestion exceeds the screen width
            if (x_pos + suggestion_width + SUGGESTION_OFFSET > screen_width) {
                break;
            }

#ifdef ENABLE_HIGHLIGHT
            // Draw background for the selected suggestion (only if highlighting is enabled)
            if (i == result_list->selected) {
                // Set the background color for the selected suggestion
                XSetForeground(display, gc, SUGGESTION_BG_COLOR);

                // Draw a filled rectangle behind the selected suggestion (adjust y position and width)
                int rect_y = TOP_PADDING; // Align with the top padding
                XFillRectangle(display, window, gc, x_pos - SUGGESTION_OFFSET, rect_y, suggestion_width + SUGGESTION_OFFSET * 2, line_height);
            }
#endif
            // Set foreground color for suggestion text
            XSetForeground(display, gc, SUGGESTION_TEXT_COLOR);

            // Draw the suggestion text
            XDrawString(display, window, gc, x_pos, TOP_PADDING + font->ascent, result_list->items[i], suggestion_len);

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
