/**[txh]********************************************************************

  Copyright (c) 2009 Salvador E. Tropea <salvador en inti gov ar>
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
 
  Module: Tiempo
  Description:
  Implementa funciones de cronómetro.
  
***************************************************************************/

#if !defined(CPPFIO_CHRONO_H)
#define CPPFIO_CHRONO_H

namespace cppfio
{

class Chrono
{
public:
 Chrono() throw() { Start(); }
 void   Start()   throw();
 double Stop()    throw();
 double GetDiff() throw();

protected:
 struct timeval start, stop;
};

} // namespace cppfio
#endif // CPPFIO_CHRONO_H
