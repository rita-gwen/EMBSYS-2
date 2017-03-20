
#include "string.h"
#include "math.h"
#include "lcdUtil.h"
#define BUFSIZE 256


static Adafruit_ILI9341 lcdCtrlObj = Adafruit_ILI9341(); // The LCD controller
static Adafruit_FT6206 touchCtrlObj = Adafruit_FT6206(); // The touch controller

//UI objects
Adafruit_GFX_Button btnStart = Adafruit_GFX_Button();
Adafruit_GFX_Button btnStop = Adafruit_GFX_Button();
Adafruit_GFX_Button btnUp = Adafruit_GFX_Button();
Adafruit_GFX_Button btnDown = Adafruit_GFX_Button();
Adafruit_GFX_Button btnPause = Adafruit_GFX_Button();
Adafruit_GFX_Button btnContinuous = Adafruit_GFX_Button();

Adafruit_GFX_Button* buttons[UI_MAX_BUTTONS] = {&btnStart, &btnStop, &btnUp, &btnDown, &btnPause, &btnContinuous};

Adafruit_GFX_Button** getButtonsList(){
  return buttons;
}

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

//accessor function mostly for debugging purposes.
Adafruit_ILI9341* getLcdCtrl(){
    return &lcdCtrlObj;
}

Adafruit_FT6206* initTouch(){
    touchCtrlObj.begin();
    return &touchCtrlObj;
}

//Page layout parameters
#define LIST_AREA_HEIGHT 200
#define BUTTON_AREA_START_Y 220
#define PADDING 4
#define BUTTONS_HACROSS 3
#define BUTTONS_VACROSS 2
#define PROGRESS_START_Y LIST_AREA_HEIGHT + PADDING
#define PROGRESS_HEIGHT BUTTON_AREA_START_Y - LIST_AREA_HEIGHT - PADDING
#define BUTTON_WIDTH (ILI9341_TFTWIDTH - (BUTTONS_HACROSS + 1)*PADDING)/BUTTONS_HACROSS
#define BUTTON_HEIGHT (ILI9341_TFTHEIGHT - BUTTON_AREA_START_Y - (BUTTONS_VACROSS + 1)*PADDING)/BUTTONS_VACROSS
#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8
#define LINE_PADDING 2
#define TEXT_SIZE 2
#define POINTER_X PADDING + LINE_PADDING
#define LIST_START_X PADDING + 2*LINE_PADDING + (CHAR_HEIGHT * TEXT_SIZE)/2
#define LIST_START_Y PADDING + 2 + LINE_PADDING
#define TEXT_COLOR ILI9341_YELLOW
#define PROGRESS_COLOR ILI9341_YELLOW
#define BG_COLOR ILI9341_BLACK


uint8_t pointerPos = 0;

void drawInterface(){
  lcdCtrlObj.fillScreen(ILI9341_BLACK);
  lcdCtrlObj.drawRect(PADDING-1, PADDING-1
                      , ILI9341_TFTWIDTH - PADDING*2 + 2, LIST_AREA_HEIGHT
                      , ILI9341_PURPLE);
  lcdCtrlObj.drawRect(PADDING, PADDING
                      , ILI9341_TFTWIDTH - PADDING*2, LIST_AREA_HEIGHT-2
                      , ILI9341_PURPLE);
  char* txtStart = "Start";
  char* txtStop = "Stop";
  char* txtUp = "Up";
  char* txtDown = "Down";
  char* txtPause = "Pause";
  char* txtContinuous = "Cont.";
  btnStart.initButton(&lcdCtrlObj, PADDING, BUTTON_AREA_START_Y + PADDING
                        , BUTTON_WIDTH, BUTTON_HEIGHT
                        , ILI9341_CYAN, ILI9341_NAVY, ILI9341_CYAN
                        , txtStart, TEXT_SIZE, UI_CMD_START_PLAYBACK);
  btnStart.drawButton(false);
  btnStop.initButton(&lcdCtrlObj, PADDING, BUTTON_AREA_START_Y + 2* PADDING + BUTTON_HEIGHT
                        , BUTTON_WIDTH, BUTTON_HEIGHT
                        , ILI9341_CYAN, ILI9341_NAVY, ILI9341_CYAN
                        , txtStop, TEXT_SIZE, UI_CMD_STOP_PLAYBACK);
  btnStop.drawButton(false);
  
  btnPause.initButton(&lcdCtrlObj, 2*PADDING + BUTTON_WIDTH, BUTTON_AREA_START_Y + PADDING
                        , BUTTON_WIDTH, BUTTON_HEIGHT
                        , ILI9341_CYAN, ILI9341_NAVY, ILI9341_CYAN
                        , txtPause, TEXT_SIZE, UI_CMD_PAUSE_PLAYBACK);
  btnPause.drawButton(false);
  btnContinuous.initButton(&lcdCtrlObj, 2*PADDING + BUTTON_WIDTH, BUTTON_AREA_START_Y + 2* PADDING + BUTTON_HEIGHT
                        , BUTTON_WIDTH, BUTTON_HEIGHT
                        , ILI9341_CYAN, ILI9341_NAVY, ILI9341_CYAN
                        , txtContinuous, TEXT_SIZE, UI_CMD_CONTINUOUS_TOGGLE);
  btnContinuous.drawButton(false);
  btnUp.initButton(&lcdCtrlObj, 3*PADDING + 2*BUTTON_WIDTH, BUTTON_AREA_START_Y + PADDING
                        , BUTTON_WIDTH, BUTTON_HEIGHT
                        , ILI9341_CYAN, ILI9341_NAVY, ILI9341_CYAN
                        , txtUp, TEXT_SIZE, UI_CMD_MOVE_UP);
  btnUp.drawButton(false);
  btnDown.initButton(&lcdCtrlObj, 3*PADDING + 2*BUTTON_WIDTH, BUTTON_AREA_START_Y + 2* PADDING + BUTTON_HEIGHT
                        , BUTTON_WIDTH, BUTTON_HEIGHT
                        , ILI9341_CYAN, ILI9341_NAVY, ILI9341_CYAN
                        , txtDown, TEXT_SIZE, UI_CMD_MOVE_DOWN);
  btnDown.drawButton(false);
}

void drawListElement(uint8_t elementPosition, char* elementName){
  lcdCtrlObj.setCursor(LIST_START_X, 
                       LIST_START_Y + (TEXT_SIZE * CHAR_HEIGHT + LINE_PADDING * 2) * elementPosition );
  lcdCtrlObj.setTextSize(TEXT_SIZE);
  lcdCtrlObj.setTextColor(TEXT_COLOR);
  lcdCtrlObj.setTextWrap(false);
  char* i= elementName;
  while(*i) lcdCtrlObj.write(*i++);
}

void _drawListPointer(uint8_t elementPosition, uint16_t color){
  int y = LIST_START_Y + (TEXT_SIZE * CHAR_HEIGHT + LINE_PADDING * 2) * elementPosition;
  lcdCtrlObj.drawTriangle(POINTER_X, y
               , POINTER_X + (CHAR_HEIGHT * TEXT_SIZE)/2, y + (CHAR_HEIGHT * TEXT_SIZE)/2
               , POINTER_X, y + (CHAR_HEIGHT * TEXT_SIZE)
               , color);
  lcdCtrlObj.fillTriangle(POINTER_X, y
               , POINTER_X + (CHAR_HEIGHT * TEXT_SIZE)/2, y + (CHAR_HEIGHT * TEXT_SIZE)/2
               , POINTER_X, y + (CHAR_HEIGHT * TEXT_SIZE)
               , color);
}

void drawListPointer(uint8_t elementPosition){
  _drawListPointer(elementPosition, TEXT_COLOR);
}

void eraseListPointer(uint8_t elementPosition){
  _drawListPointer(elementPosition, BG_COLOR);
}

static float progressBarPosition = 0.0;

void eraseProgressBar(){
  lcdCtrlObj.fillRect(PADDING, PROGRESS_START_Y
                      , ILI9341_TFTWIDTH - PADDING*2, PROGRESS_HEIGHT
                      , BG_COLOR);
  progressBarPosition = 0.0;
}

void incrementProgressBar(float p_progressPct){
  uint16_t currentPix = (uint16_t)floor((ILI9341_TFTWIDTH - PADDING*2) * progressBarPosition);
  progressBarPosition = p_progressPct;
  uint16_t newPix = (uint16_t)floor((ILI9341_TFTWIDTH - PADDING*2) * progressBarPosition);  
  lcdCtrlObj.fillRect(PADDING, PROGRESS_START_Y
                      , newPix, PROGRESS_HEIGHT
                      , PROGRESS_COLOR);  
}


void toggleContinuousBtn(bool isContinuous){
  btnContinuous.drawButton(!isContinuous);
}

void togglePauseBtn(bool isPaused){
  btnPause.drawButton(!isPaused);
}