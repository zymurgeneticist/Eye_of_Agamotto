/***************************************************
 * This is an example sketch for the Adafruit 2.2" SPI display.
 * This library works with the Adafruit 2.2" TFT Breakout w/SD card
 * ----> http://www.adafruit.com/products/1480
 * 
 * Check out the links above for our tutorials and wiring diagrams
 * These displays use SPI to communicate, 4 or 5 pins are required to
 * interface (RST is optional)
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 * 
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 * MIT license, all text above must be included in any redistribution
 > The libraries used in this project developed by Adafruit and
 > optimized by Xark here:
 > ----> https://hackaday.io/project/6038-pdqgfx-optimzed-avr-lcd-graphics
 > The majority of the "animation" (scrolling) was borrowed from code by KurtE here:
 > ----> https://forum.pjrc.com/threads/26305-Highly-optimized-ILI9341-(320x240-TFT-color-display)-library/page3
 > Thanks!
 > The rest was cobbled together, I'm sure once I get better at 
 > coding in Arduino I'll release a cleaner version.
 > Take a look at the final build and get the 3D print files here:
 > ----> http://www.instructables.com/id/Eye-of-Agamotto/
 ****************************************************/

#include "SPI.h"
#include "SD.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

// These are the pins used for the UNO and Trinket 5V Pro, you cannot use pin 2 or 7
#define _sclk 13
#define _miso 12
#define _mosi 11
#define _cs 10
#define _rst 8
#define _dc 9
#define SD_CS 4
#define _bl 6
#define hallPin1 15
#define hallPin2 16

// Use hardware SPI
Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);
int iScrollStart = 80; //new replacement code that starts scrolling at 0
int iScrollStartOld = 79; //new replacement code that starts iScrollStartOld at 0
int bl = 0; // variable for backlight setting, set at 0 to start
int hallSensor1 = 1; // variable for hall sensor state 1
int hallSensor2 = 1; // variable for hall sensor state 2
int eyeTrack = 0; // variable to track eye movements
int xloc[] = {0, 240, 0, 0}; // placement for the fire pics on the x-axis (really the y-axis in the code)
int yloc[] = {0, 40, 200, 40}; // placement for the fire pics on the y-axis (really the x-axis in the code)
int x = 0; // variable to hold the x integration from the above array
int y = 0; // variable to hold the y integration from the above array
char* fireBMPs[]={"fire01.bmp", "fire02.bmp", "fire03.bmp", "fire04.bmp",
"fire05.bmp", "fire06.bmp", "fire07.bmp", "fire08.bmp",
"fire09.bmp", "fire10.bmp", "fire11.bmp", "fire12.bmp", 
"fire13.bmp", "fire14.bmp", "fire15.bmp", "fire16.bmp", 
"fire17.bmp", "fire18.bmp", "fire19.bmp", "fire20.bmp"}; //character string for fire pics

void setup() {
  Serial.begin(9600);
  pinMode(hallPin1, INPUT); // set the pin the hall sensor 1 is on to input
  pinMode(hallPin2, INPUT); // set the pin the hall sensor 2 is on to input
  
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
    return;
  }
  Serial.println("The Eye of Agamotto");
  
    tft.begin();
  tft.fillScreen(ILI9340_BLACK);
  analogWrite(_bl, 0); // set the backlight to off so we can fade in w/o seeing the eye drawn
  tft.fillScreen(ILI9340_WHITE);

  bmpDraw("eye.bmp", 40, 160);
}

void(* resetFunc) (void) = 0; // declare reset function at address 0

void loop(void) {
   hallSensor1 = digitalRead(hallPin1); // read the state of the first hall sensor
   hallSensor2 = digitalRead(hallPin2); // read the state of the second hall sensor
  if(bl == 2){ // if hallSensor2 has been activated after some eye scrolling, dim the backlight to hide the eye
    bl = 3; // set bl value to 3 so that it doesn't keep fading the backlight in
    for (int fadeValue = 255 ; fadeValue > 0; fadeValue -= 5) {
    // sets the value (range from 255 to 0):
    analogWrite(_bl, fadeValue);
    // wait for 30 milliseconds to see the fade in effect
    delay(30);
    }
  }
  if(bl == 3){
    tft.fillScreen(ILI9340_BLACK);
    SendVerticalScrollStartAddress(0);
    bmpDraw("bleye.bmp", 0, 0); // once back light is off, draw the "angry eye"
    bl = 4; // set bl to 0 to get ready for the next normal process
    if(bl == 4){
        for (int fadeValue = 0 ; fadeValue < 255; fadeValue += 5) {
        // sets the value (range from 0 to 255):
        analogWrite(_bl, fadeValue);
        // wait for 30 milliseconds to see the fade in effect
       delay(30);
  }
  }
  for (int f = 0 ; f < 20 ; f++){ // setup to run through all the fire files
    bmpDraw(fireBMPs[f], yloc[y], xloc[x]); // run through all of the fire bmps
    if (x == 3) x = -1; // restart the xloc array reference
    if (y == 3) y = -1; // restart the yloc array reference
    if (f == 19) bl = 5; // set bl to 5 to get ready for the next fade effect
    x++; // iterate x up
    y++; // iterate y up
    }
}
  if(bl == 5){
    for (int fadeValue = 255 ; fadeValue > 0; fadeValue -= 5) { // fade out
    // sets the value (range from 255 to 0):
    analogWrite(_bl, fadeValue);
    // wait for 30 milliseconds to see the fade in effect
    delay(30);
  }
 resetFunc(); // reset back to setup once the whole thing is over
}
  tft.setCursor(0, iScrollStart == 0? 320 : iScrollStart - 0); // set bounds for scroll area, start? end : offset
  ScrollScreen();  
  delay(50);
}

void ScrollScreen(void) {
 if(eyeTrack >= 1200 && hallSensor1 == HIGH)
 {
  bl = 5; // turn off after a bunch of rounds as long as hallSensor1 isn't activated
  eyeTrack = 0; // reset eyeTrack counter
 }
 else
 {
  if(eyeTrack >= 400 && hallSensor2 == LOW) // allow a little bit of scrolling before "angry eye" is called on hallSensor2
  {
    bl = 2; // set bl setting to 2 so that it may dim
  }
  else
  {
    if (iScrollStart == 0 || iScrollStart > iScrollStartOld && iScrollStart < 160) // If the cursor is at 0 or is on its way up but less than 160
    {
    iScrollStartOld = iScrollStart; // Set the soon to be "old" value of iSrcollStart to iScrollStartOld
    iScrollStart += 2; // iScrollStart increments up
    }
    else
    {
    iScrollStartOld = iScrollStart; // Set the soon to be "old" value of iSrcollStart to iScrollStartOld
    iScrollStart -= 2; // iScrollStart increments down
    eyeTrack += 2; // increment eyeTrack up
    }   
  }  
 }                  
  SendVerticalScrollStartAddress(iScrollStart); // send scroll command data
    if(bl == 0 && hallSensor1 == LOW){ // if this is the first time the scroll command is sent and hallSensor1 is activated, power up the backlight to see the eye
  bl = 1; // set bl value to 1 so that it doesn't keep fading the backlight in
    for (int fadeValue = 0 ; fadeValue < 255; fadeValue += 5) {
    // sets the value (range from 0 to 255):
    analogWrite(_bl, fadeValue);
    // wait for 30 milliseconds to see the fade in effect
    delay(30);
  }
}

}

void WriteData16(uint16_t w) {
  tft.writedata(w >> 8);
  tft.writedata(w & 0xFF);     // XSTART 

}

void SendVerticalScrollDefinition(uint16_t wTFA, uint16_t wBFA) {
  // Did not pass in VSA as TFA+VSA=BFA must equal 320
  tft.writecommand(0x33); // Vertical Scroll definition.
  WriteData16(wTFA);   // 
  WriteData16(320-(wTFA+wBFA));
  WriteData16(wBFA);
}

void SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

  tft.writecommand(ILI9340_CASET); // Column addr set
  WriteData16(x0);   // XSTART 
  WriteData16(x1);   // XEND
  tft.writecommand(ILI9340_PASET); // Row addr set
  WriteData16(y0);   // YSTART
  WriteData16(y1);   // YEND
  tft.writecommand(ILI9340_RAMWR); // write to RAM
}  

void SendVerticalScrollStartAddress(uint16_t wVSP) {
  tft.writecommand(0x37); // Vertical Scroll definition.
  WriteData16(wVSP);   // 
}

#define BUFFPIXEL 50

void bmpDraw(char *filename, uint16_t x, uint16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print("File not found");
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print("File size: "); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: "); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: "); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print("Loaded in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File & f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File & f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
