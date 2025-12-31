
	{ DRAW_METHOD, Draw_children },

/* Dialogue portrait object methods */
UNLONG Init_Dialogue_portrait_object(struct Object *Object,
 union Method_parms *P);
UNLONG Draw_Dialogue_portrait_object(struct Object *Object,
 union Method_parms *P);
UNLONG Highlight_Dialogue_portrait_object(struct Object *Object,
 union Method_parms *P);
UNLONG Help_Dialogue_portrait_object(struct Object *Object,
 union Method_parms *P);

/* Dialogue portrait method list */
static struct Method Dialogue_portrait_methods[] = {
	{ INIT_METHOD, Init_Dialogue_portrait_object },
	{ DRAW_METHOD, Draw_Dialogue_portrait_object },
	{ HIGHLIGHT_METHOD, Highlight_Dialogue_portrait_object },
	{ HELP_METHOD, Help_Dialogue_portrait_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

/* Dialogue portrait class description */
static struct Object_class Dialogue_portrait_Class = {
	0, sizeof(struct Dialogue_portrait_object),
	&Dialogue_portrait_methods[0]
};

	/* Add portrait objects */
	{
		struct Dialogue_portrait_OID OID;

		/* Build left portrait OID */
		OID.Char_handle = Active_char_handle;
		OID.Portrait_gfx_handle =
		 Small_portrait_handles[PARTY_DATA.Active_member - 1];
		OID.Mirror_flag = FALSE;

		/* Add object */
		Add_object(Dialogue_object, &Dialogue_portrait_Class, (UNBYTE *) &OID,
		 10, 5, 34, 37);

		/* Build right portrait OID */
		OID.Char_handle = Dialogue_char_handle;
		OID.Portrait_gfx_handle = Dialogue_small_portrait_handle;
		OID.Mirror_flag = TRUE;

		/* Add object */
		Add_object(Dialogue_object, &Dialogue_portrait_Class, (UNBYTE *) &OID,
		 360 - 36 - 8, 5, 34, 37);
	}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Dialogue_portrait_object
 * FUNCTION  : Initialize method of Dialogue portrait object.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 12:58
 * LAST      : 23.06.95 12:58
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Dialogue_portrait_object(struct Object *Object, union Method_parms *P)
{
	struct Dialogue_portrait_object *Dialogue_portrait;
	struct Dialogue_portrait_OID *OID;

	Dialogue_portrait = (struct Dialogue_portrait_object *) Object;
	OID = (struct Dialogue_portrait_OID *) P;

	/* Copy data from OID */
	Dialogue_portrait->Char_handle = OID->Char_handle;
	Dialogue_portrait->Portrait_gfx_handle = OID->Portrait_gfx_handle;
	Dialogue_portrait->Mirror_flag = OID->Mirror_flag;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Dialogue_portrait_object
 * FUNCTION  : Draw method of Dialogue portrait object.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 13:03
 * LAST      : 23.06.95 13:03
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Dialogue_portrait_object(struct Object *Object, union Method_parms *P)
{
	struct Dialogue_portrait_object *Dialogue_portrait;
	UNBYTE *Ptr;

	Dialogue_portrait = (struct Dialogue_portrait_object *) Object;

	/* Erase portrait area */
	Draw_window_inside(Current_OPM, Object->X - 1, Object->Y - 1, 36, 39);

	/* Draw box around portrait */
	Draw_high_border(Current_OPM, Object->X - 1, Object->Y - 1, 36, 39);
	Put_recoloured_box(Current_OPM, Object->X, Object->Y, 34, 37,
	 &(Recolour_tables[1][0]));

	/* Display portrait */
	Ptr = MEM_Claim_pointer(Dialogue_portrait->Portrait_gfx_handle);

	/* Mirrored ? */
	if (Dialogue_portrait->Mirror_flag)
	{
		/* Yes */
		Put_masked_X_mirrored_block(Current_OPM, Object->X, Object->Y, 34, 37,
		 Ptr);
	}
	else
	{
		/* No */
		Put_masked_block(Current_OPM, Object->X, Object->Y, 34, 37, Ptr);
	}

	MEM_Free_pointer(Dialogue_portrait->Portrait_gfx_handle);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Dialogue_portrait_object
 * FUNCTION  : Highlight method of Dialogue portrait object.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 13:03
 * LAST      : 23.06.95 13:03
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Dialogue_portrait_object(struct Object *Object, union Method_parms *P)
{
	struct Dialogue_portrait_object *Dialogue_portrait;
	UNBYTE *Ptr;

	Dialogue_portrait = (struct Dialogue_portrait_object *) Object;

	/* Erase portrait area */
	Draw_window_inside(Current_OPM, Object->X - 1, Object->Y - 1, 36, 39);

	/* Draw box around portrait */
	Draw_high_border(Current_OPM, Object->X - 1, Object->Y - 1, 36, 39);
	Put_recoloured_box(Current_OPM, Object->X, Object->Y, 34, 37,
	 &(Recolour_tables[1][0]));

	/* Display portrait */
	Ptr = MEM_Claim_pointer(Dialogue_portrait->Portrait_gfx_handle);

	/* Mirrored ? */
	if (Dialogue_portrait->Mirror_flag)
  	{
		/* Yes */
		Put_masked_X_mirrored_block(Current_OPM, Object->X, Object->Y, 34, 37,
		 Ptr);
		Put_recoloured_X_mirrored_block(Current_OPM, Object->X, Object->Y, 34,
		 37, Ptr, &(Recolour_tables[5][0]));
	}
	else
	{
		/* No */
		Put_masked_block(Current_OPM, Object->X, Object->Y, 34, 37, Ptr);
		Put_recoloured_block(Current_OPM, Object->X, Object->Y, 34, 37, Ptr,
		 &(Recolour_tables[5][0]));
	}

	MEM_Free_pointer(Dialogue_portrait->Portrait_gfx_handle);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Dialogue_portrait_object
 * FUNCTION  : Help method of Dialogue_portrait object.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 13:35
 * LAST      : 23.06.95 13:35
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Dialogue_portrait_object(struct Object *Object, union Method_parms *P)
{
	struct Dialogue_portrait_object *Dialogue_portrait;
	UNCHAR Name[CHAR_NAME_LENGTH + 1];

	Dialogue_portrait = (struct Dialogue_portrait_object *) Object;

	/* Get character name */
	Get_char_name(Dialogue_portrait->Char_handle, Name);

	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) Name);

	return 0;
}

