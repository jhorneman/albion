/************
 * NAME     : INVLEFT.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 9-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : INVLEFT.H
 ************/

/* pragmas */

#pragma off (unreferenced);

/* includes */

#include <stdlib.h>
#include <stdio.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <ALBION.H>

#include <TEXT.H>
#include <GFXFUNC.H>
#include <FINDCOL.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GAMETEXT.H>
#include <PRTLOGIC.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <POPUP.H>
#include <INPUT.H>
#include <SCROLBAR.H>
#include <INVENTO.H>
#include <INVLEFT.H>
#include <INVITEMS.H>
#include <ITEMLIST.H>
#include <BUTTONS.H>
#include <INPUT.H>
#include <EVELOGIC.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <COMBAT.H>
#include <MAGIC.H>
#include <COLOURS.H>
#include <BATFORM.H>

/* defines */

#define VALUE_BAR_HEIGHT	(6)

/* type definitions */

typedef void (*Get_value_handler) (SILONG *Value_ptr, SILONG *Maximum_ptr, UNSHORT Value_index);

/* structure definitions */

/* Inventory Left 1 object */
struct InvLeft1_object {
	struct Object Object;

	SISHORT Level;
	SISHORT LP;
	SISHORT LP_max;
	SISHORT SP;
	SISHORT SP_max;
	SILONG EP;
	SISHORT TP;
};

/* Inventory Left 3 object */
struct InvLeft3_object {
	struct Object Object;

	UNSHORT Conditions;
	UNSHORT Known_languages;
	struct Temporary_spell_data Temp_spells[MAX_TEMP_SPELLS];
};

/* Value bar object OID */
struct Value_bar_OID {
	UNSHORT Help_message_nr;
	Get_value_handler Value_getter;
	UNSHORT Value_index;
	UNLONG Absolute_maximum;
};

/* Value bar object */
struct Value_bar_object {
	struct Object Object;

	UNSHORT Help_message_nr;
	Get_value_handler Value_getter;
	UNSHORT Value_index;
	SILONG Absolute_maximum;

	SILONG Value;
	SILONG Maximum;
};

/* prototypes */

/* Left inventory methods */
UNLONG Init_InvLeft_object(struct Object *Object, union Method_parms *P);

UNLONG Init_InvLeft1_object(struct Object *Object, union Method_parms *P);
UNLONG Update_InvLeft1_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_InvLeft1_object(struct Object *Object, union Method_parms *P);

UNLONG Init_InvLeft2_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_InvLeft2_object(struct Object *Object, union Method_parms *P);

UNLONG Init_InvLeft3_object(struct Object *Object, union Method_parms *P);
UNLONG Update_InvLeft3_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_InvLeft3_object(struct Object *Object, union Method_parms *P);

/* Value bar methods */
UNLONG Init_Value_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Value_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Value_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Value_bar_object(struct Object *Object, union Method_parms *P);

void Attribute_value_getter(SILONG *Value_ptr, SILONG *Maximum_ptr,
 UNSHORT Value_index);
void Skill_value_getter(SILONG *Value_ptr, SILONG *Maximum_ptr,
 UNSHORT Value_index);

/* global variables */

/* Left inventory 1 object method list */
static struct Method InvLeft1_methods[] = {
	{ INIT_METHOD,		Init_InvLeft1_object },
	{ UPDATE_METHOD,	Update_InvLeft1_object },
	{ DRAW_METHOD,		Draw_InvLeft1_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ 0, NULL}
};

/* Left inventory 1 object class description */
struct Object_class InvLeft1_Class = {
	0, sizeof(struct InvLeft1_object),
	&InvLeft1_methods[0]
};

/* Left inventory 2 object method list */
static struct Method InvLeft2_methods[] = {
	{ INIT_METHOD,		Init_InvLeft2_object },
	{ DRAW_METHOD,		Draw_InvLeft2_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ 0, NULL}
};

/* Left inventory 2 object class description */
struct Object_class InvLeft2_Class = {
	0, sizeof(struct Object),
	&InvLeft2_methods[0]
};

/* Left inventory 3 object method list */
static struct Method InvLeft3_methods[] = {
	{ INIT_METHOD,		Init_InvLeft3_object },
	{ UPDATE_METHOD,	Update_InvLeft3_object },
	{ DRAW_METHOD,		Draw_InvLeft3_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ 0, NULL}
};

/* Left inventory 3 object description */
struct Object_class InvLeft3_Class = {
	0, sizeof(struct InvLeft3_object),
	&InvLeft3_methods[0]
};

/* Value bar object method list */
static struct Method Value_bar_methods[] = {
	{ INIT_METHOD,		Init_Value_bar_object },
	{ DRAW_METHOD,		Draw_Value_bar_object },
	{ UPDATE_METHOD,	Update_Value_bar_object },
	{ HELP_METHOD,		Help_Value_bar_object },
	{ TOUCHED_METHOD,	Normal_touched },
	{ 0, NULL}
};

/* Value bar object description */
static struct Object_class Value_bar_Class = {
	0, sizeof(struct Value_bar_object),
	&Value_bar_methods[0]
};

/* Battle formation button data */
static union Button_data Battle_formation_button_data;
static struct Button_OID Battle_formation_button_OID = {
	TEXT_BUTTON_TYPE,
	1,
	666,
	&Battle_formation_button_data,
	Go_Battle_formation
};

/* Left inventory radio buttons data */
static union Button_data Left_inventory_button_data[3];
static struct Button_OID Left_inventory_button_OID[3] = {
	{
	  	TEXT_BUTTON_TYPE | RADIO_BUTTON,
		1,
		54,
		&(Left_inventory_button_data[0]),
		Switch_inventory_mode
	},
	{
	  	TEXT_BUTTON_TYPE | RADIO_BUTTON,
		2,
		55,
		&(Left_inventory_button_data[1]),
		Switch_inventory_mode
	},
	{
	  	TEXT_BUTTON_TYPE | RADIO_BUTTON,
		3,
		56,
		&(Left_inventory_button_data[2]),
		Switch_inventory_mode
	}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_InvLeft_object
 * FUNCTION  : Initialize method of Left Inventory object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.09.95 15:39
 * LAST      : 10.09.95 20:09
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_InvLeft_object(struct Object *Object, union Method_parms *P)
{
	union Method_parms P2;
	UNSHORT Obj;

	/* Add radio group object */
	Obj = Add_object
	(
		Object->Self,
		&Radio_group_Class,
		NULL,
		85,
		Panel_Y - 17,
		48,
		13
	);

	/* Initialize radio button data */
	Left_inventory_button_data[0].Text_button_data.Text = System_text_ptrs[57];
 	Left_inventory_button_data[1].Text_button_data.Text = System_text_ptrs[58];
	Left_inventory_button_data[2].Text_button_data.Text = System_text_ptrs[59];

	/* Add radio buttons to group */
	Add_object
	(
		Obj,
		&Button_Class,
		(UNBYTE *) &Left_inventory_button_OID[0],
		0,
		0,
		14,
		13
	);

	Add_object
	(
		Obj,
		&Button_Class,
		(UNBYTE *) &Left_inventory_button_OID[1],
		17,
		0,
		14,
		13
	);

	Add_object
	(
		Obj,
		&Button_Class,
		(UNBYTE *) &Left_inventory_button_OID[2],
		34,
		0,
		14,
		13
	);

	/* Set radio button */
	P2.Value = Inventory_mode;
	Execute_method
	(
		Obj,
		SET_METHOD,
		&P2
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_InvLeft1_object
 * FUNCTION  : Initialize method of Left Inventory 1 object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.09.95 23:28
 * LAST      : 28.09.95 23:28
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_InvLeft1_object(struct Object *Object, union Method_parms *P)
{
	struct InvLeft1_object *InvLeft1;

	/* Initialize left inventory */
	Init_InvLeft_object(Object, P);

	InvLeft1 = (struct InvLeft1_object *) Object;

	/* Get character data */
	InvLeft1->Level	= Get_level(Inventory_char_handle);
	InvLeft1->LP		= Get_LP(Inventory_char_handle);
	InvLeft1->LP_max	= Get_max_LP(Inventory_char_handle);
	InvLeft1->SP		= Get_SP(Inventory_char_handle);
	InvLeft1->SP_max	= Get_max_SP(Inventory_char_handle);
	InvLeft1->EP		= Get_EP(Inventory_char_handle);
	InvLeft1->TP		= Get_TP(Inventory_char_handle);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_InvLeft1_object
 * FUNCTION  : Update method of Left Inventory 1 object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.09.95 23:30
 * LAST      : 28.09.95 23:30
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_InvLeft1_object(struct Object *Object, union Method_parms *P)
{
	struct InvLeft1_object *InvLeft1;

	InvLeft1 = (struct InvLeft1_object *) Object;

	/* Have any character data changed ? */
	if ((InvLeft1->Level	!= Get_level(Inventory_char_handle)) ||
	 (InvLeft1->LP != Get_LP(Inventory_char_handle)) ||
	 (InvLeft1->LP_max != Get_max_LP(Inventory_char_handle)) ||
	 (InvLeft1->SP	!= Get_SP(Inventory_char_handle)) ||
	 (InvLeft1->SP_max != Get_max_SP(Inventory_char_handle)) ||
	 (InvLeft1->EP	!= Get_EP(Inventory_char_handle)) ||
	 (InvLeft1->TP	!= Get_TP(Inventory_char_handle)))
	{
		/* Yes -> Get character data again */
		InvLeft1->Level	= Get_level(Inventory_char_handle);
		InvLeft1->LP		= Get_LP(Inventory_char_handle);
		InvLeft1->LP_max	= Get_max_LP(Inventory_char_handle);
		InvLeft1->SP		= Get_SP(Inventory_char_handle);
		InvLeft1->SP_max	= Get_max_SP(Inventory_char_handle);
		InvLeft1->EP		= Get_EP(Inventory_char_handle);
		InvLeft1->TP		= Get_TP(Inventory_char_handle);

		/* Redraw */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_InvLeft1_object
 * FUNCTION  : Draw method of Left Inventory 1 object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 16:42
 * LAST      : 14.09.95 20:23
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_InvLeft1_object(struct Object *Object, union Method_parms *P)
{
	static struct PA InvLeft1_PA = {
		6,
		6,
		INVENTORY_MIDDLE - 12,
		40
	};

	UNCHAR String[200];
	UNBYTE *Ptr;

	/* Clear left inventory */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Main_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height,
		Object->X,
		Object->Y
	);

	/* Draw enlarged portrait */
	Ptr = MEM_Claim_pointer(Small_portrait_handles[Inventory_member - 1]);

	Draw_enlarged_block
	(
		&Main_OPM,
		0,
		22,
		PORTRAIT_WIDTH,
		PORTRAIT_HEIGHT,
		Ptr,
		&Palette
	);

	MEM_Free_pointer(Small_portrait_handles[Inventory_member - 1]);

	/* Set printing ink */
	Set_ink(SILVER_TEXT);

	/* Draw box surrounding info string */
	Draw_deep_box
	(
		&Main_OPM,
		4,
		4,
		INVENTORY_MIDDLE - 8,
		43
	);

	/* Build character description string */
	_bprintf
	(
		String,
		200,
		System_text_ptrs[632],
		Get_attribute(Inventory_char_handle, AGE),
		Get_level(Inventory_char_handle)
	);

	/* Display character description */
	Push_PA(&InvLeft1_PA);
	Display_text(&Main_OPM, String);
	Pop_PA();

	/* Draw box surrounding stats */
	Draw_deep_box
	(
		&Main_OPM,
		4,
		Panel_Y - 48 - 17,
		INVENTORY_MIDDLE - 8,
		43
	);

	/* Print LP description */
	Print_right_string
	(
		&Main_OPM,
		6,
		Panel_Y - 46 - 17,
		77,
		System_text_ptrs[633]
	);

	/* Build LP string */
	_bprintf
	(
		String,
		200,
		"%d/%d",
		Get_LP(Inventory_char_handle),
		Get_max_LP(Inventory_char_handle)
	);

	/* Print LP string */
	Print_string
	(
		&Main_OPM,
		89,
		Panel_Y - 46 - 17,
		String
	);

	/* Does the character have magical abilities ? */
	if (Character_has_magical_abilities(Inventory_char_handle))
	{
		/* Yes -> Print SP description */
		Print_right_string
		(
			&Main_OPM,
			6,
			Panel_Y - 36 - 17,
			77,
			System_text_ptrs[634]
		);

		/* Build SP string */
		_bprintf
		(
			String,
			200,
			"%d/%d",
			Get_SP(Inventory_char_handle),
			Get_max_SP(Inventory_char_handle)
		);

		/* Print SP string */
		Print_string
		(
			&Main_OPM,
			89,
			Panel_Y - 36 - 17,
			String
		);
	}

	/* Print EP description */
	Print_right_string
	(
		&Main_OPM,
		6,
		Panel_Y - 26 - 17,
		77,
		System_text_ptrs[667]
	);

	/* Build EP string */
	_bprintf
	(
		String,
		200,
		"%ld",
		Get_EP(Inventory_char_handle)
	);

	/* Print EP string */
	Print_string
	(
		&Main_OPM,
		89,
		Panel_Y - 26 - 17,
		String
	);

	/* Print TP description */
	Print_right_string
	(
		&Main_OPM,
		6,
		Panel_Y - 16 - 17,
		77,
		System_text_ptrs[668]
	);

	/* Build TP string */
	_bprintf
	(
		String,
		200,
		"%d",
		Get_TP(Inventory_char_handle)
	);

	/* Print TP string */
	Print_string
	(
		&Main_OPM,
		89,
		Panel_Y - 16 - 17,
		String
	);

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_InvLeft2_object
 * FUNCTION  : Initialize method of Left Inventory 2 object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 10:02
 * LAST      : 09.09.95 15:40
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_InvLeft2_object(struct Object *Object, union Method_parms *P)
{
	struct Value_bar_OID OID;
	UNSHORT Y;
	UNSHORT i;

	/* Initialize left inventory */
	Init_InvLeft_object(Object, P);

	/* Add attribute value bars
	  (one less because age is a special attribute) */
	Y = 12;
	for (i=0;i<MAX_ATTRS - 1;i++)
	{
		/* Build attribute value OID */
		OID.Help_message_nr	= 635 + i;
		OID.Value_getter		= Attribute_value_getter;
		OID.Value_index		= i;
		OID.Absolute_maximum	= 100;

		/* Add attribute value bar */
		Add_object
		(
			Object->Self,
			&Value_bar_Class,
			(UNBYTE *) &OID,
			31,
			Y,
			100,
			VALUE_BAR_HEIGHT
		);

		/* Increase Y-coordinate */
		Y += 10;
	}

	/* Add skill value bars */
	Y = 104;
	for (i=0;i<MAX_SKILLS;i++)
	{
		/* Build skill value OID */
		OID.Help_message_nr	= 643 + i;
		OID.Value_getter		= Skill_value_getter;
		OID.Value_index		= i;
		OID.Absolute_maximum	= 100;

		/* Add skill value bar */
		Add_object
		(
			Object->Self,
			&Value_bar_Class,
			(UNBYTE *) &OID,
			31,
			Y,
			100,
			VALUE_BAR_HEIGHT
		);

		/* Increase Y-coordinate */
		Y += 10;
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_InvLeft2_object
 * FUNCTION  : Draw method of Left Inventory 2 object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 10:02
 * LAST      : 09.09.95 20:59
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_InvLeft2_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT Y;
	UNSHORT i;

	/* Clear left inventory */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Main_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height,
		Object->X,
		Object->Y
	);

	Set_ink(SILVER_TEXT);

	/* Print attributes description */
	Print_centered_line_string
	(
		&Main_OPM,
		4,
		1,
		INVENTORY_MIDDLE - 8,
		System_text_ptrs[647]
	);

	/* Print attribute abbreviations
	  (one less because age is a special attribute) */
	Y = 11;
	for (i=0;i<MAX_ATTRS - 1;i++)
	{
		Print_right_string
		(
			&Main_OPM,
			0,
			Y,
			27,
			System_text_ptrs[649 + i]
		);

		/* Increase Y-coordinate */
		Y += 10;
	}

	/* Print skills description */
	Print_centered_line_string
	(
		&Main_OPM,
		4,
		93,
		INVENTORY_MIDDLE - 8,
		System_text_ptrs[648]
	);

	/* Print skill abbreviations */
	Y = 103;
	for (i=0;i<MAX_SKILLS;i++)
	{
		Print_right_string
		(
			&Main_OPM,
			0,
			Y,
			27,
			System_text_ptrs[657 + i]
		);

		/* Increase Y-coordinate */
		Y += 10;
	}

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_InvLeft3_object
 * FUNCTION  : Init method of Left Inventory 3 object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 20:08
 * LAST      : 10.09.95 20:08
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_InvLeft3_object(struct Object *Object, union Method_parms *P)
{
	struct InvLeft3_object *InvLeft3;
	UNSHORT i;

	/* Initialize left inventory */
	Init_InvLeft_object(Object, P);

	InvLeft3 = (struct InvLeft3_object *) Object;

	/* Get character data */
	InvLeft3->Conditions			= Get_conditions(Inventory_char_handle);
	InvLeft3->Known_languages	= Get_known_languages(Inventory_char_handle);

	for (i=0;i<(UNSHORT) MAX_TEMP_SPELLS;i++)
	{
		InvLeft3->Temp_spells[i].Duration = Get_member_temporary_spell_duration
		(
			Inventory_member,
			(TEMP_SPELL_TYPE_T) i
		);

		InvLeft3->Temp_spells[i].Strength = Get_member_temporary_spell_strength
		(
			Inventory_member,
			(TEMP_SPELL_TYPE_T) i
		);
	}

	/* Initialize battle formation button data */
	Battle_formation_button_data.Text_button_data.Text = System_text_ptrs[13];

	/* Add battle formation button */
	Add_object
	(
		Object->Self,
		&Button_Class,
		(UNBYTE *) &Battle_formation_button_OID,
		4,
		Panel_Y - 33,
		INVENTORY_MIDDLE - 8,
		13
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_InvLeft3_object
 * FUNCTION  : Update method of Left Inventory 3 object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.09.95 10:40
 * LAST      : 29.09.95 10:40
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_InvLeft3_object(struct Object *Object, union Method_parms *P)
{
	struct InvLeft3_object *InvLeft3;
	BOOLEAN Changed;
	UNSHORT i;

	InvLeft3 = (struct InvLeft3_object *) Object;

	/* Have any temporary spells changed ? */
	Changed = FALSE;
	for (i=0;i<(UNSHORT) MAX_TEMP_SPELLS;i++)
	{
		if ((InvLeft3->Temp_spells[i].Duration !=
		 Get_member_temporary_spell_duration
		(
			Inventory_member,
			(TEMP_SPELL_TYPE_T) i
		)) ||
		(InvLeft3->Temp_spells[i].Strength !=
		 Get_member_temporary_spell_strength
		(
			Inventory_member,
			(TEMP_SPELL_TYPE_T) i
		)))
		{
			Changed = TRUE;
			break;
		}
	}

	/* Have any character data changed ? */
	if ((InvLeft3->Conditions != Get_conditions(Inventory_char_handle)) ||
	 (InvLeft3->Known_languages != Get_known_languages(Inventory_char_handle)) ||
	 Changed)
	{
		/* Yes -> Get character data again */
		InvLeft3->Conditions			= Get_conditions(Inventory_char_handle);
		InvLeft3->Known_languages	= Get_known_languages(Inventory_char_handle);

		for (i=0;i<(UNSHORT) MAX_TEMP_SPELLS;i++)
		{
			InvLeft3->Temp_spells[i].Duration = Get_member_temporary_spell_duration
			(
				Inventory_member,
				(TEMP_SPELL_TYPE_T) i
			);

			InvLeft3->Temp_spells[i].Strength = Get_member_temporary_spell_strength
			(
				Inventory_member,
				(TEMP_SPELL_TYPE_T) i
			);
		}

		/* Redraw */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_InvLeft3_object
 * FUNCTION  : Draw method of Left Inventory 3 object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 10:02
 * LAST      : 10.09.95 15:29
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_InvLeft3_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT Conditions;
	UNSHORT Known_languages;
	SISHORT Duration;
	SISHORT Strength;
	UNSHORT X, Y;
	UNSHORT i;

	/* Clear left inventory */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Main_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height,
		Object->X,
		Object->Y
	);

	Set_ink(SILVER_TEXT);

	/* Print conditions description */
	Print_centered_line_string
	(
		&Main_OPM,
		4,
		1,
		INVENTORY_MIDDLE - 8,
		System_text_ptrs[661]
	);

	/* Set printing ink */
	Set_ink(RED_TEXT);

	/* Get conditions */
	Conditions = Get_conditions(Inventory_char_handle);

	/* Print conditions */
	Y = 11;
	for (i=0;i<MAX_CONDITIONS;i+=2)
	{
		/* Does the character have this condition ? */
		if (Conditions & (1 << i))
		{
			/* Yes -> Print condition name */
			Print_string
			(
				&Main_OPM,
				4,
				Y,
				System_text_ptrs[609 + i]
			);
		}

		/* Does the character have this condition ? */
		if (Conditions & (1 << (i + 1)))
		{
			/* Yes -> Print condition name */
			Print_string
			(
				&Main_OPM,
				INVENTORY_MIDDLE / 2 + 4,
				Y,
				System_text_ptrs[609 + i + 1]
			);
		}

		/* Increase Y-coordinate */
		Y += 10;
	}

	/* Set printing ink */
	Set_ink(SILVER_TEXT);

	/* Print languages description */
	Print_centered_line_string
	(
		&Main_OPM,
		4,
		73,
		INVENTORY_MIDDLE - 8,
		System_text_ptrs[662]
	);

	/* Get known languages */
	Known_languages = Get_known_languages(Inventory_char_handle);

	/* Print languages */
	X = 4;
	Y = 83;
	for (i=0;i<MAX_GAME_LANGUAGES;i++)
	{
		/* Is this language known ? */
		if (Known_languages & (1 << i))
		{
			/* Yes -> Print language name */
			Print_string
			(
				&Main_OPM,
				X,
				Y,
				System_text_ptrs[663 + i]
			);

			/* Left or right column ? */
			if (X == 4)
			{
				/* Left -> Go to the right column */
				X = INVENTORY_MIDDLE / 2 + 4;
			}
			else
			{
				/* Right -> Go to the left column */
				X = 4;

				/* Increase Y-coordinate */
				Y += 10;
			}
		}
	}

	/* Print temporary spells description */
	Print_centered_line_string
	(
		&Main_OPM,
		4,
		105,
		INVENTORY_MIDDLE - 8,
		System_text_ptrs[669]
	);

	/* Print temporary spells */
	Y = 115;
	for (i=0;i<(UNSHORT) MAX_TEMP_SPELLS;i++)
	{
		/* Get the spell's duration */
		Duration = Get_member_temporary_spell_duration
		(
			Inventory_member,
			(TEMP_SPELL_TYPE_T) i
		);

		/* Active ? */
		if (Duration)
		{
			/* Yes -> Get the spell's strength */
			Strength = Get_member_temporary_spell_strength
			(
				Inventory_member,
				(TEMP_SPELL_TYPE_T) i
			);

			/* Print temporary spell name */
			Print_string
			(
				&Main_OPM,
				4,
				Y,
				System_text_ptrs[670 + i]
			);

			/* Print temporary spell strength and duration */
			xprintf
			(
				&Main_OPM,
				64,
				Y,
				System_text_ptrs[673],
				Strength,
				Duration
			);
		}

		/* Increase Y-coordinate */
		Y += 10;
	}

	/* Draw child objects */
	Execute_child_methods
	(
		Object->Self,
		DRAW_METHOD,
		NULL
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Value_bar_object
 * FUNCTION  : Init method of Value bar object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.09.95 15:12
 * LAST      : 09.09.95 15:12
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Value_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Value_bar_object *Value_bar;
	struct Value_bar_OID *OID;

	Value_bar = (struct Value_bar_object *) Object;
	OID = (struct Value_bar_OID *) P;

	/* Copy data from OID */
	Value_bar->Help_message_nr		= OID->Help_message_nr;
	Value_bar->Value_getter			= OID->Value_getter;
	Value_bar->Value_index			= OID->Value_index;
	Value_bar->Absolute_maximum	= OID->Absolute_maximum;

	/* Load initial value and maximum */
	(Value_bar->Value_getter)
	(
		&(Value_bar->Value),
		&(Value_bar->Maximum),
		Value_bar->Value_index
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Value_bar_object
 * FUNCTION  : Draw method of Value bar object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.09.95 16:00
 * LAST      : 25.09.95 16:47
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Value_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Value_bar_object *Value_bar;
	struct BBRECT Rect;
	union Method_parms P2;
	SILONG Box_width;
	SILONG Maximum;
	SILONG Bar_width;
	SILONG Value;

	Value_bar = (struct Value_bar_object *) Object;

	/* Initialize rectangle */
	Rect.left	= Object->X - 1;
	Rect.top		= Object->Y - 1;
	Rect.width	= Object->Rect.width + 2;
	Rect.height	= Object->Rect.height + 2;

	/* Restore background */
	P2.Rect = &Rect;
	Execute_upcast_method(Object->Self,	RESTORE_METHOD, &P2);

	/* Calculate width of surrounding box */
	Maximum = max(min(Value_bar->Maximum, Value_bar->Absolute_maximum), 1);

	Box_width = max((Maximum * Object->Rect.width) /
	 Value_bar->Absolute_maximum, 1);

	/* Draw surrounding box */
	Draw_deep_box
	(
		Current_OPM,
		Object->X - 1,
		Object->Y - 1,
		Box_width + 2,
		Object->Rect.height + 2
	);

	/* Maximum higher than absolute maximum ? */
	if (Value_bar->Maximum > Value_bar->Absolute_maximum)
	{
		/* Yes -> Indicate this */
		OPM_Box
		(
			Current_OPM,
			Object->X - 1,
			Object->Y - 1,
			Box_width + 2,
			Object->Rect.height + 2,
			RED
		);
	}

	/* Calculate width of value bar */
	Value = min(Value_bar->Value, Maximum);

	Bar_width = (Value * Object->Rect.width) / Value_bar->Absolute_maximum;

	/* Is the value higher than the maximum ? */
	if (Value_bar->Value > Value_bar->Maximum)
	{
		/* Yes -> Draw special value bar */
		OPM_FillBox
		(
			Current_OPM,
			Object->X,
			Object->Y,
			Bar_width,
			Object->Rect.height,
			GOLD + 1
		);
	}
	else
	{
		/* No -> Draw normal value bar */
		OPM_FillBox
		(
			Current_OPM,
			Object->X,
			Object->Y,
			Bar_width,
			Object->Rect.height,
			BLUE
		);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Value_bar_object
 * FUNCTION  : Update method of Value bar object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.09.95 15:15
 * LAST      : 09.09.95 15:15
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Value_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Value_bar_object *Value_bar;
	SILONG Delta_value;
	SILONG Delta_maximum;
	SILONG Value;
	SILONG Maximum;

	Value_bar = (struct Value_bar_object *) Object;

	/* Get value and maximum */
	(Value_bar->Value_getter)
	(
		&Value,
		&Maximum,
		Value_bar->Value_index
	);

	/* Has the value changed ? */
	Delta_value = Value - Value_bar->Value;
	if (Delta_value)
	{
		/* Yes -> Adjust value */
		Value_bar->Value += sgn(Delta_value);
	}

	/* Has the maximum changed ? */
	Delta_maximum = Maximum - Value_bar->Maximum;
	if (Delta_maximum)
	{
		/* Yes -> Adjust maximum */
		Value_bar->Maximum += sgn(Delta_maximum);
	}

	/* Value or maximum changed ? */
	if (Delta_value || Delta_maximum)
	{
		/* Yes -> Redraw */
		Execute_method
		(
			Object->Self,
			DRAW_METHOD,
			NULL
		);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Value_bar_object
 * FUNCTION  : Help method of Value bar object.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.09.95 15:19
 * LAST      : 11.09.95 22:03
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Value_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Value_bar_object *Value_bar;
	SILONG Value;
	SILONG Maximum;
	UNCHAR String[100];

	Value_bar = (struct Value_bar_object *) Object;

	/* Get value and maximum */
	(Value_bar->Value_getter)
	(
		&Value,
		&Maximum,
		Value_bar->Value_index
	);

	/* Make help line string */
	_bprintf
	(
		String,
		100,
		"%s %ld / %ld",
		System_text_ptrs[Value_bar->Help_message_nr],
		Value,
		Maximum
	);

	/* Print help line */
	Execute_method
	(
		Help_line_object,
		SET_METHOD,
		(void *) String
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Attribute_value_getter
 * FUNCTION  : Attribute value bars value getter.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.09.95 17:27
 * LAST      : 16.10.95 22:42
 * INPUTS    : SILONG *Value_ptr - Pointer to value.
 *             SILONG *Maximum_ptr - Pointer to maximum value.
 *             UNSHORT Value_index - Attribute index (0...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Attribute_value_getter(SILONG *Value_ptr, SILONG *Maximum_ptr, UNSHORT Value_index)
{
	/* Get attribute value and maximum */
	*Value_ptr		= (SILONG) Get_attribute(Inventory_char_handle, Value_index);
	*Maximum_ptr	= (SILONG) Get_max_attribute(Inventory_char_handle, Value_index);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Skill_value_getter
 * FUNCTION  : Skill value bars value getter.
 * FILE      : INVLEFT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.09.95 17:27
 * LAST      : 16.10.95 22:37
 * INPUTS    : SILONG *Value_ptr - Pointer to value.
 *             SILONG *Maximum_ptr - Pointer to maximum value.
 *             UNSHORT Value_index - Skill index (0...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Skill_value_getter(SILONG *Value_ptr, SILONG *Maximum_ptr, UNSHORT Value_index)
{
	/* Get skill value and maximum */
	*Value_ptr		= (SILONG) Get_skill(Inventory_char_handle, Value_index);
	*Maximum_ptr	= (SILONG) Get_max_skill(Inventory_char_handle, Value_index);
}

