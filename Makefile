# Makefile for pstree program

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic -g

# Target executable
TARGET = pstree

# Default target
all:
	$(CC) $(CFLAGS) pstree.c -o $(TARGET)

# Clean up build artifacts
clean:
	rm -f $(TARGET)

# Help message
help:
	@echo "Makefile for pstree program"
	@echo "Usage:"
	@echo "  make          Build the program"
	@echo "  make clean    Remove build artifacts"
	@echo "  make help     Display this help message"

.PHONY: all clean help
