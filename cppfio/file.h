/**[txh]********************************************************************

  Copyright (c) 2004-2007 Salvador E. Tropea <salvador en inti gov ar>
  Copyright (c) 2007 Instituto Nacional de Tecnología Industrial

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

***************************************************************************/

#if !defined(CPPFIO_FILE_H)
#define CPPFIO_FILE_H

namespace cppfio
{

typedef enum { zNo, zGZip, zBZip2 } zCompress;
typedef enum { tfAfterOpen, tfOnClose, tfNever } tRemoveTemp;

class File
{
public:
 File(const char *name, const char *mode, zCompress z=zNo)
                                               throw(ExOpen,ExRead,ExIO);
 File(FILE *aFile)                             throw(ExOpen);
 ~File()                                       throw(ExClose);

 unsigned Read(void *buffer, unsigned len)     throw(ExRead);
 void ReadExactly(void *buffer, unsigned len)  throw(ExRead,ExIntegrity);
 void Write(const void *buffer, unsigned len)  throw(ExWrite);
 void Write(int v)                             throw(ExWrite);
 void Flush()                                  throw(ExWrite);
 void Seek(unsigned pos, int whence)           throw(ExIO);
 long Tell()                                   throw(ExIO);
 void Skip(unsigned len)
  throw(ExIO)
 {
  Seek(len,SEEK_CUR);
 }
 void Seek(unsigned pos)
  throw(ExIO)
 {
  Seek(pos,SEEK_SET);
 }
 void Rewind()
  throw(ExIO)
 {
  Seek(0,SEEK_SET);
 }
 void ToEnd()
  throw(ExIO)
 {
  Seek(0,SEEK_END);
 }
 long Length() throw(ExIO);
 bool Eof() throw();
 int Print(const char *fmt, ...) throw(std::bad_alloc,ExWrite);
 int PrintV(const char *fmt, va_list ap) throw(std::bad_alloc,ExWrite);
 const char *GetName() throw() { return fName; }

 u8  ReadByte()   throw(ExRead,ExIntegrity);

 u16 ReadWord()   throw(ExRead,ExIntegrity);
 u16 ReadWordL()  throw(ExRead,ExIntegrity);
 u16 ReadWordB()  throw(ExRead,ExIntegrity);

 u32 ReadDWord()  throw(ExRead,ExIntegrity);
 u32 ReadDWordL() throw(ExRead,ExIntegrity);
 u32 ReadDWordB() throw(ExRead,ExIntegrity);

 u64 ReadQWord()  throw(ExRead,ExIntegrity);
 u64 ReadQWordL() throw(ExRead,ExIntegrity);
 u64 ReadQWordB() throw(ExRead,ExIntegrity);

 void WriteWord(u16 v)   throw(ExWrite);
 void WriteWordL(u16 v)  throw(ExWrite);
 void WriteWordB(u16 v)  throw(ExWrite);

 void WriteDWord(u32 v)  throw(ExWrite);
 void WriteDWordL(u32 v) throw(ExWrite);
 void WriteDWordB(u32 v) throw(ExWrite);

protected:
 FILE *f;
 void *fbz;
 void *fgz;
 zCompress comp;
 bool bzEof;
 const char *fName;

 File() throw();
 void Open(const char *name, const char *mode, zCompress z=zNo)
  throw(ExOpen,ExRead,ExIO);

 static void XChange(u8 *p, int i1, int i2) throw()
   { u8 aux; aux=p[i1]; p[i1]=p[i2]; p[i2]=aux; }
 static void Invert(u16 &v) throw()
   { u8 *p=(u8 *)&v; XChange(p,0,1); }
 static void Invert(u32 &v) throw()
   { u8 *p=(u8 *)&v; XChange(p,0,3); XChange(p,1,2); }
 static void Invert(u64 &v) throw()
   { u8 *p=(u8 *)&v; XChange(p,0,7); XChange(p,1,6); XChange(p,2,5);
     XChange(p,3,4); }
};

class TmpFile : public File
{
public:
 TmpFile(tRemoveTemp remove=tfAfterOpen) throw(ExOpen,std::bad_alloc);
 ~TmpFile() throw(ExClose);

protected:
 tRemoveTemp mode;
};

}// namespace cppfio

#endif // CPPFIO_FILE_H

