/*
 * Raspberry Pi Serial Tool
 * 
 * Reads data from stdin and sends it to serial device /dev/ttyAMA1
 * Waits for complete transmission before exiting
 * 
 * Usage: echo "your data" | ./serial_send
 *        cat file.txt | ./serial_send
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>

#define SERIAL_DEVICE "/dev/ttyAMA1"
#define BAUD_RATE B9600
#define BUFFER_SIZE 1024

// Fallback definition for CRTSCTS if not available
#ifndef CRTSCTS
#define CRTSCTS 020000000000
#endif

int main() {
    int serial_fd;
    struct termios tty;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;
    
    // Open serial device
    serial_fd = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY);
    if (serial_fd < 0) {
        fprintf(stderr, "Error opening %s: %s\n", SERIAL_DEVICE, strerror(errno));
        return 1;
    }
    
    // Get current serial port configuration
    if (tcgetattr(serial_fd, &tty) != 0) {
        fprintf(stderr, "Error getting serial port attributes: %s\n", strerror(errno));
        close(serial_fd);
        return 1;
    }
    
    // Configure serial port settings
    // Set baud rate
    cfsetospeed(&tty, BAUD_RATE);
    cfsetispeed(&tty, BAUD_RATE);
    
    // 8 data bits, no parity, 1 stop bit (8N1)
    tty.c_cflag &= ~PARENB;        // No parity
    tty.c_cflag &= ~CSTOPB;        // 1 stop bit
    tty.c_cflag &= ~CSIZE;         // Clear data size bits
    tty.c_cflag |= CS8;            // 8 data bits
    tty.c_cflag &= ~CRTSCTS;       // No hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem control lines
    
    // Raw input mode
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;
    
    // Apply the configuration
    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error setting serial port attributes: %s\n", strerror(errno));
        close(serial_fd);
        return 1;
    }
    
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