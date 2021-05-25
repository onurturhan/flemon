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

#define CPPFIO_I
#define CPPFIO_I_stdio
#define CPPFIO_PTimer
#include <cppfio.h>

using namespace cppfio;

PTimer::PTimer(unsigned aPeriod)
 throw()
{
 period=aPeriod;
 Start();
}

void PTimer::Set()
 throw()
{
 gettimeofday(&next,NULL);
}

void PTimer::Add(unsigned micros)
 throw()
{
 next.tv_usec+=micros;
 if (next.tv_usec>=1000000)
   {
    next.tv_sec+=next.tv_usec/1000000;
    next.tv_usec=next.tv_usec%1000000;
   }
}

bool PTimer::Reached()
 throw()
{
 struct timeval now;
 gettimeofday(&now,NULL);
 if (now.tv_sec<next.tv_sec)
    return 0;
 if (now.tv_sec>next.tv_sec)
    return 1;
 return now.tv_usec>=next.tv_usec;
}

