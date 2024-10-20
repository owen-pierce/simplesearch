#include "menu.h"

void draw_menu(Display *display, Window window, GC gc, char *input, ResultList *result_list, XFontStruct *font) {
    XClearWindow(display, window);

    XSetWindowBackground(display, window, WINDOW_BG_COLOR);
    XClearWindow(display, window);

    int line_height = font->ascent + font->descent + TOP_PADDING + BOTTOM_PADDING; // Total line height with padding

    int input_len = strlen(input);
    int input_width = XTextWidth(font, input, input_len);  // Get width of the input text

    // Set foreground color for input text
    XSetForeground(display, gc, INPUT_TEXT_COLOR);

    // Draw the input text with padding
    XDrawString(display, window, gc, 10, TOP_PADDING + font->ascent, input, input_len); // Adjust y position

    if (result_list->count > 0) {
        // Starting position for the suggestions, pushed to the right of the input text
        int x_pos = 10 + input_width + SUGGESTION_OFFSET;  // Leave some space between the input and suggestions

        // Get screen width for bounds checking
        int screen_width = DisplayWidth(display, DefaultScreen(display));

        // Draw each suggestion
        for (int i = 0; i < result_list->count; i++) {
            int suggestion_len = strlen(result_list->items[i]);
            int suggestion_width = XTextWidth(font, result_list->items[i], suggestion_len);

            // Check if drawing this suggestion would exceed the screen width
            if (x_pos + suggestion_width + SUGGESTION_OFFSET > screen_width) {
                break; // Stop drawing further suggestions if they would exceed the screen width
            }

            // Draw background for the selected suggestion
            if (i == result_list->selected) {
                XSetForeground(display, gc, SUGGESTION_BG_COLOR);
                XFillRectangle(display, window, gc, x_pos - SUGGESTION_OFFSET, TOP_PADDING + font->ascent - BOTTOM_PADDING, suggestion_width + SUGGESTION_OFFSET * 2, line_height); // Adjusted y position for selection
                //XSetForeground(display, gc, BlackPixel(display, 0));  // Switch back to black for the text
            }
            XSetForeground(display, gc, SUGGESTION_TEXT_COLOR);

            // Draw the suggestion text
            XDrawString(display, window, gc, x_pos, TOP_PADDING + font->ascent, result_list->items[i], suggestion_len);

            // Update x_pos to the right for the next suggestion, leaving a gap
            x_pos += suggestion_width + SUGGESTION_OFFSET;
        }
    }

    XFlush(display);
}

void ensure_window_focus(Display *display, Window window) {
    // Raise the window to the top and ensure it has keyboard focus
    XRaiseWindow(display, window);
    XSetInputFocus(display, window, RevertToParent, CurrentTime);
}