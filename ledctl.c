/*******************************************************************************
* Program: 
*    LED Control (ledctl.c)
*    Technologic Systems TS-7500
* 
* Summary:
*   This program simlpy turns on or off the green or red LEDs on the TS-7500.
*   Notice careful semaphore usage (sbuslock, sbusunlock) within main.
*
* Usage:
*   ./ledctl <grn|red|tgl> <0|1>
*
* Examples:
*   ./ledctl grn 1 
*      Turns green LED on
*   ./ledctl tgl
*      Toggles both green and red LEDs
*
* Compile with:
*   gcc -mcpu=arm9 ledctl.c sbus.c -o ledctl
*******************************************************************************/
#include "sbus.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
   int pwr;
   
   if (argc == 3)
   {
      pwr = strtoul(argv[2], NULL, 0);
      assert(pwr <= 1);
   }
   else if ((argc == 1) || ((argc == 2) && (strcmp(argv[1],"tgl"))))
   {
      printf("Usage: %s <grn|red|tgl> <1|0>\n", argv[0]);
      return 1;
   }

   if (!(strcmp(argv[1], "grn")))
   {
      if(pwr == 0)
      {
         sbuslock();
         sbus_poke16(0x62, sbus_peek16(0x62) & ~(0x8000));
         sbusunlock();
      }
      else
      {
         sbuslock();
         sbus_poke16(0x62, sbus_peek16(0x62) | 0x8000);
         sbusunlock();
      }
   }
   else if (!(strcmp(argv[1], "red")))
   {
      if(pwr == 0)
      {
         sbuslock();
         sbus_poke16(0x62, sbus_peek16(0x62) & ~(0x4000));
         sbusunlock();
      }
      else
      {
         sbuslock();
         sbus_poke16(0x62, sbus_peek16(0x62) | 0x4000);
         sbusunlock();
      }
   }
   else if (!(strcmp(argv[1], "tgl")))
   {
      sbuslock();
      sbus_poke16(0x62, sbus_peek16(0x62) ^ 0x4000);
      sbus_poke16(0x62, sbus_peek16(0x62) ^ 0x8000);
      sbusunlock();
   }
   else if (!(strcmp(argv[1], "")))
   {
      printf("there");
   }
   else
   {
      printf("Usage: %s <grn|red|tgl> <1|0>\n", argv[0]);
   }
return 0;
}
