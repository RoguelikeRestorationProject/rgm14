/*
 * findscore.c: Rog-O-Matic XIV (CMU) Sun Jul  6 20:13:19 1986 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * Read the Rogue scoreboard to determine a goal score.
 *
 * EDITLOG
 *	LastEditDate = Sun Jul  6 20:13:19 1986 - Michael Mauldin
 *	LastFileName = /usre3/mlm/src/rog/ver14/findscore.c
 *
 * HISTORY
 *  6-Jul-86  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created.
 */

# include <stdio.h>
# include "install.h"
char TEMPFL[18] = "/tmp/RscoreXXXXXX";
# define ISDIGIT(c) ((c) >= '0' && (c) <= '9')

findscore (rogue, roguename)
register char *rogue, *roguename;
{ register int score, best = -1;
  char cmd[100], buffer[BUFSIZ];
  register char *s, *tmpfname = TEMPFL;
  FILE *tmpfil;
  int fd;

  fd = mkstemp(tmpfname);

  if (fd > 0)
  {
    /* Run 'rogue -s', and put the scores into a temp file */
    sprintf (cmd, "%s -s >%s", rogue, tmpfname); 
    system (cmd); 
  }

  /* If no temp file created, return default score */
  if (fd <= 0 || (tmpfil = fdopen(fd,"r")) == NULL)
    return (best); 

  /* Skip to the line starting with 'Rank...'. */
  while (fgets (buffer, BUFSIZ, tmpfil) != NULL)
    if (stlmatch (buffer, "Rank")) break;

  if (! feof (tmpfil)) 
  { while (fgets (buffer, BUFSIZ, tmpfil) != NULL)
    { s = buffer;				/* point s at buffer */
      while (ISDIGIT (*s)) s++;			/* Skip over rank */
      while (*s == ' ' || *s == '\t') s++;	/* Skip to score */
      score = atoi (s);				/* Read score */
      while (ISDIGIT (*s)) s++;			/* Skip over score */
      while (*s == ' ' || *s == '\t') s++;	/* Skip to player */

      if (stlmatch (s, roguename))		/* Found our heros name */
      { if (best < 0) best = score;		/* Rogy is on top! */
	break;					/* 'best' is now target */
      }

      if (score < BOGUS && 
	  (score < best || best < 0))		/* Save smallest score */
        best = score;				/*  above Rogy's score */
    }
  }

  unlink (tmpfname); 

  /* Don't quit for very small scores, it's not worth it */
  if (best < 2000) best = -1;

  return (best);
}
