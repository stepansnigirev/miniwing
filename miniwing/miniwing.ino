#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> // Hardware-specific library
#include "Adafruit_miniTFTWing.h"
#include "qrcode.h"

Adafruit_miniTFTWing ss;

#define TFT_RST    -1    // we use the seesaw for resetting to save a pin

#ifdef ESP8266
   #define TFT_CS   2
   #define TFT_DC   16
#endif
#ifdef ESP32
   #define TFT_CS   14
   #define TFT_DC   32
#endif
#ifdef TEENSYDUINO
   #define TFT_CS   8
   #define TFT_DC   3
#endif
#ifdef ARDUINO_STM32_FEATHER
   #define TFT_CS   PC5
   #define TFT_DC   PC7
#endif
#ifdef ARDUINO_NRF52_FEATHER /* BSP 0.6.5 and higher! */
   #define TFT_CS   27
   #define TFT_DC   30
#endif

// Anything else!
#if defined (__AVR_ATmega32U4__) || defined(ARDUINO_SAMD_FEATHER_M0) || defined (__AVR_ATmega328P__) || defined(ARDUINO_SAMD_ZERO) || defined(__SAMD51__) || defined(__SAM3X8E__)
   #define TFT_CS   5
   #define TFT_DC   6
#endif

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

word RGBColor( byte R, byte G, byte B){
  return ( ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3) );
}

void setup() {
  Serial.begin(9600);
  if (!ss.begin()) {
    Serial.println("seesaw couldn't be found!");
    while(1);
  }
  Serial.print("seesaw started!\tVersion: ");
  Serial.println(ss.getVersion(), HEX);

  ss.tftReset();   // reset the display
  
  ss.setBacklight(1);  // turn off the backlight

  tft.initR(INITR_MINI160x80);   // initialize a ST7735S chip, mini display
  Serial.println("TFT initialized");

  tft.setRotation(3);
  tft.fillScreen(ST77XX_WHITE);
  // init done

  tft.setTextSize(2);
  tft.setTextColor(0xFFFA);
  tft.setCursor(0,21);
  tft.println(" 1EuLLE5Q7mx\n HAwP5y9kHM1\n JAUjRzQYG22");
  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(0,20);
  tft.println(" 3LyGX8XECdm\n LHBxFyvsFLx\n koBhvFNu7j4N");
//  tft.println(" 1EuLLE5Q7mx\n HAwP5y9kHM1\n JAUjRzQYG22");

  uint32_t buttons = TFTWING_BUTTON_ALL;
  while((buttons & TFTWING_BUTTON_A) > 0){
    buttons = ss.readButtons();
    delay(100);
    Serial.println((buttons & TFTWING_BUTTON_SELECT));
  }

  tft.fillScreen(ST77XX_WHITE);
    
  // Create the QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(4)];
  qrcode_initText(&qrcode, qrcodeData, 4, 1, "﻿3LyGX8XECdmLHBxFyvsFLxkoBhvFNu7j4N");

  for (uint8_t y = 0; y < qrcode.size; y++) {
      for (uint8_t x = 0; x < qrcode.size; x++) {
          if(qrcode_getModule(&qrcode, x, y)){
            tft.fillRect(50+2*x, 1+2*y, 2, 2, ST77XX_BLACK);
          }
      }
  }
}

void loop() {

}
