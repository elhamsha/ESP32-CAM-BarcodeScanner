#include "esp_camera.h"
#include <WiFi.h>
#include "../lib/zxing/core/ReadBarcode.h"        // Include ZXing library
#include "camera_pins.h"
#include "soc/soc.h"            // Disable brownout problems
#include "soc/rtc_cntl_reg.h"   // Disable brownout problems
#include "driver/rtc_io.h"

esp_err_t cameraInitError;

#define FLASH_LED_PIN 4     // GPIO pin for the flash LED
#define NUM_FRAMES 5        // Set the number of frames to capture
#define PWM_FREQUENCY 5000  // Set PWM (Pulse Width Modulation) frequency
#define PWM_RESOLUTION 8    // Set PWM resolution (8-bit)
#define PWM_DUTY_CYCLE 128  // Set moderate brightness (50% of 255)

// Function to convert pixel format to a string for better readability
const char* pixel_format_to_string(pixformat_t format) {
    switch (format) {
        case PIXFORMAT_JPEG: return "JPEG";
        case PIXFORMAT_RGB565: return "RGB565";
        case PIXFORMAT_YUV422: return "YUV422";
        case PIXFORMAT_GRAYSCALE: return "GRAYSCALE";
        case PIXFORMAT_RAW: return "RAW";
        default: return "UNKNOWN";
    }
}

void printSensorValues(sensor_t *s) {
    if (s) {
        Serial.println("\nSensor Information:");
        Serial.printf("  - ID: %d\n", s->id.PID);
        //Serial.printf("  - Resolution: %dx%d\n", s->pixformat == PIXFORMAT_GRAYSCALE ? 0 : s->status.width, s->status.height);
        //Serial.printf("  - Exposure: %d\n", s->status.exposure);
        Serial.printf("  - Pixel Format: %s\n", pixel_format_to_string(s->pixformat));
        Serial.printf("  - Frame Size: %d\n", s->status.framesize);
        Serial.printf("  - Quality: %d\n", s->status.quality);
        Serial.printf("  - gainceiling: %d\n", s->status.gainceiling);
        Serial.printf("  - agc_gain: %d\n", s->status.agc_gain);
        Serial.printf("  - awb_gain: %d\n", s->status.awb_gain);
        Serial.printf("  - raw_gma: %d\n", s->status.raw_gma);
        Serial.printf("  - vflip: %d\n", s->status.vflip);
        Serial.printf("  - Brightness: %d\n", s->status.brightness);
        Serial.printf("  - Contrast: %d\n", s->status.contrast);
        Serial.printf("  - Saturation: %d\n", s->status.saturation);
        Serial.printf("  - Special Effects: %d\n", s->status.special_effect);
        Serial.printf("  - xclk_freq_hz: %d\n", s->xclk_freq_hz);
        Serial.printf("  - ae_level: %d\n", s->status.ae_level);
        Serial.printf("  - wb_mode: %d\n", s->status.wb_mode);
        Serial.printf("  - dcw: %d\n", s->status.dcw);
        Serial.printf("  - aec: %d\n", s->status.aec);
        Serial.printf("  - aec2: %d\n", s->status.aec2);
        Serial.printf("  - aec_value: %d\n", s->status.aec_value);
        Serial.printf("  - agc: %d\n", s->status.agc);
        Serial.printf("  - awb: %d\n", s->status.awb);
        Serial.printf("  - binning: %d\n", s->status.binning);
        Serial.printf("  - bpc: %d\n", s->status.bpc);
        Serial.printf("  - colorbar: %d\n", s->status.colorbar);
        Serial.printf("  - denoise: %d\n", s->status.denoise);
        Serial.printf("  - hmirror: %d\n", s->status.hmirror);
        Serial.printf("  - lenc: %d\n", s->status.lenc);
        Serial.printf("  - scale: %d\n", s->status.scale);
        Serial.printf("  - wpc: %d\n", s->status.wpc);
        // Add more fields as needed
    } else {
        Serial.println("Failed to get sensor");
    }
}

void setup() {
    Serial.begin(9600);

    ledcSetup(0, PWM_FREQUENCY, PWM_RESOLUTION); // Set up PWM channel
    ledcAttachPin(FLASH_LED_PIN, 0); // Attach the LED pin to PWM channel

    // Camera configuration
    camera_config_t config = {};
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_GRAYSCALE; 
    config.frame_size = FRAMESIZE_HVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    // Initialize camera
    esp_err_t cameraInitError = esp_camera_init(&config);
    if (cameraInitError != ESP_OK) {
        Serial.println("Camera init failed");
        return;
    }

    // Get the sensor configuration
    sensor_t *s = esp_camera_sensor_get();
    // printSensorValues(s); // Print sensor values

    // Turn on the flashlight with moderate brightness
    // ledcWrite(0, PWM_DUTY_CYCLE); // Write the duty cycle for PWM
    delay(100); // Allow some time for the LED to adjust
}

std::string decodeBarcode(camera_fb_t *fb) {
    std::string barcode_data = "--> Unknown"; 
    // Check if the framebuffer is valid
    if (!fb || fb->buf == NULL) {
        log_i("Failed to get frame buffer");
        return barcode_data;
    }
    if (fb->width <= 0 || fb->height <= 0) {
        log_i("Invalid framebuffer dimensions.");
        return barcode_data; 
    }

    // Calculate required size for grayscale image
    size_t requiredSize = fb->width * fb->height;

    // Check if allocated size is sufficient
    // Assuming fb->buf was allocated using malloc
    // Note: You need to track the allocated size manually when allocating
    size_t allocatedSize = requiredSize; // This should be the size you allocated

    if (allocatedSize < requiredSize) {
        log_i("Allocated size is insufficient for the framebuffer.");
        return barcode_data; // Handle the error
    }

    // Assuming fb->buf contains RGB data
    // uint8_t* grayBuffer = convertToGrayscale(fb->buf, fb->width, fb->height);

    // Create the ZXing image view with the grayscale data
    auto image = ZXing::ImageView(fb->buf, fb->width, fb->height, ZXing::ImageFormat::Lum);

    // Create a BarcodeFormats object and specify the formats you want to support
    // ZXing::BarcodeFormats specialFormats = 
    //     ZXing::BarcodeFormat::DataMatrix |
    //     ZXing::BarcodeFormat::EAN13 |
    //     ZXing::BarcodeFormat::Code128;
    
	  ZXing::BarcodeFormats specialFormats = ZXing::BarcodeFormat::Any;
    // These optimized settings are well-suited for a low-memory and low-processing-effort scenario
    // ZXing::ReaderOptions options;
    auto options = ZXing::DecodeHints();
    options.setFormats(specialFormats);
    options.setMaxNumberOfSymbols(1);   // Limit to one symbol for efficiency
    options.setTryHarder(true);         // Set to false to reduce processing effort
    // options.setTryRotate(true);      
    // options.setReturnErrors(true);   // Return errors for better handling
    // options.setTryInvert(false);      

    try {
        auto barcodesVector = ZXing::ReadBarcodes(image, options);
        log_i("Frame %d - No of barcodes: %zu\n", frameNo, barcodesVector.size());
        if (barcodesVector.size() > 0)
            barcode_data = "";

        // Print out the detected barcodes
        for (const auto& barcode : barcodesVector) {
            std::string errorMessages;
            if (barcode.error()) {
                errorMessages += "Barcode Errors: " + barcode.error().msg() + "\n"; // Accumulate error messages
                log_i(errorMessages.c_str());
                return errorMessages;
            } else {
                barcode_data += barcode.text() 
                      + ", " + ZXing::ToString(barcode.format()).c_str() + "\n"; // Combine all barcode texts with a newline
                log_i("Detected barcode: %s, %d\n", barcode_data
                      , static_cast<int>(barcode.format()));
            }
        }
    } catch (const std::exception& e) {
        log_i("Error reading barcodes: %s\n", e.what());
    }
//    free(grayBuffer);   // Clean up
    return barcode_data; 
}

void grayscaleToBW(uint8_t *buffer, size_t len, uint8_t threshold) {
  for (size_t i = 0; i < len; i++) {
    buffer[i] = buffer[i] > threshold ? 255 : 0; // Convert to 0 (black) or 255 (white)
  }
}

void loop() {
    std::string theBarcode;
    if (cameraInitError != ESP_OK) {
        return;
    }

    // Capture frames for barcode detection
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb) {
        // Convert grayscale to black and white
        grayscaleToBW(fb->buf, fb->len, 128); // Use 128 as a threshold

        // Call the original function
        theBarcode = decodeBarcode(fb);;
        // Don't forget to release the camera frame
        esp_camera_fb_return(fb); // Return frame buffer to the camera
        fb = NULL;

        // Valid barcode found, break out of both loops
        printf("the Barcode: %s \n", theBarcode.c_str());
    }
    // delay(1000); // Adjust as necessary
}