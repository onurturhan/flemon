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

#if !defined(CPPFIO_TYPE_INCLUDED)
#define CPPFIO_TYPE_INCLUDED

#ifdef __cplusplus
namespace cppfio
{
#endif // __cplusplus

// gcc types
typedef signed   char      i8;
typedef unsigned char      u8;
typedef signed   short     i16;
typedef unsigned short     u16;
typedef signed   int       i32;
typedef unsigned int       u32;
typedef signed   long long i64;
typedef unsigned long long u64;

#ifdef __cplusplus

#ifdef PLATFORM_BIG_ENDIAN
// i.e. SPARC
const int bigEndian=1, littleEndian=0;
#else
// i.e. Intel
const int bigEndian=0, littleEndian=1;
#endif

// Function templates to protect destructors
template <class T>
inline void DeleteIf(T *p)
{
 if (p)
    delete p;
}

template <class T>
inline void Delete0If(T *&p)
{
 if (p)
   {
    delete p;
    p=NULL;
   }
}


}// namespace cppfio
#endif // __cplusplus
#endif
