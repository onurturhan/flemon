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

  Module: Error Logger
  Description:
  Provides a helper to log messages and/or to abort with an error message.
  
***************************************************************************/
/*****************************************************************************

 Target:      Any
 Language:    C++
 Compiler:    GNU g++ 4.1.1 (Debian GNU/Linux)
 Text editor: SETEdit 0.5.5

*****************************************************************************/

#define CPPFIO_I
#define CPPFIO_ErrorLog
#define CPPFIO_I_stdlib
#define CPPFIO_I_string
#include <cppfio.h>
#include <time.h>

using namespace cppfio;

ErrorLog::ErrorLog(const char *name)
  throw(ExOpen) :
  File(name,"wt"),
  line(0),
  fileName(NULL)
{
}

ErrorLog::ErrorLog(FILE *f)
  throw(ExOpen) :
  File(f),
  line(0),
  fileName(NULL)
{
}

void ErrorLog::ErrorImpl(const char *fmt, va_list argptr)
  throw(ExWrite)
{
 if (fileName)
   {
    Write(fileName,strlen(fileName));
    if (line>=0)
       Print(":%d:",line);
    else
       Write(':');
   }
 Write("error: ",7);
 PrintV(fmt,argptr);
 Write(".\n",2);
}

void ErrorLog::Error(const char *fmt, ...)
  throw(ExWrite)
{
 va_list argptr;

 va_start(argptr,fmt);
 ErrorImpl(fmt,argptr);
 va_end(argptr);
}

void ErrorLog::Abort(const char *fmt, ...)
  throw(ExWrite)
{
 va_list argptr;

 va_start(argptr,fmt);
 ErrorImpl(fmt,argptr);
 va_end(argptr);
 exit(2);
}

void ErrorLog::Warning(const char *fmt, ...)
  throw(ExWrite)
{
 va_list argptr;

 if (fileName)
   {
    Write(fileName,strlen(fileName));
    if (line>=0)
       Print(":%d:",line);
    else
       Write(':');
   }
 Write("warning: ",9);
 va_start(argptr,fmt);
 PrintV(fmt,argptr);
 va_end(argptr);
 Write(".\n",2);
}

void ErrorLog::Log(const char *fmt, ...)
  throw(ExWrite)
{
 va_list argptr;

 // Write the time
 time_t t;
 time(&t);
 struct tm *st=localtime(&t);
 char b[64];
 int l=strftime(b,64,"%F %T: ",st);
 Write(b,l);

 va_start(argptr,fmt);
 PrintV(fmt,argptr);
 va_end(argptr);
 Write(".\n",2);
}

void ErrorLog::LogOpts(clpOption *opts)
 throw(ExWrite)
{
 for (; opts->name; opts++)
    {
     switch (opts->kind)
       {
        case argBool:
             if (*((bool *)opts->pointer))
                Log("--%s",opts->name);
             break;
        case argFunction:
             break;
        case argInteger:
             Log("--%s=%ld",opts->name,*((long *)opts->pointer));
             break;
        case argString:
             Log("--%s=%s",opts->name,*((char **)opts->pointer));
             break;
        case argDouble:
             Log("--%s=%g",opts->name,*((double *)opts->pointer));
             break;
       }
    }
}


