/************
 * NAME     : CONTROL.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 04.08.94 10:51
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>
#include <BBSYSTEM.H>

#include <CONTROL.H>
#include <HDOB.H>

/* global variables */

UNLONG Update_duration;

UNSHORT Mouse_X, Mouse_Y;
UNSHORT Old_mouse_X, Old_mouse_Y;
UNSHORT Button_state;

UNSHORT Highlighted_object, Focussed_object;

UNSHORT Nr_objects = 0;
UNLONG Last_object_offset = 0;
UNSHORT Warn_when_deleted_counter;

MEM_HANDLE Mouse_GTO_handle;

struct Mouse_pointer *Mouse_stack[ROOTS_MAX];
UNSHORT Mouse_stack_index = 0;

struct Root Root_stack[ROOTS_MAX];
UNSHORT Root_stack_index = 0;

struct Module Module_stack[MODULES_MAX];
UNSHORT Module_stack_index = 0;
UNSHORT Pop_counter = 0;

struct Object *Object_ptrs[OBJECTS_MAX];
UNBYTE Object_pool[OBJECT_POOL_SIZE];

struct Module Default_module =
{
	0, SCREEN_MOD,
	NULL,NULL,NULL,NULL,NULL,
	NULL
};

struct Mouse_pointer Default_mouse_pointer =
{
	-1,-1,
	16,16,
	NULL
};

void
Clear_all_events(void)
{
	struct BLEV_Event_struct Event;

	while (BLEV_GetEvent(&Event));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_input
 * FUNCTION  : Handle input.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 18:12
 * LAST      : 04.08.94 18:12
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
	struct BBPOINT Mouse;
	static SISHORT Key_or_mouse = -1;

	/* Do thang */
	SYSTEM_SystemTask();

	/* Read the current mouse coordinates */
	BLEV_GetMousePos(&Mouse);
	Mouse_X = Mouse.x;
	Mouse_Y = Mouse.y;

	/* Get current event */
	if (!BLEV_GetEvent(&Event))
	{
		Event.sl_eventtype = BLEV_NO_EVENT;
		Event.ul_pressed_keys = 0;
		Event.sl_key_code = 0;
	}

	/* Build button state */
	{
		UNSHORT Old, New;

		Old = Button_state;

		New = 0;
		if (Event.ul_pressed_keys & BLEV_MOUSELPRESSED)
			New |= 0x01;
		if (Event.ul_pressed_keys & BLEV_MOUSERPRESSED)
			New |= 0x10;

		Button_state = New | (((~Old & 0x11) & New) << 1)
		 | (((~New & 0x11) & Old) << 2);
	}

	/* Key or mouse ? */
	if (Key_or_mouse)
	{
		/* Mouse -> Was a key pressed ? */
		if (Event.sl_eventtype == BLEV_KEYDOWN)
		{
			/* Yes -> Go to key mode */
			Key_or_mouse = 0;
			// Mouse_off
			// Set_focus
			Highlighted_object = Root_stack[Root_stack_index].First_object;

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

	Clear_all_events();
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

	/* Select method */
	switch (Event->sl_eventtype)
	{
		case BLEV_NO_EVENT:
		{
			Method1 = TOUCHED_METHOD;
			break;
		}
		case BLEV_MOUSELDOWN:
		{
			Method1 = LEFT_METHOD;
			break;
		}
		case BLEV_MOUSELDBL:
		{
			Method1 = DLEFT_METHOD;
			break;
		}
		case BLEV_MOUSERDOWN:
		{
			Method1 = RIGHT_METHOD;
			break;
		}
		case BLEV_MOUSERDBL:
		{
			Method1 = DRIGHT_METHOD;
			break;
		}
	}

	/* Search the current tree */
	Search_mouse_event_object(Method1, Root_stack[Root_stack_index].First_object);
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
		Object = Object_ptrs[Handle-1];

		/* Is this a control object ? */
		if (Object->Flags & OBJECT_CONTROL)
		{
			/* Yes -> Does it have children ? */
			if (Object->Child)
				/* Yes -> Check them */
				if (Search_mouse_event_object(Method, Object->Child))
					return(TRUE);

			/* No -> Check this object */
			if (Search_mouse_method(Method, Object))
				return(TRUE);
		}
		else
		{
			/* No -> Are the mouse coordinates within it's area ? */
			if ((Mouse_X < Object->Rect.left) || (Mouse_X >= Object->Rect.left +
			 Object->Rect.width) || (Mouse_Y < Object->Rect.top) || (Mouse_Y >=
			 Object->Rect.top + Object->Rect.height))
			{
				/* No -> Is it not a container ? */
				if ((Object->Flags & OBJECT_NO_CONTAINER) && (Object->Child))
					/* Yes -> Check children (if any) */
					if (Search_mouse_event_object(Method, Object->Child))
						return(TRUE);
			}
			else
			{
				/* Yes -> Does it have children ? */
				if (Object->Child)
				{
					/* Yes -> Check them */
					if (Search_mouse_event_object(Method, Object->Child))
						return(TRUE);
				}
				else
					/* No -> Check this object */
					return (Search_mouse_method(Method, Object));
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
		/* Yes -> Does this object have this method ? */
		if (Has_method(Object->Self, Method))
		{
			/* Yes -> Execute */
			Execute_method(Object->Self, Method, 0L);
			return (TRUE);
		}

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
	UNSHORT Method1 = 0;
	UNSHORT Handle;

	/* Exit if no event */
	if (Event->sl_eventtype == BLEV_NO_EVENT)
		return;

	/* Was a diagnostic key pressed ? */
	if (Event->ul_pressed_keys & BLEV_ALT)
	{
		/* Yes -> Handle diagnostic keys */
//		Check_Kev_list(Event, &Diagnostics[0]);
		return;
	}

	/* No -> Select method */
	switch (Event->sl_key_code)
	{
		case BLEV_TAB:
		{
			Method1 = CYCLE_METHOD;
			break;
		}
		case BLEV_DEL:
		{
			Method1 = LEFT_METHOD;
			break;
		}
		case BLEV_PGDN:
		{
			Method1 = RIGHT_METHOD;
			break;
		}
		case BLEV_UP:
		{
			Method1 = MOVE_METHOD;
			P.Direction = 0;
			break;
		}
		case BLEV_DOWN:
		{
			Method1 = MOVE_METHOD;
			P.Direction = 2;
			break;
		}
		case BLEV_LEFT:
		{
			Method1 = MOVE_METHOD;
			P.Direction = 3;
			break;
		}
		case BLEV_RIGHT:
		{
			Method1 = MOVE_METHOD;
			P.Direction = 1;
			break;
		}
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
		if (Has_method(Handle,CUSTOMKEY_METHOD))
		{
			/* Yes -> Execute */
			P.Event = Event;
			if (Execute_method(Handle, CUSTOMKEY_METHOD, &P))
				/* Exit if succesful */
				return;
		}

		/* No -> Is there another method ? */
		if (Method1)
			/* Yes -> Does this object have this method ? */
			if (Has_method(Handle, Method1))
			{
				/* Yes -> Execute */
				Execute_method(Handle, Method1, &P);
				return;
			}

		/* Try parent object */
	   Handle = (Object_ptrs[Handle-1])->Parent;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_object
 * FUNCTION  : Create an instance of an object and bind it into the current
 *              object tree.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 12:05
 * LAST      : 04.08.94 12:05
 * INPUTS    : UNSHORT Parent_handle - Handle of parent object / 0 (layer 1).
 *             struct Object_class *Class - Pointer to object class.
 *             UNBYTE *OID - Pointer to OID (Object Initialization Data).
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
 UNBYTE *OID)
{
	struct Object *Object, *Ptr, *Ptr2;
	UNSHORT Handle, Self, t;

	/* Add object to pool */
	Object = Add_object_to_pool(Class);
	if (!Object)
		return(0);
	Self = Object->Self;

	/* Insert parent handle */
	Object->Parent = Parent_handle;

	/* Is the current tree empty ? */
	Handle = Root_stack[Root_stack_index].First_object;
	if (!Handle)
	{
		/* Yes -> Bind (first object) */
		Root_stack[Root_stack_index].First_object = Self;
	}
	else
	{
		/* No -> Add to layer 1 ? */
		if (!Parent_handle)
		{
			/* Yes -> Find last object in layer 1 */
			while (Handle)
			{
				Ptr = Object_ptrs[Handle-1];
				Handle = Ptr->Next;
			}
			/* Bind */
			Ptr->Next = Self;
		}
		else
		{
			/* No -> Search parent object */
			Ptr = Search_object(Handle, Parent_handle);

			/* Found it ? */
			if (!Ptr)
			{
				/* No -> Delete the object */
				Remove_object_from_pool(Self);
				/* Exit */
				return(0);
			}

			/* Search parent container object */
			Ptr2 = Ptr;
			while (Ptr2->Flags & OBJECT_NO_CONTAINER)
			{
				t = Ptr2->Parent;

				if (!t)
					break;

				Ptr2 = Object_ptrs[t-1];
			}

			/* Set parent coordinates */
			Object->Rect.left = Ptr2->Rect.left;
			Object->Rect.top = Ptr2->Rect.top;

			/* Does the parent object already have children ? */
			if (Ptr->Child)
			{
				/* Yes -> Find last child */
				Handle = Ptr->Child;
				while (Handle)
				{
					Ptr = Object_ptrs[Handle-1];
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

	/* Call object's Init method */
	Execute_method(Self, INIT_METHOD, (union Method_parms *) OID);

	return(Self);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_object
 * FUNCTION  : Search an object.
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

	while (TRUE)
	{
		/* Get current object */
		Ptr = Object_ptrs[Handle-1];

		/* Is this the one ? */
		if (Handle == Target)
			/* Yes -> Exit */
			return(Ptr);

		/* No -> Has children ? */
		if (Ptr->Child)
		{
			/* Yes -> Search children */
			T = Search_object(Ptr->Child, Target);
			/* Found anything ? */
			if (T)
				/* Yes -> Exit */
				return(T);
		}

		/* Get brother */
		Handle = Ptr->Next;

		/* Has brother ? */
		if (!Handle)
			/* No -> Not found */
			return(NULL);
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
	struct Object *Object, *Referrer, *Child;
	UNSHORT t, t2;

	/* Exit if handle is zero */
	if (!Handle)
		return;

	/* Get object data */
	Object = Object_ptrs[Handle-1];

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
			Referrer = Object_ptrs[Object->Parent - 1];
			t = Referrer->Child;
		}

		/* Found ? */
		while (t != Handle)
		{
			/* No -> Next object */
			Referrer = Object_ptrs[t-1];
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
			/* Yes -> Get brother */
			Child = Object_ptrs[t-1];
			t2 = Child->Next;

			/* Delete child */
			Delete_object(t);
			t = t2;
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
	}

	/* Call object's Exit method */
	Execute_method(Handle, EXIT_METHOD, NULL);

	/* Remove object from pool */
	Remove_object_from_pool(Handle);
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
	if (Nr_objects > OBJECTS_MAX)
		return(NULL);

	/* Will it fit in the pool ? */
	Start = Last_object_offset;
	End = Start + Class->Size;
	if (End > OBJECT_POOL_SIZE)
		return(NULL);

	/* Yes -> Count up */
	Object = (struct Object *) &Object_pool[Start];
	Last_object_offset = End;
	Nr_objects++;

	/* Find a free entry in the object list */
	for (Handle=1;Handle<=OBJECTS_MAX;Handle++)
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
	Object = Object_ptrs[Handle-1];

	/* Clear entry in object list */
	Object_ptrs[Handle-1] = NULL;

	/* Copy other objects down */
	Diff = (UNLONG) Object->Class->Size;
	Start = (UNBYTE *) Object;
	End = Start + Diff;
	BASEMEM_CopyMem(Start, End, &Object_pool[OBJECT_POOL_SIZE] - End);

	/* Adjust pointers to other objects */
	for (i=0;i<OBJECTS_MAX;i++)
	{
		/* Get pointer */
		Ptr = (UNBYTE *) Object_ptrs[i];

		/* Anything there ? */
		if (Ptr)
			/* Yes -> Above deleted object ? */
			if (Ptr > Start)
				/* Yes -> Adjust pointer */
				Object_ptrs[i] = (struct Object *) (Ptr - Diff);
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
	Object = Object_ptrs[Handle-1];

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
	Object = Object_ptrs[Handle-1];

	/* Get first child */
	h = Object->Child;

	/* Last child ? */
	while (h)
	{
		/* No -> Execute method */
		Execute_method(h, Method, P);

		/* Next brother */
		h = (Object_ptrs[h-1])->Next;
	}
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

	/* Set flag */
	Object = Object_ptrs[Handle-1];
	Object->Flags |= OBJECT_WARN_WHEN_DELETED;

	/* Load and increase counter */
	Warn_when_deleted_counter++;
	Counter = Warn_when_deleted_counter;

	/* This is the main loop. After each element follows a check to see if
	  the object was deleted. */
	while (TRUE)
	{
		Update_display();
		if (Warn_when_deleted_counter != Counter)
			break;

		Handle_input();
		if (Warn_when_deleted_counter != Counter)
			break;

		Switch_screens();
		if (Warn_when_deleted_counter != Counter)
			break;
	}

	/* Clear flag */
	Object = Object_ptrs[Handle-1];
	Object->Flags &= ~OBJECT_WARN_WHEN_DELETED;
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

	/* Exit if handle is zero */
	if (!Handle)
		return(FALSE);

	/* Get object data */
	Object = Object_ptrs[Handle-1];

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
 * NAME      : Push_root
 * FUNCTION  : Push a new root on the root stack.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.08.94 16:50
 * LAST      : 09.08.94 11:00
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
		Root_stack[Root_stack_index].Flags = 0;
		Root_stack[Root_stack_index].OPM = OPM;
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
 * LAST      : 04.08.94 16:50
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
	/* Is the stack empty ? */
	if (Root_stack_index)
	{
		/* No -> Delete tree (if any) */
		if (Root_stack[Root_stack_index].First_object)
			Delete_object(Root_stack[Root_stack_index].First_object);

		/* Decrease stack index */
		Root_stack_index--;
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
 * LAST      : 04.08.94 16:50
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
	UNSHORT i;

	/* Delete all trees on the stack */
	for (i=Root_stack_index;i>0;i--)
	{
		if (Root_stack[i].First_object)
			Delete_object(Root_stack[i].First_object);
	}

	Root_stack_index = 0;
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
	if (Module_stack_index < OBJECTS_MAX-1)
	{
		/* Yes -> Get old module */
		Old = &Module_stack[Module_stack_index];

		/* Increase stack index */
		Module_stack_index++;

		/* Get new module */
		New = &Module_stack[Module_stack_index];

		/* Clear all events */
		BLEV_ClearAllEvents();

		/* Copy global/local flag */
		New->Flags = Module->Flags;

		/* If a local module has already been pushed, any following modules
		 will also be local */
		if (Pop_counter)
			New->Flags |= (LOCAL_MOD | WAS_GLOBAL_MOD);

		/* Copy type */
		New->Type = Module->Type;

		/* The next entries are copied. If an entry is -1, the previous
		  module's entry is used. */
		{
			UNLONG *Old_ptr, *New_ptr, *Mod_ptr;
			UNSHORT i;

			Old_ptr = (UNLONG *) &(Old->MainLoop_function);
			New_ptr = (UNLONG *) &(New->MainLoop_function);
 			Mod_ptr = (UNLONG *) &(Module->MainLoop_function);

			/* Check entries */
			for (i=0;i<6;i++)
			{
				/* Transparent ? */
				if (Mod_ptr[i] == 0xFFFFFFFF)
					/* Yes -> Use previous module's entry */
					New_ptr[i] = Old_ptr[i];
				else
					/* No */
					New_ptr[i] = Mod_ptr[i];
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
			while (Counter == Pop_counter)
			{
				Update_display();
	 			if (Counter != Pop_counter)
	 				break;

				Handle_input();
	 			if (Counter != Pop_counter)
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

		/* Clear all events */
		BLEV_ClearAllEvents();

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
 * FUNCTION  : Call current module's MainLoop function.
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
Update_display(void)
{
	struct Module *Module;

	/* Get current module */
	Module = &Module_stack[Module_stack_index];

	/* Does this module have a DisUpd function ? */
	if (Module->MainLoop_function)
	{
		UNLONG Start;

		/* Yes -> Mark the time */
		Start = SYSTEM_GetTicks();

		/* Call function */
		(Module->MainLoop_function)();

		/* Store elapsed time */
		Update_duration = SYSTEM_GetTicks() - Start;
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
	Mouse_stack[0] = &Default_mouse_pointer;
	Init_mouse_pointer(Mouse_stack[0]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_mouse_pointer
 * FUNCTION  : Initialize a new mouse pointer.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.94 17:42
 * LAST      : 09.08.94 17:42
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

	/* Calculate size of sprite image */
	Size = New->Width * New->Height;

	/* Allocate memory for new mouse GTO */
	Handle = MEM_Allocate_memory(sizeof(struct GTO) + Size + 4);

	/* Prepare mouse GTO */
	Ptr = MEM_Claim_pointer(Handle);

	GTO = (struct GTO *) Ptr;
	*((UNLONG *) &(GTO->infochunk)) = 'INFO';
	GTO->name[0] = 0;
	GTO->plane = 0;
	GTO->xoffset = New->X;
	GTO->yoffset = New->Y;
	GTO->width = New->Width;
	GTO->height = New->Height;
	*((UNLONG *) &(GTO->bodychunk)) = 'BODY';

	/* Copy graphics */
	BASEMEM_CopyMem(New->Graphics, Ptr + sizeof(struct GTO), Size);

	/* Insert ENDE chunk */
	*((UNLONG *) (Ptr + sizeof(struct GTO) + Size)) = 'ENDE';

	/* Set new mouse pointer */
	SYSTEM_ShowMouseptr((struct GTO *) Ptr);

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
 * NAME      : Switch_screens
 * FUNCTION  : Display the currently invisible screen.
 * FILE      :
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
	struct Module *Module;

	/* Get mouse coordinates */
	BLEV_GetMousePos(&Mouse);

	/* Draw HDOBs */
	Draw_HDOBs(&Mouse);

	/* Get current module */
	Module = &Module_stack[Module_stack_index];

	/* Call Switch function (if any) */
	if (Module->Switch_function)
		(Module->Switch_function)();

	/* Display new screen */
	DSA_DoubleBuffer();

	/* Remove HDOBs */
	Erase_HDOBs(&Mouse);
};


#ifdef GURGLE
void
The_usual_switch_function(void)
{
	struct OPM *OPM;
	UNSHORT i;

	/* Has the main OPM changed ? */
	if (Main_OPM.status & OPMSTAT_CHANGED)
	{
		/* Yes -> Copy the main OPM to the screen */
		DSA_CopyMainOPMToScreen(&Screen, DSA_CMOPMTS_CHANGED);

		/* Set flag for next frame */
		Main_OPM.status &= ~OPMSTAT_CHANGED;
		Main_OPM.status |= OPMSTAT_USERDEFINED;

		/* Clear the changed flags of the virtual OPMs */
		/* The main OPM will be updated the next frame anyway. */
		for (i=0;i<Nr_of_virtual_OPMs;i++)
			Virtual_OPM_list[i]->status &= ~(OPMSTAT_CHANGED | OPMSTAT_USERDEFINED);
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

			/* Check virtual OPMs */
			for (i=0;i<Nr_of_virtual_OPMs;i++)
			{
				OPM = Virtual_OPM_list[i];
				/* Has this virtual OPM changed ? */
				if (OPM->status & OPMSTAT_CHANGED)
					{
						/* Yes -> Set flag for next frame */
						OPM->status &= ~OPMSTAT_CHANGED;
						OPM->status |= OPMSTAT_USERDEFINED;
					}
				else
					/* No -> Flag is useless now */
					OPM->status &= ~OPMSTAT_USERDEFINED;
			}
		}
		else
		{
			/* No -> Copy all the virtual OPMs to the screen (if changed) */
			for (i=0;i<Nr_of_virtual_OPMs;i++)
			{
				OPM = Virtual_OPM_list[i];
				/* Has this virtual OPM changed ? */
				if (OPM->status & OPMSTAT_CHANGED)
				{
					/* Yes -> Copy it */
					DSA_CopyOPMToScreen(&Screen, OPM, 0, 0, DSA_CMOPMTS_CHANGED);

					/* Set flag for next frame */
					OPM->status &= ~OPMSTAT_CHANGED;
					OPM->status |= OPMSTAT_USERDEFINED;
				}
				else
				{
				/* Was this virtual OPM updated in the previous frame ? */
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
#endif

