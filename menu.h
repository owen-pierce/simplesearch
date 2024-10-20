#ifndef MENU_H
#define MENU_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "path_utils.h"  // For using ResultList
#include "config.h"

void draw_menu(Display *display, Window window, GC gc, char *input, ResultList *result_list, XFontStruct *font);
void ensure_window_focus(Display *display, Window window);

#endif