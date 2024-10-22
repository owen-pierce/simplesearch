#ifndef MENU_H
#define MENU_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>  // Include Xft for modern font rendering
#include "path_utils.h"  // For using ResultList
#include "config.h"

// Update function signatures to use Xft
void draw_menu(Display *display, Window window, GC gc, char *input, ResultList *result_list, XftFont *font, XftDraw *draw, XftColor *xft_color, XftColor *highlight_color);
void ensure_window_focus(Display *display, Window window);

#endif
