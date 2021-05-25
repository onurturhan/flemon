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

  Description:
  Common exceptions.

***************************************************************************/

#if !defined(CPPFIO_EXCP_H)
#define CPPFIO_EXCP_H

namespace cppfio
{

class ExProcess : public std::exception
{
public:
 ExProcess() throw() : exception() {}
 virtual const char *what() const throw();
};

class ExIntegrity : public ExProcess
{
public:
 ExIntegrity() throw() : ExProcess() {}
 virtual const char *what() const throw();
};

class ExIO : public std::exception
{
public:
 ExIO() throw() : exception() {}
 virtual const char *what() const throw();
};

class ExOpen : public ExIO
{
public:
 ExOpen() throw() : ExIO() { name=NULL; }
 ExOpen(const char *aName) throw() : ExIO() { name=aName; }
 virtual const char *what() const throw();

protected:
 const char *name;
};

class ExClose : public ExIO
{
public:
 ExClose() throw() : ExIO() {}
 virtual const char *what() const throw();
};

class ExRead : public ExIO
{
public:
 ExRead() throw() : ExIO() {}
 virtual const char *what() const throw();
};

class ExWrite : public ExIO
{
public:
 ExWrite() throw() : ExIO() {}
 virtual const char *what() const throw();
};

class ExNoDevice : public ExIO
{
public:
 ExNoDevice() throw() : ExIO() {}
 virtual const char *what() const throw();
};

class ExTimedOut : public ExIO
{
public:
 ExTimedOut() throw() : ExIO() {}
 virtual const char *what() const throw();
};

class ExNULLPointer : public std::exception
{
public:
 ExNULLPointer() throw() : exception() {}
 virtual const char *what() const throw();
};

class ExBufferOverflow : public std::exception
{
public:
 ExBufferOverflow() throw() : exception() {}
 virtual const char *what() const throw();
};

class ExPCRE : public std::exception
{
public:
 ExPCRE(const char *theErr) throw() : exception() { error=theErr; }
 virtual const char *what() const throw();

protected:
 const char *error;
};

} // namespace cppfio
#endif // CPPFIO_EXCP_H
