/**[txh]********************************************************************

  Copyright (c) 2007-2009 Diego J. Brengi <Brengi at inti gob ar>
  Copyright (c) 2007-2009 Instituto Nacional de Tecnología Industrial
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
  FLEMon : FPGALibre LEON Monitor.
           Implements debug comunication with an FPGA using grlib.
           
  Based on information from : grlib-gpl-1.0.14-b2053/doc/grlib.pdf
  See also Snapgear Linux Code for other implementation and reference:
  snapgear-p31/linux-2.6.18.1/include/asm/ambapp.h
  snapgear-p31/linux-2.6.18.1/include/asm-sparc/leon3.h
  snapgear-p31/linux-2.6.18.1/drivers/ambapp/amba.c
  snapgear-p31/linux-2.6.18.1/drivers/ambapp/amba_driver.c

  Note: Assuming 32 bits for int and unsigned types.

***************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <ctype.h>

#include <readline/readline.h>
#include <readline/history.h>

//Libelf
#include <libelf.h>
#include <gelf.h>

#define CPPFIO_I
#define CPPFIO_PRegEx
#define CPPFIO_CmdLine
#define CPPFIO_ErrorLog
#define CPPFIO_ComSerie
#define CPPFIO_PTimer
#include <cppfio.h>

/*EDCL*/
#include <grlib_edcl.h>

#include "flemon.h"
#include "grlib.h"
#include "flash.h"

static const char* elf_type(GElf_Half type);
static const char* elf_machine(GElf_Half machine);
static const char* elf_kfile(Elf* elfkd);

int flemon_done = 0;

static int fplugplay=0;
static const char * serial_device=SERIALDEVICE, * device_file=NULL;
static char * ip_address=NULL;
static int baudrate=DEFBAUDRATE;
flashmem flash;

static  unsigned readingvalues [64];
COMSTATE flemon_comstat=NONE;
COMMTYPE flemon_commtype=SERIAL;

using namespace cppfio;
CmdLineParser clp;
ErrorLog error(stderr);

static ComSerie *sercomm;
const unsigned maxLen=1024;
edcl_link_t edcllink;

Grlib mylib;
  
static
void PrintCopy(FILE *f)
{
 fprintf(f,"Copyright (c) 2007-2009 Diego J. Brengi <brengi at inti gov ar>\n");
 fprintf(f,"Copyright (c) 2007-2009 Instituto Nacional de Tecnología Industrial\n");
}

static
void PrintPrgInfo()
{
 if (clp.GetVerbosity()<0) // Disabled when using --quiet
    return;
 fputs("FLeMon - FPGALibre Leon Monitor "VERSION"\n",stderr);
 PrintCopy(stderr);
 fputc('\n',stderr);
}

void PrintVersion()
{
 FILE *f=stdout;
 fputs("FLeMon - FPGALibre Leon Monitor\n",f);

 fputs(CMD_NAME" v"VERSION"\n",f);
 PrintCopy(f);
 clp.PrintGPL(f);
 fprintf(f,"\nAuthor: Diego J. Brengi.\n");
}

static
void PrintHelp(void)
{
 puts("Usage:\n"CMD_NAME" [options] ...\n");
 puts("Available options:\n");
 puts("-D, --device=DEVICE       Device for IO. Example -d /dev/ttyS0");
 puts("-i, --ip=IP_ADDRESS       IP address for target. Example -i 192.168.0.51");
 puts("-b, --baud=BAUDRATE       Serial baudrate");
 puts("-d, --dlist=FILE          Use a custom deviceid file");
 puts("-g, --plugplay            Count Plug&Play records as a core.");
 clp.PrDefaultOptions(stdout);
 puts("");
 exit(1);
}

static
struct clpOption longopts[]=
{
  { "device", 'D', argRequired, &serial_device,     argString },
  { "ip"    , 'i', argRequired, &ip_address,        argString },
  { "baud",   'b', argRequired, &baudrate,          argInteger },
  { "dlist",  'd', argRequired, &device_file,       argString },
  {"plugplay",'g', argNo,       &fplugplay ,        argBool },
  { NULL, 0, argNo, NULL, argBool }
};

cmdLineOpts opts=
{
 PrintPrgInfo, PrintHelp, PrintVersion, longopts, -1
};

/**********************************************************************
  Description: Send sync bytes and check communication
  Return: 0 on success, 1 or 2 if fail.
***********************************************************************/

int func_sync(const char * param)
{
 int retry, res;
 retry=0;

/*  if(flemon_comstat>=SYNC_DONE)
     { printf("SYNC  already done! Do cleancores first to reset status.\n");
       return ERROR;
     }
*/
 if(flemon_commtype==ETH)
  {
   init_edcl_link(&edcllink, ip_address);
   flemon_comstat=SYNC_DONE;
   return SUCCESS;
  }
 
 unsigned int magic_addr=0x80000000; // magic_response=0x82206020; //magic addr was 14
 do             //Sends auto-baudrate bytes and first query
 {
  if(clp.GetVerbosity()>0) printf("Sending SYNC bytes. ");
  ahb_uart_sync();
  res= read_ahb (readingvalues,1, magic_addr);

  if (res!=SUCCESS)
     {
      retry++;
      printf("Can't establish target conection.\n");
      sleep(1);
     }
 }while(retry<3 && res!=SUCCESS);

 if (res!=SUCCESS)
    { printf ("Abort...\n");
      return ERROR;
    }
    
/*  if (readingvalues[0]==magic_response)
  if(clp.GetVerbosity()>0) printf("Magick response OK!\n");
    else printf("Magic response error!\n");*/
    
  printf("Check address %X: 0x%08X\n", magic_addr, readingvalues[0]);
  flemon_comstat=SYNC_DONE;
return SUCCESS;
}
/**********************************************************************
  Description:  Scans AHB bus.
***********************************************************************/

int func_scanAHB(const char * param)
{
  int res;

  if(flemon_comstat>=SCANAHB_DONE)
     { printf("AHB scanning already done! Clean cores first to rescaning AHB bus.\n");
       return ERROR;
     }
  if(flemon_comstat<SYNC_DONE)
     { printf("No SYNC!.\n");
       return ERROR;
     }
   
  if(clp.GetVerbosity()>0) printf("Scanning AHB devices...");
    if(mylib.scan_ahb()==ERROR) return ERROR;
    flemon_comstat=SCANAHB_DONE;
    if(clp.GetVerbosity()>1)
    {
    printf("==================== AHB SCANNING RESULTS ========================\n");
    mylib.print_scan_ahb();
    }
    mylib.ahb2core();

  res=mylib.search_bridges();
  if(res==TRUE)
  { flemon_comstat=BRIDGE_FOUND;
    if(clp.GetVerbosity()>0)
      { printf("First AHB/APB bridge found.\n");
        mylib.print_bridge();
      }
    mylib.addcore_bridgePP();
  }

return SUCCESS;
}
/**********************************************************************
  Description: Scans APB bus.
***********************************************************************/

int func_scanAPB(const char * param)
{
  if(flemon_comstat<BRIDGE_FOUND)
     { printf("No bridges found or AHB Scanning needed!\n");
       return ERROR;
     }
  if(flemon_comstat>=SCANAPB_DONE)
     { printf("APB scanning already done!\n");
       return SUCCESS;
     }

   if(clp.GetVerbosity()>0)
     printf("Scanning APB Bus...\n");
    if (mylib.scan_apb()==ERROR) return ERROR;
    if(clp.GetVerbosity()>1)
     {
       printf("==================== AHB SCANNING RESULTS ========================\n");
       mylib.print_scan_apb();
     }
    mylib.apb2core();

flemon_comstat=SCANAPB_DONE;
return SUCCESS;
}

/**********************************************************************
  Description:  Scans AHB and APB buses.
***********************************************************************/

int func_scan(const char * param)
{
if (func_scanAHB("")==ERROR) return ERROR;
if (func_scanAPB("")==ERROR) return ERROR;
return SUCCESS;
}

/**********************************************************************
  Description: List detected cores.
***********************************************************************/

int func_ls (char * param)
{
char * s;
int cc;
s = stripwhite (param); //eat spaces

int longlisting, collapselisting;
 // -c and -l options for ls command
 PRegEx regex("\\s*-.*c");
 PRegEx regex2("\\s*-.*l");
 for( cc=0;cc<80 && param[cc]!=0; cc++);
 if (regex2.search(param,cc)) longlisting=1; else longlisting=0;
 if (regex.search(param,cc)) collapselisting=1; else collapselisting=0;

 printf("=============== DETECTED SYSTEM CORES%s%s ===============\n",
 (longlisting)?" (long)":"", (collapselisting)?" (collapsed)":"" );

 mylib.print_cores(longlisting);

return SUCCESS;
}

int func_mem (char * param)
{
 mylib.print_mem();
 return SUCCESS;
}

int func_cleancores (char * param)
{
  mylib.clean();
 if(flemon_comstat>=SYNC_DONE) flemon_comstat=SYNC_DONE;
   else flemon_comstat=NONE;
 return SUCCESS;
}

int main(int argc, char *argv[])
{
 const char * cfg, *devf;
 //Search for the first cofiguration file available
 cfg=get_filename(BASENAMECFG,NULL,USER_CFG,ROOT_CFG);
 clp.GetExtraOptsFile(cfg);
 clp.Run(argc,argv,&opts);


 if(ip_address!=NULL)
   {
   flemon_commtype=ETH;
   printf("IP Address = %s\n",ip_address);
   }

 if(flemon_commtype==SERIAL)
 {
  if(clp.GetVerbosity()>0) printf("Serial device: '%s' in %d bauds.\n",serial_device,baudrate);

  static ComSerie scmm(-1,ComSerie::ConvertBaudRate(baudrate),maxLen,serial_device);
  sercomm=&scmm;
  sercomm->Initialize();
 }

  Vendors vendorl;
  devf=get_filename(BASENAMEDEV,device_file,USER_IDS,ROOT_IDS);
  vendorl.readfile(devf);

  if(clp.GetVerbosity()>0)
  { printf("Parsed %d vendors and %d devices.\n",vendorl.nvendors(),vendorl.ndevices());}
  
  if(clp.GetVerbosity()>1)vendorl.printlist();

  mylib.set_vendor_list(&vendorl);

  if(fplugplay) mylib.addcore_PP();

 //Readline stuff...
  char *temp, *s, prompt[80]; // TODO: ¿How long?
  int done;
  
  //Readline command line
  temp = (char *)NULL;
  strcpy(prompt,"FLeMon$ ");
  initialize_readline ();       /* Bind our completer. */
  
  done = 0;
   while (!flemon_done)
    {
      temp = readline(prompt); //prompt
      /* Test for EOF. */
      if (!temp)
        exit (1);

      s = stripwhite (temp); //eat spaces
      if (*s)
        {
          add_history (s);
          execute_line (s);
        }
      free (temp);
    }

 sercomm->ShutDown();
return 0;
} //End main

cmdlineread_t parse_address (char * param, unsigned * address1, unsigned * address2)
{
char * s, *endp;

s = stripwhite (param); //eat spaces
if ( !strcmp (s,"all"))
   {
   return all;
   }

*address1=strtoul(s, &endp,0);
 if(*address1==0 && s==endp)
   return parse_addr1_error;

s=endp;

*address2=strtoul(s, &endp,0);
 if(*address2==0 && s==endp)
   return parse_addr2_error;

return ok;
}

//############### FLASH FUNCTIONS ######################################
/****************************************
Writes a signgle 32 bit word to the flash
******************************************/
int func_fwrite (char * param)
{
unsigned addr, data;
cmdlineread_t readcode = parse_address(param, &addr,&data);

 if(readcode==parse_addr1_error||readcode==parse_addr2_error || readcode==all)
   { printf("Parse error in address or data! \n Sintax: fwrite [offset] [data] \n" );
     return ERROR;
   }
 printf("Writting Address: 0x%08X+0x%08X Data: 0x%08X\n",
 flash.baseaddr(), addr,data);

 flash.enable_write(1);
 if(flash.write(addr,data))
  {
   printf("Error writing flash\n");
     flash.printstatusreg();
  }
 flash.enable_write(0);
 return SUCCESS;
}

/*****************************************************************
Takes a file, test if its and ELF-SPARC an get load address,
offset in file and size of the section.
Return: ERROR if file is not an ELF or section wasn't found.
SUCCESS if valid data.
******************************************************************/
int elf_info (char * s, int * addrr, int * off, int * lsize)
{
 int fd;
 char *sname;
 const char *clas, *ftype, *fmachine, *fkind;
 Elf *elf;
 GElf_Ehdr elfhdr;
 Elf_Scn *scn;
 GElf_Shdr shdr;
 size_t shstrndx;
 int offset, size, loadoffset, loadsize, foundflag, addr, loadaddr;
 foundflag=0;
 //Checking for library out of date and initilization
 if (elf_version(EV_CURRENT) == EV_NONE)
   {
    printf("  Error initializing LibElf\n");
    return ERROR;
   }

 if((fd = open (s, O_RDONLY)) <0)
    { printf("  Error opening file [%s].\n",s);
     return ERROR;
    }
   else printf("  Opening file [%s]\n",s);
   
  elf = elf_begin(fd, ELF_C_READ, NULL);
  if (gelf_getehdr(elf, &elfhdr) ==0)
    {
    printf("  Wrong elf header\n");
    close(fd);
    return ERROR;
    }

  fkind   = elf_kfile(elf);                 //ELF, COFF, other
  ftype   = elf_type(elfhdr.e_type);        //Executable, realic, lib..
  fmachine= elf_machine(elfhdr.e_machine);  //Architecture
  if (elfhdr.e_ident[EI_CLASS] == ELFCLASS32)
     clas="32-bit";
    else clas="64-bit";
  printf("   %s %s %s %s\n",  fkind, clas, ftype, fmachine);

  //Checking if it is a valid file for LEON
  if( elf_kind(elf)!=ELF_K_ELF)
     {printf("Nota an ELF File.\n"); return ERROR;}
  if(elfhdr.e_type!=ET_EXEC)
     {printf("ELF File is not Executable.\n"); return ERROR;}
  if(elfhdr.e_machine!=EM_SPARC)
     {printf("ELF File is not for Sparc.\n"); return ERROR;}
  if(elfhdr.e_ident[EI_CLASS] != ELFCLASS32)
     {printf("ELF File is not 32bits.\n"); return ERROR;}

  elf_getshstrndx(elf, &shstrndx);
 if ( shstrndx == 0)
    {printf("Error getting section index of the string table.\n"); return ERROR;}

  scn = NULL;
  //Loop for sections inside ELF
  while ((scn = elf_nextscn(elf, scn)) != NULL)
    {
      if (gelf_getshdr(scn, &shdr) != &shdr)
        { printf("Error getting section header\n"); return ERROR;}
      if ((sname = elf_strptr(elf, shstrndx, shdr.sh_name)) == NULL)
        { printf("Error getting name of section header\n"); return ERROR;}

      offset=shdr.sh_offset;
      size=shdr.sh_size;
      addr=shdr.sh_addr;
      printf("Size:%010dbytes Off:0x%08X addr:0x%08X ",size, offset, addr);
      printf("%s ",  sname);
      //We search for the first .text section. TODO: Is this the right choice?
      if(!strcmp(sname,".text") && foundflag==0)
        {printf(" <-This!");
         foundflag=1;
         loadoffset=offset;
         loadsize=size;
         loadaddr=addr;
        }
      printf("\n");
    }
  elf_end(elf);
  close(fd);    

*addrr=loadaddr; *off=loadoffset; *lsize=loadsize;
return SUCCESS;
}

/**********************************************
Takes an ELF file and erase flash banks needed
***********************************************/
int func_fxerase (char * param)
{
int loadoffset, loadsize, loadaddr, foundflag;
char * s;
s = stripwhite (param); //eat spaces
foundflag=ERROR;

printf("Erasing flash memory...\n");
 if (access(s,R_OK)||param==NULL)
   {printf ("  Wrong filename: [%s]\n   Usage: ferasee [filename]\n",s);
    return ERROR;
   }
//Obtaining ELF section info
foundflag=elf_info (s, &loadaddr, &loadoffset, &loadsize);
  if(foundflag!=SUCCESS)
    {  printf("No .text section found.\n"); return ERROR;}
flash.enable_write(1);
flash.erasebank(loadaddr,loadsize); //erase banks
flash.enable_write(0);
 return SUCCESS;
}

/*************************************************
Takes an ELF file an write it to the flash memory
**************************************************/
int func_fxload (char * param)
{
int loadoffset, loadsize, loadaddr, foundflag;
char * s;
s = stripwhite (param); //eat spaces
foundflag=ERROR;

printf("  Flemon ELF Flash loader\n");
 if (access(s,R_OK)||param==NULL)
   {printf ("  Wrong filename: [%s]\n   Usage: floade [filename]\n",s);
    return ERROR;
   }
//Obtaining ELF section info
foundflag=elf_info (s, &loadaddr, &loadoffset, &loadsize);
  if(foundflag!=SUCCESS)
    {  printf("No .text section found.\n"); return ERROR;}
flash.enable_write(1);
flash.file_write(s,loadaddr,loadoffset,loadsize,1); //write
flash.enable_write(0);
 return SUCCESS;
}

/****************************************************
Takes an ELF file an check against the flash content.
*****************************************************/
int func_fxverify (char * param)
{
int loadoffset, loadsize, loadaddr, foundflag;
char * s;
s = stripwhite (param); //eat spaces
foundflag=ERROR;

printf("  File verification \n");
 if (access(s,R_OK)||param==NULL)
   {printf ("  Wrong filename: [%s]\n   Usage: fverify [filename]\n",s);
    return ERROR;
   }
//Obtaining ELF section info
foundflag=elf_info (s, &loadaddr, &loadoffset, &loadsize);
  if(foundflag!=SUCCESS)
    {  printf("No .text section found.\n"); return ERROR;}
flash.enable_write(1);
 if(flash.file_write(s,loadaddr,loadoffset,loadsize,0)!=SUCCESS) //Only verify
    {printf("Verification FAILED!\n");}
flash.enable_write(0);
 return SUCCESS;
}

/****************************************
Dumps flash to screen
*****************************************/
int func_fdumpblock (char * param)
{
unsigned addr,addrend;
cmdlineread_t readcode = parse_address(param, &addr,&addrend);
 if(readcode==parse_addr1_error || readcode==all)
 if(readcode==parse_addr1_error)
   { printf("Parse error in offset1!\n fdumpblock [offset1| all] [offset2]  f\n" );
    return ERROR;}
 if(readcode==parse_addr2_error)
   { printf("Parse error in offset2!\n fdumpblock [offset1| all] [offset2]  f\n" );
    return ERROR;}
 if(readcode==all)
 { addr=0; addrend=flash.read_flashsize()-1;}

flash.enable_write(1);
if (flash.dumpblock(addr,addrend)==ERROR) printf("Error in block dump!\n");
flash.enable_write(0);
return SUCCESS;
}

/****************************************
Dumps flash to a file
*****************************************/
int func_fdumpfile (char * param)
{
unsigned addr,addrend;
cmdlineread_t readcode = parse_address(param, &addr,&addrend);
 if(readcode==parse_addr1_error || readcode==all)
 if(readcode==parse_addr1_error)
   { printf("Parse error in offset1!\n fdumpfile [offset1| all] [offset2]\n" );
    return ERROR;
   }
 if(readcode==parse_addr2_error)
   { printf("Parse error in offset2!\n fdumpfile [offset1| all] [offset2]\n" );
    return ERROR;}
 if(readcode==all)
 { addr=0; addrend=flash.read_flashsize()-1;}
return flash.dumpfile(addr,addrend, "flash.dump");
}

/*************************************************
Takes a data file an write it to the flash memory
**************************************************/
int func_ffload (char * param)
{
int loadoffset, loadsize, loadaddr, foundflag;
char * s;
s = stripwhite (param); //eat spaces
foundflag=ERROR;
//stat ftell fopen tell

printf("  Flemon data Flash loader\n");
 if (access(s,R_OK)||param==NULL)
   {printf ("  Wrong filename: [%s]\n   Usage: ffload [filename]\n",s);
    return ERROR;
   }
FILE * f=fopen(s,"r");
fseek(f,0,SEEK_END);
loadsize=ftell(f);
loadaddr=0;
loadoffset=0;
flash.enable_write(1);
flash.file_write(s,loadaddr,loadoffset,loadsize,1); //write
flash.enable_write(0);
 return SUCCESS;
}

/****************************************************
Takes a data file file an check against the flash content.
*****************************************************/
int func_ffverify (char * param)
{
int loadoffset, loadsize, loadaddr, foundflag;
char * s;
s = stripwhite (param); //eat spaces
foundflag=ERROR;

printf("  File verification \n");
 if (access(s,R_OK)||param==NULL)
   {printf ("  Wrong filename: [%s]\n   Usage: fverify [filename]\n",s);
    return ERROR;
   }

FILE * f=fopen(s,"r");
fseek(f,0,SEEK_END);
loadsize=ftell(f);
loadaddr=0;
loadoffset=0;

flash.enable_write(1);
 if(flash.file_write(s,loadaddr,loadoffset,loadsize,0)!=SUCCESS) //Only verify
    {printf("Verification FAILED!\n");}
flash.enable_write(0);
 return SUCCESS;
}

/**********************************************
Takes a data file and erase flash banks as needed
***********************************************/
int func_fferase (char * param)
{
int  loadsize, loadaddr, foundflag;
char * s;
s = stripwhite (param); //eat spaces
foundflag=ERROR;

printf("Erasing flash memory...\n");
 if (access(s,R_OK)||param==NULL)
   {printf ("  Wrong filename: [%s]\n   Usage: ferasee [filename]\n",s);
    return ERROR;
   }

FILE * f=fopen(s,"r");
fseek(f,0,SEEK_END);
loadsize=ftell(f);
loadaddr=0;

flash.enable_write(1);
flash.erasebank(loadaddr,loadsize); //erase banks
flash.enable_write(0);
 return SUCCESS;
}

/****************************************************
Erase the flash memory. Address range or complete.
*****************************************************/
int func_ferase (char * param)
{
unsigned addr,addr2;

cmdlineread_t readcode = parse_address(param, &addr,&addr2);

if ( readcode==all)
   {
    flash.enable_write(1);
    printf("Erasing all flash...\n");
   if(flash.eraseall()==ERROR) printf("Error erasing flash.\n");
   flash.enable_write(0);
   return SUCCESS;
   }

if (readcode==parse_addr1_error)
  { printf("Parse error!.\n  ferase [all | offset ]\n" );
    return ERROR;
  }

 printf("Erasing flash bank in address 0x%08X ...\n", addr);
 flash.enable_write(1);
 if(flash.erasebank(addr)==ERROR)
    printf("Error erasing bank 0x%08X+0x%08X\n",flash.baseaddr(),addr);
 flash.enable_write(0);

return 0;
}

int func_flock (char * param)
{
 unsigned offset, addr2;
 cmdlineread_t readcode = parse_address(param, &offset,&addr2);

 if (readcode==all)
   {
    flash.enable_write(1);
    flash.lockall();
    printf("Locking all flash...\n");
    flash.enable_write(0);
    return SUCCESS;
   }

 if (readcode==parse_addr1_error)
   {printf("Parse error!\n  flock [all | offset] \n" );
    return ERROR;
   }

 flash.enable_write(1);
 if(flash.lock(offset)==ERROR) printf("Error Locking Flash!\n");
 flash.enable_write(0);
 return SUCCESS;
}

int func_funlock (char * param)
{
 unsigned nouse;
 cmdlineread_t readcode = parse_address(param, &nouse,&nouse);
 if(readcode!=all)
   {printf("Usage:\n funlock all\n");
   return ERROR;
   }
 flash.enable_write(1);
 if(flash.unlock()==ERROR)
    printf("Error Unlocking Flash!\n");
 flash.enable_write(0);
return 0;
}

int func_fstat (char * param)
{
 flash.enable_write(1);
 flash.printlock();
 flash.enable_write(0);
 return SUCCESS;
}
int func_fcfi (char * param)
{
flash.enable_write(1);
flash.CFI_dump(0x00,0x2D+3);
flash.enable_write(0);
return SUCCESS;
}

/***************************************************************
Search flash memory and print CFI info
****************************************************************/
int func_fscan (const char * param)
{
 flash.init(&mylib);
 if ( (flash.search())==SUCCESS)
    {
     if (clp.GetVerbosity()>0) flash.print_search();
    }
    else { printf("No flash!\n"); return 1;}

  flash.waitstates(15,15); //Remember to save previous default states.

  flash.enable_write(1);
  flash.print_mcfg1();

   if(flash.CFI_valid()==SUCCESS)
     {
       printf("Flash is CFI compliant.\n");
       flash.CFI_read();
       flash.CFI_print((clp.GetVerbosity()>0)?1:0);
       }
   else printf("Not CFI compliant\n");

 
  flash.enable_write(0);

 return SUCCESS;
}

/**********************************************************************
  Description:  sync, scanh, scanp and fdetect
***********************************************************************/
int func_detect(char * param)
{

if(flemon_comstat<SYNC_DONE)
   { if (func_sync("")==ERROR) return ERROR;
   }

printf("Scanning AHB.\n");
if (func_scanAHB("")==ERROR) return ERROR;
printf("Scanning APB.\n");
if (func_scanAPB("")==ERROR) return ERROR;
printf("Scanning Flash.\n");
if (func_fscan("")==ERROR) return ERROR;
return SUCCESS;
}

int func_fcheck (char * param)
{
unsigned addr,addrend;
cmdlineread_t readcode = parse_address(param, &addr,&addrend);

 if(readcode==parse_addr1_error || readcode==all)
   { printf("Parse error in offset 1! \n Sintax: fcheck [offset1] [offset2] \n" );
     return ERROR;
   }

 if(readcode==parse_addr2_error)
   { printf("Parse error in offset 2!\n \n Sintax: fcheck [offset1] [offset2] \n" );
    return ERROR;
   }

printf("Blank check: 0x%08X+0x%08X (0x%08X-0x%08X)\n",
flash.baseaddr()+ addr, flash.baseaddr()+addrend, addr, addrend);

flash.enable_write(1);
if(flash.blankcheck(addr,addrend))
  { printf("Error checking flash\n");
  }
else printf("PASS\n");

flash.enable_write(0);
return SUCCESS;
}

/**********************************************************************
  Description: 
***********************************************************************/

int func_flash (char * param)
{
      printf("==================== FLASH MEMORY REPORT =========================\n");
      flash.enable_write(1);
      flash.print_mcfg1();
      flash.CFI_print(1);
      flash.enable_write(0);
return 0;
}

/**********************************************************************
  Description: 
***********************************************************************/

int write_ahb (unsigned * lbuff, unsigned nwords, unsigned address)
{
unsigned char tx[5];
unsigned  cont; //reading,

//nword range is 1 to 64 because first two bits of the control byte are for
//signaling read or write operations. The others six bits are for Lenght-1.
if (nwords >64 || nwords <1) return ERROR;


nwords&=0x3f;  //Just in case...

/*Ethernet comm*/
if (flemon_commtype==ETH)
  {
        write_ahb_edcl32 (&edcllink,lbuff, nwords, address);
        return SUCCESS;
  }

tx[0]=0xc0|(nwords-1);
tx[4]= (unsigned char) address&0x000000ff;
address=address>>8;
tx[3]= (unsigned char) address&0x000000ff;
address=address>>8;
tx[2]= (unsigned char) address&0x000000ff;
address=address>>8;
tx[1]= (unsigned char) address&0x000000ff;

sercomm->Send(tx,5);

for(cont=0;cont<nwords;cont++)
 {
  unsigned trsm = lbuff[cont];
  tx[3]= (unsigned char) trsm&0x000000ff;
  trsm>>=8;
  tx[2]= (unsigned char) trsm&0x000000ff;
  trsm>>=8;
  tx[1]= (unsigned char) trsm&0x000000ff;
  trsm>>=8;
  tx[0]= (unsigned char) trsm&0x000000ff;
  sercomm->Send(tx,4);
 }
return 0;
}

/**********************************************************************
  Description: Reads AHB bus.
  Return: Filled buffer with values.
***********************************************************************/


int read_ahb (unsigned  lbuff[], unsigned nwords, unsigned address)
{
unsigned char tx[5];
char rbytes[256];
unsigned reading, cont;

if(!nwords) return ERROR;

//nword range is 1 to 64 because first two bits of the control byte are for
//signaling read or write operations. The others six bits are for Lenght-1.
if (nwords >63 ) { printf("\n Maximun nwords limit in read_ahb()\n"); return ERROR;}
nwords&=0x3f;  //Just in case...

/*Ethernet comm*/
if (flemon_commtype==ETH)
  {
    int trycont=5, retval;
    do {
        retval=read_ahb_edcl32 (&edcllink,lbuff, nwords, address);
        trycont--;
        if (trycont<4)printf("                 Retry Ethernet: %d .\n",5-trycont);
       }
       while (retval!=READ_SUCCESS && trycont!=0);
       if (retval==READ_SUCCESS) return SUCCESS;
         else return ERROR;
  }

nwords-=1;

tx[0]=0x80|(nwords);
tx[4]= (unsigned char) address&0x000000ff;
address=address>>8;
tx[3]= (unsigned char) address&0x000000ff;
address=address>>8;
tx[2]= (unsigned char) address&0x000000ff;
address=address>>8;
tx[1]= (unsigned char) address&0x000000ff;

sercomm->Send(tx,5);

unsigned read, toflag=1;
 
 PTimer to(100000*10); //  time-out
 do
   {
    read=sercomm->RawRead(rbytes,4*(nwords+1),1); //Exact reading
    if (!read)
       usleep(1000);
    //printf("read: %d to.Reached() %d\n",read,to.Reached());
    if(to.Reached()) toflag=0;
   }
 while (!read && toflag);

 if (!toflag) {printf("\n Serial timout\n");
               return ERROR;
              }

 for (cont=0;cont<nwords+1;cont++)
    {
      reading= ((unsigned char)rbytes[(cont*4)+0]<<24)|
               ((unsigned char)rbytes[(cont*4)+1]<<16)|
               ((unsigned char)rbytes[(cont*4)+2]<<8) |
               (unsigned char)rbytes[(cont*4)+3];
      lbuff[cont]=reading;
    }
return SUCCESS;
}


/**********************************************************************
  Description:  Send bytes to help baudrate detection.
  Return: Nothing.
***********************************************************************/

int ahb_uart_sync()
{
 char buf[2];
// int res;
 buf[0]=0x55;buf[1]=0x55;
 sercomm->Send(buf,2);
 return 2;
}

/***************************************************************************

  Description: Select the first available file. If all the give files
  fails, return basefilename (current directory). Search order is
    1) Comandline file.
    2) Current dir file.
    3) Home dir.
    4) root dir file.
    5) Current dir (will fail).
  
  Return:  pointer to filename
  Example: filen=get_filename(devices,cmdline_file,"/.flemon/","/usr/local/flemon")
  
***************************************************************************/

const char * get_filename (const char * basefilename,
                           const char * cmdline ,const char * user, const char * root)
{
char * home;
const char *ret;
static char buf[100];

//Filename supplied from command line?
if (cmdline!=NULL && !access(cmdline,R_OK))
   ret=cmdline;
  else if (!access(basefilename,R_OK))
          ret=basefilename;
     else
         {
          home=getenv( "HOME" );
          strcpy(buf,home);
          strcat(buf,user);
          strcat(buf,basefilename);
          if (!access(buf,R_OK))
          ret=buf;
           else
           {
           strcpy(buf,root);
           strcat(buf,basefilename);
           if (!access(buf,R_OK))
              ret=buf;
              else ret=basefilename;
           }
         }
if(clp.GetVerbosity()>2)
printf("Using file: %s\n",ret);
return ret;
}

//Libelf helpers
static const char* elf_type(GElf_Half type)
{
  const char *val;
  switch(type)
  {
   case ET_NONE:   val = "Unknown type"; break;
   case ET_REL:    val = "Relocatable";   break;
   case ET_EXEC:   val = "Executable";   break;
   case ET_DYN:    val = "Dynamic lib";   break;
   case ET_CORE:   val = "Core file";    break;
   case ET_LOPROC:
   case ET_HIPROC: val = "Processor specific";  break;
    default:       val = "Invalid file type";
  }
  return val;
}

static const char* elf_machine(GElf_Half machine)
{
   const char *val;
   switch(machine)
    {
     case ET_NONE:       val = "None";     break;
     case EM_SPARC:      val = "Sparc";    break;
     case EM_SPARC32PLUS:val = "Sparc32"; break;
     case EM_SPARCV9:    val = "SparcV9";  break;
      default:           val = "NonSparc"; break;
    }
   return val;
}

static const char* elf_kfile(Elf* elfkd)
{
  const char *val;
  switch(elf_kind(elfkd))
   {
     case ELF_K_AR:   val = "Archive";  break;
     case ELF_K_COFF: val = "COFF";     break;
     case ELF_K_ELF:  val = "ELF";      break;
     case ELF_K_NONE: val = "Unknown Format";  break;
      default:        val= "Unkown file";
  }
  return val;
}

inline int byteA(unsigned word32)
{ return word32&0x000000FF;}

inline int byteB(unsigned word32)
{ return (word32>>8)&0x000000FF;}

inline int byteC(unsigned word32)
{ return (word32>>16)&0x000000FF;}

inline int byteD(unsigned word32)
{ return (word32>>24)&0x000000FF;}

inline void print_word (unsigned word32)
{

(isprint(byteD(word32)))?printf("%c", byteD(word32)): printf(".");
(isprint(byteC(word32)))?printf("%c", byteC(word32)): printf(".");
(isprint(byteB(word32)))?printf("%c", byteB(word32)): printf(".");
(isprint(byteA(word32)))?printf("%c", byteA(word32)): printf(".");
}


