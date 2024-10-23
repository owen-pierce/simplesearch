# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2
CFLAGS += -I/usr/include/freetype2
LDFLAGS += -lX11 -lXft -lfontconfig -lfreetype -lXinerama

# X11 library
LIBS = -lX11 -lXft

# Source files
SRCS = main.c draw_utils.c path_utils.c

# Object files
OBJS = $(SRCS:.c=.o)

# Output executable
TARGET = simplesearch

# Default rule to build the program
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up compiled files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
