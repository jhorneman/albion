/************
 * NAME     : GAME.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26.07.94 16:46
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBDOS.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBEVENT.H>
#include <BBOPM.H>
#include <BBSYSTEM.H>
#include <BBMEM.H>

#include <XLOAD.H>
#include <SCROLL.H>
#include <GFXFUNC.H>

#include <MAP.H>
#include <GAME.H>
#include <2D_MAP.H>
#include <ANIMATE.H>
#include <XFTYPES.H>

#define OPMSTAT_USERDEFINED	(1L<<16L)

/* global variables */

struct OPM Main_OPM;
struct SCREENPORT Screen;
struct BBPALETTE Palette;

UNSHORT Nr_of_virtual_OPMs = 0;
struct OPM *Virtual_OPM_list[];

MEM_HANDLE Main_OPM_handle;
MEM_HANDLE Slab_handle;

Party_data PARTY_DATA;

UNSHORT Screen_width = 360, Screen_height = 240;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BBMAIN
 * FUNCTION  : Main function of Albion
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.94 16:48
 * LAST      : 26.07.94 16:48
 * INPUTS    : UNSHORT argc - Number of arguments.
 *             UNCHAR **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
BBMAIN(UNSHORT argc, UNCHAR **argv)
{
	struct BLEV_Event_struct Event;
	UNBYTE *Ptr;
	BOOLEAN Quit_flag = FALSE;

	if (argc > 1)
		PARTY_DATA.Map_nr = atoi(argv[1]);
	else
		PARTY_DATA.Map_nr = 100;

	/* Generate main OPM */
	Main_OPM_handle = MEM_Do_allocate(Screen_width * Screen_height,
	 (UNLONG) &Main_OPM, &OPM_ftype);

	Ptr = MEM_Claim_pointer(Main_OPM_handle);
	OPM_New(Screen_width, Screen_height, 1, &Main_OPM, Ptr);
	MEM_Free_pointer(Main_OPM_handle);

	/* Open screen */
	DSA_OpenScreen(&Screen, &Main_OPM, NULL, NULL, 0, 0, SCREENTYPE_DOUBLEBUFFER);

	/* Show mouse pointer */
	SYSTEM_ShowMouseptr(SYSTEM_MousePointerNormal);

	xxInit();

	DSA_SetPal(&Screen, &Palette, 0, 256, 0);
	DSA_ActivatePal(&Screen);

	do
	{
		Main_loop();

		Switch_screens();

		SYSTEM_SystemTask();

		if(BLEV_GetEvent(&Event))
		{
			switch(Event.sl_eventtype)
			{
				case BLEV_MOUSELDBL:
				{
		 			Quit_flag = TRUE;
					break;
				}
				case BLEV_MOUSERDBL:
				{
		 			Quit_flag = TRUE;
					break;
				}
				case BLEV_MOUSERDOWN:
				{
		 			Quit_flag = TRUE;
					break;
				}
				case BLEV_MOUSERUP:
				{
		 			Quit_flag = TRUE;
					break;
				}
				case BLEV_KEYDOWN:
				{
					if (Event.sl_key_code == BLEV_ESC)
			 			Quit_flag = TRUE;
					else
						Handle_keys_2D((UNSHORT) Event.sl_key_code);
					break;
				}
			}
		}
	}
	while (!Quit_flag);

	/* Close the screen */
	DSA_CloseScreen(&Screen);

	/* Delete OPM */
	OPM_Del(&Main_OPM);
	MEM_Free_memory(Main_OPM_handle);

	xxExit();
}

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

	Display_2D_map();

	/* Display new screen */
	DSA_DoubleBuffer();

	/* Remove HDOBs */
	Erase_HDOBs(&Mouse);
};

void
xxInit(void)
{
	MEM_HANDLE Pal_handle;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Load top part of palette */
	Pal_handle = XLD_Load_file(&Xftypes[BASEPAL]);

	Ptr = MEM_Claim_pointer(Pal_handle);

	for (i=192;i<256;i++)
	{
		Palette.color[i].red = *Ptr++;
		Palette.color[i].green = *Ptr++;
 		Palette.color[i].blue = *Ptr++;
	}

	MEM_Free_pointer(Pal_handle);
	MEM_Free_memory(Pal_handle);

	/* Load slab */
	Slab_handle = XLD_Load_file(&Xftypes[SLAB]);

	Ptr = MEM_Claim_pointer(Slab_handle);

	Put_unmasked_block(&Main_OPM, 0, 0, 360, 240, Ptr);

	MEM_Free_pointer(Slab_handle);

	Init_2D_map();
}

void
Main_loop(void)
{
	Update_2D_map();
}

void
xxExit(void)
{
	Exit_2D_map();
}

void
Colour_cycle_forward(UNSHORT Start, UNSHORT Size)
{
	struct BBCOLOR *Ptr, Last;
	UNSHORT i;

	Ptr = &Palette.color[Start];

	Last.red = Ptr[Size-1].red;
	Last.green = Ptr[Size-1].green;
	Last.blue = Ptr[Size-1].blue;

	for (i=Size-1;i>0;i--)
	{
		Ptr[i].red = Ptr[i-1].red;
		Ptr[i].green = Ptr[i-1].green;
		Ptr[i].blue = Ptr[i-1].blue;
	}

	Ptr[0].red = Last.red;
	Ptr[0].green = Last.green;
	Ptr[0].blue = Last.blue;

	DSA_SetPal(&Screen, &Palette, Start, Size, Start);
}

