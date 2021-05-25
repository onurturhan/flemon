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

#if !defined(CPPFIO_PREGEX_H)
#define CPPFIO_PREGEX_H

namespace cppfio
{

class PRegEx
{
public:
 PRegEx(const char *str, int flags=0) throw(ExPCRE,std::bad_alloc);
 ~PRegEx() throw();
 bool search(const char *str, unsigned len) throw(ExPCRE);
 int  cantMatches() throw() { return available; }
 const char *getMatch(int subExp, int &len) throw(ExPCRE);
 char *getMatchStr(int subExp, int &len) throw(ExPCRE,std::bad_alloc);

protected:
 pcre *compiled;
 pcre_extra *extra;
 int cSubExps;
 int *subExps;
 const char *text;
 int available;
};


}// namespace cppfio

#endif // CPPFIO_PREGEX_H

