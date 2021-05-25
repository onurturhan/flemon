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

***************************************************************************/

#if !defined(CPPFIO_STRTOK_H)
#define CPPFIO_STRTOK_H

namespace cppfio
{

class StrTok
{
public:
 StrTok(const char *str, const char *aDelimiters) throw(std::bad_alloc);
 StrTok(char *str, const char *aDelimiters) throw();
 ~StrTok() throw();
 void SetDelimiters(const char *aDelimiters) throw() { delimiters=aDelimiters; }
 char *Get() throw();
 char *Get(const char *aDelimiters) throw();

protected:
 char *str, *savePtr;
 const char *delimiters;
 bool release;
};


}// namespace cppfio

#endif // CPPFIO_STRTOK_H
