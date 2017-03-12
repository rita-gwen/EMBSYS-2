
#ifdef __cplusplus
 extern "C" {
#endif

#define INT_FLAG_TOUCH_BIT      0x1


void  EXTI10Thru15IrqHandler(void);
OS_FLAG_GRP* InitInterruptFlags();
void Init_TouchInterrupt();

#ifdef __cplusplus
 }
#endif

