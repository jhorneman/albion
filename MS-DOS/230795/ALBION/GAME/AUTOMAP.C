/************
 * NAME     : AUTOMAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 7-2-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : AUTOMAP.H
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <SCROLL.H>
#include <GFXFUNC.H>
#include <XLOAD.H>
#include <FINDCOL.H>

#include <GAMEVAR.H>
#include <ANIMATE.H>
#include <3D_MAP.H>
#include <3D_PREP.H>
#include <3DCOMMON.H>
#include <MAP.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <AUTOMAP.H>
#include <EVELOGIC.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <GAMETEXT.H>

/* structure definitions */

struct Automap_entry {
	UNSHORT Layer_1, Layer_2;
};

/* prototypes */

/* Automap module functions */
void Automap_ModInit(void);
void Automap_ModExit(void);
void Init_Automap_display(void);
void Exit_Automap_display(void);
void Display_automap(void);

/* Automap methods */
UNLONG Touched_Automap_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Automap_object(struct Object *Object, union Method_parms *P);
UNLONG Right_Automap_object(struct Object *Object, union Method_parms *P);

/* Automap support functions */
UNSHORT Get_Automap_mouse_state(SISHORT X, SISHORT Y);
void Move_automap(SISHORT dX, SISHORT dY);
UNSHORT Find_goto_point(SISHORT X, SISHORT Y);

/* Automap scroll update function */
void Update_automap_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X,
 UNSHORT Buffer_Y, UNSHORT Playfield_X, UNSHORT Playfield_Y);

/* Automap access functions */
BOOLEAN Get_automap_bit(SISHORT X, SISHORT Y);
void Set_automap_bit(SISHORT X, SISHORT Y);

/* Automap display preparation functions */
void Create_automap_parchment_layer(void);

void Prepare_automap_floors(void);
void Shrink_floor(struct BBPALETTE *Source_pal, UNBYTE *Input, UNBYTE *Output);

void Insert_automap_walls_and_floors(void);
void Insert_automap_events(void);
void Insert_automap_NPCs(void);
void Insert_automap_goto_points(void);

void Insert_automap_wall(SISHORT X, SISHORT Y);
void Insert_automap_icon(SISHORT X, SISHORT Y, UNSHORT Icon_nr);

/* global variables */

/* Automap module */
static struct Module Automap_Mod = {
	0, SCREEN_MOD, AUTOMAP_SCREEN,
	Update_display,
	Automap_ModInit,
	Automap_ModExit,
	Init_Automap_display,
	Exit_Automap_display,
	Display_automap
};

/* Automap object class */
static struct Method Automap_methods[] = {
	{ TOUCHED_METHOD, Touched_Automap_object },
	{ LEFT_METHOD, Left_Automap_object },
	{ RIGHT_METHOD, Right_Automap_object },
	{ 0, NULL}
};

static struct Object_class Automap_Class = {
	0, sizeof(struct Object),
	&Automap_methods[0]
};

/* Memory handles */
static MEM_HANDLE Automap_handle;
static MEM_HANDLE Automap_gfx_handle;
static MEM_HANDLE Automap_buffer_handle;
static MEM_HANDLE Automap_floors_handle;

static UNLONG Automap_file_length;

static struct Automap_entry *Automap_buffer;
static UNBYTE *Automap_graphics;
static UNBYTE *Automap_floor_graphics;

static UNSHORT Automap_object;

static UNSHORT Automap_function_flags;
static BOOLEAN Automap_changed;

static UNSHORT Automap_width, Automap_height;
static UNSHORT Automap_X_offset, Automap_Y_offset;
static UNSHORT Automap_X, Automap_Y;

static struct Scroll_data Scroll_automap;

static UNBYTE Automap_update_buffer[AUTOMAP_BUFFER_HEIGHT + 1][AUTOMAP_BUFFER_WIDTH + 1];

static UNSHORT Automap_icon_animation_lengths[MAX_AUTO_ICONS-1] = {
	4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8
};
static UNLONG Automap_icon_graphics_offsets[MAX_AUTO_ICONS-1];

static struct BBRECT Automap_MA = {
	0, 0, 0, 0
};

/* Mouse pointers in automap window */
static UNSHORT Mice_Automap[] = {
	UPLEFT_MPTR, UP2D_MPTR, UPRIGHT_MPTR,
	LEFT2D_MPTR, DEFAULT_MPTR, RIGHT2D_MPTR,
	DOWNLEFT_MPTR, DOWN2D_MPTR, DOWNRIGHT_MPTR,
	DEFAULT_MPTR			/* Outside of the Automap window */
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Enter_Automap
 * FUNCTION  : Enter the Automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 17:20
 * LAST      : 07.02.95 17:20
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Enter_Automap(void)
{
	static struct Event_action Enter_automap_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		ENTER_AUTOMAPPER_ACTION, 0, 0,
		Do_enter_automap, NULL, NULL
	};

	/* In 3D map ? */
	if (_3D_map)
	{
		/* Check events */
		Enter_automap_action.Actor_index = PARTY_DATA.Active_member;
		Perform_action(&Enter_automap_action);
	}
}

BOOLEAN
Do_enter_automap(struct Event_action *Action)
{
	Exit_display();
	Push_module(&Automap_Mod);
	Init_display();

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Automap_ModInit
 * FUNCTION  : Initialize Automap module.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 17:51
 * LAST      : 07.02.95 17:51
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Automap_ModInit(void)
{
	UNLONG Offset;
	SISHORT amX, amY;
	UNSHORT i;

	Automap_function_flags = 0;

	/* Calculate automap icon graphics offsets */
	Offset = AUTOMAP_ICONS;
	for (i=2;i<=MAX_AUTO_ICONS;i++)
	{
		Automap_icon_graphics_offsets[i-2] = Offset;
		Offset += Automap_icon_animation_lengths[i-2] * 256;
	}

	/* Prepare floors for the automap */
	Prepare_automap_floors();

	/* Create and install automap mouse area */
	Automap_MA.width = Screen_width;
	Automap_MA.height = Panel_Y;
	Push_MA(&Automap_MA);

	/* Determine automap dimensions */
	Automap_width = ((Screen_width / 8) + 3) & 0xFFFC;
	if (Map_width > Automap_width - 8)
	{
		Automap_width = ((Map_width + 3) & 0xFFFC) + 8;
	}

	Automap_height = ((Panel_Y / 8) + 3) & 0xFFFC;
	if (Map_height > Automap_height - 8)
	{
		Automap_height = ((Map_height + 3) & 0xFFFC) + 8;
	}

	if (Automap_width == Automap_height)
		Automap_width += 4;

	/* Calculate automap offsets */
	Automap_X_offset = (Automap_width - Map_width) / 2;
	Automap_Y_offset = (Automap_height - Map_height) / 2;

	/* Calculate initial automap coordinates */
	amX = (PARTY_DATA.X + Automap_X_offset - 1) - (Screen_width / 16);
	amY = (PARTY_DATA.Y + Automap_Y_offset - 1) - (Panel_Y / 16);

	if (amX < 0)
	{
		amX = 0;
	}
	else
	{
		if (amX > Automap_width - (Screen_width / 8))
		{
			amX = Automap_width - (Screen_width / 8);
		}
	}

	if (amY < 0)
	{
		amY = 0;
	}
	else
	{
		if (amY > Automap_height - (Panel_Y / 8))
		{
			amY = Automap_height - (Panel_Y / 8);
		}
	}
	Automap_X = amX * 8;
	Automap_Y = amY * 8;

	/* Allocate memory for automap buffer */
	Automap_buffer_handle = MEM_Allocate_memory(Automap_width * Automap_height
	 * sizeof(struct Automap_entry));
	MEM_Clear_memory(Automap_buffer_handle);

	/* Load automap graphics */
	Automap_gfx_handle = Load_file(AUTOMAP_GFX);

	/* Create parchment layer */
	Create_automap_parchment_layer();

	/* Insert stuff in automap */
	Insert_automap_walls_and_floors();
	Insert_automap_events();
	Insert_automap_NPCs();
	Insert_automap_goto_points();

	/* Set scrolling data */
	Scroll_automap.Update_unit = Update_automap_unit;
	Scroll_automap.Unit_width = 8;
	Scroll_automap.Unit_height = 8;
	Scroll_automap.Viewport_width = Screen_width;
	Scroll_automap.Viewport_height = Panel_Y;
	Scroll_automap.Playfield_width = Automap_width * 8;
	Scroll_automap.Playfield_height = Automap_height * 8;

	/* Initialize scrolling */
	Automap_buffer = (struct Automap_entry *)
	 MEM_Claim_pointer(Automap_buffer_handle);
	Automap_graphics = MEM_Claim_pointer(Automap_gfx_handle);
	Automap_floor_graphics = MEM_Claim_pointer(Automap_floors_handle);

	Init_scroll(&Scroll_automap, Automap_X, Automap_Y);

	MEM_Free_pointer(Automap_buffer_handle);
	MEM_Free_pointer(Automap_gfx_handle);
	MEM_Free_pointer(Automap_floors_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Automap_ModExit
 * FUNCTION  : Exit Automap module.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 16:17
 * LAST      : 07.02.95 16:17
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Automap_ModExit(void)
{
	/* Destroy scroll data */
	Exit_scroll(&Scroll_automap);

	/* Free memory */
	MEM_Free_memory(Automap_floors_handle);
	MEM_Free_memory(Automap_gfx_handle);
	MEM_Free_memory(Automap_buffer_handle);

	/* Remove mouse area */
	Pop_MA();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Automap_display
 * FUNCTION  : Initialize Automap display.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.02.95 16:17
 * LAST      : 08.02.95 16:17
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_Automap_display(void)
{
	UNSHORT i;

	/* Add OPMs to update list */
	for (i=0;i<4;i++)
	{
		Add_update_OPM(&(Scroll_automap.Scroll_OPMs[i]));
	}

	/* Add interface object */
	Automap_object = Add_object(Earth_object, &Automap_Class, NULL, 0, 0,
	 Screen_width, Panel_Y);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Automap_display
 * FUNCTION  : Exit Automap display.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.02.95 16:16
 * LAST      : 26.06.95 12:50
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_Automap_display(void)
{
	UNSHORT i;

	/* Delete interface object */
	Delete_object(Automap_object);

	/* Remove OPMs from update list */
	for (i=0;i<4;i++)
	{
		Remove_update_OPM(&(Scroll_automap.Scroll_OPMs[i]));
	}

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);
	Switch_screens();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_automap
 * FUNCTION  : Display the automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.02.95 16:18
 * LAST      : 08.02.95 16:18
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_automap(void)
{
	struct Automap_entry *Ptr;
	SILONG nX, nY;
	SISHORT buffX, buffY;
	UNSHORT buffW, buffH;
	UNSHORT i, j, c, Direction;
	UNBYTE *Gfx;

	Automap_buffer = (struct Automap_entry *) MEM_Claim_pointer(Automap_buffer_handle);
	Automap_graphics = MEM_Claim_pointer(Automap_gfx_handle);
	Automap_floor_graphics = MEM_Claim_pointer(Automap_floors_handle);

	/* Update scrolling */
	Set_scroll_position(&Scroll_automap, Automap_X, Automap_Y);

	/* Clear update buffer */
	BASEMEM_FillMemByte(&(Automap_update_buffer[0][0]), (AUTOMAP_BUFFER_HEIGHT
	 + 1) * (AUTOMAP_BUFFER_WIDTH + 1), 0);

	/* Calculate automap check area coordinates and dimensions in tiles */
	buffX = (Automap_X / 8) - 1;
	buffY = (Automap_Y / 8);

	buffW = AUTOMAP_BUFFER_WIDTH + 3;
	buffH = AUTOMAP_BUFFER_HEIGHT + 2;

	if (buffX < 0)
	{
		buffW += buffX;
		buffX = 0;
	}
	else
	{
		if (buffX + buffW > Automap_width)
		{
			buffW = Automap_width + 1 - buffX;
		}
	}

	if (buffY + buffH > Automap_height)
	{
		buffH = Automap_height - buffY;
	}

	/* Determine which parts of the automap have to be re-drawn because
	 they are covered by icons */
	Ptr = Automap_buffer + (buffY * Automap_width) + buffX;
	for (i=0;i<buffH;i++)
	{
		for (j=0;j<buffW;j++)
		{
			/* Is there something in the second layer ? */
			if (Ptr->Layer_2)
			{
				/* Yes -> Set update flags */
				if (j <= AUTOMAP_BUFFER_WIDTH)
				{
					if (i <= AUTOMAP_BUFFER_HEIGHT)
					{
						Automap_update_buffer[i][j] = 0xFF;
					}

					if (i)
					{
						Automap_update_buffer[i-1][j] = 0xFF;
					}
				}

				if (j < AUTOMAP_BUFFER_WIDTH)
				{
					if (i <= AUTOMAP_BUFFER_HEIGHT)
					{
						Automap_update_buffer[i][j+1] = 0xFF;
					}

					if (i)
					{
						Automap_update_buffer[i-1][j+1] = 0xFF;
					}
				}
			}
			Ptr++;
		}
		Ptr += Automap_width - buffW;
	}

	/* Determine which parts of the automap have to be re-drawn because
	 they are covered by notes */
	for (i=0;i<Nr_auto_notes;i++)
	{

	}

	/* Redraw the automap */
	Ptr = Automap_buffer + (buffY * Automap_width) + buffX;
	for (i=0;i<buffH;i++)
	{
		for (j=0;j<buffW;j++)
		{
			/* Is this position inside the automap update buffer ? */
			if ((i <= AUTOMAP_BUFFER_HEIGHT) && (j <= AUTOMAP_BUFFER_WIDTH))
			{
				/* Does this position have to be redrawn ? */
				if (Automap_update_buffer[i][j])
				{
					/* Yes -> Convert coordinates */
					nX = ((SILONG) (buffX + j)) * 8;
					nY = ((SILONG) (buffY + i)) * 8;
					Convert_scroll_coordinates(&Scroll_automap, &nX, &nY);

					/* Inside the scroll buffer ? */
					if ((nX != -32768) && (nY != -32768))
					{
						/* Yes -> Update first layer */
						Update_automap_unit(&Scroll_automap, nX, nY,
						 buffX + j, buffY + i);
					}
				}
			}

			/* Is this position inside the automap buffer ? */
			if ((buffX + i > 0) && (buffY + i < Automap_height))
			{
				/* Yes -> Is there something in the second layer ? */
				c = (Ptr - 1)->Layer_2;
				if ((c > 1) && (c <= MAX_AUTO_ICONS))
				{
					/* Yes -> Get graphics address */
					Gfx = Automap_graphics + Automap_icon_graphics_offsets[c-2];
					Gfx += Get_animation_frame(ASYNCH_ANIM,
					 Automap_icon_animation_lengths[c-2], (buffY + i) *
					 Automap_width + buffX + j) * 256;

					/* Display object */
					Display_object_in_scroll_buffer(&Scroll_automap, (buffX + j - 1) * 8,
					 (buffY + i - 1) * 8, 16, 16, Gfx);
				}

				/* Should the YAH-symbol be drawn here ? */
				if ((buffX + j - Automap_X_offset - 1 == PARTY_DATA.X) &&
				 (buffY + i - Automap_Y_offset - 1 == PARTY_DATA.Y))
				{
					/* Yes -> Draw YAH base */
					Gfx = Automap_graphics + AUTOMAP_YAH_BASE;
					Display_object_in_scroll_buffer(&Scroll_automap,
					 (buffX + j - 2) * 8, (buffY + i - 3) * 8, 16, 16, Gfx);

					/* Does the player have a compass / cheat mode ? */
					if ((PARTY_DATA.Special_item_flags & COMPASS_ITEM_FLAG) ||
					 (Cheat_mode))
					{
						/* Yes -> Determine current direction */
						Direction = (((ANGLE_STEPS - (I3DM.Camera_angle >> 16))
						 + (ANGLE_STEPS / 16)) & (ANGLE_STEPS - 1)) / (ANGLE_STEPS / 8);

						/* Display YAH top with arrow */
						Gfx = Automap_graphics + AUTOMAP_YAH_ARROWS +
						 (Direction * 256);
						Display_object_in_scroll_buffer(&Scroll_automap,
						 (buffX + j - 2) * 8, (buffY + i - 5) * 8, 16, 16, Gfx);
					}
					else
					{
						/* No -> Display "normal" YAH top */
						Gfx = Automap_graphics + AUTOMAP_YAH_TOP;
						Display_object_in_scroll_buffer(&Scroll_automap,
						 (buffX + j - 2) * 8, (buffY + i - 5) * 8, 16, 16, Gfx);
					}
				}
			}
			Ptr++;
		}
		Ptr += Automap_width - buffW;
	}

	MEM_Free_pointer(Automap_buffer_handle);
	MEM_Free_pointer(Automap_gfx_handle);
	MEM_Free_pointer(Automap_floors_handle);

	/* Display scroll buffer */
	Scroll_display(&Scroll_automap, 0, 0);

	/* Update animations */
	Update_animation();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Touched_Automap_object
 * FUNCTION  : Touched method of Automap object.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.02.95 15:24
 * LAST      : 13.02.95 15:24
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Touched_Automap_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT mX, mY;
	UNSHORT Q;

	/* Update highlighting */
	Normal_touched(Object, P);

	/* Get mouse coordinates */
	mX = Mouse_X;
	mY = Mouse_Y;

	/* Get Automap mouse state */
	Q = Get_Automap_mouse_state(mX, mY);

	/* Change mouse pointer accordingly */
	Change_mouse(&(Mouse_pointers[Mice_Automap[Q]]));

	/* Is the mouse inside the automap window ? */
	if (Q < 9)
	{
		/* Yes -> Move depending on state */
		switch (Q)
		{
			/* Try to move up-left */
			case 0:
			{
				Move_automap(0 - (16 - mX), 0 - (16 - mY));
				break;
			}
			/* Try to move up */
			case 1:
			{
				Move_automap(0, 0 - (16 - mY));
				break;
			}
			/* Try to move up-right */
			case 2:
			{
				Move_automap(mX - (Screen_width - 16) , 0 - (16 - mY));
				break;
			}
			/* Try to move left */
			case 3:
			{
				Move_automap(0 - (16 - mX), 0);
				break;
			}
			/* Do "real" touching */
			case 4:
			{
				struct Automap_entry *Buffer_ptr;
				SISHORT X, Y;
				UNSHORT c, Nr;
				UNBYTE *Ptr;

				/* Calculate automap buffer coordinates */
				X = (Automap_X + mX) / 8;
				Y = (Automap_Y + mY) / 8;

				/* Look in the automap buffer's second layer */
				Buffer_ptr = (struct Automap_entry *)
				 MEM_Claim_pointer(Automap_buffer_handle);
				Buffer_ptr += (Y * Automap_width) + X;

				c = Buffer_ptr->Layer_2;
				if (!c && (Y < Automap_height))
				{
					c = (Buffer_ptr + Automap_width)->Layer_2;
					Y++;
				}

				MEM_Free_pointer(Automap_buffer_handle);

				/* Anything in the second layer ? */
				if (c)
				{
					/* Yes -> A goto-point ? */
					if (c == GOTO_POINT_AUTO_ICON)
					{
						/* Yes -> Get goto-point index */
						Nr = Find_goto_point(X - Automap_X_offset + 1,
						 Y - Automap_Y_offset + 1);

						/* Anything found ? */
						if (Nr != 0xFFFF)
						{
							/* Yes -> Find goto-point text */
							Ptr = MEM_Claim_pointer(Map_text_handle);

							Ptr = Find_text_block(Ptr, 0);
							Ptr += Map_text_0_subblock_offsets[Nr + 1];

							/* Print help line */
							Execute_method(Help_line_object, SET_METHOD,
							 (void *) Ptr);

							MEM_Free_pointer(Map_text_handle);
						}
					}
					else
					{
						/* No -> Print help line */
						Execute_method(Help_line_object, SET_METHOD,
						 (void *) System_text_ptrs[168 + c - 2]);
					}
				}
				break;
			}
			/* Try to move right */
			case 5:
			{
				Move_automap(mX - (Screen_width - 16), 0);
				break;
			}
			/* Try to move down-left */
			case 6:
			{
				Move_automap(0 - (16 - mX), mY - (Panel_Y - 16));
				break;
			}
			/* Try to move down */
			case 7:
			{
				Move_automap(0, mY - (Panel_Y - 16));
				break;
			}
			/* Try to move down-right */
			case 8:
			{
				Move_automap(mX - (Screen_width - 16), mY - (Panel_Y - 16));
				break;
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Automap_object
 * FUNCTION  : Left method of Automap object.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.07.95 16:14
 * LAST      : 04.07.95 16:14
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Automap_object(struct Object *Object, union Method_parms *P)
{
	SISHORT X, Y;

	/* Calculate automap buffer coordinates */
	X = (Automap_X + Mouse_X) / 8;
	Y = (Automap_Y + Mouse_Y) / 8;

	/* In cheat mode ? */
	if (Cheat_mode)
	{
		/* Yes -> Calculate map coordinates */
		X = X - Automap_X_offset + 1;
		Y = Y - Automap_Y_offset + 1;

		/* Inside the map ? */
		if ((X > 0) && (X <= Map_width) && (Y > 0) && (Y <= Map_height))
		{
			/* Yes -> Jump to this position */
  			Change_position(X, Y, PARTY_DATA.View_direction);

			/* Leave the automapper */
			Exit_display();
			Pop_module();
			Init_display();
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Right_Automap_object
 * FUNCTION  : Right method of Automap object.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.02.95 16:22
 * LAST      : 13.02.95 16:22
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Right_Automap_object(struct Object *Object, union Method_parms *P)
{
	/* Leave the automapper */
	Exit_display();
	Pop_module();
	Init_display();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_Automap_mouse_state
 * FUNCTION  : Get state of mouse in Automap window.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.02.95 15:26
 * LAST      : 13.02.95 15:26
 * INPUTS    : SISHORT X - X-coordinate of mouse
 *             SISHORT Y - Y-coordinate of mouse
 * RESULT    : UNSHORT : Mouse state (0...8 / 9 = out of window)
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_Automap_mouse_state(SISHORT X, SISHORT Y)
{
	UNSHORT Q;

	/* Exit if the mouse pointer is outside of the Automap window */
	if ((X < 0) || (X >= Screen_width) || (Y < 0) || (Y >= Panel_Y))
		return 9;

	/* Determine state */
	Q = 0;
	if (X >= 16)
	{
		if (X < Screen_width - 16)
			Q = 1;
		else
			Q = 2;
	}
	if (Y >= 16)
	{
		if (Y < Panel_Y - 16)
			Q += 3;
		else
			Q += 6;
	}

	return Q;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_automap
 * FUNCTION  : Move the automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.02.95 15:33
 * LAST      : 13.02.95 15:33
 * INPUTS    : SISHORT dX - X-component of movement vector.
 *             SISHORT dY - Y-component of movement vector.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_automap(SISHORT dX, SISHORT dY)
{
	SISHORT New_X, New_Y;

	/* Calculate new coordinates */
	New_X = Automap_X + dX;
	New_Y = Automap_Y + dY;

	/* Clip coordinates */
	if (New_X < 0)
	{
		New_X = 0;
	}
	else
	{
		if (New_X >= (Automap_width * 8) - Screen_width)
		{
			New_X = (Automap_width * 8) - Screen_width;
		}
	}

	if (New_Y < 0)
	{
		New_Y = 0;
	}
	else
	{
		if (New_Y >= (Automap_height * 8) - Panel_Y)
		{
			New_Y = (Automap_height * 8) - Panel_Y;
		}
	}

	/* Store new coordinates */
	Automap_X = New_X;
	Automap_Y = New_Y;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_goto_point
 * FUNCTION  : Find goto-point at a certain location.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 12:14
 * LAST      : 11.07.95 15:35
 * INPUTS    : SISHORT X - Map X-coordinate (1...).
 *             SISHORT Y - Map Y-coordinate (1...).
 * RESULT    : UNSHORT : Goto-point index (0xFFFF = not found).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Find_goto_point(SISHORT X, SISHORT Y)
{
	struct Goto_point *Ptr;
	UNSHORT i, Value = 0xFFFF;

	/* Get goto-point data */
	Ptr = (struct Goto_point *) (MEM_Claim_pointer(Map_handle) +
	 Goto_point_offset);

	/* Check all goto-points in this map */
	for(i=0;i<Nr_goto_points;i++)
	{
		/* Does this goto-point have the right coordinates ? */
		if ((X == Ptr[i].X) && (Y == Ptr[i].Y))
		{
			/* Yes -> Cheat mode / is this goto-point known ? */
			if (Cheat_mode ||
			 Read_bit_array(GOTO_POINTS_BIT_ARRAY, Ptr[i].Bit_nr))
			{
				/* Yes */
				Value = i;
			}
			break;
		}
	}
	MEM_Free_pointer(Map_handle);

	return Value;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_automap_unit
 * FUNCTION  : Updates a scroll unit of the automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 17:49
 * LAST      : 07.02.95 17:49
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             UNSHORT Buffer_X - X-coordinate of unit in scroll buffer.
 *             UNSHORT Buffer_Y - Y-coordinate of unit in scroll buffer.
 *             UNSHORT Playfield_X - X-coordinate of unit in playfield.
 *             UNSHORT Playfield_Y - Y-coordinate of unit in playfield.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Some pointers must be initialized when this function is
 *              called.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_automap_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X,
 UNSHORT Buffer_Y, UNSHORT Playfield_X, UNSHORT Playfield_Y)
{
	struct Automap_entry *Map_ptr;
	UNSHORT c;
	UNBYTE *Gfx;

	/* Calculate pointer to automap buffer */
	Map_ptr = Automap_buffer + Playfield_X + Playfield_Y * Automap_width;

	/* Parchment or floor ? */
	c = Map_ptr->Layer_1;
	if (c >= 10000)
	{
		/* Floor -> Get graphics address */
		Gfx = Automap_floor_graphics + 64 * (c - 10000);
	}
	else
	{
		/* Parchment -> Get graphics address */
		Gfx = Automap_graphics + 64 * c;
	}

	/* Display parchment layer */
	Put_unmasked_block(&(Scroll->Base_OPM), Buffer_X, Buffer_Y, 8, 8, Gfx);

	MEM_Free_pointer(Automap_buffer_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_automap
 * FUNCTION  : Initialize automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:44
 * LAST      : 07.02.95 15:44
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_automap(void)
{
	/* Load automap */
	Automap_handle = Load_subfile(AUTOMAP, PARTY_DATA.Map_nr);
	if (!Automap_handle)
	{
		Error(ERROR_FILE_LOAD);
		return;
	}

	/* Get automap file length */
	Automap_file_length = MEM_Get_block_size(Automap_handle);
	if (Automap_file_length < ((Map_width * Map_height + 7) / 8))
	{
		Error(ERROR_AUTOMAP_FILE);
		return;
	}

	/* Clear flag */
	Automap_changed = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_automap
 * FUNCTION  : Exit automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:45
 * LAST      : 07.02.95 15:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_automap(void)
{
	/* Save the automap */
	Save_automap();

	/* Free memory */
	MEM_Free_memory(Automap_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_automap
 * FUNCTION  : Save automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:45
 * LAST      : 07.02.95 15:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Save_automap(void)
{
	/* Changed ? */
	if (Automap_changed)
	{
		/* Yes -> Save automap */
		Save_subfile(Automap_handle, AUTOMAP, PARTY_DATA.Map_nr);

		/* Clear flag */
		Automap_changed = FALSE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_automap
 * FUNCTION  : Update the Automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 09:34
 * LAST      : 21.02.95 09:34
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_automap(void)
{
	BOOLEAN CA_completed;
	SISHORT pX, pY;
	SISHORT X, Y;
	UNSHORT Direction;
	UNSHORT i, j, k;
	UNSHORT c1, c2;
	UNBYTE Automap_CA[AMCA_WIDTH][AMCA_WIDTH];
	UNBYTE Mask[5];

	/* Clear automap CA */
	BASEMEM_FillMemByte(&(Automap_CA[0][0]), AMCA_WIDTH * AMCA_WIDTH,
	 AMCA_VOID);

	/* Get party coordinates */
	pX = PARTY_DATA.X;
	pY = PARTY_DATA.Y;

	/* Determine direction */
	Direction = (((ANGLE_STEPS - (I3DM.Camera_angle >> 16))
	 + (ANGLE_STEPS / 16)) & (ANGLE_STEPS - 1)) / (ANGLE_STEPS / 8);

	/* Fill CA depending on direction */
	switch (Direction)
	{
		case NORTH8:
		{
			/* Insert view triangle */
			for (i=0;i<=AMCA_OFFSET;i++)
			{
				for (j=0;j<i*2+1;j++)
				{
					/* Calculate coordinates */
					X = pX - i + j;
					Y = pY - i;

					/* Inside the map ? */
					if ((X > 0) && (X <= Map_width) &&
					 (Y > 0) && (Y <= Map_height))
					{
						/* Yes -> Insert vision blocked status in CA */
						if (Vision_blocked(X, Y))
						{
							Automap_CA[AMCA_OFFSET - i][AMCA_OFFSET - i + j] =
							 AMCA_BLOCKED;
						}
						else
						{
							Automap_CA[AMCA_OFFSET - i][AMCA_OFFSET - i + j] =
							 AMCA_EMPTY;
						}
					}
					else
					{
						/* No */
						Automap_CA[AMCA_OFFSET - i][AMCA_OFFSET - i + j] =
						 AMCA_OUTSIDE;
					}
				}
			}
			break;
		}
		case NORTH_EAST:
		{
			/* Insert view rectangle */
			for (i=0;i<=AMCA_OFFSET;i++)
			{
				for (j=0;j<=AMCA_OFFSET;j++)
				{
					/* Calculate coordinates */
					X = pX + j;
					Y = pY - i;

					/* Inside the map ? */
					if ((X > 0) && (X <= Map_width) &&
					 (Y > 0) && (Y <= Map_height))
					{
						/* Yes -> Insert vision blocked status in CA */
						if (Vision_blocked(X, Y))
						{
							Automap_CA[AMCA_OFFSET - i][AMCA_OFFSET + j] =
							 AMCA_BLOCKED;
						}
						else
						{
							Automap_CA[AMCA_OFFSET - i][AMCA_OFFSET + j] =
							 AMCA_EMPTY;
						}
					}
					else
					{
						/* No */
						Automap_CA[AMCA_OFFSET - i][AMCA_OFFSET + j] =
						 AMCA_OUTSIDE;
					}
				}
			}
			break;
		}
		case EAST8:
		{
			/* Insert view triangle */
			for (i=0;i<=AMCA_OFFSET;i++)
			{
				for (j=0;j<i*2+1;j++)
				{
					/* Calculate coordinates */
					X = pX + i;
					Y = pY - i + j;

					/* Inside the map ? */
					if ((X > 0) && (X <= Map_width) &&
					 (Y > 0) && (Y <= Map_height))
					{
						/* Yes -> Insert vision blocked status in CA */
						if (Vision_blocked(X, Y))
						{
							Automap_CA[AMCA_OFFSET - i + j][AMCA_OFFSET + i] =
							 AMCA_BLOCKED;
						}
						else
						{
							Automap_CA[AMCA_OFFSET - i + j][AMCA_OFFSET + i] =
							 AMCA_EMPTY;
						}
					}
					else
					{
						/* No */
						Automap_CA[AMCA_OFFSET - i + j][AMCA_OFFSET + i] =
						 AMCA_OUTSIDE;
					}
				}
			}
			break;
		}
		case SOUTH_EAST:
		{
			/* Insert view rectangle */
			for (i=0;i<=AMCA_OFFSET;i++)
			{
				for (j=0;j<=AMCA_OFFSET;j++)
				{
					/* Calculate coordinates */
					X = pX + j;
					Y = pY + i;

					/* Inside the map ? */
					if ((X > 0) && (X <= Map_width) &&
					 (Y > 0) && (Y <= Map_height))
					{
						/* Yes -> Insert vision blocked status in CA */
						if (Vision_blocked(X, Y))
						{
							Automap_CA[AMCA_OFFSET + i][AMCA_OFFSET + j] =
							 AMCA_BLOCKED;
						}
						else
						{
							Automap_CA[AMCA_OFFSET + i][AMCA_OFFSET + j] =
							 AMCA_EMPTY;
						}
					}
					else
					{
						/* No */
						Automap_CA[AMCA_OFFSET + i][AMCA_OFFSET + j] =
						 AMCA_OUTSIDE;
					}

				}
			}
			break;
		}
		case SOUTH8:
		{
			/* Insert view triangle */
			for (i=0;i<=AMCA_OFFSET;i++)
			{
				for (j=0;j<i*2+1;j++)
				{
					/* Calculate coordinates */
					X = pX - i + j;
					Y = pY + i;

					/* Inside the map ? */
					if ((X > 0) && (X <= Map_width) &&
					 (Y > 0) && (Y <= Map_height))
					{
						/* Yes -> Insert vision blocked status in CA */
						if (Vision_blocked(X, Y))
						{
							Automap_CA[AMCA_OFFSET + i][AMCA_OFFSET - i + j] =
							 AMCA_BLOCKED;
						}
						else
						{
							Automap_CA[AMCA_OFFSET + i][AMCA_OFFSET - i + j] =
							 AMCA_EMPTY;
						}
					}
					else
					{
						/* No */
						Automap_CA[AMCA_OFFSET + i][AMCA_OFFSET - i + j] =
						 AMCA_OUTSIDE;
					}
				}
			}
			break;
		}
		case SOUTH_WEST:
		{
			/* Insert view rectangle */
			for (i=0;i<=AMCA_OFFSET;i++)
			{
				for (j=0;j<=AMCA_OFFSET;j++)
				{
					/* Calculate coordinates */
					X = pX - j;
					Y = pY + i;

					/* Inside the map ? */
					if ((X > 0) && (X <= Map_width) &&
					 (Y > 0) && (Y <= Map_height))
					{
						/* Yes -> Insert vision blocked status in CA */
						if (Vision_blocked(X, Y))
						{
							Automap_CA[AMCA_OFFSET + i][AMCA_OFFSET - j] =
							 AMCA_BLOCKED;
						}
						else
						{
							Automap_CA[AMCA_OFFSET + i][AMCA_OFFSET - j] =
							 AMCA_EMPTY;
						}
					}
					else
					{
						/* No */
						Automap_CA[AMCA_OFFSET + i][AMCA_OFFSET - j] =
						 AMCA_OUTSIDE;
					}
				}
			}
			break;
		}
		case WEST8:
		{
			/* Insert view triangle */
			for (i=0;i<=AMCA_OFFSET;i++)
			{
				for (j=0;j<i*2+1;j++)
				{
					/* Calculate coordinates */
					X = pX - i;
					Y = pY - i + j;

					/* Inside the map ? */
					if ((X > 0) && (X <= Map_width) &&
					 (Y > 0) && (Y <= Map_height))
					{
						/* Yes -> Insert vision blocked status in CA */
						if (Vision_blocked(X, Y))
						{
							Automap_CA[AMCA_OFFSET - i + j][AMCA_OFFSET - i] =
							 AMCA_BLOCKED;
						}
						else
						{
							Automap_CA[AMCA_OFFSET - i + j][AMCA_OFFSET - i] =
							 AMCA_EMPTY;
						}
					}
					else
					{
						/* No */
						Automap_CA[AMCA_OFFSET - i + j][AMCA_OFFSET - i] =
						 AMCA_OUTSIDE;
					}
				}
			}
			break;
		}
		case NORTH_WEST:
		{
			/* Insert view rectangle */
			for (i=0;i<=AMCA_OFFSET;i++)
			{
				for (j=0;j<=AMCA_OFFSET;j++)
				{
					/* Calculate coordinates */
					X = pX - j;
					Y = pY - i;

					/* Inside the map ? */
					if ((X > 0) && (X <= Map_width) &&
					 (Y > 0) && (Y <= Map_height))
					{
						/* Yes -> Insert vision blocked status in CA */
						if (Vision_blocked(X, Y))
						{
							Automap_CA[AMCA_OFFSET - i][AMCA_OFFSET - j] =
							 AMCA_BLOCKED;
						}
						else
						{
							Automap_CA[AMCA_OFFSET - i][AMCA_OFFSET - j] =
							 AMCA_EMPTY;
						}
					}
					else
					{
						/* No */
						Automap_CA[AMCA_OFFSET - i][AMCA_OFFSET - j] =
						 AMCA_OUTSIDE;
					}
				}
			}
			break;
		}
	}

	/* Initialize cellular automatum */
	Automap_CA[AMCA_OFFSET][AMCA_OFFSET] = AMCA_VISIBLE;

	/* Do cellular automatum */
	do
	{
		CA_completed = TRUE;
		for (i=0;i<AMCA_WIDTH;i++)
		{
			for (j=0;j<AMCA_WIDTH;j++)
			{
				/* Get cell state */
				c1 = Automap_CA[i][j];

				/* Visible ? */
				if (c1 == AMCA_VISIBLE)
				{
					/* Yes -> Check around (orthogonal directions) */
					for (k=0;k<4;k++)
					{
						/* Clear mask */
						Mask[k] = 0;

						/* Calculate coordinates */
						X = j + Offsets4[k][0];
						Y = i + Offsets4[k][1];

						/* Inside CA ? */
						if (X && (X < AMCA_WIDTH) && Y && (Y < AMCA_WIDTH))
						{
							/* Yes -> Get cell state */
							c2 = Automap_CA[Y][X];

							/* Yes -> Is this cell empty ? */
							if (c2 == AMCA_EMPTY)
							{
								/* Yes -> Set cell state to visible */
								Automap_CA[Y][X] = AMCA_VISIBLE;

								/* Flag! */
								CA_completed = FALSE;
							}
							else
							{
								/* No -> Is vision blocked ? */
								if (c2 == AMCA_BLOCKED)
								{
									/* Yes -> Set mask */
									Mask[k] = 0xFF;

									/* Set cell state to edge */
									Automap_CA[Y][X] = AMCA_EDGE;
								}

								/* Edge cell ? */
								if (c2 == AMCA_EDGE)
								{
									/* Yes -> Set mask */
									Mask[k] = 0xFF;
								}
							}
						}
					}

					/* Duplicate first mask flag (for wrap-around) */
					Mask[4] = Mask[0];

					/* Check around (diagonal directions) */
					for (k=0;k<4;k++)
					{
						/* Calculate coordinates */
						X = j + Offsets8[k * 2 + 1][0];
						Y = i + Offsets8[k * 2 + 1][1];

						/* Inside CA ? */
						if (X && (X < AMCA_WIDTH) && Y && (Y < AMCA_WIDTH))
						{
							/* Yes -> Get cell state */
							c2 = Automap_CA[Y][X];

							/* Yes -> Is this cell empty ? */
							if (c2 == AMCA_EMPTY)
							{
								/* Yes -> Do the two corresponding orthogonal
								 directions block vision ? */
								if (!Mask[k] && !Mask[k + 1])
								{
									/* No -> Set cell state to visible */
									Automap_CA[Y][X] = AMCA_VISIBLE;

									/* Flag! */
									CA_completed = FALSE;
								}
							}
						}
					}
				}
			}
		}
	}
	while (!CA_completed);

	/* Update automap according to CA */
	Y = pY - AMCA_OFFSET;
	for (i=0;i<AMCA_WIDTH;i++)
	{
		X = pX - AMCA_OFFSET;
		for(j=0;j<AMCA_WIDTH;j++)
		{
			/* Get cell state */
			c1 = Automap_CA[i][j];

			/* Update this position ? */
			if ((c1 == AMCA_VISIBLE) || (c1 == AMCA_EDGE))
			{
				/* Yes -> Update position */
				Set_automap_bit(X, Y);
			}
			X++;
		}
		Y++;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_automap_bit
 * FUNCTION  : Get a bit from the automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:45
 * LAST      : 04.03.95 14:36
 * INPUTS    : SISHORT X - X-coordinate (1...).
 *             SISHORT Y - Y-coordinate (1...).
 * RESULT    : BOOLEAN : TRUE (bit was set) or FALSE (bit was not set).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Get_automap_bit(SISHORT X, SISHORT Y)
{
	UNSHORT Bit_nr;
	BOOLEAN Bit = FALSE;
	UNBYTE *Ptr;

	/* All bits are set when the cheat mode is on */
	if (Cheat_mode)
		return (TRUE);

	/* Exit if coordinates lie outside the map */
	if ((X < 1) || (X >= Map_width) || (Y < 1) || (Y >= Map_height))
		return (FALSE);

	/* Calculate bit number */
	Bit_nr = ((Y - 1) * Map_width) + X - 1;

	/* Exit if outside the automap file */
	if (Bit_nr / 8 >= Automap_file_length)
		return (FALSE);

	/* Read bit */
	Ptr = MEM_Claim_pointer(Automap_handle);
	Bit = (BOOLEAN) (Ptr[Bit_nr / 8] & (1 << (Bit_nr & 0x0007)));
	MEM_Free_pointer(Automap_handle);

	return Bit;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_automap_bit
 * FUNCTION  : Set a bit in the automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 16:21
 * LAST      : 07.02.95 16:21
 * INPUTS    : SISHORT X - X-coordinate (1...).
 *             SISHORT Y - Y-coordinate (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_automap_bit(SISHORT X, SISHORT Y)
{
	UNSHORT Bit_nr;
	UNBYTE *Ptr;

	/* Exit if coordinates lie outside the map */
	if ((X < 1) || (X >= Map_width) || (Y < 1) || (Y >= Map_height))
		return;

	/* Calculate bit number */
	Bit_nr = ((Y - 1) * Map_width) + X - 1;

	/* Exit if outside the automap file */
	if (Bit_nr / 8 >= Automap_file_length)
		return;

	/* Write bit */
	Ptr = MEM_Claim_pointer(Automap_handle);
	Ptr[Bit_nr / 8] |= (1 << (Bit_nr & 0x0007));
	MEM_Free_pointer(Automap_handle);

	/* Set flag */
	Automap_changed = TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Create_automap_parchment_layer
 * FUNCTION  : Create automap parchment layer.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.02.95 13:04
 * LAST      : 08.02.95 13:04
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Create_automap_parchment_layer(void)
{
	static UNBYTE Horizontal_distribution[8] = {
		0, 0, 0, 1, 1, 2, 2, 3
	};
	static UNBYTE Vertical_distribution[8] = {
		0, 0, 0, 1, 1, 1, 2, 2
	};
	static UNBYTE Surface_distribution[16] = {
		0, 0, 0, 0, 1, 1, 1, 1,
		2, 2, 2, 3, 3, 3, 3, 4
	};

	struct Automap_entry *Buffer, *Ptr, *Ptr2, *Ptr3;
	UNSHORT Base, Seed;
	UNSHORT i, j, k, l;

	Buffer = (struct Automap_entry *) MEM_Claim_pointer(Automap_buffer_handle);

	/* Insert top-left corner */
	Ptr = Buffer;
	for (i=0;i<4;i++)
	{
		Ptr2 = Ptr;
		for (j=0;j<10;j++)
		{
			Ptr2->Layer_1 = AUTOMAP_TOP_LEFT_CORNER + i * 10 + j;
			Ptr2 += Automap_width;
		}
		Ptr++;
	}
	for (i=0;i<6;i++)
	{
		Ptr2 = Ptr;
		for (j=0;j<4;j++)
		{
			Ptr2->Layer_1 = AUTOMAP_TOP_LEFT_CORNER + 40 + i * 4 + j;
			Ptr2 += Automap_width;
		}
		Ptr++;
	}

	/* Insert top-right corner */
	Ptr = Buffer + (Automap_width - 10);
	for (i=0;i<6;i++)
	{
		Ptr2 = Ptr;
		for (j=0;j<4;j++)
		{
			Ptr2->Layer_1 = AUTOMAP_TOP_RIGHT_CORNER + i * 4 + j;
			Ptr2 += Automap_width;
		}
		Ptr++;
	}
	for (i=0;i<4;i++)
	{
		Ptr2 = Ptr;
		for (j=0;j<10;j++)
		{
			Ptr2->Layer_1 = AUTOMAP_TOP_RIGHT_CORNER + 24 + i * 10 + j;
			Ptr2 += Automap_width;
		}
		Ptr++;
	}

	/* Insert bottom-left corner */
	Ptr = Buffer + (Automap_height - 10) * Automap_width;
	for (i=0;i<4;i++)
	{
		Ptr2 = Ptr;
		for (j=0;j<10;j++)
		{
			Ptr2->Layer_1 = AUTOMAP_BOTTOM_LEFT_CORNER + i * 10 + j;
			Ptr2 += Automap_width;
		}
		Ptr++;
	}
	Ptr += 6 * Automap_width;
	for (i=0;i<6;i++)
	{
		Ptr2 = Ptr;
		for (j=0;j<4;j++)
		{
			Ptr2->Layer_1 = AUTOMAP_BOTTOM_LEFT_CORNER + 40 + i * 4 + j;
			Ptr2 += Automap_width;
		}
		Ptr++;
	}

	/* Insert bottom-right corner */
	Ptr = Buffer + ((Automap_height - 4) * Automap_width)
	 + (Automap_width - 10);
	for (i=0;i<6;i++)
	{
		Ptr2 = Ptr;
		for (j=0;j<4;j++)
		{
			Ptr2->Layer_1 = AUTOMAP_BOTTOM_RIGHT_CORNER + i * 4 + j;
			Ptr2 += Automap_width;
		}
		Ptr++;
	}
	Ptr -= 6 * Automap_width;
	for (i=0;i<4;i++)
	{
		Ptr2 = Ptr;
		for (j=0;j<10;j++)
		{
			Ptr2->Layer_1 = AUTOMAP_BOTTOM_RIGHT_CORNER + 24 + i * 10 + j;
			Ptr2 += Automap_width;
		}
		Ptr++;
	}

	/* Initialize seed */
	Seed = PARTY_DATA.Map_nr;

	/* Insert top edge */
	Ptr = Buffer + 10;
	for (i=0;i<(Automap_width - 20) / 4;i++)
	{
		Seed = Seed * 17 + 87;
		Base = AUTOMAP_TOP_EDGE + 16 * Horizontal_distribution[Seed & 0x0007];
		for (j=0;j<4;j++)
		{
			Ptr2 = Ptr;
			for (k=0;k<4;k++)
			{
				Ptr2->Layer_1 = Base + j * 4 + k;
				Ptr2 += Automap_width;
			}
			Ptr++;
		}
	}

	/* Insert bottom edge */
	Ptr = Buffer + ((Automap_height - 4) * Automap_width) + 10;
	for (i=0;i<(Automap_width - 20) / 4;i++)
	{
		Seed = Seed * 17 + 87;
		Base = AUTOMAP_BOTTOM_EDGE + 16 * Horizontal_distribution[Seed & 0x0007];
		for (j=0;j<4;j++)
		{
			Ptr2 = Ptr;
			for (k=0;k<4;k++)
			{
				Ptr2->Layer_1 = Base + j * 4 + k;
				Ptr2 += Automap_width;
			}
			Ptr++;
		}
	}

	/* Insert left edge */
	Ptr = Buffer + 10 * Automap_width;
	for (i=0;i<(Automap_height - 20) / 4;i++)
	{
		Seed = Seed * 17 + 87;
		Base = AUTOMAP_LEFT_EDGE + 16 * Vertical_distribution[Seed & 0x0007];
		Ptr2 = Ptr;
		for (j=0;j<4;j++)
		{
			Ptr3 = Ptr2;
			for (k=0;k<4;k++)
			{
				Ptr3->Layer_1 = Base + j * 4 + k;
				Ptr3 += Automap_width;
			}
			Ptr2++;
		}
		Ptr += Automap_width * 4;
	}

	/* Insert right edge */
	Ptr = Buffer + (10 * Automap_width) + Automap_width - 4;
	for (i=0;i<(Automap_height - 20) / 4;i++)
	{
		Seed = Seed * 17 + 87;
		Base = AUTOMAP_RIGHT_EDGE + 16 * Vertical_distribution[Seed & 0x0007];
		Ptr2 = Ptr;
		for (j=0;j<4;j++)
		{
			Ptr3 = Ptr2;
			for (k=0;k<4;k++)
			{
				Ptr3->Layer_1 = Base + j * 4 + k;
				Ptr3 += Automap_width;
			}
			Ptr2++;
		}
		Ptr += Automap_width * 4;
	}

	/* Insert surface */
	Ptr = Buffer + (4 * Automap_width) + 4;
	for (i=0;i<(Automap_height - 8) / 4;i++)
	{
		Ptr2 = Ptr;
		for (j=0;j<(Automap_width - 8) / 4;j++)
		{
			Seed = Seed * 17 + 87;
			Base = AUTOMAP_SURFACE + 16 * Surface_distribution[(Seed & 0x00F0) >> 4];
			for (k=0;k<4;k++)
			{
				Ptr3 = Ptr2;
				for (l=0;l<4;l++)
				{
					Ptr3->Layer_1 = Base + k * 4 + l;
					Ptr3 += Automap_width;
				}
				Ptr2++;
			}
		}
		Ptr += Automap_width * 4;
	}

	MEM_Free_pointer(Automap_buffer_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Prepare_automap_floors
 * FUNCTION  : Prepare the automap floors.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.02.95 15:18
 * LAST      : 14.02.95 15:18
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will also build and install the automap
 *              palette.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Prepare_automap_floors(void)
{
	struct BBPALETTE Temporary_palette;
	struct Texture_3D *Texture;
	MEM_HANDLE Handle;
	UNSHORT i;
	UNBYTE *Ptr, *Ptr2;

	/* Allocate automap floor memory */
	Automap_floors_handle = MEM_Allocate_memory(Nr_of_floors * 64);

	/* Load the current map palette file */
	Handle = Load_subfile(PALETTE, Current_map_palette_nr);

	/* Copy colours to temporary palette */
	Ptr = MEM_Claim_pointer(Handle);
	for (i=0;i<192;i++)
	{
		Temporary_palette.color[i].red = *Ptr++;
		Temporary_palette.color[i].green = *Ptr++;
 		Temporary_palette.color[i].blue = *Ptr++;
	}

	/* Copy colours to main palette */
	Ptr -= 192 * 3;
	for (i=0;i<151;i++)
	{
		Palette.color[i].red = *Ptr++;
		Palette.color[i].green = *Ptr++;
		Palette.color[i].blue = *Ptr++;
	}

	/* Destroy palette file */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Load automap palette file */
	Handle = Load_subfile(PALETTE, AUTOMAP_PAL_NR);

	/* Copy colours to palette */
	Ptr = MEM_Claim_pointer(Handle) + 152 * 3;
	for (i=152;i<192;i++)
	{
		Palette.color[i].red = *Ptr++;
		Palette.color[i].green = *Ptr++;
 		Palette.color[i].blue = *Ptr++;
	}

	/* Destroy palette file */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Copy top 64 colours to temporary palette */
	for (i=192;i<256;i++)
	{
		Temporary_palette.color[i].red = Palette.color[i].red;
		Temporary_palette.color[i].green = Palette.color[i].green;
 		Temporary_palette.color[i].blue = Palette.color[i].blue;
	}

	/* Update palette */
	Update_palette();

	/* Convert 3D floors */
	Ptr = MEM_Claim_pointer(Automap_floors_handle);
	for (i=0;i<Nr_of_floors;i++)
	{
		/* Get pointer to floor graphics */
		Texture = &Floor_textures[i][0];
		Ptr2 = (UNBYTE *) ((((UNLONG) MEM_Claim_pointer(Texture->p.Handle) + 256) & 0xffffff00) + Texture->X);

		/* Shrink floor */
		Shrink_floor(&Temporary_palette, Ptr2, Ptr);

		MEM_Claim_pointer(Texture->p.Handle);

		/* Next floor */
		Ptr += 64;
	}
	MEM_Free_pointer(Automap_floors_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Shrink_floor
 * FUNCTION  : Shrink a 3D floor for the automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.02.95 14:52
 * LAST      : 14.02.95 14:52
 * INPUTS    : struct BBPALETTE *Source_pal - Pointer to "source palette",
 *              which belongs to the floor graphics.
 *             UNBYTE *Input - Pointer to floor graphics.
 *             UNBYTE *Output - Pointer to shrunken floor graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The "target palette" is the currently installed palette.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Shrink_floor(struct BBPALETTE *Source_pal, UNBYTE *Input, UNBYTE *Output)
{
	UNLONG Total_R, Total_G, Total_B;
	SISHORT i;
	UNSHORT j, k, l, c;
	UNBYTE *Ptr;

	for (i=7;i>=0;i--)
	{
		for (j=0;j<8;j++)
		{
			/* Calculate pointer to eight-by-eight block */
			Ptr = Input + (j * 256 + i) * 8;

			/* Clear */
			Total_R = 0;
			Total_G = 0;
			Total_B = 0;

			/* Add up colours within eight-by-eight block */
			for (k=0;k<8;k++)
			{
				for (l=0;l<8;l++)
				{
					/* Get pixel */
					c = *Ptr++;

					/* Add up R, G, and B values */
					Total_R += Source_pal->color[c].red;
					Total_G += Source_pal->color[c].green;
					Total_B += Source_pal->color[c].blue;
				}

				/* Next line */
				Ptr += 256 - 8;
			}

			/* Calculate mean R, G, and B values */
			Total_R >>= 6;
			Total_G >>= 6;
			Total_B >>= 6;

			/* Put shrunken pixel in output graphics */
			*Output++ = Find_closest_colour(Total_R, Total_G, Total_B);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Insert_automap_walls_and_floors
 * FUNCTION  : Insert walls and floors in automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.02.95 12:29
 * LAST      : 19.06.95 13:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Insert_automap_walls_and_floors(void)
{
	struct Lab_data *Lab_ptr;
	struct Square_3D *Square_ptr;
	struct Wall_3D *Wall_ptr;
	struct Object_group_3D *Group_ptr;
	struct Automap_entry *Automap_buffer_ptr;
	UNSHORT Icon;
	UNSHORT i, j;
	UNSHORT c;
	UNBYTE *Map_data_ptr;

	Map_data_ptr = MEM_Claim_pointer(Map_handle);
	Square_ptr = (struct Square_3D *) (Map_data_ptr + Map_layers_offset);

	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	Automap_buffer_ptr = (struct Automap_entry *) MEM_Claim_pointer(Automap_buffer_handle);
	Automap_buffer_ptr += (Automap_width * Automap_Y_offset) + Automap_X_offset;

	for (i=1;i<=Map_height;i++)
	{
		for (j=1;j<=Map_width;j++)
		{
			/* Is this position known ? */
			if (Get_automap_bit(j, i))
			{
				/* Yes -> Is there a floor here ? */
				if (Square_ptr->Floor_layer)
				{
					/* Yes -> Insert floor */
					Automap_buffer_ptr->Layer_1 = (UNSHORT) Square_ptr->Floor_layer
					 + 10000 - 1;

					if (Automap_buffer_ptr->Layer_1 > 20000)
					{
						Automap_buffer_ptr->Layer_1 = 0;
					}
				}

				/* Is there a wall or object group here ? */
				c = (UNSHORT) Square_ptr->Wall_layer;
				if (c)
				{
					/* Is it a wall ? */
					if (c >= FIRST_WALL)
					{
						/* Yes -> Get wall automapper icon */
						Wall_ptr = (struct Wall_3D *) (((UNBYTE *) Lab_ptr)
						 + Wall_offsets[c - FIRST_WALL]);
						Icon = Wall_ptr->Automapper_icon;
					}
					else
					{
						/* No -> Get object group automapper icon */
						Group_ptr = (struct Object_group_3D *)
						 ((c - 1) * sizeof(struct Object_group_3D) + (UNBYTE *) Lab_ptr);
						Icon = Group_ptr->Automapper_icon;
					}

					/* Is wall icon ? */
					if (Icon == WALL_AUTO_ICON)
					{
						/* Yes ->Insert wall */
						Insert_automap_wall(j, i);
					}
					else
					{
						/* No -> Insert icon */
						Insert_automap_icon(j, i, Icon);
					}
				}
			}

			/* Next map position */
			Square_ptr++;
			Automap_buffer_ptr++;
		}
		Automap_buffer_ptr += Automap_width - Map_width;
	}

	MEM_Free_pointer(Automap_buffer_handle);
	MEM_Free_pointer(Lab_data_handle);
	MEM_Free_pointer(Map_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Insert_automap_events
 * FUNCTION  : Insert events in automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.02.95 17:46
 * LAST      : 15.06.95 14:21
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Insert_automap_events(void)
{
	struct Map_event_entry *Entry_data;
	UNLONG Offset, Bit_nr;
	UNSHORT Nr_entries, Chain_nr, Icon;
	UNSHORT i, j;
	UNBYTE *Map_ptr, *Automap_event_ptr;

	Map_ptr = MEM_Claim_pointer(Map_handle);
	Offset = Event_entry_offset;
	Automap_event_ptr = Map_ptr + Event_automap_offset;

	/* Skip global events */
	Nr_entries = *((UNSHORT *) (Map_ptr + Offset));
	Offset += 2 + (Nr_entries * sizeof(struct Map_event_entry));

	/* Search all Y-coordinates */
	for (i=1;i<=Map_height;i++)
	{
		Nr_entries = *((UNSHORT *) (Map_ptr + Offset));
		Offset += 2;
		Entry_data = (struct Map_event_entry *) (Map_ptr + Offset);

		/* Search all X-coordinates */
		for (j=0;j<Nr_entries;j++)
		{
			/* Not deleted ? */
			if (Entry_data[j].First_block_nr != 0xFFFF)
			{
				/* No -> Is this position known ? */
				if (Get_automap_bit(Entry_data[j].X, i))
				{
					/* Yes -> Get chain number */
					Chain_nr = Get_event_chain_nr(Map_ptr,
					 Entry_data[j].First_block_nr);
					if (Chain_nr != 0xFFFF)
					{
						/* Has this event been saved ? */
						Bit_nr = ((PARTY_DATA.Map_nr - 1) * EVENTS_PER_MAP) +
						 Chain_nr;
						if (!Read_bit_array(EVENT_SAVE_BIT_ARRAY, Bit_nr))
						{
							/* No -> Get automapper icon */
							Icon = (UNSHORT) Automap_event_ptr[Chain_nr];

/*
	btst	d2,d4			; Yes -> Hidden ?
	beq.s	.Not_hidden1
	btst	#Show_traps_function,Automap_function	; Yes -> Show ?
	beq.s	.Next_X
	bra.s	.Show1
.Not_hidden1:
	jsr	Check_automap_door		; Check doors & chests
	jsr	Check_automap_chest
.Show1:	jsr	Insert_automap_icon		; Insert
*/

							/* Is wall icon ? */
							if (Icon == WALL_AUTO_ICON)
							{
								/* Yes ->Insert wall */
								Insert_automap_wall(Entry_data[j].X, i);
							}
							else
							{
								/* No -> Insert icon */
								Insert_automap_icon(Entry_data[j].X, i, Icon);
							}
						}
					}
				}
			}
		}
		/* Next Y */
		Offset += Nr_entries * sizeof(struct Map_event_entry);
	}

	MEM_Free_pointer(Map_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Insert_automap_NPCs
 * FUNCTION  : Insert NPCs in automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.02.95 15:11
 * LAST      : 20.02.95 15:11
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Insert_automap_NPCs(void)
{
	struct Lab_data *Lab_ptr;
	struct Object_group_3D *Group_ptr;
	UNLONG Bit_nr;
	UNSHORT Chain_nr;
	UNSHORT Icon;
	UNSHORT X, Y;
	UNSHORT NPC_type;
	UNSHORT i;
	UNBYTE *Map_ptr;
	UNBYTE *Automap_event_ptr;

	Map_ptr = MEM_Claim_pointer(Map_handle);
	Automap_event_ptr = Map_ptr + Event_automap_offset;

	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	/* Insert all NPCs */
	for (i=0;i<NPCS_PER_MAP;i++)
	{
		Icon = 0;

		/* Anyone there ? */
		if (NPC_present(i))
		{
			/* Yes -> Get coordinates */
			X = VNPCs[i].Map_X;
			Y = VNPCs[i].Map_Y;

			/* Is this position known ? */
			if (Get_automap_bit(X, Y))
			{
				/* Yes -> Any event ? */
				if (VNPCs[i].First_block_nr != 0xFFFF)
				{
					/* Yes -> Get event chain index */
					Chain_nr = Get_event_chain_nr(Map_ptr, VNPCs[i].First_block_nr);
					if (Chain_nr != 0xFFFF)
					{
						/* Has this event been saved ? */
						Bit_nr = ((PARTY_DATA.Map_nr - 1) * EVENTS_PER_MAP) + Chain_nr;
						if (!Read_bit_array(EVENT_SAVE_BIT_ARRAY, Bit_nr))
						{
							/* No -> Get automapper icon */
							Icon = (UNSHORT) Automap_event_ptr[Chain_nr];

/*
	btst	d2,d4			; Yes -> Hidden ?
	beq.s	.Not_hidden1
	btst	#Show_traps_function,Automap_function	; Yes -> Show ?
	beq.s	.Next_X
	bra.s	.Show1
.Not_hidden1:
	jsr	Check_automap_door		; Check doors & chests
	jsr	Check_automap_chest
.Show1:	jsr	Insert_automap_icon		; Insert
*/

						}
					}
				}
				else
				{
					/* No -> Get NPC type */
					NPC_type = VNPCs[i].NPC_type;

					/* Select automapper icon depending on NPC type */
					switch (NPC_type)
					{
						/* Party member */
						case PARTY_NPC:
						{
							/* Show NPCs ? */
							if (Automap_function_flags & SHOW_NPCS_FUNCTION)
							{
								/* Yes -> Set icon */
								Icon = PERSON_AUTO_ICON;
							}
							break;
						}
						/* NPC */
						case NPC_NPC:
						{
							/* Show NPCs ? */
							if (Automap_function_flags & SHOW_NPCS_FUNCTION)
							{
								/* Yes -> Set icon */
								Icon = PERSON_AUTO_ICON;
							}
							break;
						}
						/* Monster */
						case MONSTER_NPC:
						{
							/* Show monsters ? */
							if (Automap_function_flags & SHOW_MONSTERS_FUNCTION)
							{
								/* Yes -> Set icon */
								Icon = PERSON_AUTO_ICON;
							}
							break;
						}
						/* Object */
						case OBJECT_NPC:
						{
							/* No -> Get object group automapper icon */
							Group_ptr = (struct Object_group_3D *)
							 ((VNPCs[i].Graphics_nr - 1) *
							 sizeof(struct Object_group_3D) + (UNBYTE *) Lab_ptr);
							Icon = Group_ptr->Automapper_icon;
							break;
						}
					}
				}

				/* Any icon ? */
				if (Icon)
				{
					/* Yes -> Is wall icon ? */
					if (Icon == WALL_AUTO_ICON)
					{
						/* Yes ->Insert wall */
						Insert_automap_wall(X, Y);
					}
					else
					{
						/* No -> Insert icon */
						Insert_automap_icon(X, Y, Icon);
					}
				}
			}
		}
	}

	MEM_Free_pointer(Lab_data_handle);
	MEM_Free_pointer(Map_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Insert_automap_goto_points
 * FUNCTION  : Insert goto-points in automap.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.02.95 18:09
 * LAST      : 20.02.95 18:09
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Insert_automap_goto_points(void)
{
	struct Goto_point *Ptr;
	UNSHORT i;

	/* Get goto-point data */
	Ptr = (struct Goto_point *) (MEM_Claim_pointer(Map_handle) +
	 Goto_point_offset);

	/* Check all goto-points in this map */
	for(i=0;i<Nr_goto_points;i++)
	{
		/* Cheat mode / goto-point known ? */
		if (Cheat_mode || Read_bit_array(GOTO_POINTS_BIT_ARRAY, Ptr[i].Bit_nr))
		{
			/* Yes -> Insert icon */
			Insert_automap_icon((SISHORT) Ptr[i].X, (SISHORT) Ptr[i].Y,
			 GOTO_POINT_AUTO_ICON);
		}
	}

	MEM_Free_pointer(Map_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Insert_automap_wall
 * FUNCTION  : Insert a wall in the automap buffer.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 17:54
 * LAST      : 07.02.95 17:54
 * INPUTS    : SISHORT X - X-coordinate (1...).
 *             SISHORT Y - Y-coordinate (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Insert_automap_wall(SISHORT X, SISHORT Y)
{
	struct Automap_entry *Ptr;
	SISHORT X2, Y2;
	UNSHORT Wall_type;
	UNSHORT i;

	/* Determine wall type */
	Wall_type = 0;
	for (i=0;i<4;i++)
	{
		/* Get coordinates for current direction */
		X2 = X + Offsets4[i][0];
		Y2 = Y + Offsets4[i][1];

		/* Position known ? */
		if (Get_automap_bit(X2, Y2))
		{
			/* Yes -> Blocking vision ? */
			if (Vision_blocked(X2, Y2))
			{
				/* Yes -> Set flag */
				Wall_type |= (1<<i);
			}
		}
	}

	#ifdef URGLE
	/* Is full wall type ? */
	if (Wall_type == 15)
	{
		/* Yes -> Filled or crossroads ? */
		for (i=1;i<8;i+=2)
		{
			/* Get coordinates for current direction */
			X2 = X + Offsets8[i][0];
			Y2 = Y + Offsets8[i][1];

			/* Position known ? */
			if (Get_automap_bit(X2, Y2))
			{
				/* Yes -> Blocking vision ? */
				if (Vision_blocked(X2, Y2))
				{
					/* Yes -> Filled wall */
					Wall_type = 16;
					break;
				}
			}
		}
	}
	#endif

	#ifdef URGLE
	/* Can the player see hidden walls ? */
	if (Automap_function_flags & SHOW_HIDDEN_FUNCTION)
	{
		/* Yes -> Is this wall hidden ? */
		Status = Get_location_status(X, Y, 0xFFFF);
		if (!(Status & (1 << (BLOCKED_TRAVELMODES_B))))
		{
			/* Yes */
			Wall_type += 16;
		}
	}
	#endif

	/* Insert wall in automap buffer */
	Ptr = (struct Automap_entry *) MEM_Claim_pointer(Automap_buffer_handle);
	Ptr += (Automap_Y_offset + Y - 1) * Automap_width + Automap_X_offset + X - 1;
	Ptr->Layer_1 = Wall_type + AUTOMAP_WALLS;
	MEM_Free_pointer(Automap_buffer_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Insert_automap_icon
 * FUNCTION  : Insert an icon in the automap buffer.
 * FILE      : AUTOMAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.02.95 18:12
 * LAST      : 20.02.95 18:12
 * INPUTS    : SISHORT X - X-coordinate (1...).
 *             SISHORT Y - Y-coordinate (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Insert_automap_icon(SISHORT X, SISHORT Y, UNSHORT Icon_nr)
{
	struct Automap_entry *Ptr;

	/* Legal icon number ? */
	if ((Icon_nr > 1) && (Icon_nr <= MAX_AUTO_ICONS))
	{
		/* Yes -> Insert icon in automap buffer */
		Ptr = (struct Automap_entry *) MEM_Claim_pointer(Automap_buffer_handle);
		Ptr += (Automap_Y_offset + Y - 1) * Automap_width + Automap_X_offset + X - 1;
		Ptr->Layer_2 = Icon_nr;
		MEM_Free_pointer(Automap_buffer_handle);
	}
}

#if FALSE
{
	static SISHORT Extra_view_table[8][3][2] = {
		{{-1, -2}, {0, -2}, {1, -2}},		/* N */
		{{1, -2}, {2, -2}, {2, -1}},		/* NE */
		{{2, -1}, {2, 0}, {2, 1}},			/* E */
		{{2, 1}, {2, 2}, {1, 2}},			/* SE */
		{{-1, 2}, {0, 2}, {1, 2}},			/* S */
		{{-2, 1}, {-2, 2}, {-1, 2}},		/* SW */
		{{-2, -1}, {-2, 0}, {-2, 1}},		/* W */
		{{-2, -2}, {-1, -2}, {-2, -1}}	/* NW */
	};

	SISHORT X, Y, X2, Y2, X3, Y3;
	UNSHORT i, j;

	/* Can the party see ? */
	if (Party_can_see())
	{
		/* Yes -> Get party coordinates */
		X = PARTY_DATA.X;
		Y = PARTY_DATA.Y;

		/* Update current position */
		Set_automap_bit(X, Y);

		/* Update around player */
		for (i=0;i<8;i++)
		{
			/* Get coordinates for current direction */
			X2 = X + Offsets8[i][0];
			Y2 = Y + Offsets8[i][1];

			/* Update position */
			Set_automap_bit(X2, Y2);

			/* Vision blocked ? */
			if (!Vision_blocked(X2, Y2))
			{
				/* No -> Extra view */
				for (j=0;j<3;j++)
				{
					/* Get extra view coordinates */
					X3 = X2 + Extra_view_table[i][j][0];
					Y3 = Y2 + Extra_view_table[i][j][1];

					/* Update position */
					Set_automap_bit(X2, Y2);
				}
			}
		}
	}
}
#endif

