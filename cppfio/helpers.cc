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

  Module: Helpers
  Description:
  Helper functions.
  
***************************************************************************/
/*****************************************************************************

 Target:      Any
 Language:    C++
 Compiler:    GNU g++ 4.1.1 (Debian GNU/Linux)
 Text editor: SETEdit 0.5.5

*****************************************************************************/

#define CPPFIO_I
#define CPPFIO_Helpers
#define CPPFIO_StrTok
#define CPPFIO_I_string
#define CPPFIO_I_stdlib
#define CPPFIO_I_stdio
#include <cppfio.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>

using namespace cppfio;

char *cppfio::newStr(const char *s, int len)
  throw(std::bad_alloc)
{
 if (!s)
    return NULL;
 if (len<0)
    len=strlen(s);
 char *r=new char[len+1];
 memcpy(r,s,len);
 r[len]=0;

 return r;
}

/**[txh]********************************************************************

  Description:
  Concatenates two or more strings. The variable length argument list must
end with NULL. s1 and s2 can't be NULL.
  
  Return:
  A newly allocated string.
  
***************************************************************************/

char *cppfio::newStr(const char *s1, const char *s2, ...)
  throw(std::bad_alloc)
{
 const char *s;
 unsigned l1=strlen(s1), l2=strlen(s2);
 unsigned l=l1+l2;
 va_list arg;

 va_start(arg,s2);
 while ((s=va_arg(arg,const char *))!=NULL)
    l+=strlen(s);
 va_end(arg);

 char *res=new char[l+1];
 l=l1+l2;
 memcpy(res,s1,l1);
 memcpy(res+l1,s2,l2);
 va_start(arg,s2);
 while ((s=va_arg(arg,const char *))!=NULL)
   {
    unsigned len=strlen(s);
    memcpy(res+l,s,len);
    l+=len;
   }
 va_end(arg);
 res[l]=0;

 return res;
}

/**[txh]********************************************************************

  Description:
  Concatenates two or more strings. The variable length argument list must
end with NULL. The first argument is the original string and can be NULL.
It deallocates the first string.
  
  Return:
  A newly allocated string, s1 is released.
  
***************************************************************************/

char *cppfio::strCat(char *s1, const char *s2, ...)
  throw(std::bad_alloc)
{
 const char *s;
 unsigned l1=s1 ? strlen(s1) : 0, l2=strlen(s2);
 unsigned l=l1+l2;
 va_list arg;

 va_start(arg,s2);
 while ((s=va_arg(arg,const char *))!=NULL)
    l+=strlen(s);
 va_end(arg);

 char *res=new char[l+1];
 l=l1+l2;
 if (s1)
   {
    memcpy(res,s1,l1);
    delete[] s1;
   }
 memcpy(res+l1,s2,l2);
 va_start(arg,s2);
 while ((s=va_arg(arg,const char *))!=NULL)
   {
    unsigned len=strlen(s);
    memcpy(res+l,s,len);
    l+=len;
   }
 va_end(arg);
 res[l]=0;

 return res;
}

char *cppfio::skipSpaces(char *s)
  throw()
{
 if (!s) return s;
 for (; *s && isspace((unsigned char)*s); s++);
 return s;
}

void cppfio::removeEndSpaces(char *s)
 throw()
{
 if (!s) return;
 char *e=s+strlen(s)-1;
 for (; isspace((unsigned char)*e); e--) *e=0;
}

char *cppfio::removeSpaces(char *s)
 throw()
{
 if (!s) return s;
 removeEndSpaces(s);
 return skipSpaces(s);
}

bool cppfio::FileExists(const char *name)
 throw()
{
 struct stat st;

 return stat(name,&st)==0;
}

bool cppfio::FindFile(const char *name, char *buf, unsigned size,
                      const char *dirs[])
 throw()
{
 if (*name=='/')
   {// Absolute filename
    strncpy(buf,name,size);
    return FileExists(buf);
   }
 for (unsigned i=0; dirs[i]; i++)
    {
     const char *path, *d;
     if (strcmp(dirs[i],"$PATH")==0)
        path=getenv("PATH");
     else
        path=dirs[i];
     if (path)
       {
        StrTok s(strdupa(path),":");
        while ((d=s.Get())!=NULL)
          {
           snprintf(buf,size,"%s/%s",d,name);
           if (FileExists(buf))
              return true;
          }
       }
    }
 return false;
}

