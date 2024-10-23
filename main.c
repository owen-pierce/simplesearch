#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xft/Xft.h>
#include <X11/extensions/Xinerama.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "path_utils.h"
#include "draw_utils.h"
#include "config.h"

int debug = 0;  // Global debug flag

// Helper function to print debug info
void debug_print(const char *msg) {
    if (debug) {
        printf("DEBUG: %s\n", msg);
    }
}

// Function to convert hex color to XRenderColor
XRenderColor hex_to_xrendercolor(unsigned int hex) {
    return (XRenderColor) {
        .red = ((hex >> 16) & 0xFF) * 0xFFFF / 255,
        .green = ((hex >> 8) & 0xFF) * 0xFFFF / 255,
        .blue = (hex & 0xFF) * 0xFFFF / 255,
        .alpha = 0xFFFF
    };
}

int main(int argc, char *argv[]) {
    // Parse command-line arguments for the -d (debug) flag
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            debug = 1;
            printf("Debug mode enabled.\n");
        }
    }

    Display *display;
    Window window;
    XEvent event;
    int screen;
    GC gc;
    XftFont *font;
    XftDraw *draw;
    XftColor input_xft_color, suggestion_xft_color;

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    debug_print("Display opened successfully.");

    screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int window_height = FONT_SIZE + TOP_PADDING + BOTTOM_PADDING;

    // Use the WINDOW_BG_COLOR defined in config.h for the window background
    unsigned long bg_pixel = WINDOW_BG_COLOR; 
    unsigned long fg_pixel = INPUT_TEXT_COLOR; // Use input text color

    // window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, screen_width, window_height, 0,
    //                              fg_pixel, bg_pixel);

// Get Xinerama information to find the primary monitor
    int xinerama_major_version, xinerama_minor_version;
    int num_monitors;
    XineramaScreenInfo *screens = NULL;

    if (XineramaQueryExtension(display, &xinerama_major_version, &xinerama_minor_version) && XineramaIsActive(display)) {
        screens = XineramaQueryScreens(display, &num_monitors);

        if (screens && num_monitors > 0) {
            debug_print("Xinerama is active.");

            // Default to the first screen in case we can't find the primary one
            int primary_screen = 0;
            int primary_x = screens[primary_screen].x_org;
            int primary_y = screens[primary_screen].y_org;
            int primary_width = screens[primary_screen].width;
            int primary_height = screens[primary_screen].height;

            // If you want to check for the monitor with the largest area or some other criteria,
            // you could loop through the monitors to choose a different one. However, usually,
            // the first one returned by Xinerama is the "primary" monitor in most setups.

            // Center the window on the primary monitor
            int window_x = primary_x;
            int window_y = primary_y;

            window = XCreateSimpleWindow(display, RootWindow(display, screen), window_x, window_y, primary_width, window_height, 0,
                                         fg_pixel, bg_pixel);
        }
    } else {
        debug_print("Xinerama is not available or inactive.");

        // Default to creating the window on the default screen if Xinerama is not active
        window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, screen_width, window_height, 0,
                                     fg_pixel, bg_pixel);
    }

    XSetWindowAttributes attributes;
    attributes.override_redirect = True;
    attributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
    XChangeWindowAttributes(display, window, CWOverrideRedirect | CWEventMask, &attributes);
    XMapWindow(display, window);

    debug_print("Window created and mapped.");

    gc = XCreateGC(display, window, 0, NULL);

    // Initialize XftColor for input text color
    XRenderColor render_color = hex_to_xrendercolor(INPUT_TEXT_COLOR);
    XftColorAllocValue(display, DefaultVisual(display, screen), DefaultColormap(display, screen), &render_color, &input_xft_color);

    // Initialize XftColor for suggestion text color
    XRenderColor suggestion_render_color = hex_to_xrendercolor(SUGGESTION_TEXT_COLOR);
    XftColorAllocValue(display, DefaultVisual(display, screen), DefaultColormap(display, screen), &suggestion_render_color, &suggestion_xft_color);

    // Create an XftDraw object for drawing the font
    draw = XftDrawCreate(display, window, DefaultVisual(display, screen), DefaultColormap(display, screen));

    // Define the desired font with size included in the font name
    char font_desc[256];
    snprintf(font_desc, sizeof(font_desc), "%s:pixelsize=%d", CUSTOM_FONT, FONT_SIZE);  // Set the desired size
    
    // Load the custom font using XftFontOpenName
    font = XftFontOpenName(display, screen, font_desc);
    if (!font) {
        fprintf(stderr, "Unable to load font: %s\n", font_desc);
        exit(1);
    }

    debug_print("Font loaded and applied.");

    char input[MAX_INPUT_LENGTH] = {0};
    int input_len = 0;
    ResultList result_list = {0};

    ensure_window_focus(display, window);
    debug_print("Window focus ensured.");

    struct timeval last_event_time, current_time;
    gettimeofday(&last_event_time, NULL);

    while (1) {
        gettimeofday(&current_time, NULL);
        long elapsed_time = current_time.tv_sec - last_event_time.tv_sec;

        if (elapsed_time >= TIMEOUT_SECONDS) {
            printf("Exiting due to inactivity timeout.\n");
            break;
        }

        if (XPending(display) > 0) {
            XNextEvent(display, &event);

            if (event.type == Expose) {
                // Redraw the menu with current input and suggestions
                XClearWindow(display, window);
                draw_menu(display, window, gc, input, &result_list, font, draw, &input_xft_color, &suggestion_xft_color);
                debug_print("Expose event triggered.");
            }

            if (event.type == KeyPress) {
                gettimeofday(&last_event_time, NULL);  // Reset the inactivity timer on key press
                KeySym key;
                char buffer[10];
                int len = XLookupString(&event.xkey, buffer, sizeof(buffer), &key, NULL);

if (key == XK_Return) {
    debug_print("Return key pressed.");
    if (input_len > 0) {
        char *binary = NULL;
        char *args = NULL;

        // Split input into binary and arguments using first space as separator
        binary = strtok(input, " ");
        args = strtok(NULL, "");  // Get the rest of the input as arguments

        if (result_list.count > 0) {
            // If there are suggestions, use the first suggestion as the binary
            binary = result_list.items[0];
        }

        if (binary && strlen(binary) > 0) {
            char *cmd = malloc(MAX_INPUT_LENGTH);
            if (args) {
                snprintf(cmd, MAX_INPUT_LENGTH, "%s %s", binary, args);  // Binary + arguments
            } else {
                snprintf(cmd, MAX_INPUT_LENGTH, "%s", binary);           // Only binary (no arguments)
            }

            printf("Executing: %s\n", cmd);
            debug_print("Forking to execute command.");

            pid_t pid = fork();
            if (pid == 0) {
                // In child process: Execute the command via shell
                execlp("/bin/sh", "sh", "-c", cmd, (char *)NULL);
                perror("execlp failed");
                free(cmd);
                exit(1);
            } else if (pid > 0) {
                free(cmd);

                // Clean up and exit
                for (int i = 0; i < result_list.count; i++) {
                    free(result_list.items[i]);
                }
                XFreeGC(display, gc);
                XDestroyWindow(display, window);
                XCloseDisplay(display);
                exit(0);
            } else {
                perror("fork failed");
            }
        }
    }
                } else if (key == XK_Escape) {
                    debug_print("Escape key pressed. Exiting.");
                    for (int i = 0; i < result_list.count; i++) {
                        free(result_list.items[i]);
                    }
                    XFreeGC(display, gc);
                    XDestroyWindow(display, window);
                    XCloseDisplay(display);
                    exit(0);
                } else if (key == XK_Tab) {
                    debug_print("Tab key pressed for autocomplete.");
                    if (result_list.count > 0) {
                        strncpy(input, result_list.items[0], MAX_INPUT_LENGTH - 1);
                        input[MAX_INPUT_LENGTH - 1] = '\0';
                        input_len = strlen(input);
                    }
                } else if (key == XK_Down) {
                    debug_print("Down key pressed to cycle suggestions.");
                    if (result_list.selected < result_list.count - 1) {
                        result_list.selected++;
                    }
                } else if (key == XK_Up) {
                    debug_print("Up key pressed to cycle suggestions.");
                    if (result_list.selected > 0) {
                        result_list.selected--;
                    }
                } else if (key == XK_BackSpace) {
                    if (input_len > 0) {
                        input[--input_len] = '\0';
                        debug_print("Backspace key pressed.");
                    }
                } else if (len > 0 && input_len < MAX_INPUT_LENGTH - 1) {
                    input[input_len++] = buffer[0];
                    input[input_len] = '\0';
                    debug_print("Character entered.");
                }

                char *first_space = strchr(input, ' ');
                if (first_space) {
                    result_list.count = 0;  // No suggestions after the first argument
                }
                if (is_full_match(input, &result_list) || (strchr(input, ' ') && input[input_len - 1] == ' ')) {
                    result_list.count = 0;
                } else {
                    search_binaries(input, &result_list);
                }

                // Use draw_menu for redrawing the menu with updated input
                XClearWindow(display, window);
                draw_menu(display, window, gc, input, &result_list, font, draw, &input_xft_color, &suggestion_xft_color);
            }
        } else {
            ensure_window_focus(display, window);
        }
    }

    // Cleanup
    XftFontClose(display, font);
    XftDrawDestroy(draw);
    XftColorFree(display, DefaultVisual(display, screen), DefaultColormap(display, screen), &input_xft_color);
    XftColorFree(display, DefaultVisual(display, screen), DefaultColormap(display, screen), &suggestion_xft_color);
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
