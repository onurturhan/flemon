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

  Description:
  Helper functions.

***************************************************************************/

#if !defined(CPPFIO_HELPERS_H)
#define CPPFIO_HELPERS_H

namespace cppfio
{

// Allocate a new copy of the string (using new)
char *newStr(const char *s, int len=-1) throw(std::bad_alloc);
char *newStr(const char *s1, const char *s2, ...) throw(std::bad_alloc);
char *strCat(char *s1, const char *s2, ...) throw(std::bad_alloc);
// Skip spaces at the beggining of a string
char *skipSpaces(char *s) throw();
// Remove all the spaces at the end of a string
void removeEndSpaces(char *s) throw();
// A combination of skipSpaces and removeEndSpaces
char *removeSpaces(char *s) throw();
// Check if a file exists
bool FileExists(const char *name) throw();
// Find a file in a list of directories (can include PATH)
bool FindFile(const char *name, char *buf, unsigned size, const char *dirs[]) throw();

} // namespace cppfio
#endif // CPPFIO_HELPERS_H
