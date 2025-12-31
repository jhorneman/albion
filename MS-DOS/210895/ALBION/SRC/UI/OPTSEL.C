/************
 * NAME     : OPTSEL.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 22-2-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : OPTSEL.H
 ************/

/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <GAMETEXT.H>
#include <OPTSEL.H>
#include <STATAREA.H>
#include <XFTYPES.H>

/* prototypes */

/* Option window methods */
UNLONG Init_Option_window_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Option_window_object(struct Object *Object, union Method_parms *P);

void Update_spell_list(struct Scroll_bar_object *Scroll_bar);

/* Option methods */
UNLONG Init_Option_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Option_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Option_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Option_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Option_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Option_object(struct Object *Object, union Method_parms *P);

void Draw_spell(struct Option_object *Option);

/* global variables */

/* Option window method list */
static struct Method Option_window_methods[] = {
	{ INIT_METHOD, Init_Option_window_object },
	{ EXIT_METHOD, Exit_Window_object },
	{ DRAW_METHOD, Draw_Option_window_object },
	{ UPDATE_METHOD, Update_help_line },
	{ RIGHT_METHOD, Close_Window_object },
	{ CLOSE_METHOD, Close_Window_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ RESTORE_METHOD, Restore_window },
	{ 0, NULL}
};

/* Option window class description */
static struct Object_class Option_window_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct Option_window_object),
	&Option_window_methods[0]
};

/* Option method list */
static struct Method Option_methods[] = {
	{ INIT_METHOD, Init_Option_object },
	{ DRAW_METHOD, Draw_Option_object },
	{ FEEDBACK_METHOD, Feedback_Option_object },
	{ HIGHLIGHT_METHOD, Highlight_Option_object },
	{ HELP_METHOD, Help_Option_object },
	{ LEFT_METHOD, Left_Option_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

/* Option class description */
static struct Object_class Option_Class = {
	0, sizeof(struct Option_object),
	&Option_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_spell
 * FUNCTION  : Let the user select a spell.
 * FILE      : OPTSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.04.95 11:03
 * LAST      : 28.04.95 20:06
 * INPUTS    : MEM_HANDLE Char_handle - Handle of casting character.
 * RESULT    : BOOLEAN : Option selected.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Select_spell(MEM_HANDLE Char_handle)
{
	struct Option_window_OID OID;
	struct Option_object_data Option_info[SPELL_CLASSES_MAX * SPELLS_PER_CLASS];
	UNSHORT Option_window_object;
	UNSHORT Selected_class = 0xFFFF;
	UNSHORT Selected_spell = 0xFFFF;
	UNSHORT Counter;
	UNSHORT Current_spell_area;
	UNSHORT SP;
	UNSHORT Required_LP;
	UNSHORT Required_SP;
	UNSHORT i, j;

	/* Get SP of character */
	SP = Get_SP(Char_handle);

	/* Get current spell area */
	Current_spell_area = Get_spell_area();

	/* Check all spell classes */
	Counter = 0;
	for (i=0;i<SPELL_CLASSES_MAX;i++)
	{
		/* Check all spells in this class */
		for (j=1;j<=Options_per_class[i];j++)
		{
			/* Is this spell known ? */
			if (Option_known(Char_handle, i, j))
			{
				/* Yes -> Clear spell OID */
				BASEMEM_FillMemByte((UNBYTE *) &Option_info[Counter],
				 sizeof(struct Option_object_data), 0);

				/* Insert class and spell number */
				Option_info[Counter].Class_nr = i;
				Option_info[Counter].Option_nr = j;

				/* Get spell strength */
				Option_info[Counter].Strength =
				 Get_spell_strength(Char_handle, i, j) / 100;

				/* Can this spell be cast in the current area ? */
				if (Get_spell_areas(i, j) & (1 << Current_spell_area))
				{
					/* Yes -> Indicate this */
					Option_info[Counter].Status = SPELL_OK;

					/* Get LP & SP required for this spell */
					Required_LP = Calculate_required_LP(Char_handle, i, j);
					Required_SP = Get_spell_required_SP(i, j);

					/* Too much LP required ? */
					if (Required_LP == 0xFFFF)
					{
						/* Yes -> Indicate this */
						Option_info[Counter].Status = SPELL_TOO_MUCH_LP;
					}
					else
					{
						/* No -> Any LP required ? */
						if (Required_LP)
						{
							/* Yes -> Indicate this */
							Option_info[Counter].Status = SPELL_TOO_MUCH_SP;

							/* Option can only be cast once */
							Option_info[Counter].Quantity = 1;
						}
						else
						{
							/* No -> Calculate how often this spell can be cast */
							if (Required_SP)
							{
								Option_info[Counter].Quantity = SP / Required_SP;
							}
							else
							{
								Option_info[Counter].Quantity = 1;
							}
						}
					}
				}
				else
				{
					/* No -> Indicate this */
					Option_info[Counter].Status = SPELL_WRONG_AREA;
				}

				/* Count up */
				Counter++;
			}
		}
	}

	/* Any spells known ? */
	if (Counter)
	{
		/* Yes -> Initialize spell window OID */
		OID.Char_handle = Char_handle;
		OID.Option_info = &(Option_info[0]);
		OID.Selected_class_ptr = &Selected_class;
		OID.Selected_spell_ptr = &Selected_spell;
		OID.Nr_spells = Counter;

		/* Do spell window */
		Push_module(&Window_Mod);

		Option_window_object = Add_object(0, &Option_window_Class,
		 (UNBYTE *) &OID, 50, 50, 200, 119);

		Execute_method(Option_window_object, DRAW_METHOD, NULL);

		Wait_4_object(Option_window_object);
	}

	/* Copy selected spell */
	Current_use_magic_data.Class_nr = Selected_class;
	Current_use_magic_data.Option_nr = Selected_spell;

	/* Any spell selected ? */
	return((Selected_class != 0xFFFF) && (Selected_spell != 0xFFFF));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Option_window_object
 * FUNCTION  : Init method of Option Window object.
 * FILE      : OPTSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 11:55
 * LAST      : 28.04.95 20:06
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Option_window_object(struct Object *Object, union Method_parms *P)
{
	struct Option_window_object *Option_window;
	struct Option_window_OID *OID;
	struct Option_OID Option_OID;
	struct Scroll_bar_OID Scroll_bar_OID;
	SISHORT Y;
	UNSHORT i;

	Option_window = (struct Option_window_object *) Object;
	OID = (struct Option_window_OID *) P;

	/* Copy data from OID */
	Option_window->Char_handle = OID->Char_handle;
	Option_window->Option_info = OID->Option_info;
	Option_window->Selected_class_ptr = OID->Selected_class_ptr;
	Option_window->Selected_spell_ptr = OID->Selected_spell_ptr;
	Option_window->Nr_spells = OID->Nr_spells;

	/* Scroll bar needed ? */
	if (Option_window->Nr_spells > MAX_SPELLS_IN_WINDOW)
	{
		/* Yes -> Calculate width and height WITH scroll bar */
		Change_object_size(Object->Self, SPELL_WIDTH + 27 + BETWEEN +
		 SCROLL_BAR_WIDTH, (MAX_SPELLS_IN_WINDOW * (SPELL_HEIGHT + 1)) + 26);

		/* Centre window */
		Change_object_position(Object->Self, (Screen_width -
		 Object->Rect.width) / 2, (Panel_Y - Object->Rect.height) / 2);

		/* Make scroll bar */
		Scroll_bar_OID.Total_units = Option_window->Nr_spells;
		Scroll_bar_OID.Units_width = 1;
		Scroll_bar_OID.Units_height = MAX_SPELLS_IN_WINDOW;
		Scroll_bar_OID.Update = Update_spell_list;

		/* Add object */
		Option_window->Scroll_bar_object = Add_object(Object->Self,
		 &Scroll_bar_Class, (UNBYTE *) &Scroll_bar_OID, SPELL_WIDTH + 13 +
		 BETWEEN, 12, SCROLL_BAR_WIDTH, (MAX_SPELLS_IN_WINDOW *
		 (SPELL_HEIGHT + 1)) - 1);

		/* Make spell objects */
		Y = 12;
	   Option_OID.Number = 0;
		for (i=0;i<MAX_SPELLS_IN_WINDOW;i++)
		{
			/* Add object */
			Add_object(Object->Self, &Option_Class, (UNBYTE *) &Option_OID, 13, Y,
			 SPELL_WIDTH, SPELL_HEIGHT);

			/* Increase Y-coordinate */
			Y += SPELL_HEIGHT + 1;

			/* Count up */
			Option_OID.Number++;
		}
	}
	else
	{
		/* No -> Calculate width and height WITHOUT scroll bar */
		Change_object_size(Object->Self, SPELL_WIDTH + 27,
		 (Option_window->Nr_spells * (SPELL_HEIGHT + 1)) + 26);

		/* Centre window */
		Change_object_position(Object->Self, (Screen_width -
		 Object->Rect.width) / 2, (Panel_Y - Object->Rect.height) / 2);

		/* Make spell objects */
		Y = 12;
	   Option_OID.Number = 0;
		for (i=0;i<Option_window->Nr_spells;i++)
		{
			/* Add object */
			Add_object(Object->Self, &Option_Class, (UNBYTE *) &Option_OID, 13, Y,
			 SPELL_WIDTH, SPELL_HEIGHT);

			/* Increase Y-coordinate */
			Y += SPELL_HEIGHT + 1;

			/* Count up */
			Option_OID.Number++;
		}

		/* Indicate there is no scroll bar */
		Option_window->Scroll_bar_object = 0;
	}

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Option_window_object
 * FUNCTION  : Draw method of Option Window object.
 * FILE      : OPTSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 11:55
 * LAST      : 28.06.95 18:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Option_window_object(struct Object *Object, union Method_parms *P)
{
	struct Option_window_object *Option_window;
	struct OPM *OPM;
	UNSHORT W, H;

	Option_window = (struct Option_window_object *) Object;
	OPM = &(Option_window->Window_OPM);

	/* Get window dimensions */
	W = Object->Rect.width;
	H = Object->Rect.height;

	/* Draw window's shadow */
	Put_recoloured_box(OPM, 10, 10, W - 10, H - 10, &(Recolour_tables[0][0]));

	/* Draw window */
	Draw_window_inside(OPM, 7, 7, W - 14, H - 14);
	Draw_window_border(OPM, 0, 0, W, H);

	/* Draw border around spells */
	Draw_deep_border(OPM, 12, 11, SPELL_WIDTH + 2, H - 25);

	/* Draw spells */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_spell_list
 * FUNCTION  : Update the spell list (scroll bar function).
 * FILE      : OPTSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 18:00
 * LAST      : 04.04.95 18:00
 * INPUTS    : struct Scroll_bar_object *Scroll_bar - Pointer to scroll bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_spell_list(struct Scroll_bar_object *Scroll_bar)
{
	struct Object *Parent;
	UNSHORT Child;

	/* Get parent object data */
	Parent = Get_object_data(Scroll_bar->Object.Parent);

	/* Draw all child objects except scroll bar */
	Child = Parent->Child;
	while (Child)
	{
		/* Is scroll bar ? */
		if (Child != Scroll_bar->Object.Self)
		{
			/* No -> Draw */
			Execute_method(Child, DRAW_METHOD, NULL);
		}

		/* Next brother */
		Child = (Get_object_data(Child))->Next;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Option_object
 * FUNCTION  : Init method of Option object.
 * FILE      : OPTSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:53
 * LAST      : 04.04.95 10:53
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Option_object(struct Object *Object, union Method_parms *P)
{
	struct Option_object *Option;
	struct Option_OID *OID;

	Option = (struct Option_object *) Object;
	OID = (struct Option_OID *) P;

	/* Copy data from OID */
	Option->Number = OID->Number;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Option_object
 * FUNCTION  : Draw method of Option object.
 * FILE      : OPTSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:54
 * LAST      : 04.04.95 10:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Option_object(struct Object *Object, union Method_parms *P)
{
	struct Option_object *Option;
	struct Option_window_object *Option_window;
	struct OPM *OPM;

	Option = (struct Option_object *) Object;
	Option_window = (struct Option_window_object *) Get_object_data(Object->Parent);
	OPM = &(Option_window->Window_OPM);

	/* Clear spell area */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw spell */
	Draw_spell(Option);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Option_object
 * FUNCTION  : Feedback method of Option object.
 * FILE      : OPTSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:54
 * LAST      : 04.04.95 10:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_Option_object(struct Object *Object, union Method_parms *P)
{
	struct Option_object *Option;
	struct Option_window_object *Option_window;
	struct OPM *OPM;

	Option = (struct Option_object *) Object;
	Option_window = (struct Option_window_object *) Get_object_data(Object->Parent);
	OPM = &(Option_window->Window_OPM);

	/* Clear spell area */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Draw_deep_box(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw spell */
	Draw_spell(Option);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Option_object
 * FUNCTION  : Highlight method of Option object.
 * FILE      : OPTSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:54
 * LAST      : 04.04.95 10:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Option_object(struct Object *Object, union Method_parms *P)
{
	struct Option_object *Option;
	struct Option_window_object *Option_window;
	struct OPM *OPM;

	Option = (struct Option_object *) Object;
	Option_window = (struct Option_window_object *) Get_object_data(Object->Parent);
	OPM = &(Option_window->Window_OPM);

	/* Clear spell area */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Put_recoloured_box(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height, &(Recolour_tables[6][0]));

	/* Draw spell */
	Draw_spell(Option);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Option_object
 * FUNCTION  : Help method of Option object.
 * FILE      : OPTSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:55
 * LAST      : 04.04.95 10:55
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Option_object(struct Object *Object, union Method_parms *P)
{
	struct Option_object *Option;
	struct Option_window_object *Option_window;
	struct Option_object_data *Option_data;
	UNSHORT Index;
	UNCHAR String[100];

	Option = (struct Option_object *) Object;
	Option_window = (struct Option_window_object *) Get_object_data(Object->Parent);

	/* Get spell data index */
	Index = Option->Number;
	if (Option_window->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method(Option_window->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Get spell data */
	Option_data = Option_window->Option_info + Index;

	/* Does this spell cost LP ? */
	if (Option_data->Status == SPELL_TOO_MUCH_SP)
	{
		/* Yes-> Make special help line string */
		sprintf(String, System_text_ptrs[427],
		 Get_spell_required_SP(Option_data->Class_nr, Option_data->Option_nr),
		 Calculate_required_LP(Option_window->Char_handle, Option_data->Class_nr,
 		 Option_data->Option_nr));
	}
	else
	{
		/* No -> Make normal help line string */
		sprintf(String, System_text_ptrs[424],
		 Get_spell_required_SP(Option_data->Class_nr, Option_data->Option_nr));
	}

	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) String);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Option_object
 * FUNCTION  : Left method of Option object.
 * FILE      : OPTSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:55
 * LAST      : 28.04.95 20:06
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Option_object(struct Object *Object, union Method_parms *P)
{
	struct Option_object *Option;
	struct Option_window_object *Option_window;
	struct Option_object_data *Option_data;
	UNSHORT Index;

	Option = (struct Option_object *) Object;
	Option_window = (struct Option_window_object *) Get_object_data(Object->Parent);

	/* Get spell data index */
	Index = Option->Number;
	if (Option_window->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method(Option_window->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Get spell data */
	Option_data = Option_window->Option_info + Index;

	/* Is this spell for another area ? */
	if (Option_data->Status == SPELL_WRONG_AREA)
	{
		/* Yes -> Print message */
		Execute_method(Help_line_object, SET_METHOD,
		 (void *) System_text_ptrs[423]);
	}

	/* Is this spell for another area ? */
	if (Option_data->Status == SPELL_TOO_MUCH_LP)
	{
		/* Yes -> Print message */
		Execute_method(Help_line_object, SET_METHOD,
		 (void *) System_text_ptrs[426]);
	}

	/* Clicked ? */
	if (Normal_clicked(Object))
	{
		/* Is this spell for another area ? */
		if ((Option_data->Status != SPELL_WRONG_AREA) &&
		 (Option_data->Status != SPELL_TOO_MUCH_LP))
		{
			/* No -> Select this spell */
			*(Option_window->Selected_class_ptr) = Option_data->Class_nr;
			*(Option_window->Selected_spell_ptr) = Option_data->Option_nr;

			/* Close spell selection window */
			Execute_method(Option_window->Object.Self, CLOSE_METHOD, NULL);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Option
 * FUNCTION  : Draw Option object.
 * FILE      : OPTSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 16:32
 * LAST      : 04.04.95 16:32
 * INPUTS    : struct Option_object *Option - Pointer to spell object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_spell(struct Option_object *Option)
{
	struct BBRECT Clip, Old;
	struct Object *Object;
	struct Option_window_object *Option_window;
	struct Option_object_data *Option_data;
	struct OPM *OPM;
	UNSHORT Index;
	UNCHAR Number[4];

	Object = &(Option->Object);
	Option_window = (struct Option_window_object *) Get_object_data(Object->Parent);
	OPM = &(Option_window->Window_OPM);

	/* Get spell data index */
	Index = Option->Number;
	if (Option_window->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method(Option_window->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Get spell data */
	Option_data = Option_window->Option_info + Index;

	/* Select ink colour depending on spell state */
	if (Option_data->Status == SPELL_WRONG_AREA)
		Set_ink(RED_TEXT);
	else
		Set_ink(SILVER_TEXT);

	/* Print spell name */
	Print_string(OPM, Object->X + 2, Object->Y + 2,
	 Get_spell_name(Option_data->Class_nr, Option_data->Option_nr));

	/* Draw border around spell strength bar */
	Draw_deep_border(OPM, Object->X + 121, Object->Y + 3, 52, 6);

	/* Save current clip area */
	memcpy(&Old, &(OPM->clip), sizeof(struct BBRECT));

	/* Prepare new clip area */
	Clip.left = Object->X + 122;
	Clip.top = Object->Y + 4;
	Clip.height = 4;

	/* Install new clip area */
	Clip.width = Option_data->Strength / 2;
	memcpy(&(OPM->clip), &Clip, sizeof(struct BBRECT));

	/* Display full spell strength bar */
	Put_masked_block(OPM, Object->X + 122, Object->Y + 4, 50, 4,
	 &Full_spell_strength_bar[0]);

	/* Install new clip area */
	Clip.left = Object->X + 122 + Option_data->Strength / 2;
	Clip.width = 50 - Option_data->Strength / 2;
	memcpy(&(OPM->clip), &Clip, sizeof(struct BBRECT));

	/* Display empty spell strength bar */
	Put_masked_block(OPM, Object->X + 122, Object->Y + 4, 50, 4,
	 &Empty_spell_strength_bar[0]);

	/* Restore old clip area */
	memcpy(&(OPM->clip), &Old, sizeof(struct BBRECT));

	/* Can the spell be cast at all ? */
	if (Option_data->Status != SPELL_WRONG_AREA)
	{
		/* Yes -> Select ink colour depending on spell state */
		if (Option_data->Status == SPELL_OK)
			Set_ink(SILVER_TEXT);
		else
			Set_ink(RED_TEXT);

		/* Display the number of times the spell can be cast */
		sprintf(Number, "%u", min(Option_data->Quantity, 999));
		Print_centered_string(OPM, Object->X + 174, Object->Y + 2, 16, Number);
	}
}

