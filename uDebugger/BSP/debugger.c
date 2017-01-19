#include <stdint.h>
#include "print.h"

/*
 *
 * Part of a fault exception handler. Prints the given register values.
 * pc: the value of the program counter when the fault occurred.
 * lr: the value of the link register when the fault occurred.
 *
 */
void FaultPrint(uint32_t pc, uint32_t lr)
{
    // TODO: Print an error message specifying the PC and LR values when the fault occurred
    PrintString("\nHard Fault Handler data:");
    PrintString("\n\tPC Register:");
    PrintHex(pc);
    PrintString("\n\tLR Register:");
    PrintHex(lr);
}

//This function prints data in the exception stack frame and the current (in ISR) IPSR value.
void PrintExceptionStackFrame(uint32_t* stackTop, uint32_t iPsr){
  PrintString("\nHardFault handler stack frame:");
  PrintString("\n\tR0-R3: ");
  for(int i = 0; i < 4; i++){
     PrintHex(stackTop[i]);
     PrintByte(' ');
  }
  PrintString("\n\tR12: ");
  PrintHex(stackTop[4]);
  PrintString("\n\tLR: ");
  PrintHex(stackTop[5]);
  PrintString("\n\tPC: ");
  PrintHex(stackTop[6]);
  PrintString("\n\tPSR: ");
  PrintHex(stackTop[7]);
  PrintString("\nCurrent IPSR: ");
  PrintHex(iPsr);
}