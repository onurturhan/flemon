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

#if !defined(CPPFIO_TEXT_FILE_H)
#define CPPFIO_TEXT_FILE_H

namespace cppfio
{

class ErrorLog;

class TextFile : public File
{
public:
 TextFile(const char *name)
  throw(ExOpen,ExRead,ExIO) :
  File(name,"rt"),
  linePtr(NULL),
  szBLine(0)
 {}
 TextFile(FILE *f)
  throw(ExOpen) :
  File(f),
  linePtr(NULL),
  szBLine(0)
 {}
 ~TextFile() throw();
 char *ReadLine(int &len) throw(std::bad_alloc,ExRead);
 char *ReadLineNoEOL(int &len) throw(std::bad_alloc,ExRead);
 void  Process(int (*func)(TextFile &f, char *s, int len, void *data),
               ErrorLog &error, void *data=NULL);
 void  RemoveEOL(int &l)
  throw()
 {
  if (linePtr[l-1]=='\n')
    {
     l--;
     linePtr[l]=0;
    }
 }

protected:
 char *linePtr;
 size_t szBLine;

 char *ReadLineLow(int &len, bool noEOL) throw(std::bad_alloc,ExRead);
};

}// namespace cppfio

#endif // CPPFIO_TEXT_FILE_H

