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
#include <stdio.h>
#include <string.h>

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <MONLOGIC.H>
#include <GAMETEXT.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <COMBVAR.H>
#include <COMACTS.H>
#include <TACTICAL.H>
#include <EVELOGIC.H>
#include <POPUP.H>
#include <STATAREA.H>
#include <MAGIC.H>
#include <INPUT.H>
#include <INVENTO.H>
#include <ITMLOGIC.H>
#include <ITEMSEL.H>
#include <COLOURS.H>
#include <MAINMENU.H>
#include <BUTTONS.H>

/* defines */

/* Tactical icon size */
#define TACTICAL_ICON_WIDTH		(32)
#define TACTICAL_ICON_HEIGHT		(48)

/* Tactical window draw modes */
#define TACTICAL_NORMAL_DRAWMODE	 		(0)
#define TACTICAL_FEEDBACK_DRAWMODE		(1)
#define TACTICAL_HIGHLIGHT_DRAWMODE		(2)

/* structure definitions */

/* Tactical square OID*/
struct Tactical_square_OID {
	UNSHORT X, Y;
};

/* Tactical square object */
struct Tactical_square_object {
	struct Object Object;
	UNSHORT X, Y;
};

/* prototypes */

/* Tactical window methods */
UNLONG Init_Tactical_window_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Tactical_window_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Tactical_window_object(struct Object *Object, union Method_parms *P);

/* Tactical square methods */
UNLONG Init_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG DLeft_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Right_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Touch_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Pop_up_Tactical_square_object(struct Object *Object, union Method_parms *P);

/* Tactical square pop-up menu actions */
void Tactical_square_PUM_evaluator(struct PUM *PUM);
void PUM_No_action(UNLONG Data);
void PUM_Attack_tactical_square(UNLONG Data);
void PUM_Move_tactical_square(UNLONG Data);
void PUM_Cast_spell_tactical_square(UNLONG Data);
void PUM_Use_magic_item_tactical_square(UNLONG Data);
void PUM_Flee_tactical_square(UNLONG Data);
void PUM_Advance_tactical_square(UNLONG Data);
void PUM_View_combat(UNLONG Data);
void PUM_Exit_combat(UNLONG Data);
void PUM_Combat_Main_menu(UNLONG Data);

void Start_combat_round(struct Button_object *Button);

/* Tactical window display functions */
void Draw_tactical_squares(void);
void Draw_tactical_icons(void);

/* Tactical selection module functions */
void Tactical_select_ModInit(void);
void Tactical_select_ModExit(void);

void Show_participant_targets(void);
void Show_participant_magic_targets(struct Combat_participant *Casting_part);

/* global variables */

static UNSHORT Tactical_draw_mode = TACTICAL_NORMAL_DRAWMODE;
static UNSHORT Tactical_draw_index;

static Tactical_display_handler Current_tactical_display_handler = NULL;

static BOOLEAN Tactical_select_mode = FALSE;
static UNSHORT Tactical_select_message_nr;
static UNLONG Tactical_select_mask;
static UNLONG Tactical_select_mask2;
static UNSHORT Selected_tactical_square_index;

static UNSHORT Start_round_button_object;

static MEM_HANDLE Tactical_window_background_handle;
static struct OPM Tactical_window_background_OPM;

static BOOLEAN Draw_tactical_window_flag = TRUE;

static struct Combat_participant *Highlight_part;

/* Tactical window select module */
static struct Module Tactical_select_Mod = {
	LOCAL_MOD, MODE_MOD, NO_SCREEN,
	NULL,
	Tactical_select_ModInit,
	Tactical_select_ModExit,
	NULL,
	NULL,
	NULL
};

/* Tactical window method list */
static struct Method Tactical_window_methods[] = {
	{ INIT_METHOD,			Init_Tactical_window_object },
	{ DRAW_METHOD,			Draw_Tactical_window_object },
	{ UPDATE_METHOD,		Update_Tactical_window_object },
	{ TOUCHED_METHOD,		Dehighlight },
	{ 0, NULL }
};

/* Tactical window class description */
struct Object_class Tactical_window_Class = {
	0, sizeof(struct Object),
	&Tactical_window_methods[0]
};

/* Tactical square method list */
static struct Method Tactical_square_methods[] = {
	{ INIT_METHOD,			Init_Tactical_square_object },
	{ DRAW_METHOD,			Draw_Tactical_square_object },
	{ FEEDBACK_METHOD,	Feedback_Tactical_square_object },
	{ HIGHLIGHT_METHOD,	Highlight_Tactical_square_object },
	{ LEFT_METHOD,			Left_Tactical_square_object },
	{ DLEFT_METHOD,		DLeft_Tactical_square_object },
	{ RIGHT_METHOD,		Right_Tactical_square_object },
	{ TOUCHED_METHOD,		Touch_Tactical_square_object },
	{ HELP_METHOD,			Help_Tactical_square_object },
	{ POP_UP_METHOD,		Pop_up_Tactical_square_object },
	{ 0, NULL}
};

/* Tactical square class description */
static struct Object_class Tactical_square_Class = {
	0, sizeof(struct Tactical_square_object),
	&Tactical_square_methods[0]
};

/* Tactical square pop-up menu */
static struct PUME Tactical_square_PUMEs[] = {
	{PUME_AUTO_CLOSE,			0,	746,	PUM_No_action},
	{PUME_AUTO_CLOSE,			0,	1,		PUM_Attack_tactical_square},
	{PUME_AUTO_CLOSE,			0,	2,		PUM_Move_tactical_square},
	{PUME_AUTO_CLOSE,			0,	3,		PUM_Cast_spell_tactical_square},
	{PUME_AUTO_CLOSE,			0,	199,	PUM_Use_magic_item_tactical_square},
	{PUME_AUTO_CLOSE,			0,	198,	PUM_Flee_tactical_square},
	{PUME_NOT_SELECTABLE,	0,	0,		NULL},
	{PUME_AUTO_CLOSE,			0,	200,	PUM_Advance_tactical_square},
	{PUME_NOT_SELECTABLE,	0,	0,		NULL},
	{PUME_AUTO_CLOSE,			0,	488,	PUM_View_combat},
	{PUME_AUTO_CLOSE,			0,	696,	PUM_Combat_Main_menu},
	{PUME_AUTO_CLOSE,			0,	546,	PUM_Exit_combat}
};

static struct PUM Tactical_square_PUM = {
	12,
	NULL,
	NULL,
	Tactical_square_PUM_evaluator,
	Tactical_square_PUMEs
};

/* Tactical combat matrix */
struct Tactical_square Combat_matrix[NR_TACTICAL_ROWS][NR_TACTICAL_COLUMNS];

/* Start round button data */
static union Button_data Start_round_button_data;
static struct Button_OID Start_round_button_OID = {
	TEXT_BUTTON_TYPE,
	0,
	0xFFFF,
	&Start_round_button_data,
	Start_combat_round
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Tactical_window_object
 * FUNCTION  : Initialize method of Tactical window object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 11:59
 * LAST      : 01.10.95 17:22
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Tactical_window_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_OID Square_OID;
	UNSHORT i, j;

	/* Get background */
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
			Add_object
			(
				Object->Self,
				&Tactical_square_Class,
				(UNBYTE *) &Square_OID,
				(j * TACTICAL_SQUARE_WIDTH) + 8,
				(i * TACTICAL_SQUARE_HEIGHT) + 25,
				TACTICAL_SQUARE_WIDTH,
				TACTICAL_SQUARE_HEIGHT
			);
		}
	}

	/* Initialize start round button data */
	Start_round_button_data.Text_button_data.Text = System_text_ptrs[202];

	/* Add start round button */
	Start_round_button_object = Add_object
	(
		Object->Self,
		&Button_Class,
		(UNBYTE *) &Start_round_button_OID,
		8 + ((NR_TACTICAL_COLUMNS * TACTICAL_SQUARE_WIDTH) - 80) / 2,
		25 + (NR_TACTICAL_ROWS * TACTICAL_SQUARE_HEIGHT) + 3,
		80,
		13
	);

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Tactical_window_object(struct Object *Object, union Method_parms *P)
{
	if (!Fighting)
	{
		Draw_tactical_window();
	}

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Tactical_window_object(struct Object *Object, union Method_parms *P)
{
	if (Current_tactical_display_handler && !Fighting)
	{
		Draw_tactical_window();
	}

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_object *Square;
	struct Tactical_square_OID *OID;

	Square = (struct Tactical_square_object *) Object;
	OID = (struct Tactical_square_OID *) P;

	/* Copy data from OID */
	Square->X = OID->X;
	Square->Y = OID->Y;

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	if (!Fighting)
	{
		Tactical_draw_mode = TACTICAL_NORMAL_DRAWMODE;

		Draw_tactical_window();
	}

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
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

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
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

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Tactical_square_object
 * FUNCTION  : Left method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 13:15
 * LAST      : 14.09.95 22:05
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
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

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : DLeft_Tactical_square_object
 * FUNCTION  : DLeft method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.10.95 14:42
 * LAST      : 05.10.95 14:46
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
DLeft_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_object *Square;
	struct Combat_participant *Part;

	Square = (struct Tactical_square_object *) Object;

	/* Select mode ? */
	if (!Tactical_select_mode)
	{
		/* No -> Anything in this square ? */
		Part = Combat_matrix[Square->Y][Square->X].Part;
		if (Part)
		{
			/* Yes -> Party member ? */
			if (Part->Type == PARTY_PART_TYPE)
			{
				/* Yes -> Enter inventory */
				Go_Inventory(Part->Number);
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Right_Tactical_square_object
 * FUNCTION  : Right method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 13:15
 * LAST      : 14.09.95 22:04
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Right_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	/* Select mode ? */
	if (Tactical_select_mode)
	{
		/* Yes -> Abort selection */
		Pop_module();
	}
	else
	{
		/* No -> Do normal action */
		Normal_rightclicked(Object, P);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Touch_Tactical_square_object
 * FUNCTION  : Touch method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 16:57
 * LAST      : 14.09.95 21:53
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
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
			/* Yes -> Touch normally */
			Normal_touched(Object, NULL);
		}
		else
		{
			/* No -> Dehighlight */
			Dehighlight(Object, NULL);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Tactical_square_object
 * FUNCTION  : Help method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.95 20:00
 * LAST      : 01.10.95 19:39
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_object *Square;
	struct Combat_participant *Part;
	UNCHAR Char_name[CHAR_NAME_LENGTH];
	UNCHAR Help_string[200];

	Square = (struct Tactical_square_object *) Object;

	/* Anything in this square ? */
	Part = Combat_matrix[Square->Y][Square->X].Part;
	if (Part)
	{
		/* Yes -> A real participant ? */
		if (Part->Type == EMPTY_PART_TYPE)
		{
			/* No -> Clear help line */
			Execute_method
			(
				Help_line_object,
				SET_METHOD,
				NULL
			);
		}
		else
		{
			/* Yes -> Get participant name */
			Get_char_name(Part->Char_handle, Char_name);

			/* Is party member / monster LP are shown / cheat mode ? */
			if ((Part->Type == PARTY_PART_TYPE) || Show_monster_LP_flag ||
			 Cheat_mode)
			{
				/* Yes -> Build help line string */
				sprintf
				(
					Help_string,
					"%s (%u / %u)",
					Char_name,
					Get_LP(Part->Char_handle),
					Get_max_LP(Part->Char_handle)
			  	);
			}
			else
			{
				/* No -> Build help line string */
				strcpy(Help_string, Char_name);
			}

			/* Set help line */
			Execute_method
			(
				Help_line_object,
				SET_METHOD,
				(void *) Help_string
			);

			/* Is this a party member / any action ? */
			if ((Part->Type == PARTY_PART_TYPE) &&
			 (Part->Current_action != NO_COMACT))
			{
				/* Yes -> Show targets */
				Highlight_part = Part;

				Current_tactical_display_handler = Show_participant_targets;

				Draw_tactical_window();

				Current_tactical_display_handler = NULL;
			}
		}
	}
	else
	{
		/* No -> Clear help line */
		Execute_method
		(
			Help_line_object,
			SET_METHOD,
			NULL
		);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_up_Tactical_square_object
 * FUNCTION  : Pop-up method of Tactical square object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 16:46
 * LAST      : 14.09.95 19:59
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Pop_up_Tactical_square_object(struct Object *Object, union Method_parms *P)
{
	struct Tactical_square_object *Square;
	struct Combat_participant *Part;
	UNCHAR Char_name[CHAR_NAME_LENGTH];

	Square = (struct Tactical_square_object *) Object;

	/* Set default pop-up menu title */
	Tactical_square_PUM.Title = System_text_ptrs[697];

	/* Anything in this square ? */
	Part = Combat_matrix[Square->Y][Square->X].Part;
	if (Part)
	{
		/* Yes -> A party member ? */
		if (Part->Type == PARTY_PART_TYPE)
		{
			/* Yes -> Get participant name */
			Get_char_name(Part->Char_handle, Char_name);

			/* Use as pop-up menu title */
			Tactical_square_PUM.Title = Char_name;
		}

		/* Set character handle */
		PUM_char_handle = Part->Char_handle;
	}
	else
	{
		/* No -> Clear character handle */
		PUM_char_handle = NULL;
	}

	/* Store pointer to participant */
	Tactical_square_PUM.Data = (UNLONG) Part;

	/* Call pop-up menu */
	PUM_source_object_handle = Object->Self;

	Do_PUM
	(
		Object->X + 32,
		Object->Y + 8,
		&Tactical_square_PUM
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Tactical_square_PUM_evaluator
 * FUNCTION  : Evaluate tactical square pop-up menu.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 16:46
 * LAST      : 07.10.95 22:23
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
	struct Combat_participant *PUM_part;
	struct Combat_participant *Part;
	BOOLEAN Front_rows_empty;
	UNSHORT Conditions;
	UNSHORT i, j;

	PUMES = PUM->PUME_list;
	PUM_part = (struct Combat_participant *) PUM->Data;

	/* Anyone there ? */
	if (PUM_part)
	{
		/* Yes -> Act depending on participant type */
		switch (PUM_part->Type)
		{
			/* Empty */
			case EMPTY_PART_TYPE:
			{
				/* Disable all */
				PUMES[0].Flags |= PUME_ABSENT;
				PUMES[1].Flags |= PUME_ABSENT;
				PUMES[2].Flags |= PUME_ABSENT;
				PUMES[3].Flags |= PUME_ABSENT;
				PUMES[4].Flags |= PUME_ABSENT;
				PUMES[5].Flags |= PUME_ABSENT;
				PUMES[7].Flags |= PUME_ABSENT;

				break;
			}
			/* Party member */
			case PARTY_PART_TYPE:
			{
				/* Get body conditions */
				Conditions = Get_conditions(PUM_part->Char_handle);

				/* Controllable ? */
				if (Conditions & CONTROL_MASK)
				{
					/* No -> Disable all */
					PUMES[0].Flags |= PUME_ABSENT;
					PUMES[1].Flags |= PUME_ABSENT;
					PUMES[2].Flags |= PUME_ABSENT;
					PUMES[3].Flags |= PUME_ABSENT;
					PUMES[4].Flags |= PUME_ABSENT;
					PUMES[5].Flags |= PUME_ABSENT;
				}
				else
				{
					/* Yes -> Can attack ? */
					if (Conditions & ATTACK_MASK)
					{
						/* No -> Block attack */
						PUMES[1].Flags |= PUME_BLOCKED;

						/* Explain */
						PUMES[1].Blocked_message_nr = 437;
					}
					else
					{
						/* Yes -> Can do damage ? */
						if (Get_damage(PUM_part->Char_handle))
						{
							/* Yes -> Attack is possible */
							PUMES[1].Flags &= ~PUME_BLOCKED;
						}
						else
						{
							/* No -> Block attack */
							PUMES[1].Flags |= PUME_BLOCKED;

							/* Explain */
							PUMES[1].Blocked_message_nr = 438;
						}
					}

					/* Can move ? */
					if (Conditions & MOVE_MASK)
					{
						/* No -> Block move */
						PUMES[2].Flags |= PUME_BLOCKED;

						/* Explain */
						PUMES[2].Blocked_message_nr = 439;
					}
					else
					{
						/* Yes -> Move is possible */
						PUMES[2].Flags &= ~PUME_BLOCKED;
					}

					/* Does this character have magical abilities ? */
					if (Character_has_magical_abilities(PUM_part->Char_handle) &&
					 !(Conditions & MAGIC_MASK))
					{
						/* Yes -> Could use magic */
						PUMES[3].Flags &= ~PUME_ABSENT;

						/* Know any spells ? */
						if (Character_knows_spells(PUM_part->Char_handle))
						{
							/* Yes -> Can use magic */
							PUMES[3].Flags &= ~PUME_BLOCKED;
						}
						else
						{
							/* No -> Block use magic */
							PUMES[3].Flags |= PUME_BLOCKED;

							/* Explain */
							PUMES[3].Blocked_message_nr = 425;
						}
					}
					else
					{
						/* No -> Cannot use magic */
						PUMES[3].Flags |= PUME_ABSENT;
					}

					/* In the last row ? */
					if (PUM_part->Tactical_Y == NR_TACTICAL_ROWS - 1)
					{
						/* Yes -> Could flee */
						PUMES[5].Flags &= ~PUME_ABSENT;

						/* Can flee ? */
						if (Conditions & FLEE_MASK)
						{
							/* No -> Block flee */
							PUMES[5].Flags |= PUME_BLOCKED;

							/* Explain */
							PUMES[5].Blocked_message_nr = 440;
						}
						else
						{
							/* Yes -> Can flee */
							PUMES[5].Flags &= ~PUME_BLOCKED;
						}
					}
					else
					{
						/* No -> Cannot flee */
						PUMES[5].Flags |= PUME_ABSENT;
					}

					/* Are there any monsters in the front rows ? */
					Front_rows_empty = TRUE;
					for (i=2;i<=4;i++)
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
						if (!Front_rows_empty)
							break;
					}

					/* Well ? */
					if (Front_rows_empty)
					{
					  	/* Yes -> Could advance */
						PUMES[7].Flags &= ~PUME_ABSENT;
					}
					else
					{
						/* No -> Cannot advance */
						PUMES[7].Flags |= PUME_ABSENT;
					}
				}
				break;
			}
			/* Monster */
			case MONSTER_PART_TYPE:
			{
				/* Disable all */
				PUMES[0].Flags |= PUME_ABSENT;
				PUMES[1].Flags |= PUME_ABSENT;
				PUMES[2].Flags |= PUME_ABSENT;
				PUMES[3].Flags |= PUME_ABSENT;
				PUMES[4].Flags |= PUME_ABSENT;
				PUMES[5].Flags |= PUME_ABSENT;
				PUMES[7].Flags |= PUME_ABSENT;

				break;
			}
		}
	}
	else
	{
		/* No -> Disable all */
		PUMES[0].Flags |= PUME_ABSENT;
		PUMES[1].Flags |= PUME_ABSENT;
		PUMES[2].Flags |= PUME_ABSENT;
		PUMES[3].Flags |= PUME_ABSENT;
		PUMES[4].Flags |= PUME_ABSENT;
		PUMES[5].Flags |= PUME_ABSENT;
		PUMES[7].Flags |= PUME_ABSENT;
	}

	/* In cheat mode ? */
	if (!Cheat_mode)
	{
		/* No -> Cannot exit combat */
		PUMES[11].Flags |= PUME_ABSENT;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_No_action
 * FUNCTION  : No action (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.10.95 20:04
 * LAST      : 10.10.95 20:04
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_No_action(UNLONG Data)
{
	struct Combat_participant *Part;

	/* Get participant data */
	Part = (struct Combat_participant *) Data;

	/* Clear action */
	Part->Current_action = NO_COMACT;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Attack_tactical_square
 * FUNCTION  : Attack (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 21:40
 * LAST      : 02.10.95 22:47
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
	UNLONG Mask;
	UNSHORT Action = NO_COMACT;
	UNSHORT Target_square_index = 0xFFFF;
	UNSHORT Weapon_slot_index;

	/* Get participant data */
	Attacker_part = (struct Combat_participant *) Data;

	/* Search the participant's body for long-range weapons */
	Weapon_slot_index = Search_body_for_item_type
	(
		Attacker_part->Char_handle,
		LONG_RANGE_IT
	);

	/* Is the participant carrying a long-range weapon ? */
	if (Weapon_slot_index != 0xFFFF)
	{
		/* Yes -> Does it have the right ammo ? */
		if (Check_if_long_range_weapon_has_ammo
		(
			Attacker_part,
			Weapon_slot_index
		))
		{
			/* Yes -> Get potential long-range targets */
			Mask = Get_long_range_targets(Attacker_part);

			/* Any ? */
			if (Mask)
			{
				/* Yes -> Select one */
				Target_square_index = Select_tactical_square
				(
					434,
					Show_potential_attack_targets,
					Mask,
					0
				);

				/* Long-range attack */
				Action = LONG_RANGE_COMACT;
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
	}
	else
	{
		/* No -> Search the participant's body for close-range weapons */
		Weapon_slot_index = Search_body_for_item_type
		(
			Attacker_part->Char_handle,
			CLOSE_RANGE_IT
		);

		/* Get potential close-range targets */
		Mask = Get_close_range_targets(Attacker_part);

		/* Any ? */
		if (Mask)
		{
			/* Yes -> Select one */
			Target_square_index = Select_tactical_square
			(
				434,
				Show_potential_attack_targets,
				Mask,
				0
			);

			/* Close-range attack */
			Action = CLOSE_RANGE_COMACT;
		}
		else
		{
			/* No -> Apologise */
			Set_permanent_message_nr(436);
		}
	}

	/* Any action / any target selected ? */
	if ((Action != NO_COMACT) && (Target_square_index != 0xFFFF))
	{
		/* Yes -> Insert action and target data */
		Attacker_part->Current_action = Action;
		Attacker_part->Target.Attack_target_data.Target_square_index		= Target_square_index;
		Attacker_part->Target.Attack_target_data.Weapon_item_slot_index	= Weapon_slot_index;

		/* Redraw tactical window */
		Draw_tactical_window();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Move_tactical_square
 * FUNCTION  : Move (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 21:40
 * LAST      : 15.09.95 12:21
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
	Part = (struct Combat_participant *) Data;

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
			Part->Target.Move_target_data.Nr_moves = 1;
			Part->Target.Move_target_data.Target_square_indices[0] = Target_square_index;

			/* Redraw tactical window */
			Draw_tactical_window();
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
 * LAST      : 15.09.95 12:22
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
	Part = (struct Combat_participant *) Data;

	/* Cast combat spell */
	Result = Cast_combat_spell(Part, 0xFFFF);

	/* Any  ? */
	if (Result)
	{
		/* Yes -> Enter action data */
		Part->Current_action = CAST_SPELL_COMACT;

		/* Redraw tactical window */
		Draw_tactical_window();
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
 * LAST      : 15.09.95 12:22
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
	Part = (struct Combat_participant *) Data;

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

			/* Redraw tactical window */
			Draw_tactical_window();
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
 * LAST      : 15.09.95 12:22
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
	Part = (struct Combat_participant *) Data;

	/* Insert action data */
	Part->Current_action = FLEE_COMACT;

	/* Redraw tactical window */
	Draw_tactical_window();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Advance_tactical_square
 * FUNCTION  : Advance (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.09.95 15:29
 * LAST      : 21.09.95 15:53
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
	struct Combat_participant *Part;
	SISHORT X, Y;
	UNSHORT i, j;

	/* The fight is on! */
	Fighting = TRUE;

	/* Delete interface objects */
	Delete_object(Tactical_window_object);

	/* Re-draw the screen */
	Update_screen();

	/* Take 8 steps */
	for (i=0;i<8;i++)
	{
		/* Scan all COMOBs */
		for (j=0;j<MAX_COMOBS;j++)
		{
			/* Anything there ? */
			if ((COMOB_table_ptr + j)->System_flags & COMOB_PRESENT)
			{
				/* Yes -> Advance it */
				(COMOB_table_ptr + j)->Z_3D -= COMBAT_SQUARE_DEPTH / 8;
			}
		}

		/* Redraw */
		Update_screen();
	}

	Fighting = FALSE;

	Update_screen();

	/* Move all monsters and traps down */
	for (Y=NR_TACTICAL_ROWS - 1;Y>=0;Y--)
	{
		for (X=0;X<NR_TACTICAL_COLUMNS;X++)
		{
			/* Anyone there ? */
			Part = Combat_matrix[Y][X].Part;
			if (Part)
			{
				/* Yes -> Monster ? */
				if (Part->Type == MONSTER_PART_TYPE)
				{
					/* Yes -> Increase Y-coordinate */
					Part->Tactical_Y++;

					/* Move down */
					Combat_matrix[Y + 1][X].Part	= Part;
					Combat_matrix[Y][X].Part		= NULL;
				}
			}

			/* Is there a trap here ? */
			if (Is_trap(X, Y))
			{
				/* Yes -> Move down */
				memcpy
				(
					(UNBYTE *) &(Combat_matrix[Y + 1][X].Trap_data),
					(UNBYTE *) &(Combat_matrix[Y][X].Trap_data),
					sizeof(struct Trap_data)
				);
				BASEMEM_FillMemByte
				(
					(UNBYTE *) &(Combat_matrix[Y][X].Trap_data),
					sizeof(struct Trap_data),
					0
				);
			}
		}
	}

	/* Add tactical window object */
	Tactical_window_object = Add_object(Earth_object, &Tactical_window_Class,
	 NULL, TACTICAL_X, TACTICAL_Y, TACTICAL_WIDTH, TACTICAL_HEIGHT);

	/* Draw */
	Execute_method(Tactical_window_object, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_View_combat
 * FUNCTION  : View combat (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.06.95 20:19
 * LAST      : 12.06.95 20:19
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_View_combat(UNLONG Data)
{
	/* Enable combat screen re-draw in main loop */
	Fighting = TRUE;

	/* Wait for the user */
	Wait_4_user();

	/* Disable combat screen re-draw in main loop */
	Fighting = FALSE;

	/* Update the screen */
	Update_screen();

	/* Re-draw tactical window */
	Get_tactical_window_background();
	Execute_method(Tactical_window_object, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Exit_combat
 * FUNCTION  : Exit combat (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.08.95 21:37
 * LAST      : 13.08.95 21:37
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Exit_combat(UNLONG Data)
{
	struct Combat_participant *Part;
	UNSHORT i, j;

	/* Search the matrix for participants */
	for (i=0;i<4;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anything here in the matrix ? */
			if (Combat_matrix[i][j].Part)
			{
				/* Yes -> Get participant data */
				Part = Combat_matrix[i][j].Part;

				/* Is monster ? */
				if (Part->Type == MONSTER_PART_TYPE)
				{
					/* Yes -> Kill */
					Kill_participant(Part);
				}
			}
		}
	}

	Combat_round();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Combat_Main_menu
 * FUNCTION  : Enter main menu (tactical square pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.09.95 22:59
 * LAST      : 02.10.95 17:48
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Combat_Main_menu(UNLONG Data)
{
	Enter_Main_menu(MAIN_MENU_COMBAT);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Start_combat_round
 * FUNCTION  : Start combat round (button).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 22:03
 * LAST      : 01.10.95 17:17
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Start_combat_round(struct Button_object *Button)
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
 * LAST      : 01.10.95 17:40
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
	Restore_tactical_window_background
	(
		TACTICAL_X,
		TACTICAL_Y,
		TACTICAL_WIDTH,
		TACTICAL_HEIGHT
	);

	if (Draw_tactical_window_flag)
	{
		/* Draw window's shadow */
		Put_recoloured_box
		(
			&Main_OPM,
			TACTICAL_X + 10,
			TACTICAL_Y + 24 - 7 + 10,
			TACTICAL_WIDTH - 10,
			TACTICAL_HEIGHT - 24 + 7 - 10,
			&(Recolour_tables[1][0])
		);

		/* Draw window border */
		Draw_window_border
		(
			&Main_OPM,
			TACTICAL_X,
			TACTICAL_Y + 24 - 7,
			TACTICAL_WIDTH - 3,
			TACTICAL_HEIGHT - 24 + 7 - 3
		);

		/* Draw tactical window */
		Draw_window_inside
		(
			&Main_OPM,
			TACTICAL_X + 7,
			TACTICAL_Y + 24,
			(NR_TACTICAL_COLUMNS * TACTICAL_SQUARE_WIDTH) + 2,
			(NR_TACTICAL_ROWS * TACTICAL_SQUARE_HEIGHT) + 2 + 19
		);

		/* Draw tactical squares */
		Draw_tactical_squares();

		/* Call tactical display handler (if any) */
		if (Current_tactical_display_handler)
			(Current_tactical_display_handler)();

		/* Draw tactical icons */
		Draw_tactical_icons();

		/* Draw button */
		Execute_method(Start_round_button_object, DRAW_METHOD, NULL);
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
 * LAST      : 15.09.95 18:20
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_tactical_squares(void)
{
	UNSHORT X, Y;
	UNSHORT i, j;

	/* Do all rows */
	Y = TACTICAL_Y + 25;
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		/* Do all columns */
		X = TACTICAL_X + 8;
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
 * LAST      : 07.10.95 19:24
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_tactical_icons(void)
{
	struct Combat_participant *Part;
	UNSHORT X, Y;
	UNSHORT i, j;
	UNCHAR String[10];
	UNBYTE *Ptr;

	/* Set ink colour */
	Set_ink(WHITE_TEXT);

	/* Display all icons */
	Y = TACTICAL_Y + 25;
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		X = TACTICAL_X + 8;
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anything in this square ? */
			Part = Combat_matrix[i][j].Part;
			if (Part)
			{
				/* Yes -> Draw icon */
				Ptr = MEM_Claim_pointer(Part->Tactical_icon_handle);

				Put_masked_block(&Main_OPM, X, Y - 24,
				 TACTICAL_ICON_WIDTH, TACTICAL_ICON_HEIGHT, Ptr);

				MEM_Free_pointer(Part->Tactical_icon_handle);

				/* Party member ? */
				if (Part->Type == PARTY_PART_TYPE)
				{
					/* Yes -> Has action ? */
					if (Part->Current_action != NO_COMACT)
					{
						/* Yes -> Draw combat action icon */
						Put_masked_block
						(
							&Main_OPM,
							X,
							Y,
							16,
							16,
							&(Combat_action_icons[Part->Current_action - 1][0])
						);
					}
				}
			}
			/* Next column */
			X += TACTICAL_SQUARE_WIDTH;
		}
		/* Next row */
		Y += TACTICAL_SQUARE_HEIGHT;
	}

	/* Display all damage */
	Y = TACTICAL_Y + 25;
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		X = TACTICAL_X + 8;
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anything in this square ? */
			Part = Combat_matrix[i][j].Part;
			if (Part)
			{
				/* Yes -> Any damage ? */
				if ((Part->Damage) && (Part->Damage_display_timer))
				{
					/* Yes -> Draw pain symbol */
					Put_masked_block
					(
						&Main_OPM,
						X,
						Y - 8,
						32,
						32,
						&(Pain_symbol[0])
					);

					/* Build damage string */
					sprintf
					(
						String,
						"%u",
						Part->Damage
					);

					/* Display damage */
					Print_centered_string
					(
						&Main_OPM,
						X,
						Y - 8 + 14,
						32,
						String
					);

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

	OPM_New
	(
		TACTICAL_WIDTH,
		TACTICAL_HEIGHT,
		1,
		&Tactical_window_background_OPM,
		Ptr
	);

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
	OPM_CopyOPMOPM
	(
		&Main_OPM,
		&Tactical_window_background_OPM,
		TACTICAL_X,
		TACTICAL_Y,
		TACTICAL_WIDTH,
		TACTICAL_HEIGHT,
		0,
		0
	);
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
	OPM_CopyOPMOPM
	(
		&Tactical_window_background_OPM,
		&Main_OPM,
		X - TACTICAL_X,
		Y - TACTICAL_Y,
		Width,
		Height,
		X,
		Y
	);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_tactical_square
 * FUNCTION  : Let the user select a tactical square.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 14:50
 * LAST      : 09.10.95 18:06
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
	/* Reset selected index */
	Selected_tactical_square_index = 0xFFFF;

	/* Set selection data */
	Tactical_select_message_nr			= Message_nr;
	Tactical_select_mask					= Mask1;
	Tactical_select_mask2				= Mask2;
	Current_tactical_display_handler	= Handler;

	/* Select */
	Push_module(&Tactical_select_Mod);

	/* Reset selection data */
	Current_tactical_display_handler	= NULL;
	Tactical_select_mask					= 0;
	Tactical_select_mask2				= 0;

	/* Redraw tactical window */
	Draw_tactical_window();

	return Selected_tactical_square_index;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Tactical_select_ModInit
 * FUNCTION  : Initialize tactical selection module.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.10.95 17:52
 * LAST      : 09.10.95 17:52
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Tactical_select_ModInit(void)
{
	static struct BBRECT MA = {
		TACTICAL_X + 8,
		TACTICAL_Y + 25,
		NR_TACTICAL_COLUMNS * TACTICAL_SQUARE_WIDTH,
		NR_TACTICAL_ROWS * TACTICAL_SQUARE_HEIGHT
	};

	/* Install mouse area */
	Push_MA(&MA);

	/* Print text */
	Set_permanent_message_nr(Tactical_select_message_nr);

	/* Enter selection mode */
	Tactical_select_mode = TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Tactical_select_ModExit
 * FUNCTION  : Exit tactical selection module.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.10.95 17:54
 * LAST      : 09.10.95 17:54
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Tactical_select_ModExit(void)
{
	/* Leave selection mode */
	Tactical_select_mode = FALSE;

	/* Remove mouse area */
	Pop_MA();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_potential_attack_targets
 * FUNCTION  : Show potential attack targets in the tactical window.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 12:52
 * LAST      : 15.09.95 18:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_potential_attack_targets(void)
{
	UNLONG T;
	UNSHORT X, Y;
	UNSHORT i, j;

	/* Blink ? */
	T = SYSTEM_GetTicks();
	if (T & (1 << 4))
	{
		/* Yes -> Do all rows */
		Y = TACTICAL_Y + 25;
		for (i=0;i<NR_TACTICAL_ROWS;i++)
		{
			/* Do all columns */
			X = TACTICAL_X + 8;
			for (j=0;j<NR_TACTICAL_COLUMNS;j++)
			{
				/* Is this square a potential target ? */
				if (Tactical_select_mask & (1 << (i * NR_TACTICAL_COLUMNS + j)))
				{
					/* Yes -> Indicate */
					OPM_Box
					(
						&Main_OPM,
						X,
						Y,
						TACTICAL_SQUARE_WIDTH,
						TACTICAL_SQUARE_HEIGHT,
						GREEN
					);
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
 * LAST      : 15.09.95 18:23
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_potential_move_targets(void)
{
	UNLONG T;
	UNSHORT X, Y;
	UNSHORT i, j;

	/* Blink ? */
	T = SYSTEM_GetTicks();
	if (T & (1 << 4))
	{
		/* Yes -> Do all rows */
		Y = TACTICAL_Y + 25;
		for (i=0;i<NR_TACTICAL_ROWS;i++)
		{
			/* Do all columns */
			X = TACTICAL_X + 8;
			for (j=0;j<NR_TACTICAL_COLUMNS;j++)
			{
				/* Is this square a potential movement target ? */
				if (Tactical_select_mask & (1 << (i * NR_TACTICAL_COLUMNS + j)))
				{
					/* Yes -> Indicate */
					OPM_Box
					(
						&Main_OPM,
						X,
						Y,
						TACTICAL_SQUARE_WIDTH,
						TACTICAL_SQUARE_HEIGHT,
						GREEN
					);
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
		Y = TACTICAL_Y + 25;
		for (i=0;i<NR_TACTICAL_ROWS;i++)
		{
			/* Do all columns */
			X = TACTICAL_X + 8;
			for (j=0;j<NR_TACTICAL_COLUMNS;j++)
			{
				/* Is this square occupied ? */
				if (Tactical_select_mask2 & (1 << (i * NR_TACTICAL_COLUMNS + j)))
				{
					/* Yes -> Indicate */
					OPM_Box
					(
						&Main_OPM,
						X,
						Y,
						TACTICAL_SQUARE_WIDTH,
						TACTICAL_SQUARE_HEIGHT,
						RED
					);
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
 * LAST      : 15.09.95 18:24
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_potential_row_targets(void)
{
	UNLONG T;
	UNSHORT X, Y;
	UNSHORT i, j;

	/* Blink ? */
	T = SYSTEM_GetTicks();
	if (T & (1 << 4))
	{
		/* Yes -> Do all rows */
		Y = TACTICAL_Y + 25;
		for (i=0;i<NR_TACTICAL_ROWS;i++)
		{
			/* Check all columns in this row */
			X = TACTICAL_X + 8;
			for (j=0;j<NR_TACTICAL_COLUMNS;j++)
			{
				/* Is this square a potential movement target ? */
				if (Tactical_select_mask & (1 << (i * NR_TACTICAL_COLUMNS + j)))
				{
					/* Yes -> Indicate this row */
					OPM_Box
					(
						&Main_OPM,
						X,
						Y,
						TACTICAL_SQUARE_WIDTH * NR_TACTICAL_COLUMNS,
						TACTICAL_SQUARE_HEIGHT,
						GREEN
					);

					break;
				}
			}
			/* Next row */
			Y += TACTICAL_SQUARE_HEIGHT;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_participant_targets
 * FUNCTION  : Show a participant's targets in the tactical window.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.95 18:35
 * LAST      : 01.10.95 19:29
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_participant_targets(void)
{
	UNSHORT Target_square_index;
	UNSHORT X, Y;

	/* Act depending on action type */
	switch (Highlight_part->Current_action)
	{
		/* Move */
		case MOVE_COMACT:
		{
			/* Get target square index */
			Target_square_index = Highlight_part->Target.Move_target_data.Target_square_indices[Highlight_part->Target.Move_target_data.Nr_moves - 1];

			/* Get target square coordinates */
			X = Target_square_index % NR_TACTICAL_COLUMNS;
			Y = Target_square_index / NR_TACTICAL_COLUMNS;

			/* Indicate target square */
			OPM_Box
			(
				&Main_OPM,
				TACTICAL_X + 8 + (X * TACTICAL_SQUARE_WIDTH),
				TACTICAL_Y + 25 + (Y * TACTICAL_SQUARE_HEIGHT),
				TACTICAL_SQUARE_WIDTH,
				TACTICAL_SQUARE_HEIGHT,
				GREEN
			);

			break;
		}
		/* Close- and long-range attack */
		case CLOSE_RANGE_COMACT:
		case LONG_RANGE_COMACT:
		{
			/* Get target square index */
			Target_square_index = Highlight_part->Target.Attack_target_data.Target_square_index;

			/* Get tactical coordinates of target square */
			X = Target_square_index % NR_TACTICAL_COLUMNS;
			Y = Target_square_index / NR_TACTICAL_COLUMNS;

			/* Indicate target square */
			OPM_Box
			(
				&Main_OPM,
				TACTICAL_X + 8 + (X * TACTICAL_SQUARE_WIDTH),
				TACTICAL_Y + 25 + (Y * TACTICAL_SQUARE_HEIGHT),
				TACTICAL_SQUARE_WIDTH,
				TACTICAL_SQUARE_HEIGHT,
				GREEN
			);

			break;
		}
		/* Magic */
		case CAST_SPELL_COMACT:
		case USE_MAGIC_ITEM_COMACT:
		{
			/* Show magic targets */
			Show_participant_magic_targets(Highlight_part);
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_participant_magic_targets
 * FUNCTION  : Show a participant's magic targets.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.95 18:36
 * LAST      : 15.09.95 18:36
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * NOTES     : - This function assumes the participant is actually going to
 *              cast a spell or use a magic item.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_participant_magic_targets(struct Combat_participant *Casting_part)
{
	struct Combat_participant *Part;
	UNLONG Combat_target_mask;
	UNSHORT X, Y;
	UNSHORT i, j;

	/* Get combat target mask */
	Combat_target_mask = Casting_part->Target.Magic_target_data.Combat_target_mask;

	/* Check combat matrix */
	Y = TACTICAL_Y + 25;
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		X = TACTICAL_X + 8;
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Is this square targeted ? */
			if (Combat_target_mask & (1 << (i * NR_TACTICAL_COLUMNS + j)))
			{
				/* Yes -> Get participant data */
				Part = Combat_matrix[i][j].Part;

				/* Act depending on current target mode */
				switch (Casting_part->Target.Magic_target_data.Combat_target_mode)
				{
					/* A participant */
					case PART_TARGMODE:
					{
						/* Anyone there ? */
						if (Part)
						{
							/* Yes -> Indicate target square */
							OPM_Box
							(
								&Main_OPM,
								X,
								Y,
								TACTICAL_SQUARE_WIDTH,
								TACTICAL_SQUARE_HEIGHT,
								GREEN
							);
						}
						break;
					}
					/* Friend */
					case FRIEND_TARGMODE:
					{
						/* Anyone there ? */
						if (Part)
						{
							/* Yes -> Is this participant a friend ? */
							if (Part->Type == Casting_part->Type)
							{
								/* Yes -> Indicate target square */
								OPM_Box
								(
									&Main_OPM,
									X,
									Y,
									TACTICAL_SQUARE_WIDTH,
									TACTICAL_SQUARE_HEIGHT,
									GREEN
								);
							}
						}
						break;
					}
					/* Enemy */
					case ENEMY_TARGMODE:
					{
						/* Anyone there ? */
						if (Part)
						{
							/* Yes -> Is this participant an enemy ? */
							if (Part->Type != Casting_part->Type)
							{
								/* Yes -> Indicate target square */
								OPM_Box
								(
									&Main_OPM,
									X,
									Y,
									TACTICAL_SQUARE_WIDTH,
									TACTICAL_SQUARE_HEIGHT,
									GREEN
								);
							}
						}
						break;
					}
					/* Square */
					case SQUARE_TARGMODE:
					{
						/* Indicate target square */
						OPM_Box
						(
							&Main_OPM,
							X,
							Y,
							TACTICAL_SQUARE_WIDTH,
							TACTICAL_SQUARE_HEIGHT,
							GREEN
						);
						break;
					}
				}
			}
			/* Next X */
			X += TACTICAL_SQUARE_WIDTH;
		}
		/* Next Y */
		Y += TACTICAL_SQUARE_HEIGHT;
	}
}

