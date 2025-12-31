
void Draw_Number_display_object(struct Object *Object, union Method_parms *P);
void Update_Number_display_object(struct Object *Object, union Method_parms *P);
void Highlight_Number_display_object(struct Object *Object, union Method_parms *P);

static struct Method Number_display_methods[] = {
	{ DRAW_METHOD, Draw_Number_display_object },
	{ UPDATE_METHOD, Update_Number_display_object },
	{ HIGHLIGHT_METHOD, Highlight_Number_display_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

static struct Object_class Number_display_Class = {
	0, sizeof(struct Value_display_object),
	&Number_display_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Number_display_object
 * FUNCTION  : Draw method of Number display object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.01.95 18:39
 * LAST      : 12.01.95 18:39
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_Number_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Number_display;
	UNCHAR String[20];

	Number_display = (struct Value_display_object *) Object;

	/* Erase object */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_deep_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Print weight */
	sprintf(String, "%ld", Number_display->Value);
	Print_centered_string(Current_OPM, Object->X + 1, Object->Y + 1,
	 Object->Rect.width - 2, String);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Number_display_object
 * FUNCTION  : Update method of Number display object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.01.95 18:39
 * LAST      : 12.01.95 18:40
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_Number_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Number_display;
	struct InputNr_object *InputNr;

	Number_display = (struct Value_display_object *) Object;
	InputNr = (struct InputNr_object *) Get_object_data(Object->Parent);

	/* Has the value changed ? */
	if (*(InputNr->Value_ptr) != Number_display->Value)
	{
		/* Yes -> Adjust value */
		Number_display->Value = *(InputNr->Value_ptr);

		/* Redraw */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Number_display_object
 * FUNCTION  : Highlight method of Number display object.
 * FILE      : INPUT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.01.95 18:40
 * LAST      : 12.01.95 18:40
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Highlight_Number_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Number_display;
	UNCHAR String[20];

	Number_display = (struct Value_display_object *) Object;

	/* Erase object */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_deep_border(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Put_recoloured_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height, &(Recolour_tables[6][0]));

	/* Print weight */
	sprintf(String, "%ld", Number_display->Value);
	Print_centered_string(Current_OPM, Object->X + 1, Object->Y + 1,
	 Object->Rect.width - 2, String);
}

