
#define MODULES_MAX	(8)

struct XOPM {
	UNSHORT Flags;
	struct XOPM *Brother;
	struct XOPM *Child;
	struct OPM OPM;
}

/* Bitmasks for XOPM.Flags */
#define XOPM_UPDATE		(1<<0)

struct Module {
	UNBYTE Flags;
	UNBYTE Type;
	void (*DisUpd_function)(void);
	struct ScrQ_entry *ScrQ;
	void (*ModInit_function)(void);
	void (*ModExit_function)(void);
	void (*DisInit_function)(void);
	void (*DisExit_function)(void);
	/* Palette ? */
}

#define LOCAL_MOD			(1<<0)

#define SCREEN_MOD		(1<<0)
#define WINDOW_MOD		(1<<1)
#define MODE_MOD			(1<<2)

struct ScrQ_entry {
	UNSHORT Frequency;
	UNSHORT Counter;
	void (*Function)(void);
};


/* The main OPM */
struct OPM Main_OPM;
/* The screen structure */
struct SCREENPORT Screen;

/* The palette */
struct BBPALETTE Palette;


/* The current number of virtual OPMs */
UNSHORT Nr_of_virtual_OPMs;
/* A pointer to the current list of virtual OPMs */
struct Virtual_OPM *Virtual_OPM_list[];

/* The module stack */
struct Module Module_stack[MODULES_MAX];
UNSHORT Module_stack_index;



void Switch_screens(void);
void ScrQ_handler(void);

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_screen
 * FUNCTION  : Initialize the screen.
 * FILE      :
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 16:38
 * LAST      : 20.07.94 16:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_screen(void)
{

	/* Generate 360 x 240 OPM */
	OPM_New(360, 240, 1, &Main_OPM, NULL);

	/* Open 360 x 240 screen with double-buffering */
	DSA_OpenScreen(&Screen, &Main_OPM, NULL, NULL, 0, 0, SCREENTYPE_DOUBLEBUFFER);

	/* Show mouse pointer */
	SYSTEM_ShowMouseptr(SYSTEM_MousePointerNormal);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_screen
 * FUNCTION  : .
 * FILE      :
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 11:34
 * LAST      : 20.07.94 11:34
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

/*	DSA_CopyMainOPMToScreen(&scrp,DSA_CMOPMTS_ALWAYS);
	DSA_DoubleBuffer();
	DSA_CopyMainOPMToScreen(&scrp,DSA_CMOPMTS_ALWAYS);
*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Switch_screens
 * FUNCTION  : Display the currently invisible screen.
 * FILE      :
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 11:34
 * LAST      : 20.07.94 11:34
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
	struct OPM *OPM;
	struct BBPOINT Mouse;
	UNSHORT i;

	/* Get mouse coordinates */
	BLEV_GetMousePos(&Mouse);

	/* Draw HDOBs */
	Draw_HDOBs(&Mouse);

	/* Has the main OPM changed ? */
	if (Main_OPM.status & OPMSTAT_CHANGED)
	{
		/* Yes -> Copy the main OPM to the screen */
		DSA_CopyMainOPMToScreen(&Screen, DSA_CMOPMTS_ALWAYS);

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
					DSA_CopyOPMToScreen(&Screen, OPM, 0, 0, DSA_CMOPMTS_ALWAYS);

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

	/* Display new screen */
	DSA_DoubleBuffer();

	/* Remove HDOBs */
	Remove_HDOBs(&Mouse);

	/* Handle screen queue */
	ScrQ_handler();
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ScrQ_handler
 * FUNCTION  : Handle the current module's screen queue.
 * FILE      :
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 15:08
 * LAST      : 20.07.94 15:08
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
ScrQ_handler(void)
{
	struct ScrQ_entry *ScrQ_ptr;
	UNSHORT c;

	/* Get pointer to current module's screen queue */
	ScrQ_ptr = Module_stack[Module_stack_index].ScrQ;

	/* Is there a queue ? */
	if (ScrQ_ptr)
	{
		/* Yes -> Go through the queue */
		while(ScrQ_ptr->Frequency)
		{
			/* Get counter */
			c = ScrQ_ptr->Counter;

			/* Execute function if counter is zero */
			if (!c)
				ScrQ_ptr->Function();

			/* Increase counter */
			c++;

			/* Reset if counter equals frequency */
			if (c = =ScrQ_ptr->Frequency)
				c = 0;

			/* Store new counter */
			ScrQ_ptr->Counter = c;

			/* Next screen queue entry */
			ScrQ_ptr++;
		}
	}
}

