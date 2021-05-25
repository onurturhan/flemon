/**[txh]********************************************************************

  Copyright (c) 2007 Diego J. Brengi <Brengi at inti gob ar>
  Copyright (c) 2007 Instituto Nacional de Tecnología Industrial
  Bs. As. Argentine.
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA

  Description:
  Analizer : Simple protocol analyzer for serial comunication between
             FPGA and Debug software (flemon or grmon).
             Take data from STDIN and print it nicely.
             Use with intercepttty.
  Usage example:
                $flemon  -D /dev/ttyp0
                $interceptty /dev/ttyS0 -p /dev/ptyp0 | flemonspy
  Note: Assuming your compiler use 32 bits for int and unsigned types.

***************************************************************************/
#define _GNU_SOURCE

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define RX 1
#define TX 0

int main(int argc, char *argv[])
{
 char dir,pdir, * p=NULL, *pget;
 int val, word=0, lenght, command, wrcnt=0, rdcnt=0, btx=0,brx=0;
 size_t len=0; ssize_t read;
 enum stage { LENGHT, ADDR1,ADDR2,ADDR3,ADDR4, DATA} state;

 printf("Flemonspy. Basic analizer \n");
 state=LENGHT;
 while ((read = getline(&pget, &len, stdin)) != -1)
  {
        p=pget;
        //Direction
        if(p[0]=='>') { dir=RX; p++; }
          else if(p[0]=='<') { dir=TX; p++; }
               else { break;}
        //Get value
        val=strtol(p,NULL,0);
       //Print received data (FPGA to PC)
       if(dir==RX)
       {
       brx++;
       if(pdir!=RX) { word=0; printf("\n    Resp: ");}
       printf("%02X",val);
       if (word>=3) {printf (" "); word=0;}
          else word++;
       pdir=RX;
       }
       //Print transmited data (PC to FPGA)
       if(dir==TX)
       {
       btx++;
       switch (state)
        {
         case LENGHT:    if(val&0x80) {
                                       command=val;
                                       lenght=(val&0x3f) +1;
                                       lenght*=4; //one word is 4 bytes long.
                                       printf("\n%03d %s%02d(%02X) ",
                                       (val&0x40)?wrcnt++:rdcnt++,
                                       (val&0x40)?"Wr":"Rd",lenght/4 ,val);
                                       state++;
                                       }
                         else  printf("\nERROR: %02X",val); //sync byte or maybe lost something!
                         break;
         case ADDR1 :     printf("Addr[%02X",val); state++; break;
         case ADDR2 :
         case ADDR3 :     printf("%02X",val); state++; break;
         case ADDR4 :     printf("%02X] ",val);
                          if(command&0x40) {state++;}  //Sending data
                            else  {state=LENGHT;} ;    //Receive
                           word=0; break;
         case DATA  :     printf("%02X",val);
                          lenght--;
                          if (lenght==0) {state=LENGHT;}
                          if (word>=3) {printf (" "); word=0;} //word separation
                             else word++;
                          break;
         }
       pdir=TX;
       }
    fflush(stdout);
  }
 printf("\n Final report: Read commands %d , Write commands %d\n",rdcnt-1,wrcnt-1);
 printf(" Bytes: transmited %d, received %d, Total %d\n",btx,brx,btx+brx);
 return 0;
}
