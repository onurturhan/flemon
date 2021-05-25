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
 
  Module: Comunicación Serie
  Description:
  Implementa una comunicación serie sobre la base de la abstracta.
  Basado en la revisión 1.6 de la RUT 07-333.
  
***************************************************************************/
/*****************************************************************************

 Target:      Any
 Language:    C++
 Compiler:    GNU g++ 4.1.1 (Debian GNU/Linux) [gcc 2.95.4-3.3.5]
 Text editor: SETEdit 0.5.5 [0.5.3-]

 Portability notes:
 El manejo es específico de sistemas POSIX.
    
*****************************************************************************/

#if !defined(CPPFIO_CSERIE_H)
#define CPPFIO_CSERIE_H

namespace cppfio
{

const int ComSerieDefMaxBuf=4000;
const int ComSerieB0=B0,
          ComSerieB50=B50,
          ComSerieB75=B75,
          ComSerieB110=B110,
          ComSerieB134=B134,
          ComSerieB150=B150,
          ComSerieB200=B200,
          ComSerieB300=B300,
          ComSerieB600=B600,
          ComSerieB1200=B1200,
          ComSerieB1800=B1800,
          ComSerieB2400=B2400,
          ComSerieB4800=B4800,
          ComSerieB9600=B9600,
          ComSerieB19200=B19200,
          ComSerieB38400=B38400,
          ComSerieB57600=B57600,
          ComSerieB115200=B115200,
          ComSerieB230400=B230400;
#ifdef B460800
// MacOS X doesn't have it
const int ComSerieB460800=B460800;
#endif

class ComSerie : public Communication
{
public:
 ComSerie(int aPort=2, int aBR=ComSerieB19200, int aTamB=ComSerieDefMaxBuf,
          const char *ttyName=NULL) throw(ExOpen);
 ~ComSerie() throw(ExClose);
 virtual void Initialize() throw(ExOpen,std::bad_alloc);
         void ShutDown() throw(ExClose);
 virtual void Send(const unsigned char *s, int l) throw(ExWrite);
         void Send(const char *s, int l) throw(ExWrite)
              { Send((const unsigned char *)s,l); };
         void Send(const char *s) throw(ExWrite)
              { Communication::Send(s); };
 virtual int  ReadResp(char *b) throw(ExWrite);
 virtual int  RawRead(char *buf, int size, bool exact) throw(ExWrite);
 static  void TaskChanged() throw();
 static  int  ConvertBaudRate(int rate) throw();

protected:
 bool wait_flag;
 int fd, bread, br;
 int port,bytesEsperados;
 const char *ttyName;

 struct termios oldtio;

 // For threaded approach
 pthread_t pollThread;
 static void *PollInput(void* arg);

 friend void SignalHandlerIO(int status) throw();
};

const int MaxComs=4;

} // namespace cppfio
#endif // CPPFIO_CSERIE_H
