
/* Tactical selection data */
UNSHORT Tactical_selected_X, Tactical_selected_Y;
struct Combat_participant *Tactical_selected_part;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_tactical_square
 * FUNCTION  : Select the tactical square currently under the mouse pointer.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 16:45
 * LAST      : 13.03.95 16:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_tactical_square(void)
{
	struct Object *Object;
	UNSHORT mX, mY;

	/* Get current mouse coordinates */
	mX = Mouse_X;
	mY = Mouse_Y;

	/* Get tactical window data */
	Object = (struct Object *) Get_object_data(Tactical_window_object);

	/* Inside tactical window ? */
	if (Is_over_object(Tactical_window_object))
	{
		/* Yes -> Calculate tactical square coordinates */
		Tactical_selected_X = (mX - Object->Rect.left) / TACTICAL_SQUARE_WIDTH;
		Tactical_selected_Y = (mY - Object->Rect.top) / TACTICAL_SQUARE_HEIGHT;

		/* Get tactical selected participant */
		Tactical_selected_part =
		 Combat_matrix[Tactical_selected_Y][Tactical_selected_X].Part;
	}
	else
	{
		/* No -> Set tactical square coordinates to illegal values */
		Tactical_selected_X = 0xFFFF;
		Tactical_selected_Y = 0xFFFF;

		/* Set tactical selected participant to NULL */
		Tactical_selected_part = NULL;
	}
}

