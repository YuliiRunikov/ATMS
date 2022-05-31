#define ILI9341_DRIVER

// https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
// http://medesign.seas.upenn.edu/index.php/Guides/ESP32-pins
// https://github.com/espressif/arduino-esp32
// https://arduinomaster.ru/platy-arduino/esp32-arduino-raspinovka-arduino-ide/
// https://randomnerdtutorials.com/power-saving-latching-circuit/
// https://github.com/netzbasteln/MLX90640-Thermocam
// https://openhomeautomation.net/esp8266-battery
// https://circuits4you.com/2018/12/31/esp32-pwm-example/
//https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/


#define LED 17
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5 // Chip select control pin
#define TFT_DC    16 // Data Command control pin
//#define TFT_RST   15  // Reset pin (could connect to RST pin)
#define TOUCH_CS 27     // Chip select pin (T_CS) of touch screen
#define T_IRQ 39
#define SD_CS 15
//SD_Mosi 13
//SD_Miso 26
//SD_csk 14

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

// Comment out the #define below to stop the SPIFFS filing system and smooth font code being loaded
// this will save ~20kbytes of FLASH
#define SMOOTH_FONT

// With an ILI9341 display 40MHz works OK, 80MHz sometimes fails


#define SPI_FREQUENCY  40000000// Actually sets it to 26.67MHz = 80/3
#define SPI_TOUCH_FREQUENCY  2500000

//#define USE_HSPI_PORT



//вставить под воид сетап что б заработал лед ->
//-> pinMode(LED, OUTPUT);
//-> digitalWrite(LED, HIGH); 
//
///uint16_t calData[5] = { 441, 3324, 301, 3406, 7 };
  //tft.setTouch(calData);    тач  

