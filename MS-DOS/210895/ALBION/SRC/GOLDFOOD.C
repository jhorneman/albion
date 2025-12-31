/************
 * NAME     : GOLDFOOD.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 6-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : GOLDFOOD.H
 ************/

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

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <POPUP.H>
#include <INVITEMS.H>
#include <ITMLOGIC.H>
#include <GOLDFOOD.H>
#include <EVELOGIC.H>
#include <GAMETEXT.H>
#include <STATAREA.H>
#include <APRES.H>
#include <INPUTNR.H>
#include <COLOURS.H>

/* defines */

/* structure definitions */

/* Gold and food object */
struct Gold_food_object {
	struct Object Object;

	UNSHORT Type;					/* See GOLDFOOD.H */
	MEM_HANDLE Data_handle;		/* Memory handle of character / chest data */

	BOOLEAN Is_gold;
	UNSHORT Value;
};

/* prototypes */

/* Gold/food object methods */
UNLONG Init_Gold_food_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Gold_food_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Gold_food_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Gold_food_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Gold_food_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Gold_food_object(struct Object *Object, union Method_parms *P);
UNLONG Pop_up_Gold_food_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Gold_food_object(struct Object *Object, union Method_parms *P);
UNLONG DLeft_Gold_food_object(struct Object *Object, union Method_parms *P);
UNLONG Drop_Gold_food_object(struct Object *Object, union Method_parms *P);
UNLONG Inquire_drop_Gold_food_object(struct Object *Object, union Method_parms *P);

/* Gold/food drag & drop support functions */
void Drag_gold_food(struct Gold_food_object *Gold_food, UNSHORT Quantity);
void Drop_on_gold_food(struct Gold_food_object *Gold_food,
 struct Drag_drop_data *Drag_drop_data_ptr);
void Gold_food_drag_abort_handler(struct Drag_drop_data *Drag_drop_data_ptr);

void Build_gold_food_graphics(struct Gold_food_object *Gold_food,
 UNSHORT Quantity);

/* Gold/food object support functions */
void Draw_gold_food(struct Gold_food_object *Gold_food);
UNSHORT Get_gold_food_value(struct Gold_food_object *Gold_food);
void Set_gold_food_value(struct Gold_food_object *Gold_food,
 UNSHORT New_value);

/* Gold/food pop-up menu functions */
void Gold_food_PUM_evaluator(struct PUM *PUM);
void PUM_Drop_gold_food(UNLONG Data);

/* global variables */

/* Drag data */
UNSHORT Drag_quantity;

/* Gold/food object class */
static struct Method Gold_food_methods[] = {
	{ INIT_METHOD, Init_Gold_food_object },
	{ DRAW_METHOD, Draw_Gold_food_object },
	{ UPDATE_METHOD, Update_Gold_food_object },
	{ FEEDBACK_METHOD, Feedback_Gold_food_object },
	{ HIGHLIGHT_METHOD, Highlight_Gold_food_object },
	{ HELP_METHOD, Help_Gold_food_object },
	{ POP_UP_METHOD, Pop_up_Gold_food_object },
	{ LEFT_METHOD, Left_Gold_food_object },
	{ DLEFT_METHOD, DLeft_Gold_food_object },
	{ RIGHT_METHOD, Normal_rightclicked },
	{ DROP_METHOD, Drop_Gold_food_object },
	{ INQUIRE_DROP_METHOD, Inquire_drop_Gold_food_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

struct Object_class Gold_food_Class = {
	0, sizeof(struct Gold_food_object),
	&Gold_food_methods[0]
};

/* Gold/food pop-up menu */
static struct PUME Gold_food_PUMEs[] = {
	{PUME_AUTO_CLOSE, 0, 50, PUM_Drop_gold_food},
};
static struct PUM Gold_food_PUM = {
	1,
	NULL,
	0,
	Gold_food_PUM_evaluator,
	Gold_food_PUMEs
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Gold_food_object
 * FUNCTION  : Init method of Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 13:32
 * LAST      : 15.08.95 19:52
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Gold_food_object(struct Object *Object, union Method_parms *P)
{
	struct Gold_food_object *Gold_food;
	struct Gold_food_OID *OID;

	Gold_food = (struct Gold_food_object *) Object;
	OID = (struct Gold_food_OID *) P;

	/* Copy OID data */
	Gold_food->Type = OID->Type;
	Gold_food->Data_handle = OID->Data_handle;

	/* Gold or food ? */
	if ((Gold_food->Type == CHAR_GOLD_TYPE) ||
	 (Gold_food->Type == CHEST_GOLD_TYPE) ||
	 (Gold_food->Type == APRES_GOLD_TYPE))
	{
		/* Gold */
		Gold_food->Is_gold = TRUE;
	}
	else
	{
		/* Food */
		Gold_food->Is_gold = FALSE;
	}

	/* Load initial value */
	Gold_food->Value = Get_gold_food_value(Gold_food);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Gold_food_object
 * FUNCTION  : Draw method of Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 13:32
 * LAST      : 06.07.95 11:31
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Gold_food_object(struct Object *Object, union Method_parms *P)
{
	struct Gold_food_object *Gold_food;

	Gold_food = (struct Gold_food_object *) Object;

	/* Erase object */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_high_border(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw gold/food */
	Draw_gold_food(Gold_food);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Gold_food_object
 * FUNCTION  : Update method of Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 13:35
 * LAST      : 06.07.95 11:32
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Gold_food_object(struct Object *Object, union Method_parms *P)
{
	struct Gold_food_object *Gold_food;
	UNSHORT New_value;

	Gold_food = (struct Gold_food_object *) Object;

	/* Get value */
	New_value = Get_gold_food_value(Gold_food);

	/* Has the value changed ? */
	if (New_value != Gold_food->Value)
	{
		/* Yes -> Adjust value */
		Gold_food->Value += sgn(New_value - Gold_food->Value);

		/* Redraw */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Gold_food_object
 * FUNCTION  : Feedback method of Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 15:42
 * LAST      : 06.07.95 11:33
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_Gold_food_object(struct Object *Object, union Method_parms *P)
{
	struct Gold_food_object *Gold_food;

	Gold_food = (struct Gold_food_object *) Object;

	/* Erase object */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_deep_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw gold/food */
	Draw_gold_food(Gold_food);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Gold_food_object
 * FUNCTION  : Highlight method of Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 13:35
 * LAST      : 06.07.95 11:36
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Gold_food_object(struct Object *Object, union Method_parms *P)
{
	struct Gold_food_object *Gold_food;

	Gold_food = (struct Gold_food_object *) Object;

	/* Erase object */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_high_border(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Put_recoloured_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height, &(Recolour_tables[6][0]));

	/* Draw gold/food */
	Draw_gold_food(Gold_food);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Gold_food_object
 * FUNCTION  : Help method of Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 13:37
 * LAST      : 06.07.95 11:38
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Gold_food_object(struct Object *Object, union Method_parms *P)
{
	struct Gold_food_object *Gold_food;
	UNSHORT Value;
	UNCHAR String[100];

	Gold_food = (struct Gold_food_object *) Object;

	/* Get value */
	Value = Get_gold_food_value(Gold_food);

	/* Gold or food ? */
	if (Gold_food->Is_gold)
	{
		/* Gold -> Make help line string */
		sprintf(String, System_text_ptrs[48], Value);
	}
	else
	{
		/* Food -> Make help line string */
		sprintf(String, System_text_ptrs[49], Value);
	}

	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) String);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_up_Gold_food_object
 * FUNCTION  : Pop-up method of Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 15:49
 * LAST      : 06.07.95 11:39
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Pop_up_Gold_food_object(struct Object *Object, union Method_parms *P)
{
	struct Gold_food_object *Gold_food;

	Gold_food = (struct Gold_food_object *) Object;

	/* Gold or food ? */
	if (Gold_food->Is_gold)
	{
		/* Gold -> Set pop-up menu title */
		Gold_food_PUM.Title = System_text_ptrs[51];
	}
	else
	{
		/* Food -> Set pop-up menu title */
		Gold_food_PUM.Title = System_text_ptrs[53];
	}

	/* Call pop-up menu */
	PUM_source_object_handle = Object->Self;
	Do_PUM(Object->X + 16, Object->Y + 8, &Gold_food_PUM);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Gold_food_object
 * FUNCTION  : Left method of Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 13:23
 * LAST      : 07.07.95 11:26
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Gold_food_object(struct Object *Object, union Method_parms *P)
{
	struct Gold_food_object *Gold_food;
	UNSHORT Total_value;
	UNSHORT Quantity;

	Gold_food = (struct Gold_food_object *) Object;

	/* Get total value */
	Total_value = Get_gold_food_value(Gold_food);

	/* Anything ? */
	if (Total_value)
	{
		/* Yes -> Really clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Value larger than 1 ? */
			Quantity = 1;
			if (Total_value > 1)
			{
				/* Yes -> Gold or food ? */
				if (Gold_food->Is_gold)
				{
					/* Gold -> Ask the player how many should be taken */
					Quantity = (UNSHORT) Input_number_with_symbol(1, 0,
					 (SILONG) Total_value, System_text_ptrs[52], 12, 10,
					 NULL, (UNLONG) &(Gold_symbol[0]));
				}
				else
				{
					/* Food -> Ask the player how many should be taken */
					Quantity = (UNSHORT) Input_number_with_symbol(1, 0,
					 (SILONG) Total_value, System_text_ptrs[515], 20, 10,
					 NULL, (UNLONG) &(Food_symbol[0]));
				}
			}

			/* Zero ? */
			if (Quantity)
			{
				/* No -> Drag */
				Drag_gold_food(Gold_food, Quantity);
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : DLeft_Gold_food_object
 * FUNCTION  : DLeft method of Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 13:25
 * LAST      : 07.07.95 11:26
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
DLeft_Gold_food_object(struct Object *Object, union Method_parms *P)
{
	struct Gold_food_object *Gold_food;
	UNSHORT Total_value;

	Gold_food = (struct Gold_food_object *) Object;

	/* Get total value */
	Total_value = Get_gold_food_value(Gold_food);

	/* Anything ? */
	if (Total_value)
	{
		/* Yes -> Really clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Drag */
			Drag_gold_food(Gold_food, Total_value);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_Gold_food_object
 * FUNCTION  : Drop method of Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 13:26
 * LAST      : 07.07.95 11:31
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Drop_Gold_food_object(struct Object *Object, union Method_parms *P)
{
	/* Really clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Drop */
		Drop_on_gold_food((struct Gold_food_object *) Object,
		 P->Drag_drop_data_ptr);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inquire_drop_Gold_food_object
 * FUNCTION  : Inquire drop method of Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 16:40
 * LAST      : 06.07.95 16:40
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Inquire_drop_Gold_food_object(struct Object *Object,
 union Method_parms *P)
{
	struct Gold_food_object *Gold_food;
	struct Drag_drop_data *Drag_drop_data_ptr;
	UNSHORT Data_ID;

	Gold_food = (struct Gold_food_object *) Object;

	/* Get drag & drop data */
	Drag_drop_data_ptr = P->Drag_drop_data_ptr;

	/* Get drag & drop data ID */
	Data_ID = Drag_drop_data_ptr->Data_ID;

	/* Gold or food ? */
	if (Gold_food->Is_gold)
	{
		/* Gold -> Right data type ? */
		if (Data_ID == GOLD_DD_DATA_ID)
		{
			/* Yes */
			return 1;
		}
		else
		{
			/* No */
			return 0;
		}
	}
	{
		/* Food -> Right data type ? */
		if (Data_ID == FOOD_DD_DATA_ID)
		{
			/* Yes */
			return 1;
		}
		else
		{
			/* No */
			return 0;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drag_gold_food
 * FUNCTION  : Drag gold or food.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 13:27
 * LAST      : 07.07.95 10:55
 * INPUTS    : struct Gold_food_object *Gold_food - Pointer to object.
 *             UNSHORT Quantity - Quantity that should be dragged.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drag_gold_food(struct Gold_food_object *Gold_food, UNSHORT Quantity)
{
	UNSHORT Total_value;

	/* Any ? */
	if (Quantity)
	{
		/* Yes -> Remove gold */
		Total_value = Get_gold_food_value(Gold_food);
		Set_gold_food_value(Gold_food, Total_value - Quantity);

		/* Redraw gold/food object */
		Execute_method(Gold_food->Object.Self, DRAW_METHOD, NULL);

		/* Store quantity */
		Drag_quantity = Quantity;

		/* Initialize gold/food dragging */
		Init_drag(40, 21, 1);

		/* Build graphics */
		Build_gold_food_graphics(Gold_food, Quantity);

		/* Add HDOB */
		Drag_HDOB_nr = Add_HDOB(&Drag_HDOB);

		/* Pick mouse pointer */
		Push_mouse(&(Mouse_pointers[PICK_MPTR]));

		/* Gold or food ? */
		if (Gold_food->Is_gold)
		{
			/* Gold -> Enter drag & drop mode */
			Enter_drag_drop_mode(GOLD_DD_DATA_ID, Gold_food_drag_abort_handler,
			 &(Gold_food->Object), NULL);
		}
		else
		{
			/* Food -> Enter drag & drop mode */
			Enter_drag_drop_mode(FOOD_DD_DATA_ID, Gold_food_drag_abort_handler,
			 &(Gold_food->Object), NULL);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_on_gold_food
 * FUNCTION  : Drop on gold or food.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 13:28
 * LAST      : 07.07.95 10:58
 * INPUTS    : struct Gold_food_object *Gold_food - Pointer to object.
 *             struct Drag_drop_data *Drag_drop_data_ptr - Pointer to
 *              drag & drop data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_on_gold_food(struct Gold_food_object *Gold_food,
 struct Drag_drop_data *Drag_drop_data_ptr)
{
	UNSHORT Target_value;
	UNSHORT Added_quantity;

	/* Get amount in target object */
	Target_value = Get_gold_food_value(Gold_food);

	/* How many will fit in this object ? */
	Added_quantity = min(Drag_quantity, 32767 - Target_value);

	/* Any ? */
	if (Added_quantity)
	{
		/* Yes -> Move drag HDOB towards target object */
		Move_drag_HDOB_towards_object(&(Gold_food->Object));

		/* Transfer */
		Set_gold_food_value(Gold_food, Target_value + Added_quantity);
		Drag_quantity -= Added_quantity;

		/* Everything transferred ? */
		if (Drag_quantity)
		{
			/* No -> Re-build graphics */
			Build_item_graphics();
		}
		else
		{
			/* Yes -> Exit dragging */
			Exit_drag();
			Pop_mouse();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Gold_food_drag_abort_handler
 * FUNCTION  : Gold or food drag & drop abort handler.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 13:31
 * LAST      : 07.07.95 11:08
 * INPUTS    : struct Drag_drop_data *Drag_drop_data_ptr - Pointer to
 *              drag & drop data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Gold_food_drag_abort_handler(struct Drag_drop_data *Drag_drop_data_ptr)
{
	/* Act depending on drag & drop data ID */
	switch (Drag_drop_data_ptr->Data_ID)
	{
		case GOLD_DD_DATA_ID:
		case FOOD_DD_DATA_ID:
		{
			/* Drop the gold or food back on the source slot */
			Drop_on_gold_food((struct Gold_food_object *)
			 Drag_drop_data_ptr->Source_object, Drag_drop_data_ptr);

			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Gold_food
 * FUNCTION  : Draw Gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 11:35
 * LAST      : 06.07.95 11:35
 * INPUTS    : struct Gold_food_object *Gold_food - Pointer to object data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_gold_food(struct Gold_food_object *Gold_food)
{
	struct Object *Object;
	UNCHAR String[20];

	Object = &(Gold_food->Object);

	/* Gold or food ? */
	if (Gold_food->Is_gold)
	{
		/* Gold -> Draw gold symbol */
		Put_masked_block(Current_OPM, Object->X + (Object->Rect.width - 12) / 2,
		 Object->Y + 1, 12, 10, &(Gold_symbol[0]));
	}
	else
	{
		/* Food -> Draw food symbol */
		Put_masked_block(Current_OPM, Object->X + (Object->Rect.width - 20) / 2,
		 Object->Y + 1, 20, 10, &(Food_symbol[0]));
	}

	/* Set ink colour */
	Set_ink(SILVER_TEXT);

	/* Print value */
	sprintf(String, "%u", Gold_food->Value);
	Print_centered_string(Current_OPM, Object->X + 1, Object->Y + 11,
	 Object->Rect.width - 2, String);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_gold_food_value
 * FUNCTION  : Get the value out of a gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 11:18
 * LAST      : 15.08.95 19:53
 * INPUTS    : struct Gold_food_object *Gold_food - Pointer to object.
 * RESULT    : UNSHORT : Value.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_gold_food_value(struct Gold_food_object *Gold_food)
{
	UNSHORT Value = 0;

	/* Act depending on object type */
	switch(Gold_food->Type)
	{
		/* Character gold */
		case CHAR_GOLD_TYPE:
		{
			struct Character_data *Char;

			/* Get character data */
			Char = (struct Character_data *)
			 MEM_Claim_pointer(Gold_food->Data_handle);

			/* Get gold */
			Value = Char->Char_gold;

			MEM_Free_pointer(Gold_food->Data_handle);

			break;
		}
		/* Character food */
		case CHAR_FOOD_TYPE:
		{
			struct Character_data *Char;

			/* Get character data */
			Char = (struct Character_data *)
			 MEM_Claim_pointer(Gold_food->Data_handle);

			/* Get food */
			Value = Char->Char_food;

			MEM_Free_pointer(Gold_food->Data_handle);

			break;
		}
		/* Chest gold */
		case CHEST_GOLD_TYPE:
		{
			struct Chest_data *Chest;

			/* Get chest data */
			Chest = (struct Chest_data *)
			 MEM_Claim_pointer(Gold_food->Data_handle);

			/* Get gold */
			Value = Chest->Chest_gold;

			MEM_Free_pointer(Gold_food->Data_handle);

			break;
		}
		/* Chest food */
		case CHEST_FOOD_TYPE:
		{
			struct Chest_data *Chest;

			/* Get chest data */
			Chest = (struct Chest_data *)
			 MEM_Claim_pointer(Gold_food->Data_handle);

			/* Get food */
			Value = Chest->Chest_food;

			MEM_Free_pointer(Gold_food->Data_handle);

			break;
		}
		/* Apres combat gold */
		case APRES_GOLD_TYPE:
		{
			/* Get gold */
			Value = Apres_gold_coins;
			break;
		}
		/* Apres combat food */
		case APRES_FOOD_TYPE:
		{
			/* Get food */
			Value = Apres_food_rations;
			break;
		}
	}

	return Value;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_gold_food_value
 * FUNCTION  : Set the value of a gold/food object.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 11:19
 * LAST      : 15.08.95 19:53
 * INPUTS    : struct Gold_food_object *Gold_food - Pointer to object.
 *             UNSHORT New_value - New value.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_gold_food_value(struct Gold_food_object *Gold_food, UNSHORT New_value)
{
	/* Act depending on object type */
	switch(Gold_food->Type)
	{
		/* Character gold */
		case CHAR_GOLD_TYPE:
		{
			struct Character_data *Char;

			/* Get character data */
			Char = (struct Character_data *)
			 MEM_Claim_pointer(Gold_food->Data_handle);

			/* Set gold */
			Char->Char_gold = New_value;

			MEM_Free_pointer(Gold_food->Data_handle);

			break;
		}
		/* Character food */
		case CHAR_FOOD_TYPE:
		{
			struct Character_data *Char;

			/* Get character data */
			Char = (struct Character_data *)
			 MEM_Claim_pointer(Gold_food->Data_handle);

			/* Set food */
			Char->Char_food = New_value;

			MEM_Free_pointer(Gold_food->Data_handle);

			break;
		}
		/* Chest gold */
		case CHEST_GOLD_TYPE:
		{
			struct Chest_data *Chest;

			/* Get chest data */
			Chest = (struct Chest_data *)
			 MEM_Claim_pointer(Gold_food->Data_handle);

			/* Set gold */
			Chest->Chest_gold = New_value;

			MEM_Free_pointer(Gold_food->Data_handle);

			break;
		}
		/* Chest food */
		case CHEST_FOOD_TYPE:
		{
			struct Chest_data *Chest;

			/* Get chest data */
			Chest = (struct Chest_data *)
			 MEM_Claim_pointer(Gold_food->Data_handle);

			/* Set food */
			Chest->Chest_food = New_value;

			MEM_Free_pointer(Gold_food->Data_handle);

			break;
		}
		/* Apres combat gold */
		case APRES_GOLD_TYPE:
		{
			/* Set gold */
			Apres_gold_coins = New_value;
			break;
		}
		/* Apres combat food */
		case APRES_FOOD_TYPE:
		{
			/* Set food */
			Apres_food_rations = New_value;
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Gold_food_PUM_evaluator
 * FUNCTION  : Evaluate gold/food object pop-up menu.
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 15:51
 * LAST      : 06.07.95 11:23
 * INPUTS    : struct PUM *PUM - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Gold_food_PUM_evaluator(struct PUM *PUM)
{
	struct PUME *PUMES;

	PUMES = PUM->PUME_list;

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Drop_gold_food
 * FUNCTION  : Drop gold or food (gold/food object pop-up menu).
 * FILE      : GOLDFOOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 15:52
 * LAST      : 06.07.95 11:23
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Drop_gold_food(UNLONG Data)
{
}

/*
; ********** Inventory 2 - Drop gold **************
Drop_gold:
	jsr	Destroy_ghosts		; Destroy !
	Get	Inventory_handle,a0		; How much ?
	moveq.l	#0,d0
	move.w	Gold_coins(a0),d0
	move.l	d0,Maximum_number
	Free	Inventory_handle
	move.w	#32,InputNr_prompt
	move.b	#1,ObjGoldFood
	clr.l	Minimum_number
	Push	Module,InputNr_Mod
	tst.l	InputNr_number		; Any ?
	beq	.Exit
	moveq.l	#49,d0			; Are you sure ?
	jsr	Prompt_req
	tst.b	d0			; Yes ?
	beq.s	.Exit
	Get	Inventory_handle,a0		; Drop gold
	move.l	InputNr_number,d0
	sub.w	d0,Gold_coins(a0)
	mulu.w	#Gold_weight,d0
	sub.l	d0,Weight_normal(a0)
	Free	Inventory_handle
.Exit:	rts
*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Build_gold_food_graphics
 * FUNCTION  : Build gold / food graphics for dragging.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.07.95 10:05
 * LAST      : 07.07.95 11:24
 * INPUTS    : struct Gold_food_object *Gold_food - Pointer to object that
 *              is the drag source.
 *             UNSHORT Quantity - Quantity that is dragged.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Build_gold_food_graphics(struct Gold_food_object *Gold_food,
 UNSHORT Quantity)
{
	UNCHAR String[20];

	/* Clear drag OPM */
	OPM_FillBox(&Drag_OPM, 0, 0, 40, 21, 0);

	/* Gold or food ? */
	if (Gold_food->Is_gold)
	{
		/* Gold -> Draw gold symbol */
		Put_masked_block(&Drag_OPM, 14, 0, 12, 10, &(Gold_symbol[0]));

		/* Set HDOB hotspot */
		Drag_HDOB.X = 3 - 14;
		Drag_HDOB.Y = 3;
	}
	else
	{
		/* Food -> Draw food symbol */
		Put_masked_block(&Drag_OPM, 10, 0, 20, 10, &(Food_symbol[0]));

		/* Set HDOB hotspot */
		Drag_HDOB.X = 3 - 10;
		Drag_HDOB.Y = 3;
	}

	/* Print quantity */
	sprintf(String, "%u", Quantity);
	Print_centered_string(&Drag_OPM, 0, 11, 40, String);

	/* Set number of animation frames */
	Drag_HDOB.Nr_frames = 1;

	/* Change HDOB data */
	Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);
}

