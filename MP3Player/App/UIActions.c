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
#include "SD.h"
#include "UIActions.h"


void InitUI(void){
  drawInterface();

  File dir = SD.open("/");   
  uint8_t linePos = 0;
  while (1){
    File entry = dir.openNextFile();
    if (!entry){
        break;
    }
    if(!entry.isDirectory()){
        drawListElement(linePos++, entry.name());
    }
    entry.close();
  }
  dir.close();
  drawListPointer(0) ;
  drawListPointer(1) ;
}

void MovePointerUp(void){
}

void MovePointerDown(void){
}