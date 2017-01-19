
    EXTERN currentSP
    EXTERN scheduler

    PUBLIC TaskSwitch

EXC_RETURN_THREAD_MODE EQU 0xFFFFFFF9

    SECTION .text:CODE:REORDER:NOROOT(2)
    THUMB


/*
 *  Context switching interrupt handler.
 *
 *  This is the handler for the PendSV exception.
 *
 *  Expected state on entry:
 *
 *      The Cortex-M4 exception handling hardware has pushed the 
 *      following on the stack using the Main stack pointer:
 *      
 *        xPSR
 *        PC
 *        LR
 *        R12
 *        R3
 *        R2
 *        R1
 *        R0 <-- SP_Main
 *   
 */
TaskSwitch
    // Disable interrupts
    MOV         r0, #1
    msr         FAULTMASK, r0
    
    // Save registers R4-R11 using register SP
    push        {r4-r11}
    
    // Set up argument for procedure call by copying SP to R0
    mov         r0, sp
    
    // Call scheduler(uint_32 sp) to save the current SP and determine the next
    // value of currentSP. It takes its input argument sp from R0.
    bl          scheduler
    
    // Load address of currentSP into R0
    ldr         r0, =currentSP
    ldr         r0, [r0]
    
    // Load the value of currentSP into SP
    mov         sp, r0
    
    // Load registers R4-R11 using SP
    pop         {r4-r11}
    
    // Ensure that we return from exception Handler Mode to Thread Mode by 
    // loading the value EXC_RETURN_THREAD_MODE into LR
    ldr         lr, =EXC_RETURN_THREAD_MODE
    
    // Enable interrupts
    //[RC] no need, FAULTMASK will be cleared by the hardware on return from the ISR
    
    // Branch using LR to return from exception while atomically restoring 
    // registers R0-R3, R12, LR, PC and xPSR
    BX LR
    

    END
