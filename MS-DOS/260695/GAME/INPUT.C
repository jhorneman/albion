/************
 * NAME     : INPUT.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 11-1-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : INPUT.H, USERFACE.C
 ************/

/* includes */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <HDOB.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <INPUT.H>
#include <BUTTONS.H>
#include <STATAREA.H>
#include <GAMETEXT.H>
#include <ITEMLIST.H>
#include <ITMLOGIC.H>
#include <2D_DISPL.H>

/* defines */

#define INPUTNR_WIDTH	(200)
#define INPUTNR_HEIGHT	(92)

#define BOOLEANREQ_WIDTH	(200)
#define BOOLEANREQ_HEIGHT	(74)

#define MEMBERSELWIN_WIDTH		(270)
#define MEMBERSELWIN_HEIGHT	(99)

#define ITEMSELWIN_WIDTH	(213)
#define ITEMSELWIN_HEIGHT	(120)

#define ADD_DELTA			(8)
#define FIRST_ADD			(256 - (20 * ADD_DELTA))
#define MAX_DELTA_ADD	(16384)

#define NUMBER_BAR_HEIGHT			(12)
#define NUMBER_BAR_SLIDER_WIDTH	(29)

/* Text window parameters */
#define TEXT_WINDOW_X			(20)
#define TEXT_WINDOW_Y			(0)
#define TEXT_WINDOW_WIDTH		(320)
#define TEXT_WINDOW_HEIGHT		(182)

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

/* Party member select window OID */
struct MemberSel_window_OID {
	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;
};

/* Party member select window object */
struct MemberSel_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;
};

/* Selectable party member OID */
struct SelMember_OID {
	UNSHORT Member_index;
};

/* Selectable party member object */
struct SelMember_object {
	struct Object Object;
	UNSHORT Member_index;
};

/* Item select window OID */
struct ItemSel_window_OID {
	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;

	MEM_HANDLE Slots_handle;
	UNLONG Slots_offset;
};

/* Item select window object */
struct ItemSel_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;
};

/* prototypes */

/* Input module functions */
void Init_input(void);
void Exit_input(void);
void Display_input(void);

/* Input object methods */
UNLONG Customkeys_input(struct Object *Object, union Method_parms *P);
UNLONG Right_input(struct Object *Object, union Method_parms *P);

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

/* Boolean requester object methods */
UNLONG Init_BooleanReq_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_BooleanReq_object(struct Object *Object, union Method_parms *P);

/* Boolean requester button handlers */
void BooleanReq_Yes(struct Button_object *Button);
void BooleanReq_No(struct Button_object *Button);

/* Party member select window object methods */
UNLONG Init_MemberSel_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_MemberSel_object(struct Object *Object, union Method_parms *P);

/* Selectable party member object methods */
UNLONG Init_SelMember_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_SelMember_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_SelMember_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_SelMember_object(struct Object *Object, union Method_parms *P);
UNLONG Help_SelMember_object(struct Object *Object, union Method_parms *P);
UNLONG Left_SelMember_object(struct Object *Object, union Method_parms *P);

/* Item select window object methods */
UNLONG Init_ItemSel_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_ItemSel_object(struct Object *Object, union Method_parms *P);

/* Selectable item object methods */
UNLONG Draw_SelItem_object(struct Object *Object, union Method_parms *P);
UNLONG Update_SelItem_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_SelItem_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_SelItem_object(struct Object *Object, union Method_parms *P);
UNLONG Left_SelItem_object(struct Object *Object, union Method_parms *P);

/* global variables */

/* Input variables */
static struct PA Input_PA;
static BOOLEAN Input_aborted;
static UNSHORT Max_input_length;
static UNSHORT Current_input_length;
static UNSHORT Input_object;
static UNSHORT Cursor_X;
static UNCHAR Input_buffer[200];

/* Input module */
static struct Module Input_Mod = {
	LOCAL_MOD, MODE_MOD, NO_SCREEN,
	Update_display,
	Init_input, Exit_input,
	NULL, NULL,
	Display_input
};

/* Input method list */
static struct Method Input_methods[] = {
	{ RIGHT_METHOD, Right_input },
	{ CUSTOMKEY_METHOD, Customkeys_input },
	{ 0, NULL}
};

/* Input class description */
static struct Object_class Input_Class = {
	0, sizeof(struct Object),
	&Input_methods[0]
};

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

/* Member selection window method list */
static struct Method MemberSel_methods[] = {
	{ INIT_METHOD, Init_MemberSel_object },
	{ EXIT_METHOD, Exit_Window_object },
	{ DRAW_METHOD, Draw_MemberSel_object },
	{ UPDATE_METHOD, Update_help_line },
	{ RIGHT_METHOD, Close_Window_object },
	{ CLOSE_METHOD, Close_Window_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ RESTORE_METHOD, Restore_window },
	{ 0, NULL}
};

/* Member selection window class description */
static struct Object_class MemberSel_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct MemberSel_window_object),
	&MemberSel_methods[0]
};

/* Selectable member method list */
static struct Method SelMember_methods[] = {
	{ INIT_METHOD, Init_SelMember_object },
	{ DRAW_METHOD, Draw_SelMember_object },
	{ FEEDBACK_METHOD, Feedback_SelMember_object },
	{ HIGHLIGHT_METHOD, Highlight_SelMember_object },
	{ HELP_METHOD, Help_SelMember_object },
	{ LEFT_METHOD, Left_SelMember_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

/* Selectable member class description */
static struct Object_class SelMember_Class = {
	0, sizeof(struct SelMember_object),
	&SelMember_methods[0]
};

/* Item selection window method list */
static struct Method ItemSel_methods[] = {
	{ INIT_METHOD, Init_ItemSel_object },
	{ EXIT_METHOD, Exit_Window_object },
	{ DRAW_METHOD, Draw_ItemSel_object },
	{ UPDATE_METHOD, Update_help_line },
	{ RIGHT_METHOD, Close_Window_object },
	{ CLOSE_METHOD, Close_Window_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ RESTORE_METHOD, Restore_window },
	{ 0, NULL}
};

/* Item selection window class description */
static struct Object_class ItemSel_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct ItemSel_window_object),
	&ItemSel_methods[0]
};

/* Selectable item slot method list */
static struct Method SelItem_slot_methods[] = {
	{ INIT_METHOD, Init_Item_slot_object },
	{ DRAW_METHOD, Draw_SelItem_object },
	{ UPDATE_METHOD, Update_SelItem_object },
	{ FEEDBACK_METHOD, Feedback_SelItem_object },
	{ HIGHLIGHT_METHOD, Highlight_SelItem_object },
	{ HELP_METHOD, Help_Item_slot_object },
	{ TOUCHED_METHOD, Touch_Item_slot_object },
	{ LEFT_METHOD, Left_SelItem_object },
	{ 0, NULL}
};

/* Selectable item slot class description */
static struct Object_class SelItem_slot_Class = {
	0, sizeof(struct Item_slot_object),
	&SelItem_slot_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Input_string
 * FUNCTION  : Input a string.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 12:23
 * LAST      : 14.09.94 12:23
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 *             UNSHORT Width - Width of input area.
 *             UNSHORT Max_length - Maximum length of input string in characters.
 *             UNCHAR *Buffer - Pointer to destination buffer.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - A fitting PA will be installed.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Input_string(UNSHORT X, UNSHORT Y, UNSHORT Width, UNSHORT Max_length,
 UNCHAR *Buffer)
{
	Wait_4_unclick();

	/* Store input */
	Max_input_length = Max_length;

	/* Build PA */
	Input_PA.Area.left = X;
	Input_PA.Area.top = Y;
	Input_PA.Area.width = Width;
	Input_PA.Area.height = STANDARD_TEXT_HEIGHT;

	/* Clear buffer */
	Input_buffer[0] = 0;
	Current_input_length = 0;
	Cursor_X = 0;

	/* Input */
	Push_module(&Input_Mod);

	/* Was the input aborted ? */
	if (Input_aborted)
	{
		/* Yes -> Return empty string */
		Buffer[0] = 0;
	}
	else
	{
		/* No -> Copy buffer to output string */
		strncpy(Buffer, &(Input_buffer[0]), Max_input_length);
	}
}

/*
;*****************************************************************************
; [ Edit an existing string ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Maximum length of input string (.w)
;        a0 - Pointer to original string (.l)
; All registers are restored
; NOTE :
;  - A fitting PA will be installed.
;*****************************************************************************
Edit_string:
	movem.l	d0/d1/a0/a1,-(sp)
	jsr	Wait_4_unclick
	jsr	Prepare_input		; Prepare input
	move.l	a0,Original_input
	jsr	Fill_input_buffer		; Fill input buffer
	Push	Module,Input_Mod		; Input
	tst.b	Input_aborted		; Aborted ?
	bne	.Exit
	move.w	Max_length,d1		; Copy back to original
	lea.l	Input_buffer,a0
	move.l	Original_input,a1
	jsr	Strlen			; Get new string length
	cmp.w	d0,d1			; Too long ?
	bpl.s	.No
	move.w	d1,d0			; Yes -> First part only
.No:	exg.l	a0,a1			; Copy
	jsr	Strncpy
	clr.b	(a0)			; Insert EOL
.Exit:	movem.l	(sp)+,d0/d1/a0/a1
	rts

;*****************************************************************************
; [ Fill input buffer (edit) ]
; All registers are restored
;*****************************************************************************
Fill_input_buffer:
	movem.l	d0/d2/a0/a1,-(sp)
	move.w	Max_length,d1		; Copy original to buffer
	move.l	Original_input,a0
	lea.l	Input_buffer,a1
	jsr	Strlen			; Get original string length
	cmp.w	d0,d1			; Too long ?
	bpl.s	.No
	move.w	d1,d0			; Yes -> First part only
.No:	move.w	d0,Current_length		; Set length
	move.w	d0,d1			; Adjust cursor
	mulu.w	#Char_width+1,d1
	add.w	d1,Cursor_X
	exg.l	a0,a1			; Copy
	jsr	Strncpy
	clr.b	(a0)			; Insert EOL
	movem.l	(sp)+,d0/d1/a0/a1
	rts

*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	Init_input
 * FUNCTION  : Init input module.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 15:15
 * LAST      : 14.09.94 15:15
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_input(void)
{
	Mouse_off();

	Input_aborted = FALSE;

	Push_PA(&Input_PA);
	Push_MA(&(Input_PA.Area));

	Push_root(&Main_OPM);

	Input_object = Add_object(0, &Input_Class, NULL, Input_PA.Area.left,
	 Input_PA.Area.top, Input_PA.Area.width, Input_PA.Area.height);

	Highlighted_object = Input_object;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	Exit_input
 * FUNCTION  : Exit input module.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 15:15
 * LAST      : 14.09.94 15:15
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_input(void)
{
	Delete_object(Input_object);

	Pop_MA();
	Pop_PA();

	Pop_root();

	Mouse_on();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_input
 * FUNCTION  : Display input line.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 14:38
 * LAST      : 14.09.94 14:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_input(void)
{
	static BOOLEAN Cursor_state = FALSE;
	static UNSHORT Cursor_timer = 0;

	/* Erase input area */
//	Erase_print_area(Current_OPM);

	/* Print input string */
	Print_string(Current_OPM, Input_PA.Area.left, Input_PA.Area.top,
	 &(Input_buffer[0]));

	/* Time to blink cursor ? */
	if (!Cursor_timer)
	{
		/* Yes -> Reset timer */
		Cursor_timer = 4;

		/* Toggle state */
		Cursor_state = ~Cursor_state;
	}
	else
		/* No -> Count down */
		Cursor_timer--;

	/* Show cursor ? */
	if (Cursor_state)
	{
		/* Yes */
		OPM_FillBox(Current_OPM, Input_PA.Area.left + Cursor_X + 1,
		 Input_PA.Area.top, 5, STANDARD_TEXT_HEIGHT, WHITE);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Customkeys_input
 * FUNCTION  : Custom keys method of input object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 14:17
 * LAST      : 14.09.94 14:17
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Customkeys_input(struct Object *Object, union Method_parms *P)
{
	UNSHORT Key_code;

	/* Get key-code */
	Key_code = (UNSHORT) (P->Event->sl_key_code);

	switch (Key_code)
	{
		/* Clear input buffer */
		case BLEV_ESC:
			Input_buffer[0] = 0;
			Current_input_length = 0;
			Cursor_X = 0;
			break;
		/* Backspace */
		case BLEV_BS:
			/* Are there characters in the input buffer ? */
			if (Current_input_length)
			{
				/* Yes -> Remove the last one */
				Current_input_length--;
				Input_buffer[Current_input_length] = 0;
				Cursor_X = Get_line_width(&(Input_buffer[0]));
			}
			break;
		/* Done */
		case BLEV_RETURN:
			Pop_module();
			break;
		/* Default -> Key */
		default:
			/* Is there room in the input buffer ? */
			if (Current_input_length < Max_input_length)
			{
				/* Yes */
				Key_code &= 0x00FF;

				/* Exit if illegal character */
				if (Key_code < 32)
					break;

				/* Insert new character in input buffer */
				Input_buffer[Current_input_length] = Key_code;
				Current_input_length++;
				Input_buffer[Current_input_length] = 0;

				Cursor_X = Get_line_width(&(Input_buffer[0]));

				/* Is there room on screen ? */
				if (Cursor_X >= Input_PA.Area.width - 5)
				{
					/* No -> Remove character */
					Current_input_length--;
					Input_buffer[Current_input_length] = 0;
					Cursor_X = Get_line_width(&(Input_buffer[0]));
				}
			}
			break;
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Right_input
 * FUNCTION  : Right mouse button method of input object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 14:32
 * LAST      : 14.09.94 14:32
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Right_input(struct Object *Object, union Method_parms *P)
{
	Input_aborted = TRUE;
	Wait_4_unclick();
	Pop_module();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Input_number
 * FUNCTION  : Display and handle a number input window.
 * FILE      : INPUT.C
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
	OID.Value_ptr = &It;
	OID.Minimum = Minimum;
	OID.Maximum = Maximum;

	OID.Text = Text;

	OID.Symbol_width = 0;
	OID.Symbol_height = 0;
	OID.Graphics_handle = 0;
	OID.Graphics_offset = 0;

	/* Do it */
	Push_module(&Window_Mod);

	Obj = Add_object(0, &InputNr_Class, (UNBYTE *) &OID,
	 (Screen_width - INPUTNR_WIDTH) / 2, (Panel_Y - INPUTNR_HEIGHT) / 2,
	 INPUTNR_WIDTH, INPUTNR_HEIGHT);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);

	return(It);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Input_number_with_symbol
 * FUNCTION  : Display and handle a number input window with a symbol.
 * FILE      : INPUT.C
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
	OID.Value_ptr = &It;
	OID.Minimum = Minimum;
	OID.Maximum = Maximum;

	OID.Text = Text;

	OID.Symbol_width = Width;
	OID.Symbol_height = Height;
	OID.Graphics_handle = Graphics_handle;
	OID.Graphics_offset = Graphics_offset;

	/* Do it */
	Push_module(&Window_Mod);

	Obj = Add_object(0, &InputNr_Class, (UNBYTE *) &OID,
	 (Screen_width - INPUTNR_WIDTH) / 2, (Panel_Y - INPUTNR_HEIGHT) / 2,
	 INPUTNR_WIDTH, INPUTNR_HEIGHT);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);

	return(It);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_InputNr_object
 * FUNCTION  : Initialize method of Number input window object.
 * FILE      : INPUT.C
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
		69,
		&InputNr_exit_button_data,
		InputNr_Exit
	};

	struct InputNr_object *InputNr;
	struct InputNr_OID *OID;
	struct Number_bar_OID Number_bar_OID;

	InputNr = (struct InputNr_object *) Object;
	OID = (struct InputNr_OID *) P;

	/* Copy data from OID */
	InputNr->Value_ptr = OID->Value_ptr;
	InputNr->Minimum = OID->Minimum;
 	InputNr->Maximum = OID->Maximum;

	InputNr->Text = OID->Text;

	InputNr->Symbol_width = OID->Symbol_width;
	InputNr->Symbol_height = OID->Symbol_height;
	InputNr->Graphics_handle = OID->Graphics_handle;
	InputNr->Graphics_offset = OID->Graphics_offset;

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Add number bar to window */
	Number_bar_OID.Value_ptr = InputNr->Value_ptr;
	Number_bar_OID.Minimum = InputNr->Minimum;
	Number_bar_OID.Maximum = InputNr->Maximum;
	Number_bar_OID.Update = NULL;

	Add_object(Object->Self, &Number_bar_Class, (UNBYTE *) &Number_bar_OID,
	 48, 50, 104, NUMBER_BAR_HEIGHT);

	/* Initialize button data */
	InputNr_exit_button_data.Text_button_data.Text = System_text_ptrs[64];

	/* Add button to window */
 	Add_object(Object->Self, &Button_Class, (UNBYTE *) &InputNr_exit_button_OID,
	 75, 68, 50, 11);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_InputNr_object
 * FUNCTION  : Draw method of Number input window object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 17:40
 * LAST      : 11.01.95 17:40
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
	UNSHORT Y, W, H;
	UNBYTE *Ptr;

	InputNr = (struct InputNr_object *) Object;
	OPM = &(InputNr->Window_OPM);

	W = Object->Rect.width;
	H = Object->Rect.height;

	/* Draw window's shadow */
	Put_recoloured_box(OPM, 10, 10, W - 10, H - 10, &(Recolour_tables[1][0]));

	/* Draw window */
	Draw_window_inside(OPM, 7, 7, W - 14, H - 14);
	Draw_window_border(OPM, 0, 0, W, H);

	/* Any symbol ? */
	if (InputNr->Symbol_width && InputNr->Symbol_height)
	{
		/* Yes -> Calculate Y-coordinate */
		Y = 13 + (30 - InputNr->Symbol_width) / 2;

		/* Draw box around symbol */
		Draw_deep_box(OPM, 13, Y-1, InputNr->Symbol_width + 1,
		 InputNr->Symbol_height + 1);

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
		PA.Area.top = 14;
		PA.Area.width = INPUTNR_WIDTH - PA.Area.left - 16;
		PA.Area.height = 28;

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
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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

	return((UNLONG) *(Number_bar->Value_ptr));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_Number_bar_object
 * FUNCTION  : Set method of Number bar object.
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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
	Draw_window_inside(Current_OPM, X-1, Y-1, W+2, H+2);

	/* Draw surrounding box */
	Draw_deep_box(Current_OPM, X-1, Y-1, W+2, H+2);

	/* Draw top shadow */
	Put_recoloured_box(Current_OPM, X, Y, W, 1, &(Recolour_tables[2][0]));

	/* Draw left shadow */
	Put_recoloured_box(Current_OPM, X, Y+1, 1, H-1, &(Recolour_tables[2][0]));

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
	if (Number_bar->Slider_X < Number_bar->Object.Rect.width
	 - NUMBER_BAR_SLIDER_WIDTH - 24)
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
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_text_file_window
 * FUNCTION  : Draw and handle a text window containing a text block from
 *              a text file.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.02.95 12:59
 * LAST      : 22.02.95 15:11
 * INPUTS    : MEM_HANDLE Text_file_handle - Memory handle of text file.
 *             UNSHORT Text_block_nr - Text block number.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_text_file_window(MEM_HANDLE Text_file_handle, UNSHORT Text_block_nr)
{
	UNBYTE *Ptr;

	/* Get text file address */
	Ptr = MEM_Claim_pointer(Text_file_handle);

	/* Find text block */
	Ptr = Find_text_block(Ptr, Text_block_nr);

	/* Do text window */
	Do_text_window(Ptr);

	MEM_Free_pointer(Text_file_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_text_window
 * FUNCTION  : Draw and handle a text window.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.02.95 12:59
 * LAST      : 22.02.95 15:11
 * INPUTS    : UNBYTE *Ptr - Pointer to text.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_text_window(UNBYTE *Ptr)
{
	struct Processed_text Processed;
	struct PA PA;
	struct BBRECT MA;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	UNSHORT Window_Y, Window_height;

	/* Install text-style */
	Push_textstyle(&Default_text_style);

	/* Create PA */
	PA.Area.top = 0;
	PA.Area.left = TEXT_WINDOW_X + 10;
	PA.Area.width = TEXT_WINDOW_WIDTH - 20;
	PA.Area.height = TEXT_WINDOW_HEIGHT - 20;

	/* Process the text block */
	Push_PA(&PA);
	Process_text(Ptr, &Processed);
	Pop_PA();

	/* Is the text too large for the window ? */
	Window_height = TEXT_WINDOW_HEIGHT;
	if ((SILONG) Processed.Text_height < (SILONG)(Window_height - 20))
	{
		/* No -> Adjust window height */
		Window_height = Processed.Text_height + 20;
	}

	/* Is the window too small ? */
	if (Window_height < 64)
	{
		/* Yes -> Set to minimum height */
		Window_height = 64;
	}

	/* Centre window on screen */
	Window_Y = TEXT_WINDOW_Y + (TEXT_WINDOW_HEIGHT - Window_height) / 2;

	/* Adjust PA */
	PA.Area.top = Window_Y + ((Window_height - 20)
	 - Processed.Text_height) / 2 + 10;
	PA.Area.height = Window_height - 20;

	/* Create MA */
	MA.top = Window_Y;
	MA.left = TEXT_WINDOW_X;
	MA.width = TEXT_WINDOW_WIDTH;
	MA.height = Window_height;

	/* Current screen is 2D map ? */
	if (Current_screen_type(0) == MAP_2D_SCREEN)
	{
		/* Yes -> Prepare screen */
		Current_2D_OPM = &Main_OPM;
		Draw_2D_scroll_buffer(MAP_2D_X, MAP_2D_Y);
		Current_2D_OPM = NULL;
	}

	/* Install stuff */
	Push_root(&Main_OPM);
	Push_module(&Default_module);
	Push_MA(&MA);
	Push_mouse(&(Mouse_pointers[DEFAULT_MPTR]));
	Push_PA(&PA);

	/* Make background buffer and OPM */
	Background_handle = MEM_Allocate_memory(TEXT_WINDOW_WIDTH * Window_height);

	Ptr = MEM_Claim_pointer(Background_handle);
	OPM_New(TEXT_WINDOW_WIDTH, Window_height, 1, &Background_OPM, Ptr);
	MEM_Free_pointer(Background_handle);

	/* Save background */
	OPM_CopyOPMOPM(&Main_OPM, &Background_OPM, TEXT_WINDOW_X, Window_Y,
	 TEXT_WINDOW_WIDTH, Window_height, 0, 0);

	/* Draw window's shadow */
	Put_recoloured_box(&Main_OPM, TEXT_WINDOW_X + 10,
	 Window_Y + Window_height - 5, TEXT_WINDOW_WIDTH - 10, 5,
	 &(Recolour_tables[0][0]));
	Put_recoloured_box(&Main_OPM, TEXT_WINDOW_X + TEXT_WINDOW_WIDTH - 5,
	 Window_Y + 10, 5, Window_height - 15, &(Recolour_tables[0][0]));

	/* Draw window */
	Put_recoloured_box(&Main_OPM, TEXT_WINDOW_X + 7, Window_Y + 7,
	 TEXT_WINDOW_WIDTH - 14, Window_height - 14, &(Recolour_tables[1][0]));
	Draw_window_border(&Main_OPM, TEXT_WINDOW_X, Window_Y,
	 TEXT_WINDOW_WIDTH, Window_height);

	/* Print text */
	Text_wait_flag = TRUE;
	Display_processed_text(&Main_OPM, &Processed);
	Text_wait_flag = FALSE;
	Destroy_processed_text(&Processed);

	/* Restore background */
	OPM_CopyOPMOPM(&Background_OPM, &Main_OPM, 0, 0, TEXT_WINDOW_WIDTH,
	 Window_height, TEXT_WINDOW_X, Window_Y);

	/* Destroy background buffer and OPM */
	OPM_Del(&Background_OPM);
	MEM_Free_memory(Background_handle);

	/* Remove stuff */
	Pop_PA();
	Pop_MA();
	Pop_mouse();
	Pop_module();
	Pop_root();

	/* Current screen is 2D map ? */
	if (Current_screen_type(0) == MAP_2D_SCREEN)
	{
		/* Yes -> Prepare screen */
		Draw_2D_scroll_buffer(MAP_2D_X, MAP_2D_Y);
		Switch_screens();
	}

	/* Remove text-style */
	Pop_textstyle();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Boolean_requester
 * FUNCTION  : Display and handle a boolean requester window.
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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

	return(Result);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_BooleanReq_object
 * FUNCTION  : Initialize method of Boolean requester window object.
 * FILE      : INPUT.C
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
		0,
		&BooleanReq_YES_button_data,
		BooleanReq_Yes
	};
	static struct Button_OID BooleanReq_NO_button_OID = {
		TEXT_BUTTON_TYPE,
		0,
		0,
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
 * FILE      : INPUT.C
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
 * FILE      : INPUT.C
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
	BooleanReq = (struct BooleanReq_object *) Get_object_data(Button->Object.Parent);

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
 * FILE      : INPUT.C
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
	BooleanReq = (struct BooleanReq_object *) Get_object_data(Button->Object.Parent);

	/* Set result to FALSE */
	*(BooleanReq->Result_ptr) = FALSE;

	/* Exit requester */
	Delete_object(BooleanReq->Object.Self);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_party_member
 * FUNCTION  : Select a party member.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.04.95 17:02
 * LAST      : 28.04.95 14:05
 * INPUTS    : UNCHAR *Text - Pointer to window text.
 *             Member_evaluator Evaluator - Pointer to evaluation function.
 * RESULT    : UNSHORT : Selected member (1...6)  / 0 = cancelled.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Select_party_member(UNCHAR *Text, Member_evaluator Evaluator)
{
	struct MemberSel_window_OID OID;
	UNSHORT Blocked_message_nrs[6];
	UNSHORT Selected_member = 0xFFFF;
	UNSHORT Counter;
	UNSHORT Obj;
	UNSHORT i;

	/* Evaluate party members */
	Counter = 0;
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (PARTY_DATA.Member_nrs[i])
		{
			/* Yes -> Any evaluation function ? */
			if (Evaluator)
			{
				/* Yes -> Evaluate party member */
				Blocked_message_nrs[i] = (Evaluator)(Party_char_handles[i]);

				/* Absent ? */
				if (Blocked_message_nrs[i] != 0xFFFF)
				{
					/* No -> Count up */
					Counter++;
				}
			}
			else
			{
				/* No -> Party member is present */
				Blocked_message_nrs[i] = 0;

				/* Count up */
				Counter++;
			}
		}
		else
		{
			/* No -> Absent */
			Blocked_message_nrs[i] = 0xFFFF;
		}
	}

	/* Anyone to choose from ? */
	if (Counter)
	{
		/* Yes -> Prepare selection window OID */
		OID.Text = Text;
		OID.Result_ptr = &Selected_member;
		OID.Blocked_messages_ptr = Blocked_message_nrs;

		/* Do it */
		Push_module(&Window_Mod);

		Obj = Add_object(0, &MemberSel_Class, (UNBYTE *) &OID,
		 (Screen_width - MEMBERSELWIN_WIDTH) / 2, (Panel_Y
		 - MEMBERSELWIN_HEIGHT) / 2, MEMBERSELWIN_WIDTH, MEMBERSELWIN_HEIGHT);

		Execute_method(Obj, DRAW_METHOD, NULL);

		Wait_4_object(Obj);
	}

	return(Selected_member);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_MemberSel_object
 * FUNCTION  : Initialize method of Member selection window object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 12:22
 * LAST      : 28.04.95 14:13
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * NOTES     : - This function assumes there will be some party members to
 *              choose from (i.e. at least one).
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_MemberSel_object(struct Object *Object, union Method_parms *P)
{
	struct MemberSel_window_object *MemberSel_window;
	struct MemberSel_window_OID *OID;
	struct SelMember_OID SelMember_OIDs[6];
	UNSHORT Counter;
	UNSHORT X;
	UNSHORT i;

	MemberSel_window = (struct MemberSel_window_object *) Object;
	OID = (struct MemberSel_window_OID *) P;

	/* Copy data from OID */
	MemberSel_window->Text = OID->Text;
	MemberSel_window->Result_ptr = OID->Result_ptr;
	MemberSel_window->Blocked_messages_ptr = OID->Blocked_messages_ptr;

	/* Count present members */
	Counter = 0;
	for (i=0;i<6;i++)
	{
		/* Present ? */
		if (*(MemberSel_window->Blocked_messages_ptr + i) != 0xFFFF)
		{
			/* Yes -> Create selectable member object OID */
			SelMember_OIDs[Counter].Member_index = i + 1;

			/* Count up */
			Counter++;
		}
	}

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Add selectable members to window */
	X = (MEMBERSELWIN_WIDTH - (Counter * 38) + 4) / 2;
	for (i=0;i<Counter;i++)
	{
	 	Add_object(Object->Self, &SelMember_Class,
		 (UNBYTE *) &(SelMember_OIDs[i]), X, 11, 34, 37);
		X += 38;
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_MemberSel_object
 * FUNCTION  : Draw method of Member selection window object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 13:01
 * LAST      : 06.04.95 13:01
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_MemberSel_object(struct Object *Object, union Method_parms *P)
{
	struct MemberSel_window_object *MemberSel_window;
	struct PA PA;
	struct OPM *OPM;
	UNSHORT W, H;

	MemberSel_window = (struct MemberSel_window_object *) Object;
	OPM = &(MemberSel_window->Window_OPM);

	W = Object->Rect.width;
	H = Object->Rect.height;

	/* Draw window's shadow */
	Put_recoloured_box(OPM, 10, 10, W - 10, H - 10, &(Recolour_tables[1][0]));

	/* Draw window */
	Draw_window_inside(OPM, 7, 7, W - 14, H - 14);
	Draw_window_border(OPM, 0, 0, W, H);

	/* Any text ? */
	if (MemberSel_window->Text)
	{
		/* Yes -> Create print area */
		PA.Area.left = 15;
		PA.Area.top = 57;
		PA.Area.width = MEMBERSELWIN_WIDTH - PA.Area.left - 16;
		PA.Area.height = 28;

		/* Draw box around text */
		Draw_deep_box(OPM, PA.Area.left - 2, PA.Area.top - 2,
		 PA.Area.width + 4, PA.Area.height + 4);

		/* Print text */
		Push_PA(&PA);
		Set_ink(SILVER_TEXT);
		Display_text(OPM, MemberSel_window->Text);
		Pop_PA();
	}

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_SelMember_object
 * FUNCTION  : Init method of Selectable member object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 12:59
 * LAST      : 06.04.95 12:59
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	struct SelMember_OID *OID;

	SelMember = (struct SelMember_object *) Object;
	OID = (struct SelMember_OID *) P;

	/* Copy data from OID */
	SelMember->Member_index = OID->Member_index;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_SelMember_object
 * FUNCTION  : Draw method of Selectable member object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 13:05
 * LAST      : 28.04.95 14:19
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	struct MemberSel_window_object *MemberSel_window;
	struct OPM *OPM;
	UNBYTE *Ptr;

	SelMember = (struct SelMember_object *) Object;
	MemberSel_window = (struct MemberSel_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(MemberSel_window->Window_OPM);

	/* Clear portrait area */
	Draw_window_inside(OPM, Object->X - 1, Object->Y - 1, Object->Rect.width + 2,
	 Object->Rect.height + 2);
	Draw_deep_border(OPM, Object->X - 1, Object->Y - 1, Object->Rect.width + 2,
	 Object->Rect.height + 2);

	/* Is blocked ? */
	if (*(MemberSel_window->Blocked_messages_ptr + SelMember->Member_index - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box(OPM, Object->X, Object->Y, Object->Rect.width,
		 Object->Rect.height, &(Red_table[0]));
	}

	/* Draw portrait */
	Ptr = MEM_Claim_pointer(Small_portrait_handles[SelMember->Member_index - 1]);

	Put_masked_block(OPM, Object->X, Object->Y, 34, 37, Ptr);

	MEM_Free_pointer(Small_portrait_handles[SelMember->Member_index]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_SelMember_object
 * FUNCTION  : Feedback method of Selectable member object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 13:08
 * LAST      : 28.04.95 14:19
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	struct MemberSel_window_object *MemberSel_window;
	struct OPM *OPM;
	UNBYTE *Ptr;

	SelMember = (struct SelMember_object *) Object;
	MemberSel_window = (struct MemberSel_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(MemberSel_window->Window_OPM);

	/* Clear portrait area */
	Draw_deep_box(OPM, Object->X - 1, Object->Y - 1, Object->Rect.width + 2,
	 Object->Rect.height + 2);

	/* Is blocked ? */
	if (*(MemberSel_window->Blocked_messages_ptr + SelMember->Member_index - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box(OPM, Object->X, Object->Y, Object->Rect.width,
		 Object->Rect.height, &(Red_table[0]));
	}

	/* Draw portrait + feedback */
	Ptr = MEM_Claim_pointer(Small_portrait_handles[SelMember->Member_index - 1]);

	Put_masked_block(OPM, Object->X, Object->Y, 34, 37, Ptr);
	Put_recoloured_block(OPM, Object->X, Object->Y, 34, 37, Ptr,
	 &(Recolour_tables[3][0]));

	MEM_Free_pointer(Small_portrait_handles[SelMember->Member_index]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_SelMember_object
 * FUNCTION  : Highlight method of Selectable member object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 13:08
 * LAST      : 28.04.95 14:19
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	struct MemberSel_window_object *MemberSel_window;
	struct OPM *OPM;
	UNBYTE *Ptr;

	SelMember = (struct SelMember_object *) Object;
	MemberSel_window = (struct MemberSel_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(MemberSel_window->Window_OPM);

	/* Clear portrait area */
	Draw_window_inside(OPM, Object->X - 1, Object->Y - 1, Object->Rect.width + 2,
	 Object->Rect.height + 2);
	Draw_deep_border(OPM, Object->X - 1, Object->Y - 1, Object->Rect.width + 2,
	 Object->Rect.height + 2);

	/* Is blocked ? */
	if (*(MemberSel_window->Blocked_messages_ptr + SelMember->Member_index - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box(OPM, Object->X, Object->Y, Object->Rect.width,
		 Object->Rect.height, &(Red_table[0]));
	}

	/* Draw portrait + highlight */
	Ptr = MEM_Claim_pointer(Small_portrait_handles[SelMember->Member_index - 1]);

	Put_masked_block(OPM, Object->X, Object->Y, 34, 37, Ptr);
	Put_recoloured_block(OPM, Object->X, Object->Y, 34, 37, Ptr,
	 &(Recolour_tables[5][0]));

	MEM_Free_pointer(Small_portrait_handles[SelMember->Member_index]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_SelMember_object
 * FUNCTION  : Help method of Selectable member object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 16:48
 * LAST      : 28.04.95 14:19
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	MEM_HANDLE Handle;
	UNSHORT Index;
	UNCHAR Name[CHAR_NAME_LENGTH + 1];

	SelMember = (struct SelMember_object *) Object;

	/* Get party member index */
	Index = SelMember->Member_index;

	/* Get character name */
	Handle = Party_char_handles[Index - 1];
	Get_char_name(Handle, Name);

	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) Name);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_SelMember_object
 * FUNCTION  : Left method of Selectable member object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 16:39
 * LAST      : 28.04.95 14:18
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	struct MemberSel_window_object *MemberSel_window;
	UNSHORT Index;
	UNSHORT Message_nr;

	SelMember = (struct SelMember_object *) Object;
	MemberSel_window = (struct MemberSel_window_object *)
	 Get_object_data(Object->Parent);

	/* Get party member index */
	Index = SelMember->Member_index;

	/* Yes -> Get blocked message number */
	Message_nr = *(MemberSel_window->Blocked_messages_ptr +
	 SelMember->Member_index - 1);

	/* Is this party member blocked ? */
	if (Message_nr)
	{
		/* Yes -> Print blocked message */
		Execute_method(Help_line_object, SET_METHOD,
		 (void *) System_text_ptrs[Message_nr]);
	}

	/* Clicked ? */
	if (Normal_clicked(Object))
	{
		/* Is this party member blocked ? */
		if (!Message_nr)
		{
			/* No -> Select this party member */
			*(MemberSel_window->Result_ptr) = Index;

			/* Close party member selection window */
			Execute_method(MemberSel_window->Object.Self, CLOSE_METHOD, NULL);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_character_item
 * FUNCTION  : Select an item from a character's inventory.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.04.95 14:56
 * LAST      : 08.04.95 14:56
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNCHAR *Text - Pointer to window text.
 *             Item_evaluator Evaluator - Pointer to evaluation function.
 * RESULT    : UNSHORT : Selected item slot index (1...) /
 *              0xFFFF = cancelled.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Select_character_item(MEM_HANDLE Char_handle, UNCHAR *Text,
 Item_evaluator Evaluator)
{
	struct Character_data *Char;
	struct ItemSel_window_OID OID;
	struct Item_packet Slots[ITEMS_ON_BODY + ITEMS_PER_CHAR];
	UNSHORT Item_slot_indices[ITEMS_ON_BODY + ITEMS_PER_CHAR];
	UNSHORT Blocked_message_nrs[ITEMS_ON_BODY + ITEMS_PER_CHAR];
	UNSHORT Selected_item = 0xFFFF;
	UNSHORT Obj;
	UNSHORT Message_nr = 0;
	UNSHORT Counter;
	UNSHORT i;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Copy body item packets */
	Counter = 0;
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Is not tail slot OR Iskai ? */
		if ((i != TAIL-1) || (Get_race(Char_handle) == ISKAI_RACE))
		{
			/* Yes -> Is the slot empty ? */
			if (!Packet_empty(&(Char->Body_items[i])))
			{
				/* No -> Any evaluation function ? */
				if (Evaluator)
				{
					/* Yes -> Evaluate item */
					Message_nr = (Evaluator)(Char, &(Char->Body_items[i]));
				}

				/* Copy packet */
				memcpy((UNBYTE *) &Slots[Counter],
				 (UNBYTE *) &(Char->Body_items[i]), sizeof(struct Item_packet));

				/* Store blocked message number */
				Blocked_message_nrs[Counter] = Message_nr;

				/* Store slot index */
				Item_slot_indices[Counter] = i + 1;

				/* Count up */
				Counter++;
			}
		}
	}

	/* Copy backpack item packets */
	for (i=0;i<ITEMS_PER_CHAR;i++)
	{
		/* Is the slot empty ? */
		if (!Packet_empty(&(Char->Backpack_items[i])))
		{
			/* No -> Any evaluation function ? */
			if (Evaluator)
			{
				/* Yes -> Evaluate item */
				Message_nr = (Evaluator)(Char, &(Char->Backpack_items[i]));
			}

			/* Copy packet */
			memcpy((UNBYTE *) &Slots[Counter],
			 (UNBYTE *) &(Char->Backpack_items[i]), sizeof(struct Item_packet));

			/* Store blocked message number */
			Blocked_message_nrs[Counter] = Message_nr;

			/* Store slot index */
			Item_slot_indices[Counter] = ITEMS_ON_BODY + i + 1;

			/* Count up */
			Counter++;
		}
	}
	MEM_Free_pointer(Char_handle);

	/* Clear remaining item packets */
	for (i=Counter;i<ITEMS_ON_BODY + ITEMS_PER_CHAR;i++)
	{
		BASEMEM_FillMemByte((UNBYTE *) &Slots[i], sizeof(struct Item_packet), 0);
		Blocked_message_nrs[i] = 0;
	}

	/* Prepare selection window OID */
	OID.Text = Text;
	OID.Result_ptr = &Selected_item;
	OID.Blocked_messages_ptr = Blocked_message_nrs;
	OID.Slots_handle = NULL;
	OID.Slots_offset = (UNLONG) &Slots[0];

	/* Do it */
	Push_module(&Window_Mod);

	Obj = Add_object(0, &ItemSel_Class, (UNBYTE *) &OID,
	 (Screen_width - ITEMSELWIN_WIDTH) / 2, (Panel_Y
	 - ITEMSELWIN_HEIGHT) / 2, ITEMSELWIN_WIDTH, ITEMSELWIN_HEIGHT);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);

	/* Translate item slot index */
	if (Selected_item != 0xFFFF)
	{
		Selected_item = Item_slot_indices[Selected_item - 1];
	}

	return(Selected_item);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_ItemSel_object
 * FUNCTION  : Initialize method of Item selection window object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 12:22
 * LAST      : 06.04.95 12:22
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * NOTES     : - This function assumes there will be some party members to
 *              choose from (i.e. at least one).
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_ItemSel_object(struct Object *Object, union Method_parms *P)
{
	struct ItemSel_window_object *ItemSel_window;
	struct ItemSel_window_OID *OID;
	struct Item_list_OID Item_list_OID;

	ItemSel_window = (struct ItemSel_window_object *) Object;
	OID = (struct ItemSel_window_OID *) P;

	/* Copy data from OID */
	ItemSel_window->Text = OID->Text;
	ItemSel_window->Result_ptr = OID->Result_ptr;
	ItemSel_window->Blocked_messages_ptr  = OID->Blocked_messages_ptr;

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Add item list to window */
	Item_list_OID.Type = NO_CHAR_INV_TYPE;
	Item_list_OID.Nr_items = ITEMS_ON_BODY + ITEMS_PER_CHAR;
	Item_list_OID.Slots_width = 11;
	Item_list_OID.Slots_height = 3;
	Item_list_OID.Slots_handle = OID->Slots_handle;
	Item_list_OID.Slots_offset = OID->Slots_offset;
	Item_list_OID.Menu = NULL;
	Item_list_OID.Item_slot_class_ptr = &SelItem_slot_Class;

	Add_object(Object->Self, &Item_list_Class, (UNBYTE *) &Item_list_OID,
	 12, 10, 40, 40);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_ItemSel_object
 * FUNCTION  : Draw method of Item selection window object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.04.95 15:10
 * LAST      : 08.04.95 15:10
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_ItemSel_object(struct Object *Object, union Method_parms *P)
{
	struct ItemSel_window_object *ItemSel_window;
	struct PA PA;
	struct OPM *OPM;
	UNSHORT W, H;

	ItemSel_window = (struct ItemSel_window_object *) Object;
	OPM = &(ItemSel_window->Window_OPM);

	W = Object->Rect.width;
	H = Object->Rect.height;

	/* Draw window's shadow */
	Put_recoloured_box(OPM, 10, 10, W - 10, H - 10, &(Recolour_tables[1][0]));

	/* Draw window */
	Draw_window_inside(OPM, 7, 7, W - 14, H - 14);
	Draw_window_border(OPM, 0, 0, W, H);

	/* Any text ? */
	if (ItemSel_window->Text)
	{
		/* Yes -> Create print area */
		PA.Area.left = 14;
		PA.Area.top = 80;
		PA.Area.width = 185;
		PA.Area.height = 28;

		/* Draw box around text */
		Draw_deep_box(OPM, PA.Area.left - 2, PA.Area.top - 2,
		 PA.Area.width + 4, PA.Area.height + 4);

		/* Print text */
		Push_PA(&PA);
		Set_ink(SILVER_TEXT);
		Display_text(OPM, ItemSel_window->Text);
		Pop_PA();
	}

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_SelItem_object
 * FUNCTION  : Draw method of Item slot object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 15:11
 * LAST      : 27.04.95 15:11
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_SelItem_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct ItemSel_window_object *ItemSel_window;
	struct OPM *OPM;

	Item_slot = (struct Item_slot_object *) Object;
	ItemSel_window = (struct ItemSel_window_object *)
	 Get_object_data(Get_object_data(Object->Parent)->Parent);
	OPM = &(ItemSel_window->Window_OPM);

	/* Clear item slot */
	Restore_background(Object->Rect.left, Object->Rect.top, ITEM_SLOT_INNER_WIDTH,
	 ITEM_SLOT_INNER_HEIGHT);

	/* Is a blocked item ? */
	if (*(ItemSel_window->Blocked_messages_ptr + Item_slot->Number - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box(OPM, Object->X, Object->Y, ITEM_SLOT_INNER_WIDTH,
		 ITEM_SLOT_INNER_HEIGHT, &(Red_table[0]));
	}
	else
	{
		/* No -> Draw dark box */
		Put_recoloured_box(OPM, Object->X, Object->Y, ITEM_SLOT_INNER_WIDTH,
		 ITEM_SLOT_INNER_HEIGHT, &(Recolour_tables[3][0]));
	}

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_SelItem_object
 * FUNCTION  : Update method of Item slot object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 15:10
 * LAST      : 27.04.95 15:11
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_SelItem_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	struct Item_data *Item_data;

	Item_slot = (struct Item_slot_object *) Object;

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(&Packet);

		/* Is this an animated item ? */
		if (Item_data->Nr_frames > 1)
		{
			/* Yes ->  Reached last frame ? */
			if (Item_slot->Frame == Item_data->Nr_frames - 1)
			{
				/* Yes -> Back to the first frame */
				Item_slot->Frame = 0;
			}
			else
			{
				/* No -> Go forth one frame */
			   Item_slot->Frame++;
			}

			/* Is this the feedback object ? */
			if (Feedback_object == Object->Self)
			{
				/* Yes -> Re-draw item with feedback */
				Feedback_SelItem_object(Object, P);
			}
			else
			{
				/* No -> Is this the highlighted object ? */
				if (Highlighted_object == Object->Self)
				{
					/* Yes -> Re-draw item, highlighted */
					Highlight_SelItem_object(Object, P);
				}
				else
				{
					/* No -> Re-draw item */
					Draw_SelItem_object(Object, P);
				}
			}
		}
		Free_item_data();
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_SelItem_object
 * FUNCTION  : Feedback method of Item slot object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 15:10
 * LAST      : 27.04.95 15:10
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_SelItem_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct ItemSel_window_object *ItemSel_window;
	struct OPM *OPM;

	Item_slot = (struct Item_slot_object *) Object;
	ItemSel_window = (struct ItemSel_window_object *)
	 Get_object_data(Get_object_data(Object->Parent)->Parent);
	OPM = &(ItemSel_window->Window_OPM);

	/* Clear item slot */
	Restore_background(Object->Rect.left, Object->Rect.top, ITEM_SLOT_INNER_WIDTH,
	 ITEM_SLOT_INNER_HEIGHT);

	/* Is a blocked item ? */
	if (*(ItemSel_window->Blocked_messages_ptr + Item_slot->Number - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box(OPM, Object->X, Object->Y, ITEM_SLOT_INNER_WIDTH,
		 ITEM_SLOT_INNER_HEIGHT, &(Red_table[0]));
	}

	/* Draw dark box */
	Draw_deep_box(OPM, Object->X, Object->Y, ITEM_SLOT_INNER_WIDTH,
	 ITEM_SLOT_INNER_HEIGHT);

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_SelItem_object
 * FUNCTION  : Highlight method of Item slot object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 15:10
 * LAST      : 27.04.95 15:10
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_SelItem_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct ItemSel_window_object *ItemSel_window;
	struct OPM *OPM;

	Item_slot = (struct Item_slot_object *) Object;
	ItemSel_window = (struct ItemSel_window_object *)
	 Get_object_data(Get_object_data(Object->Parent)->Parent);
	OPM = &(ItemSel_window->Window_OPM);

	/* Clear item slot */
	Restore_background(Object->Rect.left, Object->Rect.top, ITEM_SLOT_INNER_WIDTH,
	 ITEM_SLOT_INNER_HEIGHT);

	/* Is a blocked item ? */
	if (*(ItemSel_window->Blocked_messages_ptr + Item_slot->Number - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box(OPM, Object->X, Object->Y, ITEM_SLOT_INNER_WIDTH,
		 ITEM_SLOT_INNER_HEIGHT, &(Red_table[0]));
	}

	/* Draw bright box */
	Put_recoloured_box(OPM, Object->X, Object->Y, ITEM_SLOT_INNER_WIDTH,
	 ITEM_SLOT_INNER_HEIGHT, &(Recolour_tables[6][0]));

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_SelItem_object
 * FUNCTION  : Left method of Selectable item slot object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.04.95 16:08
 * LAST      : 27.04.95 16:56
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_SelItem_object(struct Object *Object, union Method_parms *P)
{
	struct ItemSel_window_object *ItemSel_window;
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	UNSHORT Message_nr;

	Item_slot = (struct Item_slot_object *) Object;

	/* Get item selection window */
	ItemSel_window = (struct ItemSel_window_object *)
	 Get_object_data(Get_object_data(Object->Parent)->Parent);

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Get blocked message number */
		Message_nr = *(ItemSel_window->Blocked_messages_ptr +
		 Item_slot->Number - 1);

		/* Is a blocked item ? */
		if (Message_nr)
		{
			/* Yes -> Print blocked message */
			Execute_method(Help_line_object, SET_METHOD,
			 (void *) System_text_ptrs[Message_nr]);
		}

		/* Really clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Blocked ? */
			if (!Message_nr)
			{
				/* No -> Select this item */
				*(ItemSel_window->Result_ptr) = Item_slot->Number;

				/* Close item selection window */
				Execute_method(ItemSel_window->Object.Self, CLOSE_METHOD, NULL);
			}
		}
	}

	return 0;
}

