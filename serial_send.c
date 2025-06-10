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

// Function to convert hex digit character to integer
int hex_digit_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Function to parse hex escape sequences like \x41\x42 into actual bytes
size_t parse_hex_string(const char *input, char *output, size_t max_output) {
    size_t input_len = strlen(input);
    size_t output_pos = 0;
    size_t i = 0;
    
    while (i < input_len && output_pos < max_output - 1) {
        if (i + 3 < input_len && input[i] == '\\' && input[i+1] == 'x') {
            // Found \x sequence
            int high = hex_digit_to_int(input[i+2]);
            int low = hex_digit_to_int(input[i+3]);
            
            if (high >= 0 && low >= 0) {
                // Valid hex sequence
                output[output_pos++] = (char)((high << 4) | low);
                i += 4;
            } else {
                // Invalid hex sequence, copy literally
                output[output_pos++] = input[i++];
            }
        } else {
            // Regular character
            output[output_pos++] = input[i++];
        }
    }
    
    return output_pos;
}

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
        // Send text from command line argument, parsing hex escape sequences
        const char *text = argv[2];
        char parsed_buffer[BUFFER_SIZE];
        size_t parsed_len = parse_hex_string(text, parsed_buffer, BUFFER_SIZE);
        
        bytes_written = write(serial_fd, parsed_buffer, parsed_len);
        if (bytes_written < 0) {
            fprintf(stderr, "Error writing to serial port: %s\n", strerror(errno));
            close(serial_fd);
            return 1;
        }
        
        if ((size_t)bytes_written != parsed_len) {
            fprintf(stderr, "Warning: Only wrote %zd of %zu bytes\n", bytes_written, parsed_len);
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