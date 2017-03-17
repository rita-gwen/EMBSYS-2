/************************************************************************************
Definitions of the MP3 Player UI operations:
   - Initialize the screen
   - Start Playing
   - Update Progress
  ...etc
Each action has void actionName(void) format and takes no parameters.
All harware/software initializations should be done beforehand
************************************************************************************/

#include "bsp.h"
#include "print.h"
#include "SD.h"
#include "UIActions.h"
#include "FileRing.h"

void MovePointerUp(TS_Point* toichPoint);
void MovePointerDown(TS_Point* toichPoint);
void StartPlayback(TS_Point* toichPoint);
void StopPlayback(TS_Point* toichPoint);

struct UICommandAction{
  uint8_t uiCommand;
  void (*action)(TS_Point* toichPoint);
};

UICommandAction ui_commands[UI_MAXCOMMANDS] = {
  {UI_CMD_MOVE_DOWN,MovePointerDown},
  {UI_CMD_MOVE_UP,MovePointerUp},
  {UI_CMD_START_PLAYBACK,StartPlayback},
  {UI_CMD_STOP_PLAYBACK,StopPlayback}
};

void InitUI(void){
  drawInterface();

  RingRewindToStart();
  do{
    drawListElement(RingGetBufferPointer(), RingNextFile());
  }while(!RingIsEnd());
    
  RingNextFile();
  drawListPointer(RingGetBufferPointer()) ;
}

void DispatchUIEvent(TS_Point* touchPoint){
  char buf[BUFSIZE];
  Adafruit_GFX_Button** buttons = getButtonsList();
  for(uint8_t i = 0; i < UI_MAXCOMMANDS; i++){          //loop through UI elements
    if(buttons[i]->contains(touchPoint->x, touchPoint->y)){
      for(uint8_t j = 0; j < UI_MAXCOMMANDS; j++){      //loop though commands list
        if(buttons[i]->getCommand() == ui_commands[j].uiCommand){
          ui_commands[j].action(touchPoint);
          return;
        }
      }
      PrintWithBuf(buf, PRINTBUFMAX, "Unknown command: %u\n", buttons[i]->getCommand());
    }
  }
}

void MovePointerUp(TS_Point* toichPoint){
   char buf[BUFSIZE];
   PrintWithBuf(buf, PRINTBUFMAX, "UP\n");
}

void MovePointerDown(TS_Point* toichPoint){
   char buf[BUFSIZE];
   PrintWithBuf(buf, PRINTBUFMAX, "DOWN\n");
}

void StartPlayback(TS_Point* toichPoint){
   char buf[BUFSIZE];
   PrintWithBuf(buf, PRINTBUFMAX, "START\n");
}

void StopPlayback(TS_Point* toichPoint){
   char buf[BUFSIZE];
   PrintWithBuf(buf, PRINTBUFMAX, "STOP\n");
}