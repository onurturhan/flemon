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

  Module: Text File
  Description:
  A class derived from File adding functionality specific for text files.
  Note: The ReadLine member is based on FSF's code, created by Jan
  Brittenson <bson@gnu.ai.mit.edu>.
  
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
#define CPPFIO_TextFile
#define CPPFIO_ErrorLog
#include <cppfio.h>
#include <new>
#include <assert.h>

using namespace cppfio;

TextFile::~TextFile()
 throw()
{
 if (linePtr)
    delete[] linePtr;
}

// Always add at least this many bytes when extending the buffer.
const unsigned minChunk=64;

char *TextFile::ReadLineLow(int &len, bool noEOL)
 throw(std::bad_alloc,ExRead)
{
 if (!linePtr)
   {
    szBLine=minChunk;
    linePtr=new char[szBLine];
   }

 int nCharsAvail=szBLine; // Allocated but unused chars in linePtr.
 char *readPos=linePtr;   // Where we're reading into linePtr.
 char c;

 do
   {
    if (Read(&c,1)==0)
      {// Return partial line, if any.
       if (readPos==linePtr)
          return NULL;
       else
          break;
      }

    assert((linePtr+szBLine)==(readPos+nCharsAvail));
    /* We always want at least one char left in the buffer, since we
       always (unless we get an error while reading the first char)
       NUL-terminate the line buffer.  */
    if (nCharsAvail<2)
      {
       size_t oldSize=szBLine;
       if (szBLine>minChunk)
          szBLine*=2;
       else
          szBLine+=minChunk;

       nCharsAvail=szBLine-(readPos-linePtr);
       // Safe realloc
       char *aux=new char[szBLine];
       memcpy(aux,linePtr,oldSize);
       delete[] linePtr;
       linePtr=aux;

       readPos=linePtr+(szBLine-nCharsAvail);
       assert((linePtr+szBLine)==(readPos+nCharsAvail));
      }

    if (noEOL && c=='\n')
       break;
    *readPos++=c;
    nCharsAvail--;
   }
 while (c!='\n');

 // Done - NUL terminate.
 *readPos='\0';
 len=readPos-linePtr;

 return linePtr;
}

char *TextFile::ReadLine(int &len)
 throw(std::bad_alloc,ExRead)
{
 return ReadLineLow(len,false);
}

char *TextFile::ReadLineNoEOL(int &len)
 throw(std::bad_alloc,ExRead)
{
 return ReadLineLow(len,true);
}

/**[txh]********************************************************************

  Description:
  Calls the provided function for each line in a file. It also sets the
program name of the provided ErrorLog object and takes care of the line
numbers. If the called function returns -1 the process is aborted.
  
***************************************************************************/

void TextFile::Process(int (*func)(TextFile &f, char *s, int len, void *data),
                       ErrorLog &error, void *data)
{
 const char *oldFile=error.GetFile();
 int oldLine=error.line;
 error.SetFile(fName);
 error.line=0;

 int len;
 while (!Eof())
   {
    char *s=ReadLineNoEOL(len);
    int oldLine=++error.line;
    if (!s || func(*this,s,len,data)==-1)
       break;
    error.SetFile(fName);
    error.line=oldLine;
   }
 error.SetFile(oldFile);
 error.line=oldLine;
}

