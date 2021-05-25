/**[txh]********************************************************************

  Copyright (c) 2004-2009 Salvador E. Tropea <salvador en inti gov ar>
  Copyright (c) 2007-2009 Instituto Nacional de Tecnología Industrial

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

  Module: File I/O
  Description:
  That's a very thin layer over the stdio file I/O funtionality. The
objetive is to make it "exception aware".
  
***************************************************************************/
/*****************************************************************************

 Target:      Any
 Language:    C++
 Compiler:    GNU g++ 4.1.1 (Debian GNU/Linux)
 Text editor: SETEdit 0.5.5

*****************************************************************************/

#define CPPFIO_I
#define CPPFIO_I_string
#define CPPFIO_I_stdlib
#define CPPFIO_I_unistd
#define CPPFIO_I_zlib
#define CPPFIO_I_bzlib
#define CPPFIO_File
#define CPPFIO_Helpers
#include <cppfio.h>
#include <sys/stat.h>
#include <ctype.h>
#include <limits.h>

using namespace cppfio;

static const char *stdinName="stdin";
static const char *stdoutName="stdout";

File::File(const char *name, const char *mode, zCompress z)
 throw(ExOpen,ExRead,ExIO)
{
 if (name)
    Open(name,mode,z);
 else
   {
    comp=z;
    if (strchr(mode,'r'))
      {
       fName=stdinName;
       f=stdin;
      }
    else
      {
       fName=stdoutName;
       f=stdout;
      }
   }
}

File::File(FILE *aFile)
 throw(ExOpen)
{
 f=aFile;
 if (!f) throw ExOpen();
 comp=zNo;
 fName="unknown";
}

File::File()
 throw()
{
 f=NULL;
 comp=zNo;
 fName="unknown";
}

void File::Open(const char *name, const char *mode, zCompress z)
 throw(ExOpen,ExRead,ExIO)
{
 struct stat st;
 bool exists=stat(name,&st)==0;

 if (strchr(mode,'w') || (!exists && strchr(mode,'a')))
   {// Create
    switch (z)
      {
       case zNo:
            f=fopen(name,mode);
            if (!f) throw ExOpen();
            break;
       case zGZip:
            if (ZLIB_SUPPORT)
              {
               fgz=gzopen(name,mode);
               if (!fgz) throw ExOpen();
              }
            else
               throw ExOpen();
            break;
       case zBZip2:
            if (BZLIB_SUPPORT)
              {
               fbz=BZ2_bzopen(name,mode);
               if (!fbz) throw ExOpen();
              }
            else
               throw ExOpen();
            break;
      }
    comp=z;
   }
 else
   {// Existing file
    f=fopen(name,mode);
    if (!f) throw ExOpen();
   
    // Detect compression
    comp=zNo;
    unsigned char b[4]={0,0,0,0};
   
    // Try with gzip
    unsigned r=Read(b,4);
    if (r)
       Seek(-r,SEEK_CUR);
    if (b[0]==0x1F && b[1]==0x8B && ZLIB_SUPPORT)
      {
       fclose(f);
       f=NULL;
       fgz=gzopen(name,mode);
       if (!fgz) throw ExOpen();
       comp=zGZip;
      }
    else
      {// Try Bzip2
       if (strncmp((char *)b,"BZh",3)==0 && isdigit(b[3]) && BZLIB_SUPPORT)
         {
          fclose(f);
          f=NULL;
          fbz=BZ2_bzopen(name,mode);
          if (!fbz) throw ExOpen();
          comp=zBZip2;
         }
      }
   }
 fName=name;
}

File::~File()
 throw(ExClose)
{
 switch (comp)
   {
    case zNo:
         if (f)
           {// I'm not sure about it: I think the errors must be cleared to avoid
            // confusing a read/write error with the close error.
            clearerr(f);
            if (fName!=stdinName && fName!=stdoutName)
               if (fclose(f)) throw ExClose();
            f=NULL;
           }
         break;
    case zGZip:
         if (fgz && ZLIB_SUPPORT)
           {
            gzclose(fgz);
            fgz=NULL;
           }
         break;
    case zBZip2:
         if (fbz && BZLIB_SUPPORT)
           {
            BZ2_bzclose(fbz);
            fbz=NULL;
           }
         break;
   }
}

unsigned File::Read(void *buffer, unsigned len)
 throw(ExRead)
{
 unsigned ret=0;
 int errnum;

 switch (comp)
   {
    case zNo:
         ret=fread(buffer,1,len,f);
         if (ferror(f)) throw ExRead();
         break;
    case zGZip:
         if (ZLIB_SUPPORT)
           {
            ret=gzread(fgz,buffer,len);
            gzerror(fgz,&errnum);
            if (errnum<0) throw ExRead();
           }
         break;
    case zBZip2:
         if (BZLIB_SUPPORT)
           {
            ret=BZ2_bzread(fbz,buffer,len);
            BZ2_bzerror(fbz,&errnum);
            if (errnum<0) throw ExRead();
           }
         break;
    default:
         ret=0;
   }
 return ret;
}

void File::Write(const void *buffer, unsigned len)
 throw(ExWrite)
{
 int errnum;

 switch (comp)
   {
    case zNo:
         fwrite(buffer,1,len,f);
         if (ferror(f)) throw ExWrite();
         break;
    case zGZip:
         if (ZLIB_SUPPORT)
           {
            gzwrite(fgz,buffer,len);
            gzerror(fgz,&errnum);
            if (errnum<0) throw ExRead();
           }
         break;
    case zBZip2:
         if (BZLIB_SUPPORT)
           {
            BZ2_bzwrite(fbz,(void *)buffer,len);
            BZ2_bzerror(fbz,&errnum);
            if (errnum<0) throw ExRead();
           }
         break;
   }
}

void File::Write(int v)
 throw(ExWrite)
{
 char b=(char)v;
 Write(&b,1);
 if (ferror(f)) throw ExWrite();
}

void File::ReadExactly(void *buffer, unsigned len)
 throw(ExRead,ExIntegrity)
{
 if (Read(buffer,len)!=len) throw ExIntegrity();
}

u8 File::ReadByte()
 throw(ExRead,ExIntegrity)
{
 u8 val;
 ReadExactly(&val,1);
 return val;
}

/**[txh]********************************************************************

  Description:
  Read a 16 bits value in native format.
  
  Return: The value.
  
***************************************************************************/

u16 File::ReadWord()
 throw(ExRead,ExIntegrity)
{
 u16 val;
 ReadExactly(&val,2);
 return val;
}

/**[txh]********************************************************************

  Description:
  Read a little endian 16 bits value.
  
  Return: The value in native format.
  
***************************************************************************/

u16 File::ReadWordL()
 throw(ExRead,ExIntegrity)
{
 u16 val;
 ReadExactly(&val,2);
 if (bigEndian)
    Invert(val);
 return val;
}

/**[txh]********************************************************************

  Description:
  Read a big endian 16 bits value.
  
  Return: The value in native format.
  
***************************************************************************/

u16 File::ReadWordB()
 throw(ExRead,ExIntegrity)
{
 u16 val;
 ReadExactly(&val,2);
 if (littleEndian)
    Invert(val);
 return val;
}

/**[txh]********************************************************************

  Description:
  Read a 32 bits value in native format.
  
  Return: The value.
  
***************************************************************************/

u32 File::ReadDWord()
 throw(ExRead,ExIntegrity)
{
 u32 val;
 ReadExactly(&val,4);
 return val;
}

/**[txh]********************************************************************

  Description:
  Read a little endian 32 bits value.
  
  Return: The value in native format.
  
***************************************************************************/

u32 File::ReadDWordL()
 throw(ExRead,ExIntegrity)
{
 u32 val;
 ReadExactly(&val,4);
 if (bigEndian)
    Invert(val);
 return val;
}

/**[txh]********************************************************************

  Description:
  Read a big endian 32 bits value.
  
  Return: The value in native format.
  
***************************************************************************/

u32 File::ReadDWordB()
 throw(ExRead,ExIntegrity)
{
 u32 val;
 ReadExactly(&val,4);
 if (littleEndian)
    Invert(val);
 return val;
}

u64 File::ReadQWord()
 throw(ExRead,ExIntegrity)
{
 u64 val;
 ReadExactly(&val,8);
 return val;
}

u64 File::ReadQWordL()
 throw(ExRead,ExIntegrity)
{
 u64 val;
 ReadExactly(&val,8);
 if (bigEndian)
    Invert(val);
 return val;
}

u64 File::ReadQWordB()
 throw(ExRead,ExIntegrity)
{
 u64 val;
 ReadExactly(&val,8);
 if (littleEndian)
    Invert(val);
 return val;
}

// TODO: throw algo más descriptivo?
void File::Seek(unsigned len, int whence)
 throw(ExIO)
{
 switch (comp)
   {
    case zNo:
         if (fseek(f,len,whence))
            throw ExIO();
         break;
    case zGZip:
         if (ZLIB_SUPPORT)
            if (gzseek(fgz,len,whence)==-1)
               throw ExIO();
         break;
    case zBZip2:
         throw ExIO();
   }
}

// TODO: throw algo más descriptivo?
long File::Tell()
 throw(ExIO)
{
 long ret=0;

 switch (comp)
   {
    case zNo:
         ret=ftell(f);
         break;
    case zGZip:
         if (ZLIB_SUPPORT)
            ret=gztell(fgz);
         break;
    case zBZip2:
         throw ExIO();
    default:
         throw ExIO();
   }
 if (ret==-1)
    throw ExIO();
 return ret;
}

void File::WriteWord(u16 v)
 throw(ExWrite)
{
 Write(&v,2);
}

void File::WriteWordL(u16 v)
 throw(ExWrite)
{
 if (bigEndian)
    Invert(v);
 Write(&v,2);
}

void File::WriteWordB(u16 v)
 throw(ExWrite)
{
 if (littleEndian)
    Invert(v);
 Write(&v,2);
}

void File::WriteDWord(u32 v)
 throw(ExWrite)
{
 Write(&v,4);
}

void File::WriteDWordL(u32 v)
 throw(ExWrite)
{
 if (bigEndian)
    Invert(v);
 Write(&v,4);
}

void File::WriteDWordB(u32 v)
 throw(ExWrite)
{
 if (littleEndian)
    Invert(v);
 Write(&v,4);
}

int File::Print(const char *fmt, ...)
 throw(std::bad_alloc,ExWrite)
{
 va_list argptr;

 va_start(argptr,fmt);
 int ret=PrintV(fmt,argptr);
 va_end(argptr);

 return ret;
}

int File::PrintV(const char *fmt, va_list ap)
 throw(std::bad_alloc,ExWrite)
{
 int ret;
 char *str;

 switch (comp)
   {
    case zNo:
         ret=vfprintf(f,fmt,ap);
         if (ferror(f))
            throw ExWrite();
         break;
    case zGZip:
    case zBZip2:
         ret=vasprintf(&str,fmt,ap);
         if (ret<0)
            throw std::bad_alloc();
         try
         {
          Write(str,ret);
         }
         catch(ExWrite)
         {
          free(str);
          throw ExWrite();
         }
         break;
    default:
         ret=0;
   }
 return ret;
}

bool File::Eof()
  throw()
{
 switch (comp)
   {
    case zNo:
         return feof(f);
    case zGZip:
         if (ZLIB_SUPPORT)
            return gzeof(fgz);
    case zBZip2:
         if (BZLIB_SUPPORT)
           {
            int errnum;
            BZ2_bzerror(fbz,&errnum);
            return (errnum==BZ_STREAM_END);
           }
   }
 return false;
}

void File::Flush()
  throw(ExWrite)
{
 switch (comp)
   {
    case zNo:
         if (fflush(f)==EOF)
            throw ExWrite();
         break;
    case zGZip:
         if (ZLIB_SUPPORT)
            if (gzflush(fgz,Z_SYNC_FLUSH)<0)
               throw ExWrite();
         break;
    case zBZip2:
         if (BZLIB_SUPPORT)
            if (BZ2_bzflush(f)==EOF)
               throw ExWrite();
         break;
   }
}

long File::Length()
  throw(ExIO)
{
 long pos=Tell();
 ToEnd();
 long len=Tell();
 Seek(pos);
 return len;
}

TmpFile::TmpFile(tRemoveTemp remove)
 throw(ExOpen,std::bad_alloc) :
 File()
{
 mode=remove;
 char buffer[PATH_MAX];
 char *tmp=getenv("TEMP");

 snprintf(buffer,PATH_MAX,"%s/cppfioXXXXXX",tmp ? tmp : "/tmp");
 fName=newStr(buffer);
 int desc=mkstemp((char *)fName);
 if (desc==-1)
    throw ExOpen();
 if (mode==tfAfterOpen)
    unlink(fName);
 f=fdopen(desc,"w+");
 if (!f)
    throw ExOpen();
}

TmpFile::~TmpFile()
 throw(ExClose)
{
 if (fName)
   {
    if (mode==tfOnClose)
       unlink(fName);
    delete[] fName;
    fName=NULL;
   }
}



