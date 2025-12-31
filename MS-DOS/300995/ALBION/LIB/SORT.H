/************
 * NAME     : SORT.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 23-7-1994
 * PROJECT  : Generic sort functions
 * NOTES    :
 * SEE ALSO : SORT.C, SORTTEST.C
 ************/

/************

 To use SORT.OBJ you must write two functions :

 1 - The Swap function, which swaps elements A and B.

	INPUTS    : UNLONG A - Index of first element (0...).
	            UNLONG B - Index of second element (0...).
	            UNBYTE *Data - Pointer to data passed by Sort() caller.
	RESULT    : None.

	void
	Swap(UNLONG A, UNLONG B, UNBYTE *Data)
	{
		T = Data[A];
		Data[A] = Data[B];
		Data[B] = T;
	}

 2 - The Compare function, which compares elements A and B.

	INPUTS    : UNLONG A - Index of first element (0...).
	            UNLONG B - Index of second element (0...).
	            UNBYTE *Data - Pointer to data passed by Sort() caller.
	RESULT    : TRUE or FALSE.

	BOOLEAN
	Compare(UNLONG A, UNLONG B, UNBYTE *Data)
	{
		return(Data[A] > Data[B]);
	}

 (This compare function would result in a set which starts with the lowest
 element.)

 J.

 *************/

#ifndef SORT_H
#define SORT_H

/*
 ** Defines ****************************************************************
 */

typedef void (*Swap_function)(UNLONG, UNLONG, UNBYTE *);
typedef BOOLEAN (*Compare_function)(UNLONG, UNLONG ,UNBYTE *);

/*
 ** Prototypes *************************************************************
 */

void Shellsort(Swap_function Swap, Compare_function Compare, UNLONG Number,
 UNBYTE *Data);
void Shuttlesort(Swap_function Swap, Compare_function Compare, UNLONG Number,
 UNBYTE *Data);

#endif

