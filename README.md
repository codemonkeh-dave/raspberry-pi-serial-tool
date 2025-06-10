# Raspberry Pi Serial Tool

A C program that sends data to a specified serial device, either from command line argument or stdin, ensuring complete transmission before exiting. This tool is critical for applications requiring guaranteed serial data delivery.

## Features

- Accepts text as command line argument or reads from stdin (supports pipes and redirected input)
- Supports hex byte sequences using escape sequences (\x41\x42\x43)
- Sends data to any specified serial port (e.g., `/dev/ttyAMA1`, `/dev/ttyUSB0`)
- **Waits for complete transmission** using `tcdrain()` - unlike simple shell redirection
- **Assumes serial port is pre-configured** - doesn't modify existing settings
- Lightweight and fast - no configuration overhead
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

### Configure Serial Port

**Important**: This tool assumes your serial port is already configured. You must set up the baud rate and other settings before using this tool.

1. Enable the serial port in Raspberry Pi configuration:
   ```bash
   sudo raspi-config
   ```
   - Navigate to: `Interface Options` → `Serial Port`
   - **Disable** the login shell over serial
   - **Enable** the serial port hardware

2. Configure serial port settings (example for 9600 baud):
   ```bash
   stty -F /dev/ttyAMA1 9600 cs8 -cstopb -parenb
   ```

3. Add your user to the dialout group for serial access:
   ```bash
   sudo usermod -a -G dialout $USER
   ```

4. Reboot to apply changes:
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

### Basic Usage with Text Argument
```bash
./serial_send /dev/ttyAMA1 "Hello Serial"
./serial_send /dev/ttyUSB0 "AT+CGMI"
```

### Send Multiple Lines
```bash
./serial_send /dev/ttyAMA1 "Line 1\nLine 2\nLine 3"
```

### Send Hex Bytes
```bash
./serial_send /dev/ttyAMA1 "\x48\x65\x6c\x6c\x6f"  # "Hello" in hex
./serial_send /dev/ttyAMA1 "\x41\x54\x0D\x0A"      # "AT\r\n"
```

### Using Different Serial Ports
```bash
./serial_send /dev/ttyUSB0 "USB Serial Message"
./serial_send /dev/rfcomm0 "Bluetooth Message"
```

### Stdin Fallback (when no text argument provided)
```bash
echo "Hello Serial" | ./serial_send /dev/ttyAMA1
cat data.txt | ./serial_send /dev/ttyAMA1
printf "Line 1\nLine 2\nLine 3\n" | ./serial_send /dev/ttyAMA1
```

### Interactive Input
```bash
./serial_send /dev/ttyAMA1
# Type your data, press Ctrl+D when finished
```

## Why This Tool?

The standard bash command `echo "data" > /dev/ttyAMA1` does **not** wait for transmission completion. This tool uses `tcdrain()` to ensure all data is physically transmitted before the program exits, which is critical for:

- Reliable serial communication
- Preventing data loss
- Synchronous operations
- Protocol implementations requiring confirmed transmission

## Technical Details

- **Serial Device**: Configurable (e.g., `/dev/ttyAMA1`, `/dev/ttyUSB0`)
- **Configuration**: Uses existing port settings (must be pre-configured)
- **Transmission Guarantee**: Uses `tcdrain()` to wait for complete transmission
- **Performance**: Lightweight - no configuration overhead per operation

## Troubleshooting

### Permission Denied
- Ensure your user is in the `dialout` group: `groups $USER`
- If not, run: `sudo usermod -a -G dialout $USER` and reboot

### Device Not Found
- Verify serial port is enabled in `raspi-config`
- Check if `/dev/ttyAMA1` exists: `ls -l /dev/ttyAMA1`

### Compilation Errors
- Ensure `build-essential` is installed: `sudo apt install build-essential`