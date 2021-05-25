/**[txh]********************************************************************

  Copyright (c) 2000-2008 Salvador E. Tropea <salvador en inti gov ar>
  Copyright (c) 2000-2008 Instituto Nacional de Tecnología Industrial

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, version 2 of the License.

  This program is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
  Module: Comunicación abstracta
  Description:
  Describe la clase base para realizar comunicaciones. Originalmente creado
para controlar instrumentos de laboratorio usando RS-232 y GPIB.
  Basado en la revisión 1.5 de la RUT 07-333.

***************************************************************************/
/*****************************************************************************

 Target:      Any
 Language:    C++
 Compiler:    GNU g++ 4.1.1 (Debian GNU/Linux) [gcc 2.95.4-3.3.5]
 Text editor: SETEdit 0.5.5 [0.5.3-]

 Portability notes:
 None.
    
*****************************************************************************/

#define CPPFIO_I
#define CPPFIO_I_string
#define CPPFIO_I_unistd
#define CPPFIO_Communication
#include <cppfio.h>

using namespace cppfio;

Communication::Communication()
 throw()
{
 initialized=false;
 elog=NULL;
 fwd=echo=NULL;
}

Communication::~Communication()
 throw(ExClose)
{
}

void Communication::Send(const char *s)
 throw(ExWrite)
{
 Send((const unsigned char *)s,strlen(s));
}

int Communication::ReadRespTO(char *b)
 throw(ExWrite,ExRead)
{
 usleep(30000);
 int ret=ReadResp(b);
 if (!ret)
   {// Darle un poco de tiempo
    usleep(30000*5);
    ret=ReadResp(b);
    if (!ret && elog)
      {
       if (elog)
          elog->Warning("read timeout");
       throw ExRead();
      }
   }
 return ret;
}

