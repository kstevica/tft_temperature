#include "Adafruit_GFX.h"    // Core graphics library
#include "Adafruit_TFTLCD.h" // Hardware-specific library
#include <OneWire.h>

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0

#define LCD_RESET A4

#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

OneWire  ds(11);


Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

void setup(void) {
  Serial.begin(57600);
  tft.reset();

  uint16_t identifier = tft.readID();
  tft.begin(identifier);

  tft.setRotation(3);
  tft.fillScreen(WHITE);
  tft.setCursor(50, 20);
  tft.setTextColor(BLACK);  
  tft.setTextSize(3);
  tft.println("Temperatura");
}

void loop(void) {
  float celsius = readTemp();

  if (celsius!=-100) {
    tft.setCursor(50, 20);
    tft.setTextColor(BLACK);  
    tft.setTextSize(3);
    tft.println("Temperatura");
    tft.setCursor(110, 110);
    tft.fillRect(120, 110, 95, 60, WHITE);
    tft.println(celsius);    
  }

  delay(1000); 
}

float readTemp()
{
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;
  
  if ( !ds.search(addr)) {
    //sensor not connected
    ds.reset_search();
    delay(250);
    return -100;
  }
  
  if (OneWire::crc8(addr, 7) != addr[7]) {
      //CRC is not valid
      return -100;
  }
  switch (addr[0]) {
    case 0x10:
      //old DS1820
      type_s = 1;
      break;
    case 0x28:
      //DS18B20
      type_s = 0;
      break;
    case 0x22:
      //DS1822
      type_s = 0;
      break;
    default:      
      return -100;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);
  
  delay(1000);
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  return celsius;
}