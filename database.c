/*
 * database.c: Rog-O-Matic XIV (CMU) Fri Dec 28 23:26:25 1984 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 * 
 * This file contains the code which handles the database of objects which
 * have been used, and what the real names of the items are. There are
 * five functions:
 *
 *      useobj (oldname)             enter the object type into the database
 *      infername (oldname, name)    make the real name of obj be name
 *      used (oldname)               returns TRUE if we have entered obj
 *      know (name)                  returns TRUE if we have inferred name
 *      realname (oldname)           returns the inferred name of obj
 */

# include <curses.h>
# include <string.h>
# include "types.h"
# include "globals.h"

# define TABLESIZE 101
# define NOTFOUND  (-1)

struct  { char  fakename[64];
          char  roguenam[64]; } dbase[TABLESIZE];

int datalen = 0;

/* 
 * findentry: find the database entry for 'string'
 */

findentry (string)
char *string;
{ register int i;

  for (i = 0; i < datalen; i++)
    if (streq (dbase[i].fakename, string) ||
        *dbase[i].roguenam && streq (dbase[i].roguenam, string))
      return (i);

  return (NOTFOUND);
}

/* 
 * useobj: Indicate that we have used (i.e. read, quaffed, or zapped) an
 *         object with name 'oldname'.
 */

useobj (oldname)
char *oldname;
{ if (findentry (oldname) == NOTFOUND)
  { strcpy (dbase[datalen].fakename, oldname);
    strcpy (dbase[datalen++].roguenam, "");
  }
}

/* 
 * infername: Note that we now think that the object named 'oldname' is
 * really named 'name' (e.g. scroll 'google plex' is really a scroll of
 * light).
 */

infername (oldname, name)
char *oldname;
char *name;
{ register int i;

  i = findentry (oldname);

  if (i == NOTFOUND)
  { strcpy (dbase[datalen].fakename, oldname);
    strcpy (dbase[datalen++].roguenam, name);
  }
  else
  { if (*dbase[i].roguenam && strcmp (dbase[i].roguenam, name))
      dwait (D_ERROR, "Inconsistent inference '%s', '%s'", 
             dbase[i].roguenam, name);
    else
      strcpy (dbase[i].roguenam, name);
    }
}

/* 
 * used: Return true if we have marked 'oldname' as used.
 */

int used (oldname)
char *oldname;
{ register int i;

  for (i = 0; i < datalen; i++)
    if (streq (dbase[i].fakename, oldname))
      return (TRUE);

  return (FALSE);
}

/* 
 * know: Return true if we know what the fake name for 'name' is.
 */

int know (name)
char *name;
{ register int i;

  for (i = 0; i < datalen; i++)
    if (*dbase[i].roguenam && streq (dbase[i].roguenam, name))
      return (TRUE);

  return (FALSE);
}

/* 
 * realname: Returns the real name of an object nmed 'oldname'.
 */

char *realname (oldname)
char *oldname;
{ register int i;

  for (i = 0; i < datalen; i++)
    if (*dbase[i].roguenam && streq (dbase[i].fakename, oldname))
      return (dbase[i].roguenam);

  return ("");
}

/* 
 * dumpdatabase: Debugging, dump the database one the screen.
 */

dumpdatabase ()
{ register int i;

  for (i = 0; i < datalen; i++)
  { at (i+1, 0);
    printw ("%-32s '%s'", dbase[i].roguenam, dbase[i].fakename);
  }
}
