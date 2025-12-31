/************
 * NAME     : CONTROL.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 4-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/*
 ** Pragmas ****************************************************************
 */

#pragma off (unreferenced);

/*
 ** Includes ***************************************************************
 */

#include <string.h>
#include <i86.h>

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBDOS.H>
#include <BBEVENT.H>
#include <BBERROR.H>
#include <BBSYSTEM.H>
#include <BBEXTRDF.H>

#include <HDOB.H>

#include <GAMEVAR.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <DIAGNOST.H>
#include <SOUND.H>
#include <COLOURS.H>
#include <STATAREA.H>

/*
 ** Macro definitions ******************************************************
 */

#ifdef DEBUG

#define Control_error(a) Report_control_error(a, __LINE__, __FILE__)

#else

#define Control_error(a) Report_control_error(a, 0, NULL)

#endif

/*
 ** Defines ****************************************************************
 */

#define MAX_ROOTS					(8)
#define MAX_UI_OBJECTS 			(250)
#define UI_OBJECT_POOL_SIZE	(MAX_UI_OBJECTS * 50)

#define MAX_UPDATE_OPMS			(8)

#define MAX_MODULES				(8)
#define MAX_MOUSE_POINTERS		(8)
#define MAX_MA						(8)

#define MAX_MOUSE_IMAGE_SIZE	(1024)

/* Error codes */
#define ERROR_OBJECT_TOO_SMALL		(1)
#define ERROR_TOO_MANY_OBJECTS		(2)
#define ERROR_OBJECT_POOL_IS_FULL	(3)
#define ERROR_OBJECT_HANDLE_ZERO		(4)
#define ERROR_OBJECT_DOESNT_EXIST	(5)
#define ERROR_ILLEGAL_OBJECT_HANDLE	(6)

/*
 ** Structure definitions **************************************************
 */

/* Roots / trees */
struct Root {
	UNSHORT First_object;
	struct OPM *OPM;
};

/*
 ** Prototypes *************************************************************
 */

/* Mouse input handling support functions */
void Handle_mouse_event(struct BLEV_Event_struct *Event);
BOOLEAN Search_mouse_event_object(UNSHORT Method, UNSHORT Handle);
BOOLEAN Search_mouse_method(UNSHORT Method, struct Object *Object);
BOOLEAN Predict_method_capability(UNSHORT Method, UNSHORT Handle);

/* Key input handling support functions */
void Handle_key_event(struct BLEV_Event_struct *Event);

/* Object management functions */
struct Object *Search_object(UNSHORT Handle, UNSHORT Target);

/* Object method functions */
void Do_execute_broadcast_method(UNSHORT Handle, UNSHORT Method, union Method_parms *P);

/* Object pool management functions */
struct Object *Add_object_to_pool(struct Object_class *Class);
void Remove_object_from_pool(UNSHORT Handle);

/* Mouse pointer support functions */
void Init_mouse_pointer(struct Mouse_pointer *New);

/* Mouse area support functions */
void Init_MA(struct BBRECT *New);

/* Screen update functions */
void Show_OPMs(void);
void Show_changed_OPMs(void);

/* Error functions */
void Report_control_error(UNSHORT Error_code, int Line_nr, char *Filename);

/*
 ** Global variables *******************************************************
 */

static struct OPM *OPM_update_list[MAX_ROOTS][MAX_UPDATE_OPMS];
struct OPM *Current_OPM;

UNLONG Update_duration = 3;

BOOLEAN Drag_drop_mode = FALSE;
struct Drag_drop_data Current_drag_drop_data;

BOOLEAN Palette_has_changed = FALSE;

UNSHORT Mouse_X, Mouse_Y;
static UNSHORT Old_mouse_X, Old_mouse_Y;

UNSHORT Button_state;

static UNSHORT Mouse_counter = 0;
static BOOLEAN Clear_input_buffer_flag;

static BOOLEAN Ignore_second_left_click = FALSE;
static BOOLEAN Ignore_second_right_click = FALSE;

static UNLONG Update_duration_time_start = 0;
UNLONG Global_timer = 0;

UNSHORT Feedback_object;
UNSHORT Highlighted_object;
UNSHORT Focussed_object;

static UNSHORT Nr_objects = 0;
static UNLONG Last_object_offset = 0;
static UNSHORT Warn_when_deleted_counter = 0;

static BOOLEAN Mouse_system_initialised = FALSE;

static struct Mouse_pointer *Mouse_stack[MAX_MOUSE_POINTERS];
static UNSHORT Mouse_stack_index = 0;

static UNBYTE Mouse_images[2][MAX_MOUSE_IMAGE_SIZE];
static UNSHORT Current_mouse_image;

static UNBYTE Empty_mptrgfx[4 * 4];

static struct Mouse_pointer Empty_mouse_pointer = {
	0, 0,
	4, 4,
	Empty_mptrgfx
};

static struct BBRECT MA_stack[MAX_MA];
static UNSHORT MA_stack_index = 0;

struct BBRECT Default_MA;

static struct Root Root_stack[MAX_ROOTS];
static UNSHORT Root_stack_index = 0;

static struct Module Module_stack[MAX_MODULES];
static UNSHORT Module_stack_index = 0;
static UNSHORT Pop_counter = 0;

static struct Object *Object_ptrs[MAX_UI_OBJECTS];
static UNBYTE Object_pool[UI_OBJECT_POOL_SIZE];

struct Module Default_module =
{
	0, SCREEN_MOD, NO_SCREEN,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

/* Error data */
static UNCHAR _Control_library_name[] = "Control";

static struct Error_message _Control_errors[] = {
	{ERROR_OBJECT_TOO_SMALL,		"Object is too small."},
	{ERROR_TOO_MANY_OBJECTS, 		"Too many objects in pool."},
	{ERROR_OBJECT_POOL_IS_FULL,	"Object pool is full."},
	{ERROR_OBJECT_HANDLE_ZERO,		"Reference to object handle zero."},
	{ERROR_OBJECT_DOESNT_EXIST,	"Reference to non-existent object."},
	{ERROR_ILLEGAL_OBJECT_HANDLE, "Reference to illegal object handle."},
	{0, NULL}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Enter_drag_drop_mode
 * FUNCTION  : Enter drag & drop mode.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 13:49
 * LAST      : 06.07.95 13:36
 * INPUTS    : UNSHORT Data_ID - Drag & drop data ID.
 *             Drag_drop_abort_handler *Handler - Pointer to drag & drop
 *              abort handler.
 *             struct Object *Source_object - Pointer to drag & drop source
 *              object.
 *             UNBYTE *Data - Pointer to data for drag & drop abort handler.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Enter_drag_drop_mode(UNSHORT Data_ID, Drag_drop_abort_handler Handler,
 struct Object *Source_object, UNBYTE *Data)
{
	/* Drag & drop mode is on */
	Drag_drop_mode = TRUE;

	/* Set drag & drop data */
	Current_drag_drop_data.Data_ID			= Data_ID;
	Current_drag_drop_data.Abort_handler	= Handler;
	Current_drag_drop_data.Source_object	= Source_object;
	Current_drag_drop_data.Data				= Data;

	/* Clear input buffer */
	Clear_input_buffer();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Leave_drag_drop_mode
 * FUNCTION  : Leave drag & drop mode.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 13:49
 * LAST      : 03.03.95 13:49
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Leave_drag_drop_mode(void)
{
	/* Drag & drop mode is off */
	Drag_drop_mode = FALSE;

	/* Clear input buffer */
	Clear_input_buffer();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Abort_drag_drop_mode
 * FUNCTION  : Abort drag & drop mode.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.95 15:48
 * LAST      : 08.08.95 13:51
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Abort_drag_drop_mode(void)
{
	/* Abort drag & drop */
	if (Current_drag_drop_data.Abort_handler)
		(Current_drag_drop_data.Abort_handler)(&Current_drag_drop_data);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_event
 * FUNCTION  : Get an input event and handle input recording / playback.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.09.94 12:24
 * LAST      : 15.08.95 15:10
 * INPUTS    : struct BLEV_Event_struct *Event - Event.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_event(struct BLEV_Event_struct *Event)
{
	static BOOLEAN Handling_diagnostic_key = FALSE;

	UNSHORT Old_button_state;
	UNSHORT New_button_state;

	/* Do thang */
	SYSTEM_SystemTask();

	/* Get event */
	BLEV_GetEvent(Event);

	/* Get mouse coordinates */
	Mouse_X = Event->sl_mouse_x;
	Mouse_Y = Event->sl_mouse_y;

	/* Get old and new button state */
	Old_button_state = Button_state;
	New_button_state = Button_state & 0x0011;

	/* Check changes in the left and right mouse button states */
	if ((Event->sl_eventtype == BLEV_MOUSELDOWN)
	 || (Event->sl_eventtype == BLEV_MOUSELSDOWN))
	{
		New_button_state |= 0x01;
	}

	if ((Event->sl_eventtype == BLEV_MOUSELUP)
	 || (Event->sl_eventtype == BLEV_MOUSELSUP))
	{
		New_button_state &= ~0x01;
	}

	if ((Event->sl_eventtype == BLEV_MOUSERDOWN)
	 || (Event->sl_eventtype == BLEV_MOUSERSDOWN))
	{
		New_button_state |= 0x10;
	}

	if ((Event->sl_eventtype == BLEV_MOUSERUP)
	 || (Event->sl_eventtype == BLEV_MOUSERSUP))
	{
		New_button_state &= ~0x10;
	}

	/* Calculate the complete new button state */
	Button_state = New_button_state |
	 (((~Old_button_state & 0x0011) & New_button_state) << 1) |
	 (((~New_button_state & 0x0011) & Old_button_state) << 2);

	/* Is this a second left-click that should be ignored ? */
	if ((Event->sl_eventtype == BLEV_MOUSELDOWN) && (Ignore_second_left_click))
	{
		/* Yes -> Clear flag */
		Ignore_second_left_click = FALSE;

		/* Get next event */
		Get_event(Event);
	}

	/* Is this a second right-click that should be ignored ? */
	if ((Event->sl_eventtype == BLEV_MOUSERDOWN) && (Ignore_second_right_click))
	{
		/* Yes -> Clear flag */
		Ignore_second_right_click = FALSE;

		/* Get next event */
		Get_event(Event);
	}

	/* Handling diagnostic keys right now ? */
	if (!Handling_diagnostic_key)
	{
		/* Yes -> Was a key pressed ? */
		if (Event->sl_eventtype == BLEV_KEYDOWN)
		{
			/* Yes -> Check for low-level diagnostic keys */
			Handling_diagnostic_key = Check_low_level_diagnostic_keys(Event);

			/* Any found ? */
			if (Handling_diagnostic_key)
			{
				/* Yes -> Clear input buffer */
				Update_input();

				/* Clear flag */
				Handling_diagnostic_key = FALSE;

				/* Clear event */
				Event->sl_eventtype = BLEV_NOEVENT;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wait_4_user
 * FUNCTION  : Wait for the user to click or press a key.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.09.94 14:24
 * LAST      : 20.10.95 18:27
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wait_4_user(void)
{
	BOOLEAN Flag = FALSE;

	/* Wait for the user to release the left mouse button */
	Wait_4_unclick();

	/* Show click mouse-pointer */
	Push_mouse(&(Mouse_pointers[CLICK_MPTR]));

	while (!Flag)
	{
		/* Update the display */
		Update_display();

		/* Check if the user acted */
		Flag = Check_if_user_acted();

		/* Do double-buffering */
		Switch_screens();
	}

	/* Wait for the user to release the left mouse button */
	Wait_4_unclick();

	/* Restore original mouse-pointer */
	Pop_mouse();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_if_user_acted
 * FUNCTION  : Check if the user has clicked or pressed a key.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.95 18:28
 * LAST      : 20.10.95 18:28
 * INPUTS    : None.
 * RESULT    : BOOLEAN : User acted.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_if_user_acted(void)
{
	struct BLEV_Event_struct Event;
	BOOLEAN User_acted = FALSE;

	/* Read all events */
	do
	{
		/* Get event */
		Get_event(&Event);

		/* Did the user click or press a key ? */
		if ((Event.sl_eventtype == BLEV_KEYDOWN)
		 || (Event.sl_eventtype == BLEV_MOUSELSDOWN)
		 || (Event.sl_eventtype == BLEV_MOUSERSDOWN))
		{
			/* Yes -> Indicate */
			User_acted = TRUE;
		}
	}
	/* Until there are no more events */
	while (Event.sl_eventtype != BLEV_NOEVENT);

	return User_acted;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wait_4_click
 * FUNCTION  : Wait for the user to click the left mouse button.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 11:10
 * LAST      : 14.09.94 11:10
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wait_4_click(void)
{
	Push_mouse(&(Mouse_pointers[CLICK_MPTR]));

	while (!(Button_state & 0x0002))
	{
		Update_display();
		Update_input();
		Switch_screens();
	}
	Wait_4_unclick();

	Pop_mouse();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wait_4_unclick
 * FUNCTION  : Wait for the user to release the left mouse button.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 11:11
 * LAST      : 14.09.94 11:11
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wait_4_unclick(void)
{
	while (Button_state & 0x0001)
	{
		Update_display();
		Update_input();
		Switch_screens();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wait_4_right_unclick
 * FUNCTION  : Wait for the user to release the right mouse button.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.10.94 13:38
 * LAST      : 25.10.94 13:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wait_4_right_unclick(void)
{
	while (Button_state & 0x0010)
	{
		Update_display();
		Update_input();
		Switch_screens();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_input
 * FUNCTION  : Update input.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.09.94 13:32
 * LAST      : 05.09.94 13:32
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function makes sure that the events will be read and
 *              that the current mouse coordinates and button state remain
 *              up to date.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_input(void)
{
	struct BLEV_Event_struct Event;

	do
	{
		/* Get event */
		Get_event(&Event);
	}
	/* Until there are no more events */
	while (Event.sl_eventtype != BLEV_NOEVENT);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_input
 * FUNCTION  : Handle input.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 18:12
 * LAST      : 27.02.95 14:37
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_input(void)
{
	struct BLEV_Event_struct Event;
	struct Module *Module;
	static SISHORT Key_or_mouse = -1;

	/* Get current module */
	Module = &Module_stack[Module_stack_index];

	/* Input handling disabled ? */
	if (Module->Flags & NO_INPUT_HANDLING)
	{
		/* Yes -> Update and exit */
		Update_input();
		return;
	}

	/* Clear input buffer by default */
	Clear_input_buffer_flag = TRUE;

	/* Get current event */
	Get_event(&Event);

	/* Are we currently in key- or in mouse-mode ? */
	if (Key_or_mouse)
	{
		/* Mouse -> Was a key pressed ? */
		if (Event.sl_eventtype == BLEV_KEYDOWN)
		{
			/* Yes -> Go to key mode */
			Key_or_mouse = 0;

			/* Select initial target for key */
			Highlighted_object = Object_at(Mouse_X, Mouse_Y);

			/* Save mouse position */
			Old_mouse_X = Mouse_X;
			Old_mouse_Y = Mouse_Y;

			/* Handle key event */
			Handle_key_event(&Event);
		}
		else
		{
			/* No -> Handle mouse event */
			Handle_mouse_event(&Event);
		}
	}
	else
	{
		/* Key -> Is there a key event ? */
		if ((Event.sl_eventtype == BLEV_KEYDOWN))
		{
			/* Yes -> Was the mouse moved ? */
			if ((Mouse_X == Old_mouse_X) && (Mouse_Y == Old_mouse_Y))
			{
				/* No -> Handle key event */
				Handle_key_event(&Event);
			}
			else
			{
				/* Yes -> Go to mouse mode */
				Key_or_mouse = -1;

				/* Handle mouse event */
				Handle_mouse_event(&Event);
			}
		}
		else
		{
			/* No -> Go to mouse mode */
			Key_or_mouse = -1;

			/* Handle "mouse" event (touch) */
			Handle_mouse_event(&Event);
		}
	}

	/* Clear input buffer ? */
	if (Clear_input_buffer_flag)
	{
		/* Yes */
		Clear_input_buffer();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_input_buffer
 * FUNCTION  : Clear the input buffer.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.09.94 17:29
 * LAST      : 08.09.94 17:29
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clear_input_buffer(void)
{
	struct BLEV_Event_struct Event;

	do
	{
		/* Get event */
		Get_event(&Event);
	}
	/* Until there are no more events */
	while (Event.sl_eventtype != BLEV_NOEVENT);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_mouse_event
 * FUNCTION  : Handle mouse event.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 19:12
 * LAST      : 04.08.94 19:12
 * INPUTS    : struct BLEV_Event_struct *Event - Mouse event.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will find the smallest object containing the
 *              mouse coordinates which can handle this event.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_mouse_event(struct BLEV_Event_struct *Event)
{
	UNSHORT Method1 = 0;
	UNSHORT Root_object;

	/* Get root object */
	Root_object = Get_root_object_handle();

	/* Select normal methods */
	switch (Event->sl_eventtype)
	{
		/* First left down event */
		case BLEV_MOUSELSDOWN:
		{
			/* Are double-clicks accepted ? */
			if (Predict_method_capability(DLEFT_METHOD, Root_object))
			{
				/* Yes -> Ignore the first click, because a double-click
				 may follow. */
				Method1 = TOUCHED_METHOD;

				/* Since the double-click may already be in the input buffer,
				 make sure this buffer isn't cleared. */
				Clear_input_buffer_flag = FALSE;
			}
			else
			{
				/* No -> Accept the first click, because there is no need
				 to wait for a double-click. */
				Method1 = LEFT_METHOD;

				/* Make sure the second click is ignored (even if sent to
				 another object !) */
				Ignore_second_left_click = TRUE;
			}
			break;
		}
		/* Second left down event */
		case BLEV_MOUSELDOWN:
		{
			/* Are double-clicks accepted ? */
			if (Predict_method_capability(DLEFT_METHOD, Root_object))
			{
				/* Yes -> Accept the second click, because a double-click
				 did not occur. */
				Method1 = LEFT_METHOD;
			}
			else
			{
				/* No -> Ignore the second click, because the program has
				 already reacted to the first one. */
				Method1 = TOUCHED_METHOD;
			}
			break;
		}
		/* Left double-click event */
		case BLEV_MOUSELDBL:
		{
			Method1 = DLEFT_METHOD;
			break;
		}
		/* First right down event */
		case BLEV_MOUSERSDOWN:
		{
			/* Are double-clicks accepted ? */
			if (Predict_method_capability(DRIGHT_METHOD, Root_object))
			{
				/* Yes -> Ignore the first click, because a double-click
				 may follow. */
				Method1 = TOUCHED_METHOD;

				/* Since the double-click may already be in the input buffer,
				 make sure this buffer isn't cleared. */
				Clear_input_buffer_flag = FALSE;
			}
			else
			{
				/* No -> Accept the first click, because there is no need
				 to wait for a double-click. */
				Method1 = RIGHT_METHOD;

				/* Make sure the second click is ignored (even if sent to
				 another object !) */
				Ignore_second_right_click = TRUE;
			}
			break;
		}
		/* Second right down event */
		case BLEV_MOUSERDOWN:
		{
			/* Are double-clicks accepted ? */
			if (Predict_method_capability(DRIGHT_METHOD, Root_object))
			{
				/* Yes -> Accept the second click, because a double-click
				 did not occur. */
				Method1 = RIGHT_METHOD;
			}
			else
			{
				/* No -> Ignore the second click, because the program has
				 already reacted to the first one. */
				Method1 = TOUCHED_METHOD;
			}
			break;
		}
		/* Right double-click event */
		case BLEV_MOUSERDBL:
		{
			Method1 = DRIGHT_METHOD;
			break;
		}
		/* Else touch */
		default:
		{
			Method1 = TOUCHED_METHOD;
			break;
		}
	}

	/* Drag & drop mode ? */
	if (Drag_drop_mode)
	{
		/* Yes -> Mutate to drag & drop methods */
		switch (Method1)
		{
			/* Left method */
			case LEFT_METHOD:
			{
				/* Change to drop method */
				Method1 = DROP_METHOD;
				break;
			}
			/* Right method */
			case RIGHT_METHOD:
			{
				/* Abort drag & drop mode */
				Abort_drag_drop_mode();

				/* Change to touch method */
				Method1 = TOUCHED_METHOD;
				break;
			}
			/* Left double-click method */
			case DLEFT_METHOD:
			{
				/* Change to drag & drop left double-click method */
				Method1 = DRAG_DLEFT_METHOD;
				break;
			}
			/* All other methods */
			default:
			{
				/* Change to touch method */
				Method1 = TOUCHED_METHOD;
				break;
			}
		}
	}

	/* Was the touch method selected ? */
	if (Method1 == TOUCHED_METHOD)
	{
		/* Yes -> Make sure the input buffer isn't cleared */
		Clear_input_buffer_flag = FALSE;
	}

	/* Search the current tree */
	Search_mouse_event_object(Method1, Root_object);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_mouse_event_object
 * FUNCTION  : Search an object which can handle the current mouse event.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 19:20
 * LAST      : 04.08.94 19:20
 * INPUTS    : UNSHORT Method - Method ID.
 *             UNSHORT Handle - Handle of object.
 * RESULT    : BOOLEAN : Success or failure.
 * BUGS      : No known.
 * NOTES     : - This function will find the smallest object containing the
 *              mouse coordinates which can handle this event.
 *             - This function can handle a zero handle.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Search_mouse_event_object(UNSHORT Method, UNSHORT Handle)
{
	struct Object *Object;

	while (Handle)
	{
		/* Get object data */
		Object = Get_object_data(Handle);
		if (!Object)
		{
			return FALSE;
		}

		/* Is this object disabled ? */
		if (!(Object->Flags & OBJECT_DISABLED))
		{
			/* No -> Is this a control object ? */
			if (Object->Flags & OBJECT_CONTROL)
			{
				/* Yes -> Does it have children ? */
				if (Object->Child)
				{
					/* Yes -> Check them */
					if (Search_mouse_event_object(Method, Object->Child))
						return TRUE;
				}

				/* No -> Check this object */
				if (Search_mouse_method(Method, Object))
					return TRUE;
			}
			else
			{
				/* No -> Are the mouse coordinates within it's area ? */
				if ((Mouse_X >= Object->Rect.left) && (Mouse_X < Object->Rect.left +
				 Object->Rect.width) && (Mouse_Y >= Object->Rect.top) && (Mouse_Y <
				 Object->Rect.top + Object->Rect.height))
				{
					/* Yes -> Does it have children ? */
					if (Object->Child)
					{
						/* Yes -> Check them */
						if (Search_mouse_event_object(Method, Object->Child))
							return TRUE;
					}

					/* Check this object */
					return (Search_mouse_method(Method, Object));
				}
			}
		}

		/* Check the object's brother */
		Handle = Object->Next;
	}

	/* The handle was zero */
	return FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_mouse_method
 * FUNCTION  : Search a certain method or the custommouse method in an object.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 19:32
 * LAST      : 06.07.95 14:17
 * INPUTS    : UNSHORT Method - Method ID.
 *             struct Object *Object - Pointer to object.
 * RESULT    : BOOLEAN : Success or failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Search_mouse_method(UNSHORT Method, struct Object *Object)
{
	union Method_parms Parameters;
	union Method_parms *Parameter_ptr = NULL;
	UNLONG Result;

	/* Does this object have a CUSTOMMOUSE method ? */
	if (Has_method(Object->Self, CUSTOMMOUSE_METHOD))
	{
		/* Yes -> Execute */
		if (Execute_method(Object->Self, CUSTOMMOUSE_METHOD,
		 (union Method_parms *) &Button_state))
		{
			/* Exit if succesful */
			return TRUE;
		}
	}

	/* No -> Is there another method ? */
	if (Method)
	{
		/* Yes -> Does this object have this method ? */
		if (Has_method(Object->Self, Method))
		{
			/* Yes -> Is DROP method ? */
			if (Method == DROP_METHOD)
			{
				/* Yes -> Pass drag & drop data as parameters */
				Parameters.Drag_drop_data_ptr = &Current_drag_drop_data;
				Parameter_ptr = &Parameters;

				/* Does this object have the INQUIRE_DROP method ? */
				if (Has_method(Object->Self, INQUIRE_DROP_METHOD))
				{
					/* Yes -> Inquire whether this object can handle the
					 current drag & drop data ID */
					Result = Execute_method(Object->Self, INQUIRE_DROP_METHOD,
					 Parameter_ptr);

					/* Exit if this is not the case */
					if (!Result)
						return FALSE;
				}
			}

			/* Execute */
			Execute_method(Object->Self, Method, Parameter_ptr);
			return TRUE;
		}
	}

	return FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Predict_method_capability
 * FUNCTION  : Look for an object which can handle the current mouse method.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.02.95 09:37
 * LAST      : 23.02.95 09:37
 * INPUTS    : UNSHORT Method - Method ID.
 *             UNSHORT Handle - Handle of object.
 * RESULT    : BOOLEAN : Success or failure.
 * BUGS      : No known.
 * NOTES     : - This function will find the smallest object containing the
 *              mouse coordinates which can handle this event.
 *             - This function can handle a zero handle.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Predict_method_capability(UNSHORT Method, UNSHORT Handle)
{
	struct Object *Object;

	while (Handle)
	{
		/* Get object data */
		Object = Get_object_data(Handle);
		if (!Object)
		{
			return FALSE;
		}

		/* Is this object disabled ? */
		if (!(Object->Flags & OBJECT_DISABLED))
		{
			/* No -> Is this a control object ? */
			if (Object->Flags & OBJECT_CONTROL)
			{
				/* Yes -> Does it have children ? */
				if (Object->Child)
				{
					/* Yes -> Check them */
					if (Predict_method_capability(Method, Object->Child))
						return TRUE;
				}

				/* No -> Check this object */
				return (Has_method(Object->Self, Method));
			}
			else
			{
				/* No -> Are the mouse coordinates within it's area ? */
				if ((Mouse_X >= Object->Rect.left) && (Mouse_X < Object->Rect.left +
				 Object->Rect.width) && (Mouse_Y >= Object->Rect.top) && (Mouse_Y <
				 Object->Rect.top + Object->Rect.height))
				{
					/* Yes -> Does it have children ? */
					if (Object->Child)
					{
						/* Yes -> Check them */
						if (Predict_method_capability(Method, Object->Child))
							return TRUE;
					}

					/* Check this object */
					return (Has_method(Object->Self, Method));
				}
			}
		}

		/* Check the object's brother */
		Handle = Object->Next;
	}

	/* The handle was zero */
	return FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_key_event
 * FUNCTION  : Handle key event.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 10:15
 * LAST      : 05.08.94 10:15
 * INPUTS    : struct BLEV_Event_struct *Event - Key event.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_key_event(struct BLEV_Event_struct *Event)
{
	union Method_parms P;
	struct Object *Object;
	UNSHORT Method1 = 0;
	UNSHORT Handle;

	/* Exit if no event */
	if (Event->sl_eventtype == BLEV_NOEVENT)
		return;

	/* Check for high-level diagnostic keys */
	if (Check_high_level_diagnostic_keys(Event))
		return;

	#if FALSE
	/* Select method */
	switch (Event->sl_key_code)
	{
		/* Cycle focus */
		case BLEV_TAB:
		{
			Method1 = CYCLE_METHOD;
			break;
		}
		/* Left click */
		case BLEV_RETURN:
		{
			Method1 = LEFT_METHOD;
			break;
		}
		/* Right click */
		case ' ':
		{
			Method1 = RIGHT_METHOD;
			break;
		}
		/* Move highlight up within focus */
		case BLEV_UP:
		{
			Method1 = MOVE_METHOD;
			P.Direction = 0;
			break;
		}
		/* Move highlight down within focus */
		case BLEV_DOWN:
		{
			Method1 = MOVE_METHOD;
			P.Direction = 2;
			break;
		}
		/* Move highlight left within focus */
		case BLEV_LEFT:
		{
			Method1 = MOVE_METHOD;
			P.Direction = 3;
			break;
		}
		/* Move highlight right within focus */
		case BLEV_RIGHT:
		{
			Method1 = MOVE_METHOD;
			P.Direction = 1;
			break;
		}
		/* Close or abort */
		case BLEV_ESC:
		{
			Method1 = CLOSE_METHOD;
			break;
		}
	}
	#endif

	/* Yes -> Search for object with right method, starting with the
	 currently highlighted object */
	Handle = Highlighted_object;
	while (Handle)
	{
		/* Does this object have a CUSTOMKEY method ? */
		if (Has_method(Handle, CUSTOMKEY_METHOD))
		{
			/* Yes -> Execute */
			P.Event = Event;
			if (Execute_method(Handle, CUSTOMKEY_METHOD, &P))
			{
				/* Exit if succesful */
				return;
			}
		}

		/* No -> Is there another method ? */
		if (Method1)
		{
			/* Yes -> Does this object have this method ? */
			if (Has_method(Handle, Method1))
			{
				/* Yes -> Execute */
				Execute_method(Handle, Method1, &P);
				return;
			}
		}

		/* Try parent object */
	   Object = Get_object_data(Handle);
		if (Object)
		{
	   	Handle = Object->Parent;
		}
		else
		{
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_object
 * FUNCTION  : Create an instance of an object class and bind it into the
 *              current object tree.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 12:05
 * LAST      : 30.06.95 16:02
 * INPUTS    : UNSHORT Parent_handle - Handle of parent object / 0 (layer 1).
 *             struct Object_class *Class - Pointer to object class.
 *             UNBYTE *OID - Pointer to OID (Object Initialization Data).
 *             SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 *             UNSHORT Width - Width.
 *             UNSHORT Height - Height.
 * RESULT    : UNSHORT : Object handle / 0 (error).
 * BUGS      : No known.
 * NOTES     : - This routine MUST be re-entrant because an object's
 *              Initialize method can add new objects.
 *             - The Init method will receive a pointer to the object and a
 *              pointer to the OID.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Add_object(UNSHORT Parent_handle, struct Object_class *Class,
 UNBYTE *OID, SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	struct Object *New_object;
	struct Object *Parent_object = NULL;
	struct Object *Object;
	UNSHORT Handle;
	UNSHORT Self;

	/* Add object to pool */
	New_object = Add_object_to_pool(Class);
	if (!New_object)
		return 0;

	/* Get new handle */
	Self = New_object->Self;

	/* Insert parent handle */
	New_object->Parent = Parent_handle;

	/* Any parent given ? */
	if (Parent_handle)
	{
		/* Yes -> Get parent object data */
		Parent_object = Get_object_data(Parent_handle);

		/* Copy coordinates from parent object */
		New_object->Rect.left = Parent_object->Rect.left;
		New_object->Rect.top = Parent_object->Rect.top;

		/* Does the parent object already have children ? */
		if (Parent_object->Child)
		{
			/* Yes -> Find last child */
			Handle = Parent_object->Child;
			while (Handle)
			{
				Object = Get_object_data(Handle);
				Handle = Object->Next;
			}

			/* Bind */
			Object->Next = Self;
		}
		else
		{
			/* No -> Bind (first child object) */
			Parent_object->Child = Self;
		}
	}
	else
	{
		/* No -> Is the current tree empty ? */
		Handle = Get_root_object_handle();
		if (Handle)
		{
			/* No -> Find last object in layer 1 */
			while (Handle)
			{
				Object = Get_object_data(Handle);
				Handle = Object->Next;
			}

			/* Bind */
			Object->Next = Self;
		}
		else
		{
			/* Yes -> Bind (first object) */
			Root_stack[Root_stack_index].First_object = Self;
		}
	}

	/* Set object rectangle */
	New_object->Rect.left	+= X;
	New_object->Rect.top		+= Y;
	New_object->Rect.width	= Width;
	New_object->Rect.height	= Height;

	/* Is the new object a display base ? */
	if (!(New_object->Flags & OBJECT_DISPLAY_BASE))
	{
		/* No -> Any parent object ? */
		if (Parent_object)
		{
			/* Yes -> Copy display coordinates from parent object */
			New_object->X = Parent_object->X;
			New_object->Y = Parent_object->Y;
		}

		/* Add display coordinates */
		New_object->X += X;
		New_object->Y += Y;
	}

	/* Call object's Init method */
	Execute_method(Self, INIT_METHOD, (union Method_parms *) OID);

	return Self;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_object
 * FUNCTION  : Search an object in the object tree, starting at a given object.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 13:10
 * LAST      : 04.08.94 13:10
 * INPUTS    : UNSHORT Handle - First object handle.
 *             UNSHORT Target - Handle of target object.
 * RESULT    : struct Object * : Pointer to target object / NULL (error).
 * BUGS      : No known.
 * NOTES     : - This is a recursive function.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Object *
Search_object(UNSHORT Handle, UNSHORT Target)
{
	struct Object *Ptr, *T;

	for(;;)
	{
		/* Get current object */
		Ptr = Get_object_data(Handle);
		if (!Ptr)
		{
			return NULL;
		}

		/* Is this the one ? */
		if (Handle == Target)
		{
			/* Yes -> Exit */
			return Ptr;
		}

		/* No -> Has children ? */
		if (Ptr->Child)
		{
			/* Yes -> Search children */
			T = Search_object(Ptr->Child, Target);

			/* Found anything ? */
			if (T)
			{
				/* Yes -> Exit */
				return T;
			}
		}

		/* Get brother */
		Handle = Ptr->Next;

		/* Has brother ? */
		if (!Handle)
		{
			/* No -> Not found */
			return NULL;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Delete_object
 * FUNCTION  : Remove an object from the current tree and destroy it.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 13:50
 * LAST      : 04.08.94 13:50
 * INPUTS    : UNSHORT Handle - Handle of object.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - All child objects will be deleted as well.
 *             - Because of this, the routine MUST be re-entrant.
 *             - Exit methods will receive a pointer to the object.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Delete_object(UNSHORT Handle)
{
	struct Object *Object, *Referrer;
	UNSHORT t;

	/* Get object data */
	Object = Get_object_data(Handle);
	if (!Object)
	{
		return;
	}

	/* Handle deletion warnings */
	if (Object->Flags & OBJECT_WARN_WHEN_DELETED)
		Warn_when_deleted_counter--;

	/* Are there any objects in the tree ? */
	t = Get_root_object_handle();
	if (t)
	{
		/* Yes -> Search referring object */
		Referrer = NULL;

		/* Does the object have a parent ? */
		if (Object->Parent)
		{
			/* Yes -> Start the search with the first child */
			Referrer = Get_object_data(Object->Parent);
			t = Referrer->Child;
		}

		/* Found ? */
		while (t != Handle)
		{
			/* No -> Next object */
			Referrer = Get_object_data(t);
			t = Referrer->Next;

			/* End of the chain ? */
			if (!t)
			{
				/* Yes -> Error */
				Referrer = NULL;
				break;
			}
		}

		/* Does the object have children ? */
		t = Object->Child;
		while (t)
		{
			/* Yes -> Delete child */
			Delete_object(t);
			t = Object->Child;
		}

		/* Any referring object ? */
		if (Referrer)
		{
			/* Yes */
			t = Object->Next;

			/* Parent or brother ? */
			if (Referrer->Next == Handle)
				Referrer->Next = t;
			else
				Referrer->Child = t;
		}
		else
		{
			/* No */
			Root_stack[Root_stack_index].First_object = 0;
		}
	}

	/* Call object's Exit method */
	Execute_method(Handle, EXIT_METHOD, NULL);

	/* Remove object from pool */
	Remove_object_from_pool(Handle);

	/* Was this the feedback object ? */
	if (Handle == Feedback_object)
	{
		/* Yes */
		Feedback_object = 0;
	}

	/* Was this the highlighted object ? */
	if (Handle == Highlighted_object)
	{
		/* Yes */
		Highlighted_object = 0;
	}

	/* Was this the focussed object ? */
	if (Handle == Focussed_object)
	{
		/* Yes */
		Focussed_object = 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_object_to_pool
 * FUNCTION  : Add an object to the object pool.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 12:12
 * LAST      : 04.08.94 12:12
 * INPUTS    : struct Object_class *Class - Pointer to object class.
 * RESULT    : struct Object : Pointer to new object / NULL (error).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Object *
Add_object_to_pool(struct Object_class *Class)
{
	struct Object *Object;
	UNSHORT Start, End;
	UNSHORT Handle;

	/* Too many objects ? */
	if (Nr_objects >= MAX_UI_OBJECTS)
	{
		/* Yes -> Error */
		Control_error(ERROR_TOO_MANY_OBJECTS);
		return NULL;
	}

	/* Legal object size ? */
	if (Class->Size < sizeof(struct Object))
	{
		/* No -> Error */
		Control_error(ERROR_OBJECT_TOO_SMALL);
		return NULL;
	}

	/* Will it fit in the pool ? */
	Start = Last_object_offset;
	End = Start + Class->Size;
	if (End > UI_OBJECT_POOL_SIZE)
	{
		/* No -> Error */
		Control_error(ERROR_OBJECT_POOL_IS_FULL);
		return NULL;
	}

	/* Yes -> Count up */
	Object = (struct Object *) &Object_pool[Start];
	Last_object_offset = End;
	Nr_objects++;

	/* Find a free entry in the object list */
	for (Handle=1;Handle<=MAX_UI_OBJECTS;Handle++)
	{
		if (!Object_ptrs[Handle - 1])
		{
			Object_ptrs[Handle - 1] = Object;
			break;
		}
	}

	/* Clear object data */
	BASEMEM_FillMemByte((UNBYTE *) Object, Class->Size, 0);

	/* Store variables */
	Object->Self	= Handle;
	Object->Flags	= Class->Flags;
	Object->Class	= Class;

	return Object;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_object_from_pool
 * FUNCTION  : Remove an object from the object pool.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 12:23
 * LAST      : 04.08.94 12:23
 * INPUTS    : UNSHORT Handle - Object handle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_object_from_pool(UNSHORT Handle)
{
	struct Object *Object;
	UNLONG Diff;
	UNSHORT i;
	UNBYTE *Start, *End, *Ptr;

	/* Get pointer to object */
	Object = Get_object_data(Handle);
	if (!Object)
		return;

	/* Clear entry in object list */
	Object_ptrs[Handle-1] = NULL;

	/* Copy other objects down */
	Diff = (UNLONG) Object->Class->Size;
	Start = (UNBYTE *) Object;
	End = Start + Diff;
	BASEMEM_CopyMem(End, Start, &Object_pool[UI_OBJECT_POOL_SIZE] - End);

	/* Adjust pointers to other objects */
	for (i=0;i<MAX_UI_OBJECTS;i++)
	{
		/* Get pointer */
		Ptr = (UNBYTE *) Object_ptrs[i];

		/* Anything there ? */
		if (Ptr)
		{
			/* Yes -> Above deleted object ? */
			if (Ptr > Start)
			{
				/* Yes -> Adjust pointer */
				Object_ptrs[i] = (struct Object *) (Ptr - Diff);
			}
		}
	}

	/* Count down */
	Last_object_offset -= Diff;
	Nr_objects--;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Execute_method
 * FUNCTION  : Execute an object's method.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 14:30
 * LAST      : 04.08.94 14:30
 * INPUTS    : UNSHORT Handle - Object handle.
 *             UNSHORT Method -  Method ID.
 *             union Method_parms *P - Method-specific parameters.
 * RESULT    : UNLONG : The result of the method.
 * BUGS      : No known.
 * NOTES     : - The method will receive a pointer to the object and to the
 *              method-specific parameters.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Execute_method(UNSHORT Handle, UNSHORT Method, union Method_parms *P)
{
	struct Object *Object;
	struct Method *M;

	/* Get object data */
	Object = Get_object_data(Handle);
	if (!Object)
	{
		return 0;
	}

	/* Get method list */
	M = Object->Class->Method_list;
	if (!M)
		return FALSE;

	/* End of method list ? */
	while (M->ID)
	{
		/* No -> Found method ? */
		if (M->ID == Method)
		{
			/* Yes -> Execute */
			return ((M->Function)(Object, P));
		}
		/* No -> Next method */
		M++;
	}

	return FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Execute_child_methods
 * FUNCTION  : Execute an object's childrens' method.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 14:30
 * LAST      : 04.08.94 14:30
 * INPUTS    : UNSHORT Handle - Object handle.
 *             UNSHORT Method -  Method ID.
 *             union Method_parms *P - Method-specific parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The method will receive a pointer to the object and to the
 *              method-specific parameters.
 *             - Execute_method() is called for each child object.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Execute_child_methods(UNSHORT Handle, UNSHORT Method, union Method_parms *P)
{
	struct Object *Object;
	UNSHORT h;

	/* Get object data */
	Object = Get_object_data(Handle);
	if (!Object)
		return;

	/* Get first child */
	h = Object->Child;

	/* Last child ? */
	while (h)
	{
		/* No -> Execute method */
		Execute_method(h, Method, P);

		/* Next brother */
	   Object = Get_object_data(h);
		if (Object)
		{
	   	h = Object->Next;
		}
		else
		{
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Execute_broadcast_method
 * FUNCTION  : Execute a method for every object.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 13:13
 * LAST      : 06.09.94 13:13
 * INPUTS    : UNSHORT Handle - First object handle / 0 (entire tree).
 *             UNSHORT Method -  Method ID.
 *             union Method_parms *P - Method-specific parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The method will receive a pointer to the object and to the
 *              method-specific parameters.
 *             - All child objects of the first object and their brothers and
 *              subsequent children are examined. If they can handle this
 *              method, their children won't be checked. Thus, it is the
 *              object's responsibility to call the update method of it's
 *              children.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Execute_broadcast_method(UNSHORT Handle, UNSHORT Method, union Method_parms *P)
{
	struct Object *Object;

	/* Layer 1 ? */
	if (!Handle)
	{
		/* Yes -> Get handle of the first object in the tree */
		Handle = Get_root_object_handle();
		if (Handle)
		{
			/* Get object data */
			Object = Get_object_data(Handle);
			if (!Object)
			{
				Handle = 0;
			}
		}
	}
	else
	{
		/* No -> Does the first object have the method ? */
		if (Has_method(Handle, Method))
		{
			/* Yes -> Execute and exit */
			Execute_method(Handle, Method, P);
			return;
		}

		/* No -> Get object data */
		Object = Get_object_data(Handle);
		if (Object)
		{
			/* Start with the first object's child */
			Handle = Object->Child;
		}
		else
		{
			Handle = 0;
		}
	}

	/* Do it */
	if (Handle)
	{
		Do_execute_broadcast_method(Handle, Method, P);
	}
}

void
Do_execute_broadcast_method(UNSHORT Handle, UNSHORT Method, union Method_parms *P)
{
	struct Object *Object;

	while (Handle)
	{
		/* Get object data */
		Object = Get_object_data(Handle);
		if (!Object)
			break;

		/* Does this object have the method ? */
		if (Has_method(Handle, Method))
		{
			/* Yes -> Execute */
			Execute_method(Handle, Method, P);
		}
		else
		{
			/* No -> Does object have a child ? */
			if (Object->Child)
			{
				/* Yes -> Broadcast to children */
				Do_execute_broadcast_method(Object->Child, Method, P);
			}
		}
		/* Next brother */
		Handle = Object->Next;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Execute_upcast_method
 * FUNCTION  : Execute a method for one of an object's parents which can
 *              handle the method.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.09.95 16:21
 * LAST      : 26.09.95 21:06
 * INPUTS    : UNSHORT Handle - First object handle.
 *             UNSHORT Method -  Method ID.
 *             union Method_parms *P - Method-specific parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The method will receive a pointer to the object and to the
 *              method-specific parameters.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Execute_upcast_method(UNSHORT Handle, UNSHORT Method, union Method_parms *P)
{
	struct Object *Object;

	/* Check parent */
	Object = Get_object_data(Handle);
	if (Object)
	{
		Handle = Object->Parent;
	}
	else
	{
		Handle = 0;
	}

	/* While there is an object */
	while (Handle)
	{
		/* Does this object have the method ? */
		if (Has_method(Handle, Method))
		{
			/* Yes -> Execute and exit */
			Execute_method(Handle, Method, P);
		 	break;
		}

		/* No -> Check parent */
		Object = Get_object_data(Handle);
		if (Object)
		{
			Handle = Object->Parent;
		}
		else
		{
			Handle = 0;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_object_size
 * FUNCTION  : Change an object's size.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.04.95 11:28
 * LAST      : 05.04.95 11:28
 * INPUTS    : UNSHORT Handle - Object handle.
 *             UNSHORT Width - New width.
 *             UNSHORT Height - New height.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_object_size(UNSHORT Handle, UNSHORT Width, UNSHORT Height)
{
	struct Object *Object;

	/* Get object data */
	Object = Get_object_data(Handle);
	if (Object)
	{
		/* Change object size */
		Object->Rect.width = Width;
		Object->Rect.height = Height;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_object_position
 * FUNCTION  : Change an object's position.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.04.95 11:25
 * LAST      : 06.07.95 12:42
 * INPUTS    : UNSHORT Handle - Object handle.
 *             SISHORT X - New X-coordinate.
 *             SISHORT Y - New Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function only changes the on-screen position, not the
 *              display position.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_object_position(UNSHORT Handle, SISHORT X, SISHORT Y)
{
	struct Object *Object;

	/* Get object data */
	Object = Get_object_data(Handle);
	if (Object)
	{
		/* Change object position */
		Object->Rect.left = X;
		Object->Rect.top = Y;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_get_object_data
 * FUNCTION  : Get an object's data.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.10.94 17:48
 * LAST      : 19.10.94 17:48
 * INPUTS    : UNSHORT Handle - Object handle.
 *             int Line_nr - Line number of caller.
 *             char *Filename - Filename of caller.
 * RESULT    : struct Object * : Pointer to target object / NULL (error).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Object *
Do_get_object_data(UNSHORT Handle, int Line_nr, char *Filename)
{
	struct Object *Output = NULL;

	/* Is the handle zero ? */
	if (!Handle)
	{
		/* Yes -> Error */
//		Report_control_error(ERROR_OBJECT_HANDLE_ZERO, Line_nr, Filename);
	}
	else
	{
		/* No -> Illegal handle ? */
		if (Handle >= MAX_UI_OBJECTS)
		{
			/* Yes -> Error */
			Report_control_error(ERROR_ILLEGAL_OBJECT_HANDLE, Line_nr, Filename);
		}
		else
		{
			/* No -> Get pointer to object */
			Output = Object_ptrs[Handle - 1];

			/* Does the object exist ? */
			if (!Output)
			{
				/* No -> Error */
//				Report_control_error(ERROR_OBJECT_DOESNT_EXIST, Line_nr, Filename);
			}
		}
	}

	return Output;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Is_object_present
 * FUNCTION  : Check if an object is present.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.95 18:36
 * LAST      : 30.09.95 15:18
 * INPUTS    : UNSHORT Handle - Object handle.
 * RESULT    : BOOLEAN : TRUE (present) or FALSE (not present).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Is_object_present(UNSHORT Handle)
{
	BOOLEAN Result = FALSE;

	/* Is the handle zero ? */
	if (Handle)
	{
		/* No -> Illegal handle ? */
		if (Handle >= MAX_UI_OBJECTS)
		{
			/* Yes -> Error */
			Control_error(ERROR_ILLEGAL_OBJECT_HANDLE);
		}
		else
		{
			/* No -> Does the object exist ? */
			if (Object_ptrs[Handle - 1])
			{
				/* Yes */
				Result = TRUE;
			}
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wait_4_object
 * FUNCTION  : Wait until an object is deleted.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 16:58
 * LAST      : 12.07.95 18:38
 * INPUTS    : UNSHORT Handle - Handle of object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wait_4_object(UNSHORT Handle)
{
	struct Object *Object;
	UNSHORT Counter;

	/* Get object data */
	Object = Get_object_data(Handle);
	if (!Object)
		return;

	/* Set flag */
	Object->Flags |= OBJECT_WARN_WHEN_DELETED;

	/* Load and increase counter */
	Warn_when_deleted_counter++;
	Counter = Warn_when_deleted_counter;

	/* This is the main loop. After each element follows a check to see if
	  the object was deleted. */
	for(;;)
	{
		Update_display();
		if ((Warn_when_deleted_counter != Counter) || Quit_program_flag)
			break;

		Handle_input();
		if ((Warn_when_deleted_counter != Counter) || Quit_program_flag)
			break;

		Switch_screens();
		if ((Warn_when_deleted_counter != Counter) || Quit_program_flag)
			break;
	}

	/* Still there ? */
	if (Is_object_present(Handle))
	{
		/* Yes -> Get object data */
		Object = Get_object_data(Handle);

		/* Clear flag */
		Object->Flags &= ~OBJECT_WARN_WHEN_DELETED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Has_method
 * FUNCTION  : Check if an object has a certain method.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 19:45
 * LAST      : 04.08.94 19:45
 * INPUTS    : UNSHORT Handle - Handle of object.
 *             UNSHORT Method - Method ID.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Has_method(UNSHORT Handle, UNSHORT Method)
{
	struct Object *Object;
	struct Method *M;

	/* Get object data */
	Object = Get_object_data(Handle);
	if (!Object)
		return FALSE;

	/* Get method list */
	M = Object->Class->Method_list;
	if (!M)
		return FALSE;

	/* End of method list ? */
	while (M->ID)
	{
		/* No -> Found method ? */
		if (M->ID == Method)
		{
			/* Yes -> Success */
			return TRUE;
		}
		/* No -> Next method */
		M++;
	}

	/* Not found */
	return FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Is_over_object
 * FUNCTION  : Check if the mouse is over an object.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 14:29
 * LAST      : 20.10.94 14:29
 * INPUTS    : UNSHORT Handle - Handle of object.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Is_over_object(UNSHORT Handle)
{
	struct Object *Object;

	/* Get object data */
	Object = Get_object_data(Handle);
	if (!Object)
		return FALSE;

	/* Exit if control object */
	if (Object->Flags & OBJECT_CONTROL)
		return FALSE;

	/* Is the mouse in the rectangle ? */
	if ((Mouse_X < Object->Rect.left) || (Mouse_X >= Object->Rect.left +
		Object->Rect.width) || (Mouse_Y < Object->Rect.top) || (Mouse_Y >=
		Object->Rect.top + Object->Rect.height))
	{
		/* No */
		return FALSE;
	}
	else
	{
		/* Yes */
		return TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Delete_self
 * FUNCTION  : Delete self.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.09.94 14:20
 * LAST      : 23.06.95 16:44
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * NOTES     : - This function can serve as an object method.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Delete_self(struct Object *Object, union Method_parms *P)
{
	Delete_object(Object->Self);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Object_at
 * FUNCTION  : Find an object at a certain position.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 14:09
 * LAST      : 04.07.95 16:08
 * INPUTS    : SISHORT mX - X-coordinate.
 *             SISHORT mY - Y-coordinate.
 * RESULT    : UNSHORT : Object handle (0 = failure).
 * BUGS      : No known.
 * NOTES     : - This function will find the smallest object containing the
 *              given coordinates.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Object_at(SISHORT mX, SISHORT mY)
{
	/* Call recursive function */
	return(Find_object_at(mX, mY, Get_root_object_handle()));
}

UNSHORT
Find_object_at(SISHORT mX, SISHORT mY, UNSHORT Handle)
{
	struct Object *Object;
	UNSHORT Handle2;

	while (Handle)
	{
		/* Get object data */
		Object = Get_object_data(Handle);
		if (!Object)
			break;

		/* Is this a control object ? */
		if (Object->Flags & OBJECT_CONTROL)
		{
			/* Yes -> Does it have children ? */
			if (Object->Child)
			{
				/* Yes -> Check them */
				Handle2 = Find_object_at(mX, mY, Object->Child);

				/* Found anything ? */
				if (Handle2)
				{
					/* Yes -> Exit */
					return Handle2;
				}
			}

			/* No -> Found this object */
			return (Object->Self);
		}
		else
		{
			/* No -> Are the mouse coordinates within it's area ? */
			if ((mX >= Object->Rect.left) && (mX < Object->Rect.left +
			 Object->Rect.width) && (mY >= Object->Rect.top) && (mY <
			 Object->Rect.top + Object->Rect.height))
			{
				/* Yes -> Does it have children ? */
				if (Object->Child)
				{
					/* Yes -> Check them */
					Handle2 = Find_object_at(mX, mY, Object->Child);

					/* Found anything ? */
					if (Handle2)
					{
						/* Yes -> Exit */
						return Handle2;
					}
				}

				/* No -> Found this object */
				return (Object->Self);
			}
		}

		/* Check the object's brother */
		Handle = Object->Next;
	}

	/* The handle was zero */
	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Disable_object
 * FUNCTION  : Disable an object.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 14:51
 * LAST      : 23.06.95 14:51
 * INPUTS    : UNSHORT Handle - Handle of object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Disable_object(UNSHORT Handle)
{
	struct Object *Object;

	/* Get object data */
	Object = Get_object_data(Handle);
	if (Object)
	{
		/* Disable */
		Object->Flags |= OBJECT_DISABLED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Enable_object
 * FUNCTION  : Enable an object.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 14:51
 * LAST      : 23.06.95 14:51
 * INPUTS    : UNSHORT Handle - Handle of object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Enable_object(UNSHORT Handle)
{
	struct Object *Object;

	/* Get object data */
	Object = Get_object_data(Handle);
	if (Object)
	{
		/* Enable */
		Object->Flags &= ~OBJECT_DISABLED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_root
 * FUNCTION  : Push a new root on the root stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 16:50
 * LAST      : 26.06.95 12:31
 * INPUTS    : struct OPM *OPM - Pointer to OPM for tree.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Push_root(struct OPM *OPM)
{
	/* Is there room on the stack ? */
	if (Root_stack_index < MAX_ROOTS - 1)
	{
		/* Yes -> Increase stack index */
		Root_stack_index++;

		/* Initialize new root */
		Root_stack[Root_stack_index].First_object	= 0;
		Root_stack[Root_stack_index].OPM				= OPM;

		/* Set current OPM */
		Current_OPM = OPM;

		/* Add to update list */
		Add_update_OPM(OPM);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_root
 * FUNCTION  : Pops a root from the top of the root stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 16:50
 * LAST      : 26.06.95 12:32
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will delete all objects in the current tree.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_root(void)
{
	UNSHORT Handle;
	UNSHORT i;

	/* Is the stack empty ? */
	if (Root_stack_index)
	{
		/* No -> Delete tree (if any) */
		Handle = Get_root_object_handle();
		if (Handle)
			Delete_object(Handle);

		/* Delete update OPMs */
		for (i=0;i<MAX_UPDATE_OPMS;i++)
		{
			OPM_update_list[Root_stack_index][i] = NULL;
		}

		/* Decrease stack index */
		Root_stack_index--;

		/* Set current OPM */
		Current_OPM = Root_stack[Root_stack_index].OPM;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_root_stack
 * FUNCTION  : Reset the root stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 16:50
 * LAST      : 29.10.95 13:49
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_root_stack(void)
{
	#if FALSE
	UNSHORT i, j;

	/* Delete all trees on the stack */
	for (i=Root_stack_index;i>0;i--)
	{
		if (Root_stack[i].First_object)
		{
			/* Delete tree */
			Delete_object(Root_stack[i].First_object);

			/* Delete update OPMs */
			for (j=0;j<MAX_UPDATE_OPMS;j++)
			{
				OPM_update_list[i][j] = NULL;
			}
		}
	}
	#endif

	/* Clear stack */
	Root_stack_index = 0;

	/* Initialize new root */
	Root_stack[Root_stack_index].First_object	= 0;
	Root_stack[Root_stack_index].OPM				= &Main_OPM;

	/* Set current OPM */
	Current_OPM = &Main_OPM;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_root_OPM
 * FUNCTION  : Set the OPM of the current root.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.01.95 13:23
 * LAST      : 26.06.95 12:32
 * INPUTS    : struct OPM *OPM - Pointer to OPM for tree.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_root_OPM(struct OPM *OPM)
{
	/* Remove current root OPM from update list */
	Remove_update_OPM(Root_stack[Root_stack_index].OPM);

	/* Set root OPM */
	Root_stack[Root_stack_index].OPM = OPM;

	/* Set current OPM */
	Current_OPM = OPM;

	/* Add to update list */
	Add_update_OPM(OPM);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_root_object_handle
 * FUNCTION  : Get the handle of the current root object.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.01.95 13:29
 * LAST      : 12.01.95 13:29
 * INPUTS    : None.
 * RESULT    : UNSHORT : Object handle of root object.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_root_object_handle(void)
{
	return(Root_stack[Root_stack_index].First_object);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_object_pool
 * FUNCTION  : Clear the object pool.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.10.95 14:15
 * LAST      : 29.10.95 13:52
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clear_object_pool(void)
{
	UNSHORT i;

	/* Clear variables */
	Feedback_object				= 0;
	Highlighted_object			= 0;
	Focussed_object				= 0;

	Nr_objects						= 0;
	Last_object_offset			= 0;
	Warn_when_deleted_counter	= 0;

	/* Clear object pool */
	BASEMEM_FillMemByte
	(
		Object_pool,
		UI_OBJECT_POOL_SIZE,
		0
	);

	/* Clear object pointer list */
	for (i=0;i<MAX_UI_OBJECTS;i++)
	{
		Object_ptrs[i] = NULL;
	}

	/* Clear root stack */
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &(Root_stack[0]),
		MAX_ROOTS * sizeof(struct Root),
		0
	);

	/* Clear OPM update lists */
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &(OPM_update_list[0][0]),
		MAX_ROOTS * MAX_UPDATE_OPMS * sizeof(struct OPM *),
		0
	);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_module
 * FUNCTION  : Push a new module on the module stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 13:45
 * LAST      : 31.10.95 15:02
 * INPUTS    : struct Module *Module - Pointer to the new module.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Push_module(struct Module *Module)
{
	UNSHORT Counter;

	/* Is there room on the stack ? */
	if (Module_stack_index < MAX_MODULES - 1)
	{
		/* Yes -> Increase stack index */
		Module_stack_index++;

		/* Copy the new module's data to the stack */
		BASEMEM_CopyMem
		(
			(UNBYTE *) Module,
			(UNBYTE *) &(Module_stack[Module_stack_index]),
			sizeof(struct Module)
		);

		/* Clear input buffer */
		Clear_input_buffer();

		/* Reset the update duration */
		Update_duration_time_start = 0;
		Update_duration = 3;

		/* Is this a local module ? */
		if (Module->Flags & LOCAL_MOD)
		{
			/* Yes -> Increase local pop semaphore */
			Counter = Pop_counter;
			Counter++;
			Pop_counter = Counter;

			/* Initialize the module */
			Init_module(&(Module_stack[Module_stack_index]));

			/* This is the local main loop. After each element follows a check
			 to see if the module was popped. */
			while ((Counter <= Pop_counter) && (!Quit_program_flag))
			{
				Main_loop();
	 			if ((Counter > Pop_counter) || (Quit_program_flag))
	 				break;

				Handle_input();
	 			if ((Counter > Pop_counter) || (Quit_program_flag))
	 				break;

				Switch_screens();
			}
		}
		else
 		{
			/* No -> Initialize the module */
			Init_module(&(Module_stack[Module_stack_index]));
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_module
 * FUNCTION  : Pops a module from the top of the module stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 13:50
 * LAST      : 05.08.94 13:50
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_module(void)
{
	struct Module *Module;

	/* Is the stack empty ? */
	if (Module_stack_index)
	{
		/* No */
		Module = &Module_stack[Module_stack_index];

		/* Clear input buffer */
		Clear_input_buffer();

		/* If the current module is local, the local pop semaphore is
		  decreased. */
		if (Module->Flags & LOCAL_MOD)
			Pop_counter--;

		/* Decrease stack index */
		Module_stack_index--;

		/* Terminate the popped module */
		Exit_module(Module);

		/* Reset the update duration */
		Update_duration = 3;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_module_stack
 * FUNCTION  : Reset the module stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 13:52
 * LAST      : 31.10.95 14:57
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_module_stack(void)
{
	UNSHORT Nr_modules_on_stack;
	UNSHORT i;

	/* Pop all modules from the stack */
	Nr_modules_on_stack = Module_stack_index;
	for (i=0;i<Nr_modules_on_stack;i++)
	{
		Pop_module();
	}

	/* Clear variables */
	Pop_counter = 0;
	Update_duration_time_start = 0;

	/* Reset stack */
	Module_stack_index = 0;

	/* Put the default module on the stack */
	BASEMEM_CopyMem
	(
		(UNBYTE *) &Module_stack[0],
		(UNBYTE *) &Default_module,
		sizeof(struct Module)
	);

	/* Initialize the default module */
	Init_module(&Module_stack[0]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_module
 * FUNCTION  : Initialize a new module.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 14:05
 * LAST      : 30.09.95 15:06
 * INPUTS    : struct Module *Module - Pointer to the new module.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_module(struct Module *Module)
{
	/* Clear permanent text window */
	Clear_permanent_text();

	/* Call module init function (if any) */
	if (Module->ModInit_function)
		(Module->ModInit_function)();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_module
 * FUNCTION  : Terminate a module.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 14:10
 * LAST      : 30.09.95 15:05
 * INPUTS    : struct Module *Module - Pointer to the module.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_module(struct Module *Module)
{
	/* Call module exit function (if any) */
	if (Module->ModExit_function)
		(Module->ModExit_function)();

	/* Was this NOT a mode module ? */
	if (Module->Type != MODE_MOD)
	{
		/* Yes -> Clear permanent text window */
		Clear_permanent_text();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_loop
 * FUNCTION  : Call current module's MainLoop function.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.09.94 13:39
 * LAST      : 05.09.94 13:39
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_loop(void)
{
	struct Module *Module;

	/* Get current module */
	Module = &Module_stack[Module_stack_index];

	/* Call MainLoop function (if any) */
	if (Module->MainLoop_function)
		(Module->MainLoop_function)();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_display
 * FUNCTION  : Call current module's DisInit function.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 14:25
 * LAST      : 05.08.94 14:25
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_display(void)
{
	struct Module *Module;

	/* Get current module */
	Module = &Module_stack[Module_stack_index];

	/* Call DisInit function (if any) */
	if (Module->DisInit_function)
		(Module->DisInit_function)();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_display
 * FUNCTION  : Call current module's DisExit function.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 14:25
 * LAST      : 05.08.94 14:25
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_display(void)
{
	struct Module *Module;

	/* Get current module */
	Module = &Module_stack[Module_stack_index];

	/* Call DisExit function (if any) */
	if (Module->DisExit_function)
		(Module->DisExit_function)();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_display
 * FUNCTION  : Call current module's DisUpd function.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 14:25
 * LAST      : 31.10.95 14:55
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_display(void)
{
	struct Module *Module;
	UNLONG T;

	/* Calculate elapsed time */
//	if (Recording_flic)
//		Update_duration = 3;
//	else
	{
		T = SYSTEM_GetTicks();

		if (!Update_duration_time_start)
			Update_duration_time_start = T;

		if (Update_duration_time_start > T)
			Update_duration = Update_duration_time_start - T + 1;
		else
			Update_duration = T - Update_duration_time_start + 1;

		Update_duration_time_start = T;
	}

	/* Get current module */
	Module = &Module_stack[Module_stack_index];

	/* Call DisUpd function (if any) */
	if (Module->DisUpd_function)
		(Module->DisUpd_function)();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Current_screen_type
 * FUNCTION  : Get current module's screen type.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.04.95 16:55
 * LAST      : 29.04.95 18:00
 * INPUTS    : UNSHORT Levels - Number of levels to go down into the stack.
 * RESULT    : UNSHORT : Screen type.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Current_screen_type(UNSHORT Levels)
{
	struct Module *Module;
	SISHORT Index;
	UNSHORT Screen_type;

	/* Calculate stack index */
	Index = Module_stack_index - Levels;

	/* Too low ? */
	if (Index < 0)
	{
		/* Yes -> No screen type */
		Screen_type = NO_SCREEN;
	}
	else
	{
		/* No -> Get module */
		Module = &Module_stack[Index];

		/* Get screen type */
		Screen_type = Module->Screen_type;
	}

	return Screen_type;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_screen_type
 * FUNCTION  : Set the current module's screen type.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 20:57
 * LAST      : 12.08.95 20:57
 * INPUTS    : UNSHORT Screen type - New screen type.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_screen_type(UNSHORT Screen_type)
{
	/* Set screen type */
	Module_stack[Module_stack_index].Screen_type = Screen_type;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_mouse_system
 * FUNCTION  : Initialise the mouse pointer system.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.94 17:41
 * LAST      : 03.11.95 20:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_mouse_system(void)
{
	/* Clear hide counter */
	Mouse_counter = 0;

	/* Reset the stack */
	Mouse_stack_index = 0;

	/* Mouse system initialised ? */
	if (!Mouse_system_initialised)
	{
		/* No -> Show mouse pointer */
		SYSTEM_ShowMousePtr(SYSTEM_MousePointerNormal);

		/* Lock mouse images */
		BASEMEM_Lock_region
		(
			(UNBYTE *) Mouse_images,
			sizeof(Mouse_images)
		);

		/* Initialize new mouse pointer */
		Mouse_stack[0] = &(Mouse_pointers[DEFAULT_MPTR]);
		Init_mouse_pointer(Mouse_stack[0]);

		/* Mouse system has been initialised */
		Mouse_system_initialised = TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_mouse_system
 * FUNCTION  : Exit the mouse pointer.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.94 17:41
 * LAST      : 03.11.95 20:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_mouse_system(void)
{
	/* Mouse system initialised ? */
	if (Mouse_system_initialised)
	{
		/* Yes -> Show normal mouse pointer */
		SYSTEM_ShowMousePtr(SYSTEM_MousePointerNormal);

		/* Unlock mouse images */
		BASEMEM_Unlock_region
		(
			(UNBYTE *) Mouse_images,
			sizeof(Mouse_images)
		);

		/* Mouse system is no longer initialised */
		Mouse_system_initialised = FALSE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_mouse
 * FUNCTION  : Push a new mouse pointer on the mouse pointer stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.94 17:36
 * LAST      : 14.10.95 20:59
 * INPUTS    : struct Mouse_pointer *New;
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Push_mouse(struct Mouse_pointer *New)
{
	/* Is there room on the stack ? */
	if (Mouse_stack_index < MAX_MOUSE_POINTERS - 1)
	{
		/* Yes -> Increase stack index */
		Mouse_stack_index++;

		/* Initialize new mouse pointer */
		Mouse_stack[Mouse_stack_index] = New;
		Init_mouse_pointer(New);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_mouse
 * FUNCTION  : Pops a mouse pointer from the top of the mouse pointer stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.94 17:39
 * LAST      : 14.10.95 20:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_mouse(void)
{
	/* Is the stack empty ? */
	if (Mouse_stack_index)
	{
		/* No -> Decrease stack index */
		Mouse_stack_index--;

		/* Re-initialize old mouse pointer */
		Init_mouse_pointer(Mouse_stack[Mouse_stack_index]);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_mouse
 * FUNCTION  : Change the mouse pointer on the top of the mouse pointer stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.94 11:58
 * LAST      : 14.10.95 20:59
 * INPUTS    : struct Mouse_pointer *New;
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_mouse(struct Mouse_pointer *New)
{
	/* Any change ? */
	if (Mouse_stack[Mouse_stack_index] != New)
	{
		/* Yes -> Initialize new mouse pointer */
		Mouse_stack[Mouse_stack_index] = New;
		Init_mouse_pointer(New);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_mouse_pointer
 * FUNCTION  : Initialize a new mouse pointer.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.94 17:42
 * LAST      : 14.10.95 20:57
 * INPUTS    : struct Mouse_pointer *New - Pointer to a new mouse pointer.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_mouse_pointer(struct Mouse_pointer *New)
{
	static const UNCHAR Info_chunk[] = "INFO";
	static const UNCHAR Body_chunk[] = "BODY";
	static const UNCHAR End_chunk[] = "ENDE";

	struct GTO *GTO;
	UNLONG Size;
	UNBYTE *Ptr;

	/* Exit if new mouse pointer is NULL or if the mouse pointer is hidden */
	if (!New || Mouse_counter)
		return;

	/* Calculate size of sprite image */
	Size = New->Width * New->Height;

	/* Exit if the overall size of the image is too large */
	if ((sizeof(struct GTO) + Size + 4) > MAX_MOUSE_IMAGE_SIZE)
		return;

	/* Toggle mouse image */
	if (Current_mouse_image)
	{
		Current_mouse_image = 0;
	}
	else
	{
		Current_mouse_image = 1;
	}

	/* Get pointer to new mouse image */
	Ptr = &(Mouse_images[Current_mouse_image][0]);

	/* Build GTO */
	GTO = (struct GTO *) Ptr;

	Write_chunk_name((UNCHAR *) GTO->infochunk, Info_chunk);

	GTO->name[0]	= 0;
	GTO->transcol	= 0;
	GTO->plane		= 0xFF;
	GTO->xoffset	= 0 - New->X;
	GTO->yoffset	= 0 - New->Y;
	GTO->width		= New->Width;
	GTO->height		= New->Height;

	Write_chunk_name((UNCHAR *) GTO->bodychunk, Body_chunk);

	/* Copy graphics */
	memcpy(Ptr + sizeof(struct GTO), New->Graphics, Size);

	/* Insert ENDE chunk */
	Write_chunk_name(Ptr + sizeof(struct GTO) + Size, End_chunk);

	/* Switch mouse pointers */
	SYSTEM_HideMousePtr();
	SYSTEM_ShowMousePtr(GTO);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Write_chunk_name
 * FUNCTION  : Write a four-character chunk name.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.94 10:53
 * LAST      : 01.09.94 10:53
 * INPUTS    : UNBYTE *Address - Destination address of chunk name.
 *             const UNCHAR *Chunk_name - Chunk name.
 * RESULT    : UNBYTE * : Updated pointer.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Write_chunk_name(UNBYTE *Address, const UNCHAR *Chunk_name)
{
	UNSHORT i;

	for (i=0;i<4;i++)
	{
		*Address++ = *(Chunk_name + i);
	}

	return Address;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Mouse_on
 * FUNCTION  : Show the mouse pointer.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 13:36
 * LAST      : 14.10.95 20:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Mouse_on(void)
{
	/* Is the mouse pointer hidden ? */
	if (Mouse_counter)
	{
		/* Yes -> Count down */
		Mouse_counter--;

		/* Show ? */
		if (!Mouse_counter)
		{
			/* Yes */
//			SYSTEM_ShowMousePtr((struct GTO *) SYSTEM_MousePointerOld);
			Pop_mouse();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Mouse_off
 * FUNCTION  : Hide the mouse pointer.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 13:41
 * LAST      : 14.10.95 20:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Mouse_off(void)
{
	/* Hide ? */
	if (!Mouse_counter)
	{
		/* Yes */
		Push_mouse(&Empty_mouse_pointer);
//		SYSTEM_HideMousePtr();
	}

	/* Count up */
	Mouse_counter++;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_MA
 * FUNCTION  : Push a new mouse area on the MA stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 13:52
 * LAST      : 14.09.94 13:52
 * INPUTS    : struct BBRECT *New;
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Push_MA(struct BBRECT *New)
{
	/* Is there room on the stack ? */
	if (MA_stack_index < MAX_MA - 1)
	{
		/* Yes -> Increase stack index */
		MA_stack_index++;

		/* Initialize new MA */
		memcpy(&(MA_stack[MA_stack_index]), New, sizeof(struct BBRECT));
		Init_MA(New);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_MA
 * FUNCTION  : Pops a mouse area from the top of the MA stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 13:51
 * LAST      : 14.09.94 13:51
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_MA(void)
{
	/* Is the stack empty ? */
	if (MA_stack_index)
	{
		/* No -> Decrease stack index */
		MA_stack_index--;

		/* Re-initialize old MA */
		Init_MA(&(MA_stack[MA_stack_index]));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_MA_stack
 * FUNCTION  : Reset the mouse area stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 13:46
 * LAST      : 14.09.94 13:46
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_MA_stack(void)
{
	/* Reset the stack */
	MA_stack_index = 0;

	/* Initialize new MA */
	Default_MA.left	= 0;
	Default_MA.top		= 0;
	Default_MA.width	= Screen_width;
	Default_MA.height	= Screen_height;

	memcpy(&(MA_stack[0]), &Default_MA, sizeof(struct BBRECT));;
	Init_MA(&(MA_stack[0]));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_MA
 * FUNCTION  : Change the mouse area on top of the MA stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.10.95 19:21
 * LAST      : 19.10.95 19:21
 * INPUTS    : struct BBRECT *New;
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_MA(struct BBRECT *New)
{
	/* Initialize new MA */
	memcpy(&(MA_stack[MA_stack_index]), New, sizeof(struct BBRECT));
	Init_MA(New);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_MA
 * FUNCTION  : Initialize a new mouse area.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 13:47
 * LAST      : 07.01.95 17:48
 * INPUTS    : struct BBRECT *New - Pointer to a new mouse area.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_MA(struct BBRECT *New)
{
	struct BBPOINT Mouse;

	/* Set area */
	SYSTEM_SetMouseArea(New);

	/* Get new mouse coordinates */
	BLEV_GetMousePos(&Mouse);
	Mouse_X = Mouse.x;
	Mouse_Y = Mouse.y;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_screen
 * FUNCTION  : Update the screen.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.03.95 15:02
 * LAST      : 05.07.95 10:58
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_screen(void)
{
	Update_display();
	Update_input();
	Switch_screens();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Switch_screens
 * FUNCTION  : Display the currently invisible screen.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 11:34
 * LAST      : 18.10.95 20:12
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Switch_screens(void)
{
	static UNSHORT Error_stack_entries = 0;
	struct BBPOINT Mouse;

	/* Get time */
	Global_timer = SYSTEM_GetTicks();

	/* Update objects */
	Execute_broadcast_method(0, UPDATE_METHOD, NULL);

	/* Get mouse coordinates */
	Mouse.x = SYSTEMVAR_mousex;
	Mouse.y = SYSTEMVAR_mousey;

	/* Update HDOBs */
	Update_HDOBs();

	/* Draw HDOBs */
	Draw_HDOBs(&Mouse);

	/* Update all OPMs */
	Show_OPMs();

	/* Display new screen */
	DSA_DoubleBuffer();

	/* Has the error stack changed ? */
	if (BBERROR_StackEntries != Error_stack_entries)
	{
		/* Yes -> Has it grown ? */
		if (BBERROR_StackEntries > Error_stack_entries)
		{
			/* Yes -> Start beep (C, second octave) */
			sound(1193180 / (18242 / (1 << 2)));

			/* Wait */
			SYSTEM_WaitTicks(20);

			/* Stop beep */
			nosound();
		}

		/* Remember error stack size */
		Error_stack_entries = BBERROR_StackEntries;
	}

	/* Has the palette changed ? */
	if (Palette_has_changed)
	{
		/* Yes -> Activate palette */
		Activate_palette();

		/* Clear flag */
		Palette_has_changed = FALSE;
	}

	/* Remove HDOBs */
	Erase_HDOBs(&Mouse);

//	Record_flic_frame();

	/* Update music and sound effects */
	SOUND_Update_sound_system();

	/* Pause if CONTROL is pressed */
//	while (SYSTEM_GetBLEVStatusLong() & BLEV_CTRL) {};
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_update_OPM
 * FUNCTION  : Add an OPM to the update list.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.09.94 13:16
 * LAST      : 05.09.94 13:16
 * INPUTS    : struct OPM *OPM - Pointer to OPM that must be added.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_update_OPM(struct OPM *OPM)
{
	UNSHORT i;

	/* Is main OPM ? */
	if (OPM == &Main_OPM)
		return;

	/* No -> Is this OPM already in the list ? */
	for (i=0;i<MAX_UPDATE_OPMS;i++)
	{
		/* Is this the input OPM ? */
		if (OPM_update_list[Root_stack_index][i] == OPM)
		{
			return;
		}
	}

	/* No -> Find free slot */
	for (i=0;i<MAX_UPDATE_OPMS;i++)
	{
		/* Is this slot free ? */
		if (!OPM_update_list[Root_stack_index][i])
		{
			/* Yes -> Insert new entry */
			OPM_update_list[Root_stack_index][i] = OPM;
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_update_OPM
 * FUNCTION  : Remove an OPM from the update list.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.09.94 13:34
 * LAST      : 05.09.94 13:34
 * INPUTS    : struct OPM *OPM - Pointer to OPM that must be removed.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_update_OPM(struct OPM *OPM)
{
	UNSHORT i;

	/* Is main OPM ? */
	if (OPM == &Main_OPM)
		return;

	/* No -> Find OPM in list */
	for (i=0;i<MAX_UPDATE_OPMS;i++)
	{
		/* Is this the right slot ? */
		if (OPM_update_list[Root_stack_index][i] == OPM)
		{
			/* Yes -> Remove entry */
			OPM_update_list[Root_stack_index][i] = NULL;
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_OPMs
 * FUNCTION  : Show all OPMs in the update list.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 11:34
 * LAST      : 05.09.94 13:16
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_OPMs(void)
{
	struct OPM *OPM;
	UNSHORT i;

	/* Has the main OPM changed ? */
	if (Main_OPM.status & OPMSTAT_CHANGED)
	{
		/* Yes -> Copy the main OPM to the screen */
		DSA_CopyMainOPMToScreen(&Screen, DSA_CMOPMTS_ALWAYS);

		/* Set flag for next frame */
		Main_OPM.status &= ~OPMSTAT_CHANGED;
		Main_OPM.status |= OPMSTAT_USERDEFINED;

		/* Clear the changed flags of virtual OPMs in the main OPM. */
		/* The main OPM will be updated the next frame anyway. */
		for (i=0;i<MAX_UPDATE_OPMS;i++)
		{
			OPM = OPM_update_list[Root_stack_index][i];

			if (OPM && (OPM->status & OPMSTAT_INIT))
			{
				if ((OPM->status & OPMSTAT_VIRTUAL) &&
				 (OPM->virtualsrc == &Main_OPM))
				{
					OPM->status &= ~(OPMSTAT_CHANGED | OPMSTAT_USERDEFINED);
				}
			}
		}

		/* Copy all non-virtual OPMs to the screen (if changed) */
		for (i=0;i<MAX_UPDATE_OPMS;i++)
		{
			OPM = OPM_update_list[Root_stack_index][i];

			if (OPM && (OPM->status & OPMSTAT_INIT))
			{
				if (!((OPM->status & OPMSTAT_VIRTUAL) &&
				 (OPM->virtualsrc == &Main_OPM)))
				{
					/* Has this OPM changed ? */
					if (OPM->status & OPMSTAT_CHANGED)
					{
						/* Yes -> Copy it */
						DSA_CopyOPMToScreen(&Screen, OPM, 0, 0, DSA_CMOPMTS_ALWAYS);

						/* Set flag for next frame */
						OPM->status &= ~OPMSTAT_CHANGED;
						OPM->status |= OPMSTAT_USERDEFINED;
					}
					else
					{
						/* Was this OPM updated in the previous frame ? */
						if (OPM->status & OPMSTAT_USERDEFINED)
						{
							/* Yes -> Copy it AGAIN for the other buffer */
							DSA_CopyOPMToScreen(&Screen, OPM, 0, 0, DSA_CMOPMTS_ALWAYS);

							/* Clear flag */
							OPM->status &= ~OPMSTAT_USERDEFINED;
						}
					}
				}
			}
		}
	}
	else
	{
		/* No -> Was the main OPM updated in the previous frame ? */
		if (Main_OPM.status & OPMSTAT_USERDEFINED)
		{
			/* Yes -> Copy it AGAIN for the other buffer */
			DSA_CopyMainOPMToScreen(&Screen, DSA_CMOPMTS_ALWAYS);

			/* Clear flag */
			Main_OPM.status &= ~OPMSTAT_USERDEFINED;

			/* Check other OPMs */
			for (i=0;i<MAX_UPDATE_OPMS;i++)
			{
				OPM = OPM_update_list[Root_stack_index][i];
				if (OPM && (OPM->status & OPMSTAT_INIT))
				{
					/* Is this a virtual OPM in the main OPM ? */
					if ((OPM->status & OPMSTAT_VIRTUAL)
						&& (OPM->virtualsrc == &Main_OPM))
					{
						/* Yes -> Has this virtual OPM changed ? */
						if (OPM->status & OPMSTAT_CHANGED)
						{
							/* Yes -> Set flag for next frame */
							OPM->status &= ~OPMSTAT_CHANGED;
							OPM->status |= OPMSTAT_USERDEFINED;
						}
						else
						{
							/* No -> Flag is useless now */
							OPM->status &= ~OPMSTAT_USERDEFINED;
						}
					}
				}
			}
		}
		else
		{
			/* No -> Just copy changed OPMs */
			Show_changed_OPMs();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_changed_OPMs
 * FUNCTION  : Show all changed OPMs in the update list.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.10.94 10:55
 * LAST      : 17.10.94 10:55
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_changed_OPMs(void)
{
	struct OPM *OPM;
	UNSHORT i;

	/* Copy all the OPMs to the screen (if changed) */
	for (i=0;i<MAX_UPDATE_OPMS;i++)
	{
		OPM = OPM_update_list[Root_stack_index][i];

		if (OPM && (OPM->status & OPMSTAT_INIT))
		{
			/* Has this OPM changed ? */
			if (OPM->status & OPMSTAT_CHANGED)
			{
				/* Yes -> Copy it */
				DSA_CopyOPMToScreen(&Screen, OPM, 0, 0, DSA_CMOPMTS_ALWAYS);

				/* Set flag for next frame */
				OPM->status &= ~OPMSTAT_CHANGED;
				OPM->status |= OPMSTAT_USERDEFINED;
			}
			else
			{
				/* Was this OPM updated in the previous frame ? */
				if (OPM->status & OPMSTAT_USERDEFINED)
				{
					/* Yes -> Copy it AGAIN for the other buffer */
					DSA_CopyOPMToScreen(&Screen, OPM, 0, 0, DSA_CMOPMTS_ALWAYS);

					/* Clear flag */
					OPM->status &= ~OPMSTAT_USERDEFINED;
				}
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Report_control_error
 * FUNCTION  : Report a control error.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.10.95 19:39
 * LAST      : 29.10.95 19:25
 * INPUTS    : UNSHORT Error_code - Error code.
 *             int Line_nr - Line number where error was reported.
 *             char *Filename - Filename where error was reported.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Report_control_error(UNSHORT Error_code, int Line_nr, char *Filename)
{
	struct Error_report Report;

	/* Clear error report */
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &Report,
		sizeof(struct Error_report),
		0
	);

	/* Build error report */
	Report.Code			= Error_code;
	Report.Messages	= &(_Control_errors[0]);
	Report.Line_nr		= (UNSHORT) Line_nr;
	Report.Filename	= (UNCHAR *) Filename;

	/* Push error on the error stack */
	ERROR_PushError
	(
		Print_error,
		_Control_library_name,
		sizeof(struct Error_report),
		(UNBYTE *) &Report
	);
}

