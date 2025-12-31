/************
 * NAME     : DIAGNOST.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 13-9-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <BBDEF.H>
#include <BBDOS.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBOPM.H>
#include <BBEVENT.H>
#include <BBMEM.H>
#include <BBBASMEM.H>
#include <BBMISC.H>

#include <FLC.H>
#include <GFXFUNC.H>

#include <MAP.H>
#include <FONT.H>
#include <2D_MAP.H>
#include <GAMEVAR.H>
#include <XFTYPES.H>
#include <CONTROL.H>
#include <USERFACE.H>
#include <DIAGNOST.H>
#include <AUTOMAP.H>
#include <INPUT.H>
#include <MUSIC.H>
#include <MAGIC.H>
#include <POPUP.H>
#include <SCROLBAR.H>
#include <STATAREA.H>
#include <COMBAT.H>
#include <DIALOGUE.H>
#include <EVENTS.H>
#include <GRAPHICS.H>
#include <NRBAR.H>

/* global variables */

struct OPM Diag_OPM;
static MEM_HANDLE Diag_OPM_handle;
static UNSHORT Diag_X, Diag_Y, Diag_width, Diag_height;
static UNSHORT dCursor_X, dCursor_Y;

struct Textstyle Diagnostic_text_style = {
	7, 0, 0,
	&Normal_font,
	PRINT_JUSTIFIED, NORMAL_STYLE
};

static struct Flic_record FLC_IO;
static UNBYTE Current_flic_fname[100];
BOOLEAN Recording_flic = FALSE;
static struct OPM First_OPM, Previous_OPM, Input_OPM;
static MEM_HANDLE First_OPM_handle, Previous_OPM_handle, Input_OPM_handle;
static MEM_HANDLE Output_handle;

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_dprint
 * FUNCTION  : Initialize a diagnostic "screen".
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.94 19:21
 * LAST      : 22.09.94 19:21
 * INPUTS    : SISHORT X - Left X-coordinate of screen.
 *             SISHORT Y - Top Y-coordinate of screen.
 *             UNSHORT Width - Width of screen.
 *             UNSHORT Height - Height of screen.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_dprint(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	UNBYTE *Ptr;

	Push_module(&Default_module);

	Diag_X = X;
	Diag_Y = Y;
	Diag_width = Width;
	Diag_height = Height;

	Diag_OPM_handle = MEM_Do_allocate(Width * Height, (UNLONG) &Diag_OPM, &OPM_ftype);

	Ptr = MEM_Claim_pointer(Diag_OPM_handle);
	OPM_New(Width, Height, 1, &Diag_OPM, Ptr);
	MEM_Free_pointer(Diag_OPM_handle);

	Push_root(&Diag_OPM);
	Add_update_OPM(&Diag_OPM);

	Push_textstyle(&Diagnostic_text_style);

	Clrhome_dprint();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_dprint
 * FUNCTION  : Destroy the current diagnostic "screen".
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.94 19:17
 * LAST      : 22.09.94 19:17
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_dprint(void)
{
	if (dCursor_X || dCursor_Y)
	{
		Switch_screens();
		Wait_4_user();
	}

	Pop_textstyle();

	Remove_update_OPM(&Diag_OPM);
	Pop_root();

	OPM_Del(&Diag_OPM);

	MEM_Free_memory(Diag_OPM_handle);

	Pop_module();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clrhome_dprint
 * FUNCTION  : Erase the current diagnostic "screen" and reset the cursor.
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.94 19:17
 * LAST      : 22.09.94 19:17
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clrhome_dprint(void)
{
	dCursor_X = Diag_X;
	dCursor_Y = Diag_Y;

	OPM_FillBox(&Diag_OPM, Diag_X, Diag_Y, Diag_width, Diag_height, BLACK);
	Switch_screens();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : CR_dprint
 * FUNCTION  : "Print" a carriage return on the current diagnostic "screen".
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.94 19:17
 * LAST      : 22.09.94 19:17
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
CR_dprint(void)
{
	dCursor_X = Diag_X;
	dCursor_Y += 10;

	if (dCursor_Y > Diag_Y + Diag_height - 10)
	{
		Switch_screens();
		Wait_4_user();
		Clrhome_dprint();
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : dprintf
 * FUNCTION  : Print a string, using a printf-like format, on the current
 *              diagnostic "screen".
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.94 19:17
 * LAST      : 22.09.94 19:17
 * INPUTS    : UNCHAR *Format - Pointer to format string.
 *             ...
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Total (!) string length should not exceed 200 characters.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
dprintf(UNCHAR *Format, ...)
{
	va_list arglist;
	UNCHAR String[201];

	/* Build complete string */
	va_start(arglist, Format);
   vsprintf(&String[0], Format, arglist);
   va_end(arglist);

	/* Print it */
	Print_string(&Diag_OPM, dCursor_X, dCursor_Y, &String[0]);

	/* Update cursor position */
	dCursor_X += Get_line_width(&String[0]) + 1;
	if (dCursor_X > Diag_X + Diag_width)
		CR_dprint();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_diagnostic_keys
 * FUNCTION  : Check diagnostic keys.
 * FILE      : DIAGNOST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.09.94 11:02
 * LAST      : 13.09.94 11:02
 * INPUTS    : struct BLEV_Event_struct *Event - Key event.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_diagnostic_keys(struct BLEV_Event_struct *Event)
{
	BOOLEAN Result = TRUE;

	/* First check normal keys */
	switch (Event->sl_key_code)
	{
		/* Toggle diagnostic mode */
		case BLEV_F11:
			Diagnostic_mode++;

			if (Diagnostic_mode > 2)
				Diagnostic_mode = 0;

			if (!(_3D_map))
			{
				Redraw_2D_map();
			}
			break;
		/* Toggle cheat mode */
		case BLEV_F12:
			Cheat_mode = ~Cheat_mode;

			if (Cheat_mode)
			{
				/* Draw T1 */
				Put_masked_block(&Slab_OPM, 360 - T1_WIDTH, 240 - T1_HEIGHT,
				 T1_WIDTH, T1_HEIGHT, &(T1_symbol[0]));
			}
			else
			{
				/* Restore part of slab */
				OPM_CopyOPMOPM(&Slab_backup_OPM, &Slab_OPM, 0, 0, T1_WIDTH,
				 T1_HEIGHT, 360 - T1_WIDTH, 240 - T1_HEIGHT);
			}

			/* Draw part of slab */
			OPM_CopyOPMOPM(&Slab_OPM, &Status_area_OPM, 360 - T1_WIDTH,
			 240 - T1_HEIGHT, T1_WIDTH, T1_HEIGHT, 360 - T1_WIDTH,
			 48 - T1_HEIGHT);

			break;

		/* Nothing */
		default:
			Result = FALSE;
			break;
	}

	/* Then check diagnostic keys */
	if (!Result) // & (Event->ul_pressed_keys & BLEV_ALT))
	{
		Result = TRUE;
		switch (Event->sl_key_code)
		{
			#if FALSE
			/* Save screen */
			case BLEV_F1:
			{
				UNCHAR Buffer[100];

				if (Recording_flic)
				{
					Stop_flic_recording();
				}
				else
				{
					Buffer[0] = 0;
					Input_string(1, 1, 100, 12, Buffer);

					Update_display();
					Switch_screens();
					Switch_screens();

					if (Buffer[0])
					{
						Start_flic_recording(Buffer);
//						Save_screen(Buffer);
					}
				}

				break;
			}
			#endif
			case BLEV_F1:
			{
//				UNSHORT i, j;

				MEM_Check_memory();

				Init_dprint(0, 0, Screen_width, Screen_height);
				MEM_List();
				Exit_dprint();

/*				for (i=1;i<8;i++)
				{
					for (j=0;j<7;j++)
					{
						Text_colours[i][j] = BRIGHTER;
					}
				} */

				break;
			}
			/* Change map */
			case BLEV_F2:
			{
				BOOLEAN Result2;
				UNSHORT i;
				UNCHAR Buffer[100];

				/* Current screen is 2D map ? */
				if (Current_screen_type(0) == MAP_2D_SCREEN)
				{
					/* Yes -> Prepare screen */
					Current_2D_OPM = &Main_OPM;
					Draw_2D_scroll_buffer(MAP_2D_X, MAP_2D_Y);
					Current_2D_OPM = NULL;
				}

				/* Input map number */
				Buffer[0] = 0;
				Input_string_in_window(12, "Enter map number", Buffer);
				i = atoi(Buffer);

				/* Any number given ? */
				if (i)
				{
					/* Yes -> Change map */
					Exit_map();

					PARTY_DATA.Map_nr = i;
					PARTY_DATA.X = 10;
					PARTY_DATA.Y = 10;
					PARTY_DATA.View_direction = 3;
					PARTY_DATA.Travel_mode = 0;

					Result2 = Init_map();
					if (!Result2)
					{
						Exit_program();
					}
				}
				else
				{
					/* Current screen is 2D map ? */
					if (Current_screen_type(0) == MAP_2D_SCREEN)
					{
						/* Yes -> Prepare screen */
						Draw_2D_scroll_buffer(MAP_2D_X, MAP_2D_Y);
						Switch_screens();
					}
				}
				break;
			}
			/* Exit program */
			case BLEV_F3:
			{
				Exit_program();
				break;
			}
			case BLEV_F4:
			{
				UNCHAR Buffer[100];

				Buffer[0] = 0;
				Input_string_in_window(12, "Enter filename", Buffer);

				Update_display();
				Switch_screens();
				Switch_screens();

				if (Buffer[0])
				{
					Save_screen(Buffer);
				}

				break;
			}
			case BLEV_F5:
			{
				if (In_Combat)
				{
					Default_spell_strength = (UNSHORT) Input_number(50, 1, 100,
					 "Enter default spell strength");
				}
				break;
			}
			case BLEV_F6:
			{
				OPM_FillBox(&Status_area_OPM, 181, 3, 120, 40, BLACK);

				Push_textstyle(&Diagnostic_text_style);
				xprintf(&Status_area_OPM, 182, 4, "Map: %u", PARTY_DATA.Map_nr);
				xprintf(&Status_area_OPM, 182, 14, "X: %u, Y: %u, VD: %u",
				 PARTY_DATA.X, PARTY_DATA.Y, PARTY_DATA.View_direction);
				xprintf(&Status_area_OPM, 182, 24, "Triggermodes: %lx",
				 Get_map_event_triggers(PARTY_DATA.X, PARTY_DATA.Y));
				Pop_textstyle();

				break;
			}
			case BLEV_F7:
			{
				BOOLEAN Result2;

				Exit_map();

				PARTY_DATA.Map_nr = SHORTCUT_MAP_NR;
				PARTY_DATA.X = Shortcut_map_X;
				PARTY_DATA.Y = Shortcut_map_Y;
				PARTY_DATA.View_direction = 1;
				PARTY_DATA.Travel_mode = 0;

				Result2 = Init_map();
				if (!Result2)
				{
					Exit_program();
				}

				break;
			}
			case BLEV_F8:
			{
				Enter_Combat(2, 3);
				break;
			}
			case BLEV_F9:
			{
				break;
			}
			/* Nothing */
			default:
				Result = FALSE;
				break;
		}
	}

	/* Exit */
	return (Result);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_List
 * FUNCTION  : List the current status of the memory manager.
 * FILE      : DIAGNOST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.06.94 18:43
 * LAST      : 22.09.94 16:48
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_List(void)
{
	struct Memory_workspace *Current,*Workspace;
	struct Memory_entry *Area, *Entry;
	struct File_type *Ftype;
	MEM_HANDLE Handle;
	UNLONG TFM, PFM;
	UNSHORT i,j;

	dprintf("Free flat memory outside BBMEM : %lu bytes.", BASEMEM_Get_free_memory(BASEMEM_Status_Flat));
	CR_dprint();
	dprintf("Free DOS memory outside BBMEM : %lu bytes.", BASEMEM_Get_free_memory(BASEMEM_Status_Dos));
	CR_dprint();

	Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

	for (j=0;j<MEMORY_WORKSPACES_MAX;j++)
	{
		Workspace = &MEM_Workspaces[j];
		if (Workspace->Nr_of_areas)
		{
			/* Print workspace data */
			dprintf("*** WORKSPACE %u (%u area",j,Workspace->Nr_of_areas);
			if (Workspace->Nr_of_areas > 1)
				dprintf("s");
			dprintf(")");
			if (Workspace == Current)
				dprintf(" <- CURRENT");
			dprintf(" ***");
			CR_dprint();

			/* Do all areas in the current workspace */
			for (i=0;i<Workspace->Nr_of_areas;i++)
			{
				Area = &(Workspace->Areas[i]);

				/* Print area data */
				dprintf("--- AREA %u (%lu bytes) ---",i,Area->Size);
				CR_dprint();
				TFM = MEM_Calculate_TFM(Area);
				PFM = TFM + MEM_Calculate_EMG(Area, 255);
				dprintf("In use : %lu, free : %lu, potential : %lu.",
				 Area->Size - PFM, TFM, PFM);
				CR_dprint();
				dprintf("------------------------");
				CR_dprint();

				/* Do all memory blocks in the current area */
				Entry = Area->Next;
				while (Entry)
				{
					if (Entry->BLOCK_HANDLE)
					{
						Handle = Entry->BLOCK_HANDLE;

						/* Print memory block data */
						if (Handle->Flags & MEM_ALLOCATED)
						{
							dprintf("Allocated : %lu bytes. Handle %lu,  C:%u,  L:%u  P:%u",
							 Entry->Size, (UNLONG) (Handle - &MEM_Handles[0] + 1),
 							 Handle->Claim_counter,Handle->Load_counter,
							 Handle->Priority);

							if (Handle->Flags & MEM_INVALID)
								dprintf(" (invalid)");
						}
						else
						{
							dprintf("Persistent : %lu bytes. Handle %lu,  C:%u,  L:%u  P:%u",
							 Entry->Size, (UNLONG) (Handle - &MEM_Handles[0] + 1),
 							 Handle->Claim_counter,Handle->Load_counter,
							 Handle->Priority);
						}
						dprintf(".");
						CR_dprint();

						/* Print file type data */
						Ftype = Handle->File_type_ptr;
						if (Ftype->Name)
						{
							dprintf("  %s (%lu)",(char *) Ftype->Name,Handle->File_index);
							CR_dprint();
						}
					}
					else
					{
						dprintf("Free : %lu bytes.",Entry->Size);
						CR_dprint();
					}
					/* Next memory block */
					Entry = Entry->Next;
				}
			}
			CR_dprint();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_screen
 * FUNCTION  : Save the current screen.
 * FILE      : DIAGNOST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.09.94 11:08
 * LAST      : 13.09.94 11:08
 * INPUTS    : UNCHAR *Filename - Pointer to filename.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Save_screen(UNCHAR *Filename)
{
	static UNCHAR FORM_chunk[] = "FORM";
	static UNCHAR PBM_chunk[] = "PBM ";
	static UNCHAR BMHD_chunk[] = "BMHD";
	static UNCHAR CMAP_chunk[] = "CMAP";
	static UNCHAR BODY_chunk[] = "BODY";

	MEM_HANDLE Handle;
	struct OPM OPM;
	UNSHORT Chunk_sp = 0, i;
	UNBYTE *Chunk_stack[4], *Start, *Ptr, *Size_ptr;

	/* Allocate memory for work buffer */
	Handle = MEM_Allocate_memory((Screen_width * Screen_height) + 2000);
	Start = MEM_Claim_pointer(Handle);
	Ptr = Start;

	/* Start FORM chunk */
	Ptr = Write_chunk_name(Ptr, FORM_chunk);
	Chunk_stack[Chunk_sp++] = Ptr;
	Ptr += 4;

	/* Write FORM type : PBM */
	Ptr = Write_chunk_name(Ptr, PBM_chunk);

	/* Start BMHD chunk */
	Ptr = Write_chunk_name(Ptr, BMHD_chunk);
	Chunk_stack[Chunk_sp++] = Ptr;
	Ptr += 4;

	/* Raster width and height */
	Ptr = Write_Motorola_word(Ptr, Screen_width);
	Ptr = Write_Motorola_word(Ptr, Screen_height);

	/* X and Y */
	Ptr = Write_Motorola_word(Ptr, 0);
	Ptr = Write_Motorola_word(Ptr, 0);

	/* Number of bitplanes, masking, compression, pad byte */
	*Ptr++ = 0;
	*Ptr++ = 0;
	*Ptr++ = 0;
	*Ptr++ = 0;

	/* Transparent colour number */
	Ptr = Write_Motorola_word(Ptr, 0);

	/* X- and Y-aspect */
	*Ptr++ = 10;
	*Ptr++ = 11;

	/* Page dimensions */
	if (Super_VGA)
	{
		Ptr = Write_Motorola_word(Ptr, 640);
		Ptr = Write_Motorola_word(Ptr, 480);
	}
	else
	{
		Ptr = Write_Motorola_word(Ptr, 320);
		Ptr = Write_Motorola_word(Ptr, 200);
	}

	/* End BMHD chunk */
	Size_ptr = Chunk_stack[--Chunk_sp];
	Write_Motorola_long(Size_ptr, (UNLONG) Ptr - (UNLONG) Size_ptr - 4);

	/* Start CMAP chunk */
	Ptr = Write_chunk_name(Ptr, CMAP_chunk);
	Chunk_stack[Chunk_sp++] = Ptr;
	Ptr += 4;

	/* Write palette */
	for (i=0;i<256;i++)
	{
		*Ptr++ = Palette.color[i].red;
		*Ptr++ = Palette.color[i].green;
 		*Ptr++ = Palette.color[i].blue;
	}

	/* End CMAP chunk */
	Size_ptr = Chunk_stack[--Chunk_sp];
	Write_Motorola_long(Size_ptr, (UNLONG) Ptr - (UNLONG) Size_ptr - 4);

	/* Start BODY chunk */
	Ptr = Write_chunk_name(Ptr, BODY_chunk);
	Chunk_stack[Chunk_sp++] = Ptr;
	Ptr += 4;

	/* Create OPM for screen */
	OPM_New(Screen_width, Screen_height, 1, &OPM, Ptr);

	/* Copy screen to OPM */
	DSA_CopyScreenToOPM(&OPM, &Screen, 0, 0);

	Ptr += Screen_width * Screen_height;

	/* End BODY chunk */
	Size_ptr = Chunk_stack[--Chunk_sp];
	Write_Motorola_long(Size_ptr, (UNLONG) Ptr - (UNLONG) Size_ptr - 4);

	/* End FORM chunk */
	Size_ptr = Chunk_stack[--Chunk_sp];
	Write_Motorola_long(Size_ptr, (UNLONG) Ptr - (UNLONG) Size_ptr - 4);

	/* Save IFF-file */
	DOS_WriteFile(Filename, Start, Ptr - Start);

	/* Delete OPM and buffer */
	OPM_Del(&OPM);
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Write_Motorola_word
 * FUNCTION  : Write a word in Motorola format.
 * FILE      : DIAGNOST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.09.94 11:24
 * LAST      : 13.09.94 11:24
 * INPUTS    : UNBYTE *Address - Destination address of word.
 *             UNSHORT Word - Word.
 * RESULT    : UNBYTE * : Updated pointer.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Write_Motorola_word(UNBYTE *Address, UNSHORT Word)
{
	*Address++ = Word >> 8;
	*Address++ = Word & 0x00FF;

	return Address;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Write_Motorola_long
 * FUNCTION  : Write a longword in Motorola format.
 * FILE      : DIAGNOST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.09.94 12:58
 * LAST      : 13.09.94 12:58
 * INPUTS    : UNBYTE *Address - Destination address of word.
 *             UNLONG Long - Longword.
 * RESULT    : UNBYTE * : Updated pointer.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Write_Motorola_long(UNBYTE *Address, UNLONG Long)
{
	*Address++ = (Long & 0xFF000000) >> 24;
	*Address++ = (Long & 0x00FF0000) >> 16;
	*Address++ = (Long & 0x0000FF00) >> 8;
	*Address++ = Long & 0x000000FF;

	return Address;
}














void
Start_flic_recording(UNBYTE *Filename)
{
	UNBYTE *Ptr;

	strncpy(Current_flic_fname, Filename, 99);

	FLC_IO.Flic_filename = Current_flic_fname;

	First_OPM_handle = MEM_Do_allocate(Screen_width * Screen_height,
	 (UNLONG) &First_OPM, &OPM_ftype);
	Ptr = MEM_Claim_pointer(First_OPM_handle);
	OPM_New(Screen_width, Screen_height, 1, &First_OPM, Ptr);
	MEM_Free_pointer(First_OPM_handle);
	FLC_IO.First_frame = &First_OPM;

	Previous_OPM_handle = MEM_Do_allocate(Screen_width * Screen_height,
	 (UNLONG) &Previous_OPM, &OPM_ftype);
	Ptr = MEM_Claim_pointer(Previous_OPM_handle);
	OPM_New(Screen_width, Screen_height, 1, &Previous_OPM, Ptr);
	MEM_Free_pointer(Previous_OPM_handle);
	FLC_IO.Previous_frame = &Previous_OPM;

	Input_OPM_handle = MEM_Do_allocate(Screen_width * Screen_height,
	 (UNLONG) &Input_OPM, &OPM_ftype);
	Ptr = MEM_Claim_pointer(Input_OPM_handle);
	OPM_New(Screen_width, Screen_height, 1, &Input_OPM, Ptr);
	MEM_Free_pointer(Input_OPM_handle);
	FLC_IO.Input_frame = &Input_OPM;

	Output_handle = MEM_Allocate_memory(100000);

	FLC_IO.Palette = &Palette;

	DSA_CopyScreenToOPM(&Input_OPM, &Screen, 0, 0);
	FLC_IO.Output_buffer = MEM_Claim_pointer(Output_handle);
	FLC_Start_flic_recording(&FLC_IO);
	MEM_Free_pointer(Output_handle);

	Recording_flic = TRUE;
}

void
Stop_flic_recording(void)
{
	FLC_Stop_flic_recording();

	MEM_Free_memory(Output_handle);

	OPM_Del(&Input_OPM);
	MEM_Free_memory(Input_OPM_handle);

	OPM_Del(&Previous_OPM);
	MEM_Free_memory(Previous_OPM_handle);

	OPM_Del(&First_OPM);
	MEM_Free_memory(First_OPM_handle);

	Recording_flic = FALSE;
}

#ifdef URGLE
void
Urgle(void)
{
	struct Flic_playback FLC;
	UNSHORT i;
	UNBYTE *Yahugga;

	Yahugga = DOS_ReadFile(Current_flic_fname, NULL, NULL);

	FLC.Input_buffer = Yahugga;
	FLC.Output_frame = &Main_OPM;
	FLC.Palette = &Palette;
	FLC.Screen = &Screen;
	FLC.Black_colour_index = BLACK;

	FLC_Start_flic_playback(&FLC);
	DSA_ActivatePal(&Screen);

	for (i=1;i<FLC.Nr_frames;i++)
	{
		FLC_Playback_flic_frame();
		DSA_ActivatePal(&Screen);

		Switch_screens();
	}

	FLC_Stop_flic_playback();
}
#endif

void
Record_flic_frame(void)
{
	if (Recording_flic)
	{
		DSA_CopyScreenToOPM(&Input_OPM, &Screen, 0, 0);
		FLC_IO.Output_buffer = MEM_Claim_pointer(Output_handle);
		FLC_Record_flic_frame();
		MEM_Free_pointer(Output_handle);
	}
}

