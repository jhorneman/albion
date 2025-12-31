
struct Proxy_OID {
	UNSHORT Stolen_object_handle;
};

struct Proxy_object {
	UNSHORT Old_parent;
	UNSHORT Old_next;
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Steal_object
 * FUNCTION  : "Steal" an object from an old tree and put it into the current
 *              tree.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 16:39
 * LAST      : 23.06.95 16:39
 * INPUTS    : UNSHORT Parent_handle - Handle of parent object / 0 (layer 1).
 *             UNSHORT Handle - Handle of object that should be stolen.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Steal_object(UNSHORT Parent_handle, UNSHORT Handle)
{
	struct Object *Stolen_object;
	struct Object *Parent_object;
	struct Object *Proxy_object;
	struct Proxy_OID OID;
	UNSHORT Proxy_object_handle;

	/* Get stolen object data */
	Stolen_object = Get_object_data(Handle);
	if (!Stolen_object)
		return;

	/* Build OID */
	OID.Stolen_object_handle = Handle;

	/* Add proxy object */
	Proxy_object_handle = Add_object(Parent_handle, &Proxy_Class,
	 (UNBYTE *) , Stolen_object->Rect.left, Stolen_object->Rect.top,
	 Stolen_object->Rect.width, Stolen_object->Rect.height);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Proxy_object
 * FUNCTION  : Init method of Proxy object.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 16:45
 * LAST      : 23.06.95 16:45
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Proxy_object(struct Object *Object, union Method_parms *P)
{
	struct Proxy_object *Proxy;
	struct Proxy_OID *OID;
	struct Object *Stolen_object;

	Proxy = (struct Proxy_object *) Object;
	OID = (struct Proxy_OID *) P;

	/* Get stolen object data */
	Stolen_object = Get_object_data(OID->Stolen_object_handle);
	if (!Stolen_object)
		return 0;

	/* Store old links of stolen object in proxy object data */
	Proxy->Old_parent = Stolen_object->Parent;
	Proxy->Old_next = Stolen_object->Next;

	/* Link stolen object to proxy */
	Object->Child = Stolen_object_handle;

	/* Link proxy to stolen object */
	Stolen_object->Parent = Object->Self;
	Stolen_object->Next = 0;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Proxy_object
 * FUNCTION  : Exit method of Proxy object.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 16:51
 * LAST      : 23.06.95 16:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Exit_Proxy_object(struct Object *Object, union Method_parms *P)
{
	struct Proxy_object *Proxy;
	struct Object *Stolen_object;

	Proxy = (struct Proxy_object *) Object;

	/* Get stolen object data */
	Stolen_object = Get_object_data(Proxy->Child);
	if (!Stolen_object)
		return 0;

	/* Restore old links of stolen object */
	Stolen_object->Parent = Proxy->Old_parent;
	Stolen_object->Next = Proxy->Old_next;

	return 0;
}


