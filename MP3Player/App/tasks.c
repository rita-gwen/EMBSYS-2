/************************************************************************************

Copyright (c) 2001-2016  University of Washington Extension.

Module Name:

    tasks.c

Module Description:

    The tasks that are executed by the test application.

2016/2 Nick Strathy adapted it for NUCLEO-F401RE 

************************************************************************************/
#include <stdarg.h>

#include "bsp.h"
#include "print.h"
#include "mp3Util.h"
#include "SD.h"
#include "UIActions.h"
#include "MP3Player.h"
#include "FileRing.h"

#define PENRADIUS 3

long MapTouchToScreen(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


/************************************************************************************

   Allocate the stacks for each task.
   The maximum number of tasks the application can have is defined by OS_MAX_TASKS in os_cfg.h

************************************************************************************/

static OS_STK   LcdTouchTaskStk[APP_CFG_TASK_START_STK_SIZE];
static OS_STK   Mp3PlayerTaskStk[APP_CFG_TASK_START_STK_SIZE];
static OS_STK   UIDispatcherTaskStk[APP_CFG_TASK_START_STK_SIZE];
     
// Task prototypes
void LcdTouchTask(void* pdata);

// Useful functions
void PrintToLcdWithBuf(char *buf, int size, char *format, ...);

// Globals
BOOLEAN nextSong = OS_FALSE;
static Adafruit_FT6206*  touchCtrl;
static Adafruit_ILI9341* lcdCtrl;


/************************************************************************************

   This task is the initial task running, started by main(). It starts
   the system tick timer and creates all the other tasks. Then it deletes itself.

************************************************************************************/
void StartupTask(void* pdata)
{
	char buf[BUFSIZE];

    PjdfErrCode pjdfErr;
    INT32U length;
    static HANDLE hSD = 0;
    static HANDLE hSPI = 0;

	PrintWithBuf(buf, BUFSIZE, "StartupTask: Begin\n");
	PrintWithBuf(buf, BUFSIZE, "StartupTask: Starting timer tick\n");

    // Low-level initializations:
    //   Start the system tick
    OS_CPU_SysTickInit(OS_TICKS_PER_SEC);
    //   Congigure touch screen EXTI interrupt
    Init_TouchInterrupt();
    
    // Initialize SD card PJDF drivers
    PrintWithBuf(buf, PRINTBUFMAX, "Opening handle to SD driver: %s\n", PJDF_DEVICE_ID_SD_ADAFRUIT);
    hSD = Open(PJDF_DEVICE_ID_SD_ADAFRUIT, 0);
    if (!PJDF_IS_VALID_HANDLE(hSD)){
      PrintWithBuf(buf, PRINTBUFMAX, "Error initializing SD driver: %d\n", hSD);
      while(1);
    }
    // We talk to the SD controller over a SPI interface therefore
    // open an instance of that SPI driver and pass the handle to 
    // the SD driver.
    hSPI = Open(SD_SPI_DEVICE_ID, 0);
    if (!PJDF_IS_VALID_HANDLE(hSPI)) while(1);
    
    length = sizeof(HANDLE);
    pjdfErr = Ioctl(hSD, PJDF_CTRL_SD_SET_SPI_HANDLE, &hSPI, &length);
    if(PJDF_IS_ERROR(pjdfErr)) while(1);
    
    //Initialize FAT32 subsystem and load the root dir into the ring buffer
    PrintWithBuf(buf, BUFSIZE, "StartupTask: Opening the SD card.\n");
    SD.begin(hSD);      
    if(InitRing() != OS_ERR_NONE){
      PrintWithBuf(buf, BUFSIZE, "Cannot initialize the ring buffer.\n");
      while(1);
    }
    RingLoadFiles();    //Load file names into the ring buffer
    
    //Initi UI message queue for the UI dispatcher
    InitUIMessageQueue();


    // Create the application tasks
    PrintWithBuf(buf, BUFSIZE, "StartupTask: Creating the application tasks\n");

    // The maximum number of tasks the application can have is defined by OS_MAX_TASKS in os_cfg.h
    OSTaskCreate(UIDispatcherTask, (void*)0, &UIDispatcherTaskStk[APP_CFG_TASK_START_STK_SIZE-1], APP_TASK_TEST3_PRIO);
    OSTaskCreate(LcdTouchTask, (void*)0, &LcdTouchTaskStk[APP_CFG_TASK_START_STK_SIZE-1], APP_TASK_TEST2_PRIO);
    OSTimeDly(500); // Allow LCD to initialize before starting the playback task.
    OSTaskCreate(Mp3PlaybackTask, (void*)0, &Mp3PlayerTaskStk[APP_CFG_TASK_START_STK_SIZE-1], APP_TASK_TEST1_PRIO);

    // Delete ourselves, letting the work be done in the new tasks.
    PrintWithBuf(buf, BUFSIZE, "StartupTask: deleting self\n");
	OSTaskDel(OS_PRIO_SELF);
}


/************************************************************************************

   Runs LCD/Touch demo code

************************************************************************************/
void LcdTouchTask(void* pdata)
{
    char buf[BUFSIZE];
    touchCtrl = initTouch();
    lcdCtrl = getLcdCtrl();

    PrintWithBuf(buf, BUFSIZE, "Initializing FT6206 touchscreen controller\n");
    
    int currentcolor = ILI9341_RED;
    
    while (1) { 
        
        // We are doing event waiting here instead of polling
        touchCtrl->waitForTouch();
        TS_Point rawPoint;
       
        //Retrieve a point  
        rawPoint = touchCtrl->getPoint();

        if (rawPoint.x == 0 && rawPoint.y == 0)
        {
            continue; // usually spurious, so ignore
        }
        
        // transform touch orientation to screen orientation.
        TS_Point p = TS_Point();
        p.x = MapTouchToScreen(rawPoint.x, 0, ILI9341_TFTWIDTH, ILI9341_TFTWIDTH, 0);
        p.y = MapTouchToScreen(rawPoint.y, 0, ILI9341_TFTHEIGHT, ILI9341_TFTHEIGHT, 0);
        
        lcdCtrl->fillCircle(p.x, p.y, PENRADIUS, currentcolor);
        Adafruit_GFX_Button** buttons = getButtonsList();
        for(uint8_t i = 0; i < UI_MAXCOMMANDS; i++){          //loop through UI elements
          if(buttons[i]->contains(p.x, p.y)){
            //when UI element is identified post the associated command into the 
            //UI message queue
            PostUIQueueMessage(buttons[i]->getCommand(), &p);
            break;
          }
        }
    }
}
