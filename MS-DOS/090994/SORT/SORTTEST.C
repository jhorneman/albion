/************
 * NAME     : SORTTEST.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 23-7-1994
 * PROJECT  : Test program for generic sort functions
 * SEE ALSO : SORT.C
 ************/

/* includes */

#include <BBDEF.H>
#include <SORT.H>

#include <stdio.h>
#include <stdlib.h>

void main(void);
void Swap(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare(UNLONG A, UNLONG B, UNBYTE *Data);

#define NUMBER (50)

void
main(void)
{
	UNSHORT i,j;
	UNLONG Data[NUMBER];

	printf("\nTesting sort-function...\n");

	for (j=0;j<100;j++)
	{
		printf("Generating... ");
		for (i=0;i<NUMBER;i++)
			Data[i]=(rand() & 0xFF);

		printf("sorting ");
		if (j%2)
		{
			printf("(shellsort)... ");
			Shellsort(Swap,Compare,NUMBER,(UNBYTE *) Data);
		}
		else
		{
			printf("(shuttlesort)... ");
			Shuttlesort(Swap,Compare,NUMBER,(UNBYTE *) Data);
		}

		printf("checking... ");
		for(i=0;i<NUMBER-1;i++)
			if (Data[i]>Data[i+1])
			{
				printf("ERROR!\n");
				break;
			}

		printf("OK.\n");
	}
}

void
Swap(UNLONG A, UNLONG B, UNBYTE *Data)
{
	UNLONG T, *P;

	P = (UNLONG *) Data;

	T = P[A];
	P[A] = P[B];
	P[B] = T;
}

BOOLEAN
Compare(UNLONG A, UNLONG B, UNBYTE *Data)
{
	UNLONG *P;

	P = (UNLONG *) Data;

	return(P[A] > P[B]);
}

