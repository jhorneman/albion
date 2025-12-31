/************
 * NAME     : CALCCOL.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 22-7-1994
 * PROJECT  : Combat transparency table calculator
 * NOTES    :
 * SEE ALSO : XLOAD.C
 ************/

/* includes */

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>

#include <BBDEF.H>

#include <FINDCOL.H>

/* defines */

/* defines */

/* Number of colours in a palette */
#define SCREEN_PALETTE_ENTRIES	(256)

/* structure definitions */

/* One palette entry */
struct BBCOLOR
{
	UNBYTE red;
	UNBYTE green;
	UNBYTE blue;
	UNBYTE alpha;
};

/* Palette structure */
struct BBPALETTE
{
	UNSHORT entries;
	UNSHORT version;
	struct BBCOLOR	color[SCREEN_PALETTE_ENTRIES];
};

/* global variables */

static UNCHAR *Base_palette_path = "G:\\ALBION\\XLDLIBS\\PALETTE.000";
static UNCHAR *Combat_palette_path = "G:\\ALBION\\DATA\\PALETTES\\050.PAL";

static struct BBPALETTE Palette;

/* prototypes */

void main(int argc, char** argv);

BOOLEAN Load_palette_part(UNCHAR *Path, UNSHORT Source_colour_offset,
 UNSHORT Target_colour_offset, UNSHORT Max_colours);

void Calculate_transparency_table(UNBYTE *Table_ptr, UNSHORT Intensity);
void Calculate_luminance_table(UNBYTE *Table_ptr);
void Calculate_transluminance_table(UNBYTE *Table_ptr);

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : main
 * FUNCTION  : Main function of CALCCOL
 * FILE      : CALCCOL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.95 22:18
 * LAST      : 15.09.95 10:57
 * INPUTS    : int argc - Number of arguments.
 *             char **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
main(int argc, char** argv)
{
	int File_handle;
	UNBYTE *Trans_table_ptr;
	UNCHAR *Palette_path;
	UNCHAR *Trans_table_path;

	/* Exit if number of arguments is illegal */
	if (argc != 3)
	{
		printf("Illegal number of arguments.\n");
		return;
	}

	/* Get parameters */
	Palette_path		= argv[1];
	Trans_table_path	= argv[2];

	/* Load palette */
	Load_palette_part
	(
		Palette_path,
		0,
		0,
		80
	);

	/* Load combat palette */
	Load_palette_part
	(
		Palette_path,
		80,
		80,
		112
	);

	/* Load base palette */
	Load_palette_part
	(
		Base_palette_path,
		0,
		192,
		64
	);

	/* Clear protected colours */
	Clear_protected_colours();

	/* Protect top 64 colours */
	Add_protected_colours(192, 64);

	/* Prepare colour finding */
	Prepare_colour_find(&Palette, 0, 256);

	/* Allocate memory for transparency tables */
	Trans_table_ptr = malloc(3 * 65536);

	/* Calculate transparency tables */
	Calculate_transparency_table(Trans_table_ptr, 50);
	Calculate_luminance_table(Trans_table_ptr + 65536);
	Calculate_transluminance_table(Trans_table_ptr + 2 * 65536);

	/* Try to open transparency table file */
	File_handle = open
	(
		Trans_table_path,
		O_WRONLY | O_CREAT | O_BINARY,
		S_IRWXU | S_IRWXG | S_IRWXO
	);

	/* Success ? */
	if (File_handle == -1)
	{
		/* No -> Report error and exit */
		printf("File %s couldn't be opened.\n", Trans_table_path);
	}
	else
	{
		/* Yes -> Write transparency table */
		write
		(
			File_handle,
			Trans_table_ptr,
			3 * 65536
		);

		/* Close file */
		close(File_handle);
	}

	/* Free memory for transparency tables */
	free(Trans_table_ptr);

	/* Report */
	printf("\nTransparency tables\n %s\ncreated for palette\n %s\n\n",
	 Trans_table_path, Palette_path);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_palette_part
 * FUNCTION  : Load part of a palette and copy it to the main palette.
 * FILE      : CALCCOL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.95 22:37
 * LAST      : 15.09.95 13:36
 * INPUTS    : UNCHAR *Path - Path of palette part.
 *             UNSHORT Source_colour_offset - Offset to first colour in
 *              source palette.
 *             UNSHORT Target_colour_offset - Offset to first colour in
 *              target palette.
 *             UNSHORT Max_colours - Maximum number of colours to copy.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Load_palette_part(UNCHAR *Path, UNSHORT Source_colour_offset,
 UNSHORT Target_colour_offset, UNSHORT Max_colours)
{
	int File_handle;
	UNLONG File_length;
	UNSHORT i;
	UNBYTE *Buffer_ptr;
	UNBYTE *Ptr;

	/* Open palette file */
	File_handle = open
	(
		Path,
		O_RDONLY|O_BINARY
	);

	/* Success ? */
	if (File_handle == -1)
	{
		/* No -> Report error and exit */
		printf("Palette %s couldn't be opened.\n", Path);
		return FALSE;
	}

	/* Get file length */
	File_length = filelength(File_handle);

	/* Allocate memory for palette */
	Buffer_ptr = malloc(File_length);

	/* Read a block from the subfile */
	read
	(
		File_handle,
		Buffer_ptr,
		File_length
	);

	/* Close file */
	close(File_handle);

	/* Copy colours to palette */
	Ptr = Buffer_ptr + (Source_colour_offset * 3);
	for (i=0;i<min(Max_colours, File_length / 3);i++)
	{
		Palette.color[Target_colour_offset + i].red		= *Ptr++;
		Palette.color[Target_colour_offset + i].green	= *Ptr++;
		Palette.color[Target_colour_offset + i].blue 	= *Ptr++;
	}

	/* Free palette memory */
	free(Buffer_ptr);

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_transparency_table
 * FUNCTION  : Calculate a transparency table.
 * FILE      : CALCCOL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 15:30
 * LAST      : 15.09.95 10:27
 * INPUTS    : UNBYTE *Table_ptr - Pointer to table.
 *             UNSHORT Intensity - Intensity in %.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_transparency_table(UNBYTE *Table_ptr, UNSHORT Intensity)
{
	SISHORT Source_R, Source_G, Source_B;
	SISHORT Target_R, Target_G, Target_B;
	UNSHORT i, j;
	UNBYTE *Ptr;

	/* Calculate the transparency  table */
	Ptr = Table_ptr;
	for (i=0;i<256;i++)
	{
		/* Get the current target colour */
		Target_R = Palette.color[i].red;
		Target_G = Palette.color[i].green;
		Target_B = Palette.color[i].blue;

		for (j=0;j<256;j++)
		{
			/* Same colours ? */
			if (i == j)
			{
				/* Yes -> Don't change */
				*Ptr++ = i;
			}
			else
			{
				/* No -> Get the current source colour */
				Source_R = Palette.color[j].red;
				Source_G = Palette.color[j].green;
				Source_B = Palette.color[j].blue;

				/* Interpolate towards the target colour */
				Source_R = ((Target_R - Source_R) * (100 - Intensity)) / 100 + Source_R;
				Source_G = ((Target_G - Source_G) * (100 - Intensity)) / 100 + Source_G;
				Source_B = ((Target_B - Source_B) * (100 - Intensity)) / 100 + Source_B;

				/* Find the closest matching colour in the palette */
				*Ptr++ = Find_closest_colour(Source_R, Source_G, Source_B);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_luminance_table
 * FUNCTION  : Calculate a luminance table.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.03.95 17:34
 * LAST      : 15.09.95 10:27
 * INPUTS    : UNBYTE *Table_ptr - Pointer to table.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_luminance_table(UNBYTE *Table_ptr)
{
	SISHORT Source_R, Source_G, Source_B;
	SISHORT Target_R, Target_G, Target_B;
	UNSHORT Source_brightness;
	UNSHORT Target_brightness;
	UNSHORT i, j;
	UNBYTE *Base, *Ptr1, *Ptr2;
	UNBYTE Colour;

	/* Calculate the transparency  table */
	Base = Table_ptr;
	for (i=0;i<256;i++)
	{
		Ptr1 = Base + (i * 256) + i;
		Ptr2 = Ptr1;

		/* Get the current target colour */
		Target_R = Palette.color[i].red;
		Target_G = Palette.color[i].green;
		Target_B = Palette.color[i].blue;

		/* Calculate the target colour's brightness */
		Target_brightness = (Target_R + Target_G + Target_B);

		for (j=i;j<256;j++)
		{
			/* Same colours ? */
			if (i == j)
			{
				/* Yes -> Don't change */
				Colour = i;
			}
			else
			{
				/* No ->	Get the current source colour */
				Source_R = Palette.color[j].red;
				Source_G = Palette.color[j].green;
				Source_B = Palette.color[j].blue;

				/* Calculate the source colour's brightness */
				Source_brightness = (Source_R + Source_G + Source_B);

				/* Is the source colour brighter than the target colour ? */
				if (Source_brightness > Target_brightness)
				{
					/* Yes -> Draw source colour */
					Colour = j;
				}
				else
				{
					/* No -> Draw target colour */
					Colour = i;
				}
			}

			/* Store colours  and increase pointers */
			*Ptr1 = Colour;
			*Ptr2 = Colour;
			Ptr1++;
			Ptr2 += 256;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_transluminance_table
 * FUNCTION  : Calculate a transluminance table.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.03.95 17:47
 * LAST      : 15.09.95 10:28
 * INPUTS    : UNBYTE *Table_ptr - Pointer to table.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_transluminance_table(UNBYTE *Table_ptr)
{
	SISHORT Source_R, Source_G, Source_B;
	SISHORT Target_R, Target_G, Target_B;
	UNSHORT Target_brightness;
	UNSHORT i, j;
	UNBYTE *Ptr;

	/* Calculate the transparency  table */
	Ptr = Table_ptr;
	for (i=0;i<256;i++)
	{
		/* Get the current target colour */
		Target_R = Palette.color[i].red;
		Target_G = Palette.color[i].green;
		Target_B = Palette.color[i].blue;

		/* Calculate the target colour's brightness */
		Target_brightness = (Target_R + Target_G + Target_B) / 3;

		for (j=0;j<256;j++)
		{
			/* No -> Get the current source colour */
			Source_R = Palette.color[j].red;
			Source_G = Palette.color[j].green;
			Source_B = Palette.color[j].blue;

			/* Determine the target colour */
			Source_R += Target_brightness;
			Source_G += Target_brightness;
			Source_B += Target_brightness;

			/* Clip colour values */
			if (Source_R > 255)
				Source_R = 255;

			if (Source_G > 255)
				Source_G = 255;

			if (Source_B > 255)
				Source_B = 255;

			/* Find the closest matching colour in the palette */
			*Ptr++ = Find_closest_colour(Source_R, Source_G, Source_B);
		}
	}
}

