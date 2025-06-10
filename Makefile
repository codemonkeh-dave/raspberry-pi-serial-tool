# Raspberry Pi Serial Tool Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = serial_send
SOURCE = serial_send.c

# Default target
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

# Clean compiled files
clean:
	rm -f $(TARGET)

# Install to /usr/local/bin (optional)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

# Uninstall from /usr/local/bin
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)

.PHONY: clean install uninstall