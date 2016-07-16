/*******************************************************************************
* Program: 
*    Get DIO (dio.c)
*    Technologic Systems TS-7500 with TS-752 Development Board
* 
* Summary:
*   This program will accept any pin number between 5 and 40 and attempt to get
* or set those pins in a c program rather than scripted.  You will need the 
* TS-7500 and TS-752 development board. Although this program will enable the 
* use of said pins, it will primarily enable the use of the 8 Inputs, 3 Outputs,
* and Relays on the TS-752.  Keep in mind that if a GND or PWR pin is read (or
* something else unlogical, we don't necessarily care about the output because 
* it could be simply "junk". 
*   Notice careful semaphore usage (sbuslock, sbusunlock) within main.
*
* Usage:
*   ./dio <get|set> <pin#> <set_value (0|1|2)>
*
* 0 - GND
* 1 - 3.3V
* 2 - Z (High Impedance)
*
* Examples:
*   To read an input pin (such as 1 through 8 on the TS-752):
*      ts7500:~/sbus# ./dio get 40
*      Result of getdiopin(38) is: 1 
*
*   To set an output pin (such as 1 through 3 or relays on the TS-752):
*      ts7500:~/sbus# ./dio set 33 0
*      Pin#33 has been set to 0   [You may verify with DVM]
*
* Compile with:
*   gcc -mcpu=arm9 dio.c sbus.c -o dio
*******************************************************************************/
#include "sbus.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define DIO_Z 2

/*******************************************************************************
* Main: accept input from the command line and act accordingly.
*******************************************************************************/
int main(int argc, char **argv)
{
   int pin;
   int val;
   int returnedValue;
         
   // Check for invalid command line arguments
   if ((argc > 4) | (argc < 3))
   {
       printf("Usage: %s <get|set> <pin#> <set_value (0|1|2)>\n", argv[0]);
       return 1;
   }
   
   // We only want to get val if there are more than 3 command line arguments
   if (argc == 3)
      pin = strtoul(argv[2], NULL, 0);
   else
   {
      pin = strtoul(argv[2], NULL, 0);
      val = strtoul(argv[3], NULL, 0);
   }
   
   // If anything other than pins 5 through 40, fail program
   assert(pin <= 40 && pin >= 5);

   // Parse through the command line arguments, check for valid inputs, and exec
   if (!(strcmp(argv[1], "get")) && (argc == 3))
   {
      sbuslock();
      returnedValue = getdiopin(pin);
      sbusunlock();
      
      printf("pin#%d = %d \n", pin, returnedValue);
   }
   else if(!(strcmp(argv[1], "set")) && (argc == 4) && (val <= 2))
   {
      sbuslock();
      setdiopin(pin, val);
      sbusunlock();
      
      printf("pin#%d set to %d\n", pin, val);
   }   
   else
   {
      printf("Usage: %s <get|set> <pin#> <set_value (0|1|2)>\n", argv[0]);
      return 1;
   }
   return 0;
}
