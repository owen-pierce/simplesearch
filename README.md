# SimpleSearch

**SimpleSearch** is a lightweight X11 window application designed to simplify the process of searching for and executing binaries in your system's `PATH` variable. With features like tab autocompletion and customizable user-defined variables (like colors), it offers a user-friendly interface for quickly running your favorite applications.

## Features

- **Search for Binaries:** Easily search for all binaries available in your `PATH`.
- **Tab Autocomplete:** Use the tab key to autocomplete binary names, making it faster to find what you need.
- **User-Defined Variables:** Customize your interface with user-defined variables, such as colors, to suit your preferences.
- **Easy Execution:** Select and run the desired binary directly from the interface.

## Requirements

- X11
- gcc
- make

## Installation

To install SimpleSearch, follow these steps:

1. Clone it
   ```bash
   git clone https://github.com/owen-pierce/simplesearch.git
   ```
2. Compile it
   ```bash
   cd simplesearch
   ```
   ```bash
   make
   ```
3. Run it!
   ```bash
   ./simplesearch
   ```
## Add a Shortcut

In GNOME or any other DE you can set a shortcut to launch it like ALT+P

## Inspiration

Heavily inspired by dmenu

## Known Issues

- Writing a valid command, hitting space, then backspace causes the valid command to show on the list
- The window can sometimes lose focus resulting in an unusable application which will autoclose or can be manually killed

## Possible Changes
- In the future, I may implement a multi-tab command cycling feature. This would involve rewriting the current tab autocomplete functionality. Instead of completing the command immediately when the tab key is pressed, it would highlight the first suggestion and allow users to cycle through multiple options by pressing tab repeatedly before selecting the desired command.