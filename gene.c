/*
 * gene.c: Rog-O-Matic XIV (CMU) Sat Jul  5 23:47:33 1986 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * Initialize and summarize the gene pool
 *
 * EDITLOG
 *	LastEditDate = Sat Jul  5 23:47:33 1986 - Michael Mauldin
 *	LastFileName = /usre3/mlm/src/rog/ver14/gene.c
 *
 * HISTORY
 *  5-Jul-86  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created.
 */

# include <stdio.h>
# include <stdlib.h>
# include "types.h"
# include "install.h"

int knob[MAXKNOB];
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

char genelock[100];
char genelog[100];
char genepool[100];

main (argc, argv)
int   argc;
char *argv[];
{ int m=10, init=0, seed=0, version=RV53A, full=0;

  /* Get the options */
  while (--argc > 0 && (*++argv)[0] == '-')
  { while (*++(*argv))
    { switch (**argv)
      { when 'a':	full=2;
	when 'i':	init++;
	when 'f':	full=1;
	when 'm':	m = atoi(*argv+1); SKIPARG;
			printf ("Gene pool size %d.\n", m);
	when 's':	seed = atoi(*argv+1); SKIPARG;
			printf ("Random seed %d.\n", m);
	when 'v':	version = atoi(*argv+1); SKIPARG;
			printf ("Rogue version %d.\n", version);
	otherwise:	quit (1,
			  "Usage: gene [-if] [-msv<value>] [genepool]\n");
      }
    }
  }

  if (argc > 0)
  { if (readgenes (argv[0]))		/* Read the gene pool */
      analyzepool (full);		/* Print a summary */
    else
      fprintf (stderr, "Cannot read file '%s'\n", argv[0]);

    exit (0);
  }

  /* No file argument, assign the gene log and pool file names */
  sprintf (genelock, "%s/GeneLock%d", RGMDIR, version);
  sprintf (genelog, "%s/GeneLog%d", RGMDIR, version);
  sprintf (genepool, "%s/GenePool%d", RGMDIR, version);

  critical ();				/* Disable interrupts */
  if (lock_file (genelock, MAXLOCK))
  { if (init) 
    { srand (seed);			/* Set the random number generator */
      openlog (genelog);		/* Open the gene log file */
      initpool (MAXKNOB, m);		/* Random starting point */
      writegenes (genepool);		/* Write out the gene pool */
      closelog ();			/* Close the log file */
    }
    else if (! readgenes (genepool))	/* Read the gene pool */
      quit (1, "Cannot read file '%s'\n", genepool);

   unlock_file (genelock);
  }
  else
    quit (1, "Cannot access file '%s'\n", genepool);

  uncritical ();			/* Re-enable interrupts */
  analyzepool (full);			/* Print a summary */
}
