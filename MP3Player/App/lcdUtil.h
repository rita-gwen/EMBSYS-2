

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>

#define UI_MAXCOMMANDS          0x4
#define UI_CMD_MOVE_DOWN        0x1
#define UI_CMD_MOVE_UP          0x2
#define UI_CMD_START_PLAYBACK   0x3
#define UI_CMD_STOP_PLAYBACK    0x4


#ifndef LCDUTIL_H
#define LCDUTIL_H

Adafruit_ILI9341* initLcd();
Adafruit_FT6206* initTouch();

#endif

Adafruit_GFX_Button** getButtonsList();
void drawInterface();
void drawListElement(uint8_t elementPosition, char* elementName);
void drawListPointer(uint8_t elementPosition);
void eraseListPointer(uint8_t elementPosition);

