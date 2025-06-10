# Raspberry Pi Serial Tool

A C program that reads data from stdin and sends it to the serial device `/dev/ttyAMA1`, ensuring complete transmission before exiting. This tool is critical for applications requiring guaranteed serial data delivery.

## Features

- Reads data from stdin (supports pipes and redirected input)
- Sends data to `/dev/ttyAMA1` serial port
- **Waits for complete transmission** using `ioctl(TIOCDRAIN)` - unlike simple shell redirection
- Configures serial port with 9600 baud, 8N1 (8 data bits, no parity, 1 stop bit)
- Comprehensive error handling

## Setup

### Installing Development Tools on Raspberry Pi 4

1. Update your system:
   ```bash
   sudo apt update
   sudo apt upgrade -y
   ```

2. Install GCC and build tools:
   ```bash
   sudo apt install build-essential -y
   ```

3. Verify installation:
   ```bash
   gcc --version
   make --version
   ```

### Enable Serial Port

1. Enable the serial port in Raspberry Pi configuration:
   ```bash
   sudo raspi-config
   ```
   - Navigate to: `Interface Options` → `Serial Port`
   - **Disable** the login shell over serial
   - **Enable** the serial port hardware

2. Add your user to the dialout group for serial access:
   ```bash
   sudo usermod -a -G dialout $USER
   ```

3. Reboot to apply changes:
   ```bash
   sudo reboot
   ```

## Compilation

### Quick Compile
```bash
gcc -o serial_send serial_send.c
```

### With Makefile
```bash
make
```

## Usage

### Basic Usage
```bash
echo "Hello Serial" | ./serial_send
```

### Send File Contents
```bash
cat data.txt | ./serial_send
```

### Send Multiple Lines
```bash
printf "Line 1\nLine 2\nLine 3\n" | ./serial_send
```

### Interactive Input
```bash
./serial_send
# Type your data, press Ctrl+D when finished
```

## Why This Tool?

The standard bash command `echo "data" > /dev/ttyAMA1` does **not** wait for transmission completion. This tool uses `ioctl(TIOCDRAIN)` to ensure all data is physically transmitted before the program exits, which is critical for:

- Reliable serial communication
- Preventing data loss
- Synchronous operations
- Protocol implementations requiring confirmed transmission

## Technical Details

- **Serial Device**: `/dev/ttyAMA1` (Raspberry Pi UART)
- **Baud Rate**: 9600 bps
- **Data Format**: 8N1 (8 data bits, no parity, 1 stop bit)
- **Flow Control**: None
- **Transmission Guarantee**: Uses `ioctl(TIOCDRAIN)` to wait for complete transmission

## Troubleshooting

### Permission Denied
- Ensure your user is in the `dialout` group: `groups $USER`
- If not, run: `sudo usermod -a -G dialout $USER` and reboot

### Device Not Found
- Verify serial port is enabled in `raspi-config`
- Check if `/dev/ttyAMA1` exists: `ls -l /dev/ttyAMA1`

### Compilation Errors
- Ensure `build-essential` is installed: `sudo apt install build-essential`