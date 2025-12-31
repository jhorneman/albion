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
#include <TEXT.H>

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

MEM_HANDLE Main_OPM_handle;
MEM_HANDLE Slab_handle;

BOOLEAN Quit_program = FALSE;

struct Party_data PARTY_DATA;

UNSHORT Screen_width = 360, Screen_height = 240;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BBMAIN
 * FUNCTION  : Main function of Albion
 * FILE      : GAME.C
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
	MEM_HANDLE Pal_handle;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Evaluate arguments */
	if (argc > 1)
		PARTY_DATA.Map_nr = atoi(argv[1]);
	else
		PARTY_DATA.Map_nr = 102;

	PARTY_DATA.X = 10;
	PARTY_DATA.Y = 10;

	/* Reset system */
//	Reset_mouse_stack();
	Reset_root_stack();
	Reset_module_stack();

	/* Generate main OPM and open screen */
	Main_OPM_handle = MEM_Do_allocate(Screen_width * Screen_height,
	 (UNLONG) &Main_OPM, &OPM_ftype);

	Ptr = MEM_Claim_pointer(Main_OPM_handle);
	OPM_New(Screen_width, Screen_height, 1, &Main_OPM, Ptr);
	MEM_Free_pointer(Main_OPM_handle);

	DSA_OpenScreen(&Screen, &Main_OPM, NULL, NULL, 0, 0, SCREENTYPE_DOUBLEBUFFER);

	/* Show mouse pointer */
	SYSTEM_ShowMouseptr(SYSTEM_MousePointerNormal);

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

	DSA_SetPal(&Screen, &Palette, 192, 64, 192);
	DSA_ActivatePal(&Screen);

	/* Load slab */
	Slab_handle = XLD_Load_file(&Xftypes[SLAB]);

	Ptr = MEM_Claim_pointer(Slab_handle);

	Put_unmasked_block(&Main_OPM, 0, 0, 360, 240, Ptr);

	MEM_Free_pointer(Slab_handle);

	Init_font();

	/* Initialize the map */
	Init_map();

	/* This is the global main loop. After each element follows a check
		to see if the program should be ended. */
	while (!Quit_program)
	{
		Update_display();
		if (Quit_program)
			break;

		Handle_input();
		if (Quit_program)
			break;

		Switch_screens();
	}

	/* Close the screen */
	DSA_CloseScreen(&Screen);

	/* Delete OPM */
	OPM_Del(&Main_OPM);
	MEM_Free_memory(Main_OPM_handle);
}

void
Exit_program(void)
{
	Quit_program = TRUE;
}

