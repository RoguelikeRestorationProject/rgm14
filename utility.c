/*
 * utility.c: Rog-O-Matic XIV (CMU) Tue Mar 26 15:27:03 1985 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * This file contains all of the miscellaneous system functions which
 * determine the baud rate, time of day, etc.
 *
 * If CMU is not defined, then various functions from libcmu.a are
 * defined here (otherwise the functions from -lcmu are used).
 */

# include <sgtty.h>
# include <stdio.h>
# include <signal.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <string.h>
# include <stdlib.h>
# include <pwd.h>

# include <curses.h>
# include "install.h"

# ifndef BSD42
#     include <time.h>
# else
#     include <sys/time.h>
# endif

# define TRUE 1
# define FALSE 0

/*
 * baudrate: Determine the baud rate of the terminal
 */

#if !defined(_XOPEN_CURSES) && !defined(__NCURSES_H)
baudrate ()
{ static short  baud_convert[] =
  { 0, 50, 75, 110, 135, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600 };
  static struct sgttyb  sg;
  static short  baud_rate;

  gtty (fileno (stdin), &sg);
  baud_rate = sg.sg_ospeed == 0 ? 1200
    : sg.sg_ospeed < sizeof baud_convert / sizeof baud_convert[0]
    ? baud_convert[sg.sg_ospeed] : 9600;

  return (baud_rate);
}
#endif

/*
 * getname: get userid of player.
 */

char *getname ()
{ static char name[100];
  struct passwd *pw;
  
  name[0] = 0;

  pw = getpwuid(getuid());

  if (pw != NULL)
      strncpy(name, pw->pw_name, 100);

  name[99] = 0;

  return (name);
}

/*
 * wopen: Open a file for world access.
 */

FILE *wopen(fname, mode)
char *fname, *mode;
{ int oldmask;
  FILE *newlog;

  oldmask = umask (0111);
  newlog = fopen (fname, mode);
  umask (oldmask);

  return (newlog);  
}

/*
 * fexists: return a boolean if the named file exists
 */

fexists (fn)
char *fn;
{ struct stat pbuf;

  return (stat (fn, &pbuf) == 0);
}

/*
 * filelength: Do a stat and return the length of a file.
 */

int filelength (f)
char *f;
{ struct stat sbuf;

  if (stat (f, &sbuf) == 0)
    return (sbuf.st_size);
  else
    return (-1);
}

/*
 * critical: Disable interrupts
 */

static void (*hstat)(int), (*istat)(int), (*qstat)(int), (*pstat)(int);

critical ()
{
  hstat = signal (SIGHUP, SIG_IGN);
  istat = signal (SIGINT, SIG_IGN);
  pstat = signal (SIGPIPE, SIG_IGN);
  qstat = signal (SIGQUIT, SIG_IGN);
}

/*
 * uncritical: Enable interrupts
 */

uncritical ()
{
  signal (SIGHUP, hstat);
  signal (SIGINT, istat);
  signal (SIGPIPE, pstat);
  signal (SIGQUIT, qstat);
}

/*
 * reset_int: Set all interrupts to default
 */

reset_int ()
{
  signal (SIGHUP, SIG_DFL);
  signal (SIGINT, SIG_DFL);
  signal (SIGPIPE, SIG_DFL);
  signal (SIGQUIT, SIG_DFL);
}

/*
 * int_exit: Set up a function to call if we get an interrupt
 */
int_exit (exitproc)
void (*exitproc)(int);
{
  if (signal (SIGHUP, SIG_IGN) != SIG_IGN)  signal (SIGHUP, exitproc);
  if (signal (SIGINT, SIG_IGN) != SIG_IGN)  signal (SIGINT, exitproc);
  if (signal (SIGPIPE, SIG_IGN) != SIG_IGN) signal (SIGPIPE, exitproc);
  if (signal (SIGQUIT, SIG_IGN) != SIG_IGN) signal (SIGQUIT, exitproc);
}

/*
 * lock_file: lock a file for a maximum number of seconds.
 *            Based on the method used in Rogue 5.2.
 */

# define NOWRITE 0

lock_file (lokfil, maxtime)
char *lokfil;
int maxtime;
{ int try;
  struct stat statbuf;
  long time ();

  start:
  if (creat (lokfil, NOWRITE) > 0)
    return TRUE;

  for (try = 0; try < 60; try++)
  { sleep (1);
    if (creat (lokfil, NOWRITE) > 0)
      return TRUE;
  }

  if (stat (lokfil, &statbuf) < 0)
  { creat (lokfil, NOWRITE);
    return TRUE;
  }

  if (time (0) - statbuf.st_mtime > maxtime)
  { if (unlink (lokfil) < 0)
      return FALSE;
    goto start;
  }
  else
    return FALSE;
}

/*
 * unlock_file: Unlock a lock file.
 */

unlock_file (lokfil)
char *lokfil;
{ unlink (lokfil);
}

# ifndef CMU
/*
 * quit: Defined for compatibility with Berkeley 4.2 system
 */

/* VARARGS2 */
quit (int code, char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  exit (code);
}

/*
 * stlmatch  --  match leftmost part of string
 *
 *  Usage:  i = stlmatch (big,small)
 *	int i;
 *	char *small, *big;
 *
 *  Returns 1 iff initial characters of big match small exactly;
 *  else 0.
 *
 *  HISTORY
 * 18-May-82 Michael Mauldin (mlm) at Carnegie-Mellon University
 *      Ripped out of CMU lib for Rog-O-Matic portability
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX from Ken Greer's routine.
 *
 *  Originally from klg (Ken Greer) on IUS/SUS UNIX
 */

int stlmatch (big, small)
char *small, *big;
{ register char *s, *b;
  s = small;
  b = big;
  do
  { if (*s == '\0')
      return (1);
  }
  while (*s++ == *b++);
  return (0);
}
# endif

#if 0
/*
 *  putenv  --  put value into environment
 *
 *  Usage:  i = putenv (name,value)
 *	int i;
 *	char *name, *value;
 *
 *  Putenv associates "value" with the environment parameter "name".
 *  If "value" is 0, then "name" will be deleted from the environment.
 *  Putenv returns 0 normally, -1 on error (not enough core for malloc).
 *
 *  Putenv may need to add a new name into the environment, or to
 *  associate a value longer than the current value with a particular
 *  name.  So, to make life simpler, putenv() copies your entire
 *  environment into the heap (i.e. malloc()) from the stack
 *  (i.e. where it resides when your process is initiated) the first
 *  time you call it.
 *
 *  HISTORY
 * 25-Nov-82 Michael Mauldin (mlm) at Carnegie-Mellon University
 *      Ripped out of CMU lib for Rog-O-Matic portability
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Created for VAX.  Too bad Bell Labs didn't provide this.  It's
 *	unfortunate that you have to copy the whole environment onto the
 *	heap, but the bookkeeping-and-not-so-much-copying approach turns
 *	out to be much hairier.  So, I decided to do the simple thing,
 *	copying the entire environment onto the heap the first time you
 *	call putenv(), then doing realloc() uniformly later on.
 *	Note that "putenv(name,getenv(name))" is a no-op; that's the reason
 *	for the use of a 0 pointer to tell putenv() to delete an entry.
 *
 */

#define EXTRASIZE 5		/* increment to add to env. size */

static int  envsize = -1;	/* current size of environment */
extern char **environ;		/* the global which is your env. */

static int  findenv ();		/* look for a name in the env. */
static int  newenv ();		/* copy env. from stack to heap */
static int  moreenv ();		/* incr. size of env. */

int   putenv (name, value)
char *name, *value;
{ register int  i, j;
  register char *p;

  if (envsize < 0)
  {				/* first time putenv called */
    if (newenv () < 0)		/* copy env. to heap */
      return (-1);
  }

  i = findenv (name);		/* look for name in environment */

  if (value)
  {				/* put value into environment */
    if (i < 0)
    {				/* name must be added */
      for (i = 0; environ[i]; i++);
      if (i >= (envsize - 1))
      {				/* need new slot */
	if (moreenv () < 0)
	  return (-1);
      }
      p = malloc (strlen (name) + strlen (value) + 2);
      if (p == 0)		/* not enough core */
	return (-1);
      environ[i + 1] = 0;	/* new end of env. */
    }
    else
    {				/* name already in env. */
      p = realloc (environ[i],
	  strlen (name) + strlen (value) + 2);
      if (p == 0)
	return (-1);
    }
    sprintf (p, "%s=%s", name, value);/* copy into env. */
    environ[i] = p;
  }
  else
  {				/* delete name from environment */
    if (i >= 0)
    {				/* name is currently in env. */
      free (environ[i]);
      for (j = i; environ[j]; j++);
      environ[i] = environ[j - 1];
      environ[j - 1] = 0;
    }
  }

  return (0);
}

static int  findenv (name)
char *name;
{ register char *namechar, *envchar;
  register int  i, found;

  found = 0;
  for (i = 0; environ[i] && !found; i++)
  { envchar = environ[i];
    namechar = name;
    while (*namechar && (*namechar == *envchar))
    { namechar++;
      envchar++;
    }
    found = (*namechar == '\0' && *envchar == '=');
  }
  return (found ? i - 1 : -1);
}

static int  newenv ()
{ register char **env, *elem;
  register int  i, esize;

  for (i = 0; environ[i]; i++);
  esize = i + EXTRASIZE + 1;
  env = (char **) malloc (esize * sizeof (elem));
  if (env == 0)
    return (-1);

  for (i = 0; environ[i]; i++)
  { elem = malloc (strlen (environ[i]) + 1);
    if (elem == 0)
      return (-1);
    env[i] = elem;
    strcpy (elem, environ[i]);
  }

  env[i] = 0;
  environ = env;
  envsize = esize;
  return (0);
}

static int  moreenv ()
{ register int  esize;
  register char **env;

  esize = envsize + EXTRASIZE;
  env = (char **) realloc (environ, esize * sizeof (*env));
  if (env == 0)
    return (-1);
  environ = env;
  envsize = esize;
  return (0);
}
#endif
