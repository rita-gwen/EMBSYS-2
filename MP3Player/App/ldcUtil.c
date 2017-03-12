
#include "string.h"
#include "lcdUtil.h"
#define BUFSIZE 256


Adafruit_ILI9341 lcdCtrlObj = Adafruit_ILI9341(); // The LCD controller
Adafruit_FT6206 touchCtrlObj = Adafruit_FT6206(); // The touch controller

//UI objects
Adafruit_GFX_Button btnStart = Adafruit_GFX_Button();
Adafruit_GFX_Button btnStop = Adafruit_GFX_Button();
Adafruit_GFX_Button btnUp = Adafruit_GFX_Button();
Adafruit_GFX_Button btnDown = Adafruit_GFX_Button();

Adafruit_ILI9341* initLcd(){
    PjdfErrCode pjdfErr;
    INT32U length;

    char buf[BUFSIZE];
    PrintWithBuf(buf, BUFSIZE, "LcdTouchDemoTask: starting\n");

    PrintWithBuf(buf, BUFSIZE, "Opening LCD driver: %s\n", PJDF_DEVICE_ID_LCD_ILI9341);
    // Open handle to the LCD driver
    HANDLE hLcd = Open(PJDF_DEVICE_ID_LCD_ILI9341, 0);
    if (!PJDF_IS_VALID_HANDLE(hLcd)) while(1);

    PrintWithBuf(buf, BUFSIZE, "Opening LCD SPI driver: %s\n", LCD_SPI_DEVICE_ID);
    // We talk to the LCD controller over a SPI interface therefore
    // open an instance of that SPI driver and pass the handle to 
    // the LCD driver.
    HANDLE hSPI = Open(LCD_SPI_DEVICE_ID, 0);
    if (!PJDF_IS_VALID_HANDLE(hSPI)) while(1);

    length = sizeof(HANDLE);
    pjdfErr = Ioctl(hLcd, PJDF_CTRL_LCD_SET_SPI_HANDLE, &hSPI, &length);
    if(PJDF_IS_ERROR(pjdfErr)) while(1);

	PrintWithBuf(buf, BUFSIZE, "Initializing LCD controller\n");
    lcdCtrlObj.setPjdfHandle(hLcd);
    lcdCtrlObj.begin();  
    return &lcdCtrlObj;
}

Adafruit_FT6206* initTouch(){
    touchCtrlObj.begin();
    return &touchCtrlObj;
}

#define LIST_AREA_HEIGHT 180
#define PADDING 4
#define BUTTON_WIDTH (ILI9341_TFTWIDTH - 3*PADDING)/2
#define BUTTON_HEIGHT (ILI9341_TFTHEIGHT - LIST_AREA_HEIGHT - 3*PADDING)/2

void drawInterface(){
  lcdCtrlObj.drawRect(3, 3
                      , ILI9341_TFTWIDTH - 6, LIST_AREA_HEIGHT, ILI9341_PURPLE);
  lcdCtrlObj.drawRect(4, 4, ILI9341_TFTWIDTH - 8, LIST_AREA_HEIGHT-2, ILI9341_PURPLE);
  char* txtStart = "Start";
  char* txtStop = "Stop";
  char* txtUp = "Up";
  char* txtDown = "Down";
  btnStart.initButton(&lcdCtrlObj, PADDING, LIST_AREA_HEIGHT + PADDING
                        , BUTTON_WIDTH, BUTTON_HEIGHT
                        , ILI9341_CYAN, ILI9341_NAVY, ILI9341_CYAN
                        , txtStart, 2);
  btnStart.drawButton(false);
  btnStop.initButton(&lcdCtrlObj, PADDING, LIST_AREA_HEIGHT + 2* PADDING + BUTTON_HEIGHT
                        , BUTTON_WIDTH, BUTTON_HEIGHT
                        , ILI9341_CYAN, ILI9341_NAVY, ILI9341_CYAN
                        , txtStop, 2);
  btnStop.drawButton(false);
  btnUp.initButton(&lcdCtrlObj, 2*PADDING + BUTTON_WIDTH, LIST_AREA_HEIGHT + PADDING
                        , BUTTON_WIDTH, BUTTON_HEIGHT
                        , ILI9341_CYAN, ILI9341_NAVY, ILI9341_CYAN
                        , txtUp, 2);
  btnUp.drawButton(false);
  btnDown.initButton(&lcdCtrlObj, 2*PADDING + BUTTON_WIDTH, LIST_AREA_HEIGHT + 2* PADDING + BUTTON_HEIGHT
                        , BUTTON_WIDTH, BUTTON_HEIGHT
                        , ILI9341_CYAN, ILI9341_NAVY, ILI9341_CYAN
                        , txtDown, 2);
  btnDown.drawButton(false);
}

void drawString(){}