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

#define MAX_UI_QUEUE_SIZE 10
#define COMMAND_DATA_SIZE 32

//-- Message queue definitions
typedef struct {
  uint8_t uiCommand;
  char p_data[COMMAND_DATA_SIZE];         //additional data to pass with the command if required.
} UIQMessage;

UIQMessage qMessageMemory[MAX_UI_QUEUE_SIZE];   //the block of memory to use for queue messages
UIQMessage* qEntries[MAX_UI_QUEUE_SIZE];

static OS_MEM* qMemManager;        //queue memory manager
static OS_EVENT* uiQueue;


//-- Commands list definitions
void MovePointerUp(void* p_data);
void MovePointerDown(void* p_data);
void StartPlayback(void* p_data);
void StopPlayback(void* p_data);
void ResetProgressBar(void* p_data);
void UpdateProgressBar(void* p_data);
    
struct UICommandAction{
  uint8_t uiCommand;
  void (*action)(void* p_data);
};

UICommandAction ui_commands[UI_MAXCOMMANDS] = {
  {UI_CMD_MOVE_DOWN,MovePointerDown},
  {UI_CMD_MOVE_UP,MovePointerUp},
  {UI_CMD_START_PLAYBACK,StartPlayback},
  {UI_CMD_STOP_PLAYBACK,StopPlayback},
  {UI_CMD_RESET_PROGRESS,ResetProgressBar},
  {UI_CMD_UPDATE_PROGRESS,UpdateProgressBar}
};

/***
* Draw the UI objects on the screen
*/
void InitUI(void){
  drawInterface();

  RingRewindToStart();
  do{
    drawListElement(RingGetBufferPointer(), RingNextFile());
  }while(!RingIsEnd());
    
  RingNextFile();
  drawListPointer(RingGetBufferPointer()) ;
  ResetProgressBar((void*)0x0);
}

//-- Queue methods
/***
* Create the message queue. This should be called from the startup task to make sure 
* the queue is available by the time other tasks start running
*/
void InitUIMessageQueue(void){
    INT8U err;
    char buf[BUFSIZE];
    qMemManager = OSMemCreate(qMessageMemory, MAX_UI_QUEUE_SIZE, sizeof(UIQMessage), &err);
    if(err != OS_ERR_NONE){
      	PrintWithBuf(buf, BUFSIZE, "Error while creating UI queue memory partition. Error code %u.\n", err);
        while(1);
    }
    uiQueue = OSQCreate((void**)qEntries, MAX_UI_QUEUE_SIZE);
    if(!uiQueue){
      	PrintWithBuf(buf, BUFSIZE, "Cannot create UI queue.\n");
        while(1);
    }
}

/***
* This is a client method to be called by other threads when they need to 
* post an UI event
*/
void PostUIQueueMessage(uint8_t cmd, void* p_data, uint8_t dataSize){
    INT8U err;
    char buf[BUFSIZE];
    UIQMessage* msg = (UIQMessage*) OSMemGet(qMemManager, &err);
    if(err != OS_ERR_NONE){
      	PrintWithBuf(buf, BUFSIZE, "Error while allocating memory for UI queue message. Error code %u.\n", err);
        while(1);
    }
    if(msg){
      msg->uiCommand = cmd;
      //memcopy of the data to prevent potential contention between threads
      char* msgBuffer = (char*)(&(msg->p_data));
      char* data = (char*)p_data;
      for(uint8_t i = 0; i < dataSize; i++){
        msgBuffer[i] = data[i];
      }
      OSQPost(uiQueue, msg);
    }
}

void UIDispatcherTask(void* p_data){
    INT8U err;
    char buf[BUFSIZE];
    initLcd();
    PrintWithBuf(buf, BUFSIZE, "Initializing FT6206 touchscreen controller\n");
    InitUI();
    
    while(1){
      UIQMessage* msg = (UIQMessage*)OSQPend(uiQueue, 0, &err); //wait for the next message to process
      if (err != OS_ERR_NONE){
        PrintWithBuf(buf, PRINTBUFMAX, "Error pulling a message from the UI queue: %d\n", err);
        while(1);
      }
      uint8_t isValidCmd = 0;
      for(uint8_t j = 0; j < UI_MAXCOMMANDS; j++){      //loop though commands list to find the command handler
        if(msg->uiCommand == ui_commands[j].uiCommand){
          ui_commands[j].action(msg->p_data);
          isValidCmd = 1;
          break;
        }
      }
      if(!isValidCmd)
        PrintWithBuf(buf, PRINTBUFMAX, "Unknown command: %u\n", msg->uiCommand);
      OSMemPut(qMemManager, msg);       //returning queue message back to the memory pool
    }
}

void MovePointerUp(void* p_data){
   char buf[BUFSIZE];
   PrintWithBuf(buf, PRINTBUFMAX, "UP\n");

   eraseListPointer(RingGetBufferPointer());
   RingPrevFile();
   drawListPointer(RingGetBufferPointer()) ;
}

void MovePointerDown(void* p_data){
   char buf[BUFSIZE];
   PrintWithBuf(buf, PRINTBUFMAX, "DOWN\n");
   eraseListPointer(RingGetBufferPointer());
   RingNextFile();
   drawListPointer(RingGetBufferPointer()) ;
}

void StartPlayback(void* p_data){
   char buf[BUFSIZE];
   INT8U err;
   PrintWithBuf(buf, PRINTBUFMAX, "START\n");
   OS_FLAG_GRP* mp3Flags = GetMP3Flags();
   OSFlagPost(mp3Flags, MP3_CTRL_FLAG_PLAY, OS_FLAG_SET, &err);
}

void StopPlayback(void* p_data){
   char buf[BUFSIZE];
   INT8U err;
   PrintWithBuf(buf, PRINTBUFMAX, "STOP\n");
   OS_FLAG_GRP* mp3Flags = GetMP3Flags();
   OSFlagPost(mp3Flags, MP3_CTRL_FLAG_STOP, OS_FLAG_SET, &err);
}

void ResetProgressBar(void* p_data){
   char buf[BUFSIZE];
   PrintWithBuf(buf, PRINTBUFMAX, "RESET_PROGRESS\n");
   eraseProgressBar();
}

void UpdateProgressBar(void* p_data){
   char buf[BUFSIZE];
   PrintWithBuf(buf, PRINTBUFMAX, "UPDATE_PROGRESS %f\n", *((float*)p_data));
   incrementProgressBar(*((float*)p_data));
}

void PausePlayback(void* p_data){
   char buf[BUFSIZE];
   INT8U err;
   OS_FLAG_GRP* mp3Flags = GetMP3Flags();
   OS_FLAGS flgs = OSFlagQuery(mp3Flags, &err);
   if(flgs & MP3_CTRL_FLAG_PAUSE)
    OSFlagPost(mp3Flags, MP3_CTRL_FLAG_PAUSE, OS_FLAG_CLR, &err);
   else
    OSFlagPost(mp3Flags, MP3_CTRL_FLAG_PAUSE, OS_FLAG_SET, &err);
}