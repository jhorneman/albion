/************
 * NAME     : TACTICAL.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 25-1-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : TACTICAL.H
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <GAMETEXT.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <COMBAT.H>
#include <COMACTS.H>
#include <TACTICAL.H>
#include <EVELOGIC.H>
#include <POPUP.H>
#include <STATAREA.H>
#include <MAGIC.H>
#include <INPUT.H>

/* global variables */

UNSHORT Tactical_draw_mode = TACTICAL_NORMAL_DRAWMODE;
UNSHORT Tactical_draw_index;

Tactical_display_handler Current_tactical_display_handler = NULL;

BOOLEAN Tactical_select_mode = FALSE;
UNLONG Tactical_select_mask;
UNLONG Tactical_select_mask2;
UNSHORT Selected_tactical_square_index;

MEM_HANDLE Tactical_window_background_handle;
struct OPM Tactical_window_background_OPM;

static BOOLEAN Draw_tactical_window_flag = TRUE;

/* Tactical window select module */
static struct Module Tactical_select_Mod = {
	LOCAL_MOD, MODE_MOD, NO_SCREEN,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

/* Tactical window method list */
static struct Method Tactical_window_methods[] = {
	{ INIT_METHOD, Init_Tactical_window_object },
	{ DRAW_METHOD, Draw_Tactical_window_object },
	{ UPDATE_METHOD, Update_Tactical_window_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ CUSTOMKEY_METHOD, Customkeys_Tactical_window_object },
	{ 0, NULL }
};

/* Tactical window class description */
struct Object_class Tactical_window_Class = {
	0, sizeof(struct Object),
	&Tactical_window_methods[0]
};

/* Tactical square method list */
static struct Method Tactical_square_methods[] = {
	{ INIT_METHOD, Init_Tactical_square_object },
	{ DRAW_METHOD, Draw_Tactical_square_object },
	{ FEEDBACK_METHOD, Feedback_Tactical_square_object },
	{ HIGHLIGHT_METHOD, Highlight_Tactical_square_object },
	{ LEFT_METHOD, Left_Tactical_square_object },
	{ RIGHT_METHOD, Right_Tactical_square_object },
	{ TOUCHED_METHOD, Touch_Tactical_square_object },
	{ HELP_METHOD, Help_Tactical_square_object },
	{ POP_UP_METHOD, Pop_up_Tactical_square_object },
	{ 0, NULL}
};

/* Tactical square class description */
static struct Object_class Tactical_square_Class = {
	0, sizeof(struct Tactical_square_object),
	&Tactical_square_methods[0]
};

/* Tactical square pop-up menu */
static struct PUME Tactical_square_PUMEs[] = {
	{PUME_AUTO_CLOSE, 0, 1, PUM_Attack_tactical_square},
	{PUME_AUTO_CLOSE, 0, 2, PUM_Move_tactical_square},
	{PUME_AUTO_CLOSE, 0, 3, PUM_Cast_spell_tactical_square},
	{PUME_AUTO_CLOSE, 0, 199, PUM_Use_magic_item_tactical_square},
	{PUME_AUTO_CLOSE, 0, 198, PUM_Flee_tactical_square},
	{PUME_NOT_SELECTABLE, 0, 0, NULL},
	{PUME_AUTO_CLOSE, 0, 200, PUM_Advance_tactical_square},
	{PUME_NOT_SELECTABLE, 0, 0, NULL},
	{PUME_AUTO_CLOSE, 0, 202, PUM_Start_round},
};

struct PUM Tactical_square_PUM = {
	9,
	NULL,
	NULL,
	Tactical_square_PUM_evaluator,
	Tactical_square_PUMEs
};

/* Tactical combat matrix */
struct Tactical_square Combat_matrix[NR_TACTICAL_ROWS][NR_TACTICAL_COLUMNS];

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Tactical_window_object
 * FUNCTION  : Initialize method of Tactical window object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 11:59
 * LAST      : 14.03.95 11:59
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_Tactical_window_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_OID Square_OID;
	UNSHORT i, j;

	/* Get background */
	Put_recoloured_box(&Main_OPM, Object->X, Object->Y + 24,
	 Object->Rect.width, Object->Rect.height - 24, &(Recolour_tables[2][0]));

	Get_tactical_window_background();

	/* Do each row of squares */
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		/* Set object initialization data */
		Square_OID.Y = i;

		/* Do each square in this row */
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Set object initialization data */
			Square_OID.X = j;

			/* Add tactical square object */
			Add_object(Object->Self, &Tactical_square_Class, (UNBYTE *) &Square_OID,
			 (j * TACTICAL_SQUARE_WIDTH) + 8, (i * TACTICAL_SQUARE_HEIGHT) + 25,
			 TACTICAL_SQUARE_WIDTH, TACTICAL_SQUARE_HEIGHT);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Tactical_window_object
 * FUNCTION  : Draw method of Tactical window object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 13:07
 * LAST      : 14.03.95 13:08
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_Tactical_window_object(struct Object *Object, union Method_parms *P)
{
	if (!Fighting)
	{
		Draw_tactical_window();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Tactical_window_object
 * FUNCTION  : Update method of Tactical window object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 14:45
 * LAST      : 12.04.95 14:45
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_Tactical_window_object(struct Object *Object, union Method_parms *P)
{
	if (Current_tactical_display_handler && !Fighting)
	{
		Draw_tactical_window();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Customkeys_Tactical_window_object
 * FUNCTION  : Customkeys method of Tactical window object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 13:07
 * LAST      : 14.03.95 13:08
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Customkeys_Tactical_window_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT Key_code;
	UNSHORT i;

	/* Get key-code */
	Key_code = (UNSHORT) (P->Event->sl_key_code);

	switch (Key_code)
	{
		case BLEV_UP:
			if (Combat_camera_height < 1000)
				Combat_camera_height += 1;
			break;
		case BLEV_DOWN:
			if (Combat_camera_height > 0)
				Combat_camera_height -= 1;
			break;
		case BLEV_LEFT:
			if (Combat_projection_factor < 32768)
				Combat_projection_factor += 8;
			break;
		case BLEV_RIGHT:
			if (Combat_projection_factor > 16)
				Combat_projection_factor -= 8;
			break;
		case 'q':
			if (Combat_Z_offset < 1024)
				Combat_Z_offset += 8;
			break;
		case 'a':
			if (Combat_Z_offset > (0 - 4096))
				Combat_Z_offset -= 8;
			break;

		#if FALSE
		case 'w':
			if (Combat_square_width < 100 * COMOB_DEC_FACTOR)
			{
				Combat_square_width += COMOB_DEC_FACTOR;

				for (i=0;i<Nr_monsters;i++)
				{
					Convert_tactical_to_3D_coordinates(Monster_parts[i].Tactical_X,
					 Monster_parts[i].Tactical_Y, &(Monster_parts[i].Main_COMOB->X_3D),
					  &(Monster_parts[i].Main_COMOB->Z_3D));
					Monster_parts[i].Main_COMOB->Y_3D = 0;

					Monster_parts[i].Shadow_COMOB->X_3D = Monster_parts[i].Main_COMOB->X_3D;
					Monster_parts[i].Shadow_COMOB->Z_3D = Monster_parts[i].Main_COMOB->Z_3D;
					Monster_parts[i].Shadow_COMOB->Y_3D = 0;
				}
			}
			break;
		case 's':
			if (Combat_square_width > 10 * COMOB_DEC_FACTOR)
			{
				Combat_square_width -= COMOB_DEC_FACTOR;

				for (i=0;i<Nr_monsters;i++)
				{
					Convert_tactical_to_3D_coordinates(Monster_parts[i].Tactical_X,
					 Monster_parts[i].Tactical_Y, &(Monster_parts[i].Main_COMOB->X_3D),
					  &(Monster_parts[i].Main_COMOB->Z_3D));
					Monster_parts[i].Main_COMOB->Y_3D = 0;

					Monster_parts[i].Shadow_COMOB->X_3D = Monster_parts[i].Main_COMOB->X_3D;
					Monster_parts[i].Shadow_COMOB->Z_3D = Monster_parts[i].Main_COMOB->Z_3D;
					Monster_parts[i].Shadow_COMOB->Y_3D = 0;
				}
			}
			break;
		case 'e':
			if (Combat_square_depth < 100 * COMOB_DEC_FACTOR)
			{
				Combat_square_depth += COMOB_DEC_FACTOR;

				for (i=0;i<Nr_monsters;i++)
				{
					Convert_tactical_to_3D_coordinates(Monster_parts[i].Tactical_X,
					 Monster_parts[i].Tactical_Y, &(Monster_parts[i].Main_COMOB->X_3D),
					  &(Monster_parts[i].Main_COMOB->Z_3D));
					Monster_parts[i].Main_COMOB->Y_3D = 0;

					Monster_parts[i].Shadow_COMOB->X_3D = Monster_parts[i].Main_COMOB->X_3D;
					Monster_parts[i].Shadow_COMOB->Z_3D = Monster_parts[i].Main_COMOB->Z_3D;
					Monster_parts[i].Shadow_COMOB->Y_3D = 0;
				}
			}
			break;
		case 'd':
			if (Combat_square_depth > 10 * COMOB_DEC_FACTOR)
			{
				Combat_square_depth -= COMOB_DEC_FACTOR;

				for (i=0;i<Nr_monsters;i++)
				{
					Convert_tactical_to_3D_coordinates(Monster_parts[i].Tactical_X,
					 Monster_parts[i].Tactical_Y, &(Monster_parts[i].Main_COMOB->X_3D),
					  &(Monster_parts[i].Main_COMOB->Z_3D));
					Monster_parts[i].Main_COMOB->Y_3D = 0;

					Monster_parts[i].Shadow_COMOB->X_3D = Monster_parts[i].Main_COMOB->X_3D;
					Monster_parts[i].Shadow_COMOB->Z_3D = Monster_parts[i].Main_COMOB->Z_3D;
					Monster_parts[i].Shadow_COMOB->Y_3D = 0;
				}
			}
			break;
		#endif

		case 't':
			if (Combat_grid_flag)
			{
				Draw_tactical_window_flag = TRUE;
				Combat_grid_flag = FALSE;
			}
			else
			{
				Draw_tactical_window_flag = FALSE;
				Combat_grid_flag = TRUE;
			}

			Draw_combat_screen();
			Get_tactical_window_background();
			Update_screen();

			break;
		case '1':
			Do_explosion(0, 100 * COMOB_DEC_FACTOR, 180, 180);
			break;
		case '2':
		{
			UNSHORT Select;
			UNSHORT j;

			Select = rand() % Nr_monsters;

			for (i=0;i<3;i++)
			{
				for (j=0;j<NR_TACTICAL_COLUMNS;j++)
				{
					if (Combat_matrix[i][j].Part)
					{
						if (Select)
							Select--;
						else
						{
							Do_fireball(Combat_matrix[i][j].Part);
							return;
						}
					}
				}
			}
			break;
		}
		case '3':
			End_combat_flag = TRUE;
			break;
		case '4':
			Fighting = TRUE;
			for (i=0;i<Nr_monsters;i++)
			{
				Kill_monster(&Monster_parts[i]);
			}
			Fighting = FALSE;

			break;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Tactical_square_object
 * FUNCTION  : Initialize method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 13:10
 * LAST      : 14.03.95 13:10
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_object *Square;
	struct Tactical_square_OID *OID;

	Square = (struct Tactical_square_object *) Object;
	OID = (struct Tactical_square_OID *) P;

	/* Copy data from OID */
	Square->X = OID->X;
	Square->Y = OID->Y;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Tactical_square_object
 * FUNCTION  : Draw method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 13:14
 * LAST      : 14.03.95 13:14
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	if (!Fighting)
	{
		Tactical_draw_mode = TACTICAL_NORMAL_DRAWMODE;

		Draw_tactical_window();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Tactical_square_object
 * FUNCTION  : Feedback method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 17:45
 * LAST      : 14.03.95 17:45
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Feedback_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_object *Square;

	Square = (struct Tactical_square_object *) Object;

	if (!Fighting)
	{
		Tactical_draw_mode = TACTICAL_FEEDBACK_DRAWMODE;
		Tactical_draw_index = (Square->Y * NR_TACTICAL_COLUMNS) + Square->X;

		Draw_tactical_window();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Tactical_square_object
 * FUNCTION  : Highlight method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 17:44
 * LAST      : 14.03.95 17:44
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Highlight_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_object *Square;

	Square = (struct Tactical_square_object *) Object;

	if (!Fighting)
	{
		Tactical_draw_mode = TACTICAL_HIGHLIGHT_DRAWMODE;
		Tactical_draw_index = (Square->Y * NR_TACTICAL_COLUMNS) + Square->X;

		Draw_tactical_window();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Tactical_square_object
 * FUNCTION  : Left method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Left_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_object *Square;

	Square = (struct Tactical_square_object *) Object;
	/* Select mode ? */
	if (Tactical_select_mode)
	{
		/* Yes -> Can this square be selected ? */
		if (Tactical_select_mask & (1 << (Square->Y * NR_TACTICAL_COLUMNS +
		 Square->X)))
		 {
			/* Yes -> Really clicked ? */
			if (Normal_clicked(Object))
			{
				/* Yes -> Select */
				Selected_tactical_square_index = Square->Y * NR_TACTICAL_COLUMNS +
		 		 Square->X;

				Pop_module();
			}
		 }
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Right_Tactical_square_object
 * FUNCTION  : Right method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 13:15
 * LAST      : 14.03.95 13:15
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Right_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_object *Square;
	struct Combat_participant *Part;

	Square = (struct Tactical_square_object *) Object;

	/* Select mode ? */
	if (Tactical_select_mode)
	{
		/* Yes -> Abort selection */
		Pop_module();
	}
	else
	{
		/* No -> Anything in this square ? */
		Part = Combat_matrix[Square->Y][Square->X].Part;
		if (Part)
		{
			/* Yes -> A party member ? */
			if (Part->Type == PARTY_PART_TYPE)
			{
				/* Yes -> Do normal action */
				Normal_rightclicked(Object, P);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Touch_Tactical_square_object
 * FUNCTION  : Touch method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 16:57
 * LAST      : 13.03.95 16:57
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Touch_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_object *Square;
	struct Combat_participant *Part;

	Square = (struct Tactical_square_object *) Object;

	/* Select mode ? */
	if (Tactical_select_mode)
	{
		/* Yes -> Can this square be selected ? */
		if (Tactical_select_mask & (1 << (Square->Y * NR_TACTICAL_COLUMNS +
		 Square->X)))
		{
			/* Yes -> Still the same object ? */
			if (Object->Self != Highlighted_object)
			{
				/* No -> Redraw previously highlighted object (if any) */
				if (Highlighted_object)
				{
					Execute_method(Highlighted_object, DRAW_METHOD, NULL);
				}

				/* Store new highlighted object handle */
				Highlighted_object = Object->Self;

				/* Highlight new object */
				Execute_method(Object->Self, HIGHLIGHT_METHOD, NULL);

				/* Does this object have a help method ? */
				if (Has_method(Object->Self, HELP_METHOD))
				{
					/* Yes -> Execute help method */
					Execute_method(Object->Self, HELP_METHOD, NULL);
				}
				else
				{
					/* No -> Remove help message */
					Execute_method(Help_line_object, SET_METHOD, NULL);
				}

				/* Normal mouse pointer */
				Change_mouse(&(Mouse_pointers[DEFAULT_MPTR]));
			}
		}
		else
		{
			/* No -> Dehighlight */
			Dehighlight(Object, NULL);
		}
	}
	else
	{
		/* No -> Anything in this square ? */
		Part = Combat_matrix[Square->Y][Square->X].Part;
		if (Part)
		{
			/* Yes -> A party member ? */
			if (Part->Type == PARTY_PART_TYPE)
			{
				/* Yes -> Touch normally */
				Normal_touched(Object, NULL);
			}
			else
			{
				/* No -> Dehighlight */
				Dehighlight(Object, NULL);
			}
		}
		else
		{
			/* No -> Dehighlight */
			Dehighlight(Object, NULL);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Tactical_square_object
 * FUNCTION  : Help method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Help_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_up_Tactical_square_object
 * FUNCTION  : Pop-up method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 16:46
 * LAST      : 13.03.95 16:46
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_up_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_object *Square;
	struct Combat_participant *Part;
	UNCHAR Char_name[CHAR_NAME_LENGTH];

	Square = (struct Tactical_square_object *) Object;

	/* Anything in this square ? */
	Part = Combat_matrix[Square->Y][Square->X].Part;
	if (Part)
	{
		/* Yes -> A party member ? */
		if (Part->Type == PARTY_PART_TYPE)
		{
			/* Yes -> Get participant name */
			Get_char_name(Part->Char_handle, Char_name);

			Tactical_square_PUM.Title = Char_name;
			Tactical_square_PUM.Data = (UNLONG) Part->Number;

			/* Call pop-up menu */
			PUM_source_object_handle = Object->Self;
			PUM_char_handle = Part->Char_handle;
			Do_PUM(Object->X + 32, Object->Y + 8, &Tactical_square_PUM);
		}
	}
}

/*
;*****************************************************************************
; [ Select active character in tactical window ]
; No registers are restored
;*****************************************************************************
Tactical_member_left:
	jsr	Get_tactic_index		; Get index
	tst.w	d0
	bmi	.Exit
	jsr	Wait_4_unclick		; Wait
	move.w	d0,d4
	lea.l	Combat_matrix,a0		; Look in matrix
	lsl.w	#2,d0
	add.w	d0,a0
	tst.l	(a0)			; Anything there ?
	bne	.Full
; ---------- Check if movement is possible --------
	cmp.w	#18,d4			; In bottom two rows ?
	bmi	.Done
	Get	Active_handle,a1		; Moving possible ?
	move.w	Body_conditions(a1),d0
	Free	Active_handle
	and.w	#Move_mask,d0
	beq.s	.Can_move
	move.w	#218,d0			; "Can't move"
	jsr	Do_prompt
	bra	.Done
.Can_move:	move.l	Active_participant,a0	; Get occupied targets
	jsr	Get_occupied_targets
	move.l	d0,d1
	btst	d4,d1			; Occupied ?
	beq.s	.Free
	move.w	#221,d0			; "Occupied"
	jsr	Do_prompt
	bra	.Done
.Free:	jsr	Check_movement_range	; Find possible targets
	not.l	d1			; Remove occupied targets
	and.l	d1,d0
	btst	d4,d0			; Can be reached ?
	beq.s	.Too_far
	move.w	d4,Part_target(a0)		; Yes -> Store
	move.b	#Move_action,Part_action(a0)
	move.w	#208,d0			; Feedback
	move.w	#160,d1
	moveq.l	#Combat_pos_cicon,d3
	jsr	Feedback
	bra	.Done
.Too_far:	move.w	#219,d0			; "Can't reach"
	jsr	Do_prompt
	bra	.Done
.Full:	move.l	(a0),a0			; Is party ?
	cmp.b	#1,Part_type(a0)
	beq	.Party
; ---------- Check if attack is possible ----------
	Get	Active_handle,a1		; Attacking possible ?
	move.w	Body_conditions(a1),d0
	move.w	Damage(a1),d1
	add.w	Damage_magic(a1),d1
	Free	Active_handle
	and.w	#Attack_mask,d0
	bne.s	.Cant
	tst.w	d1
	bne.s	.Can_attack
.Cant:	move.w	#220,d0			; "Can't attack"
	jsr	Do_prompt
	bra	.Done
; ---------- Check weapon -------------------------
.Can_attack:
	Get	Active_handle,a0
	lea.l	Char_inventory+Right_hand_slot(a0),a1
	move.w	Object_index(a1),d0		; Get item in right hand
	beq	.Close			; Nothing ? -> Close range
	lea.l	Object_data+4,a1		; Get object data address
	subq.w	#1,d0
	mulu.w	#Item_data_size,d0
	add.l	d0,a1
	cmp.b	#Longrange_itemtype,Item_type(a1)	; Long range ?
	bne	.Close
; ---------- Check long-range weapon --------------
	lea.l	Char_inventory+Left_hand_slot(a0),a2
	move.w	Object_index(a2),d0		; Get item in left hand
	Free	Active_handle
	move.b	Ammo_use_ID(a1),d1		; Get required ammo ID
	beq	.Do			; (if any!)
	tst.w	d0			; Anything ?
	beq.s	.No_ammo
	lea.l	Object_data+4,a2		; Get object data address
	subq.w	#1,d0
	mulu.w	#Item_data_size,d0
	add.l	d0,a2
	cmp.b	Ammo_ID(a2),d1		; Is correct ?
	beq	.Do
.No_ammo:	move.w	#204,d0			; No ammo !
	jsr	Do_prompt
	bra	.Done
.Do:	move.l	Active_participant,a0
	move.w	d4,Part_target(a0)		; Store
	move.b	#Long_range_action,Part_action(a0)
	move.w	#208,d0			; Feedback
	move.w	#177,d1
	moveq.l	#Attack_cicon,d3
	jsr	Feedback
	bra	.Done
; ---------- Determine close-range targets --------
.Close:	Free	Active_handle
	move.l	Active_participant,a0	; Get targets
	jsr	Get_closerange_targets
	btst	d4,d0			; Can be reached ?
	beq	.Too_far
	move.w	d4,Part_target(a0)		; Yes -> Store
	move.b	#Close_range_action,Part_action(a0)
	move.w	#208,d0			; Feedback
	move.w	#177,d1
	moveq.l	#Attack_cicon,d3
	jsr	Feedback
	bra	.Done
; ---------- Try to make active -------------------
.Party:	moveq.l	#0,d7			; Get member number
	move.b	Part_nr(a0),d7
	jsr	Activate_member		; Make active
.Done:	jsr	Tactic_touched		; Touch
.Exit:	rts

;*****************************************************************************
; [ Select active character & enter Inventory in tactical window ]
; No registers are restored
;*****************************************************************************
Tactical_member_right:
	jsr	Get_tactic_index		; Get index
	tst.w	d0
	bmi.s	.Exit
	lea.l	Combat_matrix,a0		; Look in matrix
	lsl.w	#2,d0
	add.w	d0,a0
	tst.l	(a0)			; Anything there ?
	beq.s	.Exit
	move.l	(a0),a0			; Is party ?
	cmp.b	#1,Part_type(a0)
	bne.s	.Done
	moveq.l	#0,d7			; Get member number
	move.b	Part_nr(a0),d7
	cmp.w	Active_member,d7		; Same ?
	beq.s	.Skip
	jsr	Activate_member		; No -> Make active
	bne.s	.Done
.Skip:	jsr	Destroy_ghosts		; Kill!
	moveq.l	#0,d0			; Enter Inventory
	move.w	d7,d0
	lsl.w	#8,d0
	jsr	Member_right
	bra.s	.Exit
.Done:	jsr	Tactic_touched		; Touch
.Exit:	rts
*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Tactical_square_PUM_evaluator
 * FUNCTION  : Evaluate tactical square pop-up menu.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : struct PUM *PUM - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Tactical_square_PUM_evaluator(struct PUM *PUM)
{
	struct PUME *PUMES;
	struct Combat_participant *Part;
	MEM_HANDLE Char_handle;
	BOOLEAN Front_rows_empty;
	UNSHORT Conditions;
	UNSHORT i, j;

	PUMES = PUM->PUME_list;
	Char_handle = Party_char_handles[(UNSHORT) PUM->Data - 1];

	/* Get body conditions */
	Conditions = Get_conditions(Char_handle);

	/* Controllable ? */
	if (Conditions & CONTROL_MASK)
	{
		/* No -> Disable all */
		PUMES[0].Flags |= PUME_ABSENT;
		PUMES[1].Flags |= PUME_ABSENT;
		PUMES[2].Flags |= PUME_ABSENT;
		PUMES[3].Flags |= PUME_ABSENT;
		PUMES[4].Flags |= PUME_ABSENT;
	}
	else
	{
		/* Yes -> Enable all */
		PUMES[0].Flags &= ~PUME_ABSENT;
		PUMES[1].Flags &= ~PUME_ABSENT;
		PUMES[2].Flags &= ~PUME_ABSENT;
		PUMES[3].Flags &= ~PUME_ABSENT;
		PUMES[4].Flags &= ~PUME_ABSENT;

		/* Can attack ? */
		if (Conditions & ATTACK_MASK)
		{
			/* No -> Block attack */
			PUMES[0].Flags |= PUME_BLOCKED;

			/* Explain */
			PUMES[0].Blocked_message_nr = 437;
		}
		else
		{
			/* Yes -> Can do damage ? */
			if (1) //Get_damage(Char_handle))
			{
				/* Yes -> Attack is possible */
				PUMES[0].Flags &= ~PUME_BLOCKED;
			}
			else
			{
				/* No -> Block attack */
				PUMES[0].Flags |= PUME_BLOCKED;

				/* Explain */
				PUMES[0].Blocked_message_nr = 438;
			}
		}

		/* Can move ? */
		if (Conditions & MOVE_MASK)
		{
			/* No -> Block move */
			PUMES[1].Flags |= PUME_BLOCKED;

			/* Explain */
			PUMES[1].Blocked_message_nr = 439;
		}
		else
		{
			/* Yes -> Move is possible */
			PUMES[1].Flags &= ~PUME_BLOCKED;
		}

		/* Does this character have magical abilities ? */
		if (Character_has_magical_abilities(Char_handle) &&
		 !(Conditions & MAGIC_MASK))
		{
			/* Yes -> Could use magic */
			PUMES[2].Flags &= ~PUME_ABSENT;

			/* Know any spells ? */
			if (Character_knows_spells(Char_handle))
			{
				/* Yes -> Can use magic */
				PUMES[2].Flags &= ~PUME_BLOCKED;
			}
			else
			{
				/* No -> Block use magic */
				PUMES[2].Flags |= PUME_BLOCKED;

				/* Explain */
				PUMES[2].Blocked_message_nr = 425;
			}
		}
		else
		{
			/* No -> Cannot use magic */
			PUMES[2].Flags |= PUME_ABSENT;
		}

		/* In the last row ? */
		if (Party_parts[(UNSHORT) PUM->Data - 1].Tactical_Y ==
		 NR_TACTICAL_ROWS - 1)
		{
			/* Yes -> Could flee */
			PUMES[4].Flags &= ~PUME_ABSENT;

			/* Can flee ? */
			if (Conditions & FLEE_MASK)
			{
				/* No -> Block flee */
				PUMES[4].Flags |= PUME_BLOCKED;

				/* Explain */
				PUMES[4].Blocked_message_nr = 440;
			}
			else
			{
				/* Yes -> Can flee */
				PUMES[4].Flags &= ~PUME_BLOCKED;
			}
		}
		else
		{
			/* No -> Cannot flee */
			PUMES[4].Flags |= PUME_ABSENT;
		}

		/* Are there any monsters in the front rows ? */
		Front_rows_empty = TRUE;
		for (i=2;i<=3;i++)
		{
			for(j=0;j<NR_TACTICAL_COLUMNS;j++)
			{
				/* Anything here in the matrix ? */
				if (Combat_matrix[i][j].Part)
				{
					/* Yes -> Get participant data */
					Part = Combat_matrix[i][j].Part;

					/* Is monster ? */
					if (Part->Type == MONSTER_PART_TYPE)
					{
						/* Yes -> Front rows are not empty */
						Front_rows_empty = FALSE;

						break;
					}
				}
			}
			if (Front_rows_empty)
				break;
		}

		/* Well ? */
		if (Front_rows_empty)
		{
		  	/* Yes -> Could advance */
			PUMES[6].Flags &= ~PUME_ABSENT;
		}
		else
		{
			/* No -> Cannot advance */
			PUMES[6].Flags |= PUME_ABSENT;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Attack_tactical_square
 * FUNCTION  : Attack (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 21:40
 * LAST      : 12.04.95 21:40
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Attack_tactical_square(UNLONG Data)
{
	struct Combat_participant *Attacker_part;
	struct Character_data *Char;
	struct Item_data *Right_hand_item, *Left_hand_item;
	UNLONG Mask;
	UNSHORT Action = NO_COMACT;
	UNSHORT Target_square_index = 0xFFFF;
	UNSHORT Weapon_item_slot_index;

	/* Get participant data */
	Attacker_part = &Party_parts[(UNSHORT) Data - 1];

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Attacker_part->Char_handle);

	/* Anything in the right hand ? */
	if (Packet_empty(&(Char->Body_items[RIGHT_HAND-1])))
	{
		/* No -> Close-range attack */
		Action = CLOSE_RANGE_COMACT;
		Weapon_item_slot_index = RIGHT_HAND;

		/* Get potential close-range targets */
		Mask = Get_close_range_targets(Attacker_part);

		/* Any ? */
		if (Mask)
		{
			/* Yes -> Select one */
			Target_square_index = Select_tactical_square(434,
			 Show_potential_attack_targets, Mask, 0);
		}
		else
		{
			/* No -> Apologise */
			Set_permanent_message_nr(436);
		}
	}
	else
	{
		/* Yes -> Get right hand item data */
		Right_hand_item = Get_item_data(&Char->Body_items[RIGHT_HAND-1]);

		/* Close- or long-range weapon ? */
		if (Right_hand_item->Type == CLOSE_RANGE_IT)
		{
			/* Close-range weapon -> Close-range attack */
			Action = CLOSE_RANGE_COMACT;
			Weapon_item_slot_index = RIGHT_HAND;

			/* Get potential close-range targets */
			Mask = Get_close_range_targets(Attacker_part);

			/* Any ? */
			if (Mask)
			{
				/* Yes -> Select one */
				Target_square_index = Select_tactical_square(434,
				 Show_potential_attack_targets, Mask, 0);
			}
			else
			{
				/* No -> Apologise */
				Set_permanent_message_nr(436);
			}
		}
		else
		{
			/* Long-range weapon -> Long-range attack */
			Action = LONG_RANGE_COMACT;
			Weapon_item_slot_index = RIGHT_HAND;

			/* Any ammo required ? */
			if (Right_hand_item->Ammo_ID)
			{
				/* Yes -> Get left hand item data */
				Left_hand_item = Get_item_data(&Char->Body_items[LEFT_HAND-1]);

				/* Right ammo ? */
				if (Right_hand_item->Ammo_ID == Left_hand_item->Ammo_ID)
				{
					/* Yes -> Get potential long-range target */
					Mask = Get_long_range_targets(Attacker_part);

					/* Any ? */
					if (Mask)
					{
						/* Yes -> Select one */
						Target_square_index = Select_tactical_square(434,
						 Show_potential_attack_targets, Mask, 0);
					}
					else
					{
						/* No -> Apologise */
						Set_permanent_message_nr(435);
					}
				}
				else
				{
					/* No -> No can do! */
					Set_permanent_message_nr(433);
				}
				Free_item_data();
			}
			else
			{
				/* No -> Get potential long-range target */
				Mask = Get_long_range_targets(Attacker_part);

				/* Any ? */
				if (Mask)
				{
					/* Yes -> Select one */
					Target_square_index = Select_tactical_square(434,
					 Show_potential_attack_targets, Mask, 0);
				}
				else
				{
					/* No -> Apologise */
					Set_permanent_message_nr(435);
				}
			}
		}
		Free_item_data();
	}
	MEM_Free_pointer(Attacker_part->Char_handle);

	/* Any action / any target selected ? */
	if ((Action != NO_COMACT) && (Target_square_index != 0xFFFF))
	{
		/* Yes -> Insert action and target data */
		Attacker_part->Current_action = Action;
		Attacker_part->Target.Attack_target_data.Target_square_index =
		 Target_square_index;
		Attacker_part->Target.Attack_target_data.Weapon_item_slot_index =
		 Weapon_item_slot_index;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Move_tactical_square
 * FUNCTION  : Move (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Move_tactical_square(UNLONG Data)
{
	struct Combat_participant *Part;
	UNLONG Possible_mask;
	UNLONG Occupied_mask;
	UNLONG Remaining_mask;
	UNSHORT Target_square_index;

	/* Get participant data */
	Part = &Party_parts[(UNSHORT) Data - 1];

	/* Get potential movement targets */
	Possible_mask = Get_movement_range(Part);

	/* Get occupied movement targets */
	Occupied_mask = Get_occupied_move_targets(Part);

	/* Transform these */
	Remaining_mask = Possible_mask & ~Occupied_mask;
	Occupied_mask &= Possible_mask;

	/* Any ? */
	if (Remaining_mask)
	{
		/* Yes -> Select one */
		Target_square_index = Select_tactical_square(441,
		 Show_potential_move_targets, Remaining_mask, Occupied_mask);

		/* Any target selected ? */
		if (Target_square_index != 0xFFFF)
		{
			/* Yes -> Insert action and target data */
			Part->Current_action = MOVE_COMACT;
			Part->Target.Move_target_data.Target_square_index =
			 Target_square_index;
		}
	}
	else
	{
		/* No -> Apologise */
		Set_permanent_message_nr(442);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Cast_spell_tactical_square
 * FUNCTION  : Cast spell (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.04.95 11:16
 * LAST      : 13.04.95 11:16
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Cast_spell_tactical_square(UNLONG Data)
{
	struct Combat_participant *Part;
	BOOLEAN Result;

	/* Get participant data */
	Part = &Party_parts[(UNSHORT) Data - 1];

	Result = Cast_combat_spell(Part, 0xFFFF);

	/* Any  ? */
	if (Result)
	{
		/* Yes -> Enter action data */
		Part->Current_action = CAST_SPELL_COMACT;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Use_magic_item_tactical_square
 * FUNCTION  : Use magic item (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.04.95 11:40
 * LAST      : 13.04.95 11:40
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Use_magic_item_tactical_square(UNLONG Data)
{
	struct Combat_participant *Part;
	BOOLEAN Result;
	UNSHORT Selected_item_slot_index;

	/* Get participant data */
	Part = &Party_parts[(UNSHORT) Data - 1];

	/* Select item to be used */
	Selected_item_slot_index = Select_character_item(Part->Char_handle,
	 System_text_ptrs[430], Magical_item_evaluator);

	/* Any selected ? */
	if (Selected_item_slot_index != 0xFFFF)
	{
		/* Yes -> */
		Result = Cast_combat_spell(Part, Selected_item_slot_index);

		/* Any  ? */
		if (Result)
		{
			/* Yes -> Enter action data */
			Part->Current_action = USE_MAGIC_ITEM_COMACT;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Flee_tactical_square
 * FUNCTION  : Flee (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 22:03
 * LAST      : 12.04.95 22:03
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Flee_tactical_square(UNLONG Data)
{
	struct Combat_participant *Part;

	/* Get participant data */
	Part = &Party_parts[(UNSHORT) Data - 1];

	/* Insert action data */
	Part->Current_action = FLEE_COMACT;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Advance_tactical_square
 * FUNCTION  : Advance (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Advance_tactical_square(UNLONG Data)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Start_round
 * FUNCTION  : Start combat round (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Start_round(UNLONG Data)
{
	Combat_round();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_tactical_window
 * FUNCTION  : Draw the tactical window.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.03.95 13:21
 * LAST      : 12.04.95 14:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_tactical_window(void)
{
	/* Restore background */
	Restore_tactical_window_background(TACTICAL_X, TACTICAL_Y,
	 TACTICAL_WIDTH, TACTICAL_HEIGHT);

	if (Draw_tactical_window_flag)
	{
		/* Draw window's shadow */
		Put_recoloured_box(&Main_OPM, TACTICAL_X + 10, TACTICAL_Y + 24 - 7 + 10,
		 TACTICAL_WIDTH - 10, TACTICAL_HEIGHT - 24 + 7 - 10,
		 &(Recolour_tables[1][0]));

		/* Draw window border */
		Draw_window_border(&Main_OPM, TACTICAL_X, TACTICAL_Y + 24 - 7,
		 TACTICAL_WIDTH - 3, TACTICAL_HEIGHT - 24 + 7 - 3);

		/* Draw tactical window */
		Draw_window_inside(&Main_OPM, TACTICAL_X + 7, TACTICAL_Y + 24,
		 (NR_TACTICAL_COLUMNS * TACTICAL_SQUARE_WIDTH) + 2, (NR_TACTICAL_ROWS *
		 TACTICAL_SQUARE_HEIGHT) + 2);

		/* Draw tactical squares */
		Draw_tactical_squares(TACTICAL_X + 8, TACTICAL_Y + 25);

		/* Call tactical display handler (if any) */
		if (Current_tactical_display_handler)
			(Current_tactical_display_handler)(TACTICAL_X + 8, TACTICAL_Y + 25);

		/* Draw tactical icons */
		Draw_tactical_icons(TACTICAL_X + 8, TACTICAL_Y + 25);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_tactical_squares
 * FUNCTION  : Draw all tactical squares.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.03.95 13:19
 * LAST      : 12.04.95 14:00
 * INPUTS    : UNSHORT X - Left X-coordinate.
 *             UNSHORT Y - Top Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_tactical_squares(UNSHORT X, UNSHORT Y)
{
	UNSHORT Saved_X;
	UNSHORT i, j;

	/* Save X-coordinate */
	Saved_X = X;

	/* Do all rows */
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		/* Do all columns */
		X = Saved_X;
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Special drawmode / special square ? */
			if ((Tactical_draw_mode != TACTICAL_NORMAL_DRAWMODE) &&
			 ((i * NR_TACTICAL_COLUMNS + j) == Tactical_draw_index))
			{
				/* Yes -> Feedback ? */
				if (Tactical_draw_mode == TACTICAL_FEEDBACK_DRAWMODE)
				{
					/* Yes -> Feedback */
					Draw_deep_box(&Main_OPM, X, Y, TACTICAL_SQUARE_WIDTH,
					 TACTICAL_SQUARE_HEIGHT);
				}
				else
				{
					/* No -> Highlight */
					Draw_light_box(&Main_OPM, X, Y, TACTICAL_SQUARE_WIDTH,
					 TACTICAL_SQUARE_HEIGHT);
				}
			}
			else
			{
				/* No -> Just draw */
				Draw_high_border(&Main_OPM, X, Y, TACTICAL_SQUARE_WIDTH,
				 TACTICAL_SQUARE_HEIGHT);
			}
			/* Next column */
			X += TACTICAL_SQUARE_WIDTH;
		}
		/* Next row */
		Y += TACTICAL_SQUARE_HEIGHT;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_tactical_icons
 * FUNCTION  : Draw all tactical icons.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 17:24
 * LAST      : 14.03.95 17:59
 * INPUTS    : UNSHORT X - Left X-coordinate.
 *             UNSHORT Y - Top Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_tactical_icons(UNSHORT X, UNSHORT Y)
{
	struct Combat_participant *Part;
	UNSHORT Saved_Y, Saved_X;
	UNSHORT i, j;
	UNBYTE *Ptr;

	Ptr = MEM_Claim_pointer(Tactical_icons_handle);

	/* Save coordinates */
	Saved_X = X;
	Saved_Y = Y;

	/* Do all rows */
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		/* Do all columns */
		X = Saved_X;
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anything in this square ? */
			Part = Combat_matrix[i][j].Part;
			if (Part)
			{
				/* Yes -> Draw icon */
				Put_masked_block(&Main_OPM, X, Y - 24,
				 TACTICAL_ICON_WIDTH, TACTICAL_ICON_HEIGHT,
				 Ptr + (Part->Tactical_icon_nr - 1) * (TACTICAL_ICON_WIDTH *
				 TACTICAL_ICON_HEIGHT));

				/* Party member ? */
				if (Part->Type == PARTY_PART_TYPE)
				{
					/* Yes -> Has action ? */
					if (Part->Current_action != NO_COMACT)
					{
						/* Yes -> Draw combat action icon */
						Put_masked_block(&Main_OPM, X, Y, 16, 16,
						 &(Combat_action_icons[Part->Current_action - 1][0]));
					}
				}
			}
			/* Next column */
			X += TACTICAL_SQUARE_WIDTH;
		}
		/* Next row */
		Y += TACTICAL_SQUARE_HEIGHT;
	}

	MEM_Free_pointer(Tactical_icons_handle);

	/* Set ink colour */
	Set_ink(GOLD_TEXT);

	/* Do all rows */
	Y = Saved_Y;
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		/* Do all columns */
		X = Saved_X;
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anything in this square ? */
			Part = Combat_matrix[i][j].Part;
			if (Part)
			{
				/* Yes -> Any damage ? */
				if ((Part->Damage) && (Part->Damage_display_timer))
				{
					/* Yes -> Display damage */
					xprintf(&Main_OPM, X + 2, Y + 14, "%u", Part->Damage);

					/* Count down */
					Part->Damage_display_timer--;
				}
			}
			/* Next column */
			X += TACTICAL_SQUARE_WIDTH;
		}
		/* Next row */
		Y += TACTICAL_SQUARE_HEIGHT;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_tactical_window_background
 * FUNCTION  : Initialize tactical window background management.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.03.95 16:40
 * LAST      : 16.03.95 16:40
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_tactical_window_background(void)
{
	UNBYTE *Ptr;

	/* Make tactical window background buffer and OPM */
	Tactical_window_background_handle = MEM_Allocate_memory(TACTICAL_WIDTH *
	 TACTICAL_HEIGHT);

	Ptr = MEM_Claim_pointer(Tactical_window_background_handle);
	OPM_New(TACTICAL_WIDTH, TACTICAL_HEIGHT, 1,
	 &Tactical_window_background_OPM, Ptr);
	MEM_Free_pointer(Tactical_window_background_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_tactical_window_background
 * FUNCTION  : Exit tactical window background management.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.03.95 16:40
 * LAST      : 16.03.95 16:40
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_tactical_window_background(void)
{
	/* Destroy tactical window background buffer and OPM */
	OPM_Del(&Tactical_window_background_OPM);

	MEM_Free_memory(Tactical_window_background_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_tactical_window_background
 * FUNCTION  : Get tactical window background.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.03.95 16:41
 * LAST      : 16.03.95 16:41
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_tactical_window_background(void)
{
	/* Save background */
	OPM_CopyOPMOPM(&Main_OPM, &Tactical_window_background_OPM, TACTICAL_X,
	 TACTICAL_Y, TACTICAL_WIDTH, TACTICAL_HEIGHT, 0, 0);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Restore_tactical_window_background
 * FUNCTION  : Restore the tactical window's background.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.03.95 16:41
 * LAST      : 16.03.95 16:41
 * INPUTS    : UNSHORT X - Left X-coordinate of on-screen area.
 *             UNSHORT Y - Top Y-coordinate of on-screen area.
 *             UNSHORT Width - Width of area.
 *             UNSHORT Height - Height of area.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Restore_tactical_window_background(UNSHORT X, UNSHORT Y, UNSHORT Width,
 UNSHORT Height)
{
	/* Restore background */
	OPM_CopyOPMOPM(&Tactical_window_background_OPM, &Main_OPM,
	 X - TACTICAL_X, Y - TACTICAL_Y, Width, Height, X, Y);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_tactical_square
 * FUNCTION  : Let the user select a tactical square.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 14:50
 * LAST      : 12.04.95 14:50
 * INPUTS    : UNSHORT Message_nr - Message number.
 *             Tactical_display_handler Handler - Pointer to tactical display
 *              handler.
 *             UNLONG Mask1 - Selection mask 1.
 *             UNLONG Mask2 - Selection mask 2.
 * RESULT    : UNSHORT : Selected tactical square index / 0xFFFF = aborted.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Select_tactical_square(UNSHORT Message_nr, Tactical_display_handler Handler,
 UNLONG Mask1, UNLONG Mask2)
{
	static struct BBRECT MA = {
		TACTICAL_X + 8,
		TACTICAL_Y + 25,
		NR_TACTICAL_COLUMNS * TACTICAL_SQUARE_WIDTH,
		NR_TACTICAL_ROWS * TACTICAL_SQUARE_HEIGHT
	};

	/* Print text */
	Set_permanent_message_nr(Message_nr);

	/* Install mouse area */
	Push_MA(&MA);

	/* Reset selected index */
	Selected_tactical_square_index = 0xFFFF;

	/* Set selection data */
	Tactical_select_mask = Mask1;
	Tactical_select_mask2 = Mask2;
	Current_tactical_display_handler = Handler;

	/* Enter selection mode */
	Tactical_select_mode = TRUE;

	Push_module(&Tactical_select_Mod);

	/* Leave selection mode */
	Tactical_select_mode = FALSE;

	/* Reset selection data */
	Current_tactical_display_handler = NULL;
	Tactical_select_mask = 0;
	Tactical_select_mask2 = 0;

	/* Redraw tactical window */
	Draw_tactical_window();

	/* Remove mouse area */
	Pop_MA();

	/* Clear text */
	Clear_permanent_text();

	return(Selected_tactical_square_index);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_potential_attack_targets
 * FUNCTION  : Show potential attack targets in the tactical window.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 12:52
 * LAST      : 12.04.95 12:52
 * INPUTS    : UNSHORT X - Left X-coordinate.
 *             UNSHORT Y - Top Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_potential_attack_targets(UNSHORT X, UNSHORT Y)
{
	UNLONG T;
	UNSHORT Saved_X;
	UNSHORT i, j;

	/* Blink ? */
	T = SYSTEM_GetTicks();
	if (T & (1 << 4))
	{
		/* Yes -> Save X-coordinate */
		Saved_X = X;

		/* Do all rows */
		for (i=0;i<NR_TACTICAL_ROWS;i++)
		{
			/* Do all columns */
			X = Saved_X;
			for (j=0;j<NR_TACTICAL_COLUMNS;j++)
			{
				/* Is this square a potential target ? */
				if (Tactical_select_mask & (1 << (i * NR_TACTICAL_COLUMNS + j)))
				{
					/* Yes -> Indicate */
					OPM_Box(&Main_OPM, X, Y, TACTICAL_SQUARE_WIDTH,
					 TACTICAL_SQUARE_HEIGHT, GREEN);
				}

				/* Next column */
				X += TACTICAL_SQUARE_WIDTH;
			}
			/* Next row */
			Y += TACTICAL_SQUARE_HEIGHT;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_potential_move_targets
 * FUNCTION  : Show potential movement targets in the tactical window.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.04.95 10:35
 * LAST      : 13.04.95 10:35
 * INPUTS    : UNSHORT X - Left X-coordinate.
 *             UNSHORT Y - Top Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_potential_move_targets(UNSHORT X, UNSHORT Y)
{
	UNLONG T;
	UNSHORT Saved_X;
	UNSHORT i, j;

	/* Save X-coordinate */
	Saved_X = X;

	/* Blink ? */
	T = SYSTEM_GetTicks();
	if (T & (1 << 4))
	{
		/* Yes -> Do all rows */
		for (i=0;i<NR_TACTICAL_ROWS;i++)
		{
			/* Do all columns */
			X = Saved_X;
			for (j=0;j<NR_TACTICAL_COLUMNS;j++)
			{
				/* Is this square a potential movement target ? */
				if (Tactical_select_mask & (1 << (i * NR_TACTICAL_COLUMNS + j)))
				{
					/* Yes -> Indicate */
					OPM_Box(&Main_OPM, X, Y, TACTICAL_SQUARE_WIDTH,
					 TACTICAL_SQUARE_HEIGHT, GREEN);
				}

				/* Next column */
				X += TACTICAL_SQUARE_WIDTH;
			}
			/* Next row */
			Y += TACTICAL_SQUARE_HEIGHT;
		}
	}
	else
	{
		/* No -> Do all rows */
		for (i=0;i<NR_TACTICAL_ROWS;i++)
		{
			/* Do all columns */
			X = Saved_X;
			for (j=0;j<NR_TACTICAL_COLUMNS;j++)
			{
				/* Is this square occupied ? */
				if (Tactical_select_mask2 & (1 << (i * NR_TACTICAL_COLUMNS + j)))
				{
					/* Yes -> Indicate */
					OPM_Box(&Main_OPM, X, Y, TACTICAL_SQUARE_WIDTH,
					 TACTICAL_SQUARE_HEIGHT, RED);
				}

				/* Next column */
				X += TACTICAL_SQUARE_WIDTH;
			}
			/* Next row */
			Y += TACTICAL_SQUARE_HEIGHT;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_potential_row_targets
 * FUNCTION  : Show potential row targets in the tactical window.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.04.95 12:10
 * LAST      : 19.04.95 12:10
 * INPUTS    : UNSHORT X - Left X-coordinate.
 *             UNSHORT Y - Top Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_potential_row_targets(UNSHORT X, UNSHORT Y)
{
	UNLONG T;
	UNSHORT i, j;

	/* Blink ? */
	T = SYSTEM_GetTicks();
	if (T & (1 << 4))
	{
		/* Yes -> Do all rows */
		for (i=0;i<NR_TACTICAL_ROWS;i++)
		{
			/* Check all columns in this row */
			for (j=0;j<NR_TACTICAL_COLUMNS;j++)
			{
				/* Is this square a potential movement target ? */
				if (Tactical_select_mask & (1 << (i * NR_TACTICAL_COLUMNS + j)))
				{
					/* Yes -> Indicate this row */
					OPM_Box(&Main_OPM, X, Y, TACTICAL_SQUARE_WIDTH *
					 NR_TACTICAL_COLUMNS, TACTICAL_SQUARE_HEIGHT, GREEN);

					break;
				}
			}
			/* Next row */
			Y += TACTICAL_SQUARE_HEIGHT;
		}
	}
}

