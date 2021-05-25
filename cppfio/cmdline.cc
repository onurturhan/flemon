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

  Module: Command Line
  Description:
  Command Line parser and helper.@p
  Objetive: simplify the command line parsing, provide a complete and
uniform command line interface.@p
  Automagically provides: -h/--help, -V/--version, -q/--quiet and
-v/--verbose.@p
  You have to create some small functions to:
  * Print copyright information [optional]
  * Print program name + version (banner) [optional]
  * Print command line help [optional]
  * Print version, license and author information [optional]
  You have to fill a vector of clpOption structures with the command line
options.

***************************************************************************/
/*****************************************************************************

 Target:      Any
 Language:    C++
 Compiler:    GNU g++ 4.1.1 (Debian GNU/Linux)
 Text editor: SETEdit 0.5.5

 Portability notes:
 Uses GNU's getopt_long.
    
*****************************************************************************/

#define CPPFIO_I
#define CPPFIO_CmdLine
#define CPPFIO_Helpers
#define CPPFIO_TextFile
#define CPPFIO_I_stdlib
#define CPPFIO_I_string
#include <cppfio.h>
#include <stdarg.h>
#include <ctype.h>

using namespace cppfio;

static cmdLineOpts nullOptions={ NULL, NULL, NULL, NULL, 0 };
cmdLineOpts *cppfio::noCustomOptions=&nullOptions;

CmdLineParser::CmdLineParser()
  throw()
{
 verbose=0;
 shortOptions=NULL;
 longOptions=NULL;
 dbgOut=stderr;
 eArgc=0;
 eArgv=NULL;
}

CmdLineParser::~CmdLineParser()
  throw()
{
 delete[] shortOptions;
 delete[] longOptions;
 if (eArgc)
   {
    while (eArgc)
       delete[] eArgv[--eArgc];
    delete[] eArgv;
   }
}

static
void NoPrgInfo(const char *prgName, int verbose)
  throw()
{
 if (verbose<0)
    return;
 fprintf(stderr,"%s\n",prgName);
 fprintf(stderr,"All rights reserved.\n");
 fprintf(stderr,"\n");
}

static
void NoHelp()
{
 puts("Available options:\n");
 CmdLineParser::PrDefaultOptions(stdout);
 exit(1);
}

#define CallPrgInfo() { if (opts->prgInfo) opts->prgInfo(); else \
                        NoPrgInfo(argv[0],verbose); }

void CmdLineParser::Run(int aArgc, char **aArgv, cmdLineOpts *opts)
  throw(ExNULLPointer,std::bad_alloc)
{
 // Sanity
 if (!opts)
    throw ExNULLPointer();
 if (!opts->printHelp)
    opts->printHelp=NoHelp;

 // Init
 // Create the structures for getopt_long
 // Meassure
 clpOption *opt=opts->opts;
 clpOption *copts=opt;
 int numOptions=0;
 unsigned size=0;

 while (opt && opt->name)
   {
    numOptions++;
    if (opt->shortName)
      {
       size++;
       if (opt->hasArg!=argNo)
          size++;
      }
    opt++;
   }
 // Create
 longOptions=new option[numOptions+5];
 shortOptions=new char[size+5];
 opt=copts;
 numOptions=0;
 size=0;
 option *p;
 while (opt && opt->name)
   {
    p=&longOptions[numOptions];
    p->name=opt->name;
    p->val=opt->shortName;
    p->has_arg=int(opt->hasArg);
    p->flag=NULL;
    //printf("{\"%s\",%d,%p,'%c'}\n",p->name,p->has_arg,p->flag,p->val);
    numOptions++;
    if (opt->shortName)
      {
       shortOptions[size++]=opt->shortName;
       if (opt->hasArg!=argNo)
          shortOptions[size++]=':';
      }
    opt++;
   }
 // Add the help option
 p=&longOptions[numOptions];
 p->name="help";
 p->val=shortOptions[size++]='h';
 p->has_arg=0;
 p->flag=NULL;
 numOptions++;
 // Add quiet and verbose options
 p=&longOptions[numOptions];
 p->name="quiet";
 p->val=shortOptions[size++]='q';
 p->has_arg=0;
 p->flag=NULL;
 numOptions++;
 p=&longOptions[numOptions];
 p->name="verbose";
 p->val=shortOptions[size++]='v';
 p->has_arg=0;
 p->flag=NULL;
 numOptions++;
 // Add version option
 p=&longOptions[numOptions];
 p->name="version";
 p->val=shortOptions[size++]='V';
 p->has_arg=0;
 p->flag=NULL;
 numOptions++;
 // Finish the short options string
 shortOptions[size++]=0;
 //printf("\"%s\" [%d]\n",shortOptions,size);

 // Do it
 int optc, argc=aArgc;
 char **argv=aArgv;
 if (eArgc)
   {// Add extra options
    argv=new char *[argc+eArgc];
    argv[0]=aArgv[0];
    memcpy(argv+1,eArgv,sizeof(char *)*eArgc);
    memcpy(argv+eArgc+1,aArgv+1,sizeof(char *)*(argc-1));
    argc+=eArgc;
   }

 while ((optc=getopt_long(argc,argv,shortOptions,longOptions,0))!=EOF)
   {
    switch (optc)
      {
       case 'h':
            CallPrgInfo();
            opts->printHelp();
            break;
       case 'q':
            verbose--;
            break;
       case 'v':
            verbose++;
            break;
       case 'V':
            if (opts->printVersion)
               opts->printVersion();
            else
               CallPrgInfo();
            exit(0);
            break;
       default:
            clpOption *p=copts;
     
            for (; p->name; p++)
               {
                if (p->shortName==optc)
                  {
                   if (p->pointer==NULL)
                      throw ExNULLPointer();
                   switch (p->kind)
                     {
                      case argBool:
                           *((bool *)p->pointer)=true;
                           break;
                      case argFunction:
                           ((void (*)(char *))p->pointer)(optarg);
                           break;
                      case argInteger:
                           *((long *)p->pointer)=strtol(optarg,NULL,0);
                           break;
                      case argString:
                           *((char **)p->pointer)=optarg;
                           break;
                      case argDouble:
                           *((double *)p->pointer)=strtod(optarg,NULL);
                           break;
                     }
                   break;
                  }
               }
            if (!p->name)
              {
               CallPrgInfo();
               fprintf(stderr,"error: unknown option %s\n\n",argv[optind]);
               opts->printHelp();
              }
      }
   }
 CallPrgInfo();
 if (opts->extraOpts>=0)
   {
    int extra=argc-optind;
    int diff=opts->extraOpts-extra;
    if (diff>0)
      {
       fprintf(stderr,"error: missing arguments (%d)\n\n",diff);
       opts->printHelp();
      }
    if (diff<0)
      {
       fprintf(stderr,"error: too many arguments (%d)\n\n",-diff);
       opts->printHelp();
      }
   }
 lastUsed=optind;
 if (eArgc)
   {// Remove extra options
    delete[] argv;
    lastUsed-=eArgc;
   }
}

void CmdLineParser::PrintOptions(File &f, cmdLineOpts *opts, const char *pre)
  throw(std::bad_alloc,ExWrite)
{
 clpOption *opt=opts->opts;

 while (opt && opt->name)
   {
    if (pre)
       f.Print(pre);
    f.Print("--%s=",opt->name);
    switch (opt->kind)
      {
       case argBool:
            f.Print("%s\n",*((bool *)opt->pointer) ? "true" : "false");
            break;
       case argFunction:
            f.Print("<handled by a function>\n");
            break;
       case argInteger:
            f.Print("%ld\n",*((long *)opt->pointer));
            break;
       case argString:
            f.Print("\"%s\"\n",*((char **)opt->pointer));
            break;
       case argDouble:
            f.Print("%g\n",*((double *)opt->pointer));
            break;
      }
    opt++;
   }
}

int CmdLineParser::PrDebug(int level, const char *fmt, ...)
  throw()
{
 if (verbose<level)
    return 0;

 va_list argptr;
 va_start(argptr,fmt);
 int ret=vfprintf(dbgOut,fmt,argptr);
 va_end(argptr);

 return ret;
}

void CmdLineParser::PrintGPL(FILE *f)
  throw()
{
 fputs("This is free software.  You may redistribute copies of it under the terms of\n"
       "the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n"
       "There is NO WARRANTY, to the extent permitted by law.\n",f);
}

void CmdLineParser::PrDefaultOptions(FILE *f)
  throw()
{                                                        
 fputs("-h, --help                  print this help and exit\n",f);
 fputs("-q, --quiet                 reduce verbosity\n",f);
 fputs("-v, --verbose               increase verbosity\n",f);
 fputs("-V, --version               show version information and exit\n",f);
}

int CmdLineParser::GetExtraOptsFile(const char *file)
  throw()
{
 int c=0;
 try
 {
  int len, lines=0;
  char *s;
  TextFile f(file);
  do
    {
     s=f.ReadLineNoEOL(len);
     if (!s) break;
     for (; *s && isspace((unsigned char)*s); s++);
     if (*s && *s!='#')
        lines++;
    }
  while (!f.Eof());
  if (lines)
    {
     if (eArgc)
       {
        char **aux=new char *[eArgc+lines];
        memcpy(aux,eArgv,sizeof(char *)*eArgc);
        delete[] eArgv;
        eArgv=aux;
       }
     else
       {
        eArgv=new char *[lines];
       }
     f.Rewind();
     c=lines;
     do
       {
        s=f.ReadLineNoEOL(len);
        if (!s) break;
        for (; *s && isspace((unsigned char)*s); s++);
        if (*s && *s!='#')
          {
           eArgv[eArgc++]=newStr(s,len);
           lines--;
          }
       }
     while (lines);
    }
 }
 catch (...)
 {
  c=0;
 }
 return c;
}

int CmdLineParser::printIfV(int level, const char *fmt, ...)
 throw()
{
 if (GetVerbosity()<level)
    return 0;
 va_list argptr;

 va_start(argptr,fmt);
 int ret=vprintf(fmt,argptr);
 va_end(argptr);

 return ret;
}

int CmdLineParser::printIfQ(int level, const char *fmt, ...)
 throw()
{
 if (GetVerbosity()>=level)
    return 0;
 va_list argptr;

 va_start(argptr,fmt);
 int ret=vprintf(fmt,argptr);
 va_end(argptr);

 return ret;
}

