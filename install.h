/*
 * install.h: Rog-O-Matic XIV (CMU) Thu Jul  3 15:32:49 1986 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * This file contains (hopefully) all system dependent defines 
 * This version of Rog-O-Matic runs with Rogue version 5.3. 
 */

/*
 * BSD42: Defined for 4.2bsd Unix systems
 */

/* # define BSD42 */

/*
 * Rog-O-Matic's best score against Rogue 5.3  (UTexas, Feb 1984)
 * Printed when no score file is available in dumpscore()
 */

# define BEST		(11316)

/* 
 * Rog-O-Matic will not try to beat scores higher than BOGUS which
 * appear on the Rogue scoreboard.
 */

# define BOGUS		(50000)

/* 
 * This variable defines the version of Rogue we are assumed to be playing
 * if the getrogueversion() routine can't figure it out.  This must be
 * defined, and can be either "5.2", "3.6", or "5.3"
 */

# define DEFVER		"5.3"

/* 
 * This file is created whenever the Rog-O-Matic score file is accessed to
 * prevent simulatneous accesses. This variable must be defined, but will
 * not be used unless RGMDIR is also defined.
 */

# define LOCKFILE	"/tmp/Rgm Lock"

/* 
 * This variable is the level at which we always start logging the game
 */

# define GOODGAME	(18)

/* 
 * This variable is the number of seconds after which a LOCKFILE is
 * considered to be invalid.  This is necessary to avoid requiring manual
 * intervention when Rog-O-Matic dies while the score file is open.
 * This variable must be defined, but will not be used unless RGMDIR
 * is also defined.
 */

# define MAXLOCK	(120 /* seconds */)

/* 
 * This variable defines the "local" copy of Rogue, which may have only
 * intermittent access.  This was useful at CMU, since Rogue 5.3 was not
 * supported on all machines.  First Rog-O-Matic looks for "rogue" in the
 * current directory, then this file is used.  This variable need not be
 * defined.
 */

#undef NEWROGUE

/* 
 * This is the location of the player executable, which is the main
 * process for Rog-O-Matic.  If "player" does not exist in the current
 * directory, then this file is used. This variable need not be defined 
 * (but in that case there must be a "player" binary in the current 
 * directory).
 */

#undef PLAYER

/* 
 * This is the version of the "current" Rog-O-Matic, and is an uppercase
 * Roman numeral.  It must be defined.
 */

# define RGMVER		"XIV"

/* 
 * This is the standard system version of Rogue, and is used if "rogue"
 * and NEWROGUE are not available. It need not be defined, but if it is
 * not, and NEWROGUE is not defined, then there must be a "rogue" in the
 * current directory.
 */

#undef ROGUE

/* 
 * This file is created in the current directory if the logging option is
 * enabled.  If the game terminates normally, this file is renamed to
 * <killer>.<level>.<score>.  This variable must be defined.
 */

# define ROGUELOG	"roguelog"

/* 
 * This directory must be defined.  It will contain logs of Rogomatic's
 * scores, an error.log file, and the long term memory file.  It must
 * be writeable by everyone, since score files must be created and
 * destroyed by anyone running the program.  Alternatively, the
 * player process could be made setuid, with that uid owning this
 * directory.
 */

# define RGMDIR		"."

/* 
 * This file is created in the current directory if the snapshot command
 * is typed during a Rogue game.  It must be defined.
 */

# define SNAPSHOT	"snapshot.rgm"
