
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

#include <HDOB.H>
#include <GFXFUNC.H>
#include "C:\PCLIB32\LBM\LBM.H"

#define OPMSTAT_USERDEFINED	(1L<<16L)

/* The main OPM */
struct OPM Main_OPM;
/* The screen structure */
struct SCREENPORT Screen;

/* The palette */
struct BBPALETTE Palette;

struct Memory_request Request = {
	0xFF,
	BASEMEM_Status_Flat,
	100000,
	0xFFFFFFFF
};

UNBYTE Blockje[1024];
UNBYTE Table[256];

struct HDOB My_HDOB = {
	-15,-15,
	32,32,
	HDOB_RECOLOUR,
	0,
	&Table[0],
	NULL,
	(UNLONG) &Blockje,
	NULL
};

struct OPM Base_OPM, Scroll_OPM;
SISHORT SX = 0, SY = 0;

/* The current number of virtual OPMs */
UNSHORT Nr_of_virtual_OPMs = 0;
/* A pointer to the current list of virtual OPMs */
struct OPM *Virtual_OPM_list[];

void BBMAIN(void);

void
BBMAIN(void)
{
	struct BBPOINT Mouse;
	SISHORT s,t;
	SISHORT HX = 10, HY = 100;
	UNSHORT HDOB_nr,XXX;
	UNBYTE a,b,c;
	BOOLEAN Quit_flag = FALSE;

	if (!MEM_Init_memory(1, &Request))
		return;

	/* Generate 360 x 240 OPM */
	OPM_New(360, 240, 1, &Main_OPM, NULL);

	/* Open 360 x 240 screen with double-buffering */
	DSA_OpenScreen(&Screen, &Main_OPM, NULL, NULL, 0, 0, SCREENTYPE_DOUBLEBUFFER);

	OPM_New(360, 240, 1, &Base_OPM, NULL);
	OPM_CreateVirtualOPM(&Base_OPM, &Scroll_OPM, SX, SY, 200, 100);

	/* Palette auf Dummywerte setzten */
	a=255;b=0;c=0;
	for(t=1;t<256;t++){
		Palette.color[t].red=a;
		Palette.color[t].green=b;
		Palette.color[t].blue=c;
		a-=2;b+=4;c++;
	}

	/* Show mouse pointer */
	SYSTEM_ShowMouseptr(SYSTEM_MousePointerNormal);

	/* Zeichne Punkte in OPM */
	for(t=20;t<240-20;t++){
		a=t-20;
		for(s=20;s<360-20;s++){
			OPM_SetPixel(&Main_OPM,s,t,a++);
		}
	}

	/* Box um Bildschirm zeichnen  */
	OPM_Box(&Main_OPM, 0, 0, 360, 240, 128);

	for (t=0;t<256;t++)
		Table[t] = 255-t;

	for (t=0;t<10;t++)
	{
		for (s=0;s<10;s++)
		{
			Blockje[t*32+s] = (UNBYTE) s+t;
			Blockje[t*32+31-s] = (UNBYTE) s+t;
			Blockje[(31-t)*32+s] = (UNBYTE) s+t;
			Blockje[(31-t)*32+31-s] = (UNBYTE) s+t;
		}
	}

	for (t=0;t<32;t++)
	{
		Blockje[t] = 0xFF;
		Blockje[31*32+t] = 0xFF;

		Blockje[t*32] = 0xFF;
		Blockje[t*32+31] = 0xFF;
	}

//	LBM_DisplayLBM("TEST.LBM", &Main_OPM, &Palette);
	LBM_DisplayLBM("TEST.LBM", &Base_OPM, &Palette);

	DSA_SetPal(&Screen, &Palette, 0, 256, 0);
	DSA_ActivatePal(&Screen);

	Reset_HDOBs();
	HDOB_nr = Add_HDOB(&My_HDOB);

	do
	{
		struct BLEV_Event_struct Event;

		SX += 2;
		if (SX > 300)
			SX = 0;
		SY += 1;
		if (SY > 150)
			SY = 0;
		OPM_SetVirtualPosition(&Scroll_OPM, SX, SY);

		Set_HDOB_position(HDOB_nr,HX,HY);
		HX += 4;
		if (HX >300)
			HX = 10;

//		Scroll_block(&Main_OPM, 20, 20, 100, 50, 2, 0);
//		Scroll_block(&Main_OPM, 200, 80, 100, 150, 0, -2);

		Switch_screens();

		SYSTEM_SystemTask();

		/* Mausposition auslesen */
		BLEV_GetMousePos(&Mouse);

		if(BLEV_GetEvent(&Event))
		{
			switch(Event.sl_eventtype)
			{
				/* linke Maustaste doppelclick */
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
				/* rechte Maustaste losgelassen */
				case BLEV_MOUSERDOWN:
				{
		 			Quit_flag = TRUE;
					break;
				}
				/* rechte Maustaste gedrckt */
				case BLEV_MOUSERUP:
				{
		 			Quit_flag = TRUE;
					break;
				}
				case BLEV_KEYDOWN:
				{
					if (Event.sl_key_code=='Q')
			 			Quit_flag = TRUE;
					break;
				}

			}
		}
	}
	while (!Quit_flag);

	Delete_HDOB(HDOB_nr);

	/* Close the screen */
	DSA_CloseScreen(&Screen);

	/* Delete OPM */
	OPM_Del(&Main_OPM);

	MEM_Exit_memory();
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

	DSA_CopyOPMToScreen(&Screen, &Scroll_OPM, 0, 0, DSA_CMOPMTS_ALWAYS);

	/* Display new screen */
	DSA_DoubleBuffer();

	/* Remove HDOBs */
	Erase_HDOBs(&Mouse);
};

