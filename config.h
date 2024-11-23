#ifndef CONFIG_H
#define CONFIG_H

// Window and display configurations
#define FONT_SIZE 24                // Font size for input and suggestions
#define TOP_PADDING 8               // Padding above text
#define BOTTOM_PADDING 0            // Padding below text
#define SUGGESTION_OFFSET 10         // Space between suggestions
// Define a larger gap between input text and suggestions
#define INPUT_TO_SUGGESTION_GAP 20 // Increase this to move suggestions further from input

// Uncomment the following line to enable suggestion highlighting
#define ENABLE_HIGHLIGHT

// Maximum limits
#define MAX_INPUT_LENGTH 256         // Maximum length of the input
#define MAX_RESULTS 20               // Maximum number of results to display

// Color configurations (RGB Hex format)
#define INPUT_TEXT_COLOR 0xffffff    // Color for the input text (black)
#define SUGGESTION_TEXT_COLOR 0xffffff // Color for suggestion text (white)
#define SUGGESTION_BG_COLOR 0x005577   // Background color for the selected suggestion (dark gray)
#define WINDOW_BG_COLOR 0x222222      // Background color for the window (light gray)

// Define a custom font as a macro (you can change this value to any font name)
#define CUSTOM_FONT "Hack Nerd Font Regular"

// User-defined timeout (in seconds)
#define TIMEOUT_SECONDS 7             // Time in seconds for user-defined timeout

#endif