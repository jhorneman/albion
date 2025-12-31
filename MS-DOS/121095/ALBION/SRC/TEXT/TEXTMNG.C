/************
 * NAME     : TEXTMNG.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 24-8-1994
 * PROJECT  : Text functions
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>
#include <BBOPM.H>
#include <BBDSA.H>

#include <TEXT.H>
#include "TEXTVAR.H"

#include <GFXFUNC.H>
#include <FONT.H>
#include <GRAPHICS.H>
#include <CONTROL.H>
#include <GAMEVAR.H>

/* prototypes */

void Cache_text_line(struct Cached_text_line *Cached, UNSHORT Line_nr,
 UNSHORT Height, UNCHAR *Text);

void Print_cached_string(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Cached_text_line *Cached);

struct Cached_text_line *Find_cached_text_line(UNSHORT Line_nr);

void Destroy_cached_text_line(struct Cached_text_line *Cached);

struct Cached_text_line *Find_free_text_line_cache(void);

void Update_text_cache_priorities(void);

/* global variables */

static BOOLEAN Text_cache_flag = FALSE;
static struct Cached_text_line Text_cache[MAX_TEXT_CACHE];

/* Background buffer file type */
static UNCHAR Cached_text_line_fname[] = "Cached text line";

static struct File_type Cached_text_line_ftype = {
	NULL,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	Cached_text_line_fname
};

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_text_size
 * FUNCTION  : Get the height and number of lines in a processed text.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 15:21
 * LAST      : 22.06.95 15:21
 * INPUTS    : UNCHAR *Text - Pointer to text.
 *             UNSHORT *Nr_lines_ptr - Pointer to number of lines.
 *             UNSHORT *Text_height_ptr - Pointer to text height.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_text_size(UNCHAR *Text, UNSHORT *Nr_lines_ptr, UNLONG *Text_height_ptr)
{
	struct Processed_text Processed;

	/* Process text (dry run) */
	Process_text_dry_run_flag = TRUE;
	Process_text(Text, &Processed);
	Process_text_dry_run_flag = FALSE;

	/* Store results */
	if (Nr_lines_ptr)
		*Nr_lines_ptr = Processed.Nr_of_lines;
	if (Text_height_ptr)
		*Text_height_ptr = Processed.Text_height;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Process_and_display_text
 * FUNCTION  : Process and display a single text.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.94 17:20
 * LAST      : 27.06.95 10:44
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             UNCHAR *Text - Pointer to text string.
 *             UNSHORT Display_flags - Display flags.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will leave the text on the screen.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Process_and_display_text(struct OPM *OPM, UNCHAR *Text,
 UNSHORT Display_flags)
{
	struct Processed_text Processed;

	/* Process the text */
	Process_text(Text, &Processed);

	/* Text caching on */
	Text_cache_flag = TRUE;

	/* Destroy old cache (just in case) */
	Destroy_text_cache();

	/* Display processed text */
	Display_processed_text(OPM, &Processed, Display_flags);

	/* Text caching off */
	Text_cache_flag = FALSE;

	/* Destroy processed text */
	Destroy_processed_text(&Processed);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Destroy_processed_text
 * FUNCTION  : Destroy a processed text.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.08.94 13:32
 * LAST      : 25.08.94 13:32
 * INPUTS    : struct Processed_text *Processed - Pointer to processed text.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Destroy_processed_text(struct Processed_text *Processed)
{
	struct TPB *TPB;
	MEM_HANDLE Handle1, Handle2;

	/* Get first handle */
	Handle1 = Processed->First_handle;
	while (Handle1)
	{
		/* Get TPB */
		TPB = (struct TPB *) MEM_Claim_pointer(Handle1);

		/* Get handle of next TPB */
		Handle2 = TPB->Next_handle;

		/* Destroy TPB */
		MEM_Free_pointer(Handle1);
		MEM_Free_memory(Handle1);

		/* Repeat until there is no next TPB */
		Handle1 = Handle2;
	}

	/* Clear */
	Processed->First_handle = NULL;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_processed_text
 * FUNCTION  : Display a processed text.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.94 17:45
 * LAST      : 03.10.95 22:09
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             struct Processed_text *Processed - Pointer to processed text
 *              data structure.
 *             UNSHORT Display_flags - Display flags.
 * RESULT    : None.
 * BUGS      : - When the PA is installed in a virtual OPM, the mouse will be
 *              contained in the wrong area.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_processed_text(struct OPM *OPM, struct Processed_text *Processed,
 UNSHORT Display_flags)
{
	struct BLEV_Event_struct Event;
	struct PA *PA;
	struct BBRECT Text_MA;
	struct OPM Text_OPM;
	BOOLEAN Exit_flag = TRUE;
	SILONG Current_text_Y;
	UNSHORT Lines_per_page;
	UNSHORT mY;
	UNSHORT i;

	/* Get current print area data */
	PA = PA_stack[PA_stack_index];

	/* Make mouse area */
	Text_MA.left = PA->Area.left;
	Text_MA.top = PA->Area.top;
	Text_MA.width = PA->Area.width;
	Text_MA.height = PA->Area.height;

	/* Set mouse pointer */
	Push_mouse(&(Mouse_pointers[DEFAULT_MPTR]));

	/* Create virtual OPM for text output */
	OPM_CreateVirtualOPM(OPM, &Text_OPM, PA->Area.left, PA->Area.top,
	 PA->Area.width, PA->Area.height);
	Add_update_OPM(&Text_OPM);

	/* Save background (if necessary) */
	Init_print_area(&Text_OPM);

	/* Calculate lines per page */
	Lines_per_page = PA->Area.height / STANDARD_TEXT_HEIGHT;

	/* More than one page of text ? */
	if (Processed->Text_height > (UNLONG) PA->Area.height)
	{
		/* Yes -> Install mouse area */
		Push_MA(&Text_MA);

		/* Display the first page of the text */
		Current_text_Y = 0;
		Refresh_text_window(&Text_OPM, 0, PA->Area.height, Current_text_Y,
		 Processed);
		Switch_screens();
		Wait_4_user();

		/* Display second to penultimate pages of the text */
		while (((SILONG) Processed->Text_height - Current_text_Y) >
		 (SILONG) (2 * PA->Area.height))
		{
			for (i=0;i<Lines_per_page;i++)
			{
				Current_text_Y += STANDARD_TEXT_HEIGHT;
				Refresh_text_window(&Text_OPM, 0, PA->Area.height,
				 Current_text_Y, Processed);
				Switch_screens();
			}
			Wait_4_user();
		}

		/* Display last page of text */
		for (;;)
		{
			Current_text_Y += STANDARD_TEXT_HEIGHT;

			if (Current_text_Y >= (SILONG) (Processed->Text_height -
			 (UNLONG) PA->Area.height))
			{
				Current_text_Y = Processed->Text_height  - PA->Area.height - 1;

				Refresh_text_window(&Text_OPM, 0, PA->Area.height,
				 Current_text_Y, Processed);
				Switch_screens();

				break;
			}

			Refresh_text_window(&Text_OPM, 0, PA->Area.height,
			 Current_text_Y, Processed);
			Switch_screens();
		}

		/* Wait for the user to release the left mouse button */
		Wait_4_unclick();

		/* Show click mouse-pointer */
		Push_mouse(&(Mouse_pointers[CLICK_MPTR]));

		/* Allow the user to scroll the text up and down */
		while (Exit_flag)
		{
			/* Get mouse Y-position */
			mY = Mouse_Y;

			/* In Bump up area ? */
			if (mY < PA->Area.top + 9)
			{
				/* Yes */
				Change_mouse(&(Mouse_pointers[UP2D_MPTR]));

				/* Can bump up ? */
				if (Current_text_Y > 0)
				{
					/* Yes -> Bump up */
					Current_text_Y -= 9 - (mY - PA->Area.top);
					if (Current_text_Y < 0)
						Current_text_Y = 0;
				}
			}
			else
			{
				/* No -> In Bump down area ? */
				if (mY > (PA->Area.top + PA->Area.height - 9))
				{
					/* Yes */
					Change_mouse(&(Mouse_pointers[DOWN2D_MPTR]));

					/* Can bump down ? */
					if (Current_text_Y < (SISHORT) (Processed->Text_height -
					 PA->Area.height))
					{
						/* Yes -> Bump down */
						Current_text_Y += 9 - ((PA->Area.top + PA->Area.height) -
						 mY);

						if (Current_text_Y >= (SILONG)(Processed->Text_height -
						 (UNLONG) PA->Area.height))
						{
							Current_text_Y = Processed->Text_height -
							 PA->Area.height - 1;
						}
					}
				}
				else
				{
					/* No */
					Change_mouse(&(Mouse_pointers[CLICK_MPTR]));
				}
			}

			/* Update display */
			Refresh_text_window(&Text_OPM, 0, PA->Area.height,
			 Current_text_Y, Processed);
			Switch_screens();

			do
			{
				/* Read event */
				Get_event(&Event);

				/* Did the user click or press a key ? */
				if ((Event.sl_eventtype == BLEV_KEYDOWN)
				 || (Event.sl_eventtype == BLEV_MOUSELSDOWN)
				 || (Event.sl_eventtype == BLEV_MOUSERSDOWN))
				{
					/* Yes -> Exit */
					Exit_flag = FALSE;
					break;
				}
			}
			/* Until there are no more events */
			while (Event.sl_eventtype != BLEV_NOEVENT);

			/* Do double-buffering */
			Switch_screens();
		}

		/* Wait for the user to release the left mouse button */
		Wait_4_unclick();

		/* Restore original mouse-pointer */
		Pop_mouse();

		/* Remove mouse area */
		Pop_MA();
	}
	else
	{
		/* No -> Wait for user ? */
		if (Display_flags & WAIT_TEXT_DISPLAY_FLAG)
		{
			/* Yes -> Install mouse area */
			Push_MA(&Text_MA);
		}

		/* Display the text */
		Refresh_text_window(&Text_OPM, 0, PA->Area.height, 0, Processed);

		/* Wait for user ? */
		if (Display_flags & WAIT_TEXT_DISPLAY_FLAG)
		{
			/* Yes -> Show text */
			Switch_screens();

			/* Delay / no cheat mode ? */
			if ((Display_flags & DELAY_TEXT_DISPLAY_FLAG) &&
			 (!Cheat_mode))
			{
				/* Yes -> Install wait mouse pointer */
				Push_mouse(&(Mouse_pointers[WAIT_MPTR]));

				/* Wait a short while */
				SYSTEM_WaitTicks(TEXT_DELAY);

				/* Remove mouse pointer */
				Pop_mouse();

				/* Remove inputs */
				Clear_input_buffer();
			}

			/* Wait */
			Wait_4_user();

			/* Remove mouse area */
			Pop_MA();
		}
 	}

	/* Restore background ? */
	if (Display_flags & CLEAR_TEXT_DISPLAY_FLAG)
	{
		/* Yes */
		Erase_print_area(&Text_OPM);
		Switch_screens();
  		Switch_screens();
	}

	/* Exit */
	Remove_update_OPM(&Text_OPM);
	Exit_print_area();

	/* Destroy all cached lines */
	Destroy_text_cache();

	/* Reset mouse pointer */
	Pop_mouse();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Refresh_text_window
 * FUNCTION  : Refresh the current text window.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.94 17:47
 * LAST      : 27.07.95 11:51
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT Top - Top Y-coordinate.
 *             SISHORT Bottom - Bottom Y-coordinate.
 *             SILONG Current_text_Y - Current Y-coordinate within text.
 *             struct Processed_text *Processed - Pointer to processed text
 *              data structure.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Y-coordinate 0 corresponds with the first line of the
 *              print area. Current_text_Y will be added to this coordinate
 *              to determine the position in the text.
 *             - This function assumes the text starts at the top-left corner
 *              of the target OPM.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Refresh_text_window(struct OPM *OPM, SISHORT Top, SISHORT Bottom,
 SILONG Current_text_Y, struct Processed_text *Processed)
{
	struct Cached_text_line *Cached;
	struct Line_info *Line;
	struct TPB *Current_TPB;
	MEM_HANDLE Current_TPB_handle;
	MEM_HANDLE Handle;
	SILONG Target_Y;
	SILONG Y;
	UNSHORT X;
	UNSHORT Line_nr;
	UNSHORT TPB_index;
	UNBYTE *Ptr;

	/* Erase the print area */
	Erase_print_area(OPM);

	/* Determine the position of the first line in the text */
	Target_Y = Current_text_Y + Top;
	if (Target_Y < 0)
		Target_Y = 0;

	/* Find processed TPB containing the first line */
	Current_TPB_handle = Processed->First_handle;
	Current_TPB = (struct TPB *) MEM_Claim_pointer(Current_TPB_handle);
	Y = 0;
	Line_nr = 0;
	for (;;)
	{
		/* Is the first line in this TPB ? */
		Y += Current_TPB->Height;
		if (Y > Target_Y)
		{
			/* Yes -> Found it */
			Y -= Current_TPB->Height;
			break;
		}

		/* No -> Keep track of line number */
		Line_nr += Current_TPB->Nr_of_lines;

		/* Was this the last TPB ? */
		Handle = Current_TPB->Next_handle;
		MEM_Free_pointer(Current_TPB_handle);

		if (!Handle)
		{
			/* Yes -> The first line is outside of the text */
			Y = -1;
			break;
		}

		/* Next TPB */
		Current_TPB_handle = Handle;
		Current_TPB = (struct TPB *) MEM_Claim_pointer(Current_TPB_handle);
	}

	/* Is the first line inside the text ? */
	if (Y != -1)
	{
		/* Yes -> find the position of the first line in this TPB */
		TPB_index = sizeof(struct TPB);
		Ptr = MEM_Get_pointer(Current_TPB_handle);

		for (;;)
		{
			Line = (struct Line_info *) (Ptr + TPB_index);

			/* Is this the first line ? */
			Y += Line->Skip;
			if (Y > Target_Y)
			{
				/* Yes -> Found it */
				Y -= Line->Skip;
				break;
			}

			/* No -> Keep track of line number */
			Line_nr++;

			/* Next line */
			TPB_index += sizeof(struct Line_info) + Line->String_length + 1;
			if (TPB_index % 2)
				TPB_index++;
		}

		/* Update text cache priorities */
		Update_text_cache_priorities();

		/* Print all lines */
		X = 0;
		Y -= Current_text_Y;
		Target_Y = Y + Bottom - Top + 1;

		while (TRUE)
		{
			/* Is text caching on ? */
			if (Text_cache_flag)
			{
				/* Yes -> Is this line of text in the text cache ? */
				Cached = Find_cached_text_line(Line_nr);
				if (Cached)
				{
					/* Yes -> Print cached text line */
					Print_cached_string(OPM, X, Y, Cached);

					/* Boost text line cache priority */
					Cached->Priority++;
				}
				else
				{
					/* No -> Is there a free slot in the text cache ? */
					Cached = Find_free_text_line_cache();
					if (Cached)
					{
						/* Yes -> Cache the line */
						Cache_text_line(Cached, Line_nr, Line->Height, Ptr + TPB_index);

						/* Print cached text line */
						Print_cached_string(OPM, X, Y, Cached);
					}
					else
					{
						/* No -> Just print the line */
						Print_processed_string(OPM, X, Y, Ptr + TPB_index);
					}
				}
			}
			else
			{
				/* No -> Just print the line */
				Print_processed_string(OPM, X, Y, Ptr + TPB_index);
			}

			/* Exit if outside the area */
			if (Y > Target_Y)
				break;

			/* Move down */
			Y += Line->Skip;
			Line_nr++;

			/* Next line */
			TPB_index += sizeof(struct Line_info) + Line->String_length + 1;
			if (TPB_index % 2)
				TPB_index++;
			Line = (struct Line_info *) (Ptr + TPB_index);

			/* End of TPB ? */
			if (*(Ptr + TPB_index) == EOTPB)
			{
				/* Yes -> Next TPB */
				Handle = Current_TPB->Next_handle;
				if (!Handle)
					break;

				MEM_Free_pointer(Current_TPB_handle);
				Current_TPB_handle = Handle;
				Ptr = MEM_Claim_pointer(Current_TPB_handle);
				Current_TPB = (struct TPB *) Ptr;

				TPB_index = sizeof(struct TPB);
				Line = (struct Line_info *) (Ptr + TPB_index);
			}
		}
		MEM_Free_pointer(Current_TPB_handle);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Erase_print_area
 * FUNCTION  : Erase the print area.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.08.94 10:46
 * LAST      : 11.10.95 18:28
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Erase_print_area(struct OPM *OPM)
{
	struct PA *PA;
	struct PA_data *PA_data;
	struct Textstyle *Textstyle;
	struct OPM *OPM_ptr;
	SISHORT X = 0, Y = 0;

	/* Get current PA data and text style */
	PA = PA_stack[PA_stack_index];
	PA_data = &(PA_data_stack[PA_stack_index]);
	Textstyle = Textstyle_stack[Textstyle_stack_index];

	/* Is the target OPM virtual ? */
	OPM_ptr = OPM;
	while (OPM_ptr->status & OPMSTAT_VIRTUAL)
	{
		/* Yes -> Adjust coordinates */
		X -= OPM_ptr->virtualx;
		Y -= OPM_ptr->virtualy;

		/* Recurse */
		OPM_ptr = OPM_ptr->virtualsrc;
	}

	/* Transparent paper / does a background exist ? */
	if ((Textstyle->Paper == 0xFFFF) &&
	 (PA_data->Background_handle))
	{
		/* Yes -> Restore background */
		OPM_CopyOPMOPM
		(
			&(PA_data->Background_OPM),
			OPM,
			0,
			0,
			PA->Area.width,
			PA->Area.height,
			X + PA->Area.left,
			Y + PA->Area.top
		);
	}
	else
	{
		/* No -> Draw paper box */
		OPM_FillBox
		(
			OPM,
			X + PA->Area.left,
			Y + PA->Area.top,
			PA->Area.width,
			PA->Area.height,
			Textstyle->Paper
		);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_print_area
 * FUNCTION  : Prepare a print area for printing.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.08.94 10:55
 * LAST      : 11.10.95 18:25
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_print_area(struct OPM *OPM)
{
	struct PA *PA;
	struct PA_data *PA_data;
	struct Textstyle *Textstyle;
	struct OPM *OPM_ptr;
	SISHORT X = 0, Y = 0;
	UNBYTE *Ptr;

	/* Get current PA data and text style */
	PA = PA_stack[PA_stack_index];
	PA_data = &(PA_data_stack[PA_stack_index]);
	Textstyle = Textstyle_stack[Textstyle_stack_index];

	/* Transparent paper ? */
	if (Textstyle->Paper == 0xFFFF)
	{
		/* Yes -> Background memory already allocated ? */
		if (PA_data->Background_handle)
		{
			/* Yes -> Free this memory */
			MEM_Free_memory(PA_data->Background_handle);
		}

		/* Allocate memory for background */
		PA_data->Background_handle = MEM_Do_allocate
		(
			PA->Area.width * PA->Area.height,
			0,
			&Background_buffer_ftype
		);

		/* Success ? */
		if (PA_data->Background_handle)
		{
			/* Yes -> Create background OPM */
			Ptr = MEM_Claim_pointer(PA_data->Background_handle);
			OPM_New
			(
				PA->Area.width,
				PA->Area.height,
				1,
				&(PA_data->Background_OPM),
				Ptr
			);
			MEM_Free_pointer(PA_data->Background_handle);

			/* Is the target OPM virtual ? */
			OPM_ptr = OPM;
			while (OPM_ptr->status & OPMSTAT_VIRTUAL)
			{
				/* Yes -> Adjust coordinates */
				X -= OPM_ptr->virtualx;
				Y -= OPM_ptr->virtualy;

				/* Recurse */
				OPM_ptr = OPM_ptr->virtualsrc;
			}

			/* Save background */
			OPM_CopyOPMOPM
			(
				OPM,
				&(PA_data->Background_OPM),
				X + PA->Area.left,
				Y + PA->Area.top,
				PA->Area.width,
				PA->Area.height,
				0,
				0
			);
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_print_area
 * FUNCTION  : Exit a print area.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.01.95 12:11
 * LAST      : 27.07.95 11:51
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_print_area(void)
{
	struct PA_data *PA_data;
	struct Textstyle *Textstyle;

	/* Get current PA data and text style */
	PA_data = &(PA_data_stack[PA_stack_index]);
	Textstyle = Textstyle_stack[Textstyle_stack_index];

	/* Transparent paper ? */
	if (Textstyle->Paper == 0xFFFF)
	{
		/* Yes -> Destroy background buffer and OPM */
		OPM_Del(&(PA_data->Background_OPM));
		MEM_Free_memory(PA_data->Background_handle);

		/* Clear handle */
		PA_data->Background_handle = NULL;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Cache_text_line
 * FUNCTION  : Cache a line of text.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.02.95 13:35
 * LAST      : 11.10.95 18:25
 * INPUTS    : struct Cached_text_line *Cached - Pointer to cached text line
 *              data.
 *             UNSHORT Line_nr - Number of line that should be cached (0...)
 *             UNSHORT Height - Height of text line.
 *             UNCHAR *Text - Pointer to text line.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Cache_text_line(struct Cached_text_line *Cached, UNSHORT Line_nr, UNSHORT
 Height, UNCHAR *Text)
{
	struct PA *PA;
	struct OPM Cache_OPM;
	UNBYTE *Ptr;

	/* Clear cached line data */
	Cached->Line_nr		= 0xFFFF;
	Cached->Priority		= 0;
	Cached->Width			= 0;
	Cached->Height			= 0;
	Cached->Line_handle	= NULL;

	/* Get current PA data */
	PA = PA_stack[PA_stack_index];

	/* Try to allocate memory for line cache */
	Cached->Line_handle = MEM_Do_allocate
	(
		PA->Area.width * (Height + 1),
		0,
		&Cached_text_line_ftype
	);

	/* Successful ? */
	if (Cached->Line_handle)
	{
		/* Yes -> Clear memory */
		MEM_Clear_memory(Cached->Line_handle);

		/* Create line cache OPM */
		Ptr = MEM_Claim_pointer(Cached->Line_handle);

		OPM_New
		(
			PA->Area.width,
			Height + 1,
			1,
			&Cache_OPM,
			Ptr
		);

		MEM_Free_pointer(Cached->Line_handle);

		/* Print line */
		Print_processed_string(&Cache_OPM, 0, 0, Text);

		/* Destroy OPM */
		OPM_Del(&Cache_OPM);

		/* Write cache data */
		Cached->Line_nr	= Line_nr;
		Cached->Priority	= 2;
		Cached->Width		= PA->Area.width;
		Cached->Height		= Height + 1;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Destroy_cached_text_line
 * FUNCTION  : Destroy a cached text line.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.02.95 13:38
 * LAST      : 01.02.95 13:54
 * INPUTS    : struct Cached_text_line *Cached - Pointer to cached text line
 *              data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Destroy_cached_text_line(struct Cached_text_line *Cached)
{
	/* Real cached line data ? */
	if (Cached->Line_nr != 0xFFFF)
	{
		/* Yes -> Destroy line cache memory buffer */
		MEM_Free_memory(Cached->Line_handle);

		/* Clear data */
		Cached->Line_nr = 0xFFFF;
		Cached->Priority = 0;
		Cached->Width = 0;
		Cached->Height = 0;
		Cached->Line_handle = NULL;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Destroy_text_cache
 * FUNCTION  : Destroy all cached text lines.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.02.95 13:38
 * LAST      : 01.02.95 14:29
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Destroy_text_cache(void)
{
	UNSHORT i;

	/* Is text caching on ? */
	if (Text_cache_flag)
	{
		/* Yes -> Destroy all cached text lines */
		for (i=0;i<MAX_TEXT_CACHE;i++)
		{
			Destroy_cached_text_line(&(Text_cache[i]));
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_cached_text_line
 * FUNCTION  : Find a text line among the cached text lines.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.02.95 14:37
 * LAST      : 01.02.95 14:37
 * INPUTS    : UNSHORT Line_nr - Line number.
 * RESULT    : struct Cached_text_line * : Pointer to cache data / NULL.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Cached_text_line *
Find_cached_text_line(UNSHORT Line_nr)
{
	UNSHORT i;

	/* Search cache */
	for (i=0;i<MAX_TEXT_CACHE;i++)
	{
		/* Is this the desired line ? */
		if (Text_cache[i].Line_nr == Line_nr)
		{
			/* Yes -> Return pointer to cache data */
			return &(Text_cache[i]);
		}
	}

	/* Not found */
	return NULL;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_cached_string
 * FUNCTION  : Print a cached text line.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.02.95 14:40
 * LAST      : 01.02.95 14:40
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate in OPM.
 *             SISHORT Y - Top Y-coordinate in OPM.
 *             struct Cached_text_line *Cached - Pointer to cache data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_cached_string(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Cached_text_line *Cached)
{
	UNBYTE *Ptr;

	Ptr = MEM_Claim_pointer(Cached->Line_handle);
	Put_masked_block(OPM, X, Y, Cached->Width, Cached->Height, Ptr);
	MEM_Free_pointer(Cached->Line_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_free_text_line_cache
 * FUNCTION  : Find a free entry among the cached text lines.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.02.95 14:52
 * LAST      : 01.02.95 14:52
 * INPUTS    : None.
 * RESULT    : struct Cached_text_line * : Pointer to cache data / NULL.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Cached_text_line *
Find_free_text_line_cache(void)
{
	UNSHORT i;

	/* Search cache */
	for (i=0;i<MAX_TEXT_CACHE;i++)
	{
		/* Is this entry free ? */
		if (Text_cache[i].Line_nr == 0xFFFF)
		{
			/* Yes -> Return pointer to cache data */
			return &(Text_cache[i]);
		}
	}

	/* Search cache again, deleting low-priority entries */
	for (i=0;i<MAX_TEXT_CACHE;i++)
	{
		/* Does this entry have priority zero ? */
		if (!Text_cache[i].Priority)
		{
			/* Yes -> Delete this entry */
			Destroy_cached_text_line(&(Text_cache[i]));

			/* Return pointer to cache data */
			return &(Text_cache[i]);
		}
	}

	/* Nothing found */
	return NULL;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_text_cache_priorities
 * FUNCTION  : Update the priorities of the text cache entries.
 * FILE      : TEXTMNG.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.02.95 14:54
 * LAST      : 01.02.95 14:54
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_text_cache_priorities(void)
{
	UNSHORT i;

	/* Is text caching on ? */
	if (Text_cache_flag)
	{
		/* Yes -> Search cache */
		for (i=0;i<MAX_TEXT_CACHE;i++)
		{
			/* Is this entry occupied ? */
			if (Text_cache[i].Line_nr != 0xFFFF)
			{
				/* Yes -> Is the priority zero ? */
				if (Text_cache[i].Priority)
				{
					/* No -> Decrease priority */
					Text_cache[i].Priority--;
				}
			}
		}
	}
}

