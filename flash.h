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
#define ERROR -1
#define SUCCESS 0

//When searching for the MCTRL core
#define VENDORID_ESA     0x04
#define MCTRL            0x0f

/**************************************************
Flash memory handling trough MCTRL
***************************************************/
class flashmem
     {
       Grlib * mygrlib;
       ipcore * flashcore_apb, * flashcore_ahb;
       unsigned mcfg1_address, flash_address, flash_address_end;

       unsigned mcfg1_value;
       int mcfg1_pwidth, mcfg1_pwen, mcfg1_pwws, mcfg1_prws;
       
       //CFI information
       unsigned CFI_vendor, CFI_device;
       unsigned CFI_cset;
       unsigned CFI_equery, CFI_minvolt, CFI_maxvolt, CFI_vppmax, CFI_vppmin;
       unsigned CFI_sword_typ_wr_to, CFI_sword_max_wr_to, CFI_buff_typ_wr_to;
       unsigned CFI_buff_max_wr_to, CFI_block_erase_typ_to, CFI_block_erase_max_to;
       unsigned CFI_chip_erase_typ_to, CFI_chip_erase_max_to;

       unsigned CFI_flashsize, CFI_devinterface, CFI_bufferbytes;
       unsigned CFI_eblock_regions, CFI_eblock1, CFI_esize1, CFI_esize1bytes;

       unsigned statusregval;
       
       //State. What do we know about the flash?
       enum flashmode_t
       {unknown, init_ok, grlib_ok, search_ok, cfi_ok};
       flashmode_t current_flash_mode;

       public:
       flashmem  () {current_flash_mode=unknown;}
       ~flashmem(){;}
       void init (Grlib * mgrlib) {
                                   if(current_flash_mode==unknown)
                                   {
                                   mygrlib=mgrlib; mcfg1_pwidth=-1; mcfg1_pwen=-1;
                                   mcfg1_pwws=-1; mcfg1_prws=-1;
                                   current_flash_mode=grlib_ok;
                                   }
                                 }
       //We should check offset before doing any operation on the flash
       int invalidoffset(unsigned offset) {
                                            if (offset>(flash_address_end-flash_address))
                                               return ERROR;
                                               else return SUCCESS;
                                          }
       int outoffset(unsigned offset) {
                                        if (offset>(CFI_flashsize-1))
                                        return ERROR;
                                        else return SUCCESS;
                                      }
       
       //CORE functions
       int search ();           //search for the memory controller core.
       void print_search ();
       unsigned baseaddr()   { return flash_address;}
       void print_mcfg1();     //MCTRL configuration register for PROM
       //Sends commands to the flash
       int send_command(unsigned char comm);
       int send_command32(unsigned  comm);
       int send_command(unsigned char comm, unsigned char comm2);
       int send_command(unsigned char comm, unsigned char comm2, unsigned address);

       //Info from MCFG1
       int read_pwidth() {read_mcfg1(); return mcfg1_pwidth;}
       int read_pwen()   {read_mcfg1(); return mcfg1_pwen;}
       int read_pwws()   {read_mcfg1(); return mcfg1_pwws;}
       int read_prws()   {read_mcfg1(); return mcfg1_prws;}
       //Info from CFI area
       int read_flashsize()       {return CFI_flashsize;}
       unsigned read_buffsize()   {return CFI_bufferbytes;}
       //Changes in MCFG1 (MCTRL)
       void enable_write(int enable);     //Enable writing to flash (in the MCTRL core)
       void waitstates(uchar readws, uchar writews); //Change read and write waitstates

       //high level CFI Commands
       int CFI_valid ();        //is it a CFI flash mem?
       int CFI_read  ();        //Read main CFI data.
       int CFI_print (bool mode);        //Prints all known CFI data.
       int CFI_dump  (unsigned soffset, unsigned eoffset);    //Prints CFI area.
       //read memory in raw mode. Basic check only. No conversion
       int rawread     (unsigned offset, unsigned * buff, unsigned words);
       //Write bytes using buffer writes
       int buffer_write(unsigned offset, unsigned *buffer , unsigned words);
       //Write a file, given a simple offset inside this file.
       int file_write(char *filename , unsigned flashoffset,
                      unsigned long fileoffset, unsigned long lenght, bool write_verify);
       int printlock  ();          //Prints page lock info.
       int unlock();               //Unlock all blocks
       int lock(unsigned offset);  //lock all blocks
       int lockall();              //lock all blocks
       int dumpblock (unsigned offset, unsigned len);  //Print a block o flash memory
       //Dump flash to a file
       int dumpfile(unsigned offset, unsigned endoffset, const char * fn);
       int statusreg();                                //return CFI status register
       void printstatusreg();                          //Print CFI status register
       int eraseall();                                 //Erase all banks. (if unlocked)
       int erasebank(unsigned address);                //Erase only one bank (if unlocked)
       int erasebank(unsigned start, unsigned nbytes); //Erase flash from start to start+nbytes

       //Low level
       unsigned read_flash(unsigned offset);       //Reads CFI infor from flash
       int write(unsigned offset, unsigned data);  //Write 32 data bits in flash (if unlocked)
       int blankcheck(unsigned startoffset, unsigned endoffset);  //Check if blank (0xFF)
       
       private:
       unsigned read_mcfg1();
     };


