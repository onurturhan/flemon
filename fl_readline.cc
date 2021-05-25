/* Copyright (C) 1987-2002 Free Software Foundation, Inc.

   This file is part of the GNU Readline Library, a library for
   reading lines of text with interactive input and history editing.

   The GNU Readline Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The GNU Readline Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   The GNU General Public License is often shipped with GNU software, and
   is generally kept in a file called COPYING or LICENSE.  If you do not
   have a copy of the license, write to the Free Software Foundation,
   59 Temple Place, Suite 330, Boston, MA 02111 USA.

   Sourcecode from:
   fileman.c -- A tiny application which demonstrates how to use the
   GNU Readline library.  This application interactively allows users
   to manipulate files and their modes.

   Adaptation for FleMon:
   Diego J. Brengi. brengi_at_inti.gov.ar
   Copyright (C) 2009 Diego Brengi.
   Copyright (C) 2009 Instituto Nacional de Tecnología Industrial.
*/

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

extern void PrintVersion(void);
extern  int flemon_done;
extern int func_sync(const char * param);
extern int func_scanAHB(const char * param);
extern int func_scanAPB(const char * param);
extern int func_scan(const char * param);
extern int func_detect(char * param);
extern int func_ls(char * param);
extern int func_mem(char * param);
extern int func_cleancores(char * param);

extern int func_flash(char * param);
extern int func_flock (char * param);
extern int func_funlock (char * param);
extern int func_fstat (char * param);
extern int func_fcfi (char * param);
extern int func_fscan (const char * param);
extern int func_ferase (char * param);

extern int func_fdumpblock (char * param);
extern int func_fdumpfile (char * param);
extern int func_fwrite (char * param);
extern int func_fcheck (char * param);

extern int func_ffload (char * param);
extern int func_ffverify (char * param);
extern int func_fferase (char * param);

extern int func_fxload (char * param);
extern int func_fxverify (char * param);
extern int func_fxerase (char * param);

int func_help(char * param);

int func_quit(char * param)
{
printf("bye bye FleMon...\n");
exit(EXIT_SUCCESS);
}
  
/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
  const char *name;                   /* User printable name of the function. */
  rl_icpfunc_t *func;           /* Function to call to do the job. */
  const char *doc;                    /* Documentation for this function.  */
} COMMAND;

static
COMMAND commands[] = {
  { "help",    (rl_icpfunc_t *)func_help, "Display this text." },
  { "?",       (rl_icpfunc_t *)func_help, "Synonym for `help'." },
  { "quit",    (rl_icpfunc_t *)func_quit, "Quit using FleMon." },
  { "exit",    (rl_icpfunc_t *)func_quit, "Quit using FleMon." },
  { "sync",   (rl_icpfunc_t *)func_sync, "Send SYNC bytes and establish connection." },
  { "scanh",  (rl_icpfunc_t *)func_scanAHB, "Scan only AHB bus." },
  { "scanp",  (rl_icpfunc_t *)func_scanAPB, "Scan only APB bus." },
  { "scan",   (rl_icpfunc_t *)func_scan, "Scan AHB and APB buses." },
  { "detect", (rl_icpfunc_t *)func_detect, "Commands: sync, scanh, scanp and fscan." },
  { "ls",     (rl_icpfunc_t *)func_ls, "List AHB/APB detected cores. -l extended. -c collapse." },
  { "mem",    (rl_icpfunc_t *)func_mem, "Report memory map." },
  { "cleancores",  (rl_icpfunc_t *)func_cleancores, "Erase all core's information." },
  { "fscan",  (rl_icpfunc_t *)func_fscan, "Flash detection." },
  { "finfo",  (rl_icpfunc_t *)func_flash, "Flash report." },
  { "flock",  (rl_icpfunc_t *)func_flock, "Flash lock [all | offset]." },
  { "funlock",(rl_icpfunc_t *)func_funlock, "Flash unlock (all banks)" },
  { "fstat",  (rl_icpfunc_t *)func_fstat, "Flash lock bits status." },
  { "fcfi",  (rl_icpfunc_t *)func_fcfi, "Flash CFI dump." },
  { "ferase",  (rl_icpfunc_t *)func_ferase, "Flash bank erase. [offset] or 'all'." },
  { "fdumpblock",  (rl_icpfunc_t *)func_fdumpblock, "Flash dump block. [offset|all start] [offset end]." },
  { "fwrite",  (rl_icpfunc_t *)func_fwrite, "Flash write a single word. [offset] [data]" },
  { "fcheck",  (rl_icpfunc_t *)func_fcheck, "Flash blank check. [offset start] [offset end]" },

  { "fdumpfile",  (rl_icpfunc_t *)func_fdumpfile, "Flash dump to a file. [offset|all start] [offset end]." },
  { "ffload",  (rl_icpfunc_t *)func_ffload, "Flash load a plain data file. [filename]" },
  { "fferase",  (rl_icpfunc_t *)func_fferase, "Erase only needeed banks for a plain data file. [filename]" },
  { "ffverify",  (rl_icpfunc_t *)func_ffverify, "Flash data verification (after fload). [filename]" },

  { "fxerase",  (rl_icpfunc_t *)func_fxerase, "Erase only needed banks for an ELF file. [filename]" },
  { "fxload",  (rl_icpfunc_t *)func_fxload,   "Flash load ELF file. [filename]" },
  { "fxverify",  (rl_icpfunc_t *)func_fxverify, "Flash ELF verification (after fxload). [filename]" },
  { (char *)NULL, (rl_icpfunc_t *)NULL, (char *)NULL }
};

/* The names of functions that actually do the manipulation. */
int func_help(char * nouse)
{
int i;
PrintVersion();
  printf("Commands:\n");
  for (i = 0; commands[i].name; i++)
    printf("  %-15s %s\n",commands[i].name, commands[i].doc);
return 0;
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
COMMAND * find_command (char * name)
{
  register int i;

  for (i = 0; commands[i].name; i++)
    if (strcmp (name, commands[i].name) == 0)
      return (&commands[i]);

  return ((COMMAND *)NULL);
}


/* Execute a command line. */
int execute_line (char * line)
{
  register int i;
  COMMAND *command;
  char *word;

  /* Isolate the command word. */
  i = 0;
  while (line[i] && whitespace (line[i]))
    i++;
  word = line + i;

  while (line[i] && !whitespace (line[i]))
    i++;

  if (line[i])
    line[i++] = '\0';

  command = find_command (word);

  if (!command)
    {
      fprintf (stderr, "%s: No such command for FleMon.\n", word);
      return (-1);
    }

  /* Get argument to command, if any. */
  while (whitespace (line[i]))
    i++;

  word = line + i;

  /* Call the function. */
  return ((*(command->func)) (word));
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char * stripwhite (char * string)
{
  register char *s, *t;

  for (s = string; whitespace (*s); s++)
    ;
    
  if (*s == 0)
    return (s);

  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t))
    t--;
  *++t = '\0';

  return s;
}

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */
/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. */
char * command_generator (const char * text, int state)
{
  static int list_index, len;
  const char *name;

  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
    {
      list_index = 0;
      len = strlen (text);
    }

  /* Return the next name which partially matches from the command list. */
  while ((name = commands[list_index].name))
    {
      list_index++;

      if (strncmp (name, text, len) == 0)
        return (strdup(name));
    }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}

/* Attempt to complete on the contents of TEXT.  START and END bound the
   region of rl_line_buffer that contains the word to complete.  TEXT is
   the word to complete.  We can use the entire contents of rl_line_buffer
   in case we want to do some simple parsing.  Return the array of matches,
   or NULL if there aren't any. */
char ** flemon_completion (const char * text, int start, int end)
{
  char **matches;

  matches = (char **)NULL;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if (start == 0)
    matches = rl_completion_matches (text, command_generator);

  return (matches);
}

/* Tell the GNU Readline library how to complete.  We want to try to complete
   on command names if this is the first word in the line, or on filenames
   if not. */
void initialize_readline (void)
{
  /* Allow conditional parsing of the ~/.inputrc file. */
  /*rl_readline_name = "FileMan";*/

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = flemon_completion;
}

