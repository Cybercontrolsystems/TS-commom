/*******************************************************************************
* Program: 
*    DIO Set and Get (dio.c)
*    Technologic Systems TS-7500
* 
* Summary:
*   This program will return the temperature in celcius when called.
*   Note: The scripted version of gettemp (ts7500.subr) does not round off
*   whereas this .c version does.  Notice careful semaphore usage (sbuslock,
*   sbusunlock) within main.
*
* Usage:
*   ./gettemp
*
* Compile with:
*   gcc -mcpu=arm9 gettemp.c sbus.c -o dio
*******************************************************************************/
#include "sbus.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

/*******************************************************************************
* Main
*   A sample program on how to use the gettemp function of the sbus API.
*******************************************************************************/
int main()
{
   float theTemp = 0;
   
   sbuslock();
   theTemp = gettemp();
   sbusunlock();
   
   printf("%.1f\n", theTemp);

   return 0;
}
