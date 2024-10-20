#ifndef MENU_H
#define MENU_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "path_utils.h"  // For using ResultList

#define SUGGESTION_HEIGHT 30
#define SUGGESTION_OFFSET 10
#define FONT_SIZE 32
#define TOP_PADDING 10
#define BOTTOM_PADDING 10
#define INPUT_TEXT_COLOR 0x000000
#define SUGGESTION_TEXT_COLOR 0xFFFFFF
#define SUGGESTION_BG_COLOR 0x808080
#define WINDOW_BG_COLOR 0x808080

void draw_menu(Display *display, Window window, GC gc, char *input, ResultList *result_list, XFontStruct *font);
void ensure_window_focus(Display *display, Window window);

#endif