/*
 * University of Washington
 * Certificate in Embedded and Real-Time Systems
 *
 * uDebugger homework
 */


#include "bsp.h"
#include "print.h"


char clr_scrn[] = { 27, 91, 50, 74, 0 };              // esc[2J
/* Public variables ----------------------------------------------------------*/
PRINT_DEFINEBUFFER();           /* required for lightweight sprintf */
/* Private prototype ---------------------------------------------------------*/

void SystemInit(void);

// Generates a fault exception
void GenerateFault()
{
    static int count = 0;

    count++;

    if (count > 10)
    {
        PrintString("\nSuccess!\n");
        while (1);
    }

    /* TODO: Modify the code to cause a data fetch bus fault by reading */
    /* memory at an invalid address. */
    int *a = (int*) 0x06000000;
    int b = *a;
    b = b; // This line just avoids a compiler warning

    PrintString("\nFail! Should not arrive here.\n");
    while (1);
    
}

void main() {
    Hw_init();
    
    PrintString(clr_scrn); /* Clear entire screen */
    PrintString("University of Washington - Debugger Test Application \n");

    GenerateFault();

    PrintString("\nFail! Should not arrive here.\n");
    while(1);
}


void SystemInit(void) {
  // System init is called from the assembly .s file
  // We will configure the clocks in hw_init
  asm("nop");
}
  