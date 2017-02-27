// /dev/lcd_touch_spi PJDF interface implementation 

#include "bsp.h"
#include "pjdf.h"
#include "pjdfInternal.h"
#include "pjdfCtrlTouch.h"
#include <Adafruit_FT6206.h>

Adafruit_FT6206 touchCtrlInternal = Adafruit_FT6206(); // The touch controller

// Control registers etc for SPI hardware
typedef struct _PjdfContextTouch
{
    Adafruit_FT6206 *touchAdapter; // Adafruit library class
    I2C_TypeDef *i2cRegisters;          //I2C port definition
    
} PjdfContextTouch;

static PjdfContextTouch touch1Context = { 
    &touchCtrlInternal,
    I2C1
};


// Open LCD Touch device
static PjdfErrCode OpenTouch(DriverInternal *pDriver, INT8U flags)
{
    PjdfContextTouch *pContext = (PjdfContextTouch*) pDriver->deviceContext;
    if (pContext == NULL) while(1);

    if (! pContext->touchAdapter->begin(40)) {  // pass in 'sensitivity' coefficient
        return PJDF_ERR_DEVICE_NOT_INIT;
    }
    return PJDF_ERR_NONE;
}

// CloseLCD Touch device
static PjdfErrCode CloseTouch(DriverInternal *pDriver)
{
    I2C1_close();       //free the resources
    return PJDF_ERR_NONE;
}

//Reads touch point coordinates into a TS_Point class buffer
static PjdfErrCode ReadTouch(DriverInternal *pDriver, void* pBuffer, INT32U* pCount)
{
    PjdfContextTouch *pContext = (PjdfContextTouch*) pDriver->deviceContext;
    if (pContext == NULL) while(1);
    TS_Point *pointBuffer = (TS_Point*)pBuffer;
    uint16_t x, y;
    pContext->touchAdapter->readData(&x, &y);
    pointBuffer->x = (int16_t)x;
    pointBuffer->y = (int16_t)y;

    //TODO: Implement reading from touch device
    return PJDF_ERR_NONE;
}


// WriteTouch
static PjdfErrCode WriteTouch(DriverInternal *pDriver, void* pBuffer, INT32U* pCount)
{
    PjdfContextTouch *pContext = (PjdfContextTouch*) pDriver->deviceContext;
    if (pContext == NULL) while(1);

    //Nothing to do here, this device does not support writing.
    return PJDF_ERR_NONE;
}

// IoctlTouch
// Handles the request codes defined in pjdfLctTouchSpi.h
static PjdfErrCode IoctlTouch(DriverInternal *pDriver, INT8U request, void* pArgs, INT32U* pSize)
{
    INT8U osErr;
    PjdfContextTouch *pContext = (PjdfContextTouch*) pDriver->deviceContext;
    if (pContext == NULL) while(1);
    switch (request)
    {
      case PJDF_CTRL_TOUCH_SET_SENSITIVITY:
        pContext->touchAdapter->writeRegister8(FT6206_REG_THRESHHOLD, *((uint8_t*)pArgs));

        break;
      case PJDF_CTRL_TOUCH_GET_TOUCHED_FLAG:
        if(pContext->touchAdapter->touched())
          *((uint8_t*)pArgs) = 1;
        else 
          *((uint8_t*)pArgs) = 0;
        break;
      //TODO: Add specific requests handling
      default:
        while(1);
        break;
    }
    return PJDF_ERR_NONE;
}


// Initializes the Touch driver.
PjdfErrCode InitTouch(DriverInternal *pDriver, char *pName)
{   
    if (strcmp (pName, pDriver->pName) != 0) while(1); // pName should have been initialized in driversInternal[] declaration
    
    // Initialize semaphore for serializing operations on the device 
    pDriver->sem = OSSemCreate(1); 
    if (pDriver->sem == NULL) while (1);  // not enough semaphores available
    pDriver->refCount = 0; // initial number of Open handles to the device
    pDriver->maxRefCount = 1; // only one open handle allowed
  
    //TODO: Add required initialization
    pDriver->deviceContext = (void*) &touch1Context;     //get driver context
    I2C1_init();         //Initialize the device
    
  
    // Assign implemented functions to the interface pointers
    pDriver->Open = OpenTouch;
    pDriver->Close = CloseTouch;
    pDriver->Read = ReadTouch;
    pDriver->Write = WriteTouch;
    pDriver->Ioctl = IoctlTouch;
    
    pDriver->initialized = OS_TRUE;
    return PJDF_ERR_NONE;
}
