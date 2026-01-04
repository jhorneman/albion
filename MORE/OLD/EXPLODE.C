
/* structure definitions */

struct Explode_pixel_data {
	SISHORT X, Y;
	UNSHORT Colour;

	SISHORT dX, dY;		/* Multiplied with 100 */
	UNSHORT Gravity;
	UNSHORT Lifespan;

	struct Explode_pixel_erase_data Erase_data[2];
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Explode_block
 * FUNCTION  : Let a block explode.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.95 13:25
 * LAST      :
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Explode_block(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height,
 UNBYTE *Graphics_ptr)
{
	struct Explode_pixel_data *Pixel_list;
	MEM_HANDLE Pixel_data_handle;
	UNLONG Nr_pixels;
	SISHORT dX, dY;
	UNSHORT i, j;
	UNBYTE Pixel;

	/* Allocate memory for pixel data */
	Pixel_data_handle = MEM_Allocate_memory(Width * Height *
	 sizeof(struct Explode_pixel_data));

	/* Clear pixel counter */
	Nr_pixels = 0;

	/* Analyze block */
	Pixel_list = (struct Explode_pixel_data *)
	 MEM_Claim_pointer(Pixel_data_handle);

	for (i=0;i<Height;i++)
	{
		for (j=0;j<Width;j++)
		{
			/* Does the current pixel have the background colour ? */
			Pixel = *Graphics_ptr;
			if (Pixel)
			{
				/* No -> Insert data in pixel list */
				Pixel_list[Nr_pixels].X = X + j;
				Pixel_list[Nr_pixels].Y = Y + i;
				Pixel_list[Nr_pixels].Colour = Pixel;

				/* Create random pixel vector */
				do
				{
					dX = (rand() & 1600) - 800;
					dY = (rand() & 1600) - 1200;
				}
				while (!dX && !dY)

				/* Store vector */
				Pixel_list[Nr_pixels].dX = dX;
				Pixel_list[Nr_pixels].dY = dY;

				/* Count up */
				Nr_pixels++;
			}

			/* Next pixel */
			Graphics_ptr++;
		}
	}
	MEM_Free_pointer(Pixel_data_handle);

	/* Any pixels ? */
	if (Nr_pixels)
	{
		/* Yes -> */
	}

	/* Destroy pixel data */
	MEM_Free_memory(Pixel_data_handle);
}

