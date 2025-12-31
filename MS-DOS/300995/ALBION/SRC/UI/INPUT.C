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

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
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
#include <COLOURS.H>

/* defines */

#define INPUT_CURSOR_WIDTH	(5)

/* structure definitions */

/* Input OID */
struct Input_OID {
	UNSHORT Max_length;
	UNCHAR *Buffer_ptr;
};

/* Input object */
struct Input_object {
	struct Object Object;

	UNSHORT Max_length;
	UNCHAR *Buffer_ptr;

	UNSHORT Current_length;
	UNSHORT Cursor_X;

	BOOLEAN Cursor_state;
	UNSHORT Cursor_timer;

	struct PA Input_PA;
};

/* prototypes */

/* Input module functions */
void Input_ModInit(void);
void Input_ModExit(void);

/* Input object methods */
UNLONG Init_Input_object(struct Object *Object, union Method_parms *P);
UNLONG Exit_Input_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Input_object(struct Object *Object, union Method_parms *P);
UNLONG Customkeys_Input_object(struct Object *Object, union Method_parms *P);
UNLONG Right_Input_object(struct Object *Object, union Method_parms *P);

/* global variables */

/* Input module */
static struct Module Input_Mod = {
	0, MODE_MOD, NO_SCREEN,
	NULL,
	Input_ModInit, Input_ModExit,
	NULL, NULL,
	NULL
};

/* Input method list */
static struct Method Input_methods[] = {
	{ INIT_METHOD, Init_Input_object },
	{ EXIT_METHOD, Exit_Input_object },
	{ UPDATE_METHOD, Update_Input_object },
	{ RIGHT_METHOD, Right_Input_object },
	{ CUSTOMKEY_METHOD, Customkeys_Input_object },
	{ 0, NULL}
};

/* Input class description */
static struct Object_class Input_Class = {
	0, sizeof(struct Input_object),
	&Input_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Input_string_in_window
 * FUNCTION  : Input a string in a window.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.06.95 18:55
 * LAST      : 31.07.95 19:20
 * INPUTS    : UNSHORT Max_length - Maximum length of input string in
 *              characters.
 *             UNCHAR *Text_ptr - Pointer to text / NULL.
 *             UNCHAR *Buffer_ptr - Pointer to destination buffer.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - If the user aborted, the output string will be empty.
 *             - The destination buffer should be big enough for Max_length
 *              + 1 characters.
 *             - This function doesn't push a module of it's own because
 *              Input_string already does this.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Input_string_in_window(UNSHORT Max_length, UNCHAR *Text_ptr,
 UNCHAR *Buffer_ptr)
{
	struct PA PA;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	UNSHORT Window_X, Window_Y;
	UNSHORT Window_width, Window_height;
	UNBYTE *Ptr;

	/* Current screen is 2D map ? */
	if (Current_screen_type(0) == MAP_2D_SCREEN)
	{
		/* Yes -> Prepare screen */
		Current_2D_OPM = &Main_OPM;
		Draw_2D_scroll_buffer();
		Current_2D_OPM = NULL;
	}

	/* Determine window dimensions */
	Window_width = max((Max_length * 8) + 29, 129);
	Window_height = 40;
	if (Text_ptr)
		Window_height += 28 + 4;

	/* Determine window position */
	Window_X = (Screen_width - Window_width) / 2;
	Window_Y = (Screen_height - Window_height) / 2;

	/* Make background buffer and OPM */
	Background_handle = MEM_Allocate_memory(Window_width * Window_height);

	Ptr = MEM_Claim_pointer(Background_handle);
	OPM_New(Window_width, Window_height, 1, &Background_OPM, Ptr);
	MEM_Free_pointer(Background_handle);

	/* Save background */
	OPM_CopyOPMOPM(&Main_OPM, &Background_OPM, Window_X, Window_Y,
	 Window_width, Window_height, 0, 0);

	/* Draw window's shadow */
	Put_recoloured_box(&Main_OPM, Window_X + 10, Window_Y + 10,
	 Window_width - 10, Window_height - 10, &(Recolour_tables[1][0]));

	/* Draw window */
	Draw_window_inside(&Main_OPM, Window_X + 7, Window_Y + 7,
	 Window_width - 14, Window_height - 14);
	Draw_window_border(&Main_OPM, Window_X, Window_Y, Window_width,
	 Window_height);

	/* Draw input area */
	Draw_deep_box(&Main_OPM, Window_X + 13, Window_Y + Window_height - 26,
	 (Max_length * 8) + 2, STANDARD_TEXT_HEIGHT + 4);

	/* Any text ? */
	if (Text_ptr)
	{
		/* Yes -> Create print area */
		PA.Area.left = Window_X + 15;
		PA.Area.top = Window_Y + 14;
		PA.Area.width = Window_width - 31;
		PA.Area.height = 28;

		/* Draw box around text */
		Draw_deep_box(&Main_OPM, PA.Area.left - 2, PA.Area.top - 2,
		 PA.Area.width + 4, PA.Area.height + 4);

		/* Print text */
		Push_PA(&PA);
		Set_ink(SILVER_TEXT);
		Display_text(&Main_OPM, Text_ptr);
		Pop_PA();
	}

	/* Input */
	Input_string(Window_X + 15, Window_Y + Window_height - 24, Max_length * 8,
	 Max_length, Buffer_ptr);

	/* Restore background */
	OPM_CopyOPMOPM(&Background_OPM, &Main_OPM, 0, 0, Window_width,
	 Window_height, Window_X, Window_Y);

	/* Destroy background buffer and OPM */
	OPM_Del(&Background_OPM);
	MEM_Free_memory(Background_handle);

	/* Current screen is 2D map ? */
	if (Current_screen_type(0) == MAP_2D_SCREEN)
	{
		/* Yes -> Prepare screen */
		Draw_2D_scroll_buffer();
		Switch_screens();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Input_string
 * FUNCTION  : Input a string.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 12:23
 * LAST      : 29.06.95 10:56
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 *             UNSHORT Width - Width of input area.
 *             UNSHORT Max_length - Maximum length of input string in
 *              characters.
 *             UNCHAR *Buffer_ptr - Pointer to destination buffer.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - If the user aborted, the destination buffer will be empty.
 *             - The destination buffer should be big enough for Max_length
 *              + 1 characters.
 *             - If the destination buffer is not empty, the user can edit
 *              the contents.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Input_string(UNSHORT X, UNSHORT Y, UNSHORT Width, UNSHORT Max_length,
 UNCHAR *Buffer_ptr)
{
	struct Input_OID OID;
	UNSHORT Obj;

	/* Input */
	Push_module(&Input_Mod);

	/* Build input OID */
	OID.Max_length = Max_length;
	OID.Buffer_ptr = Buffer_ptr;

	/* Add input object */
	Obj = Add_object(0, &Input_Class, (UNBYTE *) &OID, X, Y, Width,
	 STANDARD_TEXT_HEIGHT);

	/* Highlight the input object so it will receive keys */
	Highlighted_object = Obj;

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	Input_ModInit
 * FUNCTION  : Init input module.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 15:15
 * LAST      : 29.06.95 10:58
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Input_ModInit(void)
{
	/* Disable mouse */
	Mouse_off();

	/* Install text style */
	Push_textstyle(&Default_text_style);

	/* Install new object tree */
	Push_root(&Main_OPM);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	Input_ModExit
 * FUNCTION  : Exit input module.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 15:15
 * LAST      : 29.06.95 10:58
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Input_ModExit(void)
{
	/* Remove object tree */
	Pop_root();

	/* Remove text style */
	Pop_textstyle();

	/* Enable mouse */
	Mouse_on();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Input_object
 * FUNCTION  : Init method of Input object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.06.95 12:23
 * LAST      : 29.06.95 12:23
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Input_object(struct Object *Object, union Method_parms *P)
{
	struct Input_object *Input;
	struct Input_OID *OID;

	Input = (struct Input_object *) Object;
	OID = (struct Input_OID *) P;

	/* Copy data from OID */
	Input->Max_length = OID->Max_length;
	Input->Buffer_ptr = OID->Buffer_ptr;

	/* Set input variables */
	Input->Current_length = strlen(Input->Buffer_ptr);
	Input->Cursor_X = Get_line_width(Input->Buffer_ptr);

	/* Reset cursor state */
	Input->Cursor_state = FALSE;
	Input->Cursor_timer = 0;

	/* Build Print Area */
	Input->Input_PA.Area.left = Object->X;
	Input->Input_PA.Area.top = Object->Y;
	Input->Input_PA.Area.width = Object->Rect.width;
	Input->Input_PA.Area.height = Object->Rect.height;

	/* Install and initialize Print Area */
	Push_PA(&(Input->Input_PA));
	Init_print_area(Current_OPM);

	/* Install MA to ensure that the input object remains highlighted */
	Push_MA(&(Input->Input_PA.Area));

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Input_object
 * FUNCTION  : Exit method of Input object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.06.95 12:53
 * LAST      : 29.06.95 12:53
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Exit_Input_object(struct Object *Object, union Method_parms *P)
{
	/* Exit and remove Print Area */
	Exit_print_area();
	Pop_PA();

	/* Remove MA */
	Pop_MA();

	/* Exit module (MUST be here !!!) */
	Pop_module();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Input_object
 * FUNCTION  : Update method of Input object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 14:38
 * LAST      : 29.06.95 11:17
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Input_object(struct Object *Object, union Method_parms *P)
{
	struct Input_object *Input;

	Input = (struct Input_object *) Object;

	/* Erase input area */
	Erase_print_area(Current_OPM);

	/* Set ink */
	Set_ink(SILVER_TEXT);

	/* Print input string */
	Print_string(Current_OPM, Object->X, Object->Y, Input->Buffer_ptr);

	/* Time to blink cursor ? */
	if (!Input->Cursor_timer)
	{
		/* Yes -> Reset timer */
		Input->Cursor_timer = 6;

		/* Toggle state */
		Input->Cursor_state = ~(Input->Cursor_state);
	}
	else
	{
		/* No -> Count down */
		Input->Cursor_timer--;
	}

	/* Show cursor ? */
	if (Input->Cursor_state)
	{
		/* Yes */
		OPM_FillBox(Current_OPM, Object->X + Input->Cursor_X + 2,
		 Object->Y + 1, INPUT_CURSOR_WIDTH, STANDARD_TEXT_HEIGHT - 1, BLACK);
		OPM_FillBox(Current_OPM, Object->X + Input->Cursor_X + 1,
		 Object->Y, INPUT_CURSOR_WIDTH, STANDARD_TEXT_HEIGHT - 1, WHITE);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Customkeys_Input_object
 * FUNCTION  : Custom keys method of Input object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 14:17
 * LAST      : 14.09.95 16:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Customkeys_Input_object(struct Object *Object, union Method_parms *P)
{
	struct Input_object *Input;
	BOOLEAN Result = TRUE;
	UNSHORT Key_code;

	Input = (struct Input_object *) Object;

	/* Get key-code */
	Key_code = (UNSHORT) (P->Event->sl_key_code);

	switch (Key_code)
	{
		/* Escape */
		case BLEV_ESC:
		{
			/* Clear input buffer */
			*(Input->Buffer_ptr) = 0;

			/* Reset length and cursor position */
			Input->Current_length = 0;
			Input->Cursor_X = 0;

			break;
		}
		/* Backspace */
		case BLEV_BS:
		{
			/* Are there characters in the input buffer ? */
			if (Input->Current_length)
			{
				/* Yes -> Decrease length */
				Input->Current_length--;

				/* Insert EOL */
				*((Input->Buffer_ptr) + Input->Current_length) = 0;

				/* Determine new cursor position */
				Input->Cursor_X = Get_line_width(Input->Buffer_ptr);
			}
			break;
		}
		/* Done */
		case BLEV_RETURN:
		{
			/* Dehighlight */
			Dehighlight(Object, NULL);

			/* Delete self */
			Delete_object(Object->Self);

			break;
		}
		/* Default (key) */
		default:
		{
			/* Is there room in the input buffer ? */
			if (Input->Current_length < Input->Max_length)
			{
				/* Yes */
				Key_code &= 0x00FF;

				/* Exit if illegal character */
				if (Key_code < 32)
				{
					Result = FALSE;
					break;
				}

				/* Insert new character in input buffer */
				*((Input->Buffer_ptr) + Input->Current_length) = Key_code;

				/* Increase length */
				Input->Current_length++;

				/* Insert EOL */
				*((Input->Buffer_ptr) + Input->Current_length) = 0;

				/* Determine new cursor position */
				Input->Cursor_X = Get_line_width(Input->Buffer_ptr);

				/* Is there room on screen ? */
				if (Input->Cursor_X >= Object->Rect.width -
				 INPUT_CURSOR_WIDTH - 1)
				{
					/* No -> Decrease length */
					Input->Current_length--;

					/* Insert EOL */
					*((Input->Buffer_ptr) + Input->Current_length) = 0;

					/* Determine new cursor position */
					Input->Cursor_X = Get_line_width(Input->Buffer_ptr);
				}
			}
			break;
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Right_Input_object
 * FUNCTION  : Right mouse button method of Input object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 14:32
 * LAST      : 29.06.95 12:24
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Right_Input_object(struct Object *Object, union Method_parms *P)
{
	struct Input_object *Input;

	Input = (struct Input_object *) Object;

	/* Clear input buffer */
	*(Input->Buffer_ptr) = 0;

	/* Dehighlight */
	Dehighlight(Object, NULL);

	/* Delete self */
	Delete_object(Object->Self);

	return 0;
}

