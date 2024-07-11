# Makefile for compiling mon.c into mon

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -muintr -O3 -g

# Target executable
TARGET = mon

# Source file
SRC = mon.c

# Default target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

# Clean up build files
clean:
	rm -f $(TARGET)
