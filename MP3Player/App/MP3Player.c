#include "bsp.h"
#include "print.h"
#include "mp3Util.h"
#include "MP3Player.h"

OS_FLAG_GRP mp3Flags;        //Flag Group to control MP3 Player task


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

/***
* MP3PlaybackTask can be in 
*
*/
void Mp3PlaybackTask(void* pdata)
{
    char buf[BUFSIZE];
    HANDLE hMp3 = InitDrivers();
    PrintWithBuf(buf, BUFSIZE, "Starting MP3 playback.\n");

    // Send initialization data to the MP3 decoder and run a test
    Mp3Init(hMp3);
    int count = 0;
    char* fileName="/MOUNT-01.MP3";
    
    while (1)
    {
        OSTimeDly(500);
        PrintWithBuf(buf, BUFSIZE, "Begin streaming sound file  count=%d\n", ++count);
        //Mp3Stream(hMp3, (INT8U*)Train_Crossing, sizeof(Train_Crossing)); 
        Mp3StreamSDFile(hMp3, fileName);
        PrintWithBuf(buf, BUFSIZE, "Done streaming sound file  count=%d\n", count);
    }
}

