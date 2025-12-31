
/* Mouse EVents */
 struct Mev {
	UNSHORT Button_mask, Button_value;
	void (*Function)(void);
};

/* Key EVents */
	struct Kev {
	UNSHORT Qualifier_mask, Qualifier_value;
	UNSHORT Key_code;
	void (*Function)(UNSHORT);
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_Mev_list
 * FUNCTION  : Check a list of mouse events.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 12:15
 * LAST      : 05.08.94 12:15
 * INPUTS    : struct Mev *Mev_list - Pointer to list of mouse events.
 * RESULT    : BOOLEAN : Success or failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_Mev_list(struct Mev *Mev_list)
{
	/* End of the list ? */
	while (Mev_list->Function)
	{
		/* No -> Does this Mev match the input event ? */
		if ((Button_state & Mev_list->Button_mask) == Mev_list->Button_value)
		{
			/* Yes -> Execute the function and exit */
			(Mev_list->Function)();
			return (TRUE);
		}

		/* No -> Next Mev */
		Mev_list++;
	}

	/* No luck */
	return(FALSE);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_Kev_list
 * FUNCTION  : Check a list of key events.
 * FILE      : CONTROL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 12:05
 * LAST      : 05.08.94 12:05
 * INPUTS    : struct BLEV_Event_struct *Event - Input event.
 *             struct Kev *Kev_list - Pointer to list of key events.
 * RESULT    : BOOLEAN : Success or failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_Kev_list(struct BLEV_Event_struct *Event, struct Kev *Kev_list)
{
	/* End of the list ? */
	while (Kev_list->Key_code)
	{
		/* No -> Does this Kev match the input event ? */
		if (((Event->ul_pressed_keys & Kev_list->Qualifier_mask)
		 == Kev_list->Qualifier_value) && (Event->sl_key_code
		 == Kev_list->Key_code))
		{
			/* Yes -> Execute the function and exit */
			(Kev_list->Function)((UNSHORT) Event->sl_key_code);
			return (TRUE);
		}

		/* No -> Next Kev */
		Kev_list++;
	}

	/* No luck */
	return(FALSE);
}

