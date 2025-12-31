
#define ADD_DELTA			(8)
#define FIRST_ADD			(256 - (20 * ADD_DELTA))
#define MAX_DELTA_ADD	(16384)


/* Number input OID */
struct InputNr_OID {
	SILONG *Value_ptr;
	SILONG Minimum, Maximum;
	UNCHAR *Text;
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
	SISHORT Delta_add;
};


extern struct Object_class InputNr_Class;


SILONG Input_number(SILONG Value, SILONG Minimum, SILONG Maximum,	UNCHAR *Text);

void InputNr_ModInit(void);
void InputNr_ModExit(void);

void Init_InputNr_object(struct Object *Object, union Method_parms *P);
void Exit_InputNr_object(struct Object *Object, union Method_parms *P);
void Draw_InputNr_object(struct Object *Object, union Method_parms *P);
void Touch_InputNr_object(struct Object *Object, union Method_parms *P);

void InputNr_Maximum(struct Button_object *Button);
void InputNr_More(struct Button_object *Button);
void InputNr_Less(struct Button_object *Button);
void InputNr_Minimum(struct Button_object *Button);
void InputNr_Exit(struct Button_object *Button);

void Draw_Number_display_object(struct Object *Object, union Method_parms *P);
void Update_Number_display_object(struct Object *Object, union Method_parms *P);
void Highlight_Number_display_object(struct Object *Object, union Method_parms *P);




struct Module InputNr_Mod = {
	0, WINDOW_MOD,
	NULL,
	InputNr_ModInit, InputNr_ModExit,
	NULL, NULL,
	NULL
};

struct Method InputNr_methods[] = {
	{ INIT_METHOD, Init_InputNr_object },
	{ EXIT_METHOD, Exit_InputNr_object },
	{ DRAW_METHOD, Draw_InputNr_object },
	{ TOUCHED_METHOD, Touch_InputNr_object },
	{ RESTORE_METHOD, Restore_window },
	{ 0, NULL}
};

struct Object_class InputNr_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct InputNr_object),
	&InputNr_methods[0]
};

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

static union Button_data InputNr_button_data[5];
static struct Button_OID InputNr_button_OID[5] = {
	{
	  	TEXT_BUTTON_TYPE,
		0,
		65,
		&(InputNr_button_data[0]),
		InputNr_Maximum
	},
	{
	  	TEXT_BUTTON_TYPE | CONTINUOUS_BUTTON,
		0,
		66,
		&(InputNr_button_data[1]),
		InputNr_More
	},
	{
	  	TEXT_BUTTON_TYPE | CONTINUOUS_BUTTON,
		0,
		67,
		&(InputNr_button_data[2]),
		InputNr_Less
	},
	{
	  	TEXT_BUTTON_TYPE,
		0,
		68,
		&(InputNr_button_data[3]),
		InputNr_Minimum
	},
	{
	  	TEXT_BUTTON_TYPE,
		0,
		69,
		&(InputNr_button_data[4]),
		InputNr_Exit
	}
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
Input_number(SILONG Value, SILONG Minimum, SILONG Maximum,	UNCHAR *Text)
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

	/* Do it */
	Push_module(&InputNr_Mod);
	Obj = Add_object(0, &InputNr_Class, (UNBYTE *) &OID, 20, 20, 150, 150);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);

	return(It);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : InputNr_ModInit
 * FUNCTION  : Initialize number input window module.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 17:38
 * LAST      : 11.01.95 17:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
InputNr_ModInit(void)
{
	Push_textstyle(&Default_text_style);
	Push_root(&Main_OPM);
	Push_mouse(&(Mouse_pointers[DEFAULT_MPTR]));

	Add_update_OPM(&Status_area_OPM);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : InputNr_ModExit
 * FUNCTION  : Exit number input window module.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 17:38
 * LAST      : 11.01.95 17:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
InputNr_ModExit(void)
{
	Remove_update_OPM(&Status_area_OPM);

	Pop_mouse();
	Pop_root();
	Pop_textstyle();
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
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_InputNr_object(struct Object *Object, union Method_parms *P)
{
	struct InputNr_object *InputNr;
	struct InputNr_OID *OID;

	InputNr = (struct InputNr_object *) Object;
	OID = (struct InputNr_OID *) P;

	/* Copy data from OID */
	InputNr->Value_ptr = OID->Value_ptr;
	InputNr->Minimum = OID->Minimum;
 	InputNr->Maximum = OID->Maximum;
 	InputNr->Text = OID->Text;

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Set root OPM */
	Set_root_OPM(&(InputNr->Window_OPM));

	/* Install mouse area */
	Push_MA(&(Object->Rect));

	/* Initialize button data */
	InputNr_button_data[0].Text_button_data.Text = System_text_ptrs[60];
 	InputNr_button_data[1].Text_button_data.Text = System_text_ptrs[61];
	InputNr_button_data[2].Text_button_data.Text = System_text_ptrs[62];
	InputNr_button_data[3].Text_button_data.Text = System_text_ptrs[63];
	InputNr_button_data[4].Text_button_data.Text = System_text_ptrs[64];

	/* Add buttons to window */
	Add_object(Object->Self, &Button_Class, (UNBYTE *) &InputNr_button_OID[0],
	 8, 8, 50, 11);
	Add_object(Object->Self, &Button_Class, (UNBYTE *) &InputNr_button_OID[1],
	 8, 21, 50, 11);
	Add_object(Object->Self, &Button_Class, (UNBYTE *) &InputNr_button_OID[2],
	 8, 44, 50, 11);
	Add_object(Object->Self, &Button_Class, (UNBYTE *) &InputNr_button_OID[3],
	 8, 67, 50, 11);
 	Add_object(Object->Self, &Button_Class, (UNBYTE *) &InputNr_button_OID[4],
	 8, 80, 50, 11);

	/* Add number display to window */
	Add_object(Object->Self, &Number_display_Class, NULL, 60, 34, 40, 11);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_InputNr_object
 * FUNCTION  : Exit method of Number input window object.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 17:40
 * LAST      : 11.01.95 17:40
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_InputNr_object(struct Object *Object, union Method_parms *P)
{
	Destroy_window((struct Window_object *) Object);
	Pop_MA();

	/* Remove help message */
	Execute_method(Help_line_object, SET_METHOD, NULL);

	/* Exit module (MUST be here !!!) */
	Pop_module();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_InputNr_object
 * FUNCTION  : Draw method of Number input window object.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 17:40
 * LAST      : 11.01.95 17:40
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_InputNr_object(struct Object *Object, union Method_parms *P)
{
	struct InputNr_object *InputNr;
	struct OPM *OPM;
	UNSHORT W, H;

	InputNr = (struct InputNr_object *) Object;
	OPM = &(InputNr->Window_OPM);

	W = Object->Rect.width;
	H = Object->Rect.height;

	/* Draw window's shadow */
	Put_recoloured_box(OPM, 10, 10, W - 10, H - 10, &(Recolour_tables[1][0]));

	/* Draw window */
	Draw_window_inside(OPM, 7, 7, W - 14, H - 14);
	Draw_window_border(OPM, 0, 0, W, H);

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Touch_InputNr_object
 * FUNCTION  : Touch method of Number input window object.
 * FILE      : INPUTNR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.01.95 10:27
 * LAST      : 13.01.95 10:27
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Touch_InputNr_object(struct Object *Object, union Method_parms *P)
{
	struct InputNr_object *InputNr;

	InputNr = (struct InputNr_object *) Object;

	/* Clear the delta adder */
	InputNr->Delta_add = 0;

	/* Dehighlight */
	Dehighlight(Object, P);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : InputNr_Maximum
 * FUNCTION  : Set value to maximum (button).
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
InputNr_Maximum(struct Button_object *Button)
{
	struct InputNr_object *InputNr;

	InputNr = (struct InputNr_object *) Get_object_data(Button->Object.Parent);

	/* Set value to minimum */
	*(InputNr->Value_ptr) = InputNr->Maximum;

	/* Clear the delta adder */
	InputNr->Delta_add = 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : InputNr_More
 * FUNCTION  : Increase value (button).
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
InputNr_More(struct Button_object *Button)
{
	struct InputNr_object *InputNr;

	InputNr = (struct InputNr_object *) Get_object_data(Button->Object.Parent);

	/* Can the value be increased ? */
	if (*(InputNr->Value_ptr) < InputNr->Maximum)
	{
		/* Yes -> Is this the first time ? */
		if (InputNr->Delta_add <= 0)
		{
			/* Yes -> Increase the value by a whole step */
			*(InputNr->Value_ptr) += 1;

			/* Initialize the delta adder */
			InputNr->Delta_add = FIRST_ADD;
		}
		else
		{
			/* No -> Increase the value by the delta adder */
			*(InputNr->Value_ptr) += InputNr->Delta_add / 256;

			/* Too high ? */
			if (*(InputNr->Value_ptr) > InputNr->Maximum)
			{
				/* Yes -> Set to maximum */
				*(InputNr->Value_ptr) = InputNr->Maximum;

				/* Clear the delta adder */
				InputNr->Delta_add = 0;
			}
			else
			{
				/* No -> Faster baby */
				InputNr->Delta_add += ADD_DELTA;

				/* But not TOO fast */
				if (InputNr->Delta_add > MAX_DELTA_ADD)
				{
					InputNr->Delta_add = MAX_DELTA_ADD;
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : InputNr_Less
 * FUNCTION  : Decrease value (button).
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
InputNr_Less(struct Button_object *Button)
{
	struct InputNr_object *InputNr;

	InputNr = (struct InputNr_object *) Get_object_data(Button->Object.Parent);

	/* Can the value be decreased ? */
	if (*(InputNr->Value_ptr) > InputNr->Minimum)
	{
		/* Yes -> Is this the first time ? */
		if (InputNr->Delta_add >= 0)
		{
			/* Yes -> Decrease the value by a whole step */
			*(InputNr->Value_ptr) -= 1;

			/* Initialize the delta adder */
			InputNr->Delta_add = -FIRST_ADD;
		}
		else
		{
			/* No -> Decrease the value by the delta adder */
			*(InputNr->Value_ptr) += InputNr->Delta_add / 256;

			/* Too low ? */
			if (*(InputNr->Value_ptr) < InputNr->Minimum)
			{
				/* Yes -> Set to minimum */
				*(InputNr->Value_ptr) = InputNr->Minimum;

				/* Clear the delta adder */
				InputNr->Delta_add = 0;
			}
			else
			{
				/* No -> Faster baby */
				InputNr->Delta_add -= ADD_DELTA;

				/* But not TOO fast */
				if (InputNr->Delta_add < -MAX_DELTA_ADD)
				{
					InputNr->Delta_add = -MAX_DELTA_ADD;
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : InputNr_Minimum
 * FUNCTION  : Set value to minimum (button).
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
InputNr_Minimum(struct Button_object *Button)
{
	struct InputNr_object *InputNr;

	InputNr = (struct InputNr_object *) Get_object_data(Button->Object.Parent);

	/* Set value to minimum */
	*(InputNr->Value_ptr) = InputNr->Minimum;

	/* Clear the delta adder */
	InputNr->Delta_add = 0;
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

