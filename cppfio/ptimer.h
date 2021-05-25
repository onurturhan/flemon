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
 
  Module: Tiempo
  Description:
  Implementa lo necesario para realizar tareas periódicamente.
  
***************************************************************************/

#if !defined(CPPFIO_PTIMER_H)
#define CPPFIO_PTIMER_H

namespace cppfio
{

class PTimer
{
public:
 PTimer(unsigned aPeriod) throw();
 void Start() throw() { Set(); Next(); }
 void Set() throw();
 void Next() throw() { Add(period); }
 bool Reached() throw();
 void Add(unsigned micros) throw();

protected:
 struct timeval next;
 unsigned period;
};

} // namespace cppfio
#endif // CPPFIO_PTIMER_H
