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

#if !defined(CPPFIO_COMMUNIC_H)
#define CPPFIO_COMMUNIC_H

namespace cppfio
{

class Communication
{
public:
 Communication() throw();
 virtual ~Communication() throw(ExClose);
 virtual void  Initialize() throw(ExOpen,std::bad_alloc)=0;
 virtual void  Send(const unsigned char *s, int l) throw(ExWrite)=0;
         void  Send(const char *s) throw(ExWrite);
         void  Send(const char *s, int l) throw(ExWrite)
               { Send((const unsigned char *)s,l); };
 virtual int   ReadResp(char *b) throw(ExRead,ExWrite,std::bad_alloc)=0;
         int   ReadRespTO(char *b) throw(ExWrite,ExRead);
 virtual int   RawRead(char *buf, int size, bool exact)
                 throw(ExRead,ExWrite,std::bad_alloc)=0;
         char *GetBuffer() throw() { return buffer; }
         void  SetErrorLog(ErrorLog *aEL) throw() { elog=aEL; }
         int   GetBufSize() throw() { return tamBuffer; }
         void  SetForward(Communication *aFwd) throw() { fwd=aFwd; }
         void  SetEcho(Communication *anEcho) throw() { echo=anEcho; }

protected:
 bool initialized;
 char *buffer;
 int tamBuffer;
 ErrorLog *elog;
 Communication *fwd;
 Communication *echo;
};

} // namespace cppfio
#endif // CPPFIO_COMMUNIC_H
