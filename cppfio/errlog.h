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

***************************************************************************/

#if !defined(CPPFIO_ERRLOG_H)
#define CPPFIO_ERRLOG_H

namespace cppfio
{

class ErrorLog : public File
{
public:
 ErrorLog(const char *name) throw(ExOpen);
 ErrorLog(FILE *f)          throw(ExOpen);

 void Abort(const char *fmt, ...)   throw(ExWrite);
 void Error(const char *fmt, ...)   throw(ExWrite);
 void ErrorImpl(const char *fmt, va_list argptr) throw(ExWrite);
 void Warning(const char *fmt, ...) throw(ExWrite);
 void Log(const char *fmt, ...)     throw(ExWrite);
 void SetFile(const char *name)     throw() { fileName=name; }
 const char *GetFile()              throw() { return fileName; }
 void LogOpts(clpOption *opts)      throw(ExWrite);

 int line;
protected:
 const char *fileName;
};

}// namespace cppfio

#endif // CPPFIO_ERRLOG_H

