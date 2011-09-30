/*
 * main.c: Rog-O-Matic XIV (CMU) Sat Mar  7 12:48:37 1987 - mlm
 */

/*=========================================================================
 * Rog-O-Matic XIV
 * Automatically exploring the dungeons of doom
 * Copyright (C) 1985 by Appel, Jacobson, Hamey, and Mauldin
 *
 * The right is granted to any person, university, or company 
 * to copy, modify, or distribute (for free) these files,
 * provided that any person receiving a copy notifies Michael Mauldin
 *
 * (1) by electronic mail to	Mauldin@CMU-CS-A.ARPA		or
 *
 * (2) by US Mail to		Michael Mauldin
 *				Dept. of Computer Science
 *				Carnegie-Mellon University
 *				Pittsburgh, PA  15213
 *
 * All other rights, including those of publication and sale, are reserved.
 *========================================================================/

/*****************************************************************
 * EDITLOG
 *	LastEditDate = Sat Mar  7 12:48:36 1987 - Michael Mauldin
 *	LastFileName = /usre3/mlm/src/rog/ver14/main.c
 *
 * History:     I.    Andrew Appel & Guy Jacobson, 10/81 [created]
 *              II.   Andrew Appel & Guy Jacobson, 1/82  [added search]
 *              III.  Michael Mauldin, 3/82              [added termcap]
 *              IV.   Michael Mauldin, 3/82              [searching]
 *              V.    Michael Mauldin, 4/82              [cheat mode]
 *              VI.   Michael Mauldin, 4/82              [object database]
 *              VII.  All three, 5/82                    [running away]
 *              VIII. Michael Mauldin, 9/82              [improved cheating]
 *              IX.   Michael Mauldin, 10/82             [replaced termcap]
 *              X.    Mauldin, Hamey,  11/82             [Fixes, Rogue 5.2]
 *              XI.   Mauldin,  11/82                    [Fixes, Score lock]
 *              XII.  Hamey, Mauldin,  06/83             [Fixes, New Replay]
 *              XIII. Mauldin, Hamey,  11/83             [Fixes, Rogue 5.3]
 *              XIV.  Mauldin          01/85             [Fixes, UT mods]
 *
 * General:
 *
 * This is the main routine for the player process, which decodes the
 * Rogue output and sends commands back. This process is execl'd by the
 * rogomatic process (cf. setup.c) which also execl's the Rogue process,
 * conveniently connecting the two via two pipes.
 *
 * Source Files:
 *
 *      arms.c          Armor, Weapon, and Ring handling functions
 *      command.c       Effector interface, sends cmds to Rogue
 *      database.c      Memory for objects "discovered"
 *      debug.c         Contains the debugging functions
 *      explore.c       Path searching functions, exploration
 *      findscore.c     Reads Rogue scoreboard
 *      io.c            I/O functions, Sensory interface
 *      main.c          Main Program for 'player' (this file)
 *      mess.c          Handles messages from Rogue
 *      monsters.c      Monster handling utilities
 *      mover.c         Creates command strings to accomplish moves
 *      rooms.c         Room specific functions, new levels
 *      scorefile.c     Score file handling utilities
 *      search.c        Does shortest path
 *      setup.c         Main program for 'rogomatic'
 *      strategy.c      Makes high level decisions
 *      survival.c      Find cycles and places to run to
 *      tactics.c       Medium level intelligence
 *      things.c        Builds commands, part of Effector interface
 *      titlepage.c     Prints the animated copyright notice
 *      utility.c       Miscellaneous Unix (tm) functions
 *      worth.c         Evaluates the items in the pack
 *
 * Include files:
 *
 *      globals.h       External defs for all global variables
 *      install.h       Machine dependent DEFINES
 *      termtokens.h    Defines various tokens to/from Rogue
 *      types.h         Global DEFINES, macros, and typedefs.
 *
 * Other files which may be included with your distribution include
 *
 *      rplot           A shell script, prints a scatter plot of Rog's scores.
 *      rgmplot.c       A program used by rplot.
 *      datesub.l       A program used by rplot.
 *      histplot.c      A program which plots a histogram of Rgm's scores.
 *
 * Acknowledgments
 *
 *	The UTexas modifications included in this distribution
 *	came from Dan Reynolds, and are included by permission.
 *	Rog-O-Matics first total winner against version 5.3 was
 *	on a UTexas computer.
 *****************************************************************/

# include <termios.h>
# include <curses.h>
# include <ctype.h>
# include <signal.h>
# include <setjmp.h>  
# include <string.h>
# include <stdlib.h>
# include "types.h"
# include "termtokens.h"
# include "install.h"

/* global data - see globals.h for current definitions */

/* Files */
FILE  *fecho=NULL;		/* Game record file 'echo' option */
FILE  *frogue=NULL;		/* Pipe from Rogue process */
FILE  *logfile=NULL;		/* File for score log */
FILE  *realstdout=NULL;		/* Real stdout for Emacs, terse mode */
FILE  *snapshot=NULL;		/* File for snapshot command */
FILE  *trogue=NULL;		/* Pipe to Rogue process */

/* Characters */
char  logfilename[100];		/* Name of log file */
char  afterid = '\0';           /* Letter of obj after identify */
char  genelock[100];		/* Gene pool lock file */
char  genelog[100];		/* Genetic learning log file */
char  genepool[100];		/* Gene pool */
char  *genocide;		/* List of monsters to be genocided */
char  genocided[100];		/* List of monsters genocided */
char  lastcmd[64];		/* Copy of last command sent to Rogue */
char  lastname[64];		/* Name of last potion/scroll/wand */
char  nextid = '\0';            /* Next object to identify */
char  screen[24][80];		/* Map of current Rogue screen */
char  sumline[128];		/* Termination message for Rogomatic */
char  ourkiller[64];		/* How we died */
char  versionstr[32];		/* Version of Rogue being used */
char  *parmstr;			/* Pointer to process arguments */

/* Integers */
int   aggravated = 0;		/* True if we have aggravated this level */
int   agoalc = NONE;		/* Goal square to arch from (col) */
int   agoalr = NONE;		/* Goal square to arch from (row) */
int   arglen = 0;		/* Length in bytes of argument space */
int   ammo = 0;                 /* How many missiles? */
int   arrowshot = 0;		/* True if an arrow shot us last turn */
int   atrow, atcol;		/* Current position of the Rogue (@) */
int   atrow0, atcol0;		/* Position at start of turn */
int   attempt = 0;		/* Number times we searched whole level */
int   badarrow = 0;		/* True if cursed/lousy arrow in hand */
int   beingheld = 0;		/* True if a fungus has ahold of us */
int   beingstalked = 0;		/* True if recently hit by inv. stalker */
int   blinded = 0;		/* True if blinded */
int   blindir = 0;		/* Last direction we moved when blind */
int   cancelled = 0;		/* True ==> recently zapped w/cancel */
int   cecho = 0;		/* Last kind of message to echo file */
int   cheat = 0;		/* True ==> cheat, use bugs, etc. */
int   checkrange = 0;           /* True ==> check range */
int   chicken = 0;		/* True ==> check run away code */
int   compression = 1;		/* True ==> move more than one square/turn */
int   confused = 0;		/* True if we are confused */
int   cosmic = 0;		/* True if we are hallucinating */
int   currentarmor = NONE;	/* Index of our armor */
int   currentweapon = NONE;     /* Index of our weapon */
int   cursedarmor = 0;		/* True if our armor is cursed */
int   cursedweapon = 0;		/* True if we are wielding cursed weapon */
int   darkdir = NONE;		/* Direction of monster being arched */
int   darkturns = 0;		/* Distance to monster being arched */
int   debugging = D_NORMAL;	/* Debugging options in effect */
int   didreadmap = 0;		/* Last level we read a map on */
int   doorlist[40];		/* List of doors on this level */
int   doublehasted = 0; 	/* True if double hasted (Rogue 3.6) */
int   droppedscare = 0;		/* True if we dropped 'scare' on this level */
int   emacs = 0;		/* True ==> format output for Emacs */
int   exploredlevel = 0;	/* We completely explored this level */
int   floating = 0;		/* True if we are levitating */
int   foughtmonster = 0;	/* True if recently fought a monster */
int   foundarrowtrap = 0;	/* Found arrow trap this level */
int   foundtrapdoor = 0;	/* Found trap door this level */
int   goalc = NONE;		/* Current goal square (col) */
int   goalr = NONE;		/* Current goal square (row) */
int   goodarrow = 0;		/* True if good (magic) arrow in hand */
int   goodweapon = 0;		/* True if weapon in hand worth >= 100 */
int   gplusdam = 1;		/* Our plus damage from strength */
int   gplushit = 0;		/* Our plus to hit from strength */
int   hasted = 0;		/* True if hasted */
int   hitstokill = 0;		/* # times we hit last monster killed */
int   interrupted = 0;		/* True if at commandtop from onintr() */
int   knowident = 0;            /* Found an identify scroll? */
int   larder = 1;               /* How much food? */
int   lastate = 0;		/* Time we last ate */
int   lastdamage = 0;           /* Amount of last hit by a monster */
int   lastdrop = NONE;		/* Last object we tried to drop */
int   lastfoodlevel = 1;	/* Last level we found food */
int   lastmonster = NONE;	/* Last monster we tried to hit */
int   lastobj = NONE;		/* What did we last use */
int   lastwand = NONE;		/* Index of last wand */
int   leftring = NONE;		/* Index of our left ring */
int   logdigested = 0;		/* True if log file has been read by replay */
int   logging = 0;		/* True if keeping record of game */
int   lyinginwait = 0;          /* True if we waited for a monster */
int   maxobj = 22;              /* How much can we carry */
int   missedstairs = 0;         /* True if we searched everywhere */
int   morecount = 0;            /* Number of messages since last command */
int   msgonscreen = 0;		/* Set implies message at top */
int   newarmor = 1;             /* Change in armor status? */
int  *newdoors = NULL;		/* New doors on screen */
int   newring = 1;              /* Change in ring status? */
int   newweapon = 1;            /* Change in weapon status? */
int   nohalf = 0;		/* True ==> no halftime show */
int   noterm = 0;		/* True ==> no user watching */
int   objcount = 0;             /* Number of objects */
int   ourscore = 0;		/* Final score when killed */
int   playing = 1;		/* True if still playing game */
int   poorarrow = 0;		/* True if arrow has missed */
int   protected = 0;		/* True if we protected our armor */
int   putonseeinv = 0;          /* Turn when last put on see inv ring */
int   quitat = BOGUS;		/* Score to beat, quit if within 10% more */
int   redhands = 0;		/* True if we have red hands */
int   replaying = 0;		/* True if replaying old game */
int   revvideo = 0;		/* True if in rev. video mode */
int   rightring = NONE;		/* Index of our right ring */
int   rogpid = 0;		/* Pid of rogue process */
int   room[9];			/* Flags for each room */
int   row, col;			/* Current cursor position */
int   scrmap[24][80];		/* Flags bits for level map */
int   singlestep = 0;		/* True ==> go one turn */
int   slowed = 0;		/* True ==> recently zapped w/slow monster */
int   stairrow, staircol;	/* Position of stairs on this level */
int   startecho = 0;		/* True ==> turn on echoing on startup */
int   teleported = 0;		/* # times teleported this level */
int   terse = 0;		/* True ==> terse mode */
int   transparent = 0;		/* True ==> user command mode */
int   trapc = NONE;		/* Location of arrow trap, this level (col) */
int   trapr = NONE;		/* Location of arrow trap, this level (row) */
int   urocnt = 0;               /* Un-identified Rogue Object count */
int   usesynch = 0;             /* Set when the inventory is correct */
int   usingarrow = 0;		/* True ==> wielding an arrow froma trap */
int   version;			/* Rogue version, integer */
int   wplusdam = 2;		/* Our plus damage from weapon bonus */
int   wplushit = 1;		/* Our plus hit from weapon bonus */
int   zone = NONE;		/* Current screen zone, 0..8 */
int   zonemap[9][9];		/* Map of zones connections */

/* Functions */
void (*istat)(int);
void onintr (int);
char getroguetoken (), *getname();
FILE *openlog();

/* Stuff list, list of objects on this level */
stuffrec slist[MAXSTUFF]; 	int slistlen=0;

/* Monster list, list of monsters on this level */
monrec mlist[MAXMONST];		int mlistlen=0;

char targetmonster = '@';	/* Monster we are arching at */

/* Monster attribute and Long term memory arrays */
attrec monatt[26];		/* Monster attributes */
lrnrec ltm;			/* Long term memory -- general */
ltmrec monhist[MAXMON];		/* Long term memory -- creatures */
int nextmon = 0;		/* Length of LTM */
int monindex[27];		/* Index into monhist array */

/* Genetic learning parameters (and defaults) */
int geneid = 0;		/* Id of genotype */
int genebest = 0;	/* Best score of genotype */
int geneavg = 0;	/* Average score of genotype */
int k_srch =	50;	/* Propensity for searching for traps */
int k_door =	50;	/* Propensity for searching for doors */
int k_rest =	50;	/* Propensity for resting */
int k_arch =	50;	/* Propensity for firing arrows */
int k_exper =	50;	/* Level*10 on which to experiment with items */
int k_run =	50;	/* Propensity for retreating */
int k_wake =	50;	/* Propensity for waking things up */
int k_food =	50;	/* Propensity for hoarding food (affects rings) */
int knob[MAXKNOB] = {50, 50, 50, 50, 50, 50, 50, 50};
char *knob_name[MAXKNOB] = {
	"trap searching:   ",
	"door searching:   ",
	"resting:          ",
	"using arrows:     ",
	"experimenting:    ",
	"retreating:       ",
	"waking monsters:  ",
	"hoarding food:    "
};
/* Door search map */
char timessearched[24][80], timestosearch;
int  searchstartr = NONE, searchstartc = NONE, reusepsd=0;
int  new_mark=1, new_findroom=1, new_search=1, new_stairs=1, new_arch=1;

/* Results of last call to makemove() */
int  ontarget= 0, targetrow= NONE, targetcol= NONE;

/* Rog-O-Matics model of his stats */
int   Level = 0, MaxLevel = 0, Gold = 0, Hp = 12, Hpmax = 12;
int   Str = 16, Strmax = 16, Ac = 6, Exp = 0, Explev = 1, turns = 0;
char  Ms[30];	/* The message about his state of hunger */

/* Miscellaneous movement tables */
int   deltrc[8] = { 1,-79,-80,-81,-1,79,80,81 };
int   deltc[8]  = { 1, 1, 0, -1, -1, -1, 0, 1 };
int   deltr[8]  = { 0, -1, -1, -1, 0, 1, 1, 1 };
char  keydir[8] = { 'l', 'u', 'k', 'y', 'h', 'b', 'j', 'n' };
int   movedir;

/* Map characters on screen into object types */
stuff translate[128] =
{    /* \00x */  none, none, none, none, none, none, none, none,
     /* \01x */ none, none, none, none, none, none, none, none,
     /* \02x */ none, none, none, none, none, none, none, none,
     /* \03x */ none, none, none, none, none, none, none, none,
     /* \04x */ none, potion, none, none, none, none, none, none,
     /* \05x */ hitter, hitter, gold, none, amulet, none, none, wand,
     /* \06x */ none, none, none, none, none, none, none, none,
     /* \07x */ none, none, food, none, none, ring, none, scroll,
     /* \10x */ none, none, none, none, none, none, none, none,
     /* \11x */ none, none, none, none, none, none, none, none,
     /* \12x */ none, none, none, none, none, none, none, none,
     /* \13x */ none, none, none, armor, none, armor, none, none,
     /* \14x */ none, none, none, none, none, none, none, none,
     /* \15x */ none, none, none, none, none, none, none, none,
     /* \16x */ none, none, none, none, none, none, none, none,
     /* \17x */ none, none, none, none, none, none, none, none
};

/* Inventory, contents of our pack */
invrec inven[MAXINV]; int invcount = 0;

/* Time history */
timerec timespent[50];

/* End of the game messages */
char *termination = "perditus";
char *gamename = "Rog-O-Matic";
char roguename[41] = "Rog-O-Matic                             ";

/* Used by onintr() to restart Rgm at top of command loop */
jmp_buf  commandtop;

/*
 * Main program
 */

main (argc, argv)
int   argc;
char *argv[];
{ char  ch, *s, *getenv(), *statusline(), msg[128];
  int startingup = 1;
  register int  i;
  struct termios termios;

  tcgetattr(fileno(stdin),&termios);

  /*
   * Initialize some storage
   */
  
  sprintf (genocided, "");
  sprintf (lastcmd, "i");
  sprintf (ourkiller, "unknown");
  sprintf (sumline, "");
  sprintf (versionstr, "");
  for (i = 80 * 24; i--; ) screen[0][i] = ' ';
 
  /* 
   * The first argument to player is a two character string encoding
   * the file descriptors of the pipe ends. See setup.c for call.
   *
   * If we get 'ZZ', then we are replaying an old game, and there
   * are no pipes to read/write.
   */

  if (argv[1][0] == 'Z')
  { replaying = 1;
    gamename = "Iteratum Rog-O-Maticus";
    termination = "finis";
    strcpy (logfilename, argv[4]);
    startreplay (&logfile, logfilename);
  }
  else
  { frogue = fdopen (argv[1][0] - 'a', "r");
    trogue = fdopen (argv[1][1] - 'a', "w");
    setbuf (trogue, (char *) NULL);
  }

  /* The second argument to player is the process id of Rogue */
  if (argc > 2) rogpid = atoi (argv[2]);                  

  /* The third argument is an option list */
  if (argc > 3) sscanf (argv[3], "%d,%d,%d,%d,%d,%d,%d,%d", 
			&cheat, &noterm, &startecho, &nohalf,
			&emacs, &terse, &transparent, &quitat);

  /* The fourth argument is the Rogue name */
  if (argc > 4)	strcpy (roguename, argv[4]);
  else		sprintf (roguename, "Rog-O-Matic %s", RGMVER);

  /* Now count argument space and assign a global pointer to it */
  arglen = 0;
  for (i=0; i<argc; i++)
  { register int len = strlen (argv[i]);
    arglen += len + 1;
    while (len >= 0) argv[i][len--] = ' ';
  }
  parmstr = argv[0];	arglen--;

  /* If we are in one-line mode, then squirrel away stdout */
  if (emacs || terse)
  { realstdout = fdopen (dup (fileno (stdout)), "w");
    freopen ("/dev/null", "w", stdout);
  }

  initscr (); crmode (); noecho ();	/* Initialize the Curses package */
  if (startecho) toggleecho ();		/* Start logging? */
  clear ();				/* Clear the screen */
  getrogver ();				/* Figure out Rogue version */

  if (!replaying)
  { restoreltm ();			/* Get long term memory of version */ 
    startlesson ();			/* Start genetic learning */
  }

  /* 
   * Give a hello message
   */

  if (replaying)
    sprintf (msg, " Replaying log file %s, version %s.", 
	     logfilename, versionstr);
  else
    sprintf (msg, " %s: version %s, genotype %d, quit at %d.",
	     roguename, versionstr, geneid, quitat);
  
  if (emacs)
  { fprintf (realstdout, "%s  (%%b)", msg); fflush (realstdout); }
  else if (terse)
  { fprintf (realstdout, "%s\n", msg); fflush (realstdout); }
  else
  { saynow (msg); }
  sendnow(";");
  getrogue (ill, 2);  /* Read the input up to end of first command */

  /* 
   * Now that we have the version figured out, we can properly
   * interpret the screen.  Force a redraw by sending a redraw
   * screen command (^L for old, ^R for new).
   *
   * Also identify wands (/), so that we can differentiate
   * older Rogue 3.6 from Rogue 3.6 with extra magic...
   */

  if (version < RV53A)
    sendnow (":%c//;", ctrl('l'));
  else
    sendnow (":%c;", ctrl('r'));

  /* 
   * If we are not replaying an old game, we must position the
   * input after the next form feed, which signals the start of
   * the level drawing.
   */
  
  if (!replaying)
    while ((int) (ch = GETROGUECHAR) != CL_TOK && (int) ch != EOF);

  clearscreen();

  /* 
   * Note: If we are replaying, the logfile is now in synch
   */

  getrogue (ill, 2);  /* Read the input up to end of first command */
  
  /* Identify all 26 monsters */
  if (!replaying)
    for (ch = 'A'; ch <= 'Z'; ch++) send ("/%c", ch);

  /*
   * Signal handling. On an interrupt, Rogomatic goes into transparent
   * mode and clears what state information it can. This code is styled
   * after that in "UNIX Programming -- Second Edition" by Brian
   * Kernigan & Dennis Ritchie. I sure wouldn't have thought of it.
   */

  istat = signal (SIGINT, SIG_IGN); /* save original status */
  setjmp (commandtop);              /* save stack position */
  if (istat != SIG_IGN)
    signal (SIGINT, onintr);

  if (interrupted)
  { saynow ("Interrupt [enter command]:");
    interrupted = 0;
    transparent = 1;
  }
  
  if (transparent) noterm = 0;

  while (playing) 
  { refresh ();

    /* If we have any commands to send, send them */
    while (resend ())
    { if (startingup) showcommand (lastcmd);
      sendnow (";"); getrogue (ill, 2);
    }
    
    if (startingup)		/* All monsters identified */
    { versiondep ();			/* Do version specific things */
      startingup = 0;			/* Clear starting flag */
    }
    
    if (!playing) break;	/* In case we died */

    /*
     * No more stored commands, so either get a command from the
     * user (if we are in transparent mode or the user has typed
     * something), or let the strategize module try its luck. If
     * strategize fails we wait for the user to type something. If
     * there is no user (noterm mode) then use ROGQUIT to signal a
     * quit command.
     */

    if ((transparent && !singlestep) ||
	(!emacs && charsavail ()) ||
        !strategize())
    { ch = (noterm) ? ROGQUIT : getch ();

      switch (ch)
      { case '?': givehelp (); break;
      
        case '\n': if (terse) 
	           { printsnap (realstdout); fflush (realstdout); }
	           else
                   { singlestep = 1; transparent = 1; }
		   break;
	           
        /* Rogue Command Characters */
        case 'H': case 'J': case 'K': case 'L':
        case 'Y': case 'U': case 'B': case 'N':
        case 'h': case 'j': case 'k': case 'l':
        case 'y': case 'u': case 'b': case 'n':
        case 's': command (T_OTHER, "%c", ch); transparent = 1; break;

        case 'f': ch = getch ();
                  for (s = "hjklyubnHJKLYUBN"; *s; s++)
                  { if (ch == *s)
                    { if (version < RV53A) command (T_OTHER, "f%c", ch); 
		      else                 command (T_OTHER, "%c", ctrl (ch)); 
		    }
                  }
                  transparent = 1; break;

        case '\f':  redrawscreen (); break;

        case 'm':   dumpmonstertable (); break;

        case 'M':   dumpmazedoor (); break;

        case '>': if (atrow == stairrow && atcol == staircol) 
                    command (T_OTHER, ">");
                  transparent = 1; break;

        case '<': if (atrow == stairrow && atcol == staircol &&
                      have (amulet) != NONE) command (T_OTHER, "<");
                  transparent = 1; break;

        case 't': transparent = !transparent; break;

        case ')': new_mark++; markcycles (DOPRINT); at (row, col); break;

        case '+': setpsd (DOPRINT); at (row, col); break;

        case 'A': attempt = (attempt+1) % 5;
		  saynow ("Attempt %d", attempt); break;

        case 'G': mvprintw (0, 0,
               "%d: Sr %d Dr %d Re %d Ar %d Ex %d Rn %d Wk %d Fd %d, %d/%d",
		  geneid, k_srch, k_door, k_rest, k_arch,
		  k_exper, k_run, k_wake, k_food, genebest, geneavg);
		  clrtoeol (); at (row, col); refresh (); break;

        case ':': chicken = !chicken;
                  say (chicken ? "chicken" : "aggressive");
                  break;

        case '~': if (replaying)
		    saynow ("Replaying log file %s, version %s.", 
			    logfilename, versionstr);
		  else
		    saynow (" %s: version %s, genotype %d, quit at %d.",
			    roguename, versionstr, geneid, quitat);
                  break;

        case '[': at (0,0);
                  printw ("%s = %d, %s = %d, %s = %d, %s = %d.",
                     "hitstokill", hitstokill,
                     "goodweapon", goodweapon,
                     "usingarrow", usingarrow,
                     "goodarrow", goodarrow);
                  clrtoeol ();
                  at (row, col);
                  refresh ();
                  break;

        case '-': saynow (statusline ());
                  break;

        case '`': clear ();
                  summary ((FILE *) NULL, '\n');
                  pauserogue ();
                  break;

        case '|': clear ();
                  timehistory ((FILE *) NULL, '\n');
                  pauserogue ();
                  break;

        case 'r': resetinv (); say ("Inventory reset."); break;

        case 'i': clear (); dumpinv ((FILE *) NULL); pauserogue (); break;

        case '/': dosnapshot ();
                  break;

        case '(': clear (); dumpdatabase (); pauserogue (); break;

        case 'c': cheat = !cheat;
                  say (cheat ? "cheating" : "righteous");
                  break;

        case 'd': toggledebug ();	break;

        case 'e': toggleecho ();        break;

        case '!': dumpstuff ();         break;

        case '@': dumpmonster ();       break;

        case '#': dumpwalls ();         break;

        case '%': clear (); havearmor (1, DOPRINT, ANY); pauserogue (); break;

        case ']': clear (); havearmor (1, DOPRINT, RUSTPROOF);
		  pauserogue (); break;

        case '=': clear (); havering (1, DOPRINT); pauserogue (); break;

        case '$': clear (); haveweapon (1, DOPRINT); pauserogue (); break;

        case '^': clear (); havebow (1, DOPRINT); pauserogue (); break;

        case '{': promptforflags (); break;

        case '&': saynow ("Object count is %d.", objcount); break;

        case '*': blinded = !blinded;
                  saynow (blinded ? "blinded" : "sighted");
                  break;

        case 'C': cosmic = !cosmic;
                  saynow (cosmic ? "cosmic" : "boring");
                  break;

        case 'E': dwait (D_ERROR, "Testing the ERROR trap..."); break;

        case 'F': dwait (D_FATAL, "Testing the FATAL trap..."); break;

        case 'R': if (replaying)
		  { positionreplay (); getrogue (ill, 2);
	            if (transparent) singlestep = 1; }
		  else
                    saynow ("Replay position only works in replay mode.");
                  break;

        case 'S': quitrogue ("saved", Gold, SAVED); 
                  playing = 0; break;

        case 'Q': quitrogue ("user typing quit", Gold, FINISHED); 
                  playing = 0; break;

        case ROGQUIT: dwait (D_ERROR, "Strategize failed, gave up.");
                      quitrogue ("gave up", Gold, SAVED); break;
      }
    }
    else
    { singlestep = 0;
    }
  }
  
  if (! replaying)
  { saveltm (Gold);			/* Save new long term memory */
    endlesson ();			/* End genetic learning */  
  }

  /* Print termination messages */
  at (23, 0); clrtoeol (); refresh ();
  endwin (); nocrmode (); noraw (); echo ();
  
  if (emacs)  
  { if (*sumline) fprintf (realstdout, " %s", sumline);
  }
  else if (terse)  
  { if (*sumline) fprintf (realstdout, "%s\n",sumline);
    fprintf (realstdout, "%s %s est.\n", gamename, termination);
  }
  else
  { if (*sumline) printf ("%s\n",sumline);
    printf ("%s %s est.\n", gamename, termination);
  }

  /* 
   * Rename log file, if it is open
   */

  if (logging)
  { char lognam[128];

    /* Make up a new log file name */
    sprintf (lognam, "%0.4s.%d.%d", ourkiller, MaxLevel, ourscore);

    /* Close the open file */
    toggleecho ();

    /* Rename the log file */
    if (link (ROGUELOG, lognam) == 0)
    { unlink (ROGUELOG);
      printf ("Log file left on %s\n", lognam);
    }
    else
      printf ("Log file left on %s\n", ROGUELOG);
  }
  termios.c_lflag |= ECHO | ICANON;
  termios.c_iflag |= ICRNL;
  termios.c_oflag |= ONLCR;
  tcsetattr(fileno(stdin),TCSANOW,&termios);

  exit (0);
}

/*
 * onintr: The SIGINT handler. Pass interrupts to main loop, setting
 * transparent mode. Also send some synchronization characters to Rogue,
 * and reset some goal variables.
 */

void
onintr (int s)
{ sendnow ("n\033");            /* Tell Rogue we don't want to quit */
  if (logging) fflush (fecho);  /* Print out everything */
  refresh ();                   /* Clear terminal output */
  clearsendqueue ();            /* Clear command queue */
  setnewgoal ();                /* Don't believe ex */
  transparent = 1;              /* Drop into transprent mode */
  interrupted = 1;              /* Mark as an interrupt */
  noterm = 0;                   /* Allow commands */
  longjmp (commandtop, 1);      /* Back to command Process */
}

/*
 * startlesson: Genetic learning algorithm, pick a genotype to
 * test this game, and set the parameters (or "knobs") accordingly.
 */

startlesson ()
{ sprintf (genelog, "%s/GeneLog%d", RGMDIR, version);
  sprintf (genepool, "%s/GenePool%d", RGMDIR, version);
  sprintf (genelock, "%s/GeneLock%d", RGMDIR, version);

  srand (0);				/* Start random number generator */
  critical ();				/* Disable interrupts */

  /* Serialize access to the gene pool */
  if (lock_file (genelock, MAXLOCK))	/* Lock the gene pool */
  { if (openlog (genelog) == NULL)	/* Open the gene log file */
      saynow ("Could not open file %s", genelog);
    if (! readgenes (genepool))		/* Read the gene pool */
      initpool (MAXKNOB, 20);		/* Random starting point */
    setknobs (&geneid, knob, &genebest, &geneavg); /* Select a genotype */
    writegenes (genepool);		/* Write out the gene pool */
    closelog ();			/* Close the gene log file */
    unlock_file (genelock);		/* Unlock the gene pool */
  }
  else
    fprintf (stderr, "Cannot lock gene pool to read '%s'\n", genepool);

  uncritical ();			/* Reenable interrupts */

  /* Cache the parameters for easier use */
  k_srch = knob[K_SRCH];	k_door = knob[K_DOOR];
  k_rest = knob[K_REST];	k_arch = knob[K_ARCH];
  k_exper = knob[K_EXPER];	k_run = knob[K_RUN];
  k_wake = knob[K_WAKE];	k_food = knob[K_FOOD];
}

/*
 * endlesson: if killed, total winner, or quit for scoreboard,
 * evaluate the performance of this genotype and save in genepool.
 */

endlesson ()
{ if (geneid > 0 &&
      (stlmatch (termination, "perditus") ||
       stlmatch (termination, "victorius") ||
       stlmatch (termination, "callidus")))
  { critical ();			/* Disable interrupts */

    if (lock_file (genelock, MAXLOCK))	/* Lock the score file */
    { openlog (genelog);		/* Open the gene log file */
      if (readgenes (genepool))		/* Read the gene pool */
      { evalknobs (geneid,Gold,Level);	/* Add the trial to the pool */
        writegenes (genepool); }	/* Write out the gene pool */
      closelog ();
      unlock_file (genelock);		/* Disable interrupts */
    }
    else 
      fprintf (stderr, "Cannot lock gene pool to evaluate '%s'\n", genepool);

    uncritical ();			/* Re-enable interrupts */
  }
}
