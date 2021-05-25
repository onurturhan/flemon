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
  Implementa lo necesario para realizar tareas periódicamente.
  
***************************************************************************/

#define CPPFIO_I
#define CPPFIO_I_stdio // NULL
#define CPPFIO_Chrono
#include <cppfio.h>

using namespace cppfio;

void Chrono::Start()
 throw()
{
 gettimeofday(&start,NULL);
}

double Chrono::Stop()
 throw()
{
 gettimeofday(&stop,NULL);
 return GetDiff();
}

double Chrono::GetDiff()
 throw()
{
 return (stop.tv_sec+stop.tv_usec/1e6)-(start.tv_sec+start.tv_usec/1e6);
}

