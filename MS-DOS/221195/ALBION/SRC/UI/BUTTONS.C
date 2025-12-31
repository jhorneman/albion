/************
 * NAME     : BUTTONS.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 5-1-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : BUTTONS.H, USERFACE.C
 ************/

/*
 ** Pragmas ****************************************************************
 */

#pragma off (unreferenced);

/*
 ** Includes ***************************************************************
 */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <BUTTONS.H>
#include <STATAREA.H>
#include <GAMETEXT.H>
#include <SCROLBAR.H>
#include <COLOURS.H>

/*
 ** Global variables *******************************************************
 */

static struct Method Button_methods[] = {
	{ INIT_METHOD,			Init_Button_object },
	{ DRAW_METHOD,			Draw_Button_object },
	{ LEFT_METHOD,			Left_Button_object },
	{ FEEDBACK_METHOD,	Feedback_Button_object },
	{ HIGHLIGHT_METHOD,	Highlight_Button_object },
	{ HELP_METHOD,			Help_Button_object },
	{ TOUCHED_METHOD,		Normal_touched },
	{ SET_METHOD,			Set_Button_object },
	{ GET_METHOD,			Get_Button_object },
	{ 0, NULL}
};

struct Object_class Button_Class = {
	0, sizeof(struct Button_object),
	&Button_methods[0]
};

static struct Method Radio_group_methods[] = {
	{ DRAW_METHOD,		Draw_Radio_group_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ SET_METHOD,		Set_Radio_group_object },
	{ GET_METHOD,		Get_Radio_group_object },
	{ 0, NULL}
};

struct Object_class Radio_group_Class = {
	0, sizeof(struct Radio_group_object),
	&Radio_group_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Button_object
 * FUNCTION  : Initialize method of Button object.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 14:22
 * LAST      : 05.01.95 14:22
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Button_object(struct Object *Object, union Method_parms *P)
{
	struct Button_object *Button;
	struct Button_OID *OID;

	Button = (struct Button_object *) Object;
	OID = (struct Button_OID *) P;

	/* Copy data from OID */
	Button->Type				= OID->Type;
	Button->Number				= OID->Number;
	Button->Help_message_nr	= OID->Help_message_nr;
	Button->Data				= OID->Data;
	Button->Function			= OID->Function;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Button_object
 * FUNCTION  : Draw method of Button object.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 14:22
 * LAST      : 25.09.95 16:36
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Button_object(struct Object *Object, union Method_parms *P)
{
	struct Button_object *Button;
	struct BBRECT Rect;
	union Method_parms P2;

	Button = (struct Button_object *) Object;

	/* Is depressed ? */
	if (Button->State)
	{
		/* Yes -> Call feedback method instead */
		Feedback_Button_object(Object, P);
	}
	else
	{
		/* No -> Initialize rectangle */
		Rect.left	= Object->X - 1;
		Rect.top		= Object->Y - 1;
		Rect.width	= Object->Rect.width + 2;
		Rect.height	= Object->Rect.height + 2;

		/* Restore background */
		P2.Rect = &Rect;
		Execute_upcast_method(Object->Self,	RESTORE_METHOD, &P2);

		/* What kind of button ? */
		switch (Button->Type & 0x00FF)
		{
			case TEXT_BUTTON_TYPE:
			{
				/* Draw border around button */
				Draw_deep_border
				(
					Current_OPM,
					Object->X - 1,
					Object->Y - 1,
					Object->Rect.width + 2,
					Object->Rect.height + 2
				);
				Draw_high_border
				(
					Current_OPM,
					Object->X,
					Object->Y,
					Object->Rect.width,
					Object->Rect.height
				);

				/* Print button text */
				Push_textstyle(&Default_text_style);

				Set_ink(SILVER_TEXT);

				Print_centered_box_string
				(
					Current_OPM,
					Object->X,
					Object->Y + 1,
					Object->Rect.width,
					Object->Rect.height,
					Button->Data->Text_button_data.Text
				);

				Pop_textstyle();

				break;
			}
			case SYMBOL_BUTTON_TYPE:
			{
				SISHORT dX, dY;
				UNSHORT W, H;
				UNBYTE *Ptr;

				/* Draw border around button */
				Draw_deep_border
				(
					Current_OPM,
					Object->X - 1,
					Object->Y - 1,
					Object->Rect.width + 2,
					Object->Rect.height + 2
				);
				Draw_high_border
				(
					Current_OPM,
					Object->X,
					Object->Y,
					Object->Rect.width,
					Object->Rect.height
				);

				/* Get dimensions of symbol */
				W = Button->Data->Symbol_button_data.Width;
				H = Button->Data->Symbol_button_data.Height;

				/* Calculate centering offsets */
				dX = Object->Rect.width - W;
				if (dX < 0)
					dX = 0;
				dX /= 2;
				dY = Object->Rect.height - H;
				if (dY < 0)
					dY = 0;
				dY /= 2;

				/* Get pointer to graphics */
				Ptr = MEM_Claim_pointer(Button->Data->Symbol_button_data.Graphics_handle)
				 + Button->Data->Symbol_button_data.Graphics_offset;

				/* Display symbol */
				Put_masked_block
				(
					Current_OPM,
					Object->X + dX,
					Object->Y + dY,
					W,
					H,
					Ptr
				);

				MEM_Free_pointer(Button->Data->Symbol_button_data.Graphics_handle);

				break;
			}
			case DOUBLE_SYMBOL_BUTTON_TYPE:
			{
				UNSHORT W, H;
				UNBYTE *Ptr;

				/* Get dimensions of symbol */
				W = Button->Data->Double_symbol_button_data.Width;
				H = Button->Data->Double_symbol_button_data.Height;

				/* Get pointer to graphics */
				Ptr = MEM_Claim_pointer(Button->Data->Symbol_button_data.Graphics_handle)
				 + Button->Data->Symbol_button_data.Graphics_offset;

				/* Display symbol */
				Put_masked_block
				(
					Current_OPM,
					Object->X,
					Object->Y,
					W,
					H,
					Ptr
				);

				MEM_Free_pointer(Button->Data->Symbol_button_data.Graphics_handle);

				break;
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Button_object
 * FUNCTION  : Feedback method of Button object.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 16:25
 * LAST      : 25.09.95 16:36
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_Button_object(struct Object *Object, union Method_parms *P)
{
	struct Button_object *Button;
	struct BBRECT Rect;
	union Method_parms P2;

	Button = (struct Button_object *) Object;

	/* Initialize rectangle */
	Rect.left	= Object->X - 1;
	Rect.top		= Object->Y - 1;
	Rect.width	= Object->Rect.width + 2;
	Rect.height	= Object->Rect.height + 2;

	/* Restore background */
	P2.Rect = &Rect;
	Execute_upcast_method(Object->Self,	RESTORE_METHOD, &P2);

	/* What kind of button ? */
	switch (Button->Type & 0x00FF)
	{
		case TEXT_BUTTON_TYPE:
		{
			/* Draw border around button */
			Draw_deep_border
			(
				Current_OPM,
				Object->X - 1,
				Object->Y - 1,
				Object->Rect.width + 2,
				Object->Rect.height + 2
			);
			Draw_deep_box
			(
				Current_OPM,
				Object->X,
				Object->Y,
				Object->Rect.width,
				Object->Rect.height
			);

			/* Print button text */
			Push_textstyle(&Default_text_style);

			Set_ink(SILVER_TEXT);

			Print_centered_box_string
			(
				Current_OPM,
				Object->X,
				Object->Y + 2,
				Object->Rect.width,
				Object->Rect.height,
				Button->Data->Text_button_data.Text
			);

			Pop_textstyle();

			break;
		}
		case SYMBOL_BUTTON_TYPE:
		{
			SISHORT dX, dY;
			UNSHORT W, H;
			UNBYTE *Ptr;

			/* Draw border around button */
			Draw_deep_border
			(
				Current_OPM,
				Object->X - 1,
				Object->Y - 1,
				Object->Rect.width + 2,
				Object->Rect.height + 2
			);
			Draw_deep_box
			(
				Current_OPM,
				Object->X,
				Object->Y,
				Object->Rect.width,
				Object->Rect.height
			);

			/* Get dimensions of symbol */
			W = Button->Data->Symbol_button_data.Width;
			H = Button->Data->Symbol_button_data.Height;

			/* Calculate centering offsets */
			dX = Object->Rect.width - W;
			if (dX < 0)
				dX = 0;
			dX /= 2;
			dY = Object->Rect.height - H;
			if (dY < 0)
				dY = 0;
			dY /= 2;

			/* Get pointer to graphics */
			Ptr = MEM_Claim_pointer(Button->Data->Symbol_button_data.Graphics_handle)
			 + Button->Data->Symbol_button_data.Graphics_offset;

			/* Display symbol */
			Put_masked_block
			(
				Current_OPM,
				Object->X + dX,
				Object->Y + dY + 1,
				W,
				H,
				Ptr
			);

			MEM_Free_pointer(Button->Data->Symbol_button_data.Graphics_handle);

			break;
		}
		case DOUBLE_SYMBOL_BUTTON_TYPE:
		{
			UNSHORT W, H;
			UNBYTE *Ptr;

			/* Get dimensions of symbol */
			W = Button->Data->Double_symbol_button_data.Width;
			H = Button->Data->Double_symbol_button_data.Height;

			/* Get pointer to graphics */
			Ptr = MEM_Claim_pointer(Button->Data->Symbol_button_data.Graphics_handle)
			 + Button->Data->Symbol_button_data.Graphics_offset;

			/* Skip to second frame */
			Ptr += W * H;

			/* Display symbol */
			Put_masked_block
			(
				Current_OPM,
				Object->X,
				Object->Y,
				W,
				H,
				Ptr
			);

			MEM_Free_pointer(Button->Data->Symbol_button_data.Graphics_handle);

			break;
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Button_object
 * FUNCTION  : Highlight method of Button object.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 14:22
 * LAST      : 25.09.95 16:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Button_object(struct Object *Object, union Method_parms *P)
{
	struct Button_object *Button;
	struct BBRECT Rect;
	union Method_parms P2;

	Button = (struct Button_object *) Object;

	/* Initialize rectangle */
	Rect.left	= Object->X - 1;
	Rect.top		= Object->Y - 1;
	Rect.width	= Object->Rect.width + 2;
	Rect.height	= Object->Rect.height + 2;

	/* Restore background */
	P2.Rect = &Rect;
	Execute_upcast_method(Object->Self,	RESTORE_METHOD, &P2);

	/* Is depressed ? */
	if (Button->State)
	{
		/* Yes -> What kind of button ? */
		switch (Button->Type & 0x00FF)
		{
			case TEXT_BUTTON_TYPE:
			{
				/* Draw border around button */
				Draw_deep_border
				(
					Current_OPM,
					Object->X - 1,
					Object->Y - 1,
					Object->Rect.width + 2,
					Object->Rect.height + 2
				);
				Draw_deep_border
				(
					Current_OPM,
					Object->X,
					Object->Y,
					Object->Rect.width,
					Object->Rect.height
				);
				Put_recoloured_box
				(
					Current_OPM,
					Object->X + 1,
					Object->Y + 1,
					Object->Rect.width - 2,
					Object->Rect.height - 2,
					&(Recolour_tables[6][0])
				);

				/* Print button text */
				Push_textstyle(&Default_text_style);

				Set_ink(SILVER_TEXT);

				Print_centered_box_string
				(
					Current_OPM,
					Object->X,
					Object->Y + 2,
					Object->Rect.width,
					Object->Rect.height,
					Button->Data->Text_button_data.Text
				);

				Pop_textstyle();

				break;
			}
			case SYMBOL_BUTTON_TYPE:
			{
				SISHORT dX, dY;
				UNSHORT W, H;
				UNBYTE *Ptr;

				/* Draw border around button */
				Draw_deep_border
				(
					Current_OPM,
					Object->X - 1,
					Object->Y - 1,
					Object->Rect.width + 2,
					Object->Rect.height + 2
				);
				Draw_deep_border
				(
					Current_OPM,
					Object->X,
					Object->Y,
					Object->Rect.width,
					Object->Rect.height
				);
				Put_recoloured_box
				(
					Current_OPM,
					Object->X + 1,
					Object->Y + 1,
					Object->Rect.width - 2,
					Object->Rect.height - 2,
					&(Recolour_tables[6][0])
				);

				/* Get dimensions of symbol */
				W = Button->Data->Symbol_button_data.Width;
				H = Button->Data->Symbol_button_data.Height;

				/* Calculate centering offsets */
				dX = Object->Rect.width - W;
				if (dX < 0)
					dX = 0;
				dX /= 2;
				dY = Object->Rect.height - H;
				if (dY < 0)
					dY = 0;
				dY /= 2;

				/* Get pointer to graphics */
				Ptr = MEM_Claim_pointer(Button->Data->Symbol_button_data.Graphics_handle)
				 + Button->Data->Symbol_button_data.Graphics_offset;

				/* Display symbol */
				Put_masked_block
				(
					Current_OPM,
					Object->X + dX,
					Object->Y + dY + 1,
					W,
					H,
					Ptr
				);

				MEM_Free_pointer(Button->Data->Symbol_button_data.Graphics_handle);

				break;
			}
			case DOUBLE_SYMBOL_BUTTON_TYPE:
			{
				UNSHORT W, H;
				UNBYTE *Ptr;

				/* NOTE : The double-symbol button really shouldn't be used as
				 a switch. This code is only present to make sure the button
				 still serves it's purpose. */

				/* Light up area around button */
				Put_recoloured_box
				(
					Current_OPM,
					Object->X,
					Object->Y,
					Object->Rect.width,
					Object->Rect.height,
					&(Recolour_tables[6][0])
				);

				/* Get dimensions of symbol */
				W = Button->Data->Double_symbol_button_data.Width;
				H = Button->Data->Double_symbol_button_data.Height;

				/* Get pointer to graphics */
				Ptr = MEM_Claim_pointer(Button->Data->Symbol_button_data.Graphics_handle)
				 + Button->Data->Symbol_button_data.Graphics_offset;

				/* Skip to second frame */
				Ptr += W * H;

				/* Display symbol */
				Put_masked_block
				(
					Current_OPM,
					Object->X,
					Object->Y,
					W,
					H,
					Ptr
				);

				MEM_Free_pointer(Button->Data->Symbol_button_data.Graphics_handle);

				break;
			}
		}
	}
	else
	{
		/* No -> What kind of button ? */
		switch (Button->Type & 0x00FF)
		{
			case TEXT_BUTTON_TYPE:
			{
				/* Draw border around button */
				Draw_deep_border
				(
					Current_OPM,
					Object->X - 1,
					Object->Y - 1,
					Object->Rect.width + 2,
					Object->Rect.height + 2
				);
				Draw_light_box
				(
					Current_OPM,
					Object->X,
					Object->Y,
					Object->Rect.width,
					Object->Rect.height
				);

				/* Print button text */
				Push_textstyle(&Default_text_style);

				Set_ink(SILVER_TEXT);

				Print_centered_box_string
				(
					Current_OPM,
					Object->X,
					Object->Y + 1,
					Object->Rect.width,
					Object->Rect.height,
					Button->Data->Text_button_data.Text
				);

				Pop_textstyle();

				break;
			}
			case SYMBOL_BUTTON_TYPE:
			{
				SISHORT dX, dY;
				UNSHORT W, H;
				UNBYTE *Ptr;

				/* Draw border around button */
				Draw_deep_border
				(
					Current_OPM,
					Object->X - 1,
					Object->Y - 1,
					Object->Rect.width + 2,
					Object->Rect.height + 2
				);
				Draw_light_box
				(
					Current_OPM,
					Object->X,
					Object->Y,
					Object->Rect.width,
					Object->Rect.height
				);

				/* Get dimensions of symbol */
				W = Button->Data->Symbol_button_data.Width;
				H = Button->Data->Symbol_button_data.Height;

				/* Calculate centering offsets */
				dX = Object->Rect.width - W;
				if (dX < 0)
					dX = 0;
				dX /= 2;
				dY = Object->Rect.height - H;
				if (dY < 0)
					dY = 0;
				dY /= 2;

				/* Get pointer to graphics */
				Ptr = MEM_Claim_pointer(Button->Data->Symbol_button_data.Graphics_handle)
				 + Button->Data->Symbol_button_data.Graphics_offset;

				/* Display symbol */
				Put_masked_block
				(
					Current_OPM,
					Object->X + dX,
					Object->Y + dY,
					W,
					H,
					Ptr
				);

				MEM_Free_pointer(Button->Data->Symbol_button_data.Graphics_handle);

				break;
			}
			case DOUBLE_SYMBOL_BUTTON_TYPE:
			{
				UNSHORT W, H;
				UNBYTE *Ptr;

				/* Get dimensions of symbol */
				W = Button->Data->Double_symbol_button_data.Width;
				H = Button->Data->Double_symbol_button_data.Height;

				/* Get pointer to graphics */
				Ptr = MEM_Claim_pointer(Button->Data->Symbol_button_data.Graphics_handle)
				 + Button->Data->Symbol_button_data.Graphics_offset;

				/* Skip to third frame */
				Ptr += W * H * 2;

				/* Display symbol */
				Put_masked_block
				(
					Current_OPM,
					Object->X,
					Object->Y,
					W,
					H,
					Ptr
				);

				MEM_Free_pointer(Button->Data->Symbol_button_data.Graphics_handle);

				break;
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Button_object
 * FUNCTION  : Left method of Button object.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.01.95 16:48
 * LAST      : 07.01.95 16:48
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Button_object(struct Object *Object, union Method_parms *P)
{
	struct Button_object *Button;

	Button = (struct Button_object *) Object;

	/* Is this a depressed radio button ? */
	if (((Button->Type & 0xFF00) != RADIO_BUTTON) || (!Button->State))
	{
		/* No */
		switch (Button->Type & 0xFF00)
		{
			/* Normal button */
			case BUTTON_BUTTON:
			{
				/* Clicked ? */
				if (Normal_clicked(Object))
				{
					/* Execute function (if any) */
					if (Button->Function)
					{
						(Button->Function)(Button);
					}
				}
				break;
			}
			/* Switch button */
			case SWITCH_BUTTON:
			{
				/* Clicked ? */
				if (Normal_clicked(Object))
				{
					/* Toggle state */
					Button->State = ~(Button->State);

					/* Redraw button */
					Execute_method(Object->Self, DRAW_METHOD, NULL);

					/* Execute function (if any) */
					if (Button->Function)
					{
						(Button->Function)(Button);
					}
				}
				break;
			}
			/* Radio button */
			case RADIO_BUTTON:
			{
				union Method_parms P2;

				/* Clicked ? */
				if (Normal_clicked(Object))
				{
					/* Change parent state */
					P2.Value = Button->Number;
					Execute_method(Object->Parent, SET_METHOD, &P2);

					/* Execute function (if any) */
					if (Button->Function)
					{
						(Button->Function)(Button);
					}
				}
				break;
			}
			/* Continous button */
			case CONTINUOUS_BUTTON:
			{
				BOOLEAN Flag = FALSE, New, First_time = TRUE;

				/* While the mouse button is pressed */
				while (Button_state & 0x0001)
				{
					/* Check if the mouse is over the object */
					New = Is_over_object(Object->Self);

					/* Any change ? */
					if (Flag != New)
					{
						/* Yes */
						Flag = New;

						/* Up or down ? */
						if (Flag)
						{
							/* Down */
							Execute_method(Object->Self, FEEDBACK_METHOD, NULL);
						}
						else
						{
							/* Up */
							Execute_method(Object->Self, DRAW_METHOD, NULL);
							First_time = TRUE;
						}
					}

					/* Is down ? */
					if (Flag)
					{
						/* Yes -> Execute function (if any) */
						Button->State = First_time;
						if (Button->Function)
						{
							(Button->Function)(Button);
						}
						First_time = FALSE;
					}

					/* Update */
					Update_display();
					Update_input();
					Switch_screens();
				}

				/* Is down ? */
				if (Flag)
				{
					/* Yes -> Restore object */
					Execute_method(Object->Self, DRAW_METHOD, NULL);
				}

				/* Reset highlighting */
				Highlighted_object = 0;

				break;
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Button_object
 * FUNCTION  : Help method of Button object.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.01.95 16:42
 * LAST      : 07.01.95 16:42
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Button_object(struct Object *Object, union Method_parms *P)
{
	struct Button_object *Button;

	Button = (struct Button_object *) Object;

	/* Is this a depressed radio button ? */
	if (((Button->Type & 0xFF00) == RADIO_BUTTON) && (Button->State))
	{
		/* Yes -> Remove help message */
		Execute_method(Help_line_object, SET_METHOD, NULL);
	}
	else
	{
		/* No -> Print help line (if any) */
		if (Button->Help_message_nr != 0xFFFF)
		{
			Execute_method(Help_line_object, SET_METHOD,
			 (void *) System_text_ptrs[Button->Help_message_nr]);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_Button_object
 * FUNCTION  : Set method of Button object.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 16:54
 * LAST      : 10.01.95 16:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Set_Button_object(struct Object *Object, union Method_parms *P)
{
	struct Button_object *Button;

	Button = (struct Button_object *) Object;

	/* New state ? */
	if (Button->State != P->Value)
	{
		/* Yes -> Set new state */
		Button->State = P->Value;

		/* Redraw button */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_Button_object
 * FUNCTION  : Get method of Button object.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 16:54
 * LAST      : 10.01.95 16:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : Button state.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_Button_object(struct Object *Object, union Method_parms *P)
{
	struct Button_object *Button;

	Button = (struct Button_object *) Object;

	return (UNLONG) Button->State;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Radio_group_object
 * FUNCTION  : Draw method of Radio_group object.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 17:08
 * LAST      : 10.01.95 17:08
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Radio_group_object(struct Object *Object, union Method_parms *P)
{
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_Radio_group_object
 * FUNCTION  : Set method of Radio_group object.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 16:54
 * LAST      : 25.08.95 14:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * NOTES     : - This method only works if the child objects are of the
 *              Button class.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Set_Radio_group_object(struct Object *Object, union Method_parms *P)
{
	struct Radio_group_object *Radio_group;
	union Method_parms P2;
	UNSHORT Handle;

	Radio_group = (struct Radio_group_object *) Object;

	/* New state ? */
	if (Radio_group->State != P->Value)
	{
		/* Yes -> Reset current button */
		P2.Value = 0;
		Handle = Find_radio_button(Radio_group, Radio_group->State);
		if (Handle)
			Execute_method(Handle, SET_METHOD, &P2);

		/* Set new state */
		Radio_group->State = P->Value;

		/* Set new button */
		P2.Value = 0xFFFF;
		Handle = Find_radio_button(Radio_group, Radio_group->State);
		if (Handle)
			Execute_method(Handle, SET_METHOD, &P2);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_Radio_group_object
 * FUNCTION  : Get method of Radio_group object.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 16:54
 * LAST      : 10.01.95 16:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : Radio group state.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_Radio_group_object(struct Object *Object, union Method_parms *P)
{
	struct Radio_group_object *Radio_group;

	Radio_group = (struct Radio_group_object *) Object;

	return (UNLONG) Radio_group->State;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_Radio_button
 * FUNCTION  : Find a radio button with a certain number in a radio group.
 * FILE      : BUTTON.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 17:17
 * LAST      : 10.01.95 17:17
 * INPUTS    : struct Radio_group_object *Radio_group - Pointer to radio
 *              group object.
 *             UNSHORT Number - Radio button number.
 * RESULT    : UNSHORT : Radio button object handle (0 = error).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Find_radio_button(struct Radio_group_object *Radio_group, UNSHORT Number)
{
	struct Object *Object;
	struct Button_object *Button;
	UNSHORT Handle;

	Object = &(Radio_group->Object);
	Handle = Object->Child;

	while (Handle)
	{
		/* Get button data */
		Object = Get_object_data(Handle);
		Button = (struct Button_object *) Object;

		/* Is this the right button ? */
		if (Button->Number == Number)
		{
			/* Yes -> Exit */
			break;
		}
		else
		{
			/* No -> Next button */
			Handle = Object->Next;
		}
	}

	return Handle;
}

