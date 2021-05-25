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

  Description: Grlib, high level functions and ipcores handling.
***************************************************************************/

#define TRUE 1
#define FALSE 0

#define CPPFIO_I
#define CPPFIO_Helpers
#include <cppfio.h>
#include <stdio.h>

//When searching for the APBCTRL AHAB/APB bridge
#define VENDORID_GAISLER   0x01
#define APBCTRL            0x06

//BAR Types
#define APB_IO  0x01
#define AHB_MEM 0x02
#define AHB_IO  0x03


//AHB P&P structure
#define AHB_WORD        8
#define APB_WORD        2
//First 64 positions are for Masters.
#define MAX_AHB_MASTER 64
//Only firsts are scanned
#define SCAN_AHB_MASTER 16
//Next 64 positions are for Slaves
#define MAX_AHB_SLAVE  64
//Only  firsts are scanned
#define SCAN_AHB_SLAVE  16

// Maximun number of grlib cores in APB bus
#define MAX_APB_SLAVE  512
#define SCAN_APB_SLAVE 16
#define MAX_APBUART     8

//Fixed memory areas
#define AREA_IO         0xfff00000
#define AREA_CFG        0xff000
#define AHBS_CONF_AREA  0x00000800

//Store AHB scanning information
typedef struct
{
unsigned int order;                //slot order 0 to 63 and 64 to 127 (master or slave)
unsigned slotaddress;              //slot AHB address
unsigned  ireg, user[3], bar[4] ;  //reading values
}ahb_slot;

//Store APB scanning information
typedef struct
{
unsigned int order;                //slot order 1 to 512 (0x???FF000-0x???FFFFF)
unsigned slotaddress;              //slot APB address
unsigned ireg, bar;                //reading values
}apb_slot;

/***************************************
Store devices descriptions in a list
*****************************************/
class Devices
{ 
 public:
 Devices(int id, const char * desc) { deviceid=id; string=cppfio::newStr(desc);next=NULL;}
 ~Devices(){delete[] string;}
 void print (){ printf("DeviceID: %03d Name: %s\n",deviceid,string);}
 Devices * get_next() {return next;}
 void set_next(Devices * nx) {next=nx;}
 int get_id (){return deviceid;}
 const char * get_string (){return (string==NULL)?"Unknown Device": string;}

 private:
  int deviceid;
  char * string;
  Devices * next;
};

/***************************************
Store vendors descriptions and points to
first element of a list of devices
*****************************************/
class Vendors
{
 
 public:
 Vendors(){next=NULL; devlist=NULL; dlines=0; vlines=0;}
 ~Vendors() {delete [] this;}
   //read a file and load device and vendor codes
 int readfile (const char * filename);
 int ndevices (void)  { return dlines;}
 int nvendors (void)  { return vlines;}
   //get vendor description
 const char * get_string ( ) {return (string==NULL)?"Unknown Vendor": string;}
   //get device description
 const char * get_device (int vid, int did);

 void printlist();
 void printdevs();
 void print();
   //search for a vendor
 Vendors * search_vendor(int vid);
   //search for a devide
 Devices * search_device(int did);

 private:

 static Vendors *  first_vendor;

 int vendorid;
 int dlines, vlines; //Number of vendors and device lines parsed
 char *string;
 Vendors * next;
 Devices * devlist;
   //adds a new vendor id.
 void add_vendor(int vid, const char * text );
   //adds a new device.
 void add_device(int vid, int did, const char * text );
};

/******************************************
Store a single IP core information
*******************************************/
class ipcore
     {
       unsigned int deviceid;
       unsigned char vendorid, version, irq;
       int nbr_banks;
       unsigned  saddress[4], eaddress[4], addr[4], mask[4];
       unsigned char addrtype[4], cacheable[4], prefetch[4];
       ipcore * collapse; //It means this core can be collapsed with other. NOT USED YET.

       ahb_slot *slotahb;  //Associated AHB slot.
       apb_slot *slotapb;  //Associated APB slot.
       Vendors * myvendor; //Associated vendor.
       Devices * mydevice; //Associated device.

       public:
       ipcore  () {clean();}
       ~ipcore(){;}
       //Reset all available info
       void clean () { vendorid=0;      deviceid=0;
                       nbr_banks=0;     slotahb=NULL;
                       slotapb=NULL;    collapse=NULL;
                       myvendor=NULL;   mydevice=NULL;
                     }
       //Return vendor number or device number
       int get_did(){return deviceid;}
       int get_vid(){return vendorid;}
       //Valid if it is an AHB master
       bool is_master(){  if(slotahb!=NULL)
                           { return (slotahb->order<64)?1:0;}
                           else return 0;
                       }
      //Valid if it is an AHB core
       bool is_ahb(){ return (slotapb!=NULL)?0:1;}
       //Set this core as a Plug&Play area. This is a fake core, only for displaying.
       void set_PParea();
       //Set this core as a P&P area of an APB bus.
       void set_bridgePParea(unsigned saddr);
       //Is this an empty core?
       int  is_empty()
                   {
                   if (vendorid==0 && deviceid==0)
                     return TRUE;
                     return FALSE;
                   }
       //is this an AHB to APB bridge?
       int is_bridge()
                   { if (vendorid==VENDORID_GAISLER && deviceid==APBCTRL)
                       return TRUE;
                       return FALSE;
                   }
       
       //Slots related to this core
       void set_ahbslot(ahb_slot *ahbsp)
                   { slotahb=ahbsp;}
       void set_apbslot(apb_slot *apbsp)
                   { slotapb=apbsp;}
       void read_ahbslot_data();
       void read_apbslot_data();
       //Search for own vendor and device inside a vendor/device listing
       void read_vendor(Vendors * vlist);
       //Return address type
       unsigned get_addrtype(int bar)
         { if (bar<nbr_banks) return addrtype[bar];
              else return 0;
         }
       //Return address type as a string.
       const char * get_addrtypestr(int bar)
         { switch  (addrtype[bar])
           {
             case APB_IO          : return "apb i/o";
             case AHB_MEM         : return "ahb mem";
             case AHB_IO          : return  "ahb i/o";
             default              : return "Undef  ";
           }
         }
       //Returna start and end addresses. For APB cores bar must be 0.
       unsigned get_startaddress(unsigned int bar) { return saddress[bar];}
       unsigned get_endaddress(unsigned int bar) { return eaddress[bar];}
       void print();               //Print info
       void printdevice();         //Print only the device
       void printvendor();         //Print only the vendor
       void lprint();              //Long print
       void print_banks();         //Print BARs
       //Get device and vendor as a string
       const char * get_devicestr()
            {
             if (mydevice==NULL) return "Unknown Device";
                else return mydevice->get_string();
            }
       const char * get_vendorstr()
            { if (myvendor==NULL) return "Unknown Vendor";
                else return myvendor->get_string();
            }

       private:
       //Get start and end addressess based on P&P information (bar type)
       int addr_bar_extract(unsigned bar, unsigned * saddress,unsigned * eaddress);
     };

/******************************************
Store all IP cores
*******************************************/
class Grlib
     {
      ipcore * bridge;          //The AHB/APB  bridge found
      Vendors * vendorlist;     //The vendor/device list
      //Ip cores. Static array (must change soon...)
      ipcore  cores [SCAN_AHB_MASTER+SCAN_AHB_SLAVE+SCAN_APB_SLAVE+4];
      //AHB and APB scaning info
      ahb_slot ahb           [SCAN_AHB_MASTER+SCAN_AHB_SLAVE];
      apb_slot apb           [SCAN_APB_SLAVE];

       public:
       Grlib  () {clean(); bridge=NULL; vendorlist=NULL; }
       ~Grlib(){;}
       int get_lastcore();          //get index of the last valid core
       int scan_ahb();              //Read ahb P&P area.
       void print_scan_ahb();       //print raw scan info.
       void clean();                //Clean cores information (prevoious)
       void addcore_PP();           //Add Plug&Play area information as a core.
       void addcore_bridgePP();     //Add an APB bus P&P area like a core.

       void ahb2core();             //Save ahb scanning info into ip cores.
       int search_bridges();        //Search for an AHB/APB bridge.
       void print_bridge();         //print bridge infomation.
       int  scan_apb();             //Make APB scanning.
       void print_scan_apb();       //Print scan info.
       void apb2core();             //Save APB scanning info inti ip cores
       void print_cores(int longp); //print all ip cores.
       void print_mem();            //print all ip cores, but sorting by address
       //Give a vendor/device list
       void set_vendor_list(Vendors * vlist){vendorlist=vlist;};
       //Search for a specific core.
       ipcore * search_core (int vendor, int idcore, bool ahb_apb, bool master_slave);
       private:
     };


