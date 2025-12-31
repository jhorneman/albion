/************
 * NAME     : 3D_MOVE.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 3-10-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>
#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBOPM.H>
#include <BBEVENT.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>

#include <XLOAD.H>
#include <GFXFUNC.H>

#include <CONTROL.H>
#include <AUTOMAP.H>
#include <MAP.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <3D_MAP.H>
#include <3D_PREP.H>
#include <3DCOMMON.H>
#include <XFTYPES.H>
#include <COLOURS.H>
#include <GRAPHICS.H>
#include <STATAREA.H>
#include <3D_MOVE.H>

/* defines */

#define TURN_3D_SPEED				(60)
#define MOVE_FORWARD_3D_SPEED		(230)
#define MOVE_LATERAL_3D_SPEED		(100)
#define MOVE_BACKWARD_3D_SPEED	(150)

#define SHIFT_3D_SPEED_FACTOR		(250)

#define CORNER_3D_SIZE				(20)

/* structure definitions */

struct Collision_3D {
	SISHORT dX, dY;
	UNSHORT Bit_mask;
};

/* prototypes */

/* 3D map move mode */
void Move_mouse_M3_ModInit(void);
void Move_mouse_M3_ModExit(void);
void Move_mouse_M3_MainLoop(void);

void Mouse_move_3D(void);

/* 3D map key move mode */
void Move_key_M3_ModInit(void);
void Move_key_M3_ModExit(void);
void Move_key_M3_MainLoop(void);

BOOLEAN Key_move_3D(void);

/* 3D map movement / collision detection functions */
void Make_3D_party_move(SILONG dX, SILONG dY);

BOOLEAN Try_3D_move(SILONG X, SILONG Y, UNSHORT NPC_index,
 UNSHORT Travel_mode);

BOOLEAN Check_3D_object_group_collision(SISHORT Square_X, SISHORT Square_Z,
 SISHORT Group_X, SISHORT Group_Z, UNSHORT Travel_mode,
 struct Object_group_3D *Object_group, struct Object_3D *Object_base);

/* global variables */

/* 3D map move mode module */
struct Module Move_mouse_M3_Mod =
{
	NO_INPUT_HANDLING, MODE_MOD, MAP_3D_SCREEN,
	Move_mouse_M3_MainLoop,
	Move_mouse_M3_ModInit,
	Move_mouse_M3_ModExit,
	NULL,
	NULL,
	Update_map_display
};

/* 3D map key move mode module */
struct Module Move_key_M3_Mod =
{
	NO_INPUT_HANDLING, MODE_MOD, MAP_3D_SCREEN,
	Move_key_M3_MainLoop,
	Move_key_M3_ModInit,
	Move_key_M3_ModExit,
	NULL,
	NULL,
	Update_map_display
};

/* 3D map mouse pointer table */
UNSHORT Mice_3D[] = {
	FORWARDLEFT_MPTR,	FORWARD_MPTR,	FORWARDRIGHT_MPTR,
	TURNLEFT_MPTR,		FORWARD_MPTR,	TURNRIGHT_MPTR,
	LEFT3D_MPTR,		POP_UP_MPTR,	RIGHT3D_MPTR,
	TURN180LEFT_MPTR,	BACKWARD_MPTR,	TURN180RIGHT_MPTR,
	DEFAULT_MPTR			// Outside of the map window
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Initialize_3D_position
 * FUNCTION  : Set the position in the 3D dungeon.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.94 14:15
 * LAST      : 07.09.94 14:15
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Initialize_3D_position(void)
{
	UNLONG X, Y;

	/* Convert current party coordinates to dungeon coordinates */
	Map_to_3D((SISHORT) PARTY_DATA.X, (SISHORT) PARTY_DATA.Y, &X, &Y);

	/* Store in 3D interface */
	*((UNLONG *) &(I3DM.Player_X[1])) = X;
	*((UNLONG *) &(I3DM.Player_Z[1])) = Y;

	/* Clear after the decimal point */
	I3DM.Player_X[0] = 0;
	I3DM.Player_Z[0] = 0;

	/* Set camera angle */
	I3DM.Camera_angle = ((0 - (PARTY_DATA.View_direction * (ANGLE_STEPS / 4)))
	 & (ANGLE_STEPS - 1)) * 65536;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_3D_mouse_state
 * FUNCTION  : Get state of mouse in 3D map window.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.09.94 11:24
 * LAST      : 21.09.94 11:24
 * INPUTS    : SISHORT X - X-coordinate of mouse
 *             SISHORT Y - Y-coordinate of mouse
 * RESULT    : UNSHORT : Mouse state (0...11 / 12 = out of window)
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_3D_mouse_state(SISHORT X, SISHORT Y)
{
	UNSHORT Width, Height;
	UNSHORT Q;

	/* Translate coordinates */
	X -= Window_3D_X;
	Y -= Window_3D_Y;

	/* Get dimensions of display window */
	Width		= I3DM.Window_3D_width;
	Height	= I3DM.Window_3D_height;

	/* Exit if the mouse pointer is outside of the 3D map window */
	if ((X <0) || (X >= Width) || (Y < 0) || (Y >= Height))
		return 12;

	/* Determine state depending on position in map window */
	Q = 0;

	if (X >= Width / 3)
	{
		if (X < Width - (Width / 3))
		{
			Q = 1;
		}
		else
		{
			Q = 2;
		}
	}

	if (Y >= Height / 10)
	{
		if (Y >= (Height * 6) / 10)
		{
			if (Y < Height - ((Height * 2) / 10))
			{
				Q += 6;
			}
			else
			{
				Q += 9;
			}
		}
		else
		{
			Q += 3;
		}
	}

	return Q;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_3D_mouse_state2
 * FUNCTION  : Get state of mouse in 3D map window, including the corners.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.10.95 18:23
 * LAST      : 07.10.95 18:26
 * INPUTS    : SISHORT X - X-coordinate of mouse
 *             SISHORT Y - Y-coordinate of mouse
 * RESULT    : UNSHORT : Mouse state (0...11 / 12 = out of window /
                 >=256 = corner state)
 * BUGS      : No known.
 * NOTES     : - It's not a pretty system but it works.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_3D_mouse_state2(SISHORT X, SISHORT Y)
{
	UNSHORT Width, Height;
	UNSHORT Q;

	/* Get normal mouse state */
	Q = Get_3D_mouse_state(X, Y);

	/* Translate coordinates */
	X -= Window_3D_X;
	Y -= Window_3D_Y;

	/* Get dimensions of display window */
	Width		= I3DM.Window_3D_width;
	Height	= I3DM.Window_3D_height;

	/* Is the mouse pointer inside of the 3D map window ? */
	if (Q < 12)
	{
		/* Yes -> Check if the mouse is in a corner */
		if (X < CORNER_3D_SIZE)
		{
			if (Y < CORNER_3D_SIZE)
			{
				Q = 256;
			}
			else
			{
				if (Y >= (Height - CORNER_3D_SIZE))
				{
					Q = 256 + 2;
				}
			}
		}
		else
		{
			if (X >= (Width - CORNER_3D_SIZE))
			{
				if (Y < CORNER_3D_SIZE)
				{
					Q = 256 + 1;
				}
				else
				{
					if (Y >= (Height - CORNER_3D_SIZE))
					{
						Q = 256 + 3;
					}
				}
			}
		}
	}

	return Q;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Rotate_3D
 * FUNCTION  : Change the camera angle in the 3D dungeon.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.94 14:22
 * LAST      : 07.09.94 14:22
 * INPUTS    : SISHORT dAlpha - Change of the angle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Rotate_3D(SISHORT dAlpha)
{
	I3DM.Camera_angle += (dAlpha * 65536);
	I3DM.Camera_angle &= (ANGLE_STEPS * 65536) - 1;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_3D_vector
 * FUNCTION  : Move along a vector in the 3D dungeon.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.94 14:23
 * LAST      : 07.09.94 14:23
 * INPUTS    : SILONG dX - X-component of vector.
 *             SILONG dY - Y-component of vector.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_3D_vector(SILONG dX, SILONG dY)
{
	UNLONG X,Y;
	SILONG fracX, fracY;

	/* Get integer part of coordinates */
	X = * ((UNLONG *) &(I3DM.Player_X[1]));
	Y = * ((UNLONG *) &(I3DM.Player_Z[1]));

	/* Get fractional part of coordinates */
	fracX = (SILONG) I3DM.Player_X[0];
	fracY = (SILONG) I3DM.Player_Z[0];

	/* Add vector to fractional part */
	fracX += dX;
	fracY += dY;

	/* Carry over to integer part */
	while (fracX >= 65536)
	{
		fracX -= 65536;
		X++;
	}
	while (fracY >= 65536)
	{
		fracY -= 65536;
		Y++;
	}

	while (fracX < 0)
	{
		fracX += 65536;
		X--;
	}
	while (fracY < 0)
	{
		fracY += 65536;
		Y--;
	}

	/* Store new coordinates */
	* ((UNLONG *) &(I3DM.Player_X[1])) = X;
	* ((UNLONG *) &(I3DM.Player_Z[1])) = Y;

	I3DM.Player_X[0] = fracX;
	I3DM.Player_Z[0] = fracY;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_mouse_M3_ModInit
 * FUNCTION  : Initialize 3D map mouse move mode module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 10:59
 * LAST      : 27.10.94 10:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_mouse_M3_ModInit(void)
{
	struct BBRECT MA;

	/* Create mouse area */
	MA.left		= Window_3D_X;
	MA.top		= Window_3D_Y;
	MA.width		= I3DM.Window_3D_width;
	MA.height	= I3DM.Window_3D_height;

	/* Install mouse area */
	Push_MA(&MA);

	/* Indicate move mode is on */
	Move_mode_flag = TRUE;

	/* Guarantee first movement */
	Mouse_move_3D();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_mouse_M3_ModExit
 * FUNCTION  : Exit 3D map mouse move mode module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 11:00
 * LAST      : 27.10.94 11:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_mouse_M3_ModExit(void)
{
	/* Remove mouse area */
	Pop_MA();

	/* Indicate move mode is off */
	Move_mode_flag = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_mouse_M3_MainLoop
 * FUNCTION  : 3D map mouse movement main loop.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 10:29
 * LAST      : 28.08.95 23:16
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_mouse_M3_MainLoop(void)
{
	UNSHORT Old_map_nr;

	/* Get old map number */
	Old_map_nr = PARTY_DATA.Map_nr;

	/* Update input handling */
	Update_input();

	/* Is the left mouse button pressed ? */
	if (Button_state & 0x0001)
	{
		/* Yes -> Move */
		Mouse_move_3D();

		/* Exit if no longer in the same map */
		if (PARTY_DATA.Map_nr != Old_map_nr)
			return;

		/* Display map */
		M3_MainLoop();
	}
	else
	{
		/* No -> Exit move mode */
		Pop_module();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Mouse_move_3D
 * FUNCTION  : Move in 3D mouse move mode.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 10:32
 * LAST      : 12.10.95 18:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Mouse_move_3D(void)
{
	struct Lab_data *Lab_ptr;
	SILONG F;
	SISHORT Conv;
	SISHORT Dom;
	SISHORT X, Y;
	SISHORT Duration;
	UNSHORT Width, Height;
	UNSHORT Q;

	/* Get mouse coordinates */
	X = Mouse_X;
	Y = Mouse_Y;

	/* Get duration of last frame */
	Duration = (SISHORT) Update_duration;

	/* Get 3D mouse state */
	Q = Get_3D_mouse_state(X, Y);

	/* Change mouse-pointer accordingly */
	Change_mouse(&(Mouse_pointers[Mice_3D[Q]]));

	/* Calculate conversion factor */
	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	Conv = 1 << (10 - Lab_ptr->Square_size_cm_log);

	MEM_Free_pointer(Lab_data_handle);

	/* Adjust mouse coordinates */
	X -= Window_3D_X;
	Y -= Window_3D_Y;

	/* Get dimensions of display window */
	Width		= I3DM.Window_3D_width;
	Height	= I3DM.Window_3D_height;

	/* Move depending on state */
	switch (Q)
	{
		/* Turn left and move forward */
		case 0:
		{
			Dom = Width / 3;

			F = (TURN_3D_SPEED * Duration * (Dom - X)) / Dom;

			Rotate_3D(F);

			Dom = ((Height * 60) / MOVE_LATERAL_3D_SPEED);

			F = (MOVE_FORWARD_3D_SPEED * Duration * (Dom - Y)) / Dom;

			F /= Conv;

			Make_3D_party_move(-F * yawSIN, F * yawCOS);
			break;
		}
		/* Move forward */
		case 1:
		case 4:
		{
			Dom = ((Height * 60) / MOVE_LATERAL_3D_SPEED);

			F = (MOVE_FORWARD_3D_SPEED * Duration * (Dom - Y)) / Dom;

			F /= Conv;

			Make_3D_party_move(-F * yawSIN, F * yawCOS);
			break;
		}
		/* Turn right and move forward */
		case 2:
		{
			Dom = Width / 3;

			F = (TURN_3D_SPEED * Duration * (X - (Width - Dom))) / Dom;

			Rotate_3D(-F);

			Dom = ((Height * 60) / MOVE_LATERAL_3D_SPEED);

			F = (MOVE_FORWARD_3D_SPEED * Duration * (Dom - Y)) / Dom;

			F /= Conv;

			Make_3D_party_move(-F * yawSIN, F * yawCOS);
			break;
		}
		/* Turn left */
		case 3:
		{
			Dom = Width / 3;

			F = (TURN_3D_SPEED * Duration * (Dom - X)) / Dom;

			Rotate_3D(F);
			break;
		}
		/* Turn right */
		case 5:
		{
			Dom = Width / 3;

			F = (TURN_3D_SPEED * Duration * (X - (Width - Dom))) / Dom;

			Rotate_3D(-F);
			break;
		}
		/* Move left */
		case 6:
		{
			Dom =  Width / 3;

			F = (MOVE_LATERAL_3D_SPEED * Duration * (Dom - X)) / Dom;

			F /= Conv;

			Make_3D_party_move(-F * yawCOS, -F * yawSIN);
			break;
		}
		/* Move right */
		case 8:
		{
			Dom = Width / 3;

			F = (MOVE_LATERAL_3D_SPEED * Duration * (X - (Width - Dom))) / Dom;

			F /= Conv;

			Make_3D_party_move(F * yawCOS, F * yawSIN);
			break;
		}
		/* Turn left and move backwards */
		case 9:
		{
			Dom = Width / 3;

			F = (TURN_3D_SPEED * Duration * (Dom - X)) / Dom;

			Rotate_3D(F);

			Dom = (Height * 2) / 10;

			F = (MOVE_BACKWARD_3D_SPEED * Duration * (Y - (Height - Dom))) / Dom;

			F /= Conv;

			Make_3D_party_move(F * yawSIN, -F * yawCOS);
			break;
		}
		/* Move backwards */
		case 10:
		{
			Dom = (Height * 2) / 10;

			F = (MOVE_BACKWARD_3D_SPEED * Duration * (Y - (Height - Dom))) / Dom;

			F /= Conv;

			Make_3D_party_move(F * yawSIN, -F * yawCOS);
			break;
		}
		/* Turn right and move backwards */
		case 11:
		{
			Dom = Width / 3;

			F = (TURN_3D_SPEED * Duration * (X - (Width - Dom))) / Dom;

			Rotate_3D(-F);

			Dom = (Height * 2) / 10;

			F = (MOVE_BACKWARD_3D_SPEED * Duration * (Y - (Height - Dom))) / Dom;

			F /= Conv;

			Make_3D_party_move(F * yawSIN, -F * yawCOS);
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_key_M3_ModInit
 * FUNCTION  : Initialize 3D map key move mode module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.10.95 22:25
 * LAST      : 04.10.95 22:25
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_key_M3_ModInit(void)
{
	/* Indicate move mode is on */
	Move_mode_flag = TRUE;

	/* Guarantee first movement */
	Key_move_3D();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_key_M3_ModExit
 * FUNCTION  : Exit 3D map key move mode module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.10.95 22:25
 * LAST      : 04.10.95 22:25
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_key_M3_ModExit(void)
{
	/* Indicate move mode is off */
	Move_mode_flag = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_key_M3_MainLoop
 * FUNCTION  : 3D map key movement main loop.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.10.95 22:25
 * LAST      : 04.10.95 22:25
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_key_M3_MainLoop(void)
{
	BOOLEAN Key_pressed;
	UNSHORT Old_map_nr;

	/* Get old map number */
	Old_map_nr = PARTY_DATA.Map_nr;

	/* Update input handling */
	Update_input();

	/* Move */
	Key_pressed = Key_move_3D();

	/* Was a key pressed ? */
	if (Key_pressed)
	{
		/* Yes -> Exit if no longer in the same map */
		if (PARTY_DATA.Map_nr != Old_map_nr)
			return;

		/* Display map */
		M3_MainLoop();
	}
	else
	{
		/* No -> Exit move mode */
		Pop_module();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Key_move_3D
 * FUNCTION  : Move in 3D key move mode.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.10.95 22:25
 * LAST      : 12.10.95 18:30
 * INPUTS    : None.
 * RESULT    : BOOLEAN : A key was pressed.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Key_move_3D(void)
{
	struct Lab_data *Lab_ptr;
	BOOLEAN Key_pressed = FALSE;
	SILONG F;
	UNLONG Modifiers;
	SISHORT Duration;
	SISHORT Conv;

	/* Get duration of last frame */
	Duration = (SISHORT) Update_duration;

	/* Calculate conversion factor */
	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	Conv = 1 << (10 - Lab_ptr->Square_size_cm_log);

	MEM_Free_pointer(Lab_data_handle);

	/* Get modifiers */
	Modifiers = SYSTEM_GetBLEVStatusLong();

	#if FALSE
	Modifiers &= ~BLEV_SHIFT;
	if (pressedkeytab[RAWSHIFTL] || pressedkeytab[RAWSHIFTR])
	{
			Modifiers |= BLEV_SHIFT;
	}
	#endif

	/* Left pressed ? */
	if (pressedkeytab[RAW_LEFT])
	{
		/* Yes -> ALT pressed ? */
		if (Modifiers & BLEV_ALT)
		{
			/* Yes -> Move left */
			F = 40 * Duration;

			/* (faster if SHIFT is pressed) */
			if (Modifiers & BLEV_CTRL)
				F = (F * SHIFT_3D_SPEED_FACTOR) / 100;

			F /= Conv;

			Make_3D_party_move(-F * yawCOS, -F * yawSIN);

			Key_pressed = TRUE;
		}
		else
		{
			/* No -> Turn left */
			F = 15 * Duration;

			/* (faster if SHIFT is pressed) */
			if (Modifiers & BLEV_CTRL)
				F = (F * SHIFT_3D_SPEED_FACTOR) / 100;

			Rotate_3D(F);

			Key_pressed = TRUE;
		}
	}
	else
	{
		/* No -> Right pressed ? */
		if (pressedkeytab[RAW_RIGHT])
		{
			/* Yes -> ALT pressed ? */
			if (Modifiers & BLEV_ALT)
			{
				/* Yes -> Move right */
				F = 40 * Duration;

				/* (faster if SHIFT is pressed) */
				if (Modifiers & BLEV_CTRL)
					F = (F * SHIFT_3D_SPEED_FACTOR) / 100;

				F /= Conv;

				Make_3D_party_move(F * yawCOS, F * yawSIN);

				Key_pressed = TRUE;
			}
			else
			{
				/* No -> Turn right */
				F = 15 * Duration;

				/* (faster if SHIFT is pressed) */
				if (Modifiers & BLEV_CTRL)
					F = (F * SHIFT_3D_SPEED_FACTOR) / 100;

				Rotate_3D(-F);

				Key_pressed = TRUE;
			}
		}
	}

	/* Up pressed ? */
	if (pressedkeytab[RAW_UP])
	{
		/* Yes -> Move forward */
		F = 100 * Duration;

		/* (faster if SHIFT is pressed) */
		if (Modifiers & BLEV_CTRL)
			F = (F * SHIFT_3D_SPEED_FACTOR) / 100;

		F /= Conv;

		Make_3D_party_move(-F * yawSIN, F * yawCOS);

		Key_pressed = TRUE;
	}
	else
	{
		/* No -> Down pressed ? */
		if (pressedkeytab[RAW_DOWN])
		{
			/* Yes -> Move backward */
			F = 60 * Duration;

			/* (faster if SHIFT is pressed) */
			if (Modifiers & BLEV_CTRL)
				F = (F * SHIFT_3D_SPEED_FACTOR) / 100;

			F /= Conv;

			Make_3D_party_move(F * yawSIN, -F * yawCOS);

			Key_pressed = TRUE;
		}
	}

	return Key_pressed;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_3D_party_move
 * FUNCTION  : The party makes a move in a 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 14:23
 * LAST      : 23.07.95 16:31
 * INPUTS    : SILONG dX - X-component of movement vector.
 *             SILONG dY - Y-component of movement vector.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The movement vector is multiplied by 65536.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_3D_party_move(SILONG dX, SILONG dY)
{
	static struct Movement_3D Party_3D_move_data = {
		0, 0,
		0, 0,
		0,
		0xFFFF
	};

	BOOLEAN Result;
	SISHORT Map_X, Map_Y;

	/* Clear flags */
	Moved = FALSE;

	/* Allowed by before-move logic ? */
	if (Before_move())
	{
		/* Yes -> Try to move */
		Party_3D_move_data.X					= * ((UNLONG *) &(I3DM.Player_X[1]));
		Party_3D_move_data.Y					= * ((UNLONG *) &(I3DM.Player_Z[1]));
		/* (I don't use shifts because dX and dY are signed.) */
		Party_3D_move_data.dX				= dX / 65536;
		Party_3D_move_data.dY				= dY / 65536;
		Party_3D_move_data.Travel_mode	= PARTY_DATA.Travel_mode;

		Result = Make_3D_move(&Party_3D_move_data);

		/* Did it work ? */
		if (Result)
		{
			/* Yes -> Set flag */
			Moved = TRUE;

			/* Move */
			if (!Party_3D_move_data.dX)
				dX = 0;
			if (!Party_3D_move_data.dY)
				dY = 0;
			Add_3D_vector(dX, dY);

			/* Calculate new map coordinates */
			_3D_to_map(*((UNLONG *) &(I3DM.Player_X[1])),
			 *((UNLONG *) &(I3DM.Player_Z[1])), &Map_X, &Map_Y);

			/* Any change ? */
			if ((PARTY_DATA.X != Map_X) || (PARTY_DATA.Y != Map_Y))
			{
				/* Yes -> Store new coordinates */
				PARTY_DATA.X = Map_X;
				PARTY_DATA.Y = Map_Y;

				/* Update */
				Update_automap(MAX_UPDATE_DEPTH);

				After_move();
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_3D_move
 * FUNCTION  : Make a move in a 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 14:23
 * LAST      : 23.07.95 16:11
 * INPUTS    : struct Movement_3D *Move - Pointer to movement structure.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Make_3D_move(struct Movement_3D *Move)
{
	BOOLEAN Result;
	SILONG dX, dY;

	/* Get movement vector */
	dX = Move->dX;
	dY = Move->dY;

	/* Try to move */
	Result = Try_3D_move
	(
		Move->X + dX,
		Move->Y + dY,
		Move->NPC_index,
		Move->Travel_mode
	);

	/* Did it work ? */
	if (!Result)
	{
		/* No -> Slide (first choice) */
		if (Move->dX > Move->dY)
		{
			dY = 0;

			Result = Try_3D_move
			(
				Move->X + dX,
				Move->Y + dY,
				Move->NPC_index,
				Move->Travel_mode
			);
		}
		else
		{
			dX = 0;

			Result = Try_3D_move
			(
				Move->X + dX,
				Move->Y + dY,
				Move->NPC_index,
				Move->Travel_mode
			);
		}

		/* Did it work ? */
		if (!Result)
		{
			/* No -> Slide (second choice) */
			if (Move->dX > Move->dY)
			{
				dX = 0;
				dY = Move->dY;

				Result = Try_3D_move
				(
					Move->X + dX,
					Move->Y + dY,
					Move->NPC_index,
					Move->Travel_mode
				);
			}
			else
			{
				dX = Move->dX;
				dY = 0;

				Result = Try_3D_move
				(
					Move->X + dX,
					Move->Y + dY,
					Move->NPC_index,
					Move->Travel_mode
				);
			}
		}
	}

	/* Did it work ? */
	if (Result)
	{
		/* Yes -> Update coordinates */
		Move->X += dX;
		Move->Y += dY;

		/* Store actual movement vector */
		Move->dX = dX;
		Move->dY = dY;
	}
	else
	{
		/* No -> Clear movement vector */
		Move->dX = 0;
		Move->dY = 0;
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_3D_move
 * FUNCTION  : Check if a certain move can be made in a 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 14:26
 * LAST      : 04.10.95 14:03
 * INPUTS    : SILONG X - New X-coordinate (dungeon coordinates).
 *             SILONG Y - New Y-coordinate (dungeon coordinates).
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 *             UNSHORT Travel_mode - Travel mode.
 * RESULT    : BOOLEAN : TRUE (success) or FALSE (failure).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Try_3D_move(SILONG X, SILONG Y, UNSHORT NPC_index, UNSHORT Travel_mode)
{
	static struct Collision_3D Collision_table[8] = {
		{ -1,-1, 0x0001 },
		{  0,-1, 0x0007 },
		{  1,-1, 0x0004 },
		{ -1, 0, 0x0049 },
		{  1, 0, 0x0124 },
		{ -1, 1, 0x0040 },
		{  0, 1, 0x01C0 },
		{  1, 1, 0x0100 }
	};

	struct Object_group_3D *Object_group_base;
	struct Object_group_3D *Object_group_ptr;
	struct Object_3D *Object_base;
	struct Square_3D *Square;
	struct Lab_data *Lab_ptr;
	BOOLEAN Result;
	SILONG Group_X, Group_Z;
	SISHORT Map_X, Map_Y;
	UNSHORT f;
	UNSHORT Edge;
	UNSHORT c;
	UNSHORT i;
	UNSHORT Q;
	UNSHORT Bitlist;
	UNSHORT Square_X, Square_Z;
	UNBYTE *Ptr;

	/* Get square size in cm */
	f = I3DM.Square_size;

	/* Calculate map coordinates */
	_3D_to_map(X, Y, &Map_X, &Map_Y);

	/* Exit if target coordinates lie outside the map */
	if ((Map_X < 1) ||
	 (Map_X > Map_width) ||
	 (Map_Y < 1) ||
	 (Map_Y > Map_height))
		return FALSE;

	/* Exit if cheat mode is on / not NPC */
	if ((NPC_index == 0xFFFF) && Cheat_mode)
		return TRUE;

	/* Calculate coordinates inside current map square */
	Square_X = X & (f - 1);
	Square_Z = Y & (f - 1);

	/* Check the destination map square */
	Result = Movement_check_3D
	(
		Map_X,
		Map_Y,
		NPC_index,
		Travel_mode
	);

	/* Collision ? */
	if (Result)
	{
		/* No -> Try edges */
		/* Build edge collision list */
		Bitlist = 0;
		for (i=0;i<8;i++)
		{
			if (!Movement_check_3D
			(
				Map_X + Collision_table[i].dX,
				Map_Y - Collision_table[i].dY,
				NPC_index,
				Travel_mode
			))
			{
				Bitlist |= Collision_table[i].Bit_mask;
			}
		}

		/* Determine edge width */
//		Edge = f / 4;
		Edge = max((f / 4), 50);

		/* Determine current edge area */
		Q = 0;
		if (Square_X >= Edge)
		{
			if (Square_X < f - Edge)
				Q = 1;
			else
				Q = 2;
		}
		if (Square_Z >= Edge)
		{
			if (Square_Z < f - Edge)
				Q += 3;
			else
				Q += 6;
		}

		/* Blocked ? */
		if (Bitlist & (1 << Q))
		{
			/* Yes -> Collision */
			Result = FALSE;
		}
		else
		{
			/* No -> Get pointers to various parts of the lab-data */
			Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

			Ptr = (UNBYTE *) (Lab_ptr + 1);

			Object_group_base = (struct Object_group_3D *) Ptr;

			Ptr += (Nr_of_object_groups * sizeof(struct Object_group_3D)) + 2;
			Ptr += (Nr_of_floors * sizeof(struct Floor_3D)) + 2;

			Object_base = (struct Object_3D *) Ptr;

			/* Get target square data */
			Square = (struct Square_3D *) (MEM_Claim_pointer(Map_handle)
			 + Map_layers_offset) + ((Map_Y - 1) * Map_width) + Map_X - 1;

			/* Get entry from wall layer */
			c = (UNSHORT) Square->Wall_layer;

			/* Object group ? */
			if (c && (c < FIRST_WALL))
			{
				/* Yes -> Collision ? */
				Object_group_ptr = &(Object_group_base[c-1]);
				if (Check_3D_object_group_collision
				(
					Square_X,
					Square_Z,
					0,
					0,
					Travel_mode,
					Object_group_ptr,
					Object_base
				))
				{
					/* Yes -> Collision */
					Result = FALSE;
				}
			}

			MEM_Free_pointer(Map_handle);

			/* Collision ? */
			if (Result)
			{
				/* No -> Is NPC ? */
				if (NPC_index != 0xFFFF)
				{
					/* Yes -> Check for collision with party */
					/* Determine distance from party to NPC */
					Group_X = *((UNLONG *) &(I3DM.Player_X[1])) - X;
					Group_Z = *((UNLONG *) &(I3DM.Player_Z[1])) - Y;

					/* Too large ? */
					if ((Group_X > 0 - f) && (Group_X < f) && (Group_Z > 0 - f)
					 && (Group_Z < f))
					{
						/* No -> Check for collision */
						Object_group_ptr =
						 &(Object_group_base[VNPCs[NPC_index].Graphics_nr - 1]);

						if (Check_3D_object_group_collision
						(
							Square_X,
							Square_Z,
							(SISHORT) Group_X,
							(SISHORT) Group_Z,
							Travel_mode,
							Object_group_ptr,
							Object_base
						))
						{
							/* Collision */
							Result = FALSE;
						}
					}
				}

				/* Collision ? */
				if (Result)
				{
					/* No -> Check for collision with NPCs */
					for (i=0;i<NPCS_PER_MAP;i++)
					{
						/* Anyone there / not checking myself ? */
						if ((NPC_present(i)) && (i != NPC_index))
						{
							/* Yes -> Determine distance to NPC */
							Group_X = VNPCs[i].Display_X - X;
							Group_Z = VNPCs[i].Display_Y - Y;

							/* Too large ? */
							if ((Group_X > 0 - f) && (Group_X < f) &&
							 (Group_Z > 0 - f) && (Group_Z < f))
							{
								/* No -> Check for collision */
								Object_group_ptr =
								 &(Object_group_base[VNPCs[i].Graphics_nr - 1]);

								if (Check_3D_object_group_collision(Square_X,
								 Square_Z, (SISHORT) Group_X, (SISHORT) Group_Z,
								 Travel_mode, Object_group_ptr, Object_base))
								{
									/* Collision */
									Result = FALSE;
									break;
								}
							}
						}
					}
				}
			}

			MEM_Free_pointer(Lab_data_handle);
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Movement_check_3D
 * FUNCTION  : Check if a certain position in a 3D map can be moved to.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 15:32
 * LAST      : 23.07.95 14:13
 * INPUTS    : SISHORT Map_X - Map X-coordinate.
 *             SISHORT Map_Y - Map Y-coordinate.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 *             UNSHORT Travel_mode - Travel mode.
 * RESULT    : BOOLEAN : TRUE (not blocked) or FALSE (blocked).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Movement_check_3D(SISHORT Map_X, SISHORT Map_Y, UNSHORT NPC_index,
 UNSHORT Travel_mode)
{
	struct Square_3D *Square;
	struct Lab_data *Lab_ptr;
	struct Floor_3D *Floor_ptr;
	BOOLEAN Result = TRUE;
	UNLONG Status;
	UNSHORT c;
	UNBYTE *Ptr;

	/* Exit if target coordinates lie outside the map */
	if ((Map_X < 1) || (Map_X > Map_width) || (Map_Y < 1) ||
	 (Map_Y > Map_height))
		return FALSE;

	/* Get pointers to various parts of the lab-data */
	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);
	Ptr = (UNBYTE *) (Lab_ptr + 1);
	Ptr += (Nr_of_object_groups * sizeof(struct Object_group_3D)) + 2;
	Floor_ptr = (struct Floor_3D *) Ptr;

	/* Get target square data */
	Square = (struct Square_3D *) (MEM_Claim_pointer(Map_handle) +
	 Map_layers_offset) + ((Map_Y - 1) * Map_width) + Map_X - 1;

	/* Try wall */
	c = (UNSHORT) Square->Wall_layer;

	/* Any wall ? */
	if (c >= FIRST_WALL)
	{
		/* Yes -> Get wall status bits */
		Status = ((struct Wall_3D *) ((UNLONG) Lab_ptr +
		 Wall_offsets[c-FIRST_WALL]))->Flags;

		/* Check bits for current travelmode */
		/* Blocked ? */
		if (Status & (1 << (BLOCKED_TRAVELMODES_B + Travel_mode)))
		{
			/* Yes */
			Result = FALSE;
		}
	}

	/* Blocked ? */
	if (Result)
	{
		/* No -> Try floor */
		c = Square->Floor_layer;

		/* Any floor ? */
		if (c)
		{
			/* Yes -> Get floor status bits */
			Status = Floor_ptr[c-1].Flags;

			/* Check bits for current travelmode */
			/* Blocked ? */
			if (Status & (1 << (BLOCKED_TRAVELMODES_B + Travel_mode)))
			{
				/* Yes */
				Result = FALSE;
			}
		}
	}

	/* Blocked ? */
	if (Result)
	{
		/* No -> Try ceiling */
		c = Square->Ceiling_layer;

		/* Any ceiling ? */
		if (c)
		{
			/* Yes -> Get ceiling status bits */
			Status = Floor_ptr[c-1].Flags;

			/* Check bits for current travelmode */
			/* Blocked ? */
			if (Status & (1 << (BLOCKED_TRAVELMODES_B + Travel_mode)))
			{
				/* Yes */
				Result = FALSE;
			}
		}
	}

	MEM_Free_pointer(Map_handle);
	MEM_Free_pointer(Lab_data_handle);

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_3D_object_group_collision
 * FUNCTION  : Check if a certain position in a 3D map causes collision with
 *              an object group.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 10:03
 * LAST      : 29.08.95 12:07
 * INPUTS    : UNSHORT Square_X - X-coordinate inside map square.
 *             UNSHORT Square_Z - Z-coordinate inside map square.
 *             SISHORT Group_X - X-offset of object group.
 *             SISHORT Group_Z - Z-offset of object group.
 *             UNSHORT Travel_mode - Travel mode.
 *             struct Object_group_3D *Object_group - Pointer to object group.
 *             struct Object_3D *Object_base - Pointer to start of object data.
 * RESULT    : BOOLEAN : TRUE (collision) or FALSE (no collision).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_3D_object_group_collision(SISHORT Square_X, SISHORT Square_Z,
 SISHORT Group_X, SISHORT Group_Z, UNSHORT Travel_mode,
 struct Object_group_3D *Object_group, struct Object_3D *Object_base)
{
	struct Object_3D *Object_ptr;
	BOOLEAN Result = FALSE;
	SISHORT Object_X, Object_Z;
	UNSHORT Object_nr, W;
	UNSHORT i;

	/* Check all objects */
	for (i=0;i<OBJECTS_PER_GROUP;i++)
	{
		/* Get object index */
		Object_nr = Object_group->Group[i].Number;

		/* Anything there ? */
		if (Object_nr)
		{
			/* Yes -> Get pointer to data */
			Object_ptr = &(Object_base[Object_nr - 1]);

			/* Blocking ? */
			if (Object_ptr->Flags &
			 (1 << (BLOCKED_TRAVELMODES_B + Travel_mode)))
			{
				/* Yes -> Get object coordinates */
				Object_X = Object_group->Group[i].X + Group_X;
				Object_Z = Object_group->Group[i].Z + Group_Z;

				/* Get object width */
				W = Object_ptr->Object_width / 2;

				/* Target coordinates inside object ? */
				if ((Square_X >= Object_X - W) && (Square_Z >= Object_Z - W) &&
				 (Square_X < Object_X + W) && (Square_Z < Object_Z + W))
				{
					/* Yes -> Collision */
					Result = TRUE;
					break;
				}
			}
		}
	}

	return (Result);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Map_to_3D
 * FUNCTION  : Convert map to 3D coordinates.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.05.95 11:59
 * LAST      : 11.07.95 15:26
 * INPUTS    : SISHORT Map_X - Map X-coordinate.
 *             SISHORT Map_Y - Map Y-coordinate.
 *             UNLONG *X_3D_ptr - Pointer to 3D X-coordinate.
 *             UNLONG *Y_3D_ptr - Pointer to 3D Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Map_to_3D(SISHORT Map_X, SISHORT Map_Y, UNLONG *X_3D_ptr,
 UNLONG *Y_3D_ptr)
{
	UNLONG f;

	/* Get square size in cm */
	f = I3DM.Square_size;

	/* Convert map to 3D coordinates */
	*X_3D_ptr = ((UNLONG) Map_X - 1) * f + (f / 2);
	*Y_3D_ptr = (UNLONG)(Map_height - Map_Y) * f + (f / 2);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : _3D_to_map
 * FUNCTION  : Convert 3D to map coordinates.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.05.95 12:01
 * LAST      : 11.07.95 15:26
 * INPUTS    : UNLONG X_3D - 3D X-coordinate.
 *             UNLONG Y_3D - 3D Y-coordinate.
 *             SISHORT *Map_X_ptr - Pointer to map X-coordinate.
 *             SISHORT *Map_Y_ptr - Pointer to map Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
_3D_to_map(UNLONG X_3D, UNLONG Y_3D, SISHORT *Map_X_ptr,
 SISHORT *Map_Y_ptr)
{
	UNLONG f;

	/* Get square size in cm */
	f = I3DM.Square_size;

	/* Convert 3D to map coordinates */
	*Map_X_ptr = (SISHORT) ((X_3D / f) + 1);
	*Map_Y_ptr = (SISHORT) (Map_height - (Y_3D / f));
}

