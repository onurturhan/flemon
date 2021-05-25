/*
OPENEDCL - EDCL(Ethernet Debugging Communication Link) minimal client for Linux.
 Copyright (C) 2010  Alexander López Parrado

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

Description:

OPENEDCL Provides access to AHB devices on a Leon-3 based SoC using an
Ethernet connection between a host PC and a FPGA based board.

Tested under: Altera EP3C25 EEK.
Based on GRETH IP core documentation included in
/grlib/ip/grlib-gpl-1.0.22-b4075/doc/grip.pdf

Ing. Alexander López Parrado M.Sc. (2010)
Universidad del Quindío
*/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "grlib_edcl.h"

/*Function to init the EDCL link*/
int init_edcl_link(edcl_link_t *link,char *ip_address){

    /*measured sequence number*/
    uint16_t measured_sequence;

    /*Dummy read*/
    uint8_t dummy_byte;

    /*Copy IP address*/
    strcpy(&link->ip_address[0],ip_address);

    measured_sequence=0;
    do  {
        /*Change current sequence_number*/
        link->sequence_number=measured_sequence;
        /*increment measured sequence number (mod-16384)*/
        measured_sequence=(measured_sequence+1)&0x3fff;
        /*Perfoms dummy-reads on 0x00000000 to obtain current EDCL core sequence-number*/
        if(read_ahb_edcl(link,&dummy_byte,1,0x00000000)==READ_SUCCESS)
            break;
        if(read_ahb_edcl(link,&dummy_byte,1,0x00000000)==READ_TIMEDOUT)
            return EDCL_LINK_ERROR;

        }while(measured_sequence);

        /*If no EDCL present*/
    if(!measured_sequence)
        return EDCL_LINK_ERROR;
    /*EDCL link succes*/
    return EDCL_LINK_SUCCESS;
    }

/*Constructs EDCL 10 btyes header*/
void edcl_construct_header(uint8_t *buffer,uint8_t rw,uint32_t address,uint16_t length,uint16_t sequence_number){

    uint8_t k;

    /*offset bytes*/
    buffer[0]=0x0;
    buffer[1]=0x0;

    /*8 upper bits of sequence*/
    buffer[2]=(sequence_number>>6)&0xff;

    /*6 lower bits of sequence, r/w=1 , upper bit of length*/
    buffer[3]=((sequence_number & 0x3f)<<2) | (rw<<1) | ( (length & 0x200)>>8);

    /*8 mid bits of length*/
    buffer[4]= (length & 0x1ff)>>1;

    /*lower bit of length and unused bits*/
    buffer[5]= (length & 0x001)<<7;

    /*Store address bytes*/
    for(k=0;k<4;k++)
        buffer[9-k]=(address>>(8*k))&0xff;
    }

/*Reads nbytes bytes from AHB device with address address, readed bytes
are stored at buffer.*/
int read_ahb_edcl (edcl_link_t *link,uint8_t  *buffer, uint16_t nbytes, uint32_t address){

    /*Global variables*/
    int sock;
    struct sockaddr_in echoserver;
    struct sockaddr_in echoclient;

    /*pointer to EDCL protocol bytes */
    uint8_t *temp_buffer;

    uint16_t k,r_sequence_number;

    unsigned int clientlen,received;

    /*For reading timeout*/
    fd_set readfds, masterfds;
    struct timeval timeout;

    /*set the timeout to TIME_OUT ms*/
    timeout.tv_sec = 0;
    timeout.tv_usec = TIME_OUT;

    /*Allocates memory for temp buffer*/
    temp_buffer=(uint8_t *)malloc((nbytes+10)*sizeof(uint8_t));

    /*Construct header for read operation*/
    edcl_construct_header(temp_buffer,0,address,nbytes,link->sequence_number);

    /* Create the UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        free(temp_buffer);
        return SOCKET_CREATE_ERROR;
    }
    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = inet_addr(link->ip_address); /* IP address */
    echoserver.sin_port = htons(1234);       /* server with dummy port */

    /* Send the word to the server */
    if (sendto(sock, temp_buffer, 10, 0,(struct sockaddr *) &echoserver,sizeof(echoserver)) != 10) {
        close(sock);
        free(temp_buffer);
        return SOCKET_SEND_ERROR;
    }

    FD_ZERO(&masterfds);
    FD_SET(sock, &masterfds);
    memcpy(&readfds, &masterfds, sizeof(fd_set));
    select(sock+1, &readfds, NULL, NULL, &timeout);
    /* Receive the word back from the server */
    clientlen = sizeof(echoclient);

    if (FD_ISSET(sock, &readfds)){
        if ((received = recvfrom(sock, temp_buffer, nbytes+10, 0,(struct sockaddr *) &echoclient,&clientlen)) != ((nbytes)+10)) {
            close(sock);
            free(temp_buffer);
            return SOCKET_RECEIVE_ERROR;
            }
    }else   {
        close(sock);
        free(temp_buffer);
        return READ_TIMEDOUT;
        }


    /* Check that client and server are using same socket */
    if (echoserver.sin_addr.s_addr != echoclient.sin_addr.s_addr) {
        close(sock);
        free(temp_buffer);
        return SOCKET_PACKET_ERROR;
        }

    /*Read received data bytes*/
    for (k=0;k<nbytes;k++)
        buffer[k]=temp_buffer[10+k];

    r_sequence_number= (((uint16_t)temp_buffer[2]) << 6 ) | (temp_buffer[3]>>2);
    /*Update sequence number if no error*/
    if(link->sequence_number==r_sequence_number)
        link->sequence_number=(link->sequence_number+1)&0x3fff;

    close(sock);
    free(temp_buffer);
    return READ_SUCCESS;
    }


/*Writes nbytes bytes stored at buffer to AHB device with address address.*/
int write_ahb_edcl (edcl_link_t *link,uint8_t  *buffer, uint16_t nbytes, uint32_t address){

    /*Global variables*/
  int sock;
  struct sockaddr_in echoserver;
  struct sockaddr_in echoclient;


  uint8_t *temp_buffer;

  uint16_t k,r_sequence_number;

  unsigned int  clientlen,received;

  fd_set readfds, masterfds;
   struct timeval timeout;

      /*set the timeout to TIME_OUT ms*/
   timeout.tv_sec = 0;
   timeout.tv_usec = TIME_OUT;

  temp_buffer=(uint8_t *)malloc((nbytes+10)*sizeof(uint8_t));

  edcl_construct_header(temp_buffer,1,address,nbytes,link->sequence_number);

  /*Copy user data to temp_buffer*/
   for (k=0;k<nbytes;k++)
      temp_buffer[10+k]=buffer[k];

 /* Create the UDP socket */
if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    free(temp_buffer);
   return SOCKET_CREATE_ERROR;
}

/* Construct the server sockaddr_in structure */
memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
echoserver.sin_family = AF_INET;                  /* Internet/IP */
echoserver.sin_addr.s_addr = inet_addr(link->ip_address); /* IP address */
echoserver.sin_port = htons(1234);       /* server with dummy port */

/* Send the word to the server */
if (sendto(sock, temp_buffer, nbytes+10, 0,
           (struct sockaddr *) &echoserver,
           sizeof(echoserver)) != (nbytes+10)) {

    close(sock);
    free(temp_buffer);
  return SOCKET_SEND_ERROR;
}

   FD_ZERO(&masterfds);
   FD_SET(sock, &masterfds);
   memcpy(&readfds, &masterfds, sizeof(fd_set));
   select(sock+1, &readfds, NULL, NULL, &timeout);


  /* Receive the word back from the server */

  clientlen = sizeof(echoclient);

   if (FD_ISSET(sock, &readfds))
   {
       if ((received = recvfrom(sock, temp_buffer, 10, 0,
                           (struct sockaddr *) &echoclient,
                           &clientlen)) != (10)) {
    close(sock);
    free(temp_buffer);

   return SOCKET_RECEIVE_ERROR;
  }

   }
   else
   {
     return READ_TIMEDOUT;
   }

  /* Check that client and server are using same socket */
  if (echoserver.sin_addr.s_addr != echoclient.sin_addr.s_addr) {

       close(sock);
       free(temp_buffer);

    return SOCKET_PACKET_ERROR;
  }

/*Received sequence number*/
r_sequence_number= (((uint16_t)temp_buffer[2]) << 6 ) | (temp_buffer[3]>>2);
/*Update sequence number if no error*/
if(link->sequence_number==r_sequence_number)
    link->sequence_number=(link->sequence_number+1)&0x3fff;


    close(sock);
    free(temp_buffer);

return READ_SUCCESS;

}

/*Writes nwords 32-bit words stored at buffer to AHB device with address address.*/
int write_ahb_edcl32 (edcl_link_t *link,uint32_t  *buffer, uint16_t nwords, uint32_t address){

    uint16_t k;
    uint32_t aux;

    /*Reorder no matter endianess*/
    for(int k=0;k<nwords;k++){
        aux=buffer[k];
       (*(( uint8_t *)buffer+k*4+0))=(aux>>24)&0xff;
        (*(( uint8_t *)buffer+k*4+1))=(aux>>16)&0xff;
        (*(( uint8_t *)buffer+k*4+2))=(aux>>8)&0xff;
        (*(( uint8_t *)buffer+k*4+3))=(aux)&0xff;
     }

    return(write_ahb_edcl(link,(uint8_t *)buffer, nwords*sizeof(uint32_t),address));

}

/*Reads nwords 32-bit words from AHB device with address address, readed words
are stored at buffer.*/
int read_ahb_edcl32 (edcl_link_t *link,uint32_t  *buffer, uint16_t nwords, uint32_t address){

   // uint16_t k;

    int ret_value;

    ret_value=read_ahb_edcl(link,(uint8_t *)buffer, nwords*sizeof(uint32_t),address);

    /*Reorder no matter endianess*/
    for(int k=0;k<nwords;k++){
        buffer[k]=        ((uint32_t)(*(( uint8_t *)buffer+k*4+0))<<24)|
        ((uint32_t)(*(( uint8_t *)buffer+k*4+1))<<16)|
        ((uint32_t)(*(( uint8_t *)buffer+k*4+2))<<8)|
        (*(( uint8_t *)buffer+k*4+3));
    }

    return ret_value;

}
