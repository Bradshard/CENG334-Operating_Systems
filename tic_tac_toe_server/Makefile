# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Executable name
TARGET = server
DEL1 = output.txt
DEL2 = error.log

# Source files
SRCS = server.c print_output.c
HDRS = game_structs.h print_output.h

# Default target
all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean up build artifacts
clean:
	rm -f $(TARGET)
	rm -f $(DEL1)
	rm -f $(DEL2)

# Convenience run target (optional)
run: $(TARGET)
	./server < input.txt > output.txt 2> error.log

