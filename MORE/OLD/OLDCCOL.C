
extern MEM_HANDLE Transparency_table_handle;
extern MEM_HANDLE Luminance_table_handle;
extern MEM_HANDLE Transluminance_table_handle;

MEM_HANDLE Transparency_table_handle;
MEM_HANDLE Luminance_table_handle;
MEM_HANDLE Transluminance_table_handle;

	/* Clear handles */
	Transparency_table_handle		= NULL;
	Luminance_table_handle			= NULL;
	Transluminance_table_handle	= NULL;

	/* Load combat palette (for colour finding) */
	Load_combat_palette();

	/* Calculate various effect tables */
	Push_mouse(&(Mouse_pointers[WAIT_MPTR]));

	Transparency_table_handle = Calculate_transparency_table(50);
	Luminance_table_handle = Calculate_luminance_table();
	Transluminance_table_handle = Calculate_transluminance_table();

	Pop_mouse();

	if (!Transparency_table_handle || !Luminance_table_handle ||
	 !Transluminance_table_handle)
	{
		Pop_module();
		return;
	}

	MEM_Free_memory(Transparency_table_handle);
	MEM_Free_memory(Luminance_table_handle);
	MEM_Free_memory(Transluminance_table_handle);

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_transparency_table
 * FUNCTION  : Calculate a transparency table.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 15:30
 * LAST      : 07.03.95 15:30
 * INPUTS    : UNSHORT Intensity - Intensity in %.
 * RESULT    : MEM_HANDLE : Handle of transparency table.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Calculate_transparency_table(UNSHORT Intensity)
{
	MEM_HANDLE Handle;
	SISHORT Source_R, Source_G, Source_B;
	SISHORT Target_R, Target_G, Target_B;
	UNSHORT i, j;
	UNBYTE *Ptr;

	/* Allocate memory for recolouring table */
	Handle = MEM_Allocate_memory(256 * 256);

	/* Success ? */
	if (Handle)
	{
		/* Yes -> Calculate the transparency  table */
		Ptr = MEM_Claim_pointer(Handle);

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
		MEM_Free_pointer(Handle);
	}

	return Handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_luminance_table
 * FUNCTION  : Calculate a luminance table.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.03.95 17:34
 * LAST      : 20.03.95 17:34
 * INPUTS    : None.
 * RESULT    : MEM_HANDLE : Handle of luminance table.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Calculate_luminance_table(void)
{
	MEM_HANDLE Handle;
	SISHORT Source_R, Source_G, Source_B;
	SISHORT Target_R, Target_G, Target_B;
	UNSHORT Source_brightness;
	UNSHORT Target_brightness;
	UNSHORT i, j;
	UNBYTE *Base, *Ptr1, *Ptr2;
	UNBYTE Colour;

	/* Allocate memory for recolouring table */
	Handle = MEM_Allocate_memory(256 * 256);

	/* Success ? */
	if (Handle)
	{
		/* Yes -> Calculate the transparency  table */
		Base = MEM_Claim_pointer(Handle);

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
		MEM_Free_pointer(Handle);
	}

	return Handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_transluminance_table
 * FUNCTION  : Calculate a transluminance table.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.03.95 17:47
 * LAST      : 20.03.95 17:47
 * INPUTS    : None.
 * RESULT    : MEM_HANDLE : Handle of transluminance table.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Calculate_transluminance_table(void)
{
	MEM_HANDLE Handle;
	SISHORT Source_R, Source_G, Source_B;
	SISHORT Target_R, Target_G, Target_B;
	UNSHORT Target_brightness;
	UNSHORT i, j;
	UNBYTE *Ptr;

	/* Allocate memory for recolouring table */
	Handle = MEM_Allocate_memory(256 * 256);

	/* Success ? */
	if (Handle)
	{
		/* Yes -> Calculate the transparency  table */
		Ptr = MEM_Claim_pointer(Handle);

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
		MEM_Free_pointer(Handle);
	}

	return Handle;
}

