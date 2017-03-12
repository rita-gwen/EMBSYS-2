/*************************************************** 
  This is a library for the Adafruit Capacitive Touch Screens

  ----> http://www.adafruit.com/products/1947
 
  Check out the links above for our tutorials and wiring diagrams
  This chipset uses I2C to communicate

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_FT6206.h>
#include "bspI2c.h"

#if defined(__SAM3X8E__)
    #define Wire Wire1
#endif

void delay(uint32_t);

//OS_CPU_SR cpu_sr = 0u;


/**************************************************************************/
/*! 
    @brief  Instantiates a new FT6206 class
*/
/**************************************************************************/
// I2C, no address adjustments or pins
Adafruit_FT6206::Adafruit_FT6206() {
  lastTouchTime = 0;
}


/**************************************************************************/
/*! 
    @brief  Setups the HW
*/
/**************************************************************************/
boolean Adafruit_FT6206::begin(uint8_t threshhold) {
  //Initialize driver handle
  hTouch = Open(PJDF_DEVICE_ID_LCD_TOUCH, (INT8U)0x0);
  // change threshhold to be higher/lower
  writeRegister(FT6206_REG_THRESHHOLD, threshhold);
  
  uint8_t reg_val = readRegister(FT6206_REG_VENDID);
  if(reg_val != 17) return false;
  reg_val = readRegister(FT6206_REG_CHIPID);
  if(reg_val != 6) return false;

  intFlags = InitInterruptFlags();
  
  return true;
}

boolean Adafruit_FT6206::touched(void) {
  
  uint8_t reg_val = readRegister(FT6206_REG_NUMTOUCHES);
  if ((reg_val == 1) || (reg_val == 2)) return true;
  return false;
}

void Adafruit_FT6206::waitForTouch(void){
    INT8U err;
    while(1){
      //Wait for the touch event to occur
      OSFlagPend(intFlags, INT_FLAG_TOUCH_BIT, OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err);
      INT32U currentTime = OSTimeGet();
      if(currentTime - lastTouchTime < FT6206_TOUCH_DEBOUNCE_TIME)
        lastTouchTime = currentTime;
      else {
        lastTouchTime = currentTime;
        break;
      }
  }
}

/*****************************/

void Adafruit_FT6206::readData(uint16_t *x, uint16_t *y) {

  uint8_t i2cdat[16];
  INT32U cnt = 16;
  uint8_t reg = 0x0;
  Ioctl(hTouch, PJDF_CTRL_TOUCH_SET_REGISTER, &reg, (INT32U)0x0);       //set the register
  Read(hTouch, &i2cdat, &cnt); 
  
  touches = i2cdat[0x02];

  //Serial.println(touches);
  if (touches > 2) {
    touches = 0;
    *x = *y = 0;
  }
  if (touches == 0) {
    *x = *y = 0;
    return;
  }

  //Serial.print("# Touches: "); Serial.print(touches);
  for (uint8_t i=0; i<2; i++) {
      touchX[i] = i2cdat[0x03 + i*6] & 0x0F;
      touchX[i] <<= 8;
      touchX[i] |= i2cdat[0x04 + i*6]; 
      touchY[i] = i2cdat[0x05 + i*6] & 0x0F;
      touchY[i] <<= 8;
      touchY[i] |= i2cdat[0x06 + i*6];
      touchID[i] = i2cdat[0x05 + i*6] >> 4;
    }
    *x = touchX[0]; *y = touchY[0];
}

//convenience methods to hide the driver complexity.
void Adafruit_FT6206::writeRegister(uint8_t reg_addr, uint8_t val){
  Ioctl(hTouch, PJDF_CTRL_TOUCH_SET_REGISTER, &reg_addr, (INT32U)0x0);       //set the register
  Write(hTouch, &val, (INT32U)0x0);      //write the register
}
uint8_t Adafruit_FT6206::readRegister(uint8_t reg_addr){
  INT32U cnt = 1;
  uint8_t reg_val;
  Ioctl(hTouch, PJDF_CTRL_TOUCH_SET_REGISTER, &reg_addr, (INT32U)0x0);       //set the register
  Read(hTouch, &reg_val, &cnt); 
  return reg_val;
}


TS_Point Adafruit_FT6206::getPoint(void) {
  uint16_t x, y;
  readData(&x, &y);
  return TS_Point(x, y, 1);
}


/****************/

TS_Point::TS_Point(void) {
  x = y = 0;
}

TS_Point::TS_Point(int16_t x0, int16_t y0, int16_t z0) {
  x = x0;
  y = y0;
  z = z0;
}

bool TS_Point::operator==(TS_Point p1) {
  return  ((p1.x == x) && (p1.y == y) && (p1.z == z));
}

bool TS_Point::operator!=(TS_Point p1) {
  return  ((p1.x != x) || (p1.y != y) || (p1.z != z));
}
