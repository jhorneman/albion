

/* Special item status bits */
#define COMPASS_INITIALIZED	(1 << 0)
#define COMPASS_VISIBLE			(1 << 1)
#define CLOCK_INITIALIZED		(1 << 2)
#define CLOCK_VISIBLE			(1 << 3)
#define MONSTER_EYE_VISIBLE	(1 << 4)

void Init_special_items(void);
void Exit_special_items(void);
void Update_special_items(void);

static UNSHORT Special_item_status = 0;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_special_items
 * FUNCTION  : Initialize special items.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 14:51
 * LAST      : 03.05.95 14:51
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_special_items(void)
{
	BOOLEAN Result;
	UNBYTE *Ptr;

	/* Compass initialized ? */
	if (!(Special_item_status & COMPASS_INITIALIZED))
	{
		/* No -> Allocate memory for compass graphics */
		Compass_OPM_handle = MEM_Do_allocate(30 * 29, (UNLONG) &Compass_OPM,
		 &OPM_ftype);
		if (!Compass_OPM_handle)
		{
			Error(ERROR_OUT_OF_MEMORY);
			return;
		}

		/* Clear memory */
		MEM_Clear_memory(Compass_OPM_handle);

		/* Insert handle in HDOB data */
		Compass_HDOB.Graphics_handle = Compass_OPM_handle;

		/* Create a new OPM */
		Ptr = MEM_Claim_pointer(Compass_OPM_handle);
		Result = OPM_New(30, 29, 1, &Compass_OPM, Ptr);
		MEM_Free_pointer(Compass_OPM_handle);

		/* Success ? */
		if (!Result)
		{
			/* No -> Free OPM memory */
			MEM_Free_memory(Compass_OPM_handle);
			Compass_OPM_handle = NULL;

			/* Report error */
			Error(ERROR_NO_OPM);

			return;
		}

		/* Indicate compass was initialized */
		Special_item_status |= COMPASS_INITIALIZED;
	}

	/* Clock initialized ? */
	if (!(Special_item_status & CLOCK_INITIALIZED))
	{
		/* No -> Allocate memory for clock graphics */
		Clock_OPM_handle = MEM_Do_allocate(32 * 25, (UNLONG) &Clock_OPM,
		 &OPM_ftype);
		if (!Clock_OPM_handle)
		{
			Error(ERROR_OUT_OF_MEMORY);
			return;
		}

		/* Clear memory */
		MEM_Clear_memory(Clock_OPM_handle);

		/* Insert handle in HDOB data */
		Clock_HDOB.Graphics_handle = Clock_OPM_handle;

		/* Create a new OPM */
		Ptr = MEM_Claim_pointer(Clock_OPM_handle);
		Result = OPM_New(32, 25, 1, &Clock_OPM, Ptr);
		MEM_Free_pointer(Clock_OPM_handle);

		/* Success ? */
		if (!Result)
		{
			/* No -> Free OPM memory */
			MEM_Free_memory(Clock_OPM_handle);
			Clock_OPM_handle = NULL;

			/* Report error */
			Error(ERROR_NO_OPM);

			return;
		}

		/* Indicate clock was initialized */
		Special_item_status |= CLOCK_INITIALIZED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_special_items
 * FUNCTION  : Exit special items.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 14:51
 * LAST      : 03.05.95 14:51
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_special_items(void)
{
	/* Compass initialized ? */
	if (Special_item_status & COMPASS_INITIALIZED)
	{
		/* Yes -> Delete compass OPM and free memory */
		OPM_Del(&Compass_OPM);
		MEM_Free_memory(Compass_OPM_handle);

		/* Indicate compass is no longer initialized */
		Special_item_status &= ~COMPASS_INITIALIZED;
	}

	/* Clock initialized ? */
	if (Special_item_status & CLOCK_INITIALIZED)
	{
		/* Yes -> Delete clock OPM and free memory */
		OPM_Del(&Clock_OPM);
		MEM_Free_memory(Clock_OPM_handle);

		/* Indicate clock is no longer initialized */
		Special_item_status &= ~CLOCK_INITIALIZED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_special_items
 * FUNCTION  : Update special items.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 15:30
 * LAST      : 04.05.95 14:54
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_special_items(void)
{
	static UNSHORT Compass_bit_frame = 0;
	UNSHORT Special_item_flags;
	UNSHORT Index;

	/* Cheat mode on ? */
	if (Cheat_mode)
	{
		/* Yes -> Party has all special items */
		Special_item_flags = 0xFFFF;
	}
	else
	{
		/* No -> Get special item flags */
		Special_item_flags = PARTY_DATA.Special_item_flags;
	}

	/* Compass initialized ? */
	if (Special_item_status & COMPASS_INITIALIZED)
	{
		/* Yes -> Does the party have the compass / 3D map ? */
		if ((Special_item_flags & COMPASS_ITEM_FLAG) && _3D_map)
		{
			/* Yes -> Re-build compass */
			Put_unmasked_block(&Compass_OPM, 0, 0, 30, 29, &Compass_symbol[0]);

			/* Get angle position index */
			Index = ((((0 - (I3DM.Camera_angle / 65536)) - (ANGLE_STEPS / 120)) &
			 (ANGLE_STEPS - 1)) / (ANGLE_STEPS / 60) + 1) % 60;

			/* Draw compass bit */
			Put_masked_block(&Compass_OPM, Compass_bit_coordinates[Index][0],
			 Compass_bit_coordinates[Index][1], 6, 6, &Compass_bit[0] +
			 Compass_bit_frame * 36);

			/* Update compass bit animation frame */
			Compass_bit_frame = (Compass_bit_frame + 1) & 0x0007;

			/* Compass already visible ? */
			if (!(Special_item_status & COMPASS_VISIBLE))
			{
				/* No -> Add compass HDOB */
				Compass_HDOB_nr = Add_HDOB(&Compass_HDOB);

				/* Indicate the compass is now visible */
				Special_item_status |= COMPASS_VISIBLE;
			}
		}
		else
		{
			/* No -> Compass visible ? */
			if (Special_item_status & COMPASS_VISIBLE)
			{
				/* Yes -> Remove compass HDOB */
				Delete_HDOB(Compass_HDOB_nr);

				/* Indicate the compass is no longer visible */
				Special_item_status &= ~COMPASS_VISIBLE;
			}
		}
	}

	/* Does the party have the monster eye / 3D map ? */
	if ((Special_item_flags & MONSTER_EYE_ITEM_FLAG) && _3D_map)
	{
		/* Yes -> Monster eye already visible ? */
		if (!(Special_item_status & MONSTER_EYE_VISIBLE))
		{
			/* No -> Add monster eye HDOB */
			Monster_eye_HDOB_nr = Add_HDOB(&Monster_eye_HDOB);

			/* Indicate the monster eye is now visible */
			Special_item_status |= MONSTER_EYE_VISIBLE;
		}

		/* Is a monster watching ? */
		if (Monster_is_watching)
		{
			/* Yes -> Open monster eye */
			Monster_eye_HDOB.Graphics_offset = (UNLONG) (&Eye_symbols[0] +
			 32 * 27);
		}
		else
		{
			/* No -> Close monster eye */
			Monster_eye_HDOB.Graphics_offset = (UNLONG) (&Eye_symbols[0]);
		}

		/* Change monster eye HDOB */
		Change_HDOB(Monster_eye_HDOB_nr, &Monster_eye_HDOB);
	}
	else
	{
		/* No -> Monster eye visible ? */
		if (Special_item_status & MONSTER_EYE_VISIBLE)
		{
			/* Yes -> Remove monster_eye HDOB */
			Delete_HDOB(Monster_eye_HDOB_nr);

			/* Indicate the monster_eye is no longer visible */
			Special_item_status &= ~MONSTER_EYE_VISIBLE;
		}
	}

	/* Clock initialized ? */
	if (Special_item_status & CLOCK_INITIALIZED)
	{
		/* Yes -> Does the party have the clock ? */
		if (Special_item_flags & CLOCK_ITEM_FLAG)
		{
			/* Yes -> Re-build clock */
			Put_unmasked_block(&Clock_OPM, 0, 0, 32, 25, &Clock_symbol[0]);

			/* Draw first digit of current hour */
			Put_masked_block(&Clock_OPM, 2, 10, 6, 7, &Time_font[0] +
			 (PARTY_DATA.Hour / 10) * 42);

			/* Draw second digit of current hour */
			Put_masked_block(&Clock_OPM, 8, 10, 6, 7, &Time_font[0] +
			 (PARTY_DATA.Hour % 10) * 42);

			/* Draw first digit of current minute */
			Put_masked_block(&Clock_OPM, 16, 10, 6, 7, &Time_font[0] +
			 (PARTY_DATA.Minute / 10) * 42);

			/* Draw second digit of current minute */
			Put_masked_block(&Clock_OPM, 22, 10, 6, 7, &Time_font[0] +
			 (PARTY_DATA.Minute % 10) * 42);

			/* Clock already visible ? */
			if (!(Special_item_status & CLOCK_VISIBLE))
			{
				/* No -> Add clock HDOB */
				Clock_HDOB_nr = Add_HDOB(&Clock_HDOB);

				/* Indicate the clock is now visible */
				Special_item_status |= CLOCK_VISIBLE;
			}
		}
		else
		{
			/* No -> Clock visible ? */
			if (Special_item_status & CLOCK_VISIBLE)
			{
				/* Yes -> Remove clock HDOB */
				Delete_HDOB(Clock_HDOB_nr);

				/* Indicate the clock is no longer visible */
				Special_item_status &= ~CLOCK_VISIBLE;
			}
		}
	}
}

