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
 
  Module: Serial Communication
  Description:
  Implements a serial communication over the abstract Communication class.
  Basado en la revisión 1.8 de la RUT 07-333.
  Cygwin support added by Hans Hübner, uses a separated thread that waits
  until something arrives using select.
  
***************************************************************************/
/*****************************************************************************

 Target:      Any
 Language:    C++
 Compiler:    GNU g++ 4.3.1 (Debian GNU/Linux) [gcc 2.95.4/3.3.5/4.1.1]
 Text editor: SETEdit 0.5.5 [0.5.3-]

 Portability notes:
 POSIX specific. Can use asynchronous signaling or a separated thread.
    
*****************************************************************************/

#define CPPFIO_I
#define CPPFIO_I_stdio
#define CPPFIO_I_stdlib
#define CPPFIO_I_unistd
#define CPPFIO_I_string
#define CPPFIO_ComSerie
#include <cppfio.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/select.h>
#include <ctype.h>

namespace cppfio
{
void SignalHandlerIO(int status) throw();
} // namespace cppfio

using namespace cppfio;

#define DEBUG_ENVIA 0

// Hasta 4 coms al mismo tiempo son soportados
static ComSerie *SHObj[MaxComs]={0,0,0,0};
static int       SHSignalOn=0;
static sigset_t  blockMask;
static int       blockMaskInitialized=0;
static pthread_mutex_t mutex;

ComSerie::ComSerie(int aPort, int BaudRate, int aTamB, const char *name)
  throw(ExOpen) :
  Communication()
{
 wait_flag=true;
 fd=-1;
 bread=0;
 buffer=NULL;
 tamBuffer=aTamB;
 port=aPort-1;
 br=BaudRate;
 ttyName=name;

 int i;
 for (i=0; i<MaxComs; i++)
    {
     if (!SHObj[i])
       {
        SHObj[i]=this;
        break;
       }
    }
 if (i==MaxComs)
    throw ExOpen();
}

void ComSerie::ShutDown()
 throw(ExClose)
{
 delete[] buffer;
 buffer=0;
 if (fd!=-1)
   {
    tcsetattr(fd,TCSANOW,&oldtio);
    if (close(fd)==-1)
       throw ExClose();
   }
 fd=-1;
 initialized=false;
}

ComSerie::~ComSerie()
 throw(ExClose)
{
 int i;
 for (i=0; i<MaxComs; i++)
     if (SHObj[i]==this)
       {
        SHObj[i]=0;
        break;
       }
 ShutDown();
}

void ComSerie::TaskChanged()
 throw()
{
 SHSignalOn=0;
}

void ComSerie::Initialize()
 throw(ExOpen,std::bad_alloc)
{
 struct termios newtio;
 struct sigaction saio;           /* definition of signal action */
 sigset_t block_mask;

 if (initialized)
    ShutDown();
 bytesEsperados=-1;


 if (ttyName)
    fd=open(ttyName,O_RDWR | O_NOCTTY | O_NONBLOCK);
 else
   {
    char dev[32];
    sprintf(dev,"/dev/ttyS%d",port<100 && port>=0 ? port : 0);
    /* open the device to be non-blocking (read will return immediatly) */
    fd=open(dev,O_RDWR | O_NOCTTY | O_NONBLOCK);
   }
 if (fd<0)
    throw ExOpen();
 //printf("Nuevo file descriptor de la comunicación: %d\n",fd);

 // Este sigset es para bloquear las signals durante la copia del buffer
 if (!blockMaskInitialized)
   {
    blockMaskInitialized=1;
    if (CPPFIO_ComSerie_USE_THREAD)
       pthread_mutex_init(&mutex,NULL);
    else
      {
       sigemptyset(&blockMask);
       sigaddset(&blockMask,SIGIO);
      }
   }

 if (!SHSignalOn)
   {
    SHSignalOn=1;
    // install the signal handler before making the device asynchronous
    saio.sa_handler=SignalHandlerIO;
    // Block other terminal-generated signals while handler runs.
    sigemptyset(&block_mask);
    sigaddset(&block_mask,SIGINT);
    sigaddset(&block_mask,SIGQUIT);
    saio.sa_mask=block_mask;
    saio.sa_flags=0;
    if (!CPPFIO_ComSerie_USE_THREAD)
       sigaction(SIGIO,&saio,NULL);
   }

 if (CPPFIO_ComSerie_USE_THREAD)
   {// Cygwin approach: a new thread
    pthread_create(&pollThread,NULL,PollInput,this);
   }
 else
   {// allow the process to receive SIGIO
    fcntl(fd,F_SETOWN,getpid());
    // Make the file descriptor asynchronous (the manual page says only
    //   O_APPEND and O_NONBLOCK, will work with F_SETFL...)
    fcntl(fd,F_SETFL,FASYNC);
   }

 tcgetattr(fd,&oldtio); /* save current port settings */
 bzero(&newtio,sizeof(newtio)); /* clear struct for new port settings */
 /* set new port settings for canonical input processing */
 newtio.c_cflag=br | CS8 | CREAD | /*CSTOPB |*/ CLOCAL;
 newtio.c_iflag=IGNPAR;
 newtio.c_oflag=0;
 newtio.c_lflag=0;//ICANON; No funca para streams de bytes
 newtio.c_cc[VMIN]=0;     /* blocking read until 1 character arrives */
 newtio.c_cc[VSTART]=_POSIX_VDISABLE;
 newtio.c_cc[VSTOP]=_POSIX_VDISABLE;
 newtio.c_cc[VINTR]=_POSIX_VDISABLE;
 newtio.c_cc[VQUIT]=_POSIX_VDISABLE;
 newtio.c_cc[VSUSP]=_POSIX_VDISABLE;
 //newtio.c_cc[VDSUSP]=_POSIX_VDISABLE;

 tcflush(fd,TCIFLUSH);
 tcsetattr(fd,TCSANOW,&newtio);

 buffer=new char[tamBuffer+1];
 initialized=true;
}

static
void Lock()
{
 if (CPPFIO_ComSerie_USE_THREAD)
    pthread_mutex_lock(&mutex);
 else
    // Disable the signal handler
    sigprocmask(SIG_BLOCK,&blockMask,NULL);
}

static
void UnLock()
{
 if (CPPFIO_ComSerie_USE_THREAD)
    pthread_mutex_unlock(&mutex);
 else
    // Enable signal handler
    sigprocmask(SIG_UNBLOCK,&blockMask,NULL);
}

int ComSerie::ReadResp(char *buf)
 throw(ExWrite)
{// No data just return
 if (!bread) return 0;
 Lock();
 // Look for an end of line
 char *end=strchr(buffer,'\n');
 if (!end)
   {
    if (bread>=tamBuffer)
      {// Buffer is full and it doesn't contain EOL, discard
       if (elog)
          elog->Warning("No EOL (%s)\n",buffer);
       bread=0;
      }
    UnLock();
    return 0;
   }
 end++;
 // Copy the data
 int ret=end-buffer;
 memcpy(buf,buffer,ret);
 buf[ret]=0;
 // Remove from the buffer
 bread-=ret;
 if (bread)
    memmove(buffer,buffer+ret,bread+1);
 UnLock();
 // Forward it
 if (fwd)
    fwd->Send(buf,ret);
 return ret;
}

int ComSerie::RawRead(char *buf, int size, bool exact)
 throw(ExWrite)
{// No data just return
 if (!bread) return 0;
 if (exact && bread<size) return 0;
 Lock();
 // Copy the data
 int ret=bread;
 if (size<bread)
    ret=size;
 memcpy(buf,buffer,ret);
 // Remove from the buffer
 bread-=ret;
 if (bread)
    memmove(buffer,buffer+ret,bread+1);
 UnLock();
 // Forward it
 if (fwd)
    fwd->Send(buf,ret);
 return ret;
}

void ComSerie::Send(const unsigned char *s, int l)
 throw(ExWrite)
{
 if (write(fd,s,l)==-1)
    throw ExWrite();
 // Echo ...
 if (echo)
    echo->Send(s,l);
 if (DEBUG_ENVIA)
   {
    printf("Enviando: <");
    int i;
    for (i=0; i<l; i++)
       {
        if (isprint(s[i]))
           printf("%c",s[i]);
        else
           printf("[0x%02X]",s[i]);
       }
    printf(">\n");
   }
}

static
int ProcessSerial(int fd, char *buffer, int &bread, int tamBuffer,
                  ErrorLog *elog)
{
 int res;

 res=read(fd,buffer+bread,tamBuffer-bread);
 if (res!=-1)
   {
    bread+=res;
    buffer[bread]=0;
    if (bread>=tamBuffer && elog)
       elog->Warning("Buffer full (%s)\n",buffer);
   }
 return res;
}

// For the threaded approach
void *ComSerie::PollInput(void* arg)
{
 ComSerie* p=(ComSerie*)arg;
 fd_set fds;

 for (;;)
    {
     FD_ZERO(&fds);
     FD_SET(p->fd,&fds);
     if (select(FD_SETSIZE,&fds,NULL,NULL,NULL)<0)
       {// TODO: something in the parent
        if (p->elog)
           p->elog->Error("Select error in ComSerie::PollInput");
        else
           perror("select");
        break;
       }
     Lock();
     int res=ProcessSerial(p->fd,p->buffer,p->bread,p->tamBuffer,p->elog);
     UnLock();
     if (res==-1)
       {
        if (p->elog)
           p->elog->Error("Read error in ComSerie::PollInput");
        else
           perror("read");
        break;
       }
    }
 return NULL;
}

void cppfio::SignalHandlerIO(int status)
 throw()
{
 int i;
 //SetStatus("Signal IO\n");
 for (i=0; i<MaxComs; i++)
    {
     ComSerie *obj=SHObj[i];
     if (obj)
       {
        int res=ProcessSerial(obj->fd,obj->buffer,obj->bread,obj->tamBuffer,obj->elog);
        if (res==-1)
          {
           if (obj->elog)
              obj->elog->Error("Read error in cppfio::SignalHandlerIO");
           else
              perror("read");
           exit(1);
          }
       }
    }
}

int ComSerie::ConvertBaudRate(int rate)
 throw()
{
 switch (rate)
   {
    case 0:
         return ComSerieB0;
    case 50:
         return ComSerieB50;
    case 75:
         return ComSerieB75;
    case 110:
         return ComSerieB110;
    case 134:
         return ComSerieB134;
    case 150:
         return ComSerieB150;
    case 200:
         return ComSerieB200;
    case 300:
         return ComSerieB300;
    case 600:
         return ComSerieB600;
    case 1200:
         return ComSerieB1200;
    case 1800:
         return ComSerieB1800;
    case 2400:
         return ComSerieB2400;
    case 4800:
         return ComSerieB4800;
    case 9600:
         return ComSerieB9600;
    case 19200:
         return ComSerieB19200;
    case 38400:
         return ComSerieB38400;
    case 57600:
         return ComSerieB57600;
    case 115200:
         return ComSerieB115200;
    case 230400:
         return ComSerieB230400;
    case 460800:
         return ComSerieB460800;
   }
 return -1;
}

