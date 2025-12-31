/************
 * NAME     : BOOLREQ.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : BOOLREQ.H, USERFACE.C
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBMEM.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <GAMETEXT.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <BOOLREQ.H>
#include <BUTTONS.H>

/* defines */

#define BOOLEANREQ_WIDTH	(200)
#define BOOLEANREQ_HEIGHT	(74)

/* structure definitions */

/* Boolean requester OID */
struct BooleanReq_OID {
	UNCHAR *Text_ptr;
	UNCHAR *Yes_text_ptr;
	UNCHAR *No_text_ptr;
	BOOLEAN *Result_ptr;
};

/* Boolean requester object */
struct BooleanReq_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNCHAR *Text_ptr;
	BOOLEAN *Result_ptr;
};

/* prototypes */

/* Boolean requester object methods */
UNLONG Init_BooleanReq_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_BooleanReq_object(struct Object *Object, union Method_parms *P);

/* Boolean requester button handlers */
void BooleanReq_Yes(struct Button_object *Button);
void BooleanReq_No(struct Button_object *Button);

/* global variables */

/* Boolean requester method list */
static struct Method BooleanReq_methods[] = {
	{ INIT_METHOD, Init_BooleanReq_object },
	{ EXIT_METHOD, Exit_Window_object },
	{ DRAW_METHOD, Draw_BooleanReq_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ RESTORE_METHOD, Restore_window },
	{ 0, NULL}
};

/* Boolean requester class description */
static struct Object_class BooleanReq_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct BooleanReq_object),
	&BooleanReq_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Boolean_requester
 * FUNCTION  : Display and handle a boolean requester window.
 * FILE      : BOOLREQ.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.02.95 17:15
 * LAST      : 25.06.95 19:46
 * INPUTS    : UNCHAR *Text - Pointer to window text.
 * RESULT    : BOOLEAN : Return value.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Boolean_requester(UNCHAR *Text)
{
	/* Do boolean requester with default button texts */
	return(Boolean_requester_with_buttons(Text, System_text_ptrs[60],
	 System_text_ptrs[61]));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Boolean_requester_with_buttons
 * FUNCTION  : Display and handle a boolean requester window.
 * FILE      : BOOLREQ.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 19:45
 * LAST      : 25.06.95 19:45
 * INPUTS    : UNCHAR *Text - Pointer to window text.
 *             UNCHAR *Yes_button_text - Pointer to yes-button text.
 *             UNCHAR *No_button_text - Pointer to no-button text.
 * RESULT    : BOOLEAN : Return value.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Boolean_requester_with_buttons(UNCHAR *Text, UNCHAR *Yes_button_text,
 UNCHAR *No_button_text)
{
	struct BooleanReq_OID OID;
	BOOLEAN Result;
	UNSHORT Obj;

	/* Prepare OID */
	OID.Text_ptr = Text;
	OID.Yes_text_ptr = Yes_button_text;
	OID.No_text_ptr = No_button_text;
	OID.Result_ptr = &Result;

	/* Do it */
	Push_module(&Window_Mod);

	Obj = Add_object(0, &BooleanReq_Class, (UNBYTE *) &OID,
	 (Screen_width - BOOLEANREQ_WIDTH) / 2, (Panel_Y - BOOLEANREQ_HEIGHT) / 2,
	 BOOLEANREQ_WIDTH, BOOLEANREQ_HEIGHT);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_BooleanReq_object
 * FUNCTION  : Initialize method of Boolean requester window object.
 * FILE      : BOOLREQ.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.02.95 17:20
 * LAST      : 25.06.95 19:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_BooleanReq_object(struct Object *Object, union Method_parms *P)
{
	static union Button_data BooleanReq_YES_button_data,
	 BooleanReq_NO_button_data;
	static struct Button_OID BooleanReq_YES_button_OID = {
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&BooleanReq_YES_button_data,
		BooleanReq_Yes
	};
	static struct Button_OID BooleanReq_NO_button_OID = {
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&BooleanReq_NO_button_data,
		BooleanReq_No
	};

	struct BooleanReq_object *BooleanReq;
	struct BooleanReq_OID *OID;

	BooleanReq = (struct BooleanReq_object *) Object;
	OID = (struct BooleanReq_OID *) P;

	/* Copy data from OID */
	BooleanReq->Text_ptr = OID->Text_ptr;
	BooleanReq->Result_ptr = OID->Result_ptr;

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Initialize button data */
	BooleanReq_YES_button_data.Text_button_data.Text = OID->Yes_text_ptr;
	BooleanReq_NO_button_data.Text_button_data.Text = OID->No_text_ptr;

	/* Add buttons to window */
 	Add_object(Object->Self, &Button_Class,
	 (UNBYTE *) &BooleanReq_YES_button_OID, 45, 50, 50, 11);
 	Add_object(Object->Self, &Button_Class,
	 (UNBYTE *) &BooleanReq_NO_button_OID, 105, 50, 50, 11);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_BooleanReq_object
 * FUNCTION  : Draw method of Boolean requester window object.
 * FILE      : BOOLREQ.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.02.95 17:26
 * LAST      : 23.02.95 17:26
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_BooleanReq_object(struct Object *Object, union Method_parms *P)
{
	struct BooleanReq_object *BooleanReq;
	struct PA PA;
	struct OPM *OPM;
	UNSHORT W, H;

	BooleanReq = (struct BooleanReq_object *) Object;
	OPM = &(BooleanReq->Window_OPM);

	W = Object->Rect.width;
	H = Object->Rect.height;

	/* Draw window's shadow */
	Put_recoloured_box(OPM, 10, 10, W - 10, H - 10, &(Recolour_tables[1][0]));

	/* Draw window */
	Draw_window_inside(OPM, 7, 7, W - 14, H - 14);
	Draw_window_border(OPM, 0, 0, W, H);

	/* Any text ? */
	if (BooleanReq->Text_ptr)
	{
		/* Yes -> Create print area */
		PA.Area.left = 15;
		PA.Area.top = 14;
		PA.Area.width = BOOLEANREQ_WIDTH - PA.Area.left - 16;
		PA.Area.height = 28;

		/* Draw box around text */
		Draw_deep_box(OPM, PA.Area.left - 2, PA.Area.top - 2,
		 PA.Area.width + 4, PA.Area.height + 4);

		/* Print text */
		Push_PA(&PA);

		Set_ink(SILVER_TEXT);

		Display_text(OPM, BooleanReq->Text_ptr);

		Pop_PA();
	}

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BooleanReq_Yes
 * FUNCTION  : Yes (button).
 * FILE      : BOOLREQ.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.02.95 17:27
 * LAST      : 23.02.95 17:27
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
BooleanReq_Yes(struct Button_object *Button)
{
	struct BooleanReq_object *BooleanReq;

	/* Get requester data */
	BooleanReq = (struct BooleanReq_object *)
	 Get_object_data(Button->Object.Parent);

	/* Set result to TRUE */
	*(BooleanReq->Result_ptr) = TRUE;

	/* Exit requester */
	Delete_object(BooleanReq->Object.Self);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BooleanReq_No
 * FUNCTION  : No (button).
 * FILE      : BOOLREQ.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.02.95 17:29
 * LAST      : 23.02.95 17:29
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
BooleanReq_No(struct Button_object *Button)
{
	struct BooleanReq_object *BooleanReq;

	/* Get requester data */
	BooleanReq = (struct BooleanReq_object *)
	 Get_object_data(Button->Object.Parent);

	/* Set result to FALSE */
	*(BooleanReq->Result_ptr) = FALSE;

	/* Exit requester */
	Delete_object(BooleanReq->Object.Self);
}

