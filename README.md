# ESP32-CAM Barcode Scanner

A lightweight and efficient barcode scanner for the ESP32-CAM, utilizing the ZXing library for fast and accurate barcode recognition. This project captures images, processes barcodes, and extracts data, making it ideal for IoT and embedded applications. Optimized for performance with WiFi support for data transmission.


## Features
- 📷 **Image capture** using ESP32-CAM
- 🔍 **Barcode & QR code recognition** with ZXing
- ⚡ **Optimized performance** for ESP32
- 📡 **WiFi support** for data transmission
- 🔧 **Easy integration** into IoT systems

## Hardware Requirements
- **ESP32-CAM**  
- FTDI Programmer (for flashing)  
- Power supply (5V via USB or battery)

## Installation

1. **Clone the Repository**
   ```sh
   git clone https://github.com/yourusername/ESP32-CAM-BarcodeScanner.git
   cd ESP32-CAM-BarcodeScanner
   ```

2. **Install PlatformIO If you haven't installed PlatformIO, do so:**
   ```sh
   pip install platformio
   ```

3. **Compile and Upload**
   - Open the project in PlatformIO.
   - Select the correct board (esp32cam).
   - Build and upload the firmware.

## Usage
   - The ESP32-CAM captures an image.
   - ZXing processes and detects barcodes.
   - Detected barcode data is sent via WiFi or Serial.
   - The result is displayed on a web server or sent to an endpoint.

## Configuration
   Modify platformio.ini to adjust settings like WiFi credentials, debug levels, and optimizations.

## Example Output
   ```yaml
Detected Barcode: 1234567890123
Type: EAN-13
   ```

## Dependencies
- ZXing for barcode detection
- ESP32 Arduino Framework
- PlatformIO

## License
This project is licensed under the MIT License.

Contributions are welcome! Feel free to open issues or submit pull requests. 🚀
