/**[txh]********************************************************************

  Copyright (c) 2007-2009 Salvador E. Tropea <salvador en inti gov ar>
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

  Description:
  Main header file for CPPFIO library.@p
  Define CPPFIO_I to request the headers individually and save compilation
time. In this case you have to indicate CPPFIO_Class for the needed
classes.@p
  If you don't define CPPFIO_I all headers are pulled.

Definitions for things that aren't classes:

CPPFIO_excp: exceptions classes.
  
***************************************************************************/

// For lazy people or code that uses almost all the classes
#ifndef CPPFIO_I
 #define CPPFIO_TextFile
 #define CPPFIO_ErrorLog
 #define CPPFIO_Pipe
 #define CPPFIO_ComSerie
 #define CPPFIO_ComUDP
 #define CPPFIO_ComFile
 #define CPPFIO_GPS
 #define CPPFIO_TCM2
 #define CPPFIO_USB2WB
 #define CPPFIO_Chrono
 #define CPPFIO_SimpleKeyboard
#endif

#include <cppfio_config.h>

typedef unsigned char uchar;

//-- ************************************************************************
//-- Solve dependencies
//-- ************************************************************************

#ifdef CPPFIO_GPS
 #define CPPFIO_Poller
 #define CPPFIO_GeoCoord
 #define CPPFIO_I_gps
#endif

#ifdef CPPFIO_TCM2
 #define CPPFIO_Poller
 #define CPPFIO_Point3D
 #define CPPFIO_Angle
 #define CPPFIO_I_tcm2
#endif

#ifdef CPPFIO_Poller
 #define CPPFIO_I_poller
 #define CPPFIO_Communication
 #define CPPFIO_CmdLine
#endif

#ifdef CPPFIO_Point3D
 #define CPPFIO_GeoCoord
#endif

#ifdef CPPFIO_GeoCoord
 #define CPPFIO_Angle
 #define CPPFIO_Point3D
 #define CPPFIO_I_geo_coord
#endif

#ifdef CPPFIO_Angle
 #define CPPFIO_I_angle
 #define CPPFIO_I_math
#endif

#ifdef CPPFIO_Point3D
 #define CPPFIO_I_point3d
#endif

#ifdef CPPFIO_ComFile
 #define CPPFIO_TextFile
 #define CPPFIO_PTimer
 #define CPPFIO_Communication
 #define CPPFIO_I_cfile
#endif

#ifdef CPPFIO_ComSerie
 #define CPPFIO_I_termios
 #define CPPFIO_I_cserie
 #define CPPFIO_I_pthread
 #define CPPFIO_Communication
 #ifdef __CYGWIN__
  // Cygwin support added by Hans Hübner, uses a separated thread that waits
  // until something arrives using select.
  #define CPPFIO_ComSerie_USE_THREAD 1
 #else
  #define CPPFIO_ComSerie_USE_THREAD 0
 #endif
#endif

#ifdef CPPFIO_ComUDP
 #define CPPFIO_I_cudp
 #define CPPFIO_I_netinet_in
 #define CPPFIO_Communication
#endif

#ifdef CPPFIO_Communication
 #define CPPFIO_I_communic
 #define CPPFIO_ErrorLog
#endif

#ifdef CPPFIO_USB2WB
 #define CPPFIO_LibUSB
 #define CPPFIO_I_usb2wb
#endif

#ifdef CPPFIO_LibUSB
 #define CPPFIO_I_libusb
 #define CPPFIO_I_usb
 #define CPPFIO_excp
#endif

#ifdef CPPFIO_PTimer
 #define CPPFIO_I_sys_time
 #define CPPFIO_I_ptimer
#endif

#ifdef CPPFIO_Chrono
 #define CPPFIO_I_sys_time
 #define CPPFIO_I_chrono
#endif

#ifdef CPPFIO_SimpleKeyboard
 #define CPPFIO_excp
 #define CPPFIO_I_keyboard
#endif

#ifdef CPPFIO_TextFile
 #define CPPFIO_File
 #define CPPFIO_I_text_file
#endif

#ifdef CPPFIO_ErrorLog
 #define CPPFIO_File
 #define CPPFIO_CmdLine
 #define CPPFIO_I_errlog
#endif

#ifdef CPPFIO_CmdLine
 #define CPPFIO_excp
 #define CPPFIO_File
 #define CPPFIO_I_cmdline
 #define CPPFIO_I_getopt
 #define CPPFIO_I_stdio
#endif

#ifdef CPPFIO_File
 #define CPPFIO_excp
 #define CPPFIO_I_file
 #define CPPFIO_I_types
 #define CPPFIO_I_stdio
 #define CPPFIO_I_stdarg
#endif

#ifdef CPPFIO_Pipe
 #define CPPFIO_excp
 #define CPPFIO_I_pipe
 #define CPPFIO_I_unistd
#endif

#ifdef CPPFIO_PRegEx
 #define CPPFIO_excp
 #define CPPFIO_I_pregex
 #define CPPFIO_I_pcre
#endif

#ifdef CPPFIO_Helpers
 #define CPPFIO_I_helpers
 #define CPPFIO_I_exception
 #define CPPFIO_I_new
#endif

#ifdef CPPFIO_StrTok
 #define CPPFIO_I_strtok
 #define CPPFIO_I_exception
 #define CPPFIO_I_new
#endif

#ifdef CPPFIO_excp
 #define CPPFIO_I_excp
 #define CPPFIO_I_exception
 #define CPPFIO_I_new
#endif

//-- ************************************************************************
//-- Includes
//-- ************************************************************************

#ifdef CPPFIO_I_stdio
 #include <stdio.h>
#endif

#ifdef CPPFIO_I_stdlib
 #include <stdlib.h>
#endif

#ifdef CPPFIO_I_unistd
 #include <unistd.h>
#endif

#ifdef CPPFIO_I_string
 #include <string.h>
 #ifndef strdupa
  // Duplicate S, returning an identical alloca'd string.
  // From GNU libc
  #define strdupa(s)                                                           \
    (__extension__                                                             \
      ({                                                                       \
        __const char *__old = (s);                                             \
        size_t __len = strlen (__old) + 1;                                     \
        char *__new = (char *) alloca (__len);                                 \
        (char *) memcpy (__new, __old, __len);                                 \
      }))
 #endif
#endif

#if defined(CPPFIO_assert) || defined(CPPFIO_I_assert)
 #include <assert.h>
#endif

#ifdef CPPFIO_I_limits
 #include <limits.h>
#endif

#ifdef CPPFIO_I_stdarg
 #include <stdarg.h>
#endif

#ifdef CPPFIO_I_math
 #include <math.h>
#endif

#ifdef CPPFIO_I_termios
 #include <termios.h>
#endif

#ifdef CPPFIO_I_netinet_in
 #include <netinet/in.h>
#endif

#ifdef CPPFIO_I_getopt
 #include <getopt.h>
#endif

#ifdef CPPFIO_I_zlib
#if ZLIB_SUPPORT
 #include <zlib.h>
#else
// Fake replacements
typedef void *gzFile;
extern gzFile gzopen(const char *path, const char *mode);
extern int gzclose(gzFile file);
extern int gzread(gzFile file, void *buf, unsigned len);
extern const char *gzerror(gzFile file, int *errnum);
extern int gzwrite(gzFile file, const void *buf, unsigned len);
extern off_t gzseek(gzFile file, off_t offset, int whence);
extern off_t gztell(gzFile file);
extern int gzeof(gzFile file);
extern int gzflush(gzFile file, int flush);
#define Z_SYNC_FLUSH 2
#endif // else ZLIB_SUPPORT
#endif // CPPFIO_I_zlib

#ifdef CPPFIO_I_bzlib
#if BZLIB_SUPPORT
 #include <bzlib.h>
#else
typedef void BZFILE;
extern BZFILE *BZ2_bzopen(const char *path, const char *mode);
extern void BZ2_bzclose(BZFILE *b);
extern int BZ2_bzread(BZFILE *b, void *buf, int len);
extern const char *BZ2_bzerror(BZFILE *b, int *errnum);
extern int BZ2_bzwrite(BZFILE *b, void *buf, int len);
extern int BZ2_bzflush(BZFILE *b);
#define BZ_STREAM_END 4
#endif // else BZLIB_SUPPORT
#endif // CPPFIO_I_bzlib

#ifdef CPPFIO_I_exception
 #include <exception>
#endif

#ifdef CPPFIO_I_new
 #include <new>
#endif

#ifdef CPPFIO_I_types
 #include <types.h>
#endif

#ifdef CPPFIO_I_sys_time
 #include <sys/time.h>
#endif

#ifdef CPPFIO_I_usb
 #include <usb.h>
#endif

#ifdef CPPFIO_I_pthread
 #include <pthread.h>
#endif

#ifdef CPPFIO_I_ptimer
 #include <ptimer.h>
#endif

#ifdef CPPFIO_I_chrono
 #include <chrono.h>
#endif

#ifdef CPPFIO_I_angle
 #include <angle.h>
#endif

#ifdef CPPFIO_I_geo_coord
 #include <geo_coord.h>
#endif

#ifdef CPPFIO_I_point3d
 #include <point3d.h>
#endif

#ifdef CPPFIO_I_pcre
 #include <pcre.h>
#endif

#ifdef CPPFIO_I_helpers
 #include <helpers.h>
#endif

#ifdef CPPFIO_I_excp
 #include <excp.h>
#endif

#ifdef CPPFIO_I_libusb
 #include <libusb.h>
#endif

#ifdef CPPFIO_I_usb2wb
 #include <cppfio_usb2wb.h>
#endif

#ifdef CPPFIO_I_pregex
 #include <pregex.h>
#endif

#ifdef CPPFIO_I_file
 #include <file.h>
#endif

#ifdef CPPFIO_I_text_file
 #include <text_file.h>
#endif

#ifdef CPPFIO_I_cmdline
 #include <cmdline.h>
#endif

#ifdef CPPFIO_I_errlog
 #include <errlog.h>
#endif

#ifdef CPPFIO_I_pipe
 #include <pipe.h>
#endif

#ifdef CPPFIO_I_strtok
 #include <strtok.h>
#endif

#ifdef CPPFIO_I_communic
 #include <communic.h>
#endif

#ifdef CPPFIO_I_cfile
 #include <cfile.h>
#endif

#ifdef CPPFIO_I_cserie
 #include <cserie.h>
#endif

#ifdef CPPFIO_I_cudp
 #include <cudp.h>
#endif

#ifdef CPPFIO_I_poller
 #include <poller.h>
#endif

#ifdef CPPFIO_I_gps
 #include <gps.h>
#endif

#ifdef CPPFIO_I_tcm2
 #include <tcm2.h>
#endif

#ifdef CPPFIO_I_keyboard
 #include <keyboard.h>
#endif
