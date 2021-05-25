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
#include "flemon.h"
#include "grlib.h"

#define CPPFIO_I
#define CPPFIO_PRegEx
#define CPPFIO_TextFile
#include <cppfio.h>

using namespace cppfio;

//First class list pointer for vendors
Vendors * Vendors::first_vendor=NULL;

/**********************************************************************
  Description: Clean all cores
***********************************************************************/

void Grlib::clean()
{
int cont;
for (cont=0;cont<(SCAN_AHB_MASTER+SCAN_AHB_SLAVE+SCAN_APB_SLAVE);cont++)
   {
    cores[cont].clean(); //Clean all cores.
   }

for (cont=0;cont<(SCAN_AHB_MASTER+SCAN_AHB_SLAVE);cont++)
   {
    ahb[cont].order=0;
    ahb[cont].ireg=0;
   }
for (cont=0;cont<SCAN_APB_SLAVE;cont++)
   {
    apb[cont].order=0;
    apb[cont].ireg=0;
   }
}

/**********************************************************************
  Description: get index of the last valid core.
  Return: index of last core. TODO. Use pointers.
**********************************************************************/

int Grlib::get_lastcore()
{
  int cont;
  for (cont=0;
       cont<(SCAN_AHB_MASTER+SCAN_AHB_SLAVE+SCAN_APB_SLAVE)&&
       cores[cont].is_empty()==FALSE;cont++);

 if(cont>=(SCAN_AHB_MASTER+SCAN_AHB_SLAVE+SCAN_APB_SLAVE))
   exit(1); //TODO: cores must be dinamically created.
 return cont;
}

/*********************************************************************
  Description: Add a "virtual" core as de AHB Plug&Play area.
  Adds an extra core when you list cores. Usefull when sorting
  cores by address to see used places.
**********************************************************************/

void Grlib::addcore_PP()
{
     int  lastcore;
     lastcore=get_lastcore();

     cores[lastcore].set_PParea();
     cores[lastcore].read_vendor(vendorlist);
}

/*********************************************************************
  Description: Scans Plug&Play area of the AHB bus. Stores information
  in ahb_slots. This is raw information. No interpretation.
**********************************************************************/

int Grlib::scan_ahb()
{
unsigned char slot;
int res, cont;
unsigned  readingvalues [8], slotaddress;

ahb_slot *myslot;

     cont=0;
     for (slot=0; slot<SCAN_AHB_MASTER; slot++)
       {
         myslot=&ahb[cont];
         slotaddress=0xfffff000 + (slot*0x20);
         //printf("Reading AHB.\n");
         res= read_ahb (readingvalues,8, slotaddress );
         if(res) { printf("  Error reading AHB.\n");return ERROR;}
         ahb[cont].order=cont;
         ahb[cont].slotaddress = slotaddress;
         ahb[cont].ireg = readingvalues[0];
         ahb[cont].user[0] = readingvalues[1];
         ahb[cont].user[1] = readingvalues[2];
         ahb[cont].user[2] = readingvalues[3];
         ahb[cont].bar[0] = readingvalues[4];
         ahb[cont].bar[1] = readingvalues[5];
         ahb[cont].bar[2] = readingvalues[6];
         ahb[cont].bar[3] = readingvalues[7];
         cont++;
      }
     //Slaves scanning
     cont=MAX_AHB_MASTER;

     for (slot=MAX_AHB_MASTER; slot< (MAX_AHB_MASTER+SCAN_AHB_SLAVE); slot++)
       {
         slotaddress=0xfffff000 + (slot*0x20);
         myslot=&ahb[SCAN_AHB_MASTER+cont-MAX_AHB_MASTER];
         res= read_ahb (readingvalues,8, slotaddress );
         if(res) {return -1;}
         myslot->order = cont;
         myslot->slotaddress = slotaddress;
         myslot->ireg = readingvalues[0];
         myslot->user[0] = readingvalues[1];
         myslot->user[1] = readingvalues[2];
         myslot->user[2] = readingvalues[3];
         myslot->bar[0] = readingvalues[4];
         myslot->bar[1] = readingvalues[5];
         myslot->bar[2] = readingvalues[6];
         myslot->bar[3] = readingvalues[7];
         cont++;
      }
return SUCCESS;
}

/**********************************************************************
  Description: Nice Printing of the AHB scanning result.
***********************************************************************/

void Grlib::print_scan_ahb()
{
  int cont;
  unsigned slot;
  ahb_slot * ahb_myslot;
  
  printf ("\nAHB   N  slotADDR   IDValue    BAR0       BAR1       BAR2       BAR3\n");
     for (slot=0; slot<(SCAN_AHB_MASTER+SCAN_AHB_SLAVE); slot++)
     {
         ahb_myslot=&ahb[slot];

         if(ahb_myslot->ireg&0xff000000) //vendorid non zero
           {
            printf ("%s %02d 0x%08X 0x%08X ",
            slot<SCAN_AHB_MASTER ? "Mastr":"Slave",ahb[slot].order, ahb_myslot->slotaddress, ahb_myslot->ireg);
            for (cont=0;cont<4;cont++)
               if (ahb_myslot->bar[cont]) printf("0x%08X ",ahb_myslot->bar[cont]);
            printf("\n");
           }
       }
}

/**********************************************************************
  Description: Create core information based on AHB scanning results.
***********************************************************************/

void Grlib::ahb2core()
{
int cont,nunit;
nunit=0;

//Take into account cores already present
 nunit=get_lastcore();
for (cont=0; cont <SCAN_AHB_MASTER+SCAN_AHB_SLAVE ; cont++)
   {
   
   if (ahb[cont].ireg&0xff000000)                        //Vendor ID is not empty
     {
     cores[nunit].set_ahbslot(&ahb[cont]);
     cores[nunit].read_ahbslot_data();
     cores[nunit].read_vendor(vendorlist);

     nunit++; //Point to next unit
     }
   }
}

/**********************************************************************
  Description: Search for the first AHB/APB bridge available.
  Return: TRUE or FALSE. If TRUE, bridge is stored in grlib struct.
***********************************************************************/

int Grlib::search_bridges()
{
  int cont;
     for (cont=0; cont<(SCAN_AHB_MASTER+SCAN_AHB_SLAVE+SCAN_APB_SLAVE); cont++)
     {
         if(cores[cont].is_bridge()) //vendorid Gaisler and is an AHB/APB bridge
            bridge = &cores[cont];
     }
  if (bridge != NULL)
     { printf("Bridge found..\n");return TRUE;}
    else
     {printf("NO bridges found..\n"); return FALSE;}
}

/***************************************************************
 Description: Search for the presence of given vendor/device
 core in APB or AHB bus, as master or as slave.
 Return: A valid pointer if found, NULL if not.
****************************************************************/

ipcore * Grlib::search_core( int vendor, int idcore, bool ahb_apb, bool master_slave)
{
  int cont;
     for (cont=0; cont<(SCAN_AHB_MASTER+SCAN_AHB_SLAVE+SCAN_APB_SLAVE); cont++)
     {
         if( cores[cont].is_ahb()==ahb_apb &&
             cores[cont].is_master()==master_slave &&
             cores[cont].get_vid()==vendor &&
             cores[cont].get_did()==idcore)
             {
             //printf("YES found..\n");
             return &cores[cont];
             }
     }
  return NULL;
}

/**********************************************************************
  Description:  Create a "virtual" core with the APB P&P information.
  This is usefull to see used addresses.
***********************************************************************/

void Grlib::addcore_bridgePP()
{
   int lastcore;
   unsigned startaddr;
   //now we add  bridge area as a system core.
   lastcore=get_lastcore();

        if (bridge==NULL) return;
        
        startaddr=bridge->get_startaddress(0);
        cores[lastcore].set_bridgePParea(startaddr);
        cores[lastcore].read_vendor(vendorlist);
}

/**********************************************************************
  Description: Scan APB bus.
  Return: Nothing, values are stored in apb slots .
***********************************************************************/

int Grlib::scan_apb()
{
 int order, slot, res;
 unsigned  startaddress, slotaddress, readingvalues[2];
 order=0;

 if (bridge!=NULL) //scans available AHB/APB Bridge
    {
     startaddress=bridge->get_startaddress(0) |0x000ff000; //XXX CHECK THIS!!!
    for (slot=0; slot<SCAN_APB_SLAVE; slot++) //How many slots should be scanned?
       {
        slotaddress=startaddress + (slot*0x08);
        //printf ("Reading APB.\n");
        res= read_ahb (readingvalues,2, slotaddress );
         if(res) { printf ("Error Reading APB\n.");  return ERROR;}//exit(1);}
        apb[slot].order=++order;
        apb[slot].slotaddress = slotaddress;
        apb[slot].ireg = readingvalues[0];
        apb[slot].bar  = readingvalues[1];
       }
    }
return SUCCESS;
}

/**********************************************************************
  Description: Print raw values reading from APB bus scan.
  Return: Print reading values.
************************************************************************/

void Grlib::print_scan_apb()
{
  unsigned slot;
  printf ("BUS%d  slotADDR   IDValue    BAR\n",0);
     for (slot=0; slot<SCAN_APB_SLAVE; slot++)
     {
         if(apb[slot].ireg&0xff000000) //vendorid non zero
           {
            printf ("%02d    0x%08X 0x%08X ",
                     slot+1, apb[slot].slotaddress, apb[slot].ireg);
            printf("0x%08X ",apb[slot].bar);
            printf("\n");
           }
     }
}

/**********************************************************************
  Description: Prints the bridge information.
***********************************************************************/

void Grlib::print_bridge()
{
 if(bridge!=NULL)
 { printf("Bridge: "); bridge->print();}
 else printf ("No bridges. ");
}

/*********************************************************************

  Description: Fills cores information based on scanned APB bus.
  Return: Nothing, values are stored in ipcore classes.
**********************************************************************/

void Grlib::apb2core()
{
int cont;
unsigned  reg;
int  add;

add=get_lastcore();

for (cont=0; cont<SCAN_APB_SLAVE ; cont++)
   {
     reg=apb[cont].ireg;

     if (reg&0xff000000)                        //Vendor ID is not empty
       {
         cores[add].set_apbslot(&apb[cont]);
         cores[add].read_apbslot_data();
         cores[add].read_vendor(vendorlist);
         add++;
       }
   }
}

/**********************************************************************
  Description: Print all detected and virtual cores.
***********************************************************************/

void Grlib::print_cores(int longp)
{
 int cont;
  for (cont=0; cont<(SCAN_AHB_MASTER+SCAN_AHB_SLAVE+SCAN_APB_SLAVE); cont++)
  {
    if( cores[cont].is_empty()==FALSE)
       {printf ("%02d- ",cont+1);
       if (longp==0) cores[cont].print();
        else cores[cont].lprint();
       printf("\n");
       }
  }
}
/**********************************************************************
  Description: Report detected address map (AHB/APB). Sorted by address.
  Return: Nothing.
***********************************************************************/

void Grlib::print_mem()
{
ipcore  *cmp1, *cmp2;
int cont,cont2, order, max, postemp,bartemp,idx;
int position[SCAN_APB_SLAVE+((SCAN_AHB_MASTER+SCAN_AHB_SLAVE)*3)][2]; //stores unit and BAR index.
unsigned  val1, val2;

order=0; idx=0;
cont=0;
  //fill position with defined core & bar index
  while (cores[cont].get_vid())
       {
         for(cont2=0;cont2<5;cont2++)
          {
            if(cores[cont].get_addrtype(cont2))
              { position[order][0]=cont;;
                position[order][1]=cont2;
                order++;
              }
          }
        cont++;
       }
max=order;

//Bubble sort
for(cont=0;cont<max-1;cont++)
   {
    for(cont2=0;cont2<max-1;cont2++)
       {
        cmp1=&cores[position[cont2][0]];  //pointer to core 1
        cmp2=&cores[position[cont2+1][0]];//pointer to core 2
        val1=cmp1->get_startaddress(position[cont2][1]); //Value1 to compare
        val2=cmp2->get_startaddress(position[cont2+1][1]); //Value2 to compare
        if(val1>val2)  //If value swapping is nedeed
         {
           postemp=position[cont2][0];               //save core index
           bartemp=position[cont2][1];               //save core.bar index
           position[cont2][0]=position[cont2+1][0];  //swap first
           position[cont2][1]=position[cont2+1][1];
           position[cont2+1][0]=postemp;             //end swap operation
           position[cont2+1][1]=bartemp;
         }
       }
    }
     //print sorted
     for (cont=0; cont<max; cont++)
     {
       cmp1=&cores[position[cont][0]];
       bartemp=position[cont][1];

       printf("%s: %c%08X-%08X%c ", cmp1->get_addrtypestr(bartemp),
                                    (cmp1->get_did()==0x006)?'[':' ',
                                    cmp1->get_startaddress(bartemp),
                                    cmp1->get_endaddress(bartemp),
                                    (cmp1->get_did()==0x006)?']':' ');
       cmp1->printdevice(); printf("\n");
     }
}


/*################# ipcores class functions ######################################*/

/**********************************************************************
  Description: Prints core information.
***********************************************************************/

void ipcore::print()
{
printdevice();
printf("  ");
printvendor();
}

/**********************************************************************
  Description: Prints core information. Only Vendor name.
***********************************************************************/

void ipcore::printvendor()
{
if(myvendor!=NULL)
  {
   printf("%-30s",myvendor->get_string());
  }
 else
  printf("VendorID:0x%02X", vendorid);
}

/**********************************************************************
  Description: Prints core information. Only device id.
***********************************************************************/

void ipcore::printdevice()
{

if(mydevice !=NULL)
 {
 printf("%-44s",mydevice->get_string());
 }
else
printf("DeviceID:0x%02X  Version:0x%02X IRQ:0x%02X        ",deviceid,version,irq);
}

/*********************************************************************
  Description: Prints core information. Long version.
**********************************************************************/

void ipcore::lprint()
{
 print();
 printf("\n  VID:0x%02X DID:0x%03X  Ver:0x%02X ", vendorid, deviceid,version);
 if (irq) printf("Irq:%d ", irq);
 if(slotahb!=NULL)                                 //Is a core in the AHB bus
   { if(slotahb->order<64)
        printf("ahb_mastr ");      //If it is a master
       else  printf("ahb_slv ");   //is a slave
     printf("Slot:%d ",slotahb->order);
   }
 if (slotapb!=NULL)                       //is an APB core
   {
    printf("apb ");
    printf("Slot:%d ",slotapb->order);
   }
 print_banks();
}

/**********************************************************************
  Description: Prints all detected memory banks. BAR
***********************************************************************/

void ipcore::print_banks()
{
int cont;
for(cont=0;cont<4 && cont<nbr_banks;cont++)
   {
    printf("\n   ");
    switch (addrtype[cont])
     { case APB_IO :  printf("apb i/o"); break;
       case AHB_MEM:  printf("ahb mem"); break;
       case AHB_IO:   printf("ahb i/o"); break;
       default:       printf("Undef  ");
     }
    printf(" 0x%08X-0x%08X ",saddress[cont],eaddress[cont]);
    if (prefetch[cont])  printf("Prefetchable ");
    if (cacheable[cont]) printf("Cacheable ");
   }
}

/*********************************************************************
  Description: Calculate start and end addresses based on BAR (Bank
  Address Registers) information. Calculation of start and end address
  depends on type.
  Return: FALSE if memory type is invalid. TRUE if valid type.
***********************************************************************/

int ipcore::addr_bar_extract(unsigned bar, unsigned * saddress,
                    unsigned * eaddress)
{
   unsigned  type;
   type=bar & 0x0000000f;

  //From doc/grlib.pdf page 34
  if (type==AHB_MEM)
  {
   *saddress=bar&0xfff00000;
   *eaddress=*saddress | (~(bar<<16) | 0x000fffff);
  }
  else if(type==AHB_IO)  //You have to add base AHB/APB bridge address later 
  {
   //BAR.ADDRESS [19:8]. ADDRESS[31:20] fixed to 0xFFF.
   *saddress=((bar>>12)&0xffffff00)|0xfff00000;
   *eaddress=*saddress | (~(bar<<4)) | 0x000000ff;
  }
  else if (type==APB_IO)
  {
   //BAR.ADDRESS [19:8]. ADDRESS[31:20] fixed to 0xFFF.
   *saddress=((bar>>12)&0x000fff00); //clear ahb/apb base address.
   *eaddress=(*saddress | (~(bar<<4)) | 0x000000ff)&0x000fffff;
  }
  else
  {
  return FALSE; //Unknowmn BAR type
  }
*eaddress+=1; //To report like grmon.
return TRUE;
}

/**********************************************************************
  Description: Read data from AHB scanning an translates to ipcore data.
***********************************************************************/

void ipcore::read_ahbslot_data()
 {
  unsigned reg,bar;
  int cont;
  
  reg=slotahb->ireg;
      vendorid=(unsigned char)(reg>>24);
      deviceid=(unsigned int)(reg>>12)&0x0fff;
      version=(unsigned char)(reg>>5)&0x1f;
      irq=(unsigned char)reg&0x1f;
      nbr_banks=0;
      collapse=NULL;
      for (cont=0;cont<4;cont++)
         {
          bar=slotahb->bar[cont];
          if(bar!=0)
            {nbr_banks++;
             prefetch[cont]    =(unsigned char) (bar>>17)&0x01 ;
             cacheable[cont]   =(unsigned char) (bar>>16)&0x01 ;
             addrtype[cont]        =(unsigned char) (bar & 0x0000000f);
             addr_bar_extract(bar,&saddress[cont],&eaddress[cont]);
            }
         }
 }

/**********************************************************************
  Description: Takes a vendor list and searchs for own vendor & device.
***********************************************************************/

void ipcore::read_vendor(Vendors * vl)
{
  myvendor=vl->search_vendor(vendorid);

  if (myvendor!=NULL)
     { mydevice=myvendor->search_device(deviceid);
    }
}

/**********************************************************************
  Description: Read data from APB scanning an translates to ipcore data.
***********************************************************************/

void ipcore::read_apbslot_data()
{
  unsigned reg,bar;
  reg=slotapb->ireg;
  bar=slotapb->bar;

         vendorid=(unsigned char)(reg>>24);
         deviceid=(unsigned int)(reg>>12)&0x0fff;
         version=(unsigned char)(reg>>5)&0x1f;
         irq=(unsigned char)reg&0x1f;
         nbr_banks=0;
         if(bar!=0)
           {nbr_banks=1;
            addrtype[0]=(unsigned char) (bar & 0x0000000f);
            addr_bar_extract(bar, &saddress[0],&eaddress[0]);
            saddress[0]|=slotapb->slotaddress&0xfff00000;
            eaddress[0]|=slotapb->slotaddress&0xfff00000;
           }

}

/**********************************************************************
  Description: Set this core as the bridge P&P area for APB bus.
  This is a "virtual core", only to see bus addresses on listing.
************************************************************************/

void ipcore::set_bridgePParea(unsigned saddr)
{
        vendorid=0x01;
        deviceid=0xffe;
        irq=0;
        nbr_banks=1;
        saddress[0]=saddr|AREA_CFG;
        eaddress[0]=(saddr|AREA_CFG)+(4*APB_WORD*16);
        addrtype[0]=APB_IO;
        cacheable[0]=0;
        prefetch[0]=0;
        myvendor=NULL;
        mydevice=NULL;
}

/**********************************************************************
  Description: Set this core as  Plug&Play area in AHB bus.
  This is a "virtual core", only to see bus addresses on listing.
************************************************************************/
void ipcore::set_PParea()

{
  vendorid=0x01;
  deviceid=0xfff;
  version=0;
  irq=0;
  nbr_banks=2;
  saddress[0]=0xfffff000; eaddress[0]=0xfffff000+(4*8*64);
  addrtype[0]=AHB_MEM;
  cacheable[0]=0; prefetch[0]=0;
  saddress[1]=0xfffff800; eaddress[1]=0xfffff800+(4*8*64);
  addrtype[1]=AHB_MEM;
  cacheable[1]=0; prefetch[1]=0;
  myvendor=NULL;
  mydevice=NULL;
}

/*################# Vendor class functions#################################*/

/**********************************************************************
  Description: reads a file and extracts Vendors and Devices for each
  vendor.
  Return:   TRUE if success;
***********************************************************************/

int Vendors::readfile( const char *filename)
{
 TextFile f(filename);
 int  len, actualvendor=0;
 int device;

 PRegEx regex("\\s*([^\\s:=]+)\\s*[:= ]\\s*\"(.*)\"\\s*");
 PRegEx regex2("\\s*VENDOR\\s*[:= ]([^\\s:=]+)\\s*[:= ]\\s*\"(.*)\"\\s*");

 char *s, *end;
 while ((s=f.ReadLineNoEOL(len))!=NULL)
   {// Execute the search in each line
    //Ignore all after #
    for(end=s; *s && *s!='#' && end-s<len ; end++);
    *end=0;
    len=end-s+1;
    
    if(regex2.search(s,len))            //Is a VENDOR LINE
       {
         int c=regex2.cantMatches();
         if(c==3)
          {
           char *s=regex2.getMatchStr(1,len);
              actualvendor=strtol(s,NULL,0);
              delete[] s;
              s=regex2.getMatchStr(2,len);
              add_vendor(actualvendor,s);
              delete[] s;
           vlines++;
          }
       }
    else if (regex.search(s,len) && actualvendor!=0)   //is a DEVICE LINE
      {
       int c=regex.cantMatches();
       if(c==3)
          {
           char *s=regex.getMatchStr(1,len);
           device=strtol(s,NULL,0);
           delete[] s;
           s=regex.getMatchStr(2,len);
           //printf ("%03X %s\n",device,s);
           add_device(actualvendor,device,s);
           delete[] s;
           dlines++;
          }
      }
   }
 return TRUE;
}

/**********************************************************************
  Description: Add a new vendor to the list.
***************************************************************************/

void Vendors::add_vendor(int vid, const char * desc )
{
  Vendors  * last;
  //First one already exist and it's empty
  if(first_vendor==NULL)
    {
     this->vendorid=vid;
     this->string=newStr(desc);
     first_vendor=this;
     this->devlist=NULL;
     return;
    }
  //search last vendor
  for(last=first_vendor;last->next!=NULL;last=last->next);

   //new vendor
   last->next=new Vendors();
   last=last->next;
   last->vendorid=vid;
   last->string=newStr(desc);
   last->devlist=NULL;
}

/**********************************************************************
  Description: Print all vendors in the list.
***********************************************************************/

void Vendors::printlist()
{
 Vendors * vp;
 for (vp=first_vendor; vp!=NULL; vp=vp->next)
  { vp->print(); vp->printdevs();}
}

/**********************************************************************
  Description: Prints only this vendor.
***********************************************************************/

void Vendors::print()
{
printf ("VendorID:%02X Name: %s\n",vendorid, (string==NULL)?"Unknown":string );
}

/**********************************************************************
  Description: Prints all devices for this vendor.
***********************************************************************/

void  Vendors::printdevs()
{
 Devices * hit;
  for(hit=devlist;hit!=NULL;hit=hit->get_next())
  {
   printf("  "); hit->print();
  }
}

/**********************************************************************
  Description:  Search for a vendor inside the list.
  Return: A pointer to the Vendor or NULL if none was found.
************************************************************************/

Vendors * Vendors::search_vendor(int vid)
{
 Vendors * last;
  for(last=first_vendor;last!=NULL;last=last->next)
  {
   if (last->vendorid==vid) return last;
  }
 return NULL;
}

/**********************************************************************
  Description: Search for a device inside this vendor.
  Return: A pointer to the Device found or NULL.
***********************************************************************/

Devices * Vendors::search_device(int did)
{
 Devices * hit;
  for(hit=devlist;hit!=NULL;hit=hit->get_next())
  {
   if (hit->get_id()==did) return hit;
  }
 return NULL;
}

/**********************************************************************
  Description: Adds a new device to an existing vendor.
  First we have too look for the vendor, an then look
  at the device list inside that vendor. A new device
  is created.
***************************************************************************/

void Vendors::add_device(int vid, int did, const char * desc )
{
  Vendors * add;
  Devices * dev;
  //Search Vendor for adding a device
  add=search_vendor(vid);
  if (add==NULL) { return;} //Sorry, No valid vendor exist
   //Create de device
   dev=new Devices(did,desc);
   
   if(add->devlist==NULL)    //Is the first device?
     {
       add->devlist=dev;    //Link first device to list pointer
     }
   else
     { Devices * last;
       //Search last device in the list
       for(last=add->devlist; last->get_next()!=NULL; last=last->get_next());
       last->set_next(dev);  //Link new device to last device in the list
     }
  //dev->print(); //print new device
}


