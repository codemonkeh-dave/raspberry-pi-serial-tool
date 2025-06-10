/*
 * Raspberry Pi Serial Tool
 * 
 * Sends data to specified serial device, either from command line argument or stdin
 * Waits for complete transmission before exiting
 * 
 * Usage: ./serial_send /dev/ttyAMA1 "hello world"
 *        ./serial_send /dev/ttyAMA1 "Line 1\nLine 2"
 *        ./serial_send /dev/ttyAMA1 "\x48\x65\x6c\x6c\x6f"  (hex bytes)
 *        echo "your data" | ./serial_send /dev/ttyAMA1     (stdin fallback)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int serial_fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;
    const char *serial_device;
    
    // Check command line arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <serial_device> [text]\n", argv[0]);
        fprintf(stderr, "Examples:\n");
        fprintf(stderr, "  %s /dev/ttyAMA1 \"hello world\"\n", argv[0]);
        fprintf(stderr, "  %s /dev/ttyAMA1 \"\\x48\\x65\\x6c\\x6c\\x6f\"  # hex bytes\n", argv[0]);
        fprintf(stderr, "  echo \"hello\" | %s /dev/ttyAMA1        # stdin fallback\n", argv[0]);
        return 1;
    }
    
    serial_device = argv[1];
    
    // Open serial device in write-only mode
    serial_fd = open(serial_device, O_WRONLY | O_NOCTTY);
    if (serial_fd < 0) {
        fprintf(stderr, "Error opening %s: %s\n", serial_device, strerror(errno));
        return 1;
    }
    
    // Note: Assumes serial port is already configured with proper baud rate and settings
    
    if (argc >= 3) {
        // Send text from command line argument
        const char *text = argv[2];
        size_t text_len = strlen(text);
        
        bytes_written = write(serial_fd, text, text_len);
        if (bytes_written < 0) {
            fprintf(stderr, "Error writing to serial port: %s\n", strerror(errno));
            close(serial_fd);
            return 1;
        }
        
        if ((size_t)bytes_written != text_len) {
            fprintf(stderr, "Warning: Only wrote %zd of %zu bytes\n", bytes_written, text_len);
        }
    } else {
        // Read data from stdin and send to serial port
        while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
            // Write data to serial port
            bytes_written = write(serial_fd, buffer, bytes_read);
            if (bytes_written < 0) {
                fprintf(stderr, "Error writing to serial port: %s\n", strerror(errno));
                close(serial_fd);
                return 1;
            }
            
            if (bytes_written != bytes_read) {
                fprintf(stderr, "Warning: Only wrote %zd of %zd bytes\n", bytes_written, bytes_read);
            }
        }
        
        if (bytes_read < 0) {
            fprintf(stderr, "Error reading from stdin: %s\n", strerror(errno));
            close(serial_fd);
            return 1;
        }
    }
    
    // CRITICAL: Wait for all data to be transmitted
    // This ensures the serial transmission is complete before the program exits
    if (tcdrain(serial_fd) < 0) {
        fprintf(stderr, "Error waiting for transmission to complete: %s\n", strerror(errno));
        close(serial_fd);
        return 1;
    }
    
    // Close serial port
    close(serial_fd);
    
    return 0;
}