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
	UNLONG l;
	SILONG ps;

	/* Any elements ? */
	if (Number > 1)
	{
		/* Yes -> Sort */
		for (l=0;l<Number-1;l++)
		{
			if ((*Compare)(l, l + 1, Data))
			{
				(*Swap)(l, l + 1, Data);
				ps = l - 1;
				while (ps >= 0)
				{
					if (!((*Compare)(ps, ps + 1, Data)))
						break;
					(*Swap)(ps, ps + 1, Data);
					ps--;
				}
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
	UNLONG inc, l;
	SILONG ps;

	/* Any elements ? */
	if (Number > 1)
	{
		/* Yes -> Sort */
		inc = Number;
		while (inc > 1)
		{
			inc = inc / 2;
			for (l=0;l<Number-inc;l++)
			{
				if ((*Compare)(l, l + inc, Data))
				{
					(*Swap)(l, l + inc, Data);
					ps = l - inc;
					while (ps >= 0)
					{
						if ((*Compare)(ps, ps + inc, Data))
						{
							(*Swap)(ps, ps + inc, Data);
							ps = ps - inc;
						}
						else
							ps = -1;
					}
				}
			}
		}
	}
}

