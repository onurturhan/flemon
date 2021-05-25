/**[txh]********************************************************************

  Copyright (c) 2008 Salvador E. Tropea <salvador en inti gov ar>
  Copyright (c) 2008 Instituto Nacional de Tecnología Industrial

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

  Module: strtok wrapper
  Description:
  Provides a wrapper for libc's strtok function. It takes care of
de/allocation of the string.
  
***************************************************************************/
/*****************************************************************************

 Target:      Any
 Language:    C++
 Compiler:    GNU g++ 4.1.1 (Debian GNU/Linux)
 Text editor: SETEdit 0.5.5

*****************************************************************************/

#define CPPFIO_I
#define CPPFIO_StrTok
#define CPPFIO_Helpers
#define CPPFIO_I_string
#include <cppfio.h>

using namespace cppfio;

StrTok::StrTok(const char *aStr, const char *aDelimiters)
 throw(std::bad_alloc)
{
 str=newStr(aStr);
 delimiters=aDelimiters;
 release=true;
 savePtr=NULL;
}

StrTok::StrTok(char *aStr, const char *aDelimiters)
 throw()
{
 str=aStr;
 delimiters=aDelimiters;
 release=false;
 savePtr=NULL;
}

StrTok::~StrTok()
 throw()
{
 if (release)
    delete[] str;
}

char *StrTok::Get()
 throw()
{
 if (!savePtr)
    return strtok_r(str,delimiters,&savePtr);
 return strtok_r(NULL,delimiters,&savePtr);
}

char *StrTok::Get(const char *aDelimiters)
 throw()
{
 delimiters=aDelimiters;
 return Get();
}


