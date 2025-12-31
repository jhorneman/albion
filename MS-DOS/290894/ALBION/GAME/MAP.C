/************
 * NAME     : MAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 10-8-94
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>
#include <BBMEM.H>
#include <BBDSA.H>

#include <XLOAD.H>

#include <ALBION.H>
#include <CONTROL.H>
#include <MAP.H>
#include <GAME.H>
#include <2D_MAP.H>
#include <3D_MAP.H>
#include <ANIMATE.H>
#include <XFTYPES.H>

/* global variables */

MEM_HANDLE Map_handle;

struct Map_data *Map_ptr;

SISHORT _3D_map;
UNSHORT Map_type;
UNSHORT Palette_nr;
UNSHORT Map_width, Map_height, Map_size;

UNLONG Event_automap_offset;
UNLONG Goto_point_offset;
UNLONG NPC_path_offset;
UNLONG Event_data_offset;
UNLONG Event_entry_offset;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_map
 * FUNCTION  : Initialize a new map
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.08.94 13:30
 * LAST      : 10.08.94 13:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_map(void)
{
	MEM_HANDLE Pal_handle;
	struct NPC_data *NPC_ptr;
	UNLONG Offset;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Load map */
	Map_handle = XLD_Load_subfile(&Xftypes[MAP_DATA], PARTY_DATA.Map_nr);
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);

	/* Get data from map */
	if (Map_ptr->Display_type == 1)
		_3D_map = TRUE;
	else
		_3D_map = FALSE;

	Map_type = (Map_ptr->Flags & MAP_TYPE) >> MAP_TYPE_B;

	Palette_nr = (UNSHORT) Map_ptr->Colour_pal_nr;
	Map_width = (UNSHORT) Map_ptr->Width;
	Map_height = (UNSHORT) Map_ptr->Height;

	Map_size = Map_width * Map_height;

	MEM_Free_pointer(Map_handle);

	/* Calculate data offsets */
	Ptr = (UNBYTE *) Map_ptr;
	Offset = sizeof(struct Map_data) + (3 * Map_size);

	if (Offset % 2)
		Offset++;

	Event_entry_offset = Offset;

	for (i=0;i<=Map_height;i++)
		Offset += *((UNSHORT *)(Ptr + Offset)) + 2;

	i = *((UNSHORT *)(Ptr + Offset));
	Offset += 2;

	Event_data_offset = Offset;

	Offset += (i * sizeof(struct Event_block));

	NPC_path_offset = Offset;

	NPC_ptr = (struct NPC_data *) ((UNBYTE *) Map_ptr
	 + sizeof(struct Map_data));

	for (i=0;i<NPCS_PER_MAP;i++)
	{
		if (NPC_ptr->Number)
		{
			if (((NPC_ptr->Flags & MOVEMENT_TYPE) >> MOVEMENT_TYPE_B)
			 == PATH_MOVEMENT)
				Offset += STEPS_MAX * 2;
			else
				Offset += 2;
		}
		NPC_ptr++;
	}

	if (_3D_map)
	{
		i = *((UNSHORT *)(Ptr + Offset));
		Offset += 2;

		Goto_point_offset = Offset;

		Offset += (i * sizeof(struct Goto_point));

		Event_automap_offset = Offset;
	}

	/* Load map palette */
	Pal_handle = XLD_Load_subfile(&Xftypes[PALETTE], Palette_nr);

	Ptr = MEM_Claim_pointer(Pal_handle);
	for (i=0;i<192;i++)
	{
		Palette.color[i].red = *Ptr++;
		Palette.color[i].green = *Ptr++;
 		Palette.color[i].blue = *Ptr++;
	}

	MEM_Free_pointer(Pal_handle);
	MEM_Free_memory(Pal_handle);

	DSA_SetPal(&Screen, &Palette, 0, 192, 0);
	DSA_ActivatePal(&Screen);

	/* Start the map display */
	if (_3D_map)
		Push_module(&M3_Mod);
	else
		Push_module(&M2_Mod);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_map
 * FUNCTION  : Leave the current map.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 11:45
 * LAST      : 11.08.94 11:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_map(void)
{
	Exit_display();
	Pop_module();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Colour_cycle_forward
 * FUNCTION  : Cycle colours forward
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 13:00
 * LAST      : 28.07.94 13:00
 * INPUTS    : UNSHORT Start - Number of first colour (0...255).
 *             UNSHORT Size - Number of colours to cycle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Colour_cycle_forward(UNSHORT Start, UNSHORT Size)
{
	struct BBCOLOR *Ptr, Last;
	UNSHORT i;

	Ptr = &Palette.color[Start];

	/* Save the last colour in the range */
	Last.red = Ptr[Size-1].red;
	Last.green = Ptr[Size-1].green;
	Last.blue = Ptr[Size-1].blue;

	/* Copy all colours forward */
	for (i=Size-1;i>0;i--)
	{
		Ptr[i].red = Ptr[i-1].red;
		Ptr[i].green = Ptr[i-1].green;
		Ptr[i].blue = Ptr[i-1].blue;
	}

	/* Insert last colour at the start */
	Ptr[0].red = Last.red;
	Ptr[0].green = Last.green;
	Ptr[0].blue = Last.blue;

	/* Update the palette */
	DSA_SetPal(&Screen, &Palette, Start, Size, Start);
}

