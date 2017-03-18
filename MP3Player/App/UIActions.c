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
#include "MP3Player.h"


void MovePointerUp(TS_Point* touchPoint);
void MovePointerDown(TS_Point* touchPoint);
void StartPlayback(TS_Point* touchPoint);
void StopPlayback(TS_Point* touchPoint);

struct UICommandAction{
  uint8_t uiCommand;
  void (*action)(TS_Point* touchPoint);
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

void MovePointerUp(TS_Point* touchPoint){
   char buf[BUFSIZE];
   PrintWithBuf(buf, PRINTBUFMAX, "UP\n");

   eraseListPointer(RingGetBufferPointer());
   RingPrevFile();
   drawListPointer(RingGetBufferPointer()) ;
}

void MovePointerDown(TS_Point* touchPoint){
   char buf[BUFSIZE];
   PrintWithBuf(buf, PRINTBUFMAX, "DOWN\n");
   eraseListPointer(RingGetBufferPointer());
   RingNextFile();
   drawListPointer(RingGetBufferPointer()) ;
}

void StartPlayback(TS_Point* touchPoint){
   char buf[BUFSIZE];
   INT8U err;
   PrintWithBuf(buf, PRINTBUFMAX, "START\n");
   OS_FLAG_GRP* mp3Flags = GetMP3Flags();
   OSFlagPost(mp3Flags, MP3_CTRL_FLAG_PLAY, OS_FLAG_SET, &err);
}

void StopPlayback(TS_Point* touchPoint){
   char buf[BUFSIZE];
   INT8U err;
   PrintWithBuf(buf, PRINTBUFMAX, "STOP\n");
   OS_FLAG_GRP* mp3Flags = GetMP3Flags();
   OSFlagPost(mp3Flags, MP3_CTRL_FLAG_STOP, OS_FLAG_SET, &err);
}