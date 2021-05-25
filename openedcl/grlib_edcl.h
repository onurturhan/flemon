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

#ifndef LEON3_EDCL_H_INCLUDED
#define GRLIB_EDCL_H_INCLUDED

#include <stdint.h>

#define TIME_OUT 10000

/*EDCL link structure*/
struct edcl_link{
    char ip_address[16];
    uint16_t sequence_number;
};

typedef struct edcl_link edcl_link_t;

int read_ahb_edcl (edcl_link_t *link,uint8_t  *buffer, uint16_t nwords, uint32_t address);
int write_ahb_edcl (edcl_link_t *link,uint8_t  *buffer, uint16_t nbytes, uint32_t address);
int read_ahb_edcl32 (edcl_link_t *link,uint32_t  *buffer, uint16_t nwords, uint32_t address);
int write_ahb_edcl32 (edcl_link_t *link,uint32_t  *buffer, uint16_t nwords, uint32_t address);
int init_edcl_link(edcl_link_t *link,char  *ip_address);

/*Functions return values*/
enum {SOCKET_CREATE_ERROR,SOCKET_SEND_ERROR,SOCKET_RECEIVE_ERROR,SOCKET_PACKET_ERROR,READ_SUCCESS,READ_TIMEDOUT,EDCL_LINK_SUCCESS,EDCL_LINK_ERROR};

#endif // GRLIB_EDCL_H_INCLUDED
