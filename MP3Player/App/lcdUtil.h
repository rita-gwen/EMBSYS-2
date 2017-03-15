

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>

#ifndef lcdUtil
#define lcdUtil

Adafruit_ILI9341* initLcd();
Adafruit_FT6206* initTouch();

#endif

void drawInterface();
void drawListElement(uint8_t elementPosition, char* elementName);
void drawListPointer(uint8_t elementPosition);

