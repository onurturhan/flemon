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
  Command Line parser and helper header.

***************************************************************************/

#if !defined(CPPFIO_CMDLINE_H)
#define CPPFIO_CMDLINE_H

namespace cppfio
{

// To indicate if we expect arguments
typedef enum { argNo=0, argRequired, argOptional } argType;
// How to process an argument
typedef enum
{
 argBool,     // bool *, set to true if the option appears at least once
 argFunction, // Function callback, invoked with the argument as parameter
 argInteger,  // long *, converted using strtol (decimal, hexa and octal)
 argString,   // char **, set to point the argument
 argDouble    // double *, converted using strtod
} argKind;

struct clpOption
{
 const char *name; // Long name (i.e. "help")
 int shortName;    // Short name (i.e. 'h')
 argType hasArg;   // Are arguments expected?
 void *pointer;    // Pointer to the data or function to hold/process
 argKind kind;     // Kind of pointer provided in the pointer field
};

typedef struct
{
 void (*prgInfo)();        // Banner for the program
 void (*printHelp)();      // --help text
 void (*printVersion)();   // --version text
 struct clpOption *opts;   // Command line options
 int extraOpts;            // How many file names are expected after the
                           // options, -1==0 or more
} cmdLineOpts;

extern cmdLineOpts *noCustomOptions;

class CmdLineParser
{
public:
 CmdLineParser() throw();
 ~CmdLineParser() throw();
 int GetExtraOptsFile(const char *file) throw();
 void Run(int aArgc, char **aArgv, cmdLineOpts *theOpts=noCustomOptions)
   throw(ExNULLPointer,std::bad_alloc);
 int GetFirstFile() throw() { return lastUsed; }
 int GetVerbosity() throw() { return verbose; }
 int SetVerbosity(int nv) throw() { int r=verbose; verbose=nv; return r; }
 int PrDebug(int level, const char *fmt, ...) throw();
 static void PrintGPL(FILE *f) throw();
 static void PrDefaultOptions(FILE *f) throw();
 // Print the message if the verbosity is >= level
 int printIfV(int level, const char *fmt, ...) throw();
 // Print the message if the verbosity is < level
 int printIfQ(int level, const char *fmt, ...) throw();
 // Dump the options
 void PrintOptions(File &f, cmdLineOpts *opts, const char *pre)
   throw(std::bad_alloc,ExWrite);

protected:
 option *longOptions;
 char *shortOptions;
 int lastUsed;
 int verbose;
 FILE *dbgOut;
 int eArgc;
 char **eArgv;
};

} // namespace cppfio
#endif // CPPFIO_CMDLINE_H
