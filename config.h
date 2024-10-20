#ifndef CONFIG_H
#define CONFIG_H

// Window and display configurations
#define FONT_SIZE 32                 // Font size for input and suggestions
#define TOP_PADDING 10               // Padding above text
#define BOTTOM_PADDING 10            // Padding below text
#define SUGGESTION_OFFSET 10         // Space between suggestions

// Maximum limits
#define MAX_INPUT_LENGTH 256         // Maximum length of the input
#define MAX_RESULTS 20               // Maximum number of results to display

// Color configurations (RGB Hex format)
#define INPUT_TEXT_COLOR 0x000000     // Color for the input text (black)
#define SUGGESTION_TEXT_COLOR 0xFFFFFF // Color for suggestion text (white)
#define SUGGESTION_BG_COLOR 0x808080   // Background color for the selected suggestion (dark gray)
#define WINDOW_BG_COLOR 0x808080       // Background color for the window (light gray)

// User-defined timeout (in seconds)
#define TIMEOUT_SECONDS 5             // Time in seconds for user-defined timeout

#endif // CONFIG_H