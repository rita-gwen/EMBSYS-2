#include "bsp.h"
#include "print.h"

OS_FLAG_GRP *intFlags = 0;


/****
* Interrupt handler for the touch screen interrupt. It sets a touch flag
* in the interrupt flags group and exits.
*/
void  EXTI10Thru15IrqHandler (void)
{
    OS_CPU_SR  cpu_sr;
    INT8U err;

    OSIntEnter();
    OS_ENTER_CRITICAL();                                        /* Tell uC/OS-II that we are starting an ISR            */
    EXTI_ClearITPendingBit(EXTI_Line10);
    OSFlagPost(intFlags, INT_FLAG_TOUCH_BIT, OS_FLAG_SET, &err);
    OS_EXIT_CRITICAL();

    OSIntExit();                                                /* Tell uC/OS-II that we are leaving the ISR            */
}

/***
* Initializing touch screen interrupt line
*/
void Init_TouchInterrupt(){
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    /* Enable clock for GPIOA */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    /* Enable clock for SYSCFG */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    /* Set pin as input */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Tell system that you will use PA10 for EXTI_Line10 */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource10);
    
    /* PD0 is connected to EXTI_Line0 */
    EXTI_InitStruct.EXTI_Line = EXTI_Line10;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);
 
    /* Add IRQ vector to NVIC */
    /* PD10 is connected to EXTI_Line10, which has EXTI15_10_IRQn vector */
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    /* Set priority */
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x05;
    /* Set sub priority */
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    /* Enable interrupt */
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&NVIC_InitStruct);  
}

OS_FLAG_GRP* InitInterruptFlags(){
  
    char buf[BUFSIZE];
    INT8U err_code;
    
    intFlags = OSFlagCreate((OS_FLAGS)(0x00), &err_code);
    if(err_code != OS_ERR_NONE){
      	PrintWithBuf(buf, BUFSIZE, "Cannot create flag group.\n");
        while(1);
    }
    return intFlags;
}

