/*
 * datesub.c: Rog-O-Matic XIV (CMU) Sun Aug 31 11:33:22 2008 - njk
 * Copyright (C) 2008 by Nicholas J. Kisseberth
 */

#include <stdio.h>

int
main(int argc, char *argv[])
{
	int i;
	char buf[1024];
	char *month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
			"Sep", "Oct", "Nov", "Dec" };

	while(!feof(stdin) && fgets(buf,1024,stdin) != NULL)
	{
		for(i = 0; i < 12 && strncmp(buf,month[i],3) != 0; i++)
			;

		if (i < 12)
			fprintf(stdout, "%d%s", i+1, buf+3);
		else
			fputs(buf, stdout);
	}
}

