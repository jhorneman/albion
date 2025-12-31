/************
 * NAME     : TEXTWIN.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : TEXTWIN.H, USERFACE.C
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <GAMETEXT.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <TEXTWIN.H>
#include <COLOURS.H>

/* defines */

/* Text window parameters */
#define TEXT_WINDOW_X			(20)
#define TEXT_WINDOW_Y			(0)
#define TEXT_WINDOW_WIDTH		(320)
#define TEXT_WINDOW_HEIGHT		(182)

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_text_file_window
 * FUNCTION  : Draw and handle a text window containing a text block from
 *              a text file.
 * FILE      : TEXTWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.02.95 12:59
 * LAST      : 27.06.95 21:50
 * INPUTS    : MEM_HANDLE Text_file_handle - Memory handle of text file.
 *             UNSHORT Text_block_nr - Text block number.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_text_file_window(MEM_HANDLE Text_file_handle, UNSHORT Text_block_nr)
{
	UNCHAR *Text_ptr;

	/* Get text file address */
	Text_ptr = (UNCHAR *) MEM_Claim_pointer(Text_file_handle);

	/* Find text block */
	Text_ptr = Find_text_block(Text_ptr, Text_block_nr);

	/* Do text window */
	Do_text_window(Text_ptr);

	MEM_Free_pointer(Text_file_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_text_file_window_with_symbol
 * FUNCTION  : Draw and handle a text window containing a text block from
 *              a text file, with a symbol.
 * FILE      : TEXTWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.95 20:29
 * LAST      : 23.07.95 20:29
 * INPUTS    : MEM_HANDLE Text_file_handle - Memory handle of text file.
 *             UNSHORT Text_block_nr - Text block number.
 *             UNSHORT Width - Width of symbol.
 *             UNSHORT Height - Height of symbol.
 *             MEM_HANDLE Graphics_handle - Handle of graphics / NULL.
 *             UNLONG Graphics_offset - Offset to graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_text_file_window_with_symbol(MEM_HANDLE Text_file_handle,
 UNSHORT Text_block_nr, UNSHORT Width, UNSHORT Height,
 MEM_HANDLE Graphics_handle, UNLONG Graphics_offset)
{
	UNCHAR *Text_ptr;

	/* Get text file address */
	Text_ptr = (UNCHAR *) MEM_Claim_pointer(Text_file_handle);

	/* Find text block */
	Text_ptr = Find_text_block(Text_ptr, Text_block_nr);

	/* Do text window */
	Do_text_window_with_symbol
	(
		Text_ptr,
		Width,
		Height,
		Graphics_handle,
		Graphics_offset
	);

	MEM_Free_pointer(Text_file_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_text_window
 * FUNCTION  : Draw and handle a text window.
 * FILE      : TEXTWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.02.95 12:59
 * LAST      : 25.10.95 13:10
 * INPUTS    : UNBYTE *Text_ptr - Pointer to text.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_text_window(UNBYTE *Text_ptr)
{
	struct Processed_text Processed;
	struct PA PA;
	struct BBRECT MA;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	UNSHORT Window_Y;
	UNSHORT Window_height;
	UNBYTE *Ptr;

	/* Install text-style */
	Push_textstyle(&Default_text_style);

	/* Create PA */
	PA.Area.top		= 0;
	PA.Area.left	= TEXT_WINDOW_X + 10;
	PA.Area.width	= TEXT_WINDOW_WIDTH - 20;
	PA.Area.height	= TEXT_WINDOW_HEIGHT - 20;

	/* Process the text block */
	Push_PA(&PA);
	Process_text(Text_ptr, &Processed);
	Pop_PA();

	/* Is the text too large for the window ? */
	Window_height = TEXT_WINDOW_HEIGHT;
	if ((SILONG) Processed.Text_height < (SILONG)(Window_height - 20))
	{
		/* No -> Adjust window height */
		Window_height = Processed.Text_height + 20;
	}

	/* Is the window too small ? */
	if (Window_height < 64)
	{
		/* Yes -> Set to minimum height */
		Window_height = 64;
	}

	/* Centre window on screen */
//	Window_Y = TEXT_WINDOW_Y + (TEXT_WINDOW_HEIGHT - Window_height) / 2;
	Window_Y = TEXT_WINDOW_Y;

	/* Adjust PA */
	if ((SILONG) (Window_height - 20) >= (SILONG) Processed.Text_height)
	{
		PA.Area.top = Window_Y + ((Window_height - 20) -
		 Processed.Text_height) / 2 + 10;
	}
	else
	{
		PA.Area.top = Window_Y + 10;
	}
	PA.Area.height = Window_height - 20;

	/* Create MA */
	MA.top		= Window_Y;
	MA.left		= TEXT_WINDOW_X;
	MA.width		= TEXT_WINDOW_WIDTH;
	MA.height	= Window_height;

	/* Install stuff */
	Push_module(&Window_Mod);
	Push_MA(&MA);
	Push_PA(&PA);

	/* Make background buffer and OPM */
	Background_handle = MEM_Do_allocate
	(
		TEXT_WINDOW_WIDTH * Window_height,
		0,
		&Background_buffer_ftype
	);

	Ptr = MEM_Claim_pointer(Background_handle);

	OPM_New
	(
		TEXT_WINDOW_WIDTH,
		Window_height,
		1,
		&Background_OPM,
		Ptr
	);

	MEM_Free_pointer(Background_handle);

	/* Save background */
	OPM_CopyOPMOPM
	(
		&Main_OPM,
		&Background_OPM,
		TEXT_WINDOW_X,
		Window_Y,
		TEXT_WINDOW_WIDTH,
		Window_height,
		0,
		0
	);

	/* Draw window's shadow */
	Put_recoloured_box
	(
		&Main_OPM,
		TEXT_WINDOW_X + 10,
		Window_Y + Window_height - 5,
		TEXT_WINDOW_WIDTH - 10,
		5,
		&(Recolour_tables[0][0])
	);
	Put_recoloured_box
	(
		&Main_OPM,
		TEXT_WINDOW_X + TEXT_WINDOW_WIDTH - 5,
		Window_Y + 10,
		5,
		Window_height - 15,
		&(Recolour_tables[0][0])
	);

	/* Draw window */
	Put_recoloured_box
	(
		&Main_OPM,
		TEXT_WINDOW_X + 7,
		Window_Y + 7,
		TEXT_WINDOW_WIDTH - 14,
		Window_height - 14,
		&(Recolour_tables[0][0])
	);
	Draw_window_border
	(
		&Main_OPM,
		TEXT_WINDOW_X,
		Window_Y,
		TEXT_WINDOW_WIDTH,
		Window_height
	);

	/* Print text */
	Display_processed_text
	(
		&Main_OPM,
		&Processed,
		WAIT_TEXT_DISPLAY_FLAG | DELAY_TEXT_DISPLAY_FLAG
	);

	Destroy_processed_text(&Processed);

	/* Restore background */
	OPM_CopyOPMOPM
	(
		&Background_OPM,
		&Main_OPM,
		0,
		0,
		TEXT_WINDOW_WIDTH,
		Window_height,
		TEXT_WINDOW_X,
		Window_Y
	);

	/* Destroy background buffer and OPM */
	OPM_Del(&Background_OPM);
	MEM_Free_memory(Background_handle);

	/* Remove stuff */
	Pop_PA();
	Pop_MA();
	Pop_module();

	/* Remove text-style */
	Pop_textstyle();

	Wait_4_unclick();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_text_window_with_symbol
 * FUNCTION  : Draw and handle a text window with a symbol.
 * FILE      : TEXTWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.95 16:06
 * LAST      : 25.10.95 13:10
 * INPUTS    : UNBYTE *Text_ptr - Pointer to text.
 *             UNSHORT Width - Width of symbol.
 *             UNSHORT Height - Height of symbol.
 *             MEM_HANDLE Graphics_handle - Handle of graphics / NULL.
 *             UNLONG Graphics_offset - Offset to graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_text_window_with_symbol(UNBYTE *Text_ptr, UNSHORT Width, UNSHORT Height,
 MEM_HANDLE Graphics_handle, UNLONG Graphics_offset)
{
	struct Processed_text Processed;
	struct PA PA;
	struct BBRECT MA;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	UNSHORT Symbol_Y;
	UNSHORT Window_Y;
	UNSHORT Window_height;
	UNBYTE *Ptr;

	/* Install text-style */
	Push_textstyle(&Default_text_style);

	/* Create PA */
	PA.Area.top		= 0;
	PA.Area.left	= TEXT_WINDOW_X + Width + 10 + 6;
	PA.Area.width	= TEXT_WINDOW_WIDTH - Width - 20 - 6;
	PA.Area.height	= TEXT_WINDOW_HEIGHT - 20;

	/* Process the text block */
	Push_PA(&PA);
	Process_text(Text_ptr, &Processed);
	Pop_PA();

	/* Is the text too large for the window ? */
	Window_height = TEXT_WINDOW_HEIGHT;
	if ((SILONG) Processed.Text_height < (SILONG)(Window_height - 20))
	{
		/* No -> Adjust window height */
		Window_height = Processed.Text_height + 20;
	}

	/* Is the window too small ? */
	if (Window_height < 64)
	{
		/* Yes -> Set to minimum height */
		Window_height = 64;
	}

	/* Centre window on screen */
//	Window_Y = TEXT_WINDOW_Y + (TEXT_WINDOW_HEIGHT - Window_height) / 2;
	Window_Y = TEXT_WINDOW_Y;

	/* Adjust PA */
	if ((SILONG) (Window_height - 20) >= (SILONG) Processed.Text_height)
	{
		PA.Area.top = Window_Y + ((Window_height - 20) -
		 Processed.Text_height) / 2 + 10;
	}
	else
	{
		PA.Area.top = Window_Y + 10;
	}
	PA.Area.height = Window_height - 20;

	/* Create MA */
	MA.top		= Window_Y;
	MA.left		= TEXT_WINDOW_X;
	MA.width		= TEXT_WINDOW_WIDTH;
	MA.height	= Window_height;

	/* Install stuff */
	Push_module(&Window_Mod);
	Push_MA(&MA);
	Push_PA(&PA);

	/* Make background buffer and OPM */
	Background_handle = MEM_Do_allocate
	(
		TEXT_WINDOW_WIDTH * Window_height,
		0,
		&Background_buffer_ftype
	);

	Ptr = MEM_Claim_pointer(Background_handle);

	OPM_New
	(
		TEXT_WINDOW_WIDTH,
		Window_height,
		1,
		&Background_OPM,
		Ptr
	);

	MEM_Free_pointer(Background_handle);

	/* Save background */
	OPM_CopyOPMOPM
	(
		&Main_OPM,
		&Background_OPM,
		TEXT_WINDOW_X,
		Window_Y,
		TEXT_WINDOW_WIDTH,
		Window_height,
		0,
		0
	);

	/* Draw window's shadow */
	Put_recoloured_box
	(
		&Main_OPM,
		TEXT_WINDOW_X + 10,
		Window_Y + Window_height - 5,
		TEXT_WINDOW_WIDTH - 10, 5,
		&(Recolour_tables[0][0])
	);
	Put_recoloured_box
	(
		&Main_OPM,
		TEXT_WINDOW_X + TEXT_WINDOW_WIDTH - 5,
		Window_Y + 10,
		5,
		Window_height - 15,
		&(Recolour_tables[0][0])
	);

	/* Draw window */
	Put_recoloured_box
	(
		&Main_OPM,
		TEXT_WINDOW_X + 7,
		Window_Y + 7,
		TEXT_WINDOW_WIDTH - 14,
		Window_height - 14,
		&(Recolour_tables[0][0])
	);
	Draw_window_border
	(
		&Main_OPM,
		TEXT_WINDOW_X,
		Window_Y,
		TEXT_WINDOW_WIDTH,
		Window_height
	);

	/* Calculate symbol's Y-coordinate */
	Symbol_Y = Window_Y  + ((Window_height - 20) - Height) / 2 + 10;

	/* Draw box around symbol */
	Draw_deep_box
	(
		&Main_OPM,
		TEXT_WINDOW_X + 9 + 3,
		Symbol_Y - 1,
		Width + 2,
		Height + 2
	);

	/* Get graphics address */
	Ptr = MEM_Claim_pointer(Graphics_handle) + Graphics_offset;

	/* Draw symbol */
	Put_masked_block
	(
		&Main_OPM,
		TEXT_WINDOW_X + 10 + 3,
		Symbol_Y,
		Width,
		Height,
		Ptr
	);

	MEM_Free_pointer(Graphics_handle);

	/* Print text */
	Display_processed_text
	(
		&Main_OPM,
		&Processed,
		WAIT_TEXT_DISPLAY_FLAG | DELAY_TEXT_DISPLAY_FLAG
	);

	Destroy_processed_text(&Processed);

	/* Restore background */
	OPM_CopyOPMOPM
	(
		&Background_OPM,
		&Main_OPM,
		0,
		0,
		TEXT_WINDOW_WIDTH,
		Window_height,
		TEXT_WINDOW_X,
		Window_Y
	);

	/* Destroy background buffer and OPM */
	OPM_Del(&Background_OPM);
	MEM_Free_memory(Background_handle);

	/* Remove stuff */
	Pop_PA();
	Pop_MA();
	Pop_module();

	/* Remove text-style */
	Pop_textstyle();

	Wait_4_unclick();
}

