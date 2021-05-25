/**[txh]********************************************************************

  Copyright (c) 2008-2009 Diego J. Brengi <Brengi at inti gob ar>
  Copyright (c) 2008-2009 Instituto Nacional de Tecnología Industrial
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

  Description: CFI Flash handling.
***************************************************************************/
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>
#include <string.h>

#include "flemon.h"
#include "grlib.h"
#include "flash.h"

//CFI (Common Flash Interface) commands from INTEL datasheet
#define CMD_LOCK         0x60
#define CMD_READARRAY    0xFF
#define CMD_READID       0x90
#define CMD_READQUERY    0x98
#define CMD_READSTATUS   0x70
#define CMD_CLEARSTATUS  0x50
#define CMD_WRITEBUFF    0xE8
#define CMD_WORDPROG     0x40
#define CMD_BLOCKERASE   0x20
#define CMD2_BLOCKERASE  0xD0
#define CMD_BLOCKERASEPS 0XB0
#define CMD_SETLOCKBIT   0X60
#define CMD2_SETLOCKBIT  0X01
#define CMD_CLEARLOCKBIT 0X60
#define CMD2_CLEARLOCKBIT 0xD0

#define CPPFIO_I
#define CPPFIO_Chrono
#define CPPFIO_File
#define CPPFIO_PTimer

#define MINBUFF 10

#include <cppfio.h>
using namespace cppfio;

int flashmem::search()
{
      if(current_flash_mode>=search_ok) return SUCCESS;
      
      //TODO: -Improve error handling here...
      //      -This only works for MCTRL. Other cores?      
      
//      flashcore_apb=search_core (mygrlib, 0x04, 0x0f, 0, 0); //Search APB core

      flashcore_apb=mygrlib->search_core(VENDORID_ESA, MCTRL, 0,0);
      
      
     if (flashcore_apb==NULL) {printf ("Error searching flash in APB bus\n");
     return ERROR;}

      //MCFG1 is for controlling Flash and ROM memories.
      mcfg1_address=flashcore_apb->get_startaddress(0);

      //Search AHB core  TODO: Use the collapse info to find matched APB core)
      flashcore_ahb=mygrlib->search_core (VENDORID_ESA, MCTRL, 1, 0);
      if (flashcore_ahb==NULL) {printf ("Error searching flash in AHB bus\n");
      return ERROR;}

      //This is the base address of the flash memory.
      flash_address=flashcore_ahb->get_startaddress(0);
      flash_address_end=flashcore_ahb->get_endaddress(0);
      current_flash_mode=search_ok;
return SUCCESS;
}

void flashmem::print_search()
{
     if(current_flash_mode<search_ok)
     {
     printf("No search!\n");
     return;
     }
     printf("Flash Core:\n");
     flashcore_ahb->lprint(); printf("\n");
     flashcore_apb->lprint(); printf("\n");
}

unsigned flashmem::read_mcfg1()
{
     unsigned int res, mbuf [MINBUFF];
     
     res=read_ahb (mbuf,1, mcfg1_address);
     if (res!=SUCCESS) { printf ("\nError reading MCFG1\n"); return mcfg1_value; }
     mcfg1_value=mbuf[0];

     //Bus width info is in bits 9-8 of MCFG1 00=8bits 01=16bits 10=32bits
     //Initial values on reset are valid and configured in the VHDL BWITH[1:0].
     mcfg1_pwen=(mcfg1_value&0x800)>>11;
     mcfg1_pwidth=(mcfg1_value&0x300)>>8;
     mcfg1_pwws=(mcfg1_value&0xF0)>>4;
     mcfg1_prws=mcfg1_value&0xf;
     
   return mcfg1_value;
}

void flashmem::print_mcfg1()
{
  if(current_flash_mode<search_ok) return;
      read_mcfg1();
      printf("MCFG1 = 0x%08X\n",mcfg1_value);
      printf("MCTRL mode: ");
      switch (mcfg1_pwidth)
       { case 0 : printf("8 bit  flash on D[31:24] \n"); break;
         case 1 : printf("16 bit flash on D[31:16]\n"); break;
         case 2 : printf("32-bit flash on D[31:0]\n"); break;
         default : printf("Unknown\n"); break;
       }
     printf("Prom Waitstates:  read=%02d   write=%02d\n",mcfg1_prws, mcfg1_pwws);
     printf("Prom write enable: %s\n",(mcfg1_pwen)?"YES":"NO");
}

void flashmem::enable_write(int enable)
{
//  if(current_flash_mode<search_ok) return;
  unsigned val [MINBUFF];
  //Read cfg1
  val[0]=read_mcfg1();
  //Return if no change is needed.
//  if(enable==mcfg1_pwen) return;
  if(enable)
     val[0]|=0x800;
  else
     val[0]&=0xFFFFF7FF;

  //Do change
  write_ahb(val,1,mcfg1_address);
  //Keep vars sinchronized.
  mcfg1_value=val[0];
  mcfg1_pwen=enable;
}

void flashmem::waitstates(uchar readws, uchar writews)
{
  unsigned int val[MINBUFF];
  //Read cfg1
  val[0]=read_mcfg1();
  readws&=0x0f; writews&=0x0f; //Waitstates from 0 to 15
  val[0]&=0xff00;
  val[0]|= readws;
  val[0]|=(writews<<4);
  //Do change
  write_ahb(val,1,mcfg1_address);
  //Keep vars sinchronized.
  mcfg1_value=val[0];
}
/*************************************************************************
 Sends commands to CFI flash
**************************************************************************/

int flashmem::send_command(unsigned char comm)
{
return send_command(0x00,comm,0x00000000);
}

int flashmem::send_command(unsigned char comm, unsigned char comm2)
{
return send_command(comm,comm2,0x00000000);
}

int flashmem::send_command(unsigned char comm, unsigned char comm2, unsigned offset)
{
if (invalidoffset(offset)) return ERROR;
int rval;
unsigned txval[MINBUFF];
 switch (mcfg1_pwidth)
 {
  case 0x0: txval[0]=(comm<<8)|comm2; txval[0]|=0xFFFF0000;
            rval=write_ahb(txval,1,flash_address+offset);
            break;
  case 0x1: txval[0]=(comm<<16)|comm2;
            rval=write_ahb(txval,1,flash_address+offset);
            break;
  case 0x2: txval[0]=comm;
            txval[1]=comm2;
            rval=write_ahb(txval,2,flash_address+offset);
            break;
  default: printf("Send_command error.\n");
 }
return rval;
}

int flashmem::send_command32(unsigned comm)
{
return send_command(0x00,comm,0x00000000);
}


int flashmem::CFI_valid()
{
  char mbuf[6];
  if(current_flash_mode<search_ok) return ERROR;

//Se if it is CFI compliant?
 send_command(CMD_CLEARSTATUS, CMD_READQUERY);

 mbuf[0]=read_flash(0x10);
 mbuf[1]=read_flash(0x11);
 mbuf[2]=read_flash(0x12);
 send_command((unsigned char)CMD_READARRAY);

//See  AP-646 pages 5 to 6 form INTEL "Common Flash Interface and command sets"
      if( strncmp (mbuf,"QRY",3)==0 )
         {
         current_flash_mode=cfi_ok;
         return SUCCESS;
         }
return ERROR;
}


//Prints CFI
int flashmem::CFI_dump (unsigned soffset, unsigned eoffset)
{ //This is not right. Limits are affected by other factors (see read_flash)

 if(soffset>(flash_address_end-flash_address)) return ERROR;  //start offset too high.
 if(eoffset>(flash_address_end-flash_address)) return ERROR;  //end offset too high.
 //Put memory in Query mode

send_command(CMD_CLEARSTATUS, CMD_READQUERY);
unsigned cc;
   for( cc=soffset; cc<=eoffset;cc++)
         {
          unsigned  mem;
          mem=read_flash(cc);
          printf ("Offsett 0x%X = [%02X]  %c \n",cc,mem,(isprint((char)mem))?mem:' ');
         }
 //Return memory to read array mode
 send_command((unsigned char)CMD_READARRAY);
return SUCCESS;
}

/*************************************************
Read CFI flash
**************************************************/
unsigned  flashmem::read_flash (unsigned offset)
{
unsigned mreading [MINBUFF], off2;
offset<<=1; //all readings are repeated.

off2=offset&0xfffffffC; //clean two LSB address bits (simple reading gets 4 bytes)

 read_ahb (mreading,1, flash_address+off2);

//selecting which byte is the right one
 switch (offset&0x3)
  {
   case 0: mreading[0]>>=16; break;;
   case 1: mreading[0]>>=24; break;;
   case 2: mreading[0]>>=0; break;;
   case 3: mreading[0]>>=8; break;;
  }
return mreading[0] & 0xff;
}

int  flashmem::rawread (unsigned offset, unsigned * buff, unsigned words)
{

 if(current_flash_mode<cfi_ok)
   { printf("CFI scan not completed!\n"); return ERROR;}

 if(words>63 || words==0)
   { printf("Bad number of words for reading!\n"); return ERROR;}

return  read_ahb (buff,words, flash_address + offset);
}

int flashmem::statusreg()
{
   send_command(CMD_READSTATUS);
   statusregval= read_flash(0x0)&0xFF;
return statusregval;
}

void flashmem::printstatusreg()
{
   if ( (statusregval&0x80) ==0)
     { printf("Write state machine busy or write buffer not available.\n");
       return;
     }
if(statusregval&0x40) printf("Block erase suspended.\n");
if(statusregval&0x20) printf("Error in block erase or clear lock-bits.\n");
if(statusregval&0x10) printf("Program error or error in setting lock-bit.\n");
if(statusregval&0x08) printf("Low programming voltage detected, operation aborted.\n");
if(statusregval&0x04) printf("Program suspended.\n");
if(statusregval&0x02) printf("Block lock bit detected, operation abort.\n");
}

int flashmem::CFI_read()
{ //Reads basic CFI info.

 if(current_flash_mode<search_ok) return ERROR;

 send_command(CMD_CLEARSTATUS,CMD_READQUERY);

 CFI_vendor=read_flash(0x0);
 CFI_device=read_flash(0x1);
 CFI_cset=read_flash(0x13);
 CFI_cset|=(read_flash(0x14)&0xFF)<<8;
 CFI_equery=read_flash(0x15);
 CFI_equery|=(read_flash(0x16)&0xFF)<<8;

 //System Interface Information
 CFI_minvolt=read_flash(0x1B);
 CFI_maxvolt=read_flash(0x1C);
 CFI_vppmax=read_flash(0x1D);
 CFI_vppmin=read_flash(0x1E);

 CFI_sword_typ_wr_to=0x01<<read_flash(0x1F);
 CFI_sword_max_wr_to=0x01<<read_flash(0x23);

  CFI_buff_typ_wr_to=read_flash(0x20);
    if(CFI_buff_typ_wr_to) CFI_buff_typ_wr_to=0x1<<CFI_buff_typ_wr_to;
 CFI_buff_max_wr_to=read_flash(0x24);
    if(CFI_buff_max_wr_to) CFI_buff_max_wr_to=0x1<<CFI_buff_max_wr_to;

 CFI_block_erase_typ_to=0x1<<read_flash(0x21);
 CFI_block_erase_max_to=0x1<<read_flash(0x25);

 CFI_chip_erase_typ_to=read_flash(0x22);
    if(CFI_chip_erase_typ_to) CFI_chip_erase_typ_to=0x1<<CFI_chip_erase_typ_to;
 CFI_chip_erase_max_to=read_flash(0x26);
    if(CFI_chip_erase_max_to) CFI_chip_erase_max_to=0x1<<CFI_chip_erase_max_to;

 //Flash Geometry

  CFI_flashsize=0x1<<read_flash(0x27);

  CFI_devinterface=read_flash(0x28);
  CFI_devinterface|= read_flash(0x29)<<8;

    CFI_bufferbytes=read_flash(0x2A);
  CFI_bufferbytes|=read_flash(0x2B)<<8;

  CFI_bufferbytes=0x1<<CFI_bufferbytes;
  CFI_eblock_regions=read_flash(0x2C);

  CFI_eblock1=read_flash(0x2D)| (read_flash(0x2E)<<8);
  
  CFI_esize1 =read_flash(0x2F)| (read_flash(0x30)<<8);
      if(!CFI_esize1) CFI_esize1bytes=131072;
        else CFI_esize1bytes=CFI_esize1*256;

 send_command((unsigned char)CMD_READARRAY);
 current_flash_mode=cfi_ok;
return SUCCESS;
  
}

int flashmem::CFI_print(bool mode)
{

      if(current_flash_mode<cfi_ok)
      { printf("CFI scan not completed!\n"); return ERROR;}

      printf("CFI Vendor: 0x%02X Device: 0x%02X  ",CFI_vendor , CFI_device);
      switch  (CFI_vendor)
      {
       case 0x89 : printf("INTEL ");
                    switch  (CFI_device)
                    {
                     case 0x16 : printf("32 Mbit\n"); break;
                     case 0x17 : printf("64 Mbit\n"); break;
                     case 0x18 : printf("128 Mbit\n"); break;
                     case 0x1D : printf("256 Mbit\n"); break;
                     default :   printf("Unknown device\n");
                    }
                   break;
       case 0x2C : printf("Micron ");
                    switch  (CFI_device)
                    {
                     case 0x16 : printf("32 Mbit\n"); break;
                     case 0x17 : printf("64 Mbit\n"); break;
                     case 0x18 : printf("128 Mbit\n"); break;
                     default :   printf("Unknown device\n");
                    }
                   break;
       default :   printf("Unknown vendor\n");
      }
      printf("Command set: 0x%04X - ", CFI_cset);
      switch  (CFI_cset)
      {
       case 0   : printf("Not specified.\n"); break;
       case 1   : printf("Intel/Sharp Extended.\n"); break;
       case 2   : printf("AMD/Fujitsu Standard.\n"); break;
       case 3   : printf("Intel Standard.\n"); break;
       case 4   : printf("AMD/Fujitsu Extended.\n"); break;
       case 6   : printf("Windbond Standard\n"); break;
       case 256 : printf("Mitsubishi Standard.\n"); break;
       case 257 : printf("Mitsubishi Extended.\n"); break;
       case 258 : printf("Page write.\n"); break;
       case 512 : printf("Intel Performance Code.\n"); break;
       case 518 : printf("Intel Data.\n"); break;
       default :   printf("Unknown.\n");
      }
    if(mode)
    {
      printf("Extended query table address: 0x%04X\n", CFI_equery);

      printf("--------CFI System interface info---------\n");
      printf("VCC Program/erase voltage (Min): %d.%d Volts\n", CFI_minvolt>>4, CFI_minvolt&0xf);
      printf("VCC program/erase voltage (Max): %d.%d Volts\n", CFI_maxvolt>>4, CFI_maxvolt&0xf);
      if (CFI_vppmax)
      printf("VPP program/erase voltage (Max): %d.%d Volts\n", CFI_vppmax>>4, CFI_vppmax&0xf);
      if (CFI_vppmin)
      printf("VPP program/erase voltage (Min): %d.%d Volts\n", CFI_vppmin>>4, CFI_vppmin&0xf);
      if(CFI_vppmax==0 && CFI_vppmin==0) printf("VPP pin not present.\n");

      printf("Single byte/word write timeout (Typ): %d usec\n",CFI_sword_typ_wr_to);
      printf("Single byte/word write timeout (Max): %d times %d = %d usec\n",
             CFI_sword_max_wr_to, CFI_sword_typ_wr_to, CFI_sword_max_wr_to*CFI_sword_typ_wr_to);

      printf("Buffer byte/word write timeout (Typ): %d usec\n",CFI_buff_typ_wr_to);
      if(CFI_buff_max_wr_to)
        {
         printf("Buffer byte/word write timeout (Max): %d times %d = %d usec\n",
                 CFI_buff_max_wr_to,CFI_buff_typ_wr_to, CFI_buff_max_wr_to*CFI_buff_typ_wr_to);
        }
      if(CFI_buff_typ_wr_to==0 && CFI_buff_max_wr_to==0)
        printf("Buffer byte/word write not supported\n");

      printf("Individual Block erase timeout (Typ): %d msec\n",CFI_block_erase_typ_to);
      printf("Individual Block erase timeout (Max): %d times %d = %d msec\n",
             CFI_block_erase_max_to,CFI_block_erase_typ_to,
             CFI_block_erase_max_to*CFI_block_erase_typ_to);

      if(CFI_chip_erase_typ_to)
        {
         printf("Full chip erase timeout (Typ): %d msec\n",CFI_chip_erase_typ_to);
        }

      if(CFI_chip_erase_max_to)
        {
         printf("Full chip erase timeout (Max): %d times %d = %d msec\n",
                CFI_chip_erase_max_to,CFI_chip_erase_typ_to,
                CFI_chip_erase_max_to*CFI_chip_erase_typ_to);
        }

      if(CFI_chip_erase_max_to==0 && CFI_chip_erase_typ_to==0)
         printf("Full chip erase not supported. \n");
    }
      printf("--------CFI Geometry info---------\n");

      printf("Device size: %d bytes\n",CFI_flashsize);

    if(mode)
    {
      printf("Device interface: 0x%02X - ",CFI_devinterface);

      switch (CFI_devinterface)
       {
        case 0 : printf("x8-only, asynchronous.\n"); break;
        case 1 : printf("x16-only, asynchronous.\n");break;
        case 2 : printf("x8 and x16 via #BYTE, asynchronous.\n");break;
        case 3 : printf("x32-only, asynchronous.\n");break;
        case 4 : printf("x16 and x32 via #WORD, asynchronous.\n");break;
        case 5 : printf("x16 and x32 via #WORD, asynchronous.\n");break;
        default: printf("Unknown.\n");
       }

      printf("Maximum number of bytes in buffer write: %d bytes\n",CFI_bufferbytes);
    }
      printf("Erase block regions: %d \n",CFI_eblock_regions);
      printf("Erase blocks size in region 1: %d bytes (y=%d)\n",CFI_esize1bytes, CFI_esize1);
      printf("Erase blocks in region 1: %d blocks\n",CFI_eblock1+1);
      printf("Total Size: %d bytes \n",(CFI_eblock1+1)*CFI_esize1bytes);      

return SUCCESS;
}

int flashmem::dumpblock (unsigned offset, unsigned endoffset)
{
unsigned buffer[64],ofs, cc;

offset   &=0xFFFFFFFC;
endoffset&=0xFFFFFFFC;
endoffset+=4;
if (endoffset<offset)
   {
   printf("Start offset bigger than end offset\n");
   return ERROR;
   }
if (invalidoffset(offset)||invalidoffset(endoffset))
  { printf("Start or end offsets out of flash memory limits\n");
   return ERROR;
  }
send_command(CMD_READARRAY);
unsigned start=offset, end=endoffset;
//printf("start 0x%08X - 0x%08X \n",offset,endoffset);
 while (start <endoffset)
 {
   if(endoffset-start>63*4) end=start+(63*4);
      else end=endoffset;
   if ( rawread(start,buffer,((end-start)/4)+0)==ERROR)
       { printf("Reading error.\n"); return ERROR;}

   for( cc=0, ofs=start; cc<((end-start)/4)+0;cc++,ofs+=4)
         {
           char pchar;
           printf ("Offsett 0x%08X = [0x%08X] [",ofs+flash_address,buffer[cc]);
           pchar=(buffer[cc]>>24)&0x000000FF;
           (isprint(pchar))?printf("%c", pchar): printf(".");
           pchar=(buffer[cc]>>16)&0x000000FF;
           (isprint(pchar))?printf("%c", pchar): printf(".");
           pchar=(buffer[cc]>>8)&0x000000FF;
           (isprint(pchar))?printf("%c", pchar): printf(".");
           pchar=buffer[cc]&0x000000FF;
           (isprint(pchar))?printf("%c", pchar): printf(".");
           if(outoffset(ofs))
             printf(" Warning, address out of flash size.");
           printf("]\n");
          }
   start=end;
 }
return SUCCESS;
}

/**********************************************************
Flash memory dump to a file
**********************************************************/
int flashmem::dumpfile(unsigned offset, unsigned endoffset, const char *fn)
{
unsigned buffer[64],ofs, cc;
File dump(fn, "w");
int pad=0;
if((endoffset-offset)%4>0) pad=4-((endoffset-offset)%4);
endoffset+=pad;
/*TODO: check if offset or endoffset are not aligned. Now y asume
//it is endoffset*/
offset   &=0xFFFFFFFC;
endoffset&=0xFFFFFFFC;
//endoffset+=4;
if (endoffset<offset) return ERROR;
if (invalidoffset(offset)||invalidoffset(endoffset))
  { printf("Start or end offsets out of flash memory limits\n");
   return ERROR;
  }

send_command(CMD_READARRAY);
unsigned start=offset, end=endoffset;
 while (start <endoffset)
 {
   if(endoffset-start>64*4) end=start+(64*4);
      else end=endoffset;
   if ( rawread(start,buffer,((end-start)/4))==-1)
       { printf("Reading error.\n"); return ERROR;}

   //TODO: Optimize writing all the buffer at once
   for( cc=0, ofs=start; cc<((end-start)/4);cc++,ofs+=4)
         {
           int converted=0;
           //????????
           converted=(buffer[cc]&0xFF000000)>>24;
           converted|=(buffer[cc]&0x00FF0000)>>8;
           converted|=(buffer[cc]&0x0000FF00)<<8;
           converted|=(buffer[cc]&0x000000FF)<<24;
           dump.WriteDWord(converted);
           dump.Flush();
           if(outoffset(ofs))
             printf(" Warning, address out of flash size.");
          }
   start=end; //next reading
 }
return SUCCESS;
}

int flashmem::printlock  (void)
{
  if(current_flash_mode<cfi_ok) return ERROR;
     send_command(CMD_READARRAY);
     send_command(CMD_READID);
       //Show flash erase bank addresses
      printf("-------- MAP -------------\n");
       unsigned flashadr=flash_address;
       for(unsigned block=0; block<CFI_eblock1+1; block++)
       {
         unsigned lockbit;
         lockbit=read_flash((block<<16)|0x2);
         printf("Block %03d  0x%08X - 0x%08X  %s \n",
         block, flashadr, flashadr+CFI_esize1bytes-1,(lockbit&0x1)?"LOCKED":"U");
         flashadr+=CFI_esize1bytes;
       }
     send_command(CMD_READARRAY);
return SUCCESS;
}

int flashmem::unlock  (void)
{
   if(current_flash_mode<cfi_ok) return ERROR;
  
  send_command(CMD_CLEARSTATUS);
  send_command(CMD_CLEARLOCKBIT,CMD2_CLEARLOCKBIT);

    int flag=0;
//I don't know wich timeout should use. I read status or a maximum timeout
    PTimer to(CFI_block_erase_typ_to * CFI_block_erase_max_to * 1000); // single  time-out
    do {
        usleep(CFI_sword_typ_wr_to*CFI_sword_max_wr_to);
        if (to.Reached()) flag=1;
       }
        while(statusreg()!=0x80 && flag==0);
//Datasheet says you can read status register to see when it's done.
  if (flag==1) printstatusreg();
//If not succefull you have to clear status
  send_command(CMD_CLEARSTATUS,CMD_READARRAY);
 return (flag)?ERROR:SUCCESS;
}

int flashmem::lock  (unsigned offset)
{
  if(current_flash_mode<cfi_ok) return ERROR;
  if(invalidoffset(offset))     return ERROR;
  
      send_command(CMD_READARRAY);
      send_command(CMD_SETLOCKBIT,CMD2_SETLOCKBIT,offset&0xFFFFFFFC);
   //Datasheet says you can read status register to see when it's done.
     usleep(CFI_sword_max_wr_to*CFI_sword_typ_wr_to);
    int retval=statusreg();
 //If not succefull you have to clear status
    if(retval!=0x80)
     {  printstatusreg();
//        send_command(CMD_CLEARSTATUS);
       send_command(CMD_CLEARSTATUS,CMD_READARRAY);
     }
    send_command(CMD_READARRAY);
return retval&0x7F;
}

int flashmem::lockall()
{
      unsigned offset=0;
      int ret;
      send_command(CMD_READARRAY);

       for(unsigned block=0; block<CFI_eblock1+1; block++)
       {
        send_command(CMD_SETLOCKBIT,CMD2_SETLOCKBIT,offset);
        //Datasheet says you can read status register to see when it's done.
        //usleep(CFI_sword_max_wr_to*CFI_sword_typ_wr_to);
        offset+=CFI_esize1bytes;
        ret=statusreg();
        //If not succefull you have to clear status
        if(ret!=0x80) break;

       printf("Block lock done: %d of %d\n",block+1 , CFI_eblock1+1);
       }

if(ret!=0x80)
  {  printstatusreg();
     send_command(CMD_CLEARSTATUS);
  }
send_command(CMD_READARRAY);
return ret&0x7F;
}


int flashmem::erasebank(unsigned offset)
{
 int retval;
 if(current_flash_mode<cfi_ok) return ERROR;
 if(invalidoffset(offset)) return ERROR;
 send_command(CMD_CLEARSTATUS);
 send_command(CMD_BLOCKERASE,CMD2_BLOCKERASE,flash_address+(offset&0xFFFFFFFC));

    int flag=0;
//I don't know wich timeout should use. I read status or a maximum timeout
    PTimer to(CFI_block_erase_typ_to*CFI_block_erase_max_to*1000); //  time-out
    do {
        usleep(CFI_block_erase_typ_to*1000);
        if (to.Reached()) flag=1;
       }
        while(statusreg()!=0x80 && flag==0);

 retval=statusreg();
 if(retval!=0x80)
 {
  printstatusreg(); send_command(CMD_CLEARSTATUS);
 }
 send_command(CMD_READARRAY);
return retval&0x7F;
}

int flashmem::erasebank(unsigned start, unsigned nbytes)
{
 if(current_flash_mode<cfi_ok) return ERROR;
 int retval;
 unsigned flashadr=0;
 retval=SUCCESS;
 Chrono timer;
 double prevtime=0;

 if(invalidoffset(start))
 {
 printf("Start address outside flash memory.\n");
 return ERROR; //Start address outside flash
 }
 if(outoffset(nbytes))
 {
  printf("Too much bytes for this flash memory.\n");
  return ERROR; //End address outside flash
 }

  for(unsigned adr=start; adr<=start+nbytes; adr+=CFI_esize1bytes)
    {
      if (erasebank(adr)==ERROR)
         {printf("Error erasing address (%08X)\n",adr+flash_address);
          retval++;
         }

       printf("Erased address %d+%d. Erase time: %.2f seg. Total:%.2f\n",
       flash_address, adr, timer.Stop()-prevtime,timer.Stop());
       prevtime=timer.Stop();
       flashadr+=CFI_esize1bytes;
     }
    printf("Total time: %.2f seg.\n",timer.Stop());
return retval;
}

int flashmem::eraseall()
{
 if(current_flash_mode<cfi_ok) return ERROR;
 int retval;
 unsigned flashadr=0;
 retval=SUCCESS;
 Chrono timer;
 double prevtime=0;
  for(unsigned block=0; block<CFI_eblock1+1; block++)
    {
      if (erasebank(flashadr)==ERROR)
         {printf("Error erasing block %d (%08X)\n",block,flashadr+flash_address);
          retval++;
         }

       printf("Erased block %d of %d. Erase time: %.2f seg. Total:%.2f\n",
       block+1, CFI_eblock1+1,timer.Stop()-prevtime,timer.Stop());
       prevtime=timer.Stop();
      flashadr+=CFI_esize1bytes;
     }
    printf("Total time: %.2f seg.\n",timer.Stop());
return retval;
}

int flashmem::write(unsigned offset, unsigned data)
{
/* 16 bits flash (Virtex Board)  write to 0x10 (same as 0x11 0x12 0x13)
418 Rd01(80) Addr[80000000] 
    Resp: 000001FF
167 Wr01(C0) Addr[80000000] 000009FF   Enable write in MCTRL core
168 Wr01(C0) Addr[00000010] 0050FFFF   Clear status reg. & (Read array mode)
169 Wr01(C0) Addr[00000010] 00500040   Clear status reg. & Word write
170 Wr01(C0) Addr[00000010] 12340070   Data 0x1234 &  Read status reg
171 Wr01(C0) Addr[00000010] 00700070   Read status reg & Read status reg
172 Wr01(C0) Addr[00000010] 00405678   Word write & Data 0x5678
173 Wr01(C0) Addr[00000010] 00700070   read status reg & read status reg
174 Wr01(C0) Addr[00000010] 0050FFFF   Clear status reg. & Read array mode
419 Rd01(80) Addr[80000000] 
    Resp: 000009FF 
175 Wr01(C0) Addr[80000000] 000001FF   Disable write in MCTRL core
*/

/*8 bits flash, pender board.  >flash write 0x700000 0x12345678
353 Rd01(80) Addr[80000000] 
    Resp: 000000FF 
653 Wr01(C0) Addr[80000000] 000008FF 
654 Wr01(C0) Addr[00700000] 50FFFFFF     Clear status reg
655 Wr01(C0) Addr[00700000] FFFFFF40     Word write
656 Wr01(C0) Addr[00700000] 12FFFF70     Data &   Read status reg
657 Wr01(C0) Addr[00700000] 4034FF70     Word write & Data &   Read status reg
658 Wr01(C0) Addr[00700000] FF405670     Word write & Data
659 Wr01(C0) Addr[00700000] FFFF4078     Word write & Data
660 Wr01(C0) Addr[00700000] 50FFFFFF     Clear status reg.
354 Rd01(80) Addr[80000000] 
    Resp: 000008FF 
661 Wr01(C0) Addr[80000000] 000000FF

Trick 1! In 8 bit mode you can't touch A0 and A1 in the flash, so you must place
the write command in the right place inside the 32 bit data word...

*/
if(current_flash_mode<cfi_ok) return ERROR;
if(invalidoffset(offset)) return ERROR;

unsigned mem, mbuf[MINBUFF];
mem=flash_address + (offset&0xFFFFFFFC);
send_command(CMD_CLEARSTATUS,CMD_READARRAY);

 switch (mcfg1_pwidth)
 {
  case 0x0: //8 bits
            mbuf[0]=0xFFFFFF40;
            write_ahb(mbuf,1,mem);
            mbuf[0]=0x00FFFF70|(data&0xFF000000);
            write_ahb(mbuf,1,mem);
            usleep(CFI_sword_typ_wr_to*CFI_sword_max_wr_to);
            mbuf[0]=0x4000FF70|(data&0xFF0000);
            write_ahb(mbuf,1,mem);
            usleep(CFI_sword_typ_wr_to*CFI_sword_max_wr_to);
            mbuf[0]=0xFF400070|(data&0xFF00);
            write_ahb(mbuf,1,mem);
            usleep(CFI_sword_typ_wr_to*CFI_sword_max_wr_to);
            mbuf[0]=0xFFFF4000|(data&0xFF);
            write_ahb(mbuf,1,mem);
            usleep(CFI_sword_typ_wr_to*CFI_sword_max_wr_to);
            break;
  case 0x1: //16 bits
            mbuf[0]=0xFFFF0040;
            write_ahb(mbuf,1,mem);
            mbuf[0]=0x00000070|(data&0xFFFF0000);
            write_ahb(mbuf,1,mem);
            usleep(CFI_sword_typ_wr_to*CFI_sword_max_wr_to);
            mbuf[0]=0x00700070;
            write_ahb(mbuf,1,mem);
            mbuf[0]=0x00400000|(data&0xFFFF);
            write_ahb(mbuf,1,mem);
            usleep(CFI_sword_typ_wr_to*CFI_sword_max_wr_to);
            break;
  case 0x2: //32 bits
            printf("Not implemented!\n");
            break;
  default:;
}
 //We only check last write operation :-(
return statusreg()&0x7F;
}

int flashmem::buffer_write(unsigned offset, unsigned *buffer , unsigned words)
{
 if(current_flash_mode<cfi_ok) return ERROR;
 if(invalidoffset(offset)) return ERROR;
 if(outoffset(offset)) return ERROR;

 unsigned int mem,  cmdend [MINBUFF], mbuf, nwrites;
 mem=flash_address + (offset&0xFFFFFFFC); //Always start at address 0,4,8, etc..

 if(words>CFI_bufferbytes/4)return ERROR;        //too much words to write
 if(words==0)return ERROR;                       //nothing to write

 nwrites=words*4/(1+mcfg1_pwidth); //depends if device is in 8 or 16 bits mode.

 send_command(CMD_WRITEBUFF,nwrites-1,mem); //Sending write command.
 rawread(mem,&mbuf,1);
 statusregval=mbuf&0xFF;
 
 if(statusregval!=0x80) {printf ("Buffer Write error\n");return ERROR;}

 write_ahb(buffer,words,mem);

 usleep(CFI_buff_typ_wr_to*CFI_buff_max_wr_to);

 if(mcfg1_pwidth) cmdend[0]=0x00D00070;
   else cmdend[0]=0xD0707070;
 write_ahb(cmdend,1,mem);

 //This lines are ok but take somo extra time when writing.
 //statusregval=rawread(mem,&mbuf,1);
 //printstatusreg();
 
return SUCCESS;
}

int flashmem::file_write( char * filename, unsigned flashoffset,
                          unsigned long fileoffset, unsigned long bytelenght, bool write_verify)
{
FILE *fh;
char verifyerror, pskip, first;

 if(current_flash_mode<cfi_ok) return ERROR;

int pad=0;
if(bytelenght%4>0) { pad=4-(bytelenght%4);} //We must write four bytes at once

//Opening the file.
printf("File: [%s]  Offset[0x%08lX-%lud] Lenght[0x%lX-%lud] pad=%d\n",
        filename, fileoffset,fileoffset,bytelenght,bytelenght,pad);
bytelenght+=pad;

if ( (fh=fopen(filename,"r")) == NULL ) return ERROR;
unsigned mem, moveoffset, reading, writing, writecount, progress;
unsigned char writebuff [256], rbuff[256];                //TODO: Must use dynamic memory.
mem=flash_address + (flashoffset&0xFFFFFFFC); //Always start at address 0,4,8, etc..
send_command(CMD_READARRAY,CMD_READARRAY,mem);
send_command(CMD_CLEARSTATUS,CMD_READARRAY,mem);

moveoffset=flashoffset;
writecount=0;
//Goto the offset location
if(fseek( fh, fileoffset, SEEK_SET)) {fclose(fh); return ERROR;}
 progress=0; writing=0; pskip=0; verifyerror=0; first=1;
 Chrono timer;
 
 do {
     unsigned nwrites;
     nwrites=read_buffsize();
     if (bytelenght-writecount < nwrites)
        {nwrites=bytelenght-writecount;}
     //Fill the buffer with 0xFF (erased flash)
     for (unsigned int ein=0;ein<read_buffsize();ein++)
     {writebuff[ein]=0xFF;}
     reading=fread(writebuff, sizeof(char), nwrites, fh);
     
     ///mmmmhh. I have to change endianness in the buffer...
     //This is only for little endian systems
     for(unsigned cnt=0;cnt<reading;cnt+=4)
        {
         char first =writebuff[cnt+0];
         char second=writebuff[cnt+1];
         char third =writebuff[cnt+2];
         char fourth=writebuff[cnt+3];
         writebuff[cnt+3]=first ;
         writebuff[cnt+2]=second;
         writebuff[cnt+1]=third;
         writebuff[cnt+0]=fourth;
        }
     // VERIFY ONLY
     if(!write_verify)
     {
       //Read current block in flash memory
       send_command(CMD_CLEARSTATUS,CMD_READARRAY,moveoffset);
       if (rawread (moveoffset, (unsigned *) rbuff, nwrites/4)==ERROR)
           printf("Warning, error reading flash!\n");
      //Compare reading block with writing block
       for(unsigned mcont=0; mcont < nwrites/4; mcont++)
          {
          unsigned * rbf,*wbf;
          rbf=(unsigned*)rbuff+mcont;
          wbf=(unsigned*)writebuff+mcont;
          if(*rbf!=*wbf) //Buffers ar not equal
            { verifyerror=1; break;}
          }
     }
    // WRITE ONLY
     else
     {
        writing=buffer_write(moveoffset, (unsigned *) writebuff, nwrites/4);
     }
     moveoffset+=read_buffsize(); //addressing is for 8 bits data.
     writecount+=nwrites;
     progress+=nwrites;

     if(progress>bytelenght/100 || first)
       {
         printf("%d/%lu (%lu%%) Time: %.2f segs. ",writecount,bytelenght,
         (writecount*100/bytelenght), timer.Stop()) ;
         printf("Write/Verify: %c\n",(write_verify)?'W':'V');
         fflush(stdout);
         progress=0;
         first=0;
       }
    }
    while ( reading == read_buffsize() && writing==0 && writecount<bytelenght && verifyerror!=1);

 send_command(CMD_READARRAY,CMD_READARRAY);

 double time = timer.Stop();
 float kbits= ((float)(bytelenght*8)/time)/1024;
 printf("\n  Total bytes: %lu Time: %f %s Kbits/seg: %.2f\n", bytelenght
 ,(time>60)?time/60:time,(time>60)?"min":"seg" , kbits) ;

fclose(fh);
if (verifyerror) return ERROR;
return SUCCESS;
}

int flashmem::blankcheck(unsigned startoffset, unsigned endoffset)
{
 if(current_flash_mode<cfi_ok) return ERROR;
 if(invalidoffset(startoffset)||invalidoffset(endoffset))
   { printf("Start or end offsets out of limits\n");
    return ERROR;
   }

 send_command(CMD_READARRAY);

 startoffset&=0xFFFFFFFC;
 endoffset&=0xFFFFFFFC;
 endoffset+=4;
 unsigned buffer[64], start, end, cc, ofs;

 start=startoffset;
 end=endoffset;

 while (start <endoffset)
 {
   if(endoffset-start>64*4) end=start+(64*4);
      else end=endoffset;

   if ( rawread(start,buffer,(end-start)/4)==-1)
       { printf("Error de lectura\n"); return ERROR;}

   for( cc=0, ofs=start; cc<(end-start)/4 ;cc++,ofs+=4)
         {
          if(outoffset(ofs))
           {
           printf ("Offsett 0x%08X = [%08X]",ofs+flash_address,buffer[cc]);
           printf(" Warning, address out of flash size.");
           printf("\n");
           }
          if(buffer[cc]!=0xFFFFFFFF)
          {printf("Blank check error in address 0x%08X, data 0x%08X\n",ofs,buffer[cc]);
          return ERROR;
          }
         }
   start=end;
 }
return SUCCESS;
}

