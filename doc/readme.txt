  Copyright (c) 2008-2009 Diego J. Brengi <Brengi at inti gob ar>
  Copyright (c) 2008-2009 Instituto Nacional de Tecnología Industrial. Bs. As. Argentine.
  
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

=======================================================================

  FLEMON - FPGALibre LEON Monitor
  +++++++++++++++++++++++++++++++


Flemon is a tool to debug a LEON/GRLIB system.
It communicates with the AHBUART (AMBA Serial Debug Interface) to get 
available core information from a target system. You can also download
an elf binary to the board flash memory. Flemon is experimental software 
under development.
 
BRIEF USAGE:
============
 flemon -h (to see command line options)
 
 Flemon is interactive and uses readline. You can type help inside
 the Flemon's command line to see available commands.

DESCRIPTION:
============
 Grlib is a VHDL IP core library, with the LEON-3 as the main processor and
 AMBA AHB and APB buses for core interconnection. This library was
 developed and maintained by Aeroflex/Gaisler, and released under GPL license.
 After building a Grlib system and downloading to your system (FPGA
 configuration) you can debug it through serial asynchronous communication,
 Ethernet, USB and more. You can do this by adding special debugging IP cores
 to your design. This debug cores allows you to access de system buses (AMBA
 AHB and AMBA APB).
 Grlib also have a fixed Plug&Play area information with IP core presence
 information and associated address in the bus. With access to the bus you can
 access this P&P area to know available IP cores and addresses. This allow you
 to access them for writing and reading operations.

 Flemon supports Grlib systems with AHBUART, AHB Serial Debug Interface
 (Asynchronous serial communication). Others types of debug communication
 (Ethernet, usb, pci, etc.) are not supported yet. After communication Flemon
 scans AHB Plug&Play area to know the available IP cores in the AHB bus. If
 an AHB/APB bridge is detected, an APB bus scanning is done and APB cores are
 also listed.

 Each IP core is listed according to the vendorid and deviceid codes taken
 from de Grlib Manual. This translation is done in the "devices" file. You
 can freely edit this file to add a new core description for your system
 (for example an unofficial core added by you to the Grlib).

 At this point you can list available IP cores and know addresses for each
 one.

 Flemon also has basic support for CFI (Common Flash Interface) flash devices
 trough the MCTRL core. This allows you to write a program to the flash memory.

 The original purpose for writing this program was to load an embedded
 Linux operating system for LEON-3 in the flash memory board, without using
 proprietary software. This image is usually an ELF executable format for
 SPARC processors. Flemon can extract the data inside this files and write it
 to the flash memory.
 

COMPILING AND INSTALLING
========================
Download the source code and unpack it. Use "make" to compile.
There is no system installation available yet.
If you want to make a Debian package, use:
make deb

 
USAGE
======
Command line options and configurations files:
----------------------------------------------
Flemon has some command line options. Type "flemon -h" to see all them.
for example "flemon -D /dev/ttyS0 -b 9600 -v" tells flemon to use ttyS0 serial
port with 9600 bps and run in verbose mode (level 1). You also have long
command line names, for example --device and --baud.
You can put this options inside a configuration file. The order of relevance
for the options are:
 1) Command line.  (if present, override 2,3 and 4)
 2) "flemon.cfg" in the current directory. (if file present, don't look further)
 3) /home/$USER/.flemon/flemon.cfg. (if file present, don't look further)
 4) /etc/flemon.cfg
 5) If an option is not present, use defaults.

An example configuration file can be this way:
####start_file######
 --device=/dev/ttyS1
 -v
####end_file########

Vendor id and Device configuration file:
----------------------------------------
Flemon uses a plain text file for translating vendor and device identification
codes into a useful human readable description. This file es called "devices".
Flemon search for this file at startup in the following order:
 1) From the command line option -d or --dlist. (if present, don't look anymore)
 2) Filename "devices" in the current directory.(if present, don't look anymore)
 3) Filename "$HOME/.flemon/devices". (if present, don't look anymore)
 4) Filename "/usr/share/flemon/devices".
 5) If none is present, report only numbers without descriptions.
You can freely edit this device file according to your needs. Look inside the
provided file to see the right syntax.

Inside Flemon:
--------------
After starting flemon you have a command line interface with auto-completion
for commands and files (TAB key). You can also browse the command history
of the current session with the up and down arrows.
Type "help" to see the available commands and a brief description for each one.

To make a basic step-by-step discovery sequence, use the following commands:

  Push reset button or the reconfigure FPGA button. AHBUART configure the baudrate
  only with the first characters received.
  
  "sync"  Send sync bytes and tries to read something to check comm link.
  "scanh" Scans P&P area in AHB bus. Search for an AHB/APB bridge.
          After this, you can use "ls" to see only AHB cores.
  "scanp" Scans P&P area in APB bus.
          After this, you can use "ls" to see AHB and APB cores.
  "fscan" Search if there is an MCTRL ip core. If present, reads CFI
          identification strings. If positive, reads CFI geometry data.
          After that you can do flash operation commands like "fstat",
          "ferase", "flock", "funlock", "fwrite" and "fload".

The  "detect" does this steps in sequence.

  "quit"  Exits flemon.

For flash usage, see commands starting with "f".
For working with ELF files se  commands starting with "fx".
For working with raw data files se  commands starting with "ff".

Inside Flemon you have autocompletion for commands and files with the TAB key.

NOTES
======
 -The sync command sends 0x55 0x55 to the AHBUART in order to setup the
  baudrate.
 -Only the first available AHB/APB bridge is used. Designs with more than one
 (if possible), are not supported.
 -Search for AHB/APB bridge and for MCTRL IP cores is done with internal
 hard coded vendor and device ids. You can't change this in the "device" file.
 (APBCTRL: hard coded vendorid 0x01 and deviceid 0x06)
 (MACTR:  hard coded vendorid 0x04 and deviceid 0x0f)
 -There is only basic PROM (flash) support for the MCTRL ip core.
 -Flemon was tested with the following hardware/bitstream combination:
  *Board: Pender GR-XC3S-1500.
   Grlib: grlib-bitfiles-1.0.20
   File:  designs/leon3-gr-xc3s-1500/bitfiles/leon3mp.bit
   Flash: 8 bit mode.
  *Board: AVNET Virtex 4-LX Evaluation (XC4VLX25).
   Grlib: grlib-bitfiles-1.0.20
   File:  designs/leon3-avnet-eval-xc4vlx25/bitfiles/leon3mp.bit
   Flash: 16 bit mode.
 -DSU (Debug Support Unit) for Debugging LEON-3 processor is not supported
  yet.

=====================================================================
FLEMONSPY

Flemonspy is simple protocol analyzer for messages between monitor software
and AHBUART. You can use it with Flemon or with other software.

Flemonspy analyze the serial communication between FPGA and Debug software
(flemon or grmon). It takes data from STDIN and print it nicely. Use in
combination with intercepttty or a similar tool.

Example:

 $interceptty /dev/ttyS0 -p /dev/ptyp0 | ./flemonspy | tee /tmp/flspy.txt
 $flemon -D /dev/ttyp0
 
The first line launch flemonspy taking data from interceptty and then
pass info to flemnspy. With tee we can see the output and save it to a 
file.

intercepttty is a nice tool available at: 
 http://www.suspectclass.com/~sgifford/interceptty/

You need ptyp/ttyp support in your Linux kernel.

