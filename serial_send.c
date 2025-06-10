/*
 * Raspberry Pi Serial Tool
 * 
 * Reads data from stdin and sends it to specified serial device
 * Waits for complete transmission before exiting
 * 
 * Usage: echo "your data" | ./serial_send /dev/ttyAMA1
 *        cat file.txt | ./serial_send /dev/ttyUSB0
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
        fprintf(stderr, "Usage: %s <serial_device>\n", argv[0]);
        fprintf(stderr, "Example: echo \"hello\" | %s /dev/ttyAMA1\n", argv[0]);
        fprintf(stderr, "         cat file.txt | %s /dev/ttyUSB0\n", argv[0]);
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