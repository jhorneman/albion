/************
 * NAME     : 2D_MAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28.07.94 13:10
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBEVENT.H>
#include <BBOPM.H>
#include <BBSYSTEM.H>
#include <BBMEM.H>

#include <XLOAD.H>
#include <SCROLL.H>
#include <GFXFUNC.H>
#include <SORT.H>

#include <MAP.H>
#include <GAME.H>
#include <2D_MAP.H>
#include <ANIMATE.H>
#include <XFTYPES.H>

/* global variables */

MEM_HANDLE Map_handle, Icondata_handle[2], Icongfx_handle[2];
MEM_HANDLE Party_graphics_handle;

Object_2D TEST = {
	0,
	0, 0,
	32,48,
	0,
   0,
	0
};

Object_2D *Object_2D_list[MAX_2D_OBJECTS];
UNSHORT Nr_2D_objects = 0, Nr_2D_rectangles = 0;

Rectangle_2D Rectangle_list[MAX_2D_OBJECTS];

UNSHORT Camera_2D_X, Camera_2D_Y;
UNSHORT Map_buffer[MAPBUF_HEIGHT+3][MAPBUF_WIDTH][MAPBUF_DEPTH][2];
UNBYTE Update_buffer[MAPBUF_HEIGHT+3][MAPBUF_WIDTH];

UNSHORT Palette_nr, Map_width, Map_height;

UNSHORT XYX = 0;

Map_data *Map_ptr;
UNBYTE *Icon_graphics[2];
Icon_2D_data *Icon_data[2];

struct Scroll_data Scroll_2D;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94
 * LAST      : 28.07.94
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_2D_map(void)
{
	MEM_HANDLE Pal_handle;
	UNSHORT i;
	UNBYTE *Ptr;

	PARTY_DATA.X = 10;
	PARTY_DATA.Y = 10;

	/* Load party graphics */
	Party_graphics_handle = XLD_Load_subfile(&Xftypes[PARTY_GFX], 1);

	/* Load map */
	Map_handle = XLD_Load_subfile(&Xftypes[MAP_DATA], PARTY_DATA.Map_nr);

	Map_ptr = (Map_data *) MEM_Claim_pointer(Map_handle);

	/* Get data from map */
	Palette_nr = (UNSHORT) Map_ptr->Colour_pal_nr;
	Map_width = (UNSHORT) Map_ptr->Map_width;
	Map_height = (UNSHORT) Map_ptr->Map_height;

	/* Load first icon set */
	Icondata_handle[0] = XLD_Load_subfile(&Xftypes[ICON_DATA],
	 (UNSHORT) Map_ptr->ICON_SET_1_NR);
	Icongfx_handle[0] = XLD_Load_subfile(&Xftypes[ICON_GFX],
	 (UNSHORT) Map_ptr->ICON_SET_1_NR);

	/* Load second icon set (if any) */
	if (Map_ptr->ICON_SET_2_NR)
	{
		Icondata_handle[1] = XLD_Load_subfile(&Xftypes[ICON_DATA],
		 (UNSHORT) Map_ptr->ICON_SET_2_NR);
		Icongfx_handle[1] = XLD_Load_subfile(&Xftypes[ICON_GFX],
		 (UNSHORT) Map_ptr->ICON_SET_2_NR);
	}

	/* Load 2D map palette */
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

	Init_animation();

	Object_2D_list[0] = &TEST;
	Nr_2D_objects = 1;

	/* Initialize scrolling */
	Scroll_2D.Update_unit = Update_2D_unit;
	Scroll_2D.Unit_width = 16;
	Scroll_2D.Unit_height = 16;
	Scroll_2D.Viewport_width = MAP2D_WIDTH;
	Scroll_2D.Viewport_height = MAP2D_HEIGHT;
	Scroll_2D.Playfield_width = Map_width * 16 ;
	Scroll_2D.Playfield_height = Map_ptr -> Map_height * 16;

	Icon_data[0] = (Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[0]);
	Icon_data[1] = (Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[1]);
	Icon_graphics[0] = MEM_Claim_pointer(Icongfx_handle[0]);
	Icon_graphics[1] = MEM_Claim_pointer(Icongfx_handle[1]);

	Init_scroll(&Scroll_2D, 0, 0);

	MEM_Free_pointer(Icongfx_handle[0]);
	MEM_Free_pointer(Icongfx_handle[1]);
	MEM_Free_pointer(Icondata_handle[0]);
	MEM_Free_pointer(Icondata_handle[1]);

	MEM_Free_pointer(Map_handle);

	Camera_2D_X = (UNSHORT)(Scroll_2D.Playfield_X / 16);
	Camera_2D_Y = (UNSHORT)(Scroll_2D.Playfield_Y / 16);
}

/*
 ******************************************************************************
 */

void
Handle_keys_2D(UNSHORT Key_code)
{
	switch (Key_code)
	{
		case BLEV_UP:
		{
			if (PARTY_DATA.Y > 2)
			{
				PARTY_DATA.Y--;
				PARTY_DATA.View_direction = NORTH;
			}
			break;
		}
		case BLEV_DOWN:
		{
			if (PARTY_DATA.Y < Map_height-1)
			{
				PARTY_DATA.Y++;
				PARTY_DATA.View_direction = SOUTH;
			}
			break;
		}
 		case BLEV_LEFT:
		{
			if (PARTY_DATA.X > 0)
			{
				PARTY_DATA.X--;
				PARTY_DATA.View_direction = WEST;
			}
			break;
		}
 		case BLEV_RIGHT:
		{
			if (PARTY_DATA.X < Map_width-2)
			{
				PARTY_DATA.X++;
				PARTY_DATA.View_direction = EAST;
			}
			break;
		}
	}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94
 * LAST      : 28.07.94
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_2D_map(void)
{
	SISHORT mvecx, mvecy;
	struct BBPOINT Mouse;

	TEST.X = PARTY_DATA.X * 16;
	TEST.Y = PARTY_DATA.Y * 16 + 15;
	TEST.Graphics_handle = Party_graphics_handle;
	TEST.Graphics_offset = PARTY_DATA.View_direction * 1536;

	if (!XYX)
	{
		switch (Palette_nr)
 		{
			case 1:
			case 2:
			{
				Colour_cycle_forward(153,7);
				Colour_cycle_forward(176,5);
				Colour_cycle_forward(181,11);
				DSA_ActivatePal(&Screen);
			}
			case 3:
			{
				Colour_cycle_forward(176,5);
				Colour_cycle_forward(181,11);
				DSA_ActivatePal(&Screen);
			}
		}

		Update_animation();
	}

	XYX++;

	if (XYX > 3)
		XYX = 0;

	BLEV_GetMousePos(&Mouse);

	mvecx = 0;
	mvecy = 0;

	if (Mouse.x < 16)
		mvecx = 0 - (16 - Mouse.x) / 2;
	if (Mouse.x > Screen_width-16)
		mvecx = (Mouse.x - Screen_width+16) / 2;
	if (Mouse.y < 16)
		mvecy = 0 - (16 - Mouse.y) / 2;
	if (Mouse.y > Screen_height-16)
		mvecy = (Mouse.y - Screen_height+16) / 2;

	Map_ptr = (Map_data *) MEM_Claim_pointer(Map_handle);
	Icon_data[0] = (Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[0]);
	Icon_data[1] = (Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[1]);
	Icon_graphics[0] = MEM_Claim_pointer(Icongfx_handle[0]);
	Icon_graphics[1] = MEM_Claim_pointer(Icongfx_handle[1]);

	Do_scroll(&Scroll_2D, mvecx, mvecy);
	Yagga();

	MEM_Free_pointer(Icongfx_handle[0]);
	MEM_Free_pointer(Icongfx_handle[1]);
	MEM_Free_pointer(Icondata_handle[0]);
	MEM_Free_pointer(Icondata_handle[1]);
	MEM_Free_pointer(Map_handle);

	TEST.X++;
//	TEST.Y++;
	if (TEST.X == 96)
	{
		TEST.X = 16;
//		TEST.Y = 175-16;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94
 * LAST      : 28.07.94
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_2D_map(void)
{
	Scroll_display(&Scroll_2D, &Screen, 0, 0);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94
 * LAST      : 28.07.94
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_2D_map(void)
{
	Map_data *Map_ptr;

	Exit_scroll(&Scroll_2D);

/*	{
		Icon_2D_data *Data;
		UNSHORT i,h;

		Icon_data[0] = (Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[0]);
		printf("\n\n");

		for (i=2500;i<2520;i++)
		{
		 	Data = &Icon_data[0][i-1];
			h = (Data->Flags & ICON_HEIGHT) >> ICON_HEIGHT_B;
			printf("%u : %u \n",i,h);
		}

		printf("\n");
		MEM_Free_pointer(Icondata_handle[0]);
	} */

	Map_ptr = (Map_data *) MEM_Claim_pointer(Map_handle);

	MEM_Free_memory(Icondata_handle[0]);
	MEM_Free_memory(Icongfx_handle[0]);

	if (Map_ptr->ICON_SET_2_NR)
	{
		MEM_Free_memory(Icondata_handle[1]);
		MEM_Free_memory(Icongfx_handle[1]);
	}

	MEM_Free_pointer(Map_handle);
	MEM_Free_memory(Map_handle);

	MEM_Free_memory(Party_graphics_handle);

/*	{
		UNSHORT i;

		printf("\nYO-HO :\n");
		for (i=0;i<YYY;i++)
		{
			printf("%u ",XXX[i]);
		}
		printf("\n\n");
	} */
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 16:00
 * LAST      : 28.07.94 16:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_2D_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X, UNSHORT Buffer_Y,
 UNSHORT Playfield_X, UNSHORT Playfield_Y)
{
	Icon_2D_data *Data, *Data2;
	UNBYTE *Map_layer_ptr, *Icon;
	UNSHORT Offset, Frame;
	UNSHORT B1,B2,B3;
	UNSHORT Underlay,Overlay;

	Camera_2D_X = (UNSHORT)(Scroll_2D.Playfield_X >> 4);
	Camera_2D_Y = (UNSHORT)(Scroll_2D.Playfield_Y >> 4);

	/* Calculate offset / hash number */
	Offset = Playfield_X + Playfield_Y * Map_width;

	/* Calculate pointer to map layer */
	Map_layer_ptr = (UNBYTE *) Map_ptr + sizeof(Map_data)
	 + 32 * sizeof(NPC_data) + (3 * Offset);

	/* Read bytes from map */
	B1 = (UNSHORT) *Map_layer_ptr++;
	B2 = (UNSHORT) *Map_layer_ptr++;
	B3 = (UNSHORT) *Map_layer_ptr++;

	/* Build overlay and underlay number */
	Underlay = ((B2 & 0x0F) << 8) | B3;
	Overlay = (B1 << 4) | (B2 >> 4);

	/* Any underlay ? */
	if (Underlay > 1)
	{
		/* Yes -> Get icon data */
		Data = &Icon_data[0][Underlay-2];
		Data2 = &Icon_data[0][Underlay-1];

		/* Animated ? */
		if (Data2->Nr_frames == 1)
		{
			/* No -> Get frame number */
			Frame = Data->First_frame + Get_animation_frame(Data2->Flags,
			 Data2->Nr_frames, Offset);

			/* Get graphics address */
			Icon = Icon_graphics[0] + 256 * Frame;

			/* Display underlay */
			Put_unmasked_block(&(Scroll->Base_OPM), Buffer_X, Buffer_Y, 16, 16,
		 	 Icon);
		}
	}

	/* Any overlay ? */
	if (Overlay > 1)
	{
		/* Yes -> Get icon data */
		Data = &Icon_data[0][Overlay-2];
		Data2 = &Icon_data[0][Overlay-1];

		/* Animated ? */
		if (Data2->Nr_frames == 1)
		{
			/* No -> Get frame number */
			Frame = Data->First_frame + Get_animation_frame(Data2->Flags,
			 Data2->Nr_frames, Offset);

			/* Get graphics address */
			Icon = Icon_graphics[0] + 256 * Frame;

			/* Display overlay */
			Put_masked_block(&(Scroll->Base_OPM), Buffer_X, Buffer_Y, 16, 16,
		 	 Icon);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 18:54
 * LAST      : 28.07.94 18:54
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Yagga(void)
{
	Object_2D *Ptr;
	UNSHORT i,j,k;
	UNSHORT Remaining_2D_objects;
	UNSHORT U, Current_object = 0;

	/* Erase previous objects */
	Set_erase_rectangles();
	Fill_map_buffer();
	Set_animation_update();

	/* Update current objects */
	for (i=0;i<Nr_2D_objects;i++)
	{
		Ptr = Object_2D_list[i];

		/* Set update rectangle */
		Set_update_rectangle(Ptr->X, Ptr->Y - (Ptr->Level * 16),
		 Ptr->Width, Ptr->Height);

		/* Calculate display index */
		Ptr->Index = ((Ptr->X + Ptr->Width - 1) / 16) - Camera_2D_X
		 + ((Ptr->Y / 16) - Camera_2D_Y) * MAPBUF_WIDTH;
	}

	/* Sort 2D objects */
	Shellsort(Swap_2D_objects, Compare_2D_objects, Nr_2D_objects, NULL);

	/* Display 2D objects */
	Remaining_2D_objects = Nr_2D_objects;

	for (i=0;i<MAPBUF_HEIGHT+3;i++)
	{
		for (j=0;j<MAPBUF_WIDTH;j++)
		{
			/* Update ? */
			U = Update_buffer[i][j];

			if (U)
			{
				/* Yes -> Do layers */
				for (k=0;k<MAPBUF_DEPTH-1;k++)
				{
					/* Update this layer ? */
					if (U & (1 << k))
					{
						if (((i-k) >= 0) && ((i-k) < MAPBUF_HEIGHT))
						{
							/* Yes -> Display underlay and overlay */
							Display_underlay(Map_buffer[i][j][k][0], j, i-k);
							Display_overlay(Map_buffer[i][j][k][1], j, i-k);
						}
					}
				}
			}

			/* Any objects left ? */
			while (Remaining_2D_objects)
			{
				/* Yes -> Get data */
				Ptr = Object_2D_list[Current_object];

				/* Draw this one ? */
				if (Ptr->Index == i * MAPBUF_WIDTH + j)
				{
					/* Yes -> Draw */
					Draw_2D_object(Ptr);

					/* Count down */
					Remaining_2D_objects--;
					Current_object++;
				}
				else
					/* No -> Exit */
					break;
			}
		}
	}


	/* Display */
	k = MAPBUF_DEPTH-1;
	for (i=0;i<MAPBUF_HEIGHT+3;i++)
	{
		for (j=0;j<MAPBUF_WIDTH;j++)
		{
			/* Update ? */
			U = Update_buffer[i][j];

			if (U)
			{
				/* Yes -> Clear */
				Update_buffer[i][j] = 0;

				/* Update this layer ? */
				if (U & (1 << k))
				{
					if (((i-k) >= 0) && ((i-k) < MAPBUF_HEIGHT))
					{
						/* Yes -> Display underlay and overlay */
						Display_underlay(Map_buffer[i][j][k][0], j, i-k);
						Display_overlay(Map_buffer[i][j][k][1], j, i-k);
					}
				}
			}
		}
	}
}

void
Display_underlay(UNSHORT Underlay_nr, UNSHORT X, UNSHORT Y)
{
	SILONG nX, nY;
	Icon_2D_data *Data, *Data2;
	UNBYTE *Icon;
	UNSHORT Frame;

	/* Any underlay ? */
	if (Underlay_nr)
	{
		/* Get scroll buffer coordinates */
		nX = ((SILONG) (Camera_2D_X + X)) * 16;
		nY = ((SILONG) (Camera_2D_Y + Y)) * 16;
		Convert_scroll_coordinates(&Scroll_2D, &nX, &nY);

		if ((nX == -32768) && (nY == -32768))
			return;

		/* Yes -> Get frame number */
		Data = &Icon_data[0][Underlay_nr-2];
		Data2 = &Icon_data[0][Underlay_nr-1];

		Frame = Data->First_frame + Get_animation_frame(Data2->Flags,
	 	 Data2->Nr_frames, (X + Y * (UNSHORT) Map_ptr->Map_width));

		/* Get graphics address */
		Icon = Icon_graphics[0] + 256 * Frame;

		/* Display underlay */
	 	Put_unmasked_block(&(Scroll_2D.Base_OPM), nX, nY, 16, 16, Icon);
	 	// OPM_Box(&(Scroll_2D.Base_OPM), nX, nY, 16, 8, 195);
	}
}

void
Display_overlay(UNSHORT Overlay_nr, UNSHORT X, UNSHORT Y)
{
	SILONG nX, nY;
	Icon_2D_data *Data, *Data2;
	UNBYTE *Icon;
	UNSHORT Frame;

	/* Any overlay ? */
	if (Overlay_nr)
	{
		/* Yes -> Get scroll buffer coordinates */
		nX = ((SILONG) (Camera_2D_X + X)) * 16;
		nY = ((SILONG) (Camera_2D_Y + Y)) * 16;
		Convert_scroll_coordinates(&Scroll_2D, &nX, &nY);

		if ((nX == -32768) && (nY == -32768))
			return;

		/* Get frame number */
		Data = &Icon_data[0][Overlay_nr-2];
		Data2 = &Icon_data[0][Overlay_nr-1];

		Frame = Data->First_frame+ Get_animation_frame(Data2->Flags,
	 	 Data2->Nr_frames, (X + Y * (UNSHORT) Map_ptr->Map_width));

		/* Get graphics address */
		Icon = Icon_graphics[0] + 256 * Frame;

		/* Display overlay */
		Put_masked_block(&(Scroll_2D.Base_OPM), nX, nY, 16, 16, Icon);
	 	// OPM_Box(&(Scroll_2D.Base_OPM), nX, nY + 8, 16, 8, 193);
	}
}

void
Draw_2D_object(Object_2D *Object)
{
	SILONG X,Y;
	SILONG vpW,vpH;
	UNSHORT objW,objH;
	UNSHORT W,H;
	SISHORT wX, wY;
	UNBYTE *Ptr;

	/* Get object dimensions */
	objW = Object->Width;
	objH = Object->Height;

	/* Get scroll buffer dimensions */
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
	wX = X + Scroll_2D.Viewport_X;
	wY = Y + Scroll_2D.Viewport_Y;

	/* Is clipping necessary ? */
	if ((X < 0) || ((X + objW - 1) >= vpW - 16)
	 || (Y < 0) || ((Y + objH - 1) >= vpH - 16))
	{
		struct BBRECT *Clip;

		/* Get clipping rectangle */
		Clip = &Scroll_2D.Base_OPM.clip;

		/* Display object in first quadrant */
		Clip->left = Scroll_2D.Viewport_X;
		Clip->top = Scroll_2D.Viewport_Y;
		Clip->width = vpW - Scroll_2D.Viewport_X - 16;
		Clip->height = vpH - Scroll_2D.Viewport_Y - 16;

		Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

		/* Display object in second quadrant */
		Clip->left = 0;
		Clip->width = Scroll_2D.Viewport_X;

		wX -= vpW;
		Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

		/* Display object in third quadrant */
		Clip->top = 0;
		Clip->height = Scroll_2D.Viewport_Y;

		wY -= vpH;
		Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

		/* Display object in fourth quadrant */
		Clip->left = Scroll_2D.Viewport_X;
		Clip->width = vpW - Scroll_2D.Viewport_X - 16;

		wX += vpW;
		Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

		/* Restore clipping rectangle */
		Clip->left = 0;
		Clip->top = 0;
		Clip->width = Scroll_2D.Base_OPM.width;
		Clip->height = Scroll_2D.Base_OPM.height;
	}
	else
	{
		/* Display object in all quadrants */
		Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
		wX -= vpW;
		Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
		wY -= vpH;
		Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
		wX += vpW;
		Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
	}

	MEM_Free_pointer(Object->Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Swap_2D_objects
 * FUNCTION  : Swap two 2D objects (for sorting).
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 19:08
 * LAST      : 28.07.94 19:08
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Swap_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data)
{
	Object_2D *T;

	T = Object_2D_list[A];
	Object_2D_list[A] = Object_2D_list[B];
	Object_2D_list[B] = T;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Compare_2D_objects
 * FUNCTION  : Compare two 2D objects (for sorting).
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 19:07
 * LAST      : 28.07.94 19:07
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 *	RESULT    : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Compare_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data)
{
	return((Object_2D_list[A])->Index > (Object_2D_list[B])->Index);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.08.94 11:11
 * LAST      : 01.08.94 11:11
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_animation_update(void)
{
	Icon_2D_data *Data;
	UNSHORT Underlay, Overlay;
	UNSHORT i,j,k,l;

	/* Check the entire map buffer */
	for (i=0;i<MAPBUF_HEIGHT+3;i++)
	{
		for (j=0;j<MAPBUF_WIDTH;j++)
		{
			/* Do layers */
			for (k=0;k<MAPBUF_DEPTH;k++)
			{
				/* Get underlay */
				Underlay = Map_buffer[i][j][k][0];

				/* Any underlay ? */
				if (Underlay)
				{
					/* Yes -> Get icon data */
					Data = &Icon_data[0][Underlay-1];

					/* Animated ? */
					if (Data->Nr_frames > 1)
					{
						/* Yes -> Update */
						for (l=0;l<MAPBUF_DEPTH;l++)
						{
							if (((i-k+l) >= 0) && ((i-k+l) < MAPBUF_HEIGHT + 3))
								Update_buffer[i-k+l][j] |= (1 << l);
						}
					}
				}

				/* Get overlay */
				Overlay = Map_buffer[i][j][k][1];

				/* Any overlay ? */
				if (Overlay)
				{
					/* Yes -> Get icon data */
					Data = &Icon_data[0][Overlay-1];

					/* Animated ? */
					if (Data->Nr_frames > 1)
					{
						/* Yes -> Update */
						for (l=0;l<MAPBUF_DEPTH;l++)
						{
							if (((i-k+l) >= 0) && ((i-k+l) < MAPBUF_HEIGHT + 3))
								Update_buffer[i-k+l][j] |= (1 << l);
						}
					}
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_update_rectangle
 * FUNCTION  : Set update rectangle.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 16:20
 * LAST      : 28.07.94 16:20
 * INPUTS    : SISHORT X - Left X-coordinate.
 *             SISHORT Y - Bottom (!) Y-coordinate.
 *             UNSHORT Width - Width in pixels.
 *             UNSHORT Height - Height in pixels.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_update_rectangle(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	Rectangle_2D *Rect;
	UNSHORT W, H;
	UNSHORT oX, oY;
	UNSHORT i,j,k;
	SISHORT X2, Y2;

	X2 = X + Width;
	Y2 = Y;
	Y = Y2 - Height + 1;

	X = X >> 4;
	Y = Y >> 4;
	X2 = (X2 + 15) >> 4;
	Y2 = (Y2 + 15) >> 4;

	/* Insert in 2D rectangle list */
	Rect = &Rectangle_list[Nr_2D_rectangles];

	Rect->X = X;
	Rect->Y = Y;
	Rect->X2 = X2;
	Rect->Y2 = Y2;

	Nr_2D_rectangles++;

	/* Adapt to camera coordinates */
	X -= Camera_2D_X;
	Y -= Camera_2D_Y;
	X2 -= Camera_2D_X;
	Y2 -= Camera_2D_Y;

	/* Exit if completely out of the viewport */
	if ((X2 < 0) || (X > MAPBUF_WIDTH) || (Y2 < 0) || (Y > MAPBUF_HEIGHT))
		return;

	/* Calculate width and height */
	if (X < 0)
	{
		W = X2;
		X = 0;
	}
	else
	{
		W = X2 - X;
	}

	if (Y < 0)
	{
		H = Y2;
		Y = 0;
	}
	else
	{
		H = Y2 - Y;
	}

	if (X2 >= MAPBUF_WIDTH)
		W = MAPBUF_WIDTH - X;

	if (Y2 >= MAPBUF_HEIGHT)
		H = MAPBUF_HEIGHT - Y;

	/* Anything left ? */
	if (!W || !H)
		return;

	/* Set rectangle in update buffer */
	for (i=0;i<H;i++)
	{
		for (j=0;j<W;j++)
		{
			for (k=0;k<MAPBUF_DEPTH;k++)
			{
					Update_buffer[Y + i + k][X + j] |= (1 << k);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_erase_rectangles
 * FUNCTION  : Set update rectangles from previous frame.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 16:30
 * LAST      : 28.07.94 16:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_erase_rectangles(void)
{
	SISHORT X,Y,X2,Y2;
	UNSHORT W,H;
	UNSHORT h,i,j,k;
	UNSHORT wX,wY;

	for (h=0;h<Nr_2D_rectangles;h++)
	{
		/* Get rectangle data from previous frame */
		X = Rectangle_list[h].X;
		Y = Rectangle_list[h].Y;
		X2 = Rectangle_list[h].X2;
		Y2 = Rectangle_list[h].Y2;

		/* Adapt to camera coordinates */
		X -= Camera_2D_X;
		Y -= Camera_2D_Y;
		X2 -= Camera_2D_X;
		Y2 -= Camera_2D_Y;

		/* Exit if completely out of the viewport */
		if ((X2 < 0) || (X > MAPBUF_WIDTH) || (Y2 < 0) || (Y > MAPBUF_HEIGHT))
			continue;

		/* Calculate width and height */
		W = X2 - X;
		H = Y2 - Y;

		/* Anything left ? */
		if (!W || !H)
			continue;

		/* Make sure initial coordinates are within viewport */
		if (X < 0)
			X += MAPBUF_WIDTH;

		if (X >= MAPBUF_WIDTH)
			X -= MAPBUF_WIDTH;

		if (Y < 0)
			Y += MAPBUF_HEIGHT;

		if (Y >= MAPBUF_HEIGHT)
			Y -= MAPBUF_HEIGHT;

		/* Set rectangle in update buffer */
		wY = Y;
		for (i=0;i<H;i++)
		{
			wX = X;
			for (j=0;j<W;j++)
			{
				for (k=0;k<MAPBUF_DEPTH;k++)
				{
					Update_buffer[wY + k][wX] |= (1 << k);
				}
				wX++;
				if (wX >= MAPBUF_WIDTH)
					wX -= MAPBUF_WIDTH;
			}
			wY++;
			if (wY >= MAPBUF_HEIGHT)
				wY -= MAPBUF_HEIGHT;
		}
	}

	/* Clear counter */
	Nr_2D_rectangles = 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.08.94 12:30
 * LAST      : 01.08.94 12:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Fill_map_buffer(void)
{
	Icon_2D_data *Data;
	UNBYTE *Ptr, *Ptr2;
	UNSHORT B1,B2,B3;
	UNSHORT Underlay,Overlay;
	UNSHORT hUnder = 0, hOver;
	UNSHORT i,j,k;

	/* Calculate camera coordinates */
	Camera_2D_X = (UNSHORT)(Scroll_2D.Playfield_X >> 4);
	Camera_2D_Y = (UNSHORT)(Scroll_2D.Playfield_Y >> 4);

	/* Calculate pointer to map layer */
	Ptr = (UNBYTE *) Map_ptr
	 + sizeof(Map_data)
	 + 32 * sizeof(NPC_data)
	 + 3 * (Camera_2D_X + Camera_2D_Y * Map_width);

	for (i=0;i<MAPBUF_HEIGHT;i++)
	{
		Ptr2 = Ptr;

		for (j=0;j<MAPBUF_WIDTH;j++)
		{
			for (k=0;k<MAPBUF_DEPTH;k++)
			{
				Map_buffer[i+k][j][k][0] = 0;
				Map_buffer[i+k][j][k][1] = 0;
			}

			/* Read bytes from map */
			B1 = (UNSHORT) *Ptr2++;
			B2 = (UNSHORT) *Ptr2++;
			B3 = (UNSHORT) *Ptr2++;

			/* Build overlay and underlay number */
			Underlay = ((B2 & 0x0F) << 8) | B3;
			Overlay = (B1 << 4) | (B2 >> 4);

			/* Any underlay ? */
			if (Underlay > 1)
			{
				/* Yes -> Get icon data */
				Data = &Icon_data[0][Underlay-1];

				/* Get icon height */
				hUnder = (Data->Flags & ICON_HEIGHT) >> ICON_HEIGHT_B;

				/* Insert underlay in map buffer */
				Map_buffer[i+hUnder][j][hUnder][0] = Underlay;
			}

			/* Any overlay ? */
			if (Overlay > 1)
			{
				/* Yes -> Get icon data */
				Data = &Icon_data[0][Overlay-1];

				/* Get icon height */
				hOver = (Data->Flags & ICON_HEIGHT) >> ICON_HEIGHT_B;

				/* Underlay over overlay ? */
				if (hUnder > hOver)
					/* Yes -> Bump up */
					hOver = hUnder;

				/* Insert overlay in map buffer */
				Map_buffer[i+hOver][j][hOver][1] = Overlay;
			}
		}
		Ptr += 3 * Map_width;
	}
}

