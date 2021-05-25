/**[txh]********************************************************************

  Copyright (c) 2003-2009 Salvador E. Tropea <salvador en inti gov ar>
  Copyright (c) 2009 Instituto Nacional de Tecnología Industrial

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
 
  Description:
  Program used to compile and link CPPFIO programs.
  Derived from Turbo Vision config tool.

***************************************************************************/
/*****************************************************************************

 Target:      Any
 Language:    C++
 Compiler:    GNU g++ 4.1.1 (Debian GNU/Linux)
 Text editor: SETEdit 0.5.5

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <cppfio_config.h>

#define VERSION "1.0.0"

static
void DLibs(void)
{
 printf("-lcppfio ");
 if (ZLIB_SUPPORT)
    printf("-lz ");
 if (BZLIB_SUPPORT)
    printf("-lbz2 ");
 if (PCRE_SUPPORT)
    printf("-lpcre ");
 if (LIBUSB_SUPPORT)
    printf("-lusb ");
}

static
void Usage(void)
{
 fputs("CPPFIO configuration tool v" VERSION "\n",stderr);
 fputs("Copyright (c) 2003-2009 by Salvador E. Tropea.\n",stderr);
 fputs("Copyright (c) 2009 by Instituto Nacional de Tecnología Industrial.\n",stderr);
 fputs("This is free software covered by the GPL license.\n",stderr);
 fprintf(stderr,"Usage: cppfio_config OPTION\n");
 fprintf(stderr,"Available options: [Only one can be specified]\n");
 fprintf(stderr,"\t--dlibs   [for dynamic link]\n");
 fprintf(stderr,"\t--version\n");
}

static
void UnknowOp(const char *s)
{
 fprintf(stderr,"Unknown option: %s\n\n",s);
 Usage();
}

int main(int argc, char *argv[])
{
 char *op;
 if (argc!=2)
   {
    Usage();
    return 1;
   }
 op=argv[1];
 if (op[0]!='-' || op[1]!='-')
   {
    UnknowOp(op);
    return 2;
   }
 op+=2;
 if (strcmp(op,"dlibs")==0)
    DLibs();
 else if (strcmp(op,"version")==0)
    fputs("CPPFIO configuration tool v" VERSION,stdout);
 else
   {
    UnknowOp(op);
    return 2;
   }
 printf("\n");
 return 0;
}
