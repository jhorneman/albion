/************
 * NAME     : COLOURS.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 13-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>

#include <FINDCOL.H>

#include <COLOURS.H>

#include <GAMEVAR.H>
#include <CONTROL.H>
#include <XFTYPES.H>
#include <3D_PREP.H>

/* defines */

/* structure definitions */

/* prototypes */

void Protect_cycled_colours(UNSHORT Palette_nr);
void Unprotect_cycled_colours(UNSHORT Palette_nr);

/* global variables */

UNSHORT Current_palette_nr;

/* Palettes */
struct BBPALETTE Palette;
static struct BBPALETTE Backup_palette;

/* Fade parameters */
static UNSHORT Current_fade_R;
static UNSHORT Current_fade_G;
static UNSHORT Current_fade_B;
static UNSHORT Current_fade_percentage = 0;

/* Recolouring tables */
UNBYTE Recolour_tables[10][256];
UNBYTE Red_table[256];

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Initialize_palette
 * FUNCTION  : Initialize the palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.10.94 11:09
 * LAST      : 13.08.95 14:44
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will load and initialize the top 64 colours.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_palette(void)
{
	MEM_HANDLE Handle;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Clear */
	Current_palette_nr = 0;

	/* Clear bottom part of palette */
	for (i=0;i<192;i++)
	{
		Palette.color[i].red		= 0;
		Palette.color[i].green	= 0;
 		Palette.color[i].blue	= 0;
	}

	/* Load top part of palette */
	Handle = Load_file(BASEPAL);

	/* Copy colours to palette */
	Ptr = MEM_Claim_pointer(Handle);
	for (i=192;i<256;i++)
	{
		Palette.color[i].red		= *Ptr++;
		Palette.color[i].green	= *Ptr++;
 		Palette.color[i].blue	= *Ptr++;
	}

	/* Destroy palette file */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Initialize backup palette structure */
	Backup_palette.entries = 256;
	Backup_palette.version = 0;

	/* Update palette */
	Update_palette(0, 256);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_palette
 * FUNCTION  : Load a palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 16:31
 * LAST      : 13.08.95 18:09
 * INPUTS    : UNSHORT Palette_nr - Palette subfile number.
 * RESULT    : BOOLEAN : TRUE (success) or FALSE (failure).
 * BUGS      : No known.
 * NOTES     : - This function will load and initialize the bottom 192 colours.
 *             - The cycled colours are protected BEFORE the recolouring
 *              tables are recoloured.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Load_palette(UNSHORT Palette_nr)
{
	MEM_HANDLE Handle;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Load palette file */
	Handle = Load_subfile(PALETTE, Palette_nr);
	if (!Handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Un-protect cycled colours of old palette */
	Unprotect_cycled_colours(Current_palette_nr);

	/* Store number of new palette */
	Current_palette_nr = Palette_nr;

	/* Copy colours to palette */
	Ptr = MEM_Claim_pointer(Handle);
	for (i=0;i<192;i++)
	{
		Palette.color[i].red		= *Ptr++;
		Palette.color[i].green	= *Ptr++;
 		Palette.color[i].blue	= *Ptr++;
	}

	/* Destroy palette file */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Protect cycled colours */
	Protect_cycled_colours(Palette_nr);

	/* Update palette */
	Update_palette(0, 192);

	/* Should the palette be faded ? */
	if (Current_fade_percentage)
	{
		/* Yes -> Fade */
		Recolour_palette(0, 256, Current_fade_R, Current_fade_G,
		 Current_fade_B, Current_fade_percentage);
	}

	return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_palette
 * FUNCTION  : Save the palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.06.95 14:07
 * LAST      : 13.08.95 17:15
 * INPUTS    : UNSHORT Start - Number of first colour.
 *             UNSHORT Size - Number of colours to recolour.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Save_palette(UNSHORT Start, UNSHORT Size)
{
	UNSHORT i;

	/* Save the palette */
	for (i=Start;i<Start + Size;i++)
	{
		Backup_palette.color[i].red	= Palette.color[i].red;
		Backup_palette.color[i].green	= Palette.color[i].green;
		Backup_palette.color[i].blue	= Palette.color[i].blue;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Restore_palette
 * FUNCTION  : Restore the palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.06.95 14:08
 * LAST      : 17.06.95 14:08
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will call Restore_palette. Thus the restored
 *              palette will be activated and all recolouring tables are
 *              updated.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Restore_palette(void)
{
	UNSHORT i;

	/* Restore the palette */
	for (i=0;i<256;i++)
	{
		Palette.color[i].red		= Backup_palette.color[i].red;
		Palette.color[i].green	= Backup_palette.color[i].green;
		Palette.color[i].blue	= Backup_palette.color[i].blue;
	}

	/* Update palette */
	Update_palette(0, 256);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_palette
 * FUNCTION  : Update the palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 16:34
 * LAST      : 14.08.95 00:28
 * INPUTS    : UNSHORT Start - Number of first colour.
 *             UNSHORT Size - Number of colours to recolour.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will make sure the palette is activated and
 *              that all recolouring tables are updated.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_palette(UNSHORT Start, UNSHORT Size)
{
	/* Save palette */
	Save_palette(Start, Size);

	/* Prepare colour finding */
	Prepare_colour_find(&Palette, 0, 256);

	/* Calculate recolouring tables */
	Calculate_recolouring_table(&(Recolour_tables[0][0]), 0, 256,
	 0, 0, 0, 50);
	Calculate_recolouring_table(&(Recolour_tables[1][0]), 0, 256,
	 0, 0, 0, 40);
	Calculate_recolouring_table(&(Recolour_tables[2][0]), 0, 256,
	 0, 0, 0, 30);
	Calculate_recolouring_table(&(Recolour_tables[3][0]), 0, 256,
	 0, 0, 0, 20);
	Calculate_recolouring_table(&(Recolour_tables[4][0]), 0, 256,
	 0, 0, 0, 10);

	Calculate_recolouring_table(&(Recolour_tables[5][0]), 0, 256,
	 0xff, 0xff, 0xff, 10);
	Calculate_recolouring_table(&(Recolour_tables[6][0]), 0, 256,
	 0xff, 0xff, 0xff, 20);
	Calculate_recolouring_table(&(Recolour_tables[7][0]), 0, 256,
	 0xff, 0xff, 0xff, 30);
	Calculate_recolouring_table(&(Recolour_tables[8][0]), 0, 256,
	 0xff, 0xff, 0xff, 40);
	Calculate_recolouring_table(&(Recolour_tables[9][0]), 0, 256,
	 0xff, 0xff, 0xff, 50);

	Calculate_recolouring_table(&(Red_table[0]), 0, 256, 0xff, 0, 0, 50);

	/* Tell CONTROL to activate palette */
	Palette_has_changed = TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Protect_cycled_colours
 * FUNCTION  : Protect the cycled colours in a palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.08.95 14:47
 * LAST      : 13.08.95 14:47
 * INPUTS    : UNSHORT Palette_nr - Palette subfile number.
 * RESULT    : BOOLEAN : TRUE (success) or FALSE (failure).
 * BUGS      : No known.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Protect_cycled_colours(UNSHORT Palette_nr)
{
	/* Protect cycled colours */
	switch (Palette_nr)
	{
		case 1:
		case 2:
		{
			Add_protected_colours(153, 7);
			Add_protected_colours(176, 5);
			Add_protected_colours(181, 11);
			break;
		}
		case 6:
		{
			Add_protected_colours(176, 5);
			Add_protected_colours(181, 11);
			break;
		}
		case 26:
		{
			Add_protected_colours(180, 12);
			break;
		}
		case 31:
		{
			Add_protected_colours(16, 64);
			break;
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Unprotect_cycled_colours
 * FUNCTION  : Unprotect the cycled colours in a palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.08.95 14:47
 * LAST      : 13.08.95 14:47
 * INPUTS    : UNSHORT Palette_nr - Palette number.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Unprotect_cycled_colours(UNSHORT Palette_nr)
{
	/* Un-protect cycled colours */
	switch (Palette_nr)
	{
		case 1:
		case 2:
		{
			Remove_protected_colours(153, 7);
			Remove_protected_colours(176, 5);
			Remove_protected_colours(181, 11);
			break;
		}
		case 6:
		{
			Remove_protected_colours(176, 5);
			Remove_protected_colours(181, 11);
			break;
		}
		case 26:
		{
			Remove_protected_colours(180, 12);
			break;
		}
		case 31:
		{
			Remove_protected_colours(16, 64);
			break;
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Cycle_colours
 * FUNCTION  : Cycled colours in a palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.08.95 14:52
 * LAST      : 13.08.95 14:52
 * INPUTS    : UNSHORT Palette_nr - Palette number.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Cycle_colours(UNSHORT Palette_nr)
{
	/* Cycle colours */
	switch (Palette_nr)
	{
		case 1:
		case 2:
		{
			Colour_cycle_forward_2D(153, 7);
			Colour_cycle_forward_2D(176, 5);
			Colour_cycle_forward_2D(181, 11);
			break;
		}
		case 3:
		{
			Colour_cycle_forward_3D(64, 4);
			Colour_cycle_forward_3D(68, 12);
			break;
		}
		case 6:
		{
			Colour_cycle_forward_2D(176, 5);
			Colour_cycle_forward_2D(181, 11);
			break;
		}
		case 14:
		{
			Colour_cycle_forward_3D(176, 4);
			Colour_cycle_forward_3D(180, 12);
			break;
		}
		case 15:
		{
			Colour_cycle_forward_3D(88, 8);
			break;
		}
		case 25:
		{
			Colour_cycle_forward_3D(176, 4);
			Colour_cycle_forward_3D(180, 12);
			break;
		}
		case 26:
		{
			Colour_cycle_forward_2D(180, 4);
			Colour_cycle_forward_2D(184, 4);
			Colour_cycle_forward_2D(188, 4);
			break;
		}
		case 31:
		{
			Colour_cycle_forward_2D(16, 64);
			break;
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_recolouring_table
 * FUNCTION  : Calculate a recolouring table for the current palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.10.94 11:18
 * LAST      : 19.10.94 11:18
 * INPUTS    : UNBYTE *Table - Pointer to start (!) of recolouring table.
 *             UNSHORT Start - Number of first colour.
 *             UNSHORT Size - Number of colours to recolour.
 *             UNSHORT Target_R - Target red value.
 *             UNSHORT Target_G - Target green value.
 *             UNSHORT Target_B - Target blue value.
 *             UNSHORT Percentage - 0% : original colour,
 *              100% : target colour.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_recolouring_table(UNBYTE *Table, UNSHORT Start, UNSHORT Size,
 UNSHORT Target_R, UNSHORT Target_G, UNSHORT Target_B, UNSHORT Percentage)
{
	UNSHORT R, G, B;
	UNSHORT i;

	for (i=Start;i<Start + Size;i++)
	{
		/* Get the current colour */
		R = Palette.color[i].red;
		G = Palette.color[i].green;
		B = Palette.color[i].blue;

		/* Interpolate towards the target colour */
		R = ((R - Target_R) * (100 - Percentage)) / 100 + Target_R;
		G = ((G - Target_G) * (100 - Percentage)) / 100 + Target_G;
		B = ((B - Target_B) * (100 - Percentage)) / 100 + Target_B;

		/* Find the closest matching colour in the palette */
		Table[i] = Find_closest_colour(R, G, B);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Recolour_palette
 * FUNCTION  : Recolour the current palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.06.95 14:14
 * LAST      : 17.06.95 14:14
 * INPUTS    : UNSHORT Start - Number of first colour.
 *             UNSHORT Size - Number of colours to recolour.
 *             UNSHORT Target_R - Target red value.
 *             UNSHORT Target_G - Target green value.
 *             UNSHORT Target_B - Target blue value.
 *             UNSHORT Percentage - 0% : original colour,
 *              100% : target colour.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function takes colours from the backup palette,
 *              changes them, and puts them in the real palette. It does
 *              not call Update_palette, but activates the new palette
 *              directly. The recolouring tables aren't adapted.
 *              This function should only be used for effects, and afterwards
 *              Restore_palette should be called.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Recolour_palette(UNSHORT Start, UNSHORT Size, UNSHORT Target_R,
 UNSHORT Target_G, UNSHORT Target_B, UNSHORT Percentage)
{
	UNSHORT R, G, B;
	UNSHORT i;

	for (i=Start;i<Start + Size;i++)
	{
		/* Get the current colour from the backup palette */
		R = Backup_palette.color[i].red;
		G = Backup_palette.color[i].green;
		B = Backup_palette.color[i].blue;

		/* Interpolate towards the target colour */
		R = ((R - Target_R) * (100 - Percentage)) / 100 + Target_R;
		G = ((G - Target_G) * (100 - Percentage)) / 100 + Target_G;
		B = ((B - Target_B) * (100 - Percentage)) / 100 + Target_B;

		/* Store the new colour in the current palette */
		Palette.color[i].red		= R;
		Palette.color[i].green	= G;
		Palette.color[i].blue	= B;
	}

	/* Tell CONTROL to activate palette */
	Palette_has_changed = TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Blend_palette
 * FUNCTION  : Blend and set the current palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 14:26
 * LAST      : 31.08.95 14:26
 * INPUTS    : UNSHORT Start - Number of first colour.
 *             UNSHORT Size - Number of colours to recolour.
 *             UNSHORT Percentage - 0% : source palette,
 *              100% : target palette.
 *             struct BBPALETTE *Source_palette_ptr - Pointer to source palette.
 *             struct BBPALETTE *Target_palette_ptr - Pointer to target palette.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Blend_palette(UNSHORT Start, UNSHORT Size, UNSHORT Percentage,
struct BBPALETTE *Source_palette_ptr, struct BBPALETTE *Target_palette_ptr)
{
	BOOLEAN Changed_palette;
	UNSHORT Source_R, Source_G, Source_B;
	UNSHORT Target_R, Target_G, Target_B;
	UNSHORT R, G, B;
	UNSHORT i;

	Changed_palette = FALSE;

	for (i=Start;i<Start + Size;i++)
	{
		/* Get a colour from the source palette */
		Source_R = Source_palette_ptr->color[i].red;
		Source_G = Source_palette_ptr->color[i].green;
		Source_B = Source_palette_ptr->color[i].blue;

		/* Get a colour from the target palette */
		Target_R = Target_palette_ptr->color[i].red;
		Target_G = Target_palette_ptr->color[i].green;
		Target_B = Target_palette_ptr->color[i].blue;

		/* Interpolate */
		R = ((Source_R - Target_R) * (100 - Percentage)) / 100 + Target_R;
		G = ((Source_G - Target_G) * (100 - Percentage)) / 100 + Target_G;
		B = ((Source_B - Target_B) * (100 - Percentage)) / 100 + Target_B;

		/* Is there a difference ? */
		if ((Palette.color[i].red != R) || (Palette.color[i].green != G) ||
		 (Palette.color[i].blue	!= B))
		{
			/* Yes -> Indicate the palette has changed */
			Changed_palette = TRUE;

			/* Store the new colour in the current palette */
			Palette.color[i].red		= R;
			Palette.color[i].green	= G;
			Palette.color[i].blue	= B;
		}
	}

	/* Has the palette changed ? */
	if (Changed_palette)
	{
		/* Yes -> Update palette */
		Update_palette(Start, Size);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Colour_cycle_forward
 * FUNCTION  : Cycle colours forward in a palette.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 13:00
 * LAST      : 31.08.95 16:27
 * INPUTS    : struct BBPALETTE *Palette_ptr - Pointer to palette.
 *             UNSHORT Start - Number of first colour (0...255).
 *             UNSHORT Size - Number of colours to cycle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Colour_cycle_forward(struct BBPALETTE *Palette_ptr, UNSHORT Start,
 UNSHORT Size)
{
	struct BBCOLOR *Ptr;
	struct BBCOLOR Last;
	UNSHORT End;
	UNSHORT i;
/*	UNSHORT j, k;
	UNBYTE Temporary[10][256];
	UNBYTE *Work1, *Work2; */

	Ptr = &(Palette_ptr->color[0]);
	End = Start + Size - 1;

	/* Make a copy of the recolouring tables */
/*	memcpy(&(Temporary[0][0]), &(Recolour_tables[0][0]), 10 * 256); */

	/* Save the last colour in the range */
	Last.red		= Ptr[End].red;
	Last.green	= Ptr[End].green;
	Last.blue	= Ptr[End].blue;

	/* Copy all colours forward */
	for (i=End;i>Start;i--)
	{
		/* Copy colour */
		Ptr[i].red		= Ptr[i-1].red;
		Ptr[i].green	= Ptr[i-1].green;
		Ptr[i].blue		= Ptr[i-1].blue;

		/* Adjust all references in the recolouring tables */
/*		for (j=0;j<10;j++)
		{
			Work1 = &(Recolour_tables[j][0]);
			Work2 = &(Temporary[j][0]);

			for (k=0;k<Start;k++)
			{
				if (Work1[k] == i-1)
					Work2[k] = i;
			}
			for (k=End+1;k<256;k++)
			{
				if (Work1[k] == i-1)
					Work2[k] = i;
			}
		} */
	}

	/* Insert last colour at the start */
	Ptr[Start].red		= Last.red;
	Ptr[Start].green	= Last.green;
	Ptr[Start].blue	= Last.blue;

	/* Adjust all references in the recolouring tables */
/*	for (j=0;j<10;j++)
	{
		Work1 = &(Recolour_tables[j][0]);
		Work2 = &(Temporary[j][0]);

		for (k=0;k<Start;k++)
		{
			if (Work1[k] == End)
				Work2[k] = Start;
		}
		for (k=End+1;k<256;k++)
		{
			if (Work1[k] == End)
				Work2[k] = Start;
		}
	} */

	/* Copy the new recolouring tables back */
/*	memcpy(&(Recolour_tables[0][0]), &(Temporary[0][0]), 10 * 256); */
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fade_to_black
 * FUNCTION  : Fade the bottom 192 colours to black.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 19:36
 * LAST      : 11.08.95 19:36
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function takes colours from the backup palette,
 *              changes them, and puts them in the real palette. It does
 *              not call Update_palette, but activates the new palette
 *              directly. The recolouring tables aren't adapted.
 *              This function should only be used for effects, and afterwards
 *              Restore_palette should be called.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Fade_to_black(void)
{
	UNSHORT i;

	/* Fade to black */
	for (i=10;i<=100;i+=10)
	{
		/* Recolour the palette */
		Recolour_palette(0, 256, 0, 0, 0, i);

		/* Wait */
		Switch_screens();
		Switch_screens();
		Switch_screens();
	}

	/* Store fade parameters */
	Current_fade_R = 0;
	Current_fade_G = 0;
	Current_fade_B = 0;
	Current_fade_percentage = 100;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fade_to_white
 * FUNCTION  : Fade the bottom 192 colours to white.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 19:36
 * LAST      : 11.08.95 19:36
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function takes colours from the backup palette,
 *              changes them, and puts them in the real palette. It does
 *              not call Update_palette, but activates the new palette
 *              directly. The recolouring tables aren't adapted.
 *              This function should only be used for effects, and afterwards
 *              Restore_palette should be called.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Fade_to_white(void)
{
	UNSHORT i;

	/* Fade to white */
	for (i=10;i<=100;i+=10)
	{
		/* Recolour the palette */
		Recolour_palette(0, 256, 0xFF, 0xFF, 0xFF, i);

		/* Wait */
		Switch_screens();
		Switch_screens();
		Switch_screens();
	}

	/* Store fade parameters */
	Current_fade_R = 0xFF;
	Current_fade_G = 0xFF;
	Current_fade_B = 0xFF;
	Current_fade_percentage = 100;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fade_from_black
 * FUNCTION  : Fade the bottom 192 colours from black.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 19:42
 * LAST      : 14.08.95 00:28
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function takes colours from the backup palette,
 *              changes them, and puts them in the real palette. It does
 *              not call Update_palette, but activates the new palette
 *              directly. The recolouring tables aren't adapted.
 *              This function should only be used for effects, and afterwards
 *              Restore_palette should be called.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Fade_from_black(void)
{
	SISHORT i;

	for (i=100;i>=0;i-=10)
	{
		/* Recolour the palette */
		Recolour_palette(0, 256, 0, 0, 0, (UNSHORT) i);

		/* Wait */
		Switch_screens();
		Switch_screens();
		Switch_screens();
	}

	/* Store fade parameters */
	Current_fade_percentage = 0;

	/* Update the entire palette (!) */
	Update_palette(0, 256);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fade_from_white
 * FUNCTION  : Fade the bottom 192 colours from white.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 19:42
 * LAST      : 14.08.95 00:28
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function takes colours from the backup palette,
 *              changes them, and puts them in the real palette. It does
 *              not call Update_palette, but activates the new palette
 *              directly. The recolouring tables aren't adapted.
 *              This function should only be used for effects, and afterwards
 *              Restore_palette should be called.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Fade_from_white(void)
{
	SISHORT i;

	for (i=100;i>=0;i-=10)
	{
		/* Recolour the palette */
		Recolour_palette(0, 256, 0xFF, 0xFF, 0xFF, (UNSHORT) i);

		/* Wait */
		Switch_screens();
		Switch_screens();
		Switch_screens();
	}

	/* Store fade parameters */
	Current_fade_percentage = 0;

	/* Update the entire palette (!) */
	Update_palette(0, 256);
}

