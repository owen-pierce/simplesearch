#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "path_utils.h"
#include "menu.h"
#include "config.h"

int main() {
    Display *display;
    Window window;
    XEvent event;
    int screen;
    GC gc;
    XFontStruct *font;

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);
    int window_height = FONT_SIZE + TOP_PADDING + BOTTOM_PADDING;

    window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, screen_width, window_height, 0,
                                 WhitePixel(display, screen), BlackPixel(display, screen));

    XSetWindowAttributes attributes;
    attributes.override_redirect = True;
    attributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
    XChangeWindowAttributes(display, window, CWOverrideRedirect | CWEventMask, &attributes);
    XMapWindow(display, window);
    
    gc = XCreateGC(display, window, 0, NULL);
    char font_name[256];
    snprintf(font_name, sizeof(font_name), "-adobe-courier-medium-r-normal--%d-0-0-0-p-0-iso8859-1", FONT_SIZE);
    font = XLoadQueryFont(display, font_name);
    if (!font) {
        fprintf(stderr, "Unable to load font: %s. Using default font.\n", font_name);
        font = XLoadQueryFont(display, "fixed");
    }
    XSetFont(display, gc, font->fid);

    char input[MAX_INPUT_LENGTH] = {0};
    int input_len = 0;
    ResultList result_list = {0};

    ensure_window_focus(display, window);

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
                draw_menu(display, window, gc, input, &result_list, font);
            }

if (event.type == KeyPress) {
    gettimeofday(&last_event_time, NULL);  // Reset the inactivity timer on key press
    KeySym key;
    char buffer[10];
    int len = XLookupString(&event.xkey, buffer, sizeof(buffer), &key, NULL);

if (key == XK_Return) {
    if (input_len > 0) {
        char *binary = NULL;
        char *args = strtok(NULL, "");  // Everything after the binary

        if (result_list.count > 0) {
            // If there are suggestions, use the first suggestion as the binary
            binary = result_list.items[0];
        } else {
            // Use the user's input if no suggestions are available
            binary = strtok(input, " ");  
        }

        if (binary && strlen(binary) > 0) {
            // Prepare the command with the selected binary and arguments
            char *cmd = malloc(MAX_INPUT_LENGTH);
            if (args) {
                snprintf(cmd, MAX_INPUT_LENGTH, "%s %s", binary, args);  // Binary + arguments
            } else {
                snprintf(cmd, MAX_INPUT_LENGTH, "%s", binary);           // Only binary (no arguments)
            }

            printf("Executing: %s\n", cmd);

            // Fork to execute the command
            pid_t pid = fork();
            if (pid == 0) {
                // In child process: Execute the command via shell
                execlp("/bin/sh", "sh", "-c", cmd, (char *)NULL);
                perror("execlp failed"); // If exec fails
                free(cmd);               // Free cmd in the child process
                exit(1);                 // Exit if exec fails
            } else if (pid > 0) {
                // Parent process: wait for the child to exit and clean up
                free(cmd);  // Free command string after use

                // Clean up resources and exit after execution
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
        // Exit the program when Esc is pressed
        for (int i = 0; i < result_list.count; i++) {
            free(result_list.items[i]);
        }
        XFreeGC(display, gc);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
        exit(0);
    } else if (key == XK_Tab) {
        // Handle Tab key for autocomplete
        if (result_list.count > 0) {
            // Autocomplete with the first suggestion
            strncpy(input, result_list.items[0], MAX_INPUT_LENGTH - 1);
            input[MAX_INPUT_LENGTH - 1] = '\0';  // Ensure null-termination
            input_len = strlen(input);  // Update the length of the input
        }
    } else if (key == XK_Down) {
        if (result_list.selected < result_list.count - 1) {
            result_list.selected++;
        }
    } else if (key == XK_Up) {
        if (result_list.selected > 0) {
            result_list.selected--;
        }
    } else if (key == XK_BackSpace) {
        if (input_len > 0) {
            input[--input_len] = '\0';
        }
    } else if (len > 0 && input_len < MAX_INPUT_LENGTH - 1) {
        input[input_len++] = buffer[0];
        input[input_len] = '\0';
    }

    // Detect if the user has started typing the second argument
    char *first_space = strchr(input, ' ');
    if (first_space) {
        // Hide the suggestions after the first argument is fully entered
        result_list.count = 0;  // No suggestions after the first argument
    } 
    if (is_full_match(input, &result_list) || (strchr(input, ' ') && input[input_len - 1] == ' ')) {
        result_list.count = 0; // Hide suggestions if there's a full match or ends with space
    } else {
        // Search for binaries based on the input if no second argument is detected
        search_binaries(input, &result_list);
    }

    // Redraw the menu
    draw_menu(display, window, gc, input, &result_list, font);
}
        } else {
            ensure_window_focus(display, window);
        }
    }

    for (int i = 0; i < result_list.count; i++) {
        free(result_list.items[i]);
    }
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
