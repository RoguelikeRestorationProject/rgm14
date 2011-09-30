/*
 * command.c: Rog-O-Matic XIV (CMU) Tue Mar 19 20:52:19 1985 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * This file contains all of the functions which send commands to
 * Rogue, this file and 'things.c' make up the effector interface.
 */

# include <curses.h>
# include <ctype.h>
# include <string.h>
# include "types.h"
# include "globals.h"

# define EQUAL 0

static int cmdonscreen = 0, comcount = 0;

/* Move one square in direction 'd' */
move1 (d)
int   d;
{ command (T_MOVING, "%c", keydir[d]);
}

/* Move in direction 'd' until we find something */
fmove (d)
int   d;
{ if (version < RV53A)	command (T_MOVING, "f%c", keydir[d]);
  else			command (T_MOVING, "%c", ctrl (keydir[d]));
}

/* Move 'count' squares in direction 'd', with time use mode 'mode' */
rmove (count, d, mode)
int   count, d, mode;
{ command (mode, "%d%c", count, keydir[d]);
}

/* Move one square in direction 'd' without picking anything up */
mmove (d, mode)
int   d, mode;
{ command (mode, "m%c", keydir[d]);
}

/*
 * command: Send a command which takes Rogue time to execute. These
 * include movement commands, sitting, and physical actions. Actions which
 * gather information are sent to Rogue using the 'send' function.
 */

/* VARARGS2 */
command (int tmode, char *f, ...)
{ va_list ap;
  int times;
  char cmd[128], functionchar (); 
  static char lastcom[32] = "";

  /* Build the command */
  va_start (ap, f);
  vsprintf (cmd, f, ap);
  va_end (ap);

  /* Echo the command if in transparent mode */
  if (transparent)		showcommand (cmd);
  else if (cmdonscreen)		clearcommand ();

  /* Figure out whether and in which direction we are moving */
  switch ((functionchar (cmd) & 037) | 0100)
  { case 'L': movedir = 0; wakemonster (movedir); break;
    case 'U': movedir = 1; wakemonster (movedir); break;
    case 'K': movedir = 2; wakemonster (movedir); break;
    case 'Y': movedir = 3; wakemonster (movedir); break;
    case 'H': movedir = 4; wakemonster (movedir); break;
    case 'B': movedir = 5; wakemonster (movedir); break;
    case 'J': movedir = 6; wakemonster (movedir); break;
    case 'N': movedir = 7; wakemonster (movedir); break;
    default:  movedir = NOTAMOVE;
  }
  
  /* If in a real game (not replaying), then check for looping */
  if (!replaying)
  { if (streq (lastcom, cmd))
    { comcount++;
      if (streq (cmd, "i") && comcount > 3)
        dwait (D_FATAL, "command: cannot synchronize inventory, invcount %d.",
               invcount);
    }
    else
    { strcpy (lastcom, cmd);
      comcount = 1;
    }
  }

  /* If command takes time to execute, mark monsters as sleeping */
  /* If they move, wakemonsters will mark them as awake */
  if (tmode != T_OTHER)
    sleepmonster ();

  /* Do time accounting */
  times = commandcount (cmd);
  if (tmode < T_OTHER || tmode >= T_LISTLEN) tmode = T_OTHER;
  turns += times;
  timespent[Level].timestamp = turns;
  timespent[Level].activity[tmode] += times > 1 ? times : 1;

  /* Do the inventory stuff */
  if (movedir == NOTAMOVE)
    adjustpack (cmd);

  /* If we have a ring of searching, take that into account */
  if (wearing ("searching") != NONE)
    bumpsearchcount ();

  send (cmd);
}

/* 
 * commandcount: Return the number of a times a command is to happen.
 */

commandcount (cmd)
char *cmd;
{ register int times = atoi (cmd);

  return (max (times, 1));
}

/* 
 * functionchar: return the function character of a command.
 */

char
functionchar (cmd)
char *cmd;
{ register char *s = cmd;

  while (ISDIGIT (*s) || *s == 'f') s++;
  return (*s);
}

/* 
 * commandarg: return the nth argument of a command.
 */

char
commandarg (cmd, n)
char *cmd;
{ register char *s = cmd;

  while (ISDIGIT (*s) || *s == 'f') s++;
  return (s[n]);
}

/* 
 * adjustpack: adjust pack in accordance with command.
 */

adjustpack (cmd)
char *cmd;
{ char functionchar(), commandarg();
  int neww, obj;

  switch (functionchar (cmd))
  { case 'd':	setrc (STUFF | USELESS, atrow, atcol);
		deleteinv (OBJECT (commandarg (cmd, 1)));
		break;

    case 'e':   removeinv (OBJECT (commandarg (cmd, 1)));
                Ms[0] = 'X'; newring = 1;
		lastate = turns;
                break;

    case 'i':	doresetinv ();
		break;

    case 'q':	lastobj = OBJECT (commandarg (cmd, 1));
		usemsg ("Quaffing", lastobj);
		strcpy (lastname, inven[lastobj].str);
		useobj (inven[lastobj].str);
		removeinv (lastobj);
		break;

    case 'r':	lastobj = OBJECT (commandarg (cmd, 1));
		usemsg ("Reading", lastobj);
		strcpy (lastname, inven[lastobj].str);
		useobj (inven[lastobj].str);
		removeinv (lastobj);
		break;

    case 't':	removeinv (OBJECT (commandarg (cmd, 2)));
		hitstokill -= 1; /* Dont blame weapon if arrow misses */
		break;

    case 'w':	if (currentweapon != NONE)
		  forget (currentweapon, INUSE);

		neww = OBJECT (commandarg (cmd, 1));
		usemsg ("About to wield", neww);

		if (commandarg (cmd, 2) == 'w')
		{ lastdrop = currentweapon = neww; }
		else
		{ lastdrop = currentweapon; currentweapon = neww; }

		remember (currentweapon, INUSE);
		
		usingarrow = (inven[currentweapon].type == missile);
		goodweapon = (weaponclass (currentweapon) >= 100);
		
		badarrow = goodarrow = poorarrow = hitstokill = 0;
		newweapon = 1;
		setbonuses ();
		break;

    case 'p': case 'z':
		lastwand = OBJECT (commandarg (cmd, 2));
		usemsg ("Pointing", lastwand);
		strcpy (lastname, inven[lastwand].str);
		useobj (inven[lastwand].str);

		/* Update number of charges */
		if (inven[lastwand].charges > 0) 
		{ if (version >= RV52A &&
		      stlmatch (inven[lastwand].str, "striking"))
		  inven[lastwand].charges -= 2;
		else
		  inven[lastwand].charges--;
		}
  
		hitstokill -= 1; /* Dont blame weapon if wand misses */
		break;

    case 's':   bumpsearchcount ();
		break;

    case 'P':	obj = OBJECT (commandarg (cmd, 1));
		usemsg ("Putting on", obj);

		if (commandarg (cmd, 2) == 'l')		leftring = obj;
		else if (commandarg (cmd, 2) == 'r')	rightring = obj;
		else if (leftring == NONE)		leftring = obj;
		else					rightring = obj;

		/* Check for putting on see invisible */
		if (streq (inven[obj].str, "see invisible"))
		{ beingstalked = 0; putonseeinv = turns; }

		remember (obj, INUSE);
		setbonuses ();
		newarmor = 1;

		break;

    case 'R':	if (commandarg (cmd, 1) == 'l')
		{ lastdrop = leftring; leftring = NONE; }
		else if (commandarg (cmd, 1) == 'r')
		{ lastdrop = rightring; rightring = NONE; }
		else if (leftring != NONE)
		{ lastdrop = leftring; leftring = NONE; }
		else
		{ lastdrop = rightring; rightring = NONE; }

		usemsg ("Taking off", lastdrop);

  		forget (lastdrop, INUSE);
		setbonuses ();
		newarmor = 1;

		break;

    case 'T':   lastdrop = currentarmor;
		usemsg ("About to take off", currentarmor);
                forget (currentarmor, INUSE);
                currentarmor = NONE;
                newarmor = 1;
		break;

    case 'W':	currentarmor = OBJECT (commandarg (cmd, 1));
		usemsg ("About to wear", currentarmor);
		remember (currentarmor, INUSE);
                newarmor = 1;
		break;
  }
}

/* 
 * bumpsearchcount: Note that we just searched this square.
 */

bumpsearchcount ()
{ register int dr, dc;
  for (dr = -1; dr <= 1; dr++)
    for (dc = -1; dc <= 1; dc++)
      timessearched[atrow+dr][atcol+dc]++;
}

/* 
 * replaycommand: Find the old command in the log file and send it.
 */

replaycommand ()
{ char oldcmd[128];

  getoldcommand (oldcmd);
  command (T_OTHER, oldcmd); 
  return (1); 
}

/* 
 * showcommand:		Echo a string in the lower right hand corner.
 * clearcommand:	Remove the command we showed.
 */

showcommand (cmd)
char *cmd;
{ register char *s;
  at (23,72); standout (); printw (" ");
  for (s=cmd; *s; s++) printw ("%s", unctrl (*s));
  printw (" "); standend (); clrtoeol (); at (row, col); refresh ();
  cmdonscreen = 1;
}

clearcommand ()
{ at (23,72); clrtoeol (); at (row, col);
  cmdonscreen = 0;
}
/*
 * usemsg: About to use an item, tell the user.
 */

usemsg (str, obj)
char *str;
int obj;
{ if (! dwait (D_INFORM, "%s (%s", str, itemstr (obj)))
    saynow ("%s (%s", str, itemstr (obj));
}

