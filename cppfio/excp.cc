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

  Module: Exception
  Description:
  Common exceptions.

***************************************************************************/
/*****************************************************************************

 Target:      Any
 Language:    C++
 Compiler:    GNU g++ 4.1.1 (Debian GNU/Linux)
 Text editor: SETEdit 0.5.5

 Portability notes:
 Uses GNU's getopt_long.
    
*****************************************************************************/

#define CPPFIO_I
#define CPPFIO_I_stdio
#define CPPFIO_I_string
#define CPPFIO_excp
#include <cppfio.h>

#include <errno.h>

using namespace cppfio;

const unsigned maxBuf=1024;
static char bufMsg[maxBuf];

const char *ExProcess::what() const throw()
{
 return "processing";
};

const char *ExIntegrity::what() const throw()
{
 return "corrupted data";
};

const char *ExIO::what() const throw()
{
 snprintf(bufMsg,maxBuf,"I/O fail: %s",strerror(errno));
 return bufMsg;
}

const char *ExOpen::what() const throw()
{
 if (name)
   {
    snprintf(bufMsg,maxBuf,"opening <%s> file",name);
    return bufMsg;
   }
 return "opening file";
}

const char *ExClose::what() const throw()
{
 return "closing file";
}

const char *ExRead::what() const throw()
{
 return "reading from file";
}

const char *ExWrite::what() const throw()
{
 snprintf(bufMsg,maxBuf,"writing to file: %s",strerror(errno));
 return bufMsg;
}

const char *ExTimedOut::what() const throw()
{
 return "time-out";
}

const char *ExNoDevice::what() const throw()
{
 return "no such a device";
}

const char *ExNULLPointer::what() const throw()
{
 return "NULL pointer assignment/usage";
}

const char *ExBufferOverflow::what() const throw()
{
 return "buffer overflow";
}

const char *ExPCRE::what() const throw()
{
 return error;
}

