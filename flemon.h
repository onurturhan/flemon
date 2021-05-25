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
  FLEMon : FPGALibre LEON Monitor. Header file.
           Implements debug comunication with an FPGA using grlib.
           
  Note: Assuming your compiler use 32 bits for int and unsigned types.

***************************************************************************/

#define ERROR -1
#define SUCCESS 0

#define VERSION "Version: 0.3-2"
#define CMD_NAME "flemon"

//Program Default
#define DEFBAUDRATE 115200
#define SERIALDEVICE "/dev/ttyS0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

//Configuration file
#define BASENAMECFG "flemon.cfg"
#define USER_CFG "/.flemon/"
#define ROOT_CFG "/etc/"
//Device & vendor names file
#define BASENAMEDEV "devices"
#define USER_IDS "/.flemon/"
#define ROOT_IDS "/usr/share/flemon/"

enum COMSTATE
{ NONE, SYNC_DONE, SCANAHB_DONE, BRIDGE_FOUND, SCANAPB_DONE
};

enum COMMTYPE
{ SERIAL, ETH
};

enum cmdlineread_t
{ ok, parse_addr1_error, parse_addr2_error, all
};

//**********Readline stuff**************
extern  char * stripwhite (char * string);
extern  int execute_line (char * line);
void initialize_readline (void);
//**************************************

//**Functions for low level access***
int  read_ahb        (unsigned lbuff [], unsigned nwords, unsigned address);
int  ahb_uart_sync();
int write_ahb (unsigned lbuff [], unsigned nwords, unsigned address);
//***********************************

const char * get_filename (const char * basefilename,
                           const char * cmdline ,const char * user, const char * root);

inline void print_word (unsigned word32);



