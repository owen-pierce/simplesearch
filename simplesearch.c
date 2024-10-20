#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fontconfig/fontconfig.h>

#define MAX_INPUT_LENGTH 256 // Maximum input length
#define MAX_RESULTS 20 // Maximum results to show
#define SUGGESTION_HEIGHT 30 // Suggestion height
#define SUGGESTION_OFFSET 10 // Suggestion offset
#define MAX_ARGS 10  // Maximum number of arguments to pass

#define TIMEOUT_SECONDS 5  // User-defined timeout (in seconds)

#define FONT_SIZE 32 // Define the font size
#define TOP_PADDING 10
#define BOTTOM_PADDING 10

#define INPUT_TEXT_COLOR 0x000000     // Black text for input
#define SUGGESTION_TEXT_COLOR 0xFFFFFF // White text for suggestions
#define SUGGESTION_BG_COLOR 0x808080   // Dark gray background for suggestions
#define WINDOW_BG_COLOR 0x808080      // Light gray background for the window

typedef struct {
    int count;
    int selected;
    char *items[MAX_RESULTS];
} ResultList;

// Function to get paths from $PATH
char** get_path_dirs(int *count) {
    char *path_env = getenv("PATH");
    char *path = strdup(path_env);
    char *token;
    char **dirs = NULL;
    *count = 0;

    token = strtok(path, ":");
    while (token != NULL) {
        dirs = realloc(dirs, sizeof(char*) * (++(*count)));
        dirs[*count - 1] = strdup(token);
        token = strtok(NULL, ":");
    }
    free(path);
    return dirs;
}

// Check if file is an executable
int is_executable(const char *filepath) {
    return access(filepath, X_OK) == 0;
}

int is_full_match(const char *input, ResultList *result_list) {
    for (int i = 0; i < result_list->count; i++) {
        if (strcmp(input, result_list->items[i]) == 0) {
            return 1;  // Found a full match
        }
    }
    return 0;  // No full match found
}


// Search for binary in directories
int search_binaries(const char *query, ResultList *result_list) {
    int dir_count = 0;
    char **dirs = get_path_dirs(&dir_count);
    struct dirent *entry;
    char filepath[1024];

    result_list->count = 0;
    result_list->selected = 0;

    for (int i = 0; i < dir_count; i++) {
        DIR *dir = opendir(dirs[i]);
        if (dir) {
            while ((entry = readdir(dir)) != NULL) {
                if (strncmp(entry->d_name, query, strlen(query)) == 0) {
                    snprintf(filepath, sizeof(filepath), "%s/%s", dirs[i], entry->d_name);
                    if (is_executable(filepath)) {
                        int is_duplicate = 0;
                        for (int j = 0; j < result_list->count; j++) {
                            if (strcmp(result_list->items[j], entry->d_name) == 0) {
                                is_duplicate = 1;
                                break;
                            }
                        }
                        if (!is_duplicate && result_list->count < MAX_RESULTS) {
                            result_list->items[result_list->count++] = strdup(entry->d_name);
                        }
                    }
                }
            }
            closedir(dir);
        }
    }

    for (int i = 0; i < dir_count; i++) {
        free(dirs[i]);
    }
    free(dirs);
    return result_list->count;
}

void draw_menu(Display *display, Window window, GC gc, char *input, ResultList *result_list, XFontStruct *font) {
    XClearWindow(display, window);

    XSetWindowBackground(display, window, WINDOW_BG_COLOR);
    XClearWindow(display, window);

    // Define padding values
    int top_padding = 10;      // Padding above the input
    int bottom_padding = 10;   // Padding below the input
    int line_height = font->ascent + font->descent + top_padding + bottom_padding; // Total line height with padding

    int input_len = strlen(input);
    int input_width = XTextWidth(font, input, input_len);  // Get width of the input text

    // Set foreground color for input text
    XSetForeground(display, gc, INPUT_TEXT_COLOR);

    // Draw the input text with padding
    XDrawString(display, window, gc, 10, top_padding + font->ascent, input, input_len); // Adjust y position

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
                XFillRectangle(display, window, gc, x_pos - SUGGESTION_OFFSET, top_padding + font->ascent - bottom_padding, suggestion_width + SUGGESTION_OFFSET * 2, line_height); // Adjusted y position for selection
                //XSetForeground(display, gc, BlackPixel(display, 0));  // Switch back to black for the text
            }
            XSetForeground(display, gc, SUGGESTION_TEXT_COLOR);

            // Draw the suggestion text
            XDrawString(display, window, gc, x_pos, top_padding + font->ascent, result_list->items[i], suggestion_len);

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

    int window_height = FONT_SIZE + TOP_PADDING + BOTTOM_PADDING; // Calculate height based on font size and padding
    window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, screen_width, window_height, 0,
                                WhitePixel(display, screen), BlackPixel(display, screen));

    // Set window attributes for borderless
    XSetWindowAttributes attributes;
    attributes.override_redirect = True; // Make the window borderless
    attributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    XChangeWindowAttributes(display, window, CWOverrideRedirect | CWEventMask, &attributes);

    XMapWindow(display, window);
    
    // Create a Graphics Context (GC)
    gc = XCreateGC(display, window, 0, NULL);
    char font_name[256];
    snprintf(font_name, sizeof(font_name), "-adobe-courier-medium-r-normal--%d-0-0-0-p-0-iso8859-1", FONT_SIZE);
    // Load the font
    font = XLoadQueryFont(display, font_name);
    if (!font) {
        fprintf(stderr, "Unable to load font: %s. Using default font.\n", font_name);
        font = XLoadQueryFont(display, "fixed");
    }
    XSetFont(display, gc, font->fid);
    XSetForeground(display, gc, BlackPixel(display, screen));

    XSetWindowBackground(display, window, 0xAAAAAA);
    XClearWindow(display, window);

    char input[MAX_INPUT_LENGTH] = {0};
    int input_len = 0;
    ResultList result_list = {0};

    printf("Window created, width: %d, height: %d\n", screen_width, SUGGESTION_HEIGHT + 30);
    fflush(stdout);

    // Ensure the window gets keyboard focus after mapping
    ensure_window_focus(display, window);

    struct timeval last_event_time, current_time;
    gettimeofday(&last_event_time, NULL);  // Initialize the timer

    // Main event loop
    while (1) {
        // Set timeout for input (in seconds)
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
            // Extract the binary (first word) and arguments (rest of the input)
            char *binary = strtok(input, " ");  // First word is the binary
            char *args = strtok(NULL, "");      // Everything after the binary

            if (binary && (result_list.count > 0 || strlen(binary) > 0)) {
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
                    // Parent process waits for the child to complete
                    //wait(NULL);
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
            // Ensure the window retains focus in case it gets lost
            ensure_window_focus(display, window);
        }
    }

    // Cleanup
    for (int i = 0; i < result_list.count; i++) {
        free(result_list.items[i]);
    }
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}