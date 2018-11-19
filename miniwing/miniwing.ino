#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> // Hardware-specific library
#include "Adafruit_miniTFTWing.h"
#include "qrcode.h"
#include <Bitcoin.h>

#define NIGHTMODE

#ifdef NIGHTMODE
#define BGCOLOR ST77XX_BLACK
#define MAINCOLOR ST77XX_WHITE
#else
#define BGCOLOR ST77XX_WHITE
#define MAINCOLOR ST77XX_BLACK
#endif
// definitions below are used to choose correct pins for 
// different boards such that the code will work for any

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

// TFT screen
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// Buttons controller
Adafruit_miniTFTWing ss; 

// our account master public key
// n-th receiving public key is derived from master key as hdpubkey.child(0).child(n)
// for change address we use hdpubkey.child(1).child(n)
HDPublicKey hdpubkey("xpub6CVUWasPyNyumoPescCiKVrmuxy6wLtF4bSKiZmSrY4Dp7fdYZ5vVSE9r4ybxDM7RfUhzcfofKEoYvyVvLcfqrG16Vgp84QnMbUZae4bJBE");

bool use_change = false; // receiving address
int n = 0; // current child number we want to display

void setup() {
  // open serial port to print debug information
  Serial.begin(9600);
  if (!ss.begin()) { // if failed to talk to buttons controller
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
  tft.fillScreen(BGCOLOR);
  // init done

}

// prints address to the screen
void showText(char * addr){
  tft.setTextSize(2);
  if(use_change){
    tft.setTextColor(ST77XX_RED);
  }else{
    tft.setTextColor(ST77XX_GREEN);
  }
  tft.fillScreen(BGCOLOR);
  tft.setCursor(0,20);
  tft.println(addr);
}

// displays address as a QR code
void showQR(char * addr){
  tft.fillScreen(BGCOLOR);

  
  // Create the QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(4)];
  qrcode_initText(&qrcode, qrcodeData, 4, 1, addr);

  tft.fillRect(80, 0, qrcode.size*2+10, qrcode.size*2+10, ST77XX_WHITE);
  for (uint8_t y = 0; y < qrcode.size; y++) {
      for (uint8_t x = 0; x < qrcode.size; x++) {
          if(qrcode_getModule(&qrcode, x, y)){
            tft.fillRect(85+2*x, 5+2*y, 2, 2, ST77XX_BLACK);
          }
      }
  }
  tft.setTextSize(1);
  tft.setCursor(0,10);
  if(use_change){
    tft.setTextColor(ST77XX_RED);
    tft.print("Change #");
  }else{
    tft.setTextColor(ST77XX_GREEN);
    tft.print("Recv #");
  }
  tft.println(n);
  tft.println();
  tft.print("m/");
  tft.print(int(use_change));
  tft.print("/");
  tft.println(n);
  tft.println();
  for(int i=0; i<strlen(addr); i++){
    tft.print(addr[i]);
    if((i+1)%12 == 0){
      tft.println();
    }
  }
}

bool QR_mode = true; // show qr or text?
bool new_addr = true; // if pubkey needs to be recalculated
char addr[35]; // char buffer to store address

void loop() {
  // check if we need to recalculate the address (it's slow and not necessary if we switch between QR and text)
  if(new_addr){
    tft.fillScreen(BGCOLOR); // to show that something happens
    // bitcoin logic
    // use_change will be converted to int and we will use it as first index for derivation
    HDPublicKey hdchild = hdpubkey.child(use_change).child(n);
    hdchild.publicKey.address(addr, sizeof(addr)); // populates addr array with actual address
    new_addr = false;
  }
  
  if(!QR_mode){
    showText(addr);
  }else{
    showQR(addr);
  }

  uint32_t buttons = TFTWING_BUTTON_ALL;
  while(buttons == TFTWING_BUTTON_ALL){ // wait for any button
    buttons = ss.readButtons();
    delay(100);
  }
  Serial.println(buttons);
  Serial.println(TFTWING_BUTTON_ALL);
  Serial.println();
  delay(100); // delay to remove gitter
  // toggle QR / text display
  if( (buttons & TFTWING_BUTTON_A) == 0 ){
    QR_mode = !QR_mode; // switch between QR and text
    Serial.println("A");
    return;
  }
  // right btn click
  if( (buttons & TFTWING_BUTTON_RIGHT) == 0 ){
    n += 1;
    new_addr = true;
    Serial.println("Right");
    return;
  }
  // left btn click, only if n > 0
  if( (buttons & TFTWING_BUTTON_LEFT) == 0 && (n > 0)){
    Serial.println("Left");
    n -= 1;
    new_addr = true;
    return;
  }

  if( (buttons & TFTWING_BUTTON_UP) == 0 && (use_change)){
    Serial.println("Up");
    use_change = false;
    new_addr = true;
    return;
  }
  if( (buttons & TFTWING_BUTTON_DOWN) == 0 && (!use_change)){
    Serial.println("Up");
    use_change = true;
    new_addr = true;
    return;
  }
}
