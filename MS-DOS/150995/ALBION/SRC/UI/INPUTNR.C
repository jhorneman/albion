/************
 * NAME     : INPUTNR.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : INPUTNR.H
 ************/

/* includes */

#include <stdlib.h>
#include <stdio.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>
#include <BBEVENT.H>

#include <HDOB.H>
#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <GAMETEXT.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <INPUTNR.H>
#include <SCROLBAR.H>
#include <BUTTONS.H>
#include <COLOURS.H>

/* defines */

#define INPUTNR_WIDTH	(200)
#define INPUTNR_HEIGHT	(92)

#define ADD_DELTA			(8)
#define FIRST_ADD			(256 - (20 * ADD_DELTA))
#define MAX_DELTA_ADD	(16384)

#define NUMBER_BAR_HEIGHT			(12)
#define NUMBER_BAR_SLIDER_WIDTH	(29)

/* structure definitions */

/* Number input OID */
struct InputNr_OID {
	SILONG *Value_ptr;
	SILONG Minimum, Maximum;

	UNCHAR *Text;

	UNSHORT Symbol_width, Symbol_height;
	MEM_HANDLE Graphics_handle;
	UNLONG Graphics_offset;
};

/* Number input object */
struct InputNr_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	SILONG *Value_ptr;
	SILONG Minimum, Maximum;

	UNCHAR *Text;

	UNSHORT Symbol_width, Symbol_height;
	MEM_HANDLE Graphics_handle;
	UNLONG Graphics_offset;

};

/* Number bar OID */
struct Number_bar_OID {
	SILONG *Value_ptr, Minimum, Maximum;
	void (*Update)(struct Number_bar_object *Number_bar);
};

/* Number bar object */
struct Number_bar_object {
	struct Object Object;
	UNSHORT Flags;

	SILONG *Value_ptr, Minimum, Maximum;

	void (*Update)(struct Number_bar_object *Number_bar);

	UNSHORT Slider_X;				/* X-coordinate of slider */
	SISHORT Delta_add;
};

/* prototypes */

/* Number input object methods */
UNLONG Init_InputNr_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_InputNr_object(struct Object *Object, union Method_parms *P);

/* Number input button handler */
void InputNr_Exit(struct Button_object *Button);

/* Number bar object methods */
UNLONG Init_Number_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Number_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Get_Number_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Set_Number_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Number_bar_object(struct Object *Object, union Method_parms *P);

/* Number bar support functions */
void Number_bar_page_up(struct Number_bar_object *Number_bar);
void Number_bar_page_down(struct Number_bar_object *Number_bar);
void Light_number_slider(struct Number_bar_object *Number_bar);
void Update_number_slider(struct Number_bar_object *Number_bar, UNSHORT X);
void Do_draw_number_bar(struct Number_bar_object *Number_bar);

void Number_bar_1_up(struct Button_object *Button);
void Number_bar_1_down(struct Button_object *Button);

/* global variables */

/* Number input window method list */
static struct Method InputNr_methods[] = {
	{ INIT_METHOD, Init_InputNr_object },
	{ EXIT_METHOD, Exit_Window_object },
	{ DRAW_METHOD, Draw_InputNr_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ RESTORE_METHOD, Restore_window },
	{ 0, NULL}
};

/* Number input window class description */
static struct Object_class InputNr_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct InputNr_object),
	&InputNr_methods[0]
};

/* Number bar method list */
static struct Method Number_bar_methods[] = {
	{ INIT_METHOD, Init_Number_bar_object },
	{ DRAW_METHOD, Draw_Number_bar_object },
	{ LEFT_METHOD, Left_Number_bar_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ GET_METHOD, Get_Number_bar_object },
	{ SET_METHOD, Set_Number_bar_object },
	{ 0, NULL}
};

/* Number bar class description */
static struct Object_class Number_bar_Class = {
	0, sizeof(struct Number_bar_object),
	&Number_bar_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Input_number
 * FUNCTION  : Display and handle a number input window.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 17:35
 * LAST      : 11.01.95 17:35
 * INPUTS    : SILONG Value - Start value.
 *             SILONG Minimum - Minimum value.
 *             SILONG Maximum - Maximum value.
 *             UNCHAR *Text - Pointer to window text.
 * RESULT    : SILONG : Return value.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG
Input_number(SILONG Value, SILONG Minimum, SILONG Maximum, UNCHAR *Text)
{
	struct InputNr_OID OID;
	SILONG It;
	UNSHORT Obj;

	/* Store value */
	It = Value;

	/* Prepare OID */
	OID.Value_ptr			= &It;
	OID.Minimum				= Minimum;
	OID.Maximum				= Maximum;

	OID.Text					= Text;

	OID.Symbol_width		= 0;
	OID.Symbol_height		= 0;
	OID.Graphics_handle	= 0;
	OID.Graphics_offset	= 0;

	/* Do it */
	Push_module(&Window_Mod);

	Obj = Add_object
	(
		0,
		&InputNr_Class,
		(UNBYTE *) &OID,
		(Screen_width - INPUTNR_WIDTH) / 2,
		(Panel_Y - INPUTNR_HEIGHT) / 2,
		INPUTNR_WIDTH,
		INPUTNR_HEIGHT
	);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);

	return It;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Input_number_with_symbol
 * FUNCTION  : Display and handle a number input window with a symbol.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.02.95 14:15
 * LAST      : 23.02.95 14:15
 * INPUTS    : SILONG Value - Start value.
 *             SILONG Minimum - Minimum value.
 *             SILONG Maximum - Maximum value.
 *             UNCHAR *Text - Pointer to window text.
 *             UNSHORT Width - Width of symbol.
 *             UNSHORT Height - Height of symbol.
 *             MEM_HANDLE Graphics_handle - Handle of graphics / NULL.
 *             UNLONG Graphics_offset - Offset to graphics.
 * RESULT    : SILONG : Return value.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG
Input_number_with_symbol(SILONG Value, SILONG Minimum, SILONG Maximum,
 UNCHAR *Text, UNSHORT Width, UNSHORT Height, MEM_HANDLE Graphics_handle,
 UNLONG Graphics_offset)
{
	struct InputNr_OID OID;
	SILONG It;
	UNSHORT Obj;

	/* Store value */
	It = Value;

	/* Prepare OID */
	OID.Value_ptr			= &It;
	OID.Minimum				= Minimum;
	OID.Maximum				= Maximum;

	OID.Text					= Text;

	OID.Symbol_width		= Width;
	OID.Symbol_height		= Height;
	OID.Graphics_handle	= Graphics_handle;
	OID.Graphics_offset	= Graphics_offset;

	/* Do it */
	Push_module(&Window_Mod);

	Obj = Add_object
	(
		0,
		&InputNr_Class,
		(UNBYTE *) &OID,
		(Screen_width - INPUTNR_WIDTH) / 2,
		(Panel_Y - INPUTNR_HEIGHT) / 2,
		INPUTNR_WIDTH,
		INPUTNR_HEIGHT
	);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);

	return It;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_InputNr_object
 * FUNCTION  : Initialize method of Number input window object.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 17:39
 * LAST      : 11.01.95 17:39
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_InputNr_object(struct Object *Object, union Method_parms *P)
{
	static union Button_data InputNr_exit_button_data;
	static struct Button_OID InputNr_exit_button_OID = {
		TEXT_BUTTON_TYPE,
		0,
		695,
		&InputNr_exit_button_data,
		InputNr_Exit
	};

	struct InputNr_object *InputNr;
	struct InputNr_OID *OID;
	struct Number_bar_OID Number_bar_OID;

	InputNr = (struct InputNr_object *) Object;
	OID = (struct InputNr_OID *) P;

	/* Copy data from OID */
	InputNr->Value_ptr			= OID->Value_ptr;
	InputNr->Minimum				= OID->Minimum;
 	InputNr->Maximum				= OID->Maximum;

	InputNr->Text					= OID->Text;

	InputNr->Symbol_width		= OID->Symbol_width;
	InputNr->Symbol_height		= OID->Symbol_height;
	InputNr->Graphics_handle	= OID->Graphics_handle;
	InputNr->Graphics_offset	= OID->Graphics_offset;

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Add number bar to window */
	Number_bar_OID.Value_ptr	= InputNr->Value_ptr;
	Number_bar_OID.Minimum		= InputNr->Minimum;
	Number_bar_OID.Maximum		= InputNr->Maximum;
	Number_bar_OID.Update		= NULL;

	Add_object
	(
		Object->Self,
		&Number_bar_Class,
		(UNBYTE *) &Number_bar_OID,
		48,
		50,
		104,
		NUMBER_BAR_HEIGHT
	);

	/* Initialize button data */
	InputNr_exit_button_data.Text_button_data.Text = System_text_ptrs[64];

	/* Add button to window */
 	Add_object
	(
		Object->Self,
		&Button_Class,
		(UNBYTE *) &InputNr_exit_button_OID,
		75,
		68,
		50,
		11
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_InputNr_object
 * FUNCTION  : Draw method of Number input window object.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 17:40
 * LAST      : 11.09.95 15:59
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_InputNr_object(struct Object *Object, union Method_parms *P)
{
	struct InputNr_object *InputNr;
	struct PA PA;
	struct OPM *OPM;
	UNSHORT Y;
	UNBYTE *Ptr;

	InputNr = (struct InputNr_object *) Object;
	OPM = &(InputNr->Window_OPM);

	/* Draw window */
	Draw_Window_object(Object, P);

	/* Any symbol ? */
	if (InputNr->Symbol_width && InputNr->Symbol_height)
	{
		/* Yes -> Calculate Y-coordinate */
		Y = 13 + (30 - InputNr->Symbol_width) / 2;

		/* Draw box around symbol */
		Draw_deep_box
		(
			OPM,
			13,
			Y - 1,
			InputNr->Symbol_width + 2,
			InputNr->Symbol_height + 2
		);

		/* Get graphics address */
		Ptr = MEM_Claim_pointer(InputNr->Graphics_handle) +
		 InputNr->Graphics_offset;

		/* Draw symbol */
		Put_masked_block(OPM, 14, Y, InputNr->Symbol_width,
		 InputNr->Symbol_height, Ptr);
		MEM_Free_pointer(InputNr->Graphics_handle);
	}

	/* Any text ? */
	if (InputNr->Text)
	{
		/* Yes -> Create print area */
		if (InputNr->Symbol_width)
			PA.Area.left = 15 + InputNr->Symbol_width + 4;
		else
			PA.Area.left = 15;
		PA.Area.top		= 14;
		PA.Area.width	= INPUTNR_WIDTH - PA.Area.left - 16;
		PA.Area.height	= 28;

		/* Draw box around text */
		Draw_deep_box(OPM, PA.Area.left - 2, PA.Area.top - 2,
		 PA.Area.width + 4, PA.Area.height + 4);

		/* Print text */
		Push_PA(&PA);
		Set_ink(SILVER_TEXT);
		Display_text(OPM, InputNr->Text);
		Pop_PA();
	}

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : InputNr_Exit
 * FUNCTION  : Accept value (button).
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.01.95 10:44
 * LAST      : 12.01.95 10:44
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
InputNr_Exit(struct Button_object *Button)
{
	Delete_object(Button->Object.Parent);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Number_bar_object
 * FUNCTION  : Initialize method of Number bar object.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.01.95 11:04
 * LAST      : 13.01.95 11:04
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Number_bar_object(struct Object *Object, union Method_parms *P)
{
	static union Button_data Number_bar_down_button_data,
	 Number_bar_up_button_data;
	static struct Button_OID Number_bar_up_button_OID = {
		TEXT_BUTTON_TYPE | CONTINUOUS_BUTTON,
		0,
		0,
		&Number_bar_up_button_data,
		Number_bar_1_up
	};
	static struct Button_OID Number_bar_down_button_OID = {
		TEXT_BUTTON_TYPE | CONTINUOUS_BUTTON,
		0,
		0,
		&Number_bar_down_button_data,
		Number_bar_1_down
	};
	static UNCHAR *Down_text = "<";
	static UNCHAR *Up_text = ">";

	struct Number_bar_object *Number_bar;
	struct Number_bar_OID *OID;

	Number_bar = (struct Number_bar_object *) Object;
	OID = (struct Number_bar_OID *) P;

	/* Copy data from OID */
	Number_bar->Value_ptr = OID->Value_ptr;
	Number_bar->Minimum = OID->Minimum;
 	Number_bar->Maximum = OID->Maximum;
 	Number_bar->Update = OID->Update;

	/* Calculate number bar slider position */
	Number_bar->Slider_X = ((*(Number_bar->Value_ptr) - Number_bar->Minimum)
	 * (Number_bar->Object.Rect.width - NUMBER_BAR_SLIDER_WIDTH - 24))
	 / (Number_bar->Maximum - Number_bar->Minimum);

	/* Initialize button data */
	Number_bar_down_button_data.Text_button_data.Text = Down_text;
	Number_bar_up_button_data.Text_button_data.Text = Up_text;

	/* Add buttons */
	Add_object(Object->Self, &Button_Class, (UNBYTE *) &Number_bar_down_button_OID,
	 0, 0, 10, NUMBER_BAR_HEIGHT);

	Add_object(Object->Self, &Button_Class, (UNBYTE *) &Number_bar_up_button_OID,
	 Object->Rect.width - 10, 0, 10, NUMBER_BAR_HEIGHT);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Number_bar_object
 * FUNCTION  : Draw method of Number bar object.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.01.95 12:37
 * LAST      : 13.01.95 12:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Number_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Number_bar_object *Number_bar;

	Number_bar = (struct Number_bar_object *) Object;

	/* Deselect slider */
	Number_bar->Flags &= ~SLIDER_SELECTED;

	/* Draw number bar */
	Do_draw_number_bar(Number_bar);

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_Number_bar_object
 * FUNCTION  : Get method of Number bar object.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.01.95 12:36
 * LAST      : 13.01.95 12:36
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : Number bar result.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_Number_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Number_bar_object *Number_bar;

	Number_bar = (struct Number_bar_object *) Object;

	return (UNLONG) *(Number_bar->Value_ptr);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_Number_bar_object
 * FUNCTION  : Set method of Number bar object.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.02.95 11:24
 * LAST      : 23.02.95 11:24
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Set_Number_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Number_bar_object *Number_bar;
	SILONG New_value;

	Number_bar = (struct Number_bar_object *) Object;

	New_value = (SILONG) P->Value;

	/* New value above maximum ? */
	if (New_value > Number_bar->Maximum)
		New_value = Number_bar->Maximum;

	/* New value below minimum ? */
	if (New_value < Number_bar->Minimum)
		New_value = Number_bar->Minimum;

	/* Set new value */
	*(Number_bar->Value_ptr) = New_value;

	/* Set slider position */
	Number_bar->Slider_X = ((*(Number_bar->Value_ptr) - Number_bar->Minimum) *
	 (Number_bar->Object.Rect.width - NUMBER_BAR_SLIDER_WIDTH - 24)) /
	 (Number_bar->Maximum - Number_bar->Minimum);

	/* Draw number bar */
	Do_draw_number_bar(Number_bar);

	/* Update display */
	Update_display();
	Switch_screens();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Number_bar_object
 * FUNCTION  : Left method of Number bar object.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.01.95 11:40
 * LAST      : 23.02.95 10:38
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Number_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Number_bar_object *Number_bar;
	struct BBRECT Drag_MA;
	UNSHORT X, mX, oldX, Offset;

	Number_bar = (struct Number_bar_object *) Object;

	/* Determine action */
	X = Object->Rect.left + 12;
	mX = Mouse_X;

	/* Still in bar area ? */
	if ((mX < X) || (mX >= X + Object->Rect.width - 24))
		return 0;

	/* Yes -> To the left of the slider ? */
	if (mX < X + Number_bar->Slider_X)
	{
		/* Yes -> Page down */
		Number_bar_page_down(Number_bar);
		return 0;
	}

	/* No -> To the right of the slider ? */
	if (mX > X + Number_bar->Slider_X + NUMBER_BAR_SLIDER_WIDTH)
	{
		/* Yes -> Page up */
		Number_bar_page_up(Number_bar);
		return 0;
	}

	/* No -> Select slider */
	Number_bar->Flags |= SLIDER_SELECTED;

	/* Make drag mouse area */
	Drag_MA.left = X;
	Drag_MA.top = Object->Rect.top;
	Drag_MA.width = Object->Rect.width - NUMBER_BAR_SLIDER_WIDTH - 24 + 1;
	Drag_MA.height = Object->Rect.height;

	/* Mouse off */
	Mouse_off();

	/* Hide HDOBs */
	Hide_HDOBs();

	/* Get slider X-coordinate */
	X = Number_bar->Slider_X;
	if (X > Object->Rect.width - NUMBER_BAR_SLIDER_WIDTH - 24)
		X = Object->Rect.width - NUMBER_BAR_SLIDER_WIDTH - 24;
	X += Object->Rect.left + 12;

	/* Save X offset */
	Offset = Mouse_X - X;

	/* Install mouse area */
	Push_MA(&Drag_MA);

	/* Set mouse position */
	SYSTEM_SetMousePtr(X, Mouse_Y);

	/* Drag slider */
	oldX = Number_bar->Slider_X;
	for (;;)
	{
		/* Update needed ? */
		if (Number_bar->Slider_X != oldX)
		{
			/* Yes -> Update */
			Update_number_slider(Number_bar, Number_bar->Slider_X);
			oldX = Number_bar->Slider_X;
		}
		else
		{
			/* No -> Just draw the number bar */
			Do_draw_number_bar(Number_bar);
		}

		/* Update display */
		Update_display();
		Switch_screens();

		/* Update input handling */
		Update_input();

		/* Exit if left button isn't pressed */
		if (!(Button_state & 0x0001))
			break;

		/* Set slider position */
		Number_bar->Slider_X = Mouse_X - (Object->Rect.left + 12);
	}

	/* Remove mouse area */
	Pop_MA();

	/* Reset mouse position */
	SYSTEM_SetMousePtr(Mouse_X + Offset, Mouse_Y);

	/* Mouse on */
	Mouse_on();

	/* Deselect slider */
	Number_bar->Flags &= ~SLIDER_SELECTED;

	/* Set slider position */
	Number_bar->Slider_X = ((*(Number_bar->Value_ptr) - Number_bar->Minimum)
	 * (Number_bar->Object.Rect.width - NUMBER_BAR_SLIDER_WIDTH - 24))
	 / (Number_bar->Maximum - Number_bar->Minimum);

	/* Re-draw number bar */
	Do_draw_number_bar(Number_bar);

	/* Show HDOBs */
	Show_HDOBs();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Number_bar_page_up
 * FUNCTION  :	Move the slider up one page.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.01.95 12:31
 * LAST      : 23.02.95 10:38
 * INPUTS    : struct Number_bar_object *Number_bar - Pointer to number bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Number_bar_page_up(struct Number_bar_object *Number_bar)
{
	UNSHORT Page;

	/* Already reached maximum ? */
	if (*(Number_bar->Value_ptr) < Number_bar->Maximum)
	{
		/* No -> Page up */
		Page = Number_bar->Maximum - Number_bar->Minimum;
		if (Page >= 10)
			*(Number_bar->Value_ptr) += (SILONG) (Page / 10);
		else
			*(Number_bar->Value_ptr) += 1;

		if (*(Number_bar->Value_ptr) > Number_bar->Maximum)
			*(Number_bar->Value_ptr) = Number_bar->Maximum;

		/* Set slider position */
		Number_bar->Slider_X = ((*(Number_bar->Value_ptr) - Number_bar->Minimum)
		 * (Number_bar->Object.Rect.width - NUMBER_BAR_SLIDER_WIDTH - 24))
		 / (Number_bar->Maximum - Number_bar->Minimum);

		/* Select slider */
		Light_number_slider(Number_bar);

		/* Wait */
		Wait_4_unclick();

		/* Draw number bar */
		Do_draw_number_bar(Number_bar);

		/* Update */
		if (Number_bar->Update)
			(Number_bar->Update)(Number_bar);

		/* Update display */
		Update_display();
		Switch_screens();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Number_bar_page_down
 * FUNCTION  :	Move the slider down one page.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.01.95 12:31
 * LAST      : 23.02.95 10:38
 * INPUTS    : struct Number_bar_object *Number_bar - Pointer to number bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Number_bar_page_down(struct Number_bar_object *Number_bar)
{
	UNSHORT Page;

	/* Already reached minimum ? */
	if (*(Number_bar->Value_ptr) > Number_bar->Minimum)
	{
		/* No -> Page down */
		Page = Number_bar->Maximum - Number_bar->Minimum;
		if (Page > 10)
			*(Number_bar->Value_ptr) -= (SILONG) (Page / 10);
		else
			*(Number_bar->Value_ptr) -= 1;

		if (*(Number_bar->Value_ptr) < Number_bar->Minimum)
			*(Number_bar->Value_ptr) = Number_bar->Minimum;

		/* Set slider position */
		Number_bar->Slider_X = ((*(Number_bar->Value_ptr) - Number_bar->Minimum)
		 * (Number_bar->Object.Rect.width - NUMBER_BAR_SLIDER_WIDTH - 24))
		 / (Number_bar->Maximum - Number_bar->Minimum);

		/* Select slider */
		Light_number_slider(Number_bar);

		/* Wait */
		Wait_4_unclick();

		/* Draw number bar */
		Do_draw_number_bar(Number_bar);

		/* Update */
		if (Number_bar->Update)
			(Number_bar->Update)(Number_bar);

		/* Update display */
		Update_display();
		Switch_screens();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Light_number_slider
 * FUNCTION  : Light a number bar's slider.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.01.95 11:36
 * LAST      : 13.01.95 11:36
 * INPUTS    : struct Number_bar_object *Number_bar - Pointer to number bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Light_number_slider(struct Number_bar_object *Number_bar)
{
	/* Select slider */
	Number_bar->Flags |= SLIDER_SELECTED;

	/* Draw number bar */
	Do_draw_number_bar(Number_bar);

	/* Update display */
	Update_display();
	Switch_screens();

	/* Deselect slider */
	Number_bar->Flags &= ~SLIDER_SELECTED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_number_slider
 * FUNCTION  : Update a number bar's slider.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.01.95 11:34
 * LAST      : 13.01.95 11:34
 * INPUTS    : struct Number_bar_object *Number_bar - Pointer to number bar
 *              object.
 *             UNSHORT X - New slider X-position.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_number_slider(struct Number_bar_object *Number_bar, UNSHORT X)
{
	/* Too far right ? */
	if (X > Number_bar->Object.Rect.width - NUMBER_BAR_SLIDER_WIDTH - 24)
		X = Number_bar->Object.Rect.width - NUMBER_BAR_SLIDER_WIDTH - 24;

	/* Set variables */
	Number_bar->Slider_X = X;
	*(Number_bar->Value_ptr) = (X * (Number_bar->Maximum - Number_bar->Minimum)
	 / (Number_bar->Object.Rect.width - NUMBER_BAR_SLIDER_WIDTH - 24))
	 + Number_bar->Minimum;

	/* Re-draw number bar */
	Do_draw_number_bar(Number_bar);

	/* Update */
	if (Number_bar->Update)
		(Number_bar->Update)(Number_bar);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_draw_number_bar
 * FUNCTION  : Draw a number bar.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.01.95 11:29
 * LAST      : 13.01.95 12:19
 * INPUTS    : struct Number_bar_object *Number_bar - Pointer to number bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_draw_number_bar(struct Number_bar_object *Number_bar)
{
	UNSHORT X, Y, W, H;
	UNSHORT i;
	UNCHAR String[10];

	/* Select colours */
	i = (Number_bar->Flags & SLIDER_SELECTED) ? 1 : 0;

	/* Get coordinates and dimensions */
	X = Number_bar->Object.X + 12;
	Y = Number_bar->Object.Y;
	W = Number_bar->Object.Rect.width - 24;
	H = Number_bar->Object.Rect.height;

	/* Draw inside */
	Draw_window_inside
	(
		Current_OPM,
		X - 1,
		Y - 1,
		W + 2,
		H + 2
	);

	/* Draw surrounding box */
	Draw_deep_box
	(
		Current_OPM,
		X - 1,
		Y - 1,
		W + 2,
		H + 2
	);

	/* Draw top shadow */
	Put_recoloured_box
	(
		Current_OPM,
		X,
		Y,
		W,
		1,
		&(Recolour_tables[2][0])
	);

	/* Draw left shadow */
	Put_recoloured_box
	(
		Current_OPM,
		X,
		Y + 1,
		1,
		H - 1,
		&(Recolour_tables[2][0])
	);

	/* Draw slider */
	X += Number_bar->Slider_X;
	W = NUMBER_BAR_SLIDER_WIDTH;

	OPM_SetPixel(Current_OPM, X, Y, Slider_colours[i][0]);
	OPM_HorLine(Current_OPM, X+1, Y, W-2, Slider_colours[i][1]);
	OPM_SetPixel(Current_OPM, X+W-1, Y, Slider_colours[i][2]);

	OPM_VerLine(Current_OPM, X, Y+1, H-2, Slider_colours[i][3]);
	OPM_FillBox(Current_OPM, X+1, Y+1, W-2, H-2, Slider_colours[i][4]);
	OPM_VerLine(Current_OPM, X+W-1, Y+1, H-2, Slider_colours[i][5]);

	OPM_SetPixel(Current_OPM, X, Y+H-1, Slider_colours[i][6]);
	OPM_HorLine(Current_OPM, X+1, Y+H-1, W-2, Slider_colours[i][7]);
	OPM_SetPixel(Current_OPM, X+W-1, Y+H-1, Slider_colours[i][8]);

	/* Print value on slider */
	Set_ink(GOLD_TEXT);
	sprintf(String, "%ld", *(Number_bar->Value_ptr));
	Print_centered_string(Current_OPM, X, Y+2, W, String);

	/* Is the slider at the far right ? */
	if (Number_bar->Slider_X < Number_bar->Object.Rect.width -
	 NUMBER_BAR_SLIDER_WIDTH - 24)
	{
		/* No -> Draw shadow to the right of the slider */
		Put_recoloured_box(Current_OPM, X+W, Y+1, 1, H-1, &(Recolour_tables[2][0]));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Number_bar_1_up
 * FUNCTION  : Increase value by one (button).
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.02.95 11:22
 * LAST      : 23.02.95 11:22
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Number_bar_1_up(struct Button_object *Button)
{
	union Method_parms P;
	struct Number_bar_object *Number_bar;
	SILONG Value;

	/* Get number bar data */
	Number_bar = (struct Number_bar_object *) Get_object_data(Button->Object.Parent);

	/* Get current value */
	Value = (SILONG) Execute_method(Button->Object.Parent, GET_METHOD, NULL);

	/* Can the value be increased ? */
	if (Value < Number_bar->Maximum)
	{
		/* Yes -> Is this the first time ? */
		if (Button->State)
		{
			/* Yes -> Increase the value by a whole step */
			Value++;

			/* Initialize the delta adder */
			Number_bar->Delta_add = FIRST_ADD;
		}
		else
		{
			/* No -> Increase the value by the delta adder */
			Value += Number_bar->Delta_add / 256;

			/* Too high ? */
			if (Value > Number_bar->Maximum)
			{
				/* Yes -> Set to maximum */
				Value = Number_bar->Maximum;

				/* Clear the delta adder */
				Number_bar->Delta_add = 0;
			}
			else
			{
				/* No -> Faster baby */
				Number_bar->Delta_add += ADD_DELTA;

				/* But not TOO fast */
				if (Number_bar->Delta_add > MAX_DELTA_ADD)
				{
					Number_bar->Delta_add = MAX_DELTA_ADD;
				}
			}
		}
	}

	/* Set new value */
	P.Value = (UNLONG) Value;
	Execute_method(Button->Object.Parent, SET_METHOD, &P);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Number_bar_1_down
 * FUNCTION  : Decrease value by one (button).
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.02.95 11:22
 * LAST      : 23.02.95 11:22
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Number_bar_1_down(struct Button_object *Button)
{
	union Method_parms P;
	struct Number_bar_object *Number_bar;
	SILONG Value;

	/* Get number bar data */
	Number_bar = (struct Number_bar_object *) Get_object_data(Button->Object.Parent);

	/* Get current value */
	Value = (SILONG) Execute_method(Button->Object.Parent, GET_METHOD, NULL);

	/* Can the value be decreased ? */
	if (Value > Number_bar->Minimum)
	{
		/* Yes -> Is this the first time ? */
		if (Button->State)
		{
			/* Yes -> Decrease the value by a whole step */
			Value--;

			/* Initialize the delta adder */
			Number_bar->Delta_add = -FIRST_ADD;
		}
		else
		{
			/* No -> Decrease the value by the delta adder */
			Value += Number_bar->Delta_add / 256;

			/* Too low ? */
			if (Value < Number_bar->Minimum)
			{
				/* Yes -> Set to minimum */
				Value = Number_bar->Minimum;

				/* Clear the delta adder */
				Number_bar->Delta_add = 0;
			}
			else
			{
				/* No -> Faster baby */
				Number_bar->Delta_add -= ADD_DELTA;

				/* But not TOO fast */
				if (Number_bar->Delta_add < -MAX_DELTA_ADD)
				{
					Number_bar->Delta_add = -MAX_DELTA_ADD;
				}
			}
		}
	}

	/* Set new value */
	P.Value = (UNLONG) Value;
	Execute_method(Button->Object.Parent, SET_METHOD, &P);
}

