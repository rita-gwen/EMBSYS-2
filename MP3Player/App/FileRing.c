/************************************************
* Simple ring buffer for file names. Used both by the UI and MP3Player 
* tasks. Methods are thread-safe to prevent thread interference
*
************************************************/
#include "bsp.h"
#include "SD.h"
#include "FileRing.h"

char fileNames[FILE_NUMBER_MAX][FILE_NAME_SIZE];
uint8_t nameBufferPointer;
uint8_t numFiles;       //current count of files in the buffer
OS_EVENT* ringMutex;     //need a synchronization primitive to prevent concurrent access between threads

INT8U InitRing(void){
  INT8U err;
  ringMutex = OSMutexCreate(APP_RING_MUTEX_PRIO, &err);
  return err;
}

void RingLoadFiles(void){
  INT8U err;
  File dir = SD.open("/");   
  uint8_t linePos = 0;
  OSMutexPend(ringMutex, 0, &err);
  while(dir.getNextFileName(fileNames[linePos]) && linePos < FILE_NUMBER_MAX){linePos++;}
  OSMutexPost(ringMutex);
  dir.close();
  numFiles = linePos;
  nameBufferPointer = 0;
}

char* RingCurrentFile(void){
  INT8U err;
  char* res;
  OSMutexPend(ringMutex, 0, &err);
  res = fileNames[nameBufferPointer];
  OSMutexPost(ringMutex);
  return res;
}

char* RingNextFile(void){
  INT8U err;
  char* res;
  OSMutexPend(ringMutex, 0, &err);
  if(nameBufferPointer < numFiles - 1)
    nameBufferPointer++;
  else {
    nameBufferPointer = 0;
  }
  res = fileNames[nameBufferPointer];
  OSMutexPost(ringMutex);
  return res;
}

char* RingPrevFile(void){
  INT8U err;
  char* res;
  OSMutexPend(ringMutex, 0, &err);
  if(nameBufferPointer > 0)
    nameBufferPointer--;
  else {
    nameBufferPointer = numFiles - 1;
  }
  res = fileNames[nameBufferPointer];
  OSMutexPost(ringMutex);
  return res;
}

uint8_t RingGetBufferPointer(void){
  INT8U err;
  uint8_t res;
  OSMutexPend(ringMutex, 0, &err);
  res = nameBufferPointer;
  OSMutexPost(ringMutex);
  return res;
}

uint8_t RingGetFileCount(void){
    return numFiles;
}

void RingRewindToStart(void){
  INT8U err;
  uint8_t res;
  OSMutexPend(ringMutex, 0, &err);
  nameBufferPointer = numFiles;
  OSMutexPost(ringMutex);
}

bool RingIsEnd(){
  INT8U err;
  OSMutexPend(ringMutex, 0, &err);
  bool res = (nameBufferPointer == numFiles - 1);
  OSMutexPost(ringMutex);
  return res;
}
                