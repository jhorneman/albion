/************
 * NAME     : 2D_DISPL.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* pragmas */

#pragma off (unreferenced);

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBOPM.H>
#include <BBEVENT.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <ALBION.H>

#include <XLOAD.H>
#include <SCROLL.H>
#include <GFXFUNC.H>
#include <SORT.H>
#include <TEXT.H>

#include <CONTROL.H>
#include <MAP.H>
#include <GAME.H>
#include <EVELOGIC.H>
#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <2D_MAP.H>
#include <2D_DISPL.H>
#include <ANIMATE.H>
#include <XFTYPES.H>
#include <COLOURS.H>
#include <SPECITEM.H>

#include <STATAREA.H>

/* defines */

#define MAX_2D_OBJECTS			(120)
#define MAX_2D_RECTANGLES		(MAX_2D_OBJECTS + 40)

#define MAX_2D_CAMERA_SPEED	(8)

/* structure definitions */

/* 2D rectangles */
/* (coordinates are in map squares counted from 0, 0 !) */
struct Rectangle_2D {
	SISHORT X, Y;
	SISHORT X2, Y2;
};

/* prototypes */

/* 2D map display support functions */
void Display_extra_icons(UNSHORT Underlay_nr, UNSHORT Overlay_nr, UNSHORT X,
 UNSHORT Y, UNSHORT Hash_nr);
void Display_underlay(UNSHORT Underlay_nr, UNSHORT X, UNSHORT Y,
 UNSHORT Hash_nr);
void Display_overlay(UNSHORT Overlay_nr, UNSHORT X, UNSHORT Y,
 UNSHORT Hash_nr);
void Display_2D_object(struct Object_2D *Object);

/* 2D object sort functions */
void Swap_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data);

/* 2D map display support functions */
void Set_animation_update(void);
void Set_update_rectangle(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height);
void Set_erase_rectangles(void);
void Fill_map_buffer(void);

#ifdef DEBUG

/* Diagnostic functions */
void Display_underlay_diagnostics(struct OPM *OPM, UNSHORT X, UNSHORT Y,
 UNLONG Flags);
void Display_overlay_diagnostics(struct OPM *OPM, UNSHORT X, UNSHORT Y,
 UNLONG Flags);
void Display_event_diagnostics(struct OPM *OPM, UNSHORT X, UNSHORT Y,
 UNSHORT Map_X, UNSHORT Map_Y);

#endif

/* global variables */

static BOOLEAN _2D_camera_locked = FALSE;

static UNBYTE *Icon_graphics_ptr;
static struct Icon_2D_data *Icon_data_ptr;

struct Scroll_data Scroll_2D;

struct OPM *Current_2D_OPM = NULL;

static struct Object_2D *Object_2D_list[MAX_2D_OBJECTS];
static UNSHORT Nr_2D_objects = 0;
static Nr_2D_rectangles = 0;

static struct Rectangle_2D Rectangle_list[MAX_2D_RECTANGLES];

UNSHORT Mapbuf_width, Mapbuf_height;
UNSHORT Map_2D_width, Map_2D_height;

UNSHORT Mapbuf_X, Mapbuf_Y;

static UNLONG Camera_2D_X, Camera_2D_Y;
static UNLONG Camera_2D_target_X, Camera_2D_target_Y;

static UNSHORT Map_buffer[MAX_MAPBUF_HEIGHT+3][MAX_MAPBUF_WIDTH][MAPBUF_DEPTH][2];
static UNBYTE Update_buffer[MAX_MAPBUF_HEIGHT+3][MAX_MAPBUF_WIDTH];

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Prepare_2D_display
 * FUNCTION  : Prepare internal variables for display.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 19:23
 * LAST      : 14.11.94 19:23
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Prepare_2D_display(void)
{
	/* Clear map buffers */
	BASEMEM_FillMemLong
	(
		(UNBYTE *) &(Map_buffer[0][0][0][0]),
		(MAX_MAPBUF_HEIGHT + 3) * MAX_MAPBUF_WIDTH * MAPBUF_DEPTH * 4,
		0L
	);

	/* Clear update buffer */
	BASEMEM_FillMemByte
	(
		&(Update_buffer[0][0]),
		(MAX_MAPBUF_HEIGHT + 3) * MAX_MAPBUF_WIDTH,
		0
	);

	/* Calculate target camera coordinates */
	Calculate_2D_camera_target_position();

	/* Set camera position */
	Set_2D_camera_position(Camera_2D_target_X, Camera_2D_target_Y);

	/* Initialize scrolling */
	Icon_data_ptr = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle);
	Icon_graphics_ptr = MEM_Claim_pointer(Icongfx_handle);

	Init_scroll
	(
		&Scroll_2D,
		Camera_2D_X,
		Camera_2D_Y
	);

	MEM_Free_pointer(Icongfx_handle);
	MEM_Free_pointer(Icondata_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_2D_camera_target_position
 * FUNCTION  : Calculate the desired position of the camera in a 2D map.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.04.95 10:16
 * LAST      : 12.08.95 11:56
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_2D_camera_target_position(void)
{
	/* Calculate target camera coordinates */
	Camera_2D_target_X = max(0, ((SILONG) Party_object_X -
	 ((SILONG) Map_2D_width / 2) + 16));
	Camera_2D_target_Y = max(0, ((SILONG) Party_object_Y -
	 ((SILONG) Map_2D_height / 2) - 8));

/*	if (Preview_direction != 0xFFFF)
	{
		Camera_2D_targetX += 32 * Offsets8[Preview_direction][0];
		Camera_2D_targetY += 32 * Offsets8[Preview_direction][1];
	} */
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_2D_map
 * FUNCTION  : Draw all objects and animated parts of the 2D map.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 18:54
 * LAST      : 04.10.95 15:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_2D_map(void)
{
	struct Object_2D *Ptr;
	SILONG dX, dY;
	SISHORT X_speed, Y_speed;
	SISHORT X, Y;
	UNSHORT Remaining_2D_objects;
	UNSHORT U;
	UNSHORT Current_object = 0;
	UNSHORT objW, objH;
	UNSHORT Hash_nr;
	UNSHORT i, j, k, l, m, n;

	/* Is the camera locked ? */
	if (!_2D_camera_locked)
	{
		/* No -> Calculate target camera coordinates */
		Calculate_2D_camera_target_position();
	}

	/* Calculate vector from current to target camera coordinates */
	dX = Camera_2D_target_X - Camera_2D_X;
	dY = Camera_2D_target_Y - Camera_2D_Y;

	/* Determine camera speed */
	X_speed = ((abs(dX) + MAX_2D_CAMERA_SPEED - 1) /
	 MAX_2D_CAMERA_SPEED) * sgn(dX);
	Y_speed = ((abs(dY) + MAX_2D_CAMERA_SPEED - 1) /
	 MAX_2D_CAMERA_SPEED) * sgn(dY);

	/* Move camera */
	Move_2D_camera(X_speed, Y_speed);

	/* Get pointers */
	Icon_data_ptr = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle);
	Icon_graphics_ptr = MEM_Claim_pointer(Icongfx_handle);

	/* Scroll to 2D camera position */
	Set_scroll_position(&Scroll_2D, Camera_2D_X, Camera_2D_Y);

	/* Set map buffer coordinates */
	Mapbuf_X = (UNSHORT)(Scroll_2D.Playfield_X / 16);
	Mapbuf_Y = (UNSHORT)(Scroll_2D.Playfield_Y / 16);

	/* Read the currently visible part of the map */
	Fill_map_buffer();

	/* Has the map been changed ? */
	if (Map_changed)
	{
		/* Yes -> Force a redraw of the entire map */
		Redraw_2D_map();

		/* Clear flag */
		Map_changed = FALSE;
	}
	else
	{
		/* No -> Prepare the display of special items */
		Prepare_2D_special_items_display();

		/* Arrange for the removal of previous objects */
		Set_erase_rectangles();

		/* Arrange for the updating of animated parts */
	  	Set_animation_update();
	}

	/* Clear counter */
	Nr_2D_rectangles = 0;

	/* Prepare the display of special items (again) */
	Prepare_2D_special_items_display();

	/* Update current objects */
	for (i=0;i<Nr_2D_objects;i++)
	{
		Ptr = Object_2D_list[i];

		/* Set update rectangle */
		Set_update_rectangle(Ptr->X, Ptr->Y - (Ptr->Level * 16),
		 Ptr->Width, Ptr->Height);

		/* Calculate display index */
		Ptr->Index = ((Ptr->X + Ptr->Width - 1) / 16) - Mapbuf_X
		 + ((Ptr->Y / 16) - Mapbuf_Y) * Mapbuf_width;
	}

	/* Sort 2D objects */
	Shuttlesort(Swap_2D_objects, Compare_2D_objects, Nr_2D_objects, NULL);

	/* Calculate initial hash number */
	Hash_nr = (Mapbuf_Y * Map_width) + Mapbuf_X;

	/* Display 2D objects */
	Remaining_2D_objects = Nr_2D_objects;

	for (i=0;i<Mapbuf_height+3;i++)
	{
		for (j=0;j<Mapbuf_width;j++)
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
						if (((i-k) >= 0) && ((i-k) < Mapbuf_height))
						{
							/* Yes -> Display underlay and overlay */
							Display_underlay(Map_buffer[i][j][k][0], j, i-k, Hash_nr);
							Display_overlay(Map_buffer[i][j][k][1], j, i-k, Hash_nr);
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
				if (Ptr->Index <= i * Mapbuf_width + j)
				{
					/* Yes -> Draw */
					Display_2D_object(Ptr);

					/* Display extra icons ? */
					if (Ptr->Flags & ADD_ICONS_OVER_OBJECT)
					{
						/* Get object dimensions */
						objW = (Ptr->Width + 15) / 16;
						objH = (Ptr->Height + 15) / 16;

						/* Get object position */
						X = j - objW + 1;
						Y = i - objH + 1;

						/* Display extra icons over object */
						for (l=0;l<objH;l++)
						{
							for (m=0;m<objW;m++)
							{
								for (n=0;n<MAPBUF_DEPTH-1;n++)
								{
									if ((X + m >= 0) && (X + m < Mapbuf_width) &&
									 (Y + l >= 0) && (Y + l < Mapbuf_height+3))
									{
										Display_extra_icons(Map_buffer[Y + l][X + m][n][0],
										 Map_buffer[Y + l][X + m][n][1], X + m, Y + l - n, Hash_nr);
									}
								}
							}
						}
					}

					/* Count down */
					Remaining_2D_objects--;
					Current_object++;
				}
				else
				{
					/* No -> Exit */
					break;
				}
			}

			/* Update hash number */
			Hash_nr++;
		}

		/* Update hash number */
		Hash_nr += (Map_width - Mapbuf_width);
	}

	/* Calculate initial hash number */
	Hash_nr = (Mapbuf_Y * Map_width) + Mapbuf_X;

	/* Display top level */
	k = MAPBUF_DEPTH-1;
	for (i=0;i<Mapbuf_height+3;i++)
	{
		for (j=0;j<Mapbuf_width;j++)
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
					if (((i-k) >= 0) && ((i-k) < Mapbuf_height))
					{
						/* Yes -> Display underlay and overlay */
						Display_underlay(Map_buffer[i][j][k][0], j, i-k, Hash_nr);
						Display_overlay(Map_buffer[i][j][k][1], j, i-k, Hash_nr);
					}
				}
			}

			/* Update hash number */
			Hash_nr++;
		}

		/* Update hash number */
		Hash_nr += (Map_width - Mapbuf_width);
	}

	/* Display special items in 2D */
	Display_special_items_2D();

	/* Draw 2D map scroll buffer */
	Draw_2D_scroll_buffer();

	/* Free pointers */
	MEM_Free_pointer(Icongfx_handle);
	MEM_Free_pointer(Icondata_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Redraw_2D_objects
 * FUNCTION  : Redraw all 2D objects.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 14:32
 * LAST      : 11.08.95 15:24
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This is quite a dirty function which redraws the 2D objects
 *              directly onto the main OPM. It is needed to view animations
 *              in the 2D map.
 *             - This function uses the objects currently in the object list.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Redraw_2D_objects(void)
{
	struct Object_2D *Ptr;
	SILONG X, Y;
	UNSHORT i, j;
	UNSHORT Remaining_2D_objects;
	UNSHORT Current_object = 0;
	UNBYTE *Gfx_ptr;

	/* Display 2D objects */
	Remaining_2D_objects = Nr_2D_objects;

	for (i=0;i<Mapbuf_height+3;i++)
	{
		for (j=0;j<Mapbuf_width;j++)
		{
			/* Any objects left ? */
			while (Remaining_2D_objects)
			{
				/* Yes -> Get data */
				Ptr = Object_2D_list[Current_object];

				/* Draw this one ? */
				if (Ptr->Index <= i * Mapbuf_width + j)
				{
					/* Yes -> Calculate object coordinates in playfield */
					X = (SILONG) Ptr->X;
					Y = (SILONG) (Ptr->Y - (Ptr->Level * 16) -
					 Ptr->Height + 1);

					/* Convert to screen coordinates */
					X -= Scroll_2D.Playfield_X;
					Y -= Scroll_2D.Playfield_Y;

					/* Get graphics address */
					Gfx_ptr = MEM_Claim_pointer(Ptr->Graphics_handle) +
					 Ptr->Graphics_offset;

					/* Display object */
					Put_masked_block(&Main_OPM, (SISHORT) X, (SISHORT) Y,
					 Ptr->Width, Ptr->Height, Gfx_ptr);

					MEM_Free_pointer(Ptr->Graphics_handle);

					/* Count down */
					Remaining_2D_objects--;
					Current_object++;
				}
				else
				{
					/* No -> Exit */
					break;
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_2D_scroll_buffer
 * FUNCTION  : Draw the 2D map scroll buffer.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.04.95 20:34
 * LAST      : 11.08.95 15:23
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_2D_scroll_buffer(void)
{
	/* OPM given ? */
	if (Current_2D_OPM)
	{
		/* Yes -> Display 2D map scroll buffer in OPM */
		Scroll_display_OPM(&Scroll_2D, Current_2D_OPM, MAP_2D_X, MAP_2D_Y);
	}
	else
	{
		/* No -> Display 2D map scroll buffer directly on the screen */
		Scroll_display(&Scroll_2D, MAP_2D_X, MAP_2D_Y);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_extra_icons
 * FUNCTION  : Display extra icons covering any objects.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.09.94 11:50
 * LAST      : 25.07.95 14:26
 * INPUTS    : UNSHORT Underlay_nr - Number from map / 0 (empty).
 *             UNSHORT Overlay_nr - Number from map / 0 (empty).
 *             UNSHORT X - X-coordinate relative to camera.
 *             UNSHORT Y - Y-coordinate relative to camera.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_extra_icons(UNSHORT Underlay_nr, UNSHORT Overlay_nr, UNSHORT X,
 UNSHORT Y, UNSHORT Hash_nr)
{
	SILONG nX, nY;
	struct Icon_2D_data *Data;
	UNBYTE *Icon;
	UNSHORT Frame;

	Underlay_nr &= 0x7fff;
	Overlay_nr &= 0x7fff;

	/* Get scroll buffer coordinates */
	nX = ((SILONG) (Mapbuf_X + X)) * 16;
	nY = ((SILONG) (Mapbuf_Y + Y)) * 16;
	Convert_scroll_coordinates(&Scroll_2D, &nX, &nY);

	/* Exit if coordinates lie outside the scroll buffer */
	if ((nX == -32768) && (nY == -32768))
		return;

	/* Any underlay ? */
	if (Underlay_nr)
	{
		/* Yes -> Get icon data */
		Data = &(Icon_data_ptr[Underlay_nr-2]);

		/* Visible ? */
		if (!(Data->Flags & ICON_INVISIBLE))
		{
			/* Add next icon ? */
			if (Data->Flags & ADD_NEXT_ICON)
			{
				/* Yes -> Get NEXT icon's data */
				Data = &(Icon_data_ptr[Underlay_nr-1]);

				/* Get frame number */
				Frame = Data->First_frame + Get_animation_frame(Data->Flags,
			 	 Data->Nr_frames, Hash_nr);

				/* Get graphics address */
				Icon = Icon_graphics_ptr + 256 * Frame;

				/* Display underlay (masked!) */
			 	Put_masked_block(&(Scroll_2D.Base_OPM), nX, nY, 16, 16, Icon);
			}
		}
	}

	/* Any overlay ? */
	if (Overlay_nr)
	{
		/* Yes -> Get icon data */
		Data = &(Icon_data_ptr[Overlay_nr-2]);

		/* Visible ? */
		if (!(Data->Flags & ICON_INVISIBLE))
		{
			/* Add next icon ? */
			if (Data->Flags & ADD_NEXT_ICON)
			{
				/* Yes -> Get NEXT icon's data */
				Data = &(Icon_data_ptr[Overlay_nr-1]);

				/* Get frame number */
				Frame = Data->First_frame + Get_animation_frame(Data->Flags,
				 Data->Nr_frames, Hash_nr);

				/* Get graphics address */
				Icon = Icon_graphics_ptr + 256 * Frame;

				/* Display underlay */
				Put_masked_block(&(Scroll_2D.Base_OPM), nX, nY, 16, 16, Icon);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_underlay
 * FUNCTION  : Display an underlay in the scroll buffer.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 18:54
 * LAST      : 25.07.95 14:26
 * INPUTS    : UNSHORT Underlay_nr - Number from map / 0 (empty).
 *             UNSHORT X - X-coordinate relative to camera.
 *             UNSHORT Y - Y-coordinate relative to camera.
 *             UNSHORT Hash_nr - Number used for animation hashing.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_underlay(UNSHORT Underlay_nr, UNSHORT X, UNSHORT Y, UNSHORT Hash_nr)
{
	SILONG nX, nY;
	struct Icon_2D_data *Data;
	UNBYTE *Icon;
	UNSHORT Frame;

	Underlay_nr &= 0x7fff;

	/* Get scroll buffer coordinates */
	nX = ((SILONG) (Mapbuf_X + X)) * 16;
	nY = ((SILONG) (Mapbuf_Y + Y)) * 16;
	Convert_scroll_coordinates(&Scroll_2D, &nX, &nY);

	/* Exit if coordinates lie outside the scroll buffer */
	if ((nX == -32768) && (nY == -32768))
		return;

	/* Any underlay ? */
	if (Underlay_nr)
	{
		/* Yes -> Get frame number */
		Data = &(Icon_data_ptr[Underlay_nr-2]);

		/* Visible ? */
		if (!(Data->Flags & ICON_INVISIBLE))
		{
			/* Yes -> Get frame number */
			Frame = Data->First_frame + Get_animation_frame(Data->Flags,
		 	 Data->Nr_frames, Hash_nr);

			/* Get graphics address */
			Icon = Icon_graphics_ptr + 256 * Frame;

			/* Display underlay */
		 	Put_unmasked_block(&(Scroll_2D.Base_OPM), nX, nY, 16, 16, Icon);
		}

		#ifdef DEBUG
		if (Diagnostic_mode == 1)
			Display_underlay_diagnostics(&(Scroll_2D.Base_OPM), nX, nY, Data->Flags);
		#endif
	}

	#ifdef DEBUG
	if (Diagnostic_mode == 2)
		Display_event_diagnostics(&(Scroll_2D.Base_OPM), nX, nY, Mapbuf_X + X + 1,
		 Mapbuf_Y + Y + 1);
	#endif
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_overlay
 * FUNCTION  : Display an overlay in the scroll buffer.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 18:54
 * LAST      : 25.07.95 14:26
 * INPUTS    : UNSHORT Overlay_nr - Number from map / 0 (empty).
 *             UNSHORT X - X-coordinate relative to camera.
 *             UNSHORT Y - Y-coordinate relative to camera.
 *             UNSHORT Hash_nr - Number used for animation hashing.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_overlay(UNSHORT Overlay_nr, UNSHORT X, UNSHORT Y, UNSHORT Hash_nr)
{
	SILONG nX, nY;
	struct Icon_2D_data *Data;
	UNBYTE *Icon;
	UNSHORT Frame;

	Overlay_nr &= 0x7fff;

	/* Get scroll buffer coordinates */
	nX = ((SILONG) (Mapbuf_X + X)) * 16;
	nY = ((SILONG) (Mapbuf_Y + Y)) * 16;
	Convert_scroll_coordinates(&Scroll_2D, &nX, &nY);

	/* Exit if coordinates lie outside the scroll buffer */
	if ((nX == -32768) && (nY == -32768))
		return;

	/* Any overlay ? */
	if (Overlay_nr)
	{
		/* Yes -> Get frame number */
		Data = &(Icon_data_ptr[Overlay_nr-2]);

		/* Visible ? */
		if (!(Data->Flags & ICON_INVISIBLE))
		{
			/* Yes -> Get frame number */
			Frame = Data->First_frame + Get_animation_frame(Data->Flags,
		 	 Data->Nr_frames, Hash_nr);

			/* Get graphics address */
			Icon = Icon_graphics_ptr + 256 * Frame;

			/* Display overlay */
			Put_masked_block(&(Scroll_2D.Base_OPM), nX, nY, 16, 16, Icon);
		}

		#ifdef DEBUG
		if (Diagnostic_mode == 1)
			Display_overlay_diagnostics(&(Scroll_2D.Base_OPM), nX, nY, Data->Flags);
		#endif
	}

	#ifdef DEBUG
	if (Diagnostic_mode == 2)
		Display_event_diagnostics(&(Scroll_2D.Base_OPM), nX, nY, Mapbuf_X + X + 1,
		 Mapbuf_Y + Y + 1);
	#endif
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_2D_object
 * FUNCTION  : Display a 2D object in the scroll buffer.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 18:54
 * LAST      : 16.02.95 12:58
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
	SILONG X, Y;
	UNSHORT objW, objH;
	UNBYTE *Ptr;

	/* Get object dimensions */
	objW = Object->Width;
	objH = Object->Height;

	/* Calculate object coordinates in playfield */
	X = (SILONG) Object->X;
	Y = (SILONG) (Object->Y - (Object->Level * 16) - objH + 1);

	/* Get graphics address */
	Ptr = MEM_Claim_pointer(Object->Graphics_handle) + Object->Graphics_offset;

	/* Display object */
	Display_object_in_scroll_buffer(&Scroll_2D, X, Y, objW, objH, Ptr);

	MEM_Free_pointer(Object->Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Swap_2D_objects
 * FUNCTION  : Swap two 2D objects (for sorting).
 * FILE      : 2D_DISPL.C
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
	struct Object_2D *T;

	T = Object_2D_list[A];
	Object_2D_list[A] = Object_2D_list[B];
	Object_2D_list[B] = T;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Compare_2D_objects
 * FUNCTION  : Compare two 2D objects (for sorting).
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 19:07
 * LAST      : 17.07.95 14:54
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : BOOLEAN : element A should come after element B.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Compare_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Object_2D *O1, *O2;

	O1 = Object_2D_list[A];
	O2 = Object_2D_list[B];

	/* Are the indices identical ? */
	if (O1->Index == O2->Index)
	{
		/* Yes -> Are the Y-coordinates identical ? */
		if (O1->Y == O2->Y)
		{
			/* Yes -> The object with lowest X-coordinate comes first */
			return(O1->X > O2->X);
		}
		else
		{
			/* No -> The object with lowest Y-coordinate comes first */
			return(O1->Y > O2->Y);
		}
	}
	else
	{
		/* No -> The object with lowest index comes first */
		return(O1->Index > O2->Index);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_animation_update
 * FUNCTION  : Mark animated map squares that must be updated.
 * FILE      : 2D_DISPL.C
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
	UNSHORT i, j, k, l;

	/* Check the entire map buffer */
	for (i=0;i<Mapbuf_height+3;i++)
	{
		for (j=0;j<Mapbuf_width;j++)
		{
			/* Do layers */
			for (k=0;k<MAPBUF_DEPTH;k++)
			{
				/* Animated underlay ? */
				if (Map_buffer[i][j][k][0] & 0x8000)
				{
					/* Yes -> Update */
					for (l=0;l<MAPBUF_DEPTH;l++)
					{
						if (((i-k+l) >= 0) && ((i-k+l) < Mapbuf_height + 3))
							Update_buffer[i-k+l][j] |= (1 << l);
					}
				}

				/* Animated overlay ? */
				if (Map_buffer[i][j][k][1] & 0x8000)
				{
					/* Yes -> Update */
					for (l=0;l<MAPBUF_DEPTH;l++)
					{
						if (((i-k+l) >= 0) && ((i-k+l) < Mapbuf_height + 3))
							Update_buffer[i-k+l][j] |= (1 << l);
					}
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_map_area
 * FUNCTION  : Update a 2D map area.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.95 13:14
 * LAST      : 23.08.95 22:14
 * INPUTS    : SISHORT X - Left X-coordinate in map squares (1...).
 *             SISHORT Y - Top Y-coordinate in map squares (1...).
 *             UNSHORT Width - Width of area in map squares.
 *             UNSHORT Height - Height of area in map squares.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function is most useful to make sure parts of the map
 *              are re-drawn after an icon change.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_2D_map_area(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	struct Rectangle_2D *Rect;

	/* Exit if the 2D rectangle list is full */
	if (Nr_2D_rectangles == MAX_2D_RECTANGLES)
		return;

	/* Insert in 2D rectangle list */
	Rect = &Rectangle_list[Nr_2D_rectangles];

	Rect->X	= X - 1;
	Rect->Y	= Y - 1;
	Rect->X2	= X + Width - 1;
	Rect->Y2	= Y + Height - 1;

	Nr_2D_rectangles++;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_2D_screen_area
 * FUNCTION  : Update a 2D screen area.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.95 13:14
 * LAST      : 23.08.95 22:14
 * INPUTS    : SISHORT X - Left X-coordinate in pixels (0...).
 *             SISHORT Y - Top Y-coordinate in pixels (0...).
 *             UNSHORT Width - Width of area in pixels.
 *             UNSHORT Height - Height of area in pixels.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_2D_screen_area(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	struct Rectangle_2D *Rect;
	SISHORT X2, Y2;

	/* Exit if the 2D rectangle list is full */
	if (Nr_2D_rectangles == MAX_2D_RECTANGLES)
		return;

	X += Scroll_2D.Playfield_X;
	Y += Scroll_2D.Playfield_Y;

	X2 = X + Width;
	Y2 = Y + Height;

	X = X / 16;
	Y = Y / 16;
	X2 = (X2 + 15) / 16;
	Y2 = (Y2 + 15) / 16;

	/* Insert in 2D rectangle list */
	Rect = &Rectangle_list[Nr_2D_rectangles];

	Rect->X	= X;
	Rect->Y	= Y;
	Rect->X2	= X2;
	Rect->Y2	= Y2;

	Nr_2D_rectangles++;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_update_rectangle
 * FUNCTION  : Set update rectangle.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 16:20
 * LAST      : 28.07.94 16:20
 * INPUTS    : SISHORT X - Left X-coordinate in pixels.
 *             SISHORT Y - Bottom (!) Y-coordinate in pixels.
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
	struct Rectangle_2D *Rect;
	SISHORT X2, Y2;
	UNSHORT W, H;
	UNSHORT i, j, k;

	X2 = X + Width;
	Y2 = Y;
	Y = Y2 - Height + 1;

	X = X / 16;
	Y = Y / 16;
	X2 = (X2 + 15) / 16;
	Y2 = (Y2 + 16) / 16;

	/* Exit if the 2D rectangle list is full */
	if (Nr_2D_rectangles == MAX_2D_RECTANGLES)
		return;

	/* Insert in 2D rectangle list */
	Rect = &Rectangle_list[Nr_2D_rectangles];

	Rect->X	= X;
	Rect->Y	= Y;
	Rect->X2	= X2;
	Rect->Y2	= Y2;

	Nr_2D_rectangles++;

	/* Adapt to camera coordinates */
	X -= Mapbuf_X;
	Y -= Mapbuf_Y;
	X2 -= Mapbuf_X;
	Y2 -= Mapbuf_Y;

	/* Exit if completely out of the viewport */
	if ((X2 < 0) || (X > Mapbuf_width) || (Y2 < 0) || (Y > Mapbuf_height))
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

	if (X2 >= Mapbuf_width)
		W = Mapbuf_width - X;

	if (Y2 >= Mapbuf_height)
		H = Mapbuf_height - Y;

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
 * NAME      : Redraw_2D_map
 * FUNCTION  : Redraw the entire 2D map.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.11.94 10:55
 * LAST      : 08.11.94 10:55
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Redraw_2D_map(void)
{
	UNSHORT i, j, k;

	for (i=0;i<Mapbuf_height;i++)
	{
		for (j=0;j<Mapbuf_width;j++)
		{
			for (k=0;k<MAPBUF_DEPTH;k++)
			{
				Update_buffer[i + k][j] |= (1 << k);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_erase_rectangles
 * FUNCTION  : Set update rectangles from previous frame.
 * FILE      : 2D_DISPL.C
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
	SISHORT X, Y;
	SISHORT X2, Y2;
	UNSHORT W, H;
	UNSHORT h, i, j, k;
	UNSHORT wX, wY;

	for (h=0;h<Nr_2D_rectangles;h++)
	{
		/* Get rectangle data from previous frame */
		X	= Rectangle_list[h].X;
		Y	= Rectangle_list[h].Y;
		X2	= Rectangle_list[h].X2;
		Y2	= Rectangle_list[h].Y2;

		/* Adapt to camera coordinates */
		X -= Mapbuf_X;
		Y -= Mapbuf_Y;
		X2 -= Mapbuf_X;
		Y2 -= Mapbuf_Y;

		/* Exit if completely out of the viewport */
		if ((X2 < 0) || (X > Mapbuf_width) || (Y2 < 0) || (Y > Mapbuf_height))
			continue;

		/* Calculate width and height */
		W = X2 - X;
		H = Y2 - Y;

		/* Anything left ? */
		if (!W || !H)
			continue;

		/* Make sure initial coordinates are within viewport */
		if (X < 0)
			X += Mapbuf_width;

		if (X >= Mapbuf_width)
			X -= Mapbuf_width;

		if (Y < 0)
			Y += Mapbuf_height;

		if (Y >= Mapbuf_height)
			Y -= Mapbuf_height;

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
				if (wX >= Mapbuf_width)
					wX -= Mapbuf_width;
			}
			wY++;
			if (wY >= Mapbuf_height)
				wY -= Mapbuf_height;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fill_map_buffer
 * FUNCTION  : Fill map buffer.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.08.94 12:30
 * LAST      : 11.05.95 11:57
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
	struct Icon_2D_data *Data;
	struct Square_2D *Ptr, *Ptr2;
	UNSHORT Underlay_nr, Overlay_nr;
	UNSHORT hUnder = 0, hOver;
	UNSHORT i, j;
	UNBYTE B1, B2, B3;

	/* Clear map buffer */
	BASEMEM_FillMemLong
	(
		(UNBYTE *) &(Map_buffer[0][0][0][0]),
		(MAX_MAPBUF_HEIGHT + 3) * MAX_MAPBUF_WIDTH * MAPBUF_DEPTH * 4,
		0L
	);

	/* Calculate pointer to map layer */
	Ptr = (struct Square_2D *) (MEM_Claim_pointer(Map_handle) +
	 Map_layers_offset);
	Ptr += Mapbuf_X + (Mapbuf_Y * Map_width);

	for (i=0;i<Mapbuf_height;i++)
	{
		Ptr2 = Ptr;

		for (j=0;j<Mapbuf_width;j++)
		{
			/* Read bytes from map */
			B1 = Ptr2->m[0];
			B2 = Ptr2->m[1];
			B3 = Ptr2->m[2];
			Ptr2++;

			/* Build overlay and underlay number */
			Underlay_nr = ((B2 & 0x0F) << 8) | B3;
			Overlay_nr = (B1 << 4) | (B2 >> 4);

			/* Any underlay ? */
			if (Underlay_nr > 1)
			{
				/* Yes -> Get icon data */
				Data = &(Icon_data_ptr[Underlay_nr-2]);

				/* Get icon height */
				hUnder = (Data->Flags & ICON_HEIGHT) >> ICON_HEIGHT_B;

				/* Animated ? */
				if (Data->Nr_frames > 1)
				{
					Underlay_nr |= 0x8000;
				}

				/* Insert underlay in map buffer */
				Map_buffer[i+hUnder][j][hUnder][0] = Underlay_nr;
			}

			/* Any overlay ? */
			if (Overlay_nr > 1)
			{
				/* Yes -> Get icon data */
				Data = &(Icon_data_ptr[Overlay_nr-2]);

				/* Get icon height */
				hOver = (Data->Flags & ICON_HEIGHT) >> ICON_HEIGHT_B;

				/* Underlay over overlay ? */
				if (hUnder > hOver)
				{
					/* Yes -> Bump up */
					hOver = hUnder;
				}

				/* Animated ? */
				if (Data->Nr_frames > 1)
				{
					Overlay_nr |= 0x8000;
				}

				/* Insert overlay in map buffer */
				Map_buffer[i+hOver][j][hOver][1] = Overlay_nr;
			}
		}
		Ptr += Map_width;
	}

	MEM_Free_pointer(Map_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_2D_object_list
 * FUNCTION  : Clear the 2D object list.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.05.95 17:20
 * LAST      : 10.05.95 17:20
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clear_2D_object_list(void)
{
	/* Reset list */
	Nr_2D_objects = 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_2D_object
 * FUNCTION  : Add a 2D object to the list.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.05.95 17:17
 * LAST      : 10.05.95 17:17
 * INPUTS    : struct Object_2D *Object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_2D_object(struct Object_2D *Object)
{
	/* Is there room in the list ? */
	if (Nr_2D_objects < MAX_2D_OBJECTS)
	{
		/* Yes -> Add object to object list */
		Object_2D_list[Nr_2D_objects] = Object;

		/* Count up */
		Nr_2D_objects++;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_2D_camera_position
 * FUNCTION  : Set the position of the 2D map camera.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 11:42
 * LAST      : 12.08.95 11:42
 * INPUTS    : UNLONG New_X - New camera X-coordinate.
 *             UNLONG New_Y - New camera Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_2D_camera_position(UNLONG New_X, UNLONG New_Y)
{
	/* Set camera position */
	Camera_2D_X = New_X;
	Camera_2D_Y = New_Y;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_2D_camera_position
 * FUNCTION  : Get the position of the 2D map camera.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 11:44
 * LAST      : 12.08.95 11:44
 * INPUTS    : UNLONG *X_ptr - Pointer to camera X-coordinate.
 *             UNLONG *Y_ptr - Pointer to camera Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_2D_camera_position(UNLONG *X_ptr, UNLONG *Y_ptr)
{
	/* Get camera position */
	*X_ptr = Camera_2D_X;
	*Y_ptr = Camera_2D_Y;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_2D_camera_target position
 * FUNCTION  : Set the target position of the 2D map camera.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 11:59
 * LAST      : 12.08.95 11:59
 * INPUTS    : UNLONG New_X - New target camera X-coordinate.
 *             UNLONG New_Y - New target camera Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_2D_camera_target_position(UNLONG New_X, UNLONG New_Y)
{
	/* Set target camera position */
	Camera_2D_target_X = New_X;
	Camera_2D_target_Y = New_Y;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_2D_camera_target position
 * FUNCTION  : Get the target position of the 2D map camera.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.08.95 01:02
 * LAST      : 13.08.95 01:02
 * INPUTS    : UNLONG *X_ptr - Pointer to camera X-coordinate.
 *             UNLONG *Y_ptr - Pointer to camera Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_2D_camera_target_position(UNLONG *X_ptr, UNLONG *Y_ptr)
{
	/* Get camera target position */
	*X_ptr = Camera_2D_target_X;
	*Y_ptr = Camera_2D_target_Y;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_2D_camera
 * FUNCTION  : Move the position of the 2D map camera.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 11:43
 * LAST      : 12.08.95 11:43
 * INPUTS    : SILONG dX - X-component of 2D camera movement vector.
 *             SILONG dY - Y-component of 2D camera movement vector.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_2D_camera(SILONG dX, SILONG dY)
{
	/* Change camera position */
	Camera_2D_X += dX;
	Camera_2D_Y += dY;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Lock_2D_camera
 * FUNCTION  : Lock the 2D map camera.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 11:38
 * LAST      : 26.10.95 22:10
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Lock_2D_camera(void)
{
	/* Is the camera already locked ? */
	if (!_2D_camera_locked)
	{
		#if FALSE
		/* No -> Wait until the camera has reached it's target */
		for(;;)
		{
			/* Has the camera reached it's target ? */
			if ((Camera_2D_target_X == Camera_2D_X) &&
			 (Camera_2D_target_Y == Camera_2D_Y))
			{
				/* Yes -> Break */
				break;
			}

			/* Update */
			Update_display();
			Switch_screens();
		}
		#endif

		/* Lock the 2D camera */
		_2D_camera_locked = TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Unlock_2D_camera
 * FUNCTION  : Unlock the 2D map camera.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 11:39
 * LAST      : 12.08.95 11:39
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Unlock_2D_camera(void)
{
	/* Unlock the 2D camera */
	_2D_camera_locked = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_2D_unit
 * FUNCTION  : Updates a scroll unit of the 2D map.
 * FILE      : 2D_DISPL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 16:00
 * LAST      : 28.07.94 16:00
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             UNSHORT Buffer_X - X-coordinate of unit in scroll buffer.
 *             UNSHORT Buffer_Y - Y-coordinate of unit in scroll buffer.
 *             UNSHORT Playfield_X - X-coordinate of unit in playfield.
 *             UNSHORT Playfield_Y - Y-coordinate of unit in playfield.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Several pointers (for instance to icon data and graphics)
 *              must have been initialized.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_2D_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X, UNSHORT Buffer_Y,
 UNSHORT Playfield_X, UNSHORT Playfield_Y)
{
	struct Square_2D *Map_ptr;
	struct Icon_2D_data *Data;
	UNBYTE *Icon;
	UNSHORT B1, B2, B3;
	UNSHORT Underlay_nr;
	UNSHORT Overlay_nr;

	/* Calculate pointer to map layer */
	Map_ptr = (struct Square_2D *) (MEM_Claim_pointer(Map_handle)
	 + Map_layers_offset);
	Map_ptr += Playfield_X + (Playfield_Y * Map_width);

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
		Data = &(Icon_data_ptr[Underlay_nr-2]);

		/* Visible ? */
		if (!(Data->Flags & ICON_INVISIBLE))
		{
			/* Animated ? */
			if (Data->Nr_frames == 1)
			{
				/* No -> Get graphics address */
				Icon = Icon_graphics_ptr + 256 * Data->First_frame;

				/* Display underlay */
				Put_unmasked_block(&(Scroll->Base_OPM), Buffer_X, Buffer_Y, 16, 16,
			 	 Icon);
			}
		}

		#ifdef DEBUG
		if (Diagnostic_mode == 1)
			Display_underlay_diagnostics(&(Scroll->Base_OPM), Buffer_X, Buffer_Y,
			 Data->Flags);
		#endif
	}

	/* Any overlay ? */
	if (Overlay_nr > 1)
	{
		/* Yes -> Get icon data */
		Data = &(Icon_data_ptr[Overlay_nr-2]);

		/* Visible ? */
		if (!(Data->Flags & ICON_INVISIBLE))
		{
			/* Yes -> Animated ? */
			if (Data->Nr_frames == 1)
			{
				/* No -> Get graphics address */
				Icon = Icon_graphics_ptr + 256 * Data->First_frame;

				/* Display overlay */
				Put_masked_block(&(Scroll->Base_OPM), Buffer_X, Buffer_Y, 16, 16,
			 	 Icon);

			}
		}

		#ifdef DEBUG
		if (Diagnostic_mode == 1)
		{
			Display_overlay_diagnostics(&(Scroll->Base_OPM), Buffer_X, Buffer_Y,
			 Data->Flags);
		}
		#endif
	}

	#ifdef DEBUG
	if (Diagnostic_mode == 2)
	{
		Display_event_diagnostics(&(Scroll->Base_OPM), Buffer_X, Buffer_Y,
		 Playfield_X + 1, Playfield_Y + 1);
	}
	#endif

	MEM_Free_pointer(Map_handle);
}



#ifdef DEBUG

void
Display_underlay_diagnostics(struct OPM *OPM, UNSHORT X, UNSHORT Y,
 UNLONG Flags)
{
	UNSHORT State;

	if (Flags & (1L << (BLOCKED_TRAVELMODES_B + PARTY_DATA.Travel_mode)))
		OPM_FillBox(OPM, X + 2, Y + 2, 12, 12, WHITE);

	if (Flags & (1L << BLOCKED_DIRECTIONS_B))
		OPM_HorLine(OPM, X, Y, 16, WHITE);
	if (Flags & (1L << (BLOCKED_DIRECTIONS_B + 1)))
		OPM_VerLine(OPM, X + 15, Y, 16, WHITE);
	if (Flags & (1L << (BLOCKED_DIRECTIONS_B + 2)))
		OPM_HorLine(OPM, X, Y + 15, 16, WHITE);
	if (Flags & (1L << (BLOCKED_DIRECTIONS_B + 3)))
		OPM_VerLine(OPM, X, Y, 16, WHITE);

	State = (Flags & STATE) >> STATE_B;

	switch (State)
	{
		case SITTING_NL_STATE:
			xprintf(OPM, X + 1, Y + 4, "NL");
			break;
		case SITTING_NR_STATE:
			xprintf(OPM, X + 1, Y + 4, "NR");
			break;
		case SITTING_SL_STATE:
			xprintf(OPM, X + 1, Y + 4, "SL");
			break;
		case SITTING_SR_STATE:
			xprintf(OPM, X + 1, Y + 4, "SR");
			break;
		case SITTING_E_STATE:
			xprintf(OPM, X + 1, Y + 4, "E");
			break;
		case SITTING_W_STATE:
			xprintf(OPM, X + 1, Y + 4, "W");
			break;
	}

	if (Flags & ADD_NEXT_ICON)
		OPM_Box(OPM, X + 12, Y + 12, 3, 3, WHITE);
}

void
Display_overlay_diagnostics(struct OPM *OPM, UNSHORT X, UNSHORT Y,
 UNLONG Flags)
{
	UNSHORT State;

	if (!(Flags & UNDERLAY_PRIORITY))
	{
		if (Flags & ICON_INVISIBLE)
		{
			if (!(Flags & (1L << (BLOCKED_TRAVELMODES_B + PARTY_DATA.Travel_mode))))
				OPM_FillBox(OPM, X + 3, Y + 3, 10, 10, BLACK);

			if (!(Flags & (1L << BLOCKED_DIRECTIONS_B)))
				OPM_HorLine(OPM, X + 1, Y + 1, 14, BLACK);
			if (!(Flags & (1L << (BLOCKED_DIRECTIONS_B + 1))))
				OPM_VerLine(OPM, X + 14, Y + 1, 14, BLACK);
			if (!(Flags & (1L << (BLOCKED_DIRECTIONS_B + 2))))
				OPM_HorLine(OPM, X + 1, Y + 14, 14, BLACK);
			if (!(Flags & (1L << (BLOCKED_DIRECTIONS_B + 3))))
				OPM_VerLine(OPM, X + 1, Y + 1, 14, BLACK);
		}
		else
		{
			if (Flags & (1L << (BLOCKED_TRAVELMODES_B + PARTY_DATA.Travel_mode)))
				OPM_FillBox(OPM, X + 3, Y + 3, 10, 10, RED);

			if (Flags & (1L << BLOCKED_DIRECTIONS_B))
				OPM_HorLine(OPM, X + 1, Y + 1, 14, RED);
			if (Flags & (1L << (BLOCKED_DIRECTIONS_B + 1)))
				OPM_VerLine(OPM, X + 14, Y + 1, 14, RED);
			if (Flags & (1L << (BLOCKED_DIRECTIONS_B + 2)))
				OPM_HorLine(OPM, X + 1, Y + 14, 14, RED);
			if (Flags & (1L << (BLOCKED_DIRECTIONS_B + 3)))
				OPM_VerLine(OPM, X + 1, Y + 1, 14, RED);
		}

		State = (Flags & STATE) >> STATE_B;

		switch (State)
		{
			case SITTING_NL_STATE:
				xprintf(OPM, X + 9, Y + 4, "NL");
				break;
			case SITTING_NR_STATE:
				xprintf(OPM, X + 7, Y + 7, "NR");
				break;
			case SITTING_SL_STATE:
				xprintf(OPM, X + 7, Y + 7, "SL");
				break;
			case SITTING_SR_STATE:
				xprintf(OPM, X + 7, Y + 7, "SR");
				break;
			case SITTING_E_STATE:
				xprintf(OPM, X + 7, Y + 7, "E");
				break;
			case SITTING_W_STATE:
				xprintf(OPM, X + 7, Y + 7, "W");
				break;
			case SLEEPING1_STATE:
			case SLEEPING2_STATE:
				xprintf(OPM, X + 7, Y + 7, "Z");
				break;
		}

		if (Flags & ADD_NEXT_ICON)
			OPM_Box(OPM, X + 12, Y + 12, 3, 3, RED);
	}
	else
	{
		OPM_Box(OPM, X, Y, 3, 3, RED);
		if (Flags & (1L << (BLOCKED_TRAVELMODES_B + PARTY_DATA.Travel_mode)))
			OPM_Box(OPM, X + 3, Y + 3, 10, 10, RED);
	}
}

void
Display_event_diagnostics(struct OPM *OPM, UNSHORT X, UNSHORT Y,
 UNSHORT Map_X, UNSHORT Map_Y)
{
	if (Get_map_event_triggers(Map_X, Map_Y))
	{
		OPM_Box(OPM, X + 6, Y + 6, 4, 4, WHITE);
	}
}

#endif
