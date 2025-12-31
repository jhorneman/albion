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
#include <BBOPM.H>
#include <BBEVENT.H>
#include <BBMEM.H>

#include <XLOAD.H>
#include <SCROLL.H>
#include <GFXFUNC.H>
#include <SORT.H>
#include <TEXT.H>

#include <ALBION.H>
#include <CONTROL.H>
#include <MAP.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <2D_MAP.H>
#include <2D_DISPL.H>
#include <ANIMATE.H>
#include <XFTYPES.H>
#include <FONT.H>
#include <GRAPHICS.H>

/* global variables */

struct Module M2_Mod = {
	0, SCREEN_MOD,
	M2_Main_loop,
	Init_2D_map,
	Exit_2D_map,
	NULL,NULL,
//	Init_2D_display,
//	Exit_2D_display,
	Display_2D_map
};

struct Method M2_methods[] = {
	{ INIT_METHOD, Init_2D_object },
	{ EXIT_METHOD, Exit_2D_object },
	{ RIGHT_METHOD, Exit_program },
	{ CUSTOMMOUSE_METHOD, Custommouse_2D },
	{ CUSTOMKEY_METHOD, Customkeys_2D }
};

struct Object_class M2_Class = {
	sizeof(struct Object),
	&M2_methods[0]
};

BOOLEAN Bumped, Moved;

UNLONG Yoho_X, Yoho_Y;
SISHORT Party_offset[6][3];

UNSHORT M2_Object;

MEM_HANDLE Icondata_handle[2], Icongfx_handle[2];
MEM_HANDLE Party_graphics_handles[6];

UNSHORT Party_gfx_nrs[6] = {
	1,2,3,1,2,3
};

struct Object_2D Party_objects[6];

UNSHORT Party_path[128][3];
UNSHORT Path_indices[6];
UNSHORT Party_frames[6];
UNSHORT Party_state[6];
UNSHORT Party_animation[] = {0,0,0,1,1,1,2,2,2,1,1,1};

BOOLEAN Big_guy;
SIBYTE Big_guy_list[] = {
	FALSE, FALSE, TRUE, FALSE
};

UNBYTE *Icon_graphics[2];
struct Icon_2D_data *Icon_data[2];

struct Scroll_data Scroll_2D;

struct OPM M2_OPM;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_2D_object
 * FUNCTION  : Initialize method of 2D map object.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 13:03
 * LAST      : 06.09.94 13:03
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_2D_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT i;

	Object->Rect.left += P->Rect.left;
	Object->Rect.top += P->Rect.top;
	Object->Rect.width = P->Rect.width;
	Object->Rect.height = P->Rect.height;

	for (i=0;i<4;i++)
	{
		Add_update_OPM(&(Scroll_2D.Scroll_OPMs[i]));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_2D_object
 * FUNCTION  : Exit method of 2D map object.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 13:03
 * LAST      : 06.09.94 13:03
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_2D_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT i;

	for (i=0;i<4;i++)
	{
		Remove_update_OPM(&(Scroll_2D.Scroll_OPMs[i]));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Custommouse_2D
 * FUNCTION  : Custom mouse method of 2D map object.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 13:03
 * LAST      : 06.09.94 13:03
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Custommouse_2D(struct Object *Object, union Method_parms *P)
{
	static UNSHORT Mice_2D[] = {
	 UPLEFT_MPTR, UP2D_MPTR, UPRIGHT_MPTR,
	 LEFT2D_MPTR, DEFAULT_MPTR, RIGHT2D_MPTR,
	 DOWNLEFT_MPTR, DOWN2D_MPTR, DOWNRIGHT_MPTR
	};
	UNSHORT X, Y;
	UNSHORT pX, pY, pW, pH;
	UNSHORT Q, State;

	/* Get mouse coordinates and button-state */
	X = Mouse_X;
	Y = Mouse_Y;
	State = Button_state;

	/* Exit if the mouse pointer is outside of the 2D map window */
	if ((X >= Screen_width) || (Y >= Panel_Y))
		return((UNLONG) FALSE);

	/* Determine dimensions of party leader */
	if (Big_guy)
	{
		pW = 32;
		pH = 48;
	}
	else
	{
		pW = 16;
		pH = 32;
	}

	/* Determine position of party leader */
	pX = Yoho_X - Scroll_2D.Playfield_X;
	pY = Yoho_Y - Scroll_2D.Playfield_Y - pH + 1;

	/* Determine state depending on position relative to party leader */
	Q = 0;
	if (X >= pX)
	{
		if (X < pX + pW)
			Q = 1;
		else
			Q = 2;
	}
	if (Y >= pY)
	{
		if (Y < pY + pH)
			Q += 3;
		else
			Q += 6;
	}

	/* Change mouse pointer depending on state */
	Change_mouse(&(Mouse_pointers[Mice_2D[Q]]));

	/* Exit if the right mouse-button is pressed */
	if (State & 0x010)
		return((UNLONG) FALSE);

	/* Is the left mouse-button pressed ? */
	if (Button_state & 0x0001)
	{
		/* Yes -> Move depending on state */
		switch (Q)
		{
			/* Try to move up-left */
			case 0:
				Make_2D_move(NORTH_WEST);
				break;
			/* Try to move up */
			case 1:
				Make_2D_move(NORTH8);
				break;
			/* Try to move up-right */
			case 2:
				Make_2D_move(NORTH_EAST);
				break;
			/* Try to move left */
			case 3:
				Make_2D_move(WEST8);
				break;
			/* No action */
			case 4:
				break;
			/* Try to move right */
			case 5:
				Make_2D_move(EAST8);
				break;
			/* Try to move down-left */
			case 6:
				Make_2D_move(SOUTH_WEST);
				break;
			/* Try to move down */
			case 7:
				Make_2D_move(SOUTH8);
				break;
			/* Try to move down-right */
			case 8:
				Make_2D_move(SOUTH_EAST);
				break;
		}
	}

	return((UNLONG) TRUE);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Customkeys_2D
 * FUNCTION  : Custom keys method of 2D map object.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 13:03
 * LAST      : 06.09.94 13:03
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Customkeys_2D(struct Object *Object, union Method_parms *P)
{
	UNSHORT Key_code;

	/* Get key-code */
	Key_code = (UNSHORT) (P->Event->sl_key_code);

	switch (Key_code)
	{
/*		case 'p':
			Scroll_display_OPM(&Scroll_2D, &Main_OPM, MAP2D_X, MAP2D_Y);
			Display_text(&Main_OPM, XString1);
			break; */
		case BLEV_UP:
			Make_2D_move(NORTH8);
			break;
		case BLEV_LEFT:
			Make_2D_move(WEST8);
			break;
		case BLEV_RIGHT:
			Make_2D_move(EAST8);
			break;
		case BLEV_DOWN:
			Make_2D_move(SOUTH8);
			break;
		case BLEV_ESC:
			Quit_program = TRUE;
			break;
		case BLEV_F11:
		{
		 	struct Map_data *Map_data;
			struct Square_2D *Map_ptr;
			UNSHORT B1,B2,B3;
			UNSHORT Underlay_nr, Overlay_nr;

			Map_data = (struct Map_data *) MEM_Claim_pointer(Map_handle);
			Map_ptr = (struct Square_2D *)(Map_data + 1);
			Map_ptr += PARTY_DATA.X-1 + ((PARTY_DATA.Y-1) * Map_width);

			/* Read bytes from map */
			B1 = (UNSHORT) Map_ptr->m[0];
			B2 = (UNSHORT) Map_ptr->m[1];
			B3 = (UNSHORT) Map_ptr->m[2];

			MEM_Free_pointer(Map_handle);

			/* Build overlay and underlay number */
			Underlay_nr = ((B2 & 0x0F) << 8) | B3;
			Overlay_nr = (B1 << 4) | (B2 >> 4);

			OPM_FillBox(&Panel_OPM, 181, 3, 80, 20, BLACK);

			Push_textstyle(&Diagnostic_text_style);
			Printf(&Panel_OPM, 182, 4, "Underlay : %lu", Underlay_nr);
			Printf(&Panel_OPM, 182, 14, "Overlay : %lu", Overlay_nr);
			Pop_textstyle();

			break;
		}
		case BLEV_F12:
			Cheat_mode = ~Cheat_mode;
			break;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_2D_map
 * FUNCTION  : Initialize 2D map module.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:14
 * LAST      : 28.07.94 12:14
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
	UNSHORT i,j;

	OPM_CreateVirtualOPM(&Main_OPM, &M2_OPM, 0, 0, Screen_width, Panel_Y);
	Push_root(&M2_OPM);

	{
		struct BBRECT x;

		x.left = 0;
		x.top = 0;
		x.width = Screen_width;
		x.height = Panel_Y;

		M2_Object = Add_object(0, &M2_Class, (UNBYTE *) &x);
	}

	/* Load party graphics */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);

	Big_guy = Big_guy_list[Map_ptr->ICON_SET_1_NR - 1];

	{
		UNSHORT i,j;

		if (Big_guy)
			j = PART1_GFX;
		else
			j = PART2_GFX;

		for (i=0;i<3;i++)
			Party_graphics_handles[i] = XLD_Load_subfile(&Xftypes[j],
			 Party_gfx_nrs[i]);
	}

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

	/* Initialize animation */
	Init_animation();

	/* Initialize party objects */
	{
		struct Gfx_header *Gfx;

//		for (i=0;i<3;i++)
		i = 0;
		{
			Gfx = (struct Gfx_header *) MEM_Claim_pointer(Party_graphics_handles[i]);

			Party_frames[i] = rand() % 12;

			Party_objects[i].Width = Gfx->Width;
			Party_objects[i].Height = Gfx->Height;
			Party_objects[i].Level = 0;
			Party_objects[i].Graphics_handle = Party_graphics_handles[i];

			Object_2D_list[Nr_2D_objects] = &Party_objects[i];
			Nr_2D_objects++;

			MEM_Free_pointer(Party_graphics_handles[i]);
		}
	}

	/* Initialize party path */
	{
		UNSHORT X,Y;

		X = (PARTY_DATA.X-1) * 16;
	 	Y = (PARTY_DATA.Y-1) * 16 + 15;

		for (i=0;i<128;i++)
		{
			Party_path[i][0] = X;
			Party_path[i][1] = Y;
			Party_path[i][2] = PARTY_DATA.View_direction;
		}

		Yoho_X = Party_path[0][0];
		Yoho_Y = Party_path[0][1];

		for (i=0;i<3;i++)
		{
			j = (2-i) * 2;
			if (Big_guy)
				j *= 2;

			Path_indices[i] = j;

			Party_objects[i].X = Party_path[j][0];
			Party_objects[i].Y = Party_path[j][1];

			Party_objects[i].Graphics_offset = ((Party_path[j][2] * 3
			 + Party_animation[Party_frames[i]])
			 * Party_objects[i].Width * Party_objects[i].Height)
			 + sizeof(struct Gfx_header);

			if (i)
			{
				Party_offset[i][0] = (rand() & 7) - 3;
				Party_offset[i][1] = 0 - (rand() & 3);
				Party_offset[i][2] = 0 - (rand() & 3);
			}
			else
			{
				Party_offset[i][0] = 0;
				Party_offset[i][1] = 0;
				Party_offset[i][2] = 0;
			}
		}
	}

	/* Initialize scrolling */
	Scroll_2D.Update_unit = Update_2D_unit;
	Scroll_2D.Unit_width = 16;
	Scroll_2D.Unit_height = 16;
	Scroll_2D.Viewport_width = Screen_width;
	Scroll_2D.Viewport_height = Panel_Y;
	Scroll_2D.Playfield_width = Map_width * 16 ;
	Scroll_2D.Playfield_height = Map_height * 16;

	Icon_data[0] = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[0]);
	Icon_data[1] = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[1]);
	Icon_graphics[0] = MEM_Claim_pointer(Icongfx_handle[0]);
	Icon_graphics[1] = MEM_Claim_pointer(Icongfx_handle[1]);

	{
		UNLONG X,Y;

		X = Yoho_X - 180 + 16;
		Y = Yoho_Y - 96 - 8;

		Init_scroll(&Scroll_2D, 0, 0);
	}

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
 * #FUNCTION HEADER BEGIN#
 * NAME      :	Exit_2D_map
 * FUNCTION  : Exit 2D map module.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:00
 * LAST      : 28.07.94 12:00
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
	UNSHORT i;

	Exit_scroll(&Scroll_2D);

	Pop_root();
	OPM_Del(&M2_OPM);

	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);

	MEM_Free_memory(Icondata_handle[0]);
	MEM_Free_memory(Icongfx_handle[0]);

	if (Map_ptr->ICON_SET_2_NR)
	{
		MEM_Free_memory(Icondata_handle[1]);
		MEM_Free_memory(Icongfx_handle[1]);
	}

	MEM_Free_pointer(Map_handle);
	MEM_Free_memory(Map_handle);

	for (i=0;i<6;i++)
		MEM_Free_memory(Party_graphics_handles[i]);

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M2_Main_loop
 * FUNCTION  : 2D map main loop.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 11:19
 * LAST      : 06.09.94 11:19
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
M2_Main_loop(void)
{
	Update_display();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_2D_map
 * FUNCTION  : Display the 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:18
 * LAST      : 28.07.94 12:18
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
	Yog_sototh();

	/* Initialize pointers */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);
	Icon_data[0] = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[0]);
	Icon_data[1] = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[1]);
	Icon_graphics[0] = MEM_Claim_pointer(Icongfx_handle[0]);
	Icon_graphics[1] = MEM_Claim_pointer(Icongfx_handle[1]);

	/* Update display */
	{
		UNLONG X,Y;

		X = Yoho_X - 180 + 16;
		Y = Yoho_Y - 96 - 8;

		Set_scroll_position(&Scroll_2D, X, Y);
		Scroll_display(&Scroll_2D, &Screen, MAP2D_X, MAP2D_Y);

		Draw_2D_map();
	}

	/* Free pointers */
	MEM_Free_pointer(Icongfx_handle[0]);
	MEM_Free_pointer(Icongfx_handle[1]);
	MEM_Free_pointer(Icondata_handle[0]);
	MEM_Free_pointer(Icondata_handle[1]);
	MEM_Free_pointer(Map_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_2D_move
 * FUNCTION  : Make a move in a 2D.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 06.09.94 17:54
 * INPUTS    : UNSHORT Direction - Direction (0...7).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_2D_move(UNSHORT Direction)
{
	Moved = FALSE;
	Bumped = FALSE;

	/* Allowed by before-move logic ? */
	if (Before_move())
	{
		/* Yes -> Try to move */
		if (Try_2D_move(PARTY_DATA.X, PARTY_DATA.Y, Direction))
		{
			Moved = TRUE;
			After_move();
		}
		else
			Bumped = TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_2D_move
 * FUNCTION  : Check if a certain move can be made in a 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 08.09.94 16:42
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 *             UNSHORT Direction - Direction (0...7).
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Try_2D_move(UNSHORT X, UNSHORT Y, UNSHORT Direction)
{
	static UNSHORT View_directions[8] = {
	NORTH, NORTH, EAST, SOUTH, SOUTH, SOUTH, WEST, NORTH
	};
	static UNSHORT Direction_pairs[8][2] = {
	{NORTH, NORTH}, {NORTH, EAST}, {EAST, EAST}, {SOUTH, EAST},
	{SOUTH, SOUTH}, {SOUTH, WEST}, {WEST, WEST}, {NORTH, WEST}
	};
	static SISHORT DVectors[4][2] = {{0,1}, {1,1}, {2,1}, {2,0}};

	static UNSHORT Slide_direction = 0xFFFF;

	UNLONG Status;
	BOOLEAN Result = TRUE, Rotate_180, Slide_flag;
	SISHORT dX, dY, ddDir, corrX = 0;
	UNSHORT cDir, dDir;
	UNSHORT cVD, tVD = 0xFFFF, ntVD;
	UNSHORT Q, i, h;

	/* Can this location be left in this direction ? */
	Status = Get_location_status(X,Y);
	if ((Status & (1 << (BLOCKED_DIRECTIONS_B + Direction_pairs[Direction][0])))
	 && (Status & (1 << (BLOCKED_DIRECTIONS_B + Direction_pairs[Direction][1]))))
		return(FALSE);

	/* Get movement vector */
	dX = Offsets8[Direction][0];
	dY = Offsets8[Direction][1];

	/* Get current direction */
	cVD = PARTY_DATA.View_direction;
	cDir = cVD * 2;

	/* Calculate direction difference */
	if (Direction >= cDir)
	{
		dDir = Direction - cDir;
		ddDir = 1;
	}
	else
	{
		dDir = cDir - Direction;
		ddDir = -1;
	}

	if (dDir > 4)
	{
		dDir = 8 - dDir;
		ddDir = 0 - ddDir;
	}

	/* Does the player want to move orthogonally and is the difference in
	 directions equal to 90 degrees ? */
	if ((dDir == 2) && (!(Direction & 1)))
	{
		/* Yes -> The player must be rotated */
		tVD = View_directions[Direction];
	}

	/* Is the difference in directions greater than 90 degrees ? */
	if (dDir > 2)
	{
		/* Yes -> The player must be rotated. An extra rotation step is needed
		 between the current and the target direction. This extra view (!)
		 direction must be determined. */
		tVD = View_directions[(cDir + (ddDir * ((dDir + 1) / 2))) & 0x0007];
	}

	/* Is rotation of the player necessary ? */
	if (tVD != 0xFFFF)
	{
		/* Yes -> Rotate. */
		PARTY_DATA.View_direction = tVD;
		Move_2D(0,0);

		return(TRUE);
	}

	/* Does the player want to move diagonally ? */
	if (Direction & 1)
	{
		/* Yes -> First we must determine the block state for the
			current position */
		Q = 0;

		if ((dX < 0) && Big_guy)
			corrX = 1;

		for (i=0;i<4;i++)
		{
			if (!Do_check_2D(X + (DVectors[i][0] * dX) + corrX,
				Y + (DVectors[i][1] * dY)))
				Q |= (1<<i);
		}

		/* Then we act according to the block state */
		switch (Q)
		{
			/* Movement is possible */
			case 0:
				Move_2D(dX, dY);
				break;
			/* Movement is blocked */
			case 9:
			case 10:
			case 11:
			case 13:
			case 14:
			case 15:
				Result = FALSE;
				break;
			/* Try a diagonal move */
			case 8:
				Status = Get_location_status(X + 2 * dX, Y);
				h = (Status & ICON_HEIGHT) >> ICON_HEIGHT_B;
				if (h > 0)
				{
					Move_2D(dX, dY);
				}
				else
				{
					/* Try a vertical move */
					if (Movement_check_2D(X, Y + dY))
					{
						if (dY < 0)
							ntVD = NORTH;
						else
							ntVD = SOUTH;

						if (ntVD == Slide_direction)
							PARTY_DATA.View_direction = ntVD;

						Move_2D(0, dY);

						Slide_flag = TRUE;
						Slide_direction = ntVD;
					}
					else
						Result = FALSE;
				}
				break;
			/* Try a horizontal slide */
			case 1:
			case 2:
			case 3:
			case 5:
			case 6:
			case 7:
				if (Movement_check_2D(X + dX, Y))
				{
					if (dX < 0)
						ntVD = WEST;
					else
						ntVD = EAST;

					if (ntVD == Slide_direction)
						PARTY_DATA.View_direction = ntVD;

					Move_2D(dX, 0);

					Slide_flag = TRUE;
					Slide_direction = ntVD;
				}
				else
					Result = FALSE;
				break;
			/* Try a vertical slide */
			case 12:
				if (Movement_check_2D(X, Y + dY))
				{
					if (dY < 0)
						ntVD = NORTH;
					else
						ntVD = SOUTH;

					if (ntVD == Slide_direction)
						PARTY_DATA.View_direction = ntVD;

					Move_2D(0, dY);

					Slide_flag = TRUE;
					Slide_direction = ntVD;
				}
				else
					Result = FALSE;
				break;
			/* Try a horizontal OR a vertical slide */
			case 4:
				if (!Do_check_2D(X - dX + corrX,	Y + dY))
				{
					if (Movement_check_2D(X, Y + dY))
					{
						if (dY < 0)
							ntVD = NORTH;
						else
							ntVD = SOUTH;

						if (ntVD == Slide_direction)
							PARTY_DATA.View_direction = ntVD;

						Move_2D(0, dY);

						Slide_flag = TRUE;
						Slide_direction = ntVD;
					}
					else
						Result = FALSE;
				}
				else
				{
					if (!Do_check_2D(X + 2 * dX + corrX, Y - dY))
					{
						/* Try a horizontal slide */
						if (Movement_check_2D(X + dX, Y))
						{
							if (dX < 0)
								ntVD = WEST;
							else
								ntVD = EAST;

							if (ntVD == Slide_direction)
								PARTY_DATA.View_direction = ntVD;

							Move_2D(dX, 0);

							Slide_flag = TRUE;
							Slide_direction = ntVD;
						}
						else
							Result = FALSE;
					}
					else
						Result = FALSE;
				}
				break;
		}
	}
	else
	{
		/* No -> Is the current view direction horizontal or vertical ? */
		if (cVD & 1)
		{
			/* Horizontal movement */
			/* Determine the block state for the current position */
			Q = 0;

			if ((dX > 0) && Big_guy)
				corrX = 1;

			for (i=0;i<3;i++)
			{
				if (!Do_check_2D(X + dX + corrX, Y + i - 1))
					Q |= (1<<i);
			}

			/* Then we act according to the block state */
			switch (Q)
			{
				/* Movement is possible */
				case 0:
				case 1:
				case 4:
				case 5:
					Move_2D(dX, dY);
					break;
				/* Movement is blocked */
				case 7:
					Result = FALSE;
					break;
				/* Try a vertical slide up */
				case 2:
				case 6:
					if (Movement_check_2D(X, Y - 1))
						Move_2D(0, -1);
					else
						Result = FALSE;
					break;
				/* Try a vertical slide down */
				case 3:
					if (Movement_check_2D(X, Y + 1))
						Move_2D(0, 1);
					else
						Result = FALSE;
					break;
			}
		}
		else
		{
			/* Vertical movement */
			/* Determine the block state for the current position */
			Q = 0;
			for (i=0;i<4;i++)
			{
				if (!Do_check_2D(X + i - 1, Y + dY))
					Q |= (1<<i);
			}

			/* Then we act according to the block state */
			switch (Q)
			{
				/* Movement is possible */
				case 0:
				case 1:
				case 8:
				case 9:
					Move_2D(dX, dY);
					break;
				/* Movement is blocked */
				case 5:
				case 6:
				case 10:
				case 11:
				case 13:
				case 15:
					Result = FALSE;
					break;
				/* Try a horizontal slide to the right */
				case 2:
				case 3:
				case 7:
					if (Movement_check_2D(X + 1, Y))
						Move_2D(1, 0);
					else
						Result = FALSE;
					break;
				/* Try a horizontal slide to the left */
				case 4:
				case 12:
				case 14:
					if (Movement_check_2D(X - 1, Y))
						Move_2D(-1, 0);
					else
						Result = FALSE;
					break;
			}

		}
	}

	/* Any sliding this time ? */
	if (!Slide_flag)
		/* No -> Clear */
		Slide_direction = 0xFFFF;

	return(Result);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Movement_check_2D
 * FUNCTION  : Check if a certain position in a 2D map can be moved to.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 06.09.94 17:54
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * NOTES     : - This function will also check the square next to the given
 *              location in 2D maps where big characters are shown.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Movement_check_2D(UNSHORT X, UNSHORT Y)
{
	if (Big_guy)
		return(Do_check_2D(X,Y) & Do_check_2D(X+1,Y));
	else
		return(Do_check_2D(X,Y));
}

BOOLEAN
Do_check_2D(UNSHORT X, UNSHORT Y)
{
	UNLONG Status;

	/* Exit if target coordinates lie outside the map */
	if ((X < 1) || (X >= Map_width) || (Y < 1) || (Y >= Map_height))
		return(FALSE);

	/* Exit if cheat mode is on */
	if (Cheat_mode)
		return(TRUE);

	/* Get location status */
	Status = Get_location_status(X,Y);

	/* Check bits for current travelmode */
	if (Status & (1 << (BLOCKED_TRAVELMODES_B + PARTY_DATA.Travel_mode)))
		return(FALSE);
	else
		return(TRUE);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_2D
 * FUNCTION  : Move the party along a vector in a 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 06.09.94 17:54
 * INPUTS    : SISHORT dX - X-component of vector.
 *             SISHORT dY - Y-component of vector.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_2D(SISHORT dX, SISHORT dY)
{
	UNSHORT v,v2,i,k;
	SISHORT idx;
	UNSHORT X, Y;
	UNSHORT State1 = 0, State2;
	UNSHORT Frame;

	if (Big_guy)
	{
		X = PARTY_DATA.X + dX;
		Y = PARTY_DATA.Y + dY;

		State1 = ((Get_location_status(X,Y) & STATE) >> STATE_B);

		if (State1 != STANDING_STATE)
		{
			PARTY_DATA.X = X;
			PARTY_DATA.Y = Y;
			dX = 0;
			dY = 0;
		}
	}
	Party_state[0] = State1;

	if (Big_guy)
	{
		v = 4;
		v2 = 4;
	}
	else
	{
	v = 8;
	v2 = 2;
	}

	for (k=0;k<v;k++)
	{
		Nr_2D_objects = 0;

		for (i=0;i<1;i++)
		{
			idx = Path_indices[i];
			idx++;
			if (idx>=128)
			{
				idx -= 128;

				if (i)
				{
					Party_offset[i][0] = (rand() & 3) - 2;
					Party_offset[i][1] = 0 - (rand() & 3);
					Party_offset[i][2] = 0 - (rand() & 3);
				}
			}
			Path_indices[i] = idx;

			if (!i)
			{
				Party_path[idx][0] = (PARTY_DATA.X-1) * 16 + v2 * (dX * k);
				Party_path[idx][1] = (PARTY_DATA.Y-1) * 16 + 15 + v2 * (dY * k);
				Party_path[idx][2] = PARTY_DATA.View_direction;

				Yoho_X = Party_path[idx][0];
				Yoho_Y = Party_path[idx][1];
			}

			idx += Party_offset[i][2];
			if (idx<0)
				idx += 128;
			if (idx>=128)
				idx -= 128;

			Party_objects[i].X = Party_path[idx][0]; // + Party_offset[i][0];
			Party_objects[i].Y = Party_path[idx][1]; // + Party_offset[i][1];

			switch (Party_state[i])
			{
				case SITTING_NL_STATE:
					Frame = 12;
					break;
				case SITTING_NR_STATE:
					Frame = 12;
					Party_objects[i].X -= 16;
					break;
				case SITTING_E_STATE:
					Frame = 13;
					break;
				case SITTING_SL_STATE:
					Frame = 14;
					break;
				case SITTING_SR_STATE:
					Frame = 14;
					Party_objects[i].X -= 16;
					break;
				case SITTING_W_STATE:
					Frame = 15;
					break;
				default:
					Frame = Party_path[idx][2] * 3 + Party_animation[Party_frames[i]];
					break;
			}

			Party_objects[i].Graphics_offset = Frame * (Party_objects[i].Width
			 * Party_objects[i].Height) + sizeof(struct Gfx_header);

			if (dX || dY)
				Party_frames[i] = (Party_frames[i] + 1) % 12;

			Object_2D_list[Nr_2D_objects] = &Party_objects[i];
			Nr_2D_objects++;
		}

 		Display_2D_map();

		if (k < (v-1))
		{
			Switch_screens();
		}
	}

	PARTY_DATA.X += dX;
	PARTY_DATA.Y += dY;
}

		#ifdef URGLE
		State2 = ((Get_location_status(X+1,Y) & STATE) >> STATE_B);

		switch (State1)
		{
			case STANDING_STATE:
				if (dX >= 0)
				{
					if ((State2 == SITTING_N_STATE) || (State2 == SITTING_E_STATE)
					 || (State2 == SITTING_S_STATE) || (State2 == SITTING_W_STATE))
					{
						X++;
						PARTY_DATA.X = X;
						PARTY_DATA.Y = Y;
						dX = 0;
						dY = 0;
						State1 = State2;
					}
				}
				else
				{
					State2 = Party_state[0];
					if ((State2 == SITTING_N_STATE) || (State2 == SITTING_E_STATE)
					 || (State2 == SITTING_S_STATE) || (State2 == SITTING_W_STATE))
					{
						dX *= 2;
					}
				}
				break;
			case SITTING_N_STATE:
			case SITTING_E_STATE:
			case SITTING_S_STATE:
			case SITTING_W_STATE:
				State2 = ((Get_location_status(X+1,Y) & STATE) >> STATE_B);
				if (State1 != State2)
				{
					State2 = ((Get_location_status(X-1,Y) & STATE) >> STATE_B);
					if (State1 == State2)
					{
						if ((Party_state[0] == State1) && (dX > 0))
							State1 = STANDING_STATE;
						else
							X--;
					}
				}
				PARTY_DATA.X = X;
				PARTY_DATA.Y = Y;
				dX = 0;
				dY = 0;
				break;
		}
		#endif

