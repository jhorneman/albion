/************
 * NAME     : CONTROL.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 4-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>

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
#include <MUSIC.H>

/* global variables */

static struct OPM *OPM_update_list[ROOTS_MAX][UPDATE_OPMS_MAX];
struct OPM *Current_OPM;

UNLONG Update_duration = 1;

BOOLEAN Drag_drop_mode = FALSE;

static Drag_drop_abort_handler Current_drag_drop_abort_handler;

UNSHORT Mouse_X, Mouse_Y;
static UNSHORT Old_mouse_X, Old_mouse_Y;
UNSHORT Button_state;
static UNSHORT Mouse_counter = 0;
static BOOLEAN Clear_input_buffer_flag;

static BOOLEAN Ignore_second_left_click = FALSE;
static BOOLEAN Ignore_second_right_click = FALSE;

UNSHORT Feedback_object;
UNSHORT Highlighted_object;
UNSHORT Focussed_object;

static UNSHORT Nr_objects = 0;
static UNLONG Last_object_offset = 0;
static UNSHORT Warn_when_deleted_counter = 0;

static MEM_HANDLE Mouse_GTO_handle;

static struct Mouse_pointer *Mouse_stack[MOUSE_POINTERS_MAX];
static UNSHORT Mouse_stack_index = 0;

static struct BBRECT MA_stack[MA_MAX];
static UNSHORT MA_stack_index = 0;

struct BBRECT Default_MA;

static struct Root Root_stack[ROOTS_MAX];
static UNSHORT Root_stack_index = 0;

static struct Module Module_stack[MODULES_MAX];
static UNSHORT Module_stack_index = 0;
static UNSHORT Pop_counter = 0;

static struct Object *Object_ptrs[UI_OBJECTS_MAX];
static UNBYTE Object_pool[UI_OBJECT_POOL_SIZE];

struct Module Default_module =
{
	0, SCREEN_MOD, NO_SCREEN,
	NULL, NULL, NULL, NULL, NULL,	NULL
};

static UNCHAR _Control_library_name[] = "Control";

static struct Error_message _Control_errors[] = {
	{ERROR_PARENT_NOT_FOUND, 		"Parent of object could not be found."},
	{ERROR_TOO_MANY_OBJECTS, 		"Too many objects in pool."},
	{ERROR_OBJECT_POOL_IS_FULL,	"Object pool is full."},
	{ERROR_OBJECT_HANDLE_ZERO,		"Reference to object handle zero."},
	{ERROR_OBJECT_DOESNT_EXIST,	"Reference to non-existent object."},
	{ERROR_ILLEGAL_OBJECT_HANDLE, "Reference to illegal object handle."},
	{ERROR_OBJECT_TOO_SMALL,		"Object is too small."},
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
 * LAST      : 03.03.95 13:49
 * INPUTS    : Drag_drop_abort_handler *Handler - Pointer to drag & drop
 *              abort handler.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Enter_drag_drop_mode(Drag_drop_abort_handler Handler)
{
	/* Drag & drop mode is on */
	Drag_drop_mode = TRUE;

	/* Install drag & drop abort handler */
	Current_drag_drop_abort_handler = Handler;

	/* Clear input buffer */
	Clear_input_buffer();

	/* Set an appropriate mouse-pointer */
//	Set_appropriate_mouse();
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

	/* Set an appropriate mouse-pointer */
//	Set_appropriate_mouse();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_event
 * FUNCTION  : Get an input event and handle input recording / playback.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.09.94 12:24
 * LAST      : 19.09.94 12:24
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
	UNSHORT Old, New;

	/* Do thang */
	SYSTEM_SystemTask();

	/* Get event */
	BLEV_GetEvent(Event);

	/* Get mouse coordinates */
	Mouse_X = Event->sl_mouse_x;
	Mouse_Y = Event->sl_mouse_y;

	/* Get old and new button state */
	Old = Button_state;
	New = Button_state & 0x0011;

	/* Check changes in the left and right mouse button states */
	if ((Event->sl_eventtype == BLEV_MOUSELDOWN)
	 || (Event->sl_eventtype == BLEV_MOUSELSDOWN))
		New |= 0x01;

	if ((Event->sl_eventtype == BLEV_MOUSELUP)
	 || (Event->sl_eventtype == BLEV_MOUSELSUP))
		New &= ~0x01;

	if ((Event->sl_eventtype == BLEV_MOUSERDOWN)
	 || (Event->sl_eventtype == BLEV_MOUSERSDOWN))
		New |= 0x10;

	if ((Event->sl_eventtype == BLEV_MOUSERUP)
	 || (Event->sl_eventtype == BLEV_MOUSERSUP))
		New &= ~0x10;

/*	if (Event->ul_pressed_keys & BLEV_MOUSELPRESSED)
	{
		New |= 0x01;
	}
	else
	{
		New &= ~0x01;
	}

	if (Event->ul_pressed_keys & BLEV_MOUSERPRESSED)
	{
		New |= 0x10;
	}
	else
	{
		New &= ~0x10;
	} */

	/* Calculate the complete new button state */
	Button_state = New | (((~Old & 0x0011) & New) << 1)
	 | (((~New & 0x0011) & Old) << 2);

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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wait_4_user
 * FUNCTION  : Wait for the user to click or press a key.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.09.94 14:24
 * LAST      : 13.09.94 14:24
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
	struct BLEV_Event_struct Event;
	BOOLEAN Flag = TRUE;

	/* Wait for the user to release the left mouse button */
	Wait_4_unclick();

	/* Show click mouse-pointer */
	Push_mouse(&(Mouse_pointers[CLICK_MPTR]));

	while (Flag)
	{
		/* Update the display */
		Update_display();

		do
		{
			/* Read event */
			Get_event(&Event);

			/* Did the user click or press a key ? */
			if ((Event.sl_eventtype == BLEV_KEYDOWN)
			 || (Event.sl_eventtype == BLEV_MOUSELSDOWN)
			 || (Event.sl_eventtype == BLEV_MOUSERSDOWN))
			{
				/* Yes -> Exit */
				Flag = FALSE;
				break;
			}
		}
		/* Until there are no more events */
		while (Event.sl_eventtype != BLEV_NOEVENT);

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
			// Mouse_off
			// Set_focus

			/* This causes odd effects and must be changed ! */
//			Highlighted_object = Root_stack[Root_stack_index].First_object;

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
				// Unfocus
				// Mouse_on

				Handle_mouse_event(&Event);
			}
		}
		else
		{
			/* No -> Go to mouse mode */
			Key_or_mouse = -1;
			// Unfocus
			// Mouse_on

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
	Root_object = Root_stack[Root_stack_index].First_object;

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
				/* Abort drag & drop */
				if (Current_drag_drop_abort_handler)
					(Current_drag_drop_abort_handler)();

				/* Change to touch method */
				Method1 = TOUCHED_METHOD;
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
			return(FALSE);
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
						return(TRUE);
				}

				/* No -> Check this object */
				if (Search_mouse_method(Method, Object))
					return(TRUE);
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
							return(TRUE);
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
	return(FALSE);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_mouse_method
 * FUNCTION  : Search a certain method or the custommouse method in an object.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 19:32
 * LAST      : 04.08.94 19:32
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
	/* Does this object have a CUSTOMMOUSE method ? */
	if (Has_method(Object->Self, CUSTOMMOUSE_METHOD))
	{
		/* Yes -> Execute */
		if (Execute_method(Object->Self, CUSTOMMOUSE_METHOD, (union Method_parms *) &Button_state))
		{
			/* Exit if succesful */
			return (TRUE);
		}
	}

	/* No -> Is there another method ? */
	if (Method)
	{
		/* Yes -> Does this object have this method ? */
		if (Has_method(Object->Self, Method))
		{
			/* Yes -> Execute */
			Execute_method(Object->Self, Method, 0L);
			return (TRUE);
		}
	}

	return(FALSE);
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
			return(FALSE);
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
						return(TRUE);
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
							return(TRUE);
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
	return(FALSE);
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

	/* Handle diagnostic keys */
	if (Check_diagnostic_keys(Event))
		return;

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
		case BLEV_DEL:
		{
			Method1 = LEFT_METHOD;
			break;
		}
		/* Right click */
		case BLEV_PGDN:
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
 * LAST      : 04.08.94 12:05
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
 *             - The Init method should initialize the object's area,
 *              although the top-left corner will be set to the parent
 *              object's top-left corner.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Add_object(UNSHORT Parent_handle, struct Object_class *Class,
 UNBYTE *OID, SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	struct Object *Object;
	struct Object *Ptr;
	struct Object *Ptr2;
	UNSHORT Handle;
	UNSHORT Self;

	/* Add object to pool */
	Object = Add_object_to_pool(Class);
	if (!Object)
		return(0);

	/* Get new handle */
	Self = Object->Self;

	/* Insert parent handle */
	Object->Parent = Parent_handle;

	/* Is the current tree empty ? */
	Handle = Root_stack[Root_stack_index].First_object;
	if (!Handle)
	{
		/* Yes -> Any parent given ? */
		if (Parent_handle)
		{
			/* Yes -> Delete the object */
			Remove_object_from_pool(Self);

			/* Report error & exit */
			Control_error(ERROR_PARENT_NOT_FOUND);
			return(0);
		}
		else
		{
			/* No -> Bind (first object) */
			Root_stack[Root_stack_index].First_object = Self;
		}
	}
	else
	{
		/* No -> Any parent given ? */
		if (!Parent_handle)
		{
			/* No -> Find last object in layer 1 */
			while (Handle)
			{
				Ptr = Get_object_data(Handle);
				Handle = Ptr->Next;
			}

			/* Bind into layer 1 */
			Ptr->Next = Self;
		}
		else
		{
			/* Yes -> Search parent object */
			Ptr = Search_object(Handle, Parent_handle);

			/* Found it ? */
			if (!Ptr)
			{
				/* No -> Delete the object */
				Remove_object_from_pool(Self);

				/* Report error & exit */
				Control_error(ERROR_PARENT_NOT_FOUND);
				return(0);
			}

			/* Search parent container object */
			Ptr2 = Ptr;
/*			while (Ptr2->Flags & OBJECT_NO_CONTAINER)
			{
				t = Ptr2->Parent;

				if (!t)
					break;

				Ptr2 = Get_object_data(t);
			} */

			/* Set parent coordinates */
			Object->Rect.left = Ptr2->Rect.left;
			Object->Rect.top = Ptr2->Rect.top;

			if (Object->Flags & OBJECT_DISPLAY_BASE)
			{
				Object->X = 0;
				Object->Y = 0;
			}
			else
			{
/*				if (Ptr2->Flags & OBJECT_DISPLAY_BASE)
				{
					Object->X = 0 - Ptr2->Rect.left;
					Object->Y = 0 - Ptr2->Rect.top;
				}
				else */
				{
					Object->X = Ptr2->X;
					Object->Y = Ptr2->Y;
				}
			}

			/* Does the parent object already have children ? */
			if (Ptr->Child)
			{
				/* Yes -> Find last child */
				Handle = Ptr->Child;
				while (Handle)
				{
					Ptr = Get_object_data(Handle);
					Handle = Ptr->Next;
				}

				/* Bind */
				Ptr->Next = Self;
			}
			else
				/* No -> Bind (first child object) */
				Ptr->Child = Self;
		}
	}

	/* Set coordinates and dimensions */
	Object->Rect.left += X;
	Object->Rect.top += Y;
	Object->Rect.width = Width;
	Object->Rect.height = Height;

	if (!(Object->Flags & OBJECT_DISPLAY_BASE))
	{
		Object->X += X;
		Object->Y += Y;
	}

	/* Call object's Init method */
	Execute_method(Self, INIT_METHOD, (union Method_parms *) OID);

	return(Self);
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
			return(NULL);
		}

		/* Is this the one ? */
		if (Handle == Target)
		{
			/* Yes -> Exit */
			return(Ptr);
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
				return(T);
			}
		}

		/* Get brother */
		Handle = Ptr->Next;

		/* Has brother ? */
		if (!Handle)
		{
			/* No -> Not found */
			return(NULL);
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
	t = Root_stack[Root_stack_index].First_object;
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
	if (Nr_objects >= UI_OBJECTS_MAX)
	{
		/* Yes -> Error */
		Control_error(ERROR_TOO_MANY_OBJECTS);
		return(NULL);
	}

	/* Legal object size ? */
	if (Class->Size < sizeof(struct Object))
	{
		/* No -> Error */
		Control_error(ERROR_OBJECT_TOO_SMALL);
		return(NULL);
	}

	/* Will it fit in the pool ? */
	Start = Last_object_offset;
	End = Start + Class->Size;
	if (End > UI_OBJECT_POOL_SIZE)
	{
		/* No -> Error */
		Control_error(ERROR_OBJECT_POOL_IS_FULL);
		return(NULL);
	}

	/* Yes -> Count up */
	Object = (struct Object *) &Object_pool[Start];
	Last_object_offset = End;
	Nr_objects++;

	/* Find a free entry in the object list */
	for (Handle=1;Handle<=UI_OBJECTS_MAX;Handle++)
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
	Object->Self = Handle;
	Object->Flags = Class->Flags;
	Object->Class = Class;

	return(Object);
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
	for (i=0;i<UI_OBJECTS_MAX;i++)
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
		return(0);
	}

	/* Get method list */
	M = Object->Class->Method_list;
	if (!M)
		return(FALSE);

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

	return(FALSE);
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
 * FUNCTION  : Execute a method for each object.
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
		Handle = Root_stack[Root_stack_index].First_object;
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
 * LAST      : 05.04.95 11:25
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
 * NAME      : Get_object_data
 * FUNCTION  : Get an object's data.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.10.94 17:48
 * LAST      : 19.10.94 17:48
 * INPUTS    : UNSHORT Handle - Object handle.
 * RESULT    : struct Object * : Pointer to target object / NULL (error).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Object *
Get_object_data(UNSHORT Handle)
{
	struct Object *Output = NULL;

	/* Is the handle zero ? */
	if (!Handle)
	{
		/* Yes -> Error */
		Control_error(ERROR_OBJECT_HANDLE_ZERO);
	}
	else
	{
		/* No -> Illegal handle ? */
		if (Handle >= UI_OBJECTS_MAX)
		{
			/* Yes -> Error */
			Control_error(ERROR_ILLEGAL_OBJECT_HANDLE);
		}
		else
		{
			/* No -> Get pointer to object */
			Output = Object_ptrs[Handle - 1];

			/* Does the object exist ? */
			if (!Output)
			{
				/* No -> Error */
				Control_error(ERROR_OBJECT_DOESNT_EXIST);
			}
		}
	}

	return(Output);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wait_4_object
 * FUNCTION  : Wait until an object is deleted.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 16:58
 * LAST      : 04.08.94 16:58
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
		if ((Warn_when_deleted_counter != Counter) || (Quit_program))
			break;

		Handle_input();
		if ((Warn_when_deleted_counter != Counter) || (Quit_program))
			break;

		Switch_screens();
		if ((Warn_when_deleted_counter != Counter) || (Quit_program))
			break;
	}

	/* Get object data */
	Object = Get_object_data(Handle);

	/* Still there ? */
	if (Object)
	{
		/* Yes -> Clear flag */
		Object->Flags &= ~OBJECT_WARN_WHEN_DELETED;
	}
	else
	{
		/* No -> Ignore error */
		ERROR_ClearStack();
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
		return(FALSE);

	/* Get method list */
	M = Object->Class->Method_list;
	if (!M)
		return(FALSE);

	/* End of method list ? */
	while (M->ID)
	{
		/* No -> Found method ? */
		if (M->ID == Method)
		{
			/* Yes -> Success */
			return(TRUE);
		}
		/* No -> Next method */
		M++;
	}

	/* Not found */
	return(FALSE);
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
		return(FALSE);

	/* Exit if control object */
	if (Object->Flags & OBJECT_CONTROL)
		return(FALSE);

	/* Is the mouse in the rectangle ? */
	if ((Mouse_X < Object->Rect.left) || (Mouse_X >= Object->Rect.left +
		Object->Rect.width) || (Mouse_Y < Object->Rect.top) || (Mouse_Y >=
		Object->Rect.top + Object->Rect.height))
	{
		/* No */
		return(FALSE);
	}
	else
	{
		/* Yes */
		return(TRUE);
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
 * LAST      : 03.03.95 14:09
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
	return(Find_object_at(mX, mY, Root_stack[Root_stack_index].First_object));
}

UNSHORT
Find_object_at(SISHORT mX, SISHORT mY, UNSHORT Handle)
{
	struct Object *Object;

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
				return(Find_object_at(mX, mY, Object->Child));
			}

			/* No -> Found this object */
			return(Object->Self);
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
					return(Find_object_at(mX, mY, Object->Child));
				}

				/* No -> Found this object */
				return(Object->Self);
			}
		}

		/* Check the object's brother */
		Handle = Object->Next;
	}

	/* The handle was zero */
	return(0);
}

#if FALSE
/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_appropriate_mouse
 * FUNCTION  : Set a mouse pointer appropriate to the underlying object.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 14:16
 * LAST      : 03.03.95 14:16
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will call the touch method of the
 *              underlying object.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_appropriate_mouse(void)
{
	/* Touch object under mouse */
	Search_mouse_event_object(TOUCHED_METHOD,
	 Root_stack[Root_stack_index].First_object);
}
#endif

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
	if (Root_stack_index < ROOTS_MAX-1)
	{
		/* Yes -> Increase stack index */
		Root_stack_index++;

		/* Initialize new root */
		Root_stack[Root_stack_index].First_object = 0;
		Root_stack[Root_stack_index].OPM = OPM;

		/* Set current OPM */
		Current_OPM = OPM;

		/* Add to update list */
		Add_update_OPM(OPM);

		/* Force update of all OPMs */
//		Update_all_OPMs();
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
	UNSHORT i;

	/* Is the stack empty ? */
	if (Root_stack_index)
	{
		/* No -> Delete tree (if any) */
		if (Root_stack[Root_stack_index].First_object)
			Delete_object(Root_stack[Root_stack_index].First_object);

		/* Delete update OPMs */
		for (i=0;i<UPDATE_OPMS_MAX;i++)
		{
			OPM_update_list[Root_stack_index][i] = NULL;
		}

		/* Decrease stack index */
		Root_stack_index--;

		/* Set current OPM */
		Current_OPM = Root_stack[Root_stack_index].OPM;

		/* Force update of all OPMs */
//		Update_all_OPMs();
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
 * LAST      : 26.06.95 12:32
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
	UNSHORT i,j;

	/* Delete all trees on the stack */
	for (i=Root_stack_index;i>0;i--)
	{
		if (Root_stack[i].First_object)
		{
			/* Delete tree */
			Delete_object(Root_stack[i].First_object);

			/* Delete update OPMs */
			for (j=0;j<UPDATE_OPMS_MAX;j++)
			{
				OPM_update_list[i][j] = NULL;
			}
		}
	}

	Root_stack_index = 0;

	/* Initialize new root */
	Root_stack[Root_stack_index].First_object = 0;
	Root_stack[Root_stack_index].OPM = &Main_OPM;

	/* Set current OPM */
	Current_OPM = &Main_OPM;
//	Update_all_OPMs();
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

	/* Force update of all OPMs */
//	Update_all_OPMs();
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
 * NAME      : Push_module
 * FUNCTION  : Push a new module on the module stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 13:45
 * LAST      : 05.08.94 13:45
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
	struct Module *Old, *New;

	/* Is there room on the stack ? */
	if (Module_stack_index < MODULES_MAX-1)
	{
		/* Yes -> Get old module */
		Old = &Module_stack[Module_stack_index];

		/* Increase stack index */
		Module_stack_index++;

		/* Get new module */
		New = &Module_stack[Module_stack_index];

		/* Clear input buffer */
		Clear_input_buffer();

		/* Copy global/local flag */
		New->Flags = Module->Flags;

		/* If a local module has already been pushed, any following modules
		 will also be local */
		if (Pop_counter)
			New->Flags |= (LOCAL_MOD | WAS_GLOBAL_MOD);

		/* Copy type and screen type */
		New->Type = Module->Type;
		New->Screen_type = Module->Screen_type;

		/* The next entries are copied. If an entry is -1, the previous
		  module's entry is used. */
		{
			void **Old_ptr, **New_ptr, **Mod_ptr;
			UNSHORT i;

			Old_ptr = (void **) &(Old->MainLoop_function);
			New_ptr = (void **) &(New->MainLoop_function);
 			Mod_ptr = (void **) &(Module->MainLoop_function);

			/* Check entries */
			for (i=0;i<6;i++)
			{
				/* Transparent ? */
				if (Mod_ptr[i] == (void **) 0xFFFFFFFF)
				{
					/* Yes -> Use previous module's entry */
					New_ptr[i] = Old_ptr[i];
				}
				else
				{
					/* No */
					New_ptr[i] = Mod_ptr[i];
				}
			}
		}

		/* Clear all claims if the original module was global */
		if (New->Flags & WAS_GLOBAL_MOD)
			MEM_Clear_all_claims();

		/* Is this a local module ? */
		if (New->Flags & LOCAL_MOD)
		{
			UNSHORT Counter;

			/* Yes -> Increase local pop semaphore */
			Counter = Pop_counter;
			Counter++;
			Pop_counter = Counter;

			/* Initialize the module */
			Init_module(New);

			/* This is the local main loop. After each element follows a check
			 to see if the module was popped. */
			while ((Counter == Pop_counter) && (!Quit_program))
			{
				Main_loop();
	 			if ((Counter != Pop_counter) || (Quit_program))
	 				break;

				Handle_input();
	 			if ((Counter != Pop_counter) || (Quit_program))
	 				break;

				Switch_screens();
			}
		}
		else
 		{
			/* No -> Initialize the module */
			Init_module(New);
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
 * LAST      : 05.08.94 13:52
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
	struct Module *New;
	UNSHORT i,v;

	/* Pop all modules from the stack */
	v = Module_stack_index;
	for (i=0;i<v;i++)
	{
		Pop_module();
	}

	/* Reset stack */
	Module_stack_index = 0;

	/* Put the default module on the stack */
	New = &Module_stack[0];
	BASEMEM_CopyMem((UNBYTE *) New, (UNBYTE *) &Default_module,
	 sizeof(struct Module));

	/* Initialize the default module */
	Init_module(New);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_module
 * FUNCTION  : Initialize a new module.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 14:05
 * LAST      : 05.08.94 14:05
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
 * LAST      : 05.08.94 14:10
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
	if (Module->ModExit_function)
		(Module->ModExit_function)();

//	Set_appropriate_mouse();
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
 * LAST      : 05.09.94 13:37
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
	static UNLONG Start = 0;
	UNLONG T;

	/* Calculate elapsed time */
	if (Recording_flic)
		Update_duration = 3;
	else
	{
		T = SYSTEM_GetTicks();
		if (!Start)
			Start = T;
		if (Start > T)
			Update_duration = Start - T + 1;
		else
			Update_duration = T - Start + 1;
		Start = T;
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

	return(Screen_type);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_mouse
 * FUNCTION  : Push a new mouse pointer on the mouse pointer stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.94 17:36
 * LAST      : 09.08.94 17:36
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
	if (Mouse_stack_index < MOUSE_POINTERS_MAX-1)
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
 * LAST      : 09.08.94 17:39
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
 * NAME      : Reset_mouse_stack
 * FUNCTION  : Reset the mouse pointer stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.94 17:41
 * LAST      : 09.08.94 17:41
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_mouse_stack(void)
{
	/* Reset the stack */
	Mouse_stack_index = 0;

	/* Initialize new mouse pointer */
	Mouse_stack[0] = &(Mouse_pointers[DEFAULT_MPTR]);
	Init_mouse_pointer(Mouse_stack[0]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_mouse
 * FUNCTION  : Change the mouse pointer on the top of the mouse pointer stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.94 11:58
 * LAST      : 01.09.94 11:58
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
 * LAST      : 01.09.94 11:57
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
	MEM_HANDLE Handle;
	struct GTO *GTO;
	UNLONG Size;
	UNBYTE *Ptr;
	static UNCHAR Info_chunk[] = "INFO", Body_chunk[] = "BODY";
	static UNCHAR End_chunk[] = "ENDE";

	/* Exit if new mouse pointer is NULL */
	if (!New)
		return;

	/* Calculate size of sprite image */
	Size = New->Width * New->Height;

	/* Allocate memory for new mouse GTO */
	Handle = MEM_Allocate_memory(sizeof(struct GTO) + Size + 4);

	/* Prepare mouse GTO */
	Ptr = MEM_Claim_pointer(Handle);
	GTO = (struct GTO *) Ptr;

	Write_chunk_name((UNCHAR *) GTO->infochunk, Info_chunk);
	GTO->name[0] = 0;
	GTO->transcol = 0;
	GTO->plane = 0xFF;
	GTO->xoffset = 0 - New->X;
	GTO->yoffset = 0 - New->Y;
	GTO->width = New->Width;
	GTO->height = New->Height;
	Write_chunk_name((UNCHAR *) GTO->bodychunk, Body_chunk);

	/* Copy graphics */
	memcpy(Ptr + sizeof(struct GTO), New->Graphics, Size);

	/* Insert ENDE chunk */
	Write_chunk_name(Ptr + sizeof(struct GTO) + Size, End_chunk);

	/* Set new mouse pointer */
	SYSTEM_HideMousePtr();
	SYSTEM_ShowMousePtr(GTO);

	/* Free current mouse pointer (if any) */
	if (Mouse_GTO_handle)
	{
		MEM_Free_pointer(Mouse_GTO_handle);
		MEM_Free_memory(Mouse_GTO_handle);
	}

	/* This is now the current mouse pointer */
	Mouse_GTO_handle = Handle;
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
 *             UNCHAR Chunk_name[] - Chunk name.
 * RESULT    : UNBYTE * : Updated pointer.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Write_chunk_name(UNBYTE *Address, UNCHAR Chunk_name[])
{
	UNSHORT i;

	for (i=0;i<4;i++)
	{
		*Address++ = Chunk_name[i];
	}

	return(Address);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Mouse_on
 * FUNCTION  : Show the mouse pointer.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.94 13:36
 * LAST      : 14.09.94 13:36
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
			SYSTEM_ShowMousePtr((struct GTO *) MEM_Claim_pointer(Mouse_GTO_handle));
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
 * LAST      : 14.09.94 13:41
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
		SYSTEM_HideMousePtr();
		MEM_Free_pointer(Mouse_GTO_handle);
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
	if (MA_stack_index < MA_MAX-1)
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
	Default_MA.left = 0;
	Default_MA.top = 0;
	Default_MA.width = Screen_width;
	Default_MA.height = Screen_height;

	memcpy(&(MA_stack[0]), &Default_MA, sizeof(struct BBRECT));;
	Init_MA(&(MA_stack[0]));
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
 * LAST      : 16.03.95 15:02
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
 * LAST      : 05.08.94 14:53
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
	struct BBPOINT Mouse;

	/* Update objects */
	Execute_broadcast_method(0, UPDATE_METHOD, NULL);

	/* Get mouse coordinates */
	Mouse.x = Mouse_X;
	Mouse.y = Mouse_Y;

	/* Update HDOBs */
	Update_HDOBs();

	/* Draw HDOBs */
	Draw_HDOBs(&Mouse);

	/* Update all OPMs */
	Show_OPMs();

	/* Display new screen */
	DSA_DoubleBuffer();

	/* Remove HDOBs */
	Erase_HDOBs(&Mouse);

	Record_flic_frame();

	/* Update music */
	Update_music();
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
	for (i=0;i<UPDATE_OPMS_MAX;i++)
	{
		/* Is this the input OPM ? */
		if (OPM_update_list[Root_stack_index][i] == OPM)
		{
			return;
		}
	}

	/* No -> Find free slot */
	for (i=0;i<UPDATE_OPMS_MAX;i++)
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
	for (i=0;i<UPDATE_OPMS_MAX;i++)
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
	BOOLEAN No_main_OPM = TRUE;
	UNSHORT i;

	#if FALSE
	/* Are there any virtual OPMs derived from the main OPM or the main OPM
	 itself ? */
	for (i=0;i<UPDATE_OPMS_MAX;i++)
	{
		OPM = OPM_update_list[Root_stack_index][i];

		if (OPM && (OPM->status & OPMSTAT_INIT))
		{
			if (OPM->status & OPMSTAT_VIRTUAL)
			{
				if (OPM->virtualsrc == &Main_OPM)
				{
					No_main_OPM = FALSE;
					break;
				}
			}
			else
			{
				if (OPM == &Main_OPM)
				{
					No_main_OPM = FALSE;
					break;
				}
			}
		}
	}

	/* Should the main OPM be checked ? */
	if (No_main_OPM)
	{
		/* No */
		Show_changed_OPMs();
	}
	else
	#endif

	{
		/* Yes -> Has the main OPM changed ? */
		if (Main_OPM.status & OPMSTAT_CHANGED)
		{
			/* Yes -> Copy the main OPM to the screen */
			DSA_CopyMainOPMToScreen(&Screen, DSA_CMOPMTS_ALWAYS);

			/* Set flag for next frame */
			Main_OPM.status &= ~OPMSTAT_CHANGED;
			Main_OPM.status |= OPMSTAT_USERDEFINED;

			/* Clear the changed flags of virtual OPMs in the main OPM. */
			/* The main OPM will be updated the next frame anyway. */
			for (i=0;i<UPDATE_OPMS_MAX;i++)
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
			for (i=0;i<UPDATE_OPMS_MAX;i++)
			{
				OPM = OPM_update_list[Root_stack_index][i];

				if (OPM && (OPM->status & OPMSTAT_INIT)
				 && (!((OPM->status & OPMSTAT_VIRTUAL) && (OPM->virtualsrc == &Main_OPM))))
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
				for (i=0;i<UPDATE_OPMS_MAX;i++)
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
				Show_changed_OPMs();
			}
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
	for (i=0;i<UPDATE_OPMS_MAX;i++)
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

#if FALSE
/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_all_OPMs
 * FUNCTION  : Update all OPMs in the update list.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.10.94 18:04
 * LAST      : 01.10.94 18:04
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_all_OPMs(void)
{
	struct OPM *OPM;
	UNSHORT i;

	/* Update the main OPM */
	Main_OPM.status |= OPMSTAT_CHANGED;

	/* Update all non-virtual OPMs */
	for (i=0;i<UPDATE_OPMS_MAX;i++)
	{
		OPM = OPM_update_list[Root_stack_index][i];

		if (OPM && (OPM->status & OPMSTAT_INIT) &&
		 (!((OPM->status & OPMSTAT_VIRTUAL) &&
		 (OPM->virtualsrc == &Main_OPM))))
		{
			OPM->status |= OPMSTAT_CHANGED;
		}
	}
}
#endif

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Control_error
 * FUNCTION  : Report a control error.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 18:26
 * LAST      : 04.01.95 18:26
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : CONTROL.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Control_error(UNSHORT Error_code)
{
	struct Error_report Report;

	/* Build error report */
	Report.Code = Error_code;
	Report.Messages = &(_Control_errors[0]);

	/* Push error on the error stack */
	ERROR_PushError(Print_error, _Control_library_name,
	 sizeof(struct Error_report), (UNBYTE *) &Report);
}

