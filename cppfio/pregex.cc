/**[txh]********************************************************************

  Copyright (c) 2007 Salvador E. Tropea <salvador en inti gov ar>
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

  Module: Perl Compatible Regular Expressions
  Description:
  Provides a wrapper for the PCRE library.
  
***************************************************************************/
/*****************************************************************************

 Target:      Any
 Language:    C++
 Compiler:    GNU g++ 4.1.1 (Debian GNU/Linux)
 Text editor: SETEdit 0.5.5

*****************************************************************************/

#define CPPFIO_I
#define CPPFIO_PRegEx
#define CPPFIO_I_string
#include <cppfio.h>

using namespace cppfio;

PRegEx::PRegEx(const char *str, int flags)
  throw(ExPCRE,std::bad_alloc)
{
 compiled=NULL;
 extra=NULL;
 subExps=NULL;
 available=0;

 const char *error;
 int   errorOffset;

 compiled=pcre_compile(str,flags,&error,&errorOffset,0);
 if (!compiled) throw ExPCRE(error);
 extra=pcre_study(compiled,0,&error);
 if (error) throw ExPCRE(error);
 cSubExps=(pcre_info(compiled,0,0)+1)*3;
 subExps=new int[cSubExps];
}

PRegEx::~PRegEx()
  throw()
{
 free(compiled);
 free(extra);
 delete[] subExps;
}

bool PRegEx::search(const char *str, unsigned len)
  throw(ExPCRE)
{
 text=str;
 int hits=pcre_exec(compiled,extra,str,len,0,0,subExps,cSubExps);
 if (hits<0 && hits!=PCRE_ERROR_NOMATCH)
    throw ExPCRE("PCRE exec error");
 if (hits!=PCRE_ERROR_NOMATCH)
   {
    available=hits;
    return true;
   }
 return false;
}

const char *PRegEx::getMatch(int subExp, int &len)
  throw(ExPCRE)
{
 if (subExp>=available || subExps[subExp*2]<0)
    throw ExPCRE("Invalid subexpression");
 subExp*=2;
 len=subExps[subExp+1]-subExps[subExp];
 return text+subExps[subExp];
}

char *PRegEx::getMatchStr(int subExp, int &len)
  throw(ExPCRE,std::bad_alloc)
{
 const char *s=getMatch(subExp,len);
 char *r=new char[len+1];
 memcpy(r,s,len);
 r[len]=0;
 return r;
}
