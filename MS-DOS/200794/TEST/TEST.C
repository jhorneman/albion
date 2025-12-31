/************
 * NAME     : TEST.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 8-6-1994
 * PROJECT  :
 * NOTES    :
 * SEE ALSO : BBMEM.H
 ************/

/* includes */

#include <BBDEF.H>
#include <stdio.h>
#include <string.h>
#include <BBMEM.H>
#include <TEST.H>

UNBYTE Mem1[200004],Mem2[400004],Counter = 2;

void main(void);
void Check(void);

MEM_HANDLE hx[10];

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : main
 * FUNCTION  : Test the memory manager.
 * FILE      : TEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.06.94 13:35
 * LAST      : 08.06.94 13:35
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
main(void)
{
	UNLONG Size, *p, l;
	UNSHORT i;
	struct Memory_workspace *Second;

	MEM_Init_memory();

	MEM_Check_memory();
	MEM_List();

	for (i=0;i<8;i++)
	{
		if (i==3)
		{
			printf("Creating a second workspace.\n");
			Second = MEM_Add_workspace(300000,0xFF);
			MEM_Push_workspace(Second);
		}

		Size = ((i%9)+1)*10000;

		printf("Allocating %lu bytes.\n",Size),
		hx[i] = MEM_Allocate_memory(Size);

		p = (UNLONG *) MEM_Claim_pointer(hx[i]);
		l = MEM_Get_block_size(hx[i]);
		*p = (UNLONG) hx[i];
		*(p+l/4-1) = (UNLONG) hx[i];
		MEM_Free_pointer(hx[i]);

		MEM_Check_memory();
		MEM_List();
		Check();

		if (i>3)
		{
			printf("Freeing memory.\n");
			if (i<7)
				MEM_Push_workspace(NULL);
			MEM_Free_memory(hx[i-4]);
			MEM_Check_memory();
			MEM_List();
			hx[i-4] = NULL;
			Check();
			if (i<7)
				MEM_Pop_workspace();
		}
	}

  MEM_Exit_memory();
}

void Check()
{
	UNSHORT i;
	UNLONG *p,l;

	for (i=0;i<10;i++)
	{
		if (hx[i])
		{
			p = (UNLONG *) MEM_Claim_pointer(hx[i]);
			l = MEM_Get_block_size(hx[i]);
			if ((*p != (UNLONG) hx[i]) || (*(p+l/4-1) != (UNLONG) hx[i]))
				printf("Error on handle %lu.\n",hx[i]);
			MEM_Free_pointer(hx[i]);
		}
	}
}

void BASE_Get_memory(struct Memory_entry *Area)
{
	UNBYTE *Start;
	UNLONG Size;

	switch (Counter)
	{
	case 2:
		Start = &Mem1[0];
  		Size = 200004;
		Counter = 1;
		break;
	case 1:
		Start = &Mem2[0];
		Size = 400004;
		Counter = 0;
		break;
	case 0:
		Size = 0;
		break;
	}

/* Initialize memory area */
	Area->Start = Start;
	Area->Size = Size;
	Area->MEMORY_TYPE = 0x01;
}

void BASE_Free_memory(struct Memory_entry *Area)
{
	Area->Size = 0;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Fill_memory
 * FUNCTION  : Fill memory with a byte value.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 16:20
 * LAST      : 10.06.94 16:20
 * INPUTS    : UNBYTE *Start - Pointer to memory.
 *             UNLONG Size - Size of memory block.
 *             UNBYTE Value - Fill value.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This is a general function which has no direct relationship
 *              with the memory manager.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Fill_memory(UNBYTE *Start, UNLONG Size, UNBYTE Value)
{
	UNLONG i;

	/* Fill */
	for (i=0;i<Size;i++)
		*Start++ = Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Fill_memory_long
 * FUNCTION  : Fill memory with a longword value.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.06.94 17:32
 * LAST      : 11.06.94 17:32
 * INPUTS    : UNBYTE *Start - Pointer to memory.
 *             UNLONG Size - Size of memory block.
 *             UNLONG Value - Fill value.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This is a general function which has no direct relationship
 *              with the memory manager.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Fill_memory_long(UNBYTE *Start, UNLONG Size, UNLONG Value)
{
	UNLONG i, *p;

	/* Fill */
	p = (UNLONG *) Start;
	Size /= 4;
	for (i=0;i<Size;i++)
		*p++ = Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XXX_Copy_memory
 * FUNCTION  : Copy a block of memory from one location to another.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 16:20
 * LAST      : 10.06.94 16:20
 * INPUTS    : UNBYTE *Source - Pointer to source.
 *             UNBYTE *Target - Pointer to target.
 *             UNLONG Size - Size of memory block.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This is a general function which has no direct relationship
 *              with the memory manager.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XXX_Copy_memory(UNBYTE *Source, UNBYTE *Target, UNLONG Size)
{
/*	UNLONG i; */

	memmove((void *) Target,(void *) Source,(size_t) Size);

/*	if (Source != Target)
	{*/
		/* Is the source address smaller than the target address ? */
		/* if (Source < Target)
		{*/
			/* Yes -> Is the target address in the source area ? */
			/*if (Target < (Source + Size))
			{*/
				/* Yes -> Copy backwards */
				/*Source += Size;
				Target += Size;
				for (i=0;i<Size;i++)
					*(--Target) = *(--Source);
			}
			else
			{*/
				/* No -> Copy forwards */
				/*for (i=0;i<Size;i++)
					*Target++ = *Source++;
			}
		}
		else
		{*/
			/* No -> Copy forwards */
			/*for (i=0;i<Size;i++)
				*Target++ = *Source++;
		}
	}*/
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Align
 * FUNCTION  : Align the size or address of a memory block.
 * FILE      : BBMEMALO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.06.94 10:05
 * LAST      : 01.06.94 10:05
 * INPUTS    : UNLONG Value - a size or address.
 * RESULT    : UNLONG : Properly aligned value.
 * BUGS      : No known.
 * NOTES		 : - This function ensures that all memory blocks are aligned to
 *					 a value most practical to the host platform.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
MEM_Align(UNLONG Value)
{
	return((Value+ALIGNMENT-1) & (0-ALIGNMENT));
}

