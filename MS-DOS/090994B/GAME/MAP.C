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
#include <GAMEVAR.H>
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

SISHORT Offsets4[4][2] = {
	{0,-1}, {1,0}, {0,1}, {-1,0}
};
SISHORT Offsets8[8][2] = {
	{0,-1}, {1,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}
};

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

void
Yog_sototh(void)
{
	static UNSHORT XYX = 0;

	/* Do colour cycling */
	if (!XYX)
	{
		switch (Palette_nr)
		{
			case 1:
			case 2:
				Colour_cycle_forward(153,7);
				Colour_cycle_forward(176,5);
				Colour_cycle_forward(181,11);
				DSA_ActivatePal(&Screen);
				break;
			case 3:
				Colour_cycle_forward_3D(64,4);
				Colour_cycle_forward_3D(68,12);
				break;
			case 6:
				Colour_cycle_forward(176,5);
				Colour_cycle_forward(181,11);
				DSA_ActivatePal(&Screen);
				break;
		}

		Update_animation();
	}

	XYX++;

	if (XYX > 2)
		XYX = 0;
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

BOOLEAN
Before_move(void)
{
	return(TRUE);
}

void
After_move(void)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_location_status
 * FUNCTION  : Get the status of a location.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 16:47
 * LAST      : 06.09.94 16:47
 * INPUTS    : SISHORT X - X-coordinate in map (1...).
 *             SISHORT Y - Y-coordinate in map (1...).
 * RESULT    : UNLONG : Location status.
 * BUGS      : No known.
 * NOTES     : - The routine assumes no two NPC's can be in the same place.
 *             - The routine will output :
 *              the NPC icon status when an NPC is present, it's icon status
 *              <> 0 and the {Underlay priority}-bit is cleared.
 *              Otherwise it will output : the overlay icon status when an
 *              overlay is present and the {Underlay priority}-bit is cleared.
 *             - The routine doesn't check 3D objects.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_location_status(SISHORT X, SISHORT Y)
{
	struct Map_data *Map_data;
	UNLONG Under_status = 0, Over_status = 0, NPC_status = 0;
	UNSHORT i;

	if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
		return(0);

	Map_data = (struct Map_data *) MEM_Claim_pointer(Map_handle);

	if (_3D_map)
	{
		struct Lab_data *Lab_ptr;
		struct Square_3D *Map_ptr;
		struct Wall_3D *Wall_ptr;
		struct Overlay_3D *Overlay_ptr;
		UNBYTE Byte, *Ptr;

		Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

		Map_ptr = (struct Square_3D *)(Map_data + 1);
		Map_ptr += X-1 + ((Y-1) * Map_width);

		Byte = Map_ptr->Wall_layer;

		/* Empty or object group ? */
		if (Byte < FIRST_WALL)
		{
			/* Yes -> Use default status */
			Under_status = 0;
		}
		else
		{
			/* No -> Legal wall number ? */
			Byte -= FIRST_WALL;
			if (Byte > Nr_of_walls)
				/* No */
				Under_status = 0;
			else
			{
				/* Yes -> Find wall data */
				Ptr = (UNBYTE *) (Lab_ptr + 1)
				 + (Nr_of_object_groups * sizeof(struct Object_group_3D)) + 2
				 + (Nr_of_floors * sizeof(struct Floor_3D)) + 2
				 + (Nr_of_objects * sizeof(struct Object_3D)) + 2;
				Wall_ptr = (struct Wall_3D *) Ptr;

				for (i=0;i<Byte;i++)
				{
					/* Skip overlay data */
					Overlay_ptr = (struct Overlay_3D *) (Wall_ptr + 1);
					Overlay_ptr += Wall_ptr->Nr_overlays;
					Wall_ptr = (struct Wall_3D *) Overlay_ptr;
				}

				/* Get wall data */
				Under_status = Wall_ptr->Flags;
			}
		}
		MEM_Free_pointer(Lab_data_handle);

		/* No overlay */
		Over_status = 0;
	}
	else
	{
		struct Icon_2D_data *Icon_data_ptr;
		struct Square_2D *Map_ptr;
		UNSHORT B1,B2,B3;
		UNSHORT Underlay_nr, Overlay_nr;

		Icon_data_ptr = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[0]);

		Map_ptr = (struct Square_2D *)(Map_data + 1);
		Map_ptr += X-1 + ((Y-1) * Map_width);

		/* Read bytes from map */
		B1 = (UNSHORT) Map_ptr->m[0];
		B2 = (UNSHORT) Map_ptr->m[1];
		B3 = (UNSHORT) Map_ptr->m[2];

		/* Build overlay and underlay number */
		Underlay_nr = ((B2 & 0x0F) << 8) | B3;
		Overlay_nr = (B1 << 4) | (B2 >> 4);

		/* Any underlay ? */
		if (Underlay_nr > 1)
		{
			/* Yes -> Get icon data */
			Under_status = Icon_data_ptr[Underlay_nr-2].Flags;
		}

		/* Any overlay ? */
		if (Overlay_nr > 1)
		{
			/* Yes -> Get icon data */
			Over_status = Icon_data_ptr[Overlay_nr-2].Flags;
		}

		MEM_Free_pointer(Icondata_handle[0]);
	}

/*
; --------- Check NPC status ----------------------
.Check_NPC:
	moveq.l	#0,d2			; No NPC icon status
	cmpi.b	#World_2D,Current_map_type	; 2D wilderness ?
	beq.s	.End
	lea.l	VNPC_data,a0		; Check NPC's
	move.l	CD_value,d6
	moveq.l	#0,d7
.Loop:	tst.b	NPC_char_nr(a0)		; Anyone there ?
	beq.s	.Next
	btst	d7,d6			; Deleted	?
	bne.s	.Next
	cmp.w	Current_NPC,d7		; Is self ?
	beq.s	.Next
	cmp.w	VMap_X(a0),d4		; Right coordinates	?
	bne.s	.Next
	cmp.w	VMap_Y(a0),d5
	bne.s	.Next
	move.l	NPC_icon_status(a0),d2	; Get icon status
	bra.s	.End
.Next:	lea.l	VNPC_data_size(a0),a0	; Next NPC
	addq.w	#1,d7
	cmpi.w	#Max_chars,d7
	bmi.s	.Loop
.End:
*/

	MEM_Free_pointer(Map_handle);

	/* Is there an NPC with priority ? */
	if (NPC_status && (NPC_status & UNDERLAY_PRIORITY))
		/* Yes -> Use NPC status */
		return(NPC_status);

	/* Is there an overlay ? */
	if (Over_status)
	{
		/* Yes -> Does the underlay have priority ? */
		if ((Over_status & UNDERLAY_PRIORITY) ||
		 (((Under_status & ICON_HEIGHT) >> ICON_HEIGHT_B) > 1))
			/* Yes -> Use underlay status */
			return(Under_status);
		else
			/* No -> Use overlay status */
			return(Over_status);
	}
	else
		/* Use underlay status */
		return(Under_status);
}

