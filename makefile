# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2

# X11 library
LIBS = -lX11

# Source files
SRCS = main.c menu.c path_utils.c

# Object files
OBJS = $(SRCS:.c=.o)

# Output executable
TARGET = simplesearch

# Default rule to build the program
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up compiled files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
