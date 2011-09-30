/*
 * stats.c: Rog-O-Matic XIV (CMU) Fri Dec 28 23:28:59 1984 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 * 
 * A package for maintaining probabilities and statistics.
 *
 * Functions:
 *
 *    A probability is a simple count of Bernoulli trials.
 *
 *	clearprob:	Clear a probability.
 *	addprob:	Add success/failure to a probability.
 *	prob:		Calculate p(success) of a statistic.
 *	parseprob:	Parse a probability from a string.
 *	writeprob:	Write a probability to a file.
 *
 *    A statistic is a random variable with a mean and stdev.
 *
 *	clearstat:	Clear a statistic.
 *	addstat:	Add a data point to a statistic.
 *	mean:		Calculate the mean of a statistic.
 *	stdev:		Calculate the std. dev. of a statistic.
 *	parsestat:	Parse a statistic from a string.
 *	writestat:	Write a statistic to a file.
 */

# include <stdio.h>
# include <math.h>
# include "types.h"

/* 
 * clearprob: zero a probability structure.
 */

clearprob (p)
register  probability *p;
{ p->fail = p->win = 0;
}

/* 
 * addprob: Add a data point to a probability
 */

addprob (p, success)
register probability *p;
register int success;
{ 
  if (success)	p->win++;
  else		p->fail++;
}

/* 
 * prob: Calculate a probability
 */

double prob (p)
register probability *p;
{ register int trials = p->fail + p->win;

  if (trials < 1)	return (0.0);
  else			return ((double) p->win / trials);
}

/* 
 * parseprob: Parse a probability structure from buffer 'buf'
 */

parseprob (buf, p)
register char *buf;
register probability *p;
{ p->win = p->fail = 0;
  sscanf (buf, "%d %d", &p->fail, &p->win);
}

/* 
 * writeprob. Write the value of a probability structure to file 'f'.
 */

writeprob (f, p)
register FILE *f;
register probability *p;
{ fprintf (f, "%d %d", p->fail, p->win);
}

/* 
 * clearstat: zero a statistic structure.
 */

clearstat (s)
register  statistic * s;
{ s->count = 0;
  s->sum = s->sumsq = s->low = s->high = 0.0;
}

/* 
 * addstat: Add a data point to a statistic
 */

addstat (s, datum)
register statistic *s;
register int datum;
{ double d = (double) datum;

  s->count++;
  s->sum += d;
  s->sumsq += d*d;

  if (s->count < 2)	s->low = s->high = d;
  else if (d < s->low)	s->low = d;
  else if (d > s->high)	s->high = d;
}

/* 
 * mean: Return the mean of a statistic
 */

double mean (s)
register statistic *s;
{
  if (s->count < 1)	return (0.0);
  else			return (s->sum / s->count);
}

/* 
 * stdev: Return the standard deviation of a statistic
 */

double stdev (s)
register statistic *s;
{ register n = s->count;

  if (n < 2)	return (0.0);
  else		return (sqrt ((n * s->sumsq - s->sum * s->sum) / (n * (n-1))));
}

/* 
 * parsestat: Parse a statistic structure from buffer 'buf'
 */

parsestat (buf, s)
register char *buf;
register statistic *s;
{ s->count = 0;
  s->sum = s->sumsq = s->low = s->high = 0.0;
  sscanf (buf, "%d %lf %lf %lf %lf",
      &s->count, &s->sum, &s->sumsq, &s->low, &s->high);
}

/* 
 * writestat. Write the value of a statistic structure to file 'f'.
 */

writestat (f, s)
register FILE *f;
register statistic *s;
{ fprintf (f, "%d %lg %lg %lg %lg",
           s->count, s->sum, s->sumsq, s->low, s->high);
}
