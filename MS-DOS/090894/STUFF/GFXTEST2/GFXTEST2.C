
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
#include <SCROLL.H>
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

struct Scroll_data Test;

/* The current number of virtual OPMs */
UNSHORT Nr_of_virtual_OPMs = 0;
/* A pointer to the current list of virtual OPMs */
struct OPM *Virtual_OPM_list[];

void BBMAIN(void);
void My_update_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X,
 UNSHORT Buffer_Y, UNSHORT Playfield_X, UNSHORT Playfield_Y);
void Switch_screens(void);

void
BBMAIN(void)
{
	struct BBPOINT Mouse;
	SISHORT s,t;
	SISHORT Old_mx, Old_my;
	SISHORT mvecx = 0, mvecy = 0;
	UNSHORT HDOB_nr,XXX;
	UNBYTE a,b,c;
	BOOLEAN Quit_flag = FALSE;
	struct BLEV_Event_struct Event;

	if (!MEM_Init_memory(1, &Request))
		return;

	/* Generate 360 x 240 OPM */
	OPM_New(360, 240, 1, &Main_OPM, NULL);

	/* Open 360 x 240 screen with double-buffering */
	DSA_OpenScreen(&Screen, &Main_OPM, NULL, NULL, 0, 0, SCREENTYPE_DOUBLEBUFFER);

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

	DSA_SetPal(&Screen, &Palette, 0, 256, 0);
	DSA_ActivatePal(&Screen);

	Test.Update_unit = My_update_unit;
	Test.Unit_width = 16;
	Test.Unit_height = 16;
	Test.Viewport_width = 360;
	Test.Viewport_height = 192;
	Test.Playfield_width = 10000;
	Test.Playfield_height = 10000;

	SYSTEM_SystemTask();

	BLEV_GetMousePos(&Mouse);

	Old_mx = Mouse.x;
	Old_my = Mouse.y;

	Init_scroll(&Test, 0, 0);

	do
	{
		Do_scroll(&Test, mvecx, mvecy);

		Switch_screens();

		SYSTEM_SystemTask();

		/* Mausposition auslesen */
		BLEV_GetMousePos(&Mouse);

		if ((Mouse.x > 16) && (Mouse.x < 352))
			mvecx = Old_mx - Mouse.x;
		if ((Mouse.y > 16) && (Mouse.y < 224))
			mvecy = Old_my - Mouse.y;

/*		if (mvecx > 7)
			mvecx = 7;
		if (mvecy > 7)
			mvecy = 7;
		if (mvecx < -7)
			mvecx = -7;
		if (mvecy < -7)
			mvecy = -7;
*/
		Old_mx = Mouse.x;
		Old_my = Mouse.y;

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
					if (Event.sl_key_code=='q')
			 			Quit_flag = TRUE;
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

	MEM_Exit_memory();
}

void
My_update_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X, UNSHORT Buffer_Y,
 UNSHORT Playfield_X, UNSHORT Playfield_Y)
{
	OPM_FillBox(&(Scroll->Base_OPM), Buffer_X, Buffer_Y, Scroll->Unit_width,
	 Scroll->Unit_height, (Playfield_X + 4*Playfield_Y) & 0xFF);
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

	Scroll_display(&Test, &Screen, 0, 0);

	/* Display new screen */
	DSA_DoubleBuffer();

	/* Remove HDOBs */
	Erase_HDOBs(&Mouse);
};

