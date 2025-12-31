/************
 * NAME     : SORT.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 23-7-1994
 * PROJECT  : Generic sort functions
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : SORT.H
 ************/

/* includes */

#include <BBDEF.H>
#include <SORT.H>

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Shuttlesort
 * FUNCTION  : Generic shuttlesort
 * FILE      : SORT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 14:53
 * LAST      : 23.07.94 14:53
 * INPUTS    : Swap_function Swap - Pointer to swap function.
 *             Compare_function Compare - Pointer to compare function.
 *             UNLONG Number - Number of elements to be sorted.
 *             UNBYTE *Data - Pointer to data for swap and compare functions.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will NOT change the order of identical
 *              elements.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Shuttlesort(Swap_function Swap, Compare_function Compare, UNLONG Number,
 UNBYTE *Data)
{
	UNLONG L;
	SILONG PS;

	for (L=0;L<Number;L++)
	{
		if ((*Compare)(L,L+1,Data))
		{
			(*Swap)(L,L+1,Data);
			PS = L-1;
			while (PS>=0)
			{
				if (!((*Compare)(PS,PS+1,Data)))
					break;
				(*Swap)(PS,PS+1,Data);
				PS--;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Shellsort
 * FUNCTION  : Generic shellsort
 * FILE      : SORT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 14:02
 * LAST      : 23.07.94 14:02
 * INPUTS    : Swap_function Swap - Pointer to swap function.
 *             Compare_function Compare - Pointer to compare function.
 *             UNLONG Number - Number of elements to be sorted.
 *             UNBYTE *Data - Pointer to data for swap and compare functions.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function is quicker than Shuttlesort, but WILL change
 *              the order of identical elements.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Shellsort(Swap_function Swap, Compare_function Compare, UNLONG Number,
 UNBYTE *Data)
{
	UNLONG Inc, L;
	SILONG PS;

	Inc = Number;
	while (Inc>1)
	{
		Inc = Inc/2;
		for (L=0;L<Number-Inc;L++)
		{
			if ((*Compare)(L,L+Inc,Data))
			{
				(*Swap)(L,L+Inc,Data);
				PS = L-Inc;
				while (PS>=0)
				{
					if ((*Compare)(PS,PS+Inc,Data))
					{
						(*Swap)(PS,PS+Inc,Data);
						PS = PS-Inc;
					}
					else
						PS = -1;
				}
			}
		}
	}
}

