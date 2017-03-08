#include "bsp.h"
#include "pjdf.h"
#include "pjdfInternal.h"
#include "bspI2c.h"
#include <pjdfCtrlI2C.h>

// Control registers etc for SPI hardware
typedef struct _PjdfContextI2C
{
    uint8_t slaveAddr;               // Address of the slave device on the bus
    I2C_TypeDef *i2cRegisters;          //I2C port definition
    
} PjdfContextI2C;

static PjdfContextI2C i2cContext = { 
    0x0,
    I2C1
};

OS_CPU_SR cpu_sr = 0u;


// Open I2C port device
static PjdfErrCode OpenI2C(DriverInternal *pDriver, INT8U flags)
{
    PjdfContextI2C *pContext = (PjdfContextI2C*) pDriver->deviceContext;
    if (pContext == NULL) while(1);
    //nothing to do
    return PJDF_ERR_NONE;
}

// Close I2C port
static PjdfErrCode CloseI2C(DriverInternal *pDriver)
{
    PjdfContextI2C *pContext = (PjdfContextI2C*) pDriver->deviceContext;
    if (pContext == NULL) while(1);
    if(pContext->i2cRegisters == I2C1)
      I2C1_close();
    else
      return PJDF_ERR_INVALID_HANDLE;         //device not supported
    return PJDF_ERR_NONE;
}

//Reads a string of bytes from the I2C port as a single transaction
static PjdfErrCode ReadI2C(DriverInternal *pDriver, void* pBuffer, INT32U* pCount)
{
    PjdfContextI2C *pContext = (PjdfContextI2C*) pDriver->deviceContext;
    if (pContext == NULL) while(1);

    OS_ENTER_CRITICAL();
    I2C_start(pContext->i2cRegisters, (pContext->slaveAddr)<<1, I2C_Direction_Receiver);
    uint8_t i;
    for (i=0; i < (*pCount - 1); i++)
      ((uint8_t*)pBuffer)[i] = I2C_read_ack(pContext->i2cRegisters);
    ((uint8_t*)pBuffer)[i] = I2C_read_nack(pContext->i2cRegisters);
    OS_EXIT_CRITICAL();

    return PJDF_ERR_NONE;
}

static PjdfErrCode WriteI2C(DriverInternal *pDriver, void* pBuffer, INT32U* pCount)
{
    PjdfContextI2C *pContext = (PjdfContextI2C*) pDriver->deviceContext;
    if (pContext == NULL) while(1);
    OS_ENTER_CRITICAL();

    I2C_start(pContext->i2cRegisters, (pContext->slaveAddr)<<1, I2C_Direction_Transmitter);
    uint8_t i;
    for (i=0; i < (*pCount); i++)
      I2C_write(pContext->i2cRegisters, ((uint8_t*)pBuffer)[i]);
    I2C_stop(pContext->i2cRegisters);

    OS_EXIT_CRITICAL();

    return PJDF_ERR_NONE;
}

// Handles the request codes defined in pjdfCtrlI2C.h
static PjdfErrCode IoctlI2C(DriverInternal *pDriver, INT8U request, void* pArgs, INT32U* pSize)
{
    INT8U osErr;
    PjdfContextI2C *pContext = (PjdfContextI2C*) pDriver->deviceContext;
    if (pContext == NULL) while(1);
    switch (request)
    {
      case PJDF_CTRL_I2C_SET_SLAVE_ADDR:
        pContext->slaveAddr = ((uint8_t*)pArgs)[0];
        break;
      default:
        return PJDF_ERR_UNKNOWN_CTRL_REQUEST;
        break;
    }
    return PJDF_ERR_NONE;
}


// Initializes the Touch driver.
PjdfErrCode InitI2C(DriverInternal *pDriver, char *pName)
{   
    if (strcmp (pName, pDriver->pName) != 0) while(1); // pName should have been initialized in driversInternal[] declaration
    
    // Initialize semaphore for serializing operations on the device 
    pDriver->sem = OSSemCreate(1); 
    if (pDriver->sem == NULL) while (1);  // not enough semaphores available
    pDriver->refCount = 0; // initial number of Open handles to the device
    pDriver->maxRefCount = 1; // only one open handle allowed
  
    //TODO: Add required initialization
    pDriver->deviceContext = (void*) &i2cContext;     //get driver context
    I2C1_init();         //Initialize the device. This is I2C1-specific driver, won't work for I2C2
    
    // Assign implemented functions to the interface pointers
    pDriver->Open = OpenI2C;
    pDriver->Close = CloseI2C;
    pDriver->Read = ReadI2C;
    pDriver->Write = WriteI2C;
    pDriver->Ioctl = IoctlI2C;
    
    pDriver->initialized = OS_TRUE;
    return PJDF_ERR_NONE;
}
