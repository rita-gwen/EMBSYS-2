#include "bsp.h"
#include "print.h"
#include "bspMp3.h"
#include "SD.h"
#include "mp3Util.h"
#include "MP3Player.h"
#include "FileRing.h"
#include "UIActions.h"

static OS_FLAG_GRP* mp3Flags;        //Flag Group to control MP3 Player task
static File dataFile;


/***
*Does required drivers initialization and returns handle of the MP3Player
driver ready to use.
*/
HANDLE InitDrivers(){
    PjdfErrCode pjdfErr;
    INT32U length;
    char buf[BUFSIZE];
    
    PrintWithBuf(buf, BUFSIZE, "Mp3DemoTask: starting\n");

    PrintWithBuf(buf, BUFSIZE, "Opening MP3 driver: %s\n", PJDF_DEVICE_ID_MP3_VS1053);
    // Open handle to the MP3 decoder driver
    HANDLE hMp3 = Open(PJDF_DEVICE_ID_MP3_VS1053, 0);
    if (!PJDF_IS_VALID_HANDLE(hMp3)) while(1);

    PrintWithBuf(buf, BUFSIZE, "Opening MP3 SPI driver: %s\n", MP3_SPI_DEVICE_ID);
    // We talk to the MP3 decoder over a SPI interface therefore
    // open an instance of that SPI driver and pass the handle to 
    // the MP3 driver.
    HANDLE hSPI = Open(MP3_SPI_DEVICE_ID, 0);
    if (!PJDF_IS_VALID_HANDLE(hSPI)) while(1);

    length = sizeof(HANDLE);
    pjdfErr = Ioctl(hMp3, PJDF_CTRL_MP3_SET_SPI_HANDLE, &hSPI, &length);
    if(PJDF_IS_ERROR(pjdfErr)) while(1);
    return hMp3;
}

OS_FLAG_GRP* GetMP3Flags(void){
  return mp3Flags;
}

#define PROGRESS_UPDATE_FREQUENCY 3000

/***
* MP3PlaybackTask can be in 
*
*/
void Mp3PlaybackTask(void* pdata)
{
    INT8U err_code;
    char buf[BUFSIZE];
    uint8_t mp3State = MP3_STATE_STOPPED;
    char* fileName;
    INT8U mp3Buf[MP3_DECODER_BUF_SIZE];
    INT32U iBufPos = 0;
    float playProgress = 0.0;
    INT16U loopCount = 0;
    
    HANDLE hMp3 = InitDrivers();
    
    mp3Flags = OSFlagCreate((OS_FLAGS)(0x00), &err_code);
    if(err_code != OS_ERR_NONE){
      	PrintWithBuf(buf, BUFSIZE, "Error while creating MP3Flags. Error code %u.\n", err_code);
        while(1);
    }
    
    PrintWithBuf(buf, BUFSIZE, "Starting MP3 playback.\n");

    // Send initialization data to the MP3 decoder and run a test
    Mp3Init(hMp3);
    
    //loop forever
    while (1)
    {
      //  if state is Stopped
      if(mp3State == MP3_STATE_STOPPED){
        playProgress = 0.0;
        PostUIQueueMessage(UI_CMD_RESET_PROGRESS, &playProgress, sizeof(playProgress));
        //      wait for the play signal
        OSFlagPend(mp3Flags, MP3_CTRL_FLAG_PLAY, OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err_code);
        //      change state to PLAYING
        mp3State = MP3_STATE_PLAYING;
        Mp3StreamInit(hMp3);
        //      open the file
        fileName = RingCurrentFile();
        dataFile = SD.open(fileName, O_READ);
        iBufPos = 0;
        if (!dataFile) 
        {
            PrintWithBuf(buf, PRINTBUFMAX, "Error: could not open SD card file '%s'\n", fileName);
            mp3State = MP3_STATE_STOPPED;
            continue;
        }
        PrintWithBuf(buf, PRINTBUFMAX, "Starting file playback: '%s'\n", fileName);
      }
      //  if EOF
      if(!dataFile.available()){
        //     close file
        dataFile.close();
        //     if continuous play flag set
        if(OSFlagAccept(mp3Flags, MP3_CTRL_FLAG_CONTINUOUS, OS_FLAG_WAIT_SET_ANY, &err_code)){
          //          move ring buffer pointer
          fileName = RingNextFile();
          //TODO:          post Update UI message
          //          open the file
          dataFile = SD.open(fileName, O_READ);
          iBufPos = 0;
        }
        //     else
        else{
          //          change state to Stopped
          mp3State = MP3_STATE_STOPPED;
          continue;
        }
      }
      //  load buffer
      iBufPos = 0;
      while (dataFile.available() && iBufPos < MP3_DECODER_BUF_SIZE)
      {
            mp3Buf[iBufPos++] = dataFile.read();
      }
      //  play buffer
      Write(hMp3, mp3Buf, &iBufPos);
      
      //update progress bar every so often
      if(loopCount >=PROGRESS_UPDATE_FREQUENCY){
        playProgress = 1.0 * dataFile.position() / dataFile.size();
        PostUIQueueMessage(UI_CMD_UPDATE_PROGRESS, &playProgress, sizeof(playProgress));
        loopCount = 0;
      }
      else
        loopCount++;
      
      //TODO:  update UI progress
      //  check for signals 
      OS_FLAGS flags = OSFlagQuery(mp3Flags, &err_code);
      //       if Stop signal received
      if(flags & MP3_CTRL_FLAG_STOP){
          //clear post flag
          OSFlagAccept(mp3Flags, MP3_CTRL_FLAG_STOP, OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, &err_code);
          //          close the file
          dataFile.close();
          //          change state to Stopped
          mp3State = MP3_STATE_STOPPED;
          continue;
      }
      //       if Pause signal received
      if(flags & MP3_CTRL_FLAG_PAUSE){
          //clear pause flag
          OSFlagAccept(mp3Flags, MP3_CTRL_FLAG_PAUSE, OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, &err_code);
          //          change state to Paused
          mp3State = MP3_STATE_PAUSED;
          //          wait for Play signal
          OS_FLAGS flg = OSFlagPend(mp3Flags, MP3_CTRL_FLAG_PLAY + MP3_CTRL_FLAG_STOP, 
                      OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err_code);
          //          if Stop signal is received while paused
          if(flg & MP3_CTRL_FLAG_STOP){
            //          close the file
            dataFile.close();
            //          change state to Stopped
            mp3State = MP3_STATE_STOPPED;
          }
          continue;
      }
      //       if Play signal received while playing do not consume the signal
      if(flags & MP3_CTRL_FLAG_PLAY){
        //          close current file
        dataFile.close();
        //          change state to Stopped and move on. 
        mp3State = MP3_STATE_STOPPED;
      }
   }
}

