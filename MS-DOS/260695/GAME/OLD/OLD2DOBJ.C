/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_2D_object
 * FUNCTION  : Display a 2D object in the scroll buffer.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 18:54
 * LAST      : 16.08.94 14:12
 * INPUTS    : struct Object_2D *Object - Pointer to object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_2D_object(struct Object_2D *Object)
{
	struct BBRECT *Clip;
	SILONG X,Y;
	SILONG vpW,vpH;
	UNSHORT vpX,vpY;
	UNSHORT objW,objH;
	SISHORT wX, wY;
	UNBYTE *Ptr;

	/* Get object dimensions */
	objW = Object->Width;
	objH = Object->Height;

	/* Get scroll buffer coordinates and dimensions */
	vpX = Scroll_2D.Viewport_X;
	vpY = Scroll_2D.Viewport_Y;
	vpW = (SILONG) Scroll_2D.Base_OPM.width;
	vpH = (SILONG) Scroll_2D.Base_OPM.height;

	/* Calculate object coordinates in playfield */
	X = (SILONG) Object->X;
	Y = (SILONG) (Object->Y - (Object->Level * 16) - objH + 1);

	/* Convert to scroll buffer coordinates */
	X -= Scroll_2D.Playfield_X;
	Y -= Scroll_2D.Playfield_Y;

	/* Inside the scroll buffer ? */
 	if ((X <= 0 - objW) || (X >= vpW - 16)
	 || (Y <= 0 - objH) || (Y >= vpH - 16))
		return;

	/* Yes -> Get graphics address */
	Ptr = MEM_Claim_pointer(Object->Graphics_handle)
	 + Object->Graphics_offset;

	/* Add viewport coordinates */
	wX = X + vpX;
	wY = Y + vpY;

 	/* Get clipping rectangle */
	Clip = &Scroll_2D.Base_OPM.clip;

	/* Draw object */
	if (vpY < 16)
	{
		if (vpX < 16)
		{
			/* Scroll buffer is not divided */
			/* Draw object in first quadrant */
			Clip->left = vpX;
			Clip->top = vpY;
			Clip->width = vpX + vpW - 16;
			Clip->height = vpY + vpH - 16;

			//OPM_FillBox(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, WHITE);
			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
		}
		else
		{
			/* Scroll buffer is divided horizontally */
			/* Draw object in first quadrant */
			Clip->left = vpX;
			Clip->top = vpY;
			Clip->width = vpW - vpX;
			Clip->height = vpY + vpH - 16;

			//OPM_FillBox(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, WHITE);
			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

			/* Draw object in second quadrant */
			Clip->left = 0;
			Clip->width = vpX - 16;

			wX -= vpW;
			//OPM_FillBox(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, WHITE);
			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
		}
	}
	else
	{
		if (vpX < 16)
		{
			/* Scroll buffer is divided vertically */
			/* Draw object in first quadrant */
			Clip->left = vpX;
			Clip->top = vpY;
			Clip->width = vpX + vpW - 16;
			Clip->height = vpH - vpY;

			//OPM_FillBox(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, WHITE);
			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

			/* Draw object in third quadrant */
			Clip->top = 0;
			Clip->height = vpY - 16;

			wY -= vpH;

			//OPM_FillBox(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, WHITE);
			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
		}
		else
		{
			/* Scroll buffer is divided horizontally AND vertically */
			/* Draw object in first quadrant */
			Clip->left = vpX;
			Clip->top = vpY;
			Clip->width = vpW - vpX;
			Clip->height = vpH - vpY;

			//OPM_FillBox(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, WHITE);
			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

			/* Draw object in second quadrant */
			Clip->left = 0;
			Clip->width = vpX - 16;

			wX -= vpW;
			//OPM_FillBox(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, WHITE);
			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

			/* Draw object in third quadrant */
			Clip->top = 0;
			Clip->height = vpY - 16;

			wY -= vpH;

			//OPM_FillBox(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, WHITE);
			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

			/* Draw object in fourth quadrant */
			Clip->left = vpX;
			Clip->width = vpW - vpX;

			wX += vpW;

			//OPM_FillBox(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, WHITE);
			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
		}
	}

	/* Restore clipping rectangle */
	Clip->left = 0;
	Clip->top = 0;
	Clip->width = vpW;
	Clip->height = vpH;

	MEM_Free_pointer(Object->Graphics_handle);
}

