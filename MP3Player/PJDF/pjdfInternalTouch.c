// /dev/lcd_touch_spi PJDF interface implementation 

#include "bsp.h"
#include "pjdf.h"
#include "pjdfInternal.h"
#include "pjdfCtrlTouch.h"
#include "pjdfCtrlI2C.h"
#include <Adafruit_FT6206.h>

Adafruit_FT6206 touchCtrlInternal = Adafruit_FT6206(); // The touch controller

// Control registers etc for SPI hardware
typedef struct _PjdfContextTouch
{
    HANDLE i2cHandle; // I2C driver handler
    uint8_t registerAddr;       //address of the device register for the next read/write operation
} PjdfContextTouch;

static PjdfContextTouch touch1Context = { 
    (HANDLE)0x0,
    0x0
};


// Open LCD Touch device
static PjdfErrCode OpenTouch(DriverInternal *pDriver, INT8U flags)
{
    PjdfContextTouch *pContext = (PjdfContextTouch*) pDriver->deviceContext;
    if (pContext == NULL) 
      return PJDF_ERR_DEVICE_NOT_OPEN;

    return PJDF_ERR_NONE;       //nothing to do
}

// CloseLCD Touch device
static PjdfErrCode CloseTouch(DriverInternal *pDriver)
{
    PjdfContextTouch *pContext = (PjdfContextTouch*) pDriver->deviceContext;
    if (pContext == NULL) 
      return PJDF_ERR_DEVICE_NOT_OPEN;
    return Close(pContext->i2cHandle);       //free the resources
}

//Reads a set of registers
static PjdfErrCode ReadTouch(DriverInternal *pDriver, void* pBuffer, INT32U* pCount)
{
    PjdfContextTouch *pContext = (PjdfContextTouch*) pDriver->deviceContext;
    if (pContext == NULL) 
      return PJDF_ERR_DEVICE_NOT_OPEN;
    //write register address
    INT32U cnt = 1;
    Write(pContext->i2cHandle, &(pContext->registerAddr), &cnt);
    Read(pContext->i2cHandle, pBuffer, pCount); //read the requested number of registers
         
    return PJDF_ERR_NONE;
}


// SInce it's not crear if multiple registers can be written in one transaction
//this function will write only first value in pBuffer an ignore the rest.
static PjdfErrCode WriteTouch(DriverInternal *pDriver, void* pBuffer, INT32U* pCount)
{
    PjdfContextTouch *pContext = (PjdfContextTouch*) pDriver->deviceContext;
    if (pContext == NULL) 
      return PJDF_ERR_DEVICE_NOT_OPEN;
    uint8_t buffer[2] = {pContext->registerAddr, *((uint8_t*)pBuffer)};
    INT32U cnt = 2;
    Write(pContext->i2cHandle, buffer, &cnt);
    return PJDF_ERR_NONE;
}

// IoctlTouch
// Handles the request codes defined in pjdfLctTouchSpi.h
static PjdfErrCode IoctlTouch(DriverInternal *pDriver, INT8U request, void* pArgs, INT32U* pSize)
{
    PjdfContextTouch *pContext = (PjdfContextTouch*) pDriver->deviceContext;
    if (pContext == NULL) 
      return PJDF_ERR_DEVICE_NOT_OPEN;
    switch (request)
    {
      case PJDF_CTRL_TOUCH_SET_REGISTER:
        pContext->registerAddr = *((uint8_t*)pArgs);
        break;
      //TODO: Add specific requests handling
      default:
        return PJDF_ERR_UNKNOWN_CTRL_REQUEST;
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
  
    pDriver->deviceContext = (void*) &touch1Context;     //get driver context
    //Open the I2C1 port required to talk to this device.
    HANDLE hI2C = Open(PJDF_DEVICE_ID_I2C1, 0);
    if (!PJDF_IS_VALID_HANDLE(hI2C)) while(1);
    touch1Context.i2cHandle = hI2C;
    //set the device slave address
    uint8_t slaveAddr = FT6206_ADDR;
    Ioctl(hI2C, PJDF_CTRL_I2C_SET_SLAVE_ADDR, &slaveAddr, (INT32U*)0);   
  
    // Assign implemented functions to the interface pointers
    pDriver->Open = OpenTouch;
    pDriver->Close = CloseTouch;
    pDriver->Read = ReadTouch;
    pDriver->Write = WriteTouch;
    pDriver->Ioctl = IoctlTouch;
    
    pDriver->initialized = OS_TRUE;
    return PJDF_ERR_NONE;
}
