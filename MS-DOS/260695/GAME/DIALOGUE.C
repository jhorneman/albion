/************
 * NAME     : DIALOGUE.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : DIALOGUE.H
 ************/

/* includes */

#include <stdlib.h>
#include <string.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <GAMETEXT.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <DIALOGUE.H>
#include <EVELOGIC.H>
#include <INPUT.H>
#include <STATAREA.H>
#include <XFTYPES.H>
#include <ITMLOGIC.H>

/* defines */

#define NR_DICTIONARIES		(3)

#define MAX_MC_ENTRIES		(10)

#define MC_ENTRY_WIDTH		(280)

/* Dialogue window parameters */
#define DIALOGUE_WINDOW_X			(49)
#define DIALOGUE_WINDOW_Y			(19)
#define DIALOGUE_WINDOW_WIDTH		(262)
#define DIALOGUE_WINDOW_HEIGHT	(Panel_Y - DIALOGUE_WINDOW_Y)

/* Multiple-choice entry types */
#define EMPTY_MC_ENTRY_TYPE		(0xFFFF)
#define NORMAL_MC_ENTRY_TYPE		(0)
#define FAQ_MC_ENTRY_TYPE			(1)
#define DEFAULT_MC_ENTRY_TYPE		(2)

#define FIRST_NORMAL_MC_ENTRY		(10)
#define NR_NORMAL_MC_ENTRIES		(FIRST_NORMAL_MC_ENTRY + 10)
#define NR_FAQ_MC_ENTRIES			(1)
#define NR_DEFAULT_MC_ENTRIES		(5)

/* Default multiple-choice entries */
#define WORD_DEF_MC_ENTRY				(0)
#define GIVE_ITEM_DEF_MC_ENTRY		(1)
#define JOIN_PARTY_DEF_MC_ENTRY		(2)
#define LEAVE_PARTY_DEF_MC_ENTRY		(3)
#define END_DIALOGUE_DEF_MC_ENTRY	(4)

/* Dialogue error codes */
#define DIALOGUE_ERR_ILLEGAL_DEFAULT_MC_INDEX	(1)

#define DIALOGUE_ERR_MAX	(1)

/* structure definitions */

/* Multiple-choice entry */
struct MC_entry {
	UNSHORT Type;			/* See above */
	UNSHORT Index;
	MEM_HANDLE Text_handle;
	UNLONG Text_offset;
};

/* Dialogue portrait OID */
struct Dialogue_portrait_OID {
	MEM_HANDLE Char_handle;
	MEM_HANDLE Portrait_gfx_handle;
	BOOLEAN Mirror_flag;
};

/* Dialogue portrait object */
struct Dialogue_portrait_object {
	struct Object Object;
	MEM_HANDLE Char_handle;
	MEM_HANDLE Portrait_gfx_handle;
	BOOLEAN Mirror_flag;
};

/* Multiple-choice window OID */
struct MC_window_OID {
	UNSHORT Nr_entries;
	struct MC_entry *Entry_list;

	UNSHORT *Result_ptr;
};

/* Multiple-choice window object */
struct MC_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNSHORT *Result_ptr;
};

/* Multiple-choice entry OID */
struct MC_entry_OID {
	struct MC_entry Data;
	UNSHORT Index;
};

/* Multiple-choice entry object */
struct MC_entry_object {
	struct Object Object;
	struct MC_entry Data;
	UNSHORT Index;
};

/* prototypes */

/* Dialoge module functions */
void Dialogue_ModInit(void);
void Dialogue_ModExit(void);
void Dialogue_DisInit(void);
void Dialogue_DisExit(void);
void Dialogue_MainLoop(void);

/* Multiple-choice entry handlers */
void Handle_normal_MC_entry(UNSHORT Text_block_nr, UNSHORT MC_index);
void Handle_FAQ_MC_entry(UNSHORT MC_index);
void Handle_default_MC_entry(UNSHORT MC_index);

/* Multiple-choice list management functions */
UNSHORT Add_normal_MC_entries(struct MC_entry *MC_list, UNSHORT Nr_MC_entries,
 struct Dialogue_node *Node_ptr);
UNSHORT Add_some_default_MC_entries(struct MC_entry *MC_list,
 UNSHORT Nr_MC_entries, struct Dialogue_node *Node_ptr);
UNSHORT Add_FAQ_MC_entries(struct MC_entry *MC_list, UNSHORT Nr_MC_entries);
UNSHORT Add_default_MC_entries(struct MC_entry *MC_list,
 UNSHORT Nr_MC_entries);

UNSHORT Select_multiple_choice(UNSHORT Nr_entries,
 struct MC_entry *Entry_list);

/* Multiple-choice window object methods */
UNLONG Init_MC_window_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_MC_window_object(struct Object *Object, union Method_parms *P);

/* Multiple-choice entry object methods */
UNLONG Init_MC_entry_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_MC_entry_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_MC_entry_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_MC_entry_object(struct Object *Object, union Method_parms *P);
UNLONG Left_MC_entry_object(struct Object *Object, union Method_parms *P);

/* Multiple-choice entry support functions */
void Draw_MC_entry(struct MC_entry_object *MC_entry);

/* Dialogue error handling functions */
void Dialogue_error(UNSHORT Error_code);
void Dialogue_print_error(UNCHAR *buffer, UNBYTE *data);

/* Dictionary support functions */

/* global variables */

static struct Module Dialogue_Mod = {
	0, SCREEN_MOD, DIALOGUE_SCREEN,
	Dialogue_MainLoop,
	Dialogue_ModInit,
	Dialogue_ModExit,
	Dialogue_DisInit,
	Dialogue_DisExit,
	NULL
};

BOOLEAN In_Dialogue = FALSE;
BOOLEAN End_dialogue_flag;
BOOLEAN Dialogue_partner_in_party;

MEM_HANDLE Dialogue_char_handle;
MEM_HANDLE Dialogue_small_portrait_handle;

MEM_HANDLE Dialogue_event_set_handles[2];
MEM_HANDLE Dialogue_event_text_handles[2];

UNBYTE New_words_bit_array[(WORDS_MAX+7)/8];

struct Dialogue_node Current_dialogue_node;

static UNSHORT Dialogue_object;

/* Dialogue parameters */
UNSHORT Dialogue_type;
UNSHORT Dialogue_char_type;
UNSHORT Dialogue_char_index;
static UNSHORT Dialogue_party_member_index;

/* Dialogue screen method list */
static struct Method Dialogue_methods[] = {
	{ UPDATE_METHOD, Update_Status_area },
	{ 0, NULL}
};

/* Dialogue screen class description */
static struct Object_class Dialogue_Class = {
	0, sizeof(struct Object),
	&Dialogue_methods[0]
};

/* Multiple-choice window method list */
static struct Method MC_window_methods[] = {
	{ INIT_METHOD, Init_MC_window_object },
	{ EXIT_METHOD, Exit_Window_object },
	{ DRAW_METHOD, Draw_MC_window_object },
	{ UPDATE_METHOD, Update_Status_area },
	{ TOUCHED_METHOD, Dehighlight },
	{ RESTORE_METHOD, Restore_window },
	{ 0, NULL}
};

/* Multiple-choice window class description */
static struct Object_class MC_window_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct MC_window_object),
	&MC_window_methods[0]
};

/* Multiple-choice entry method list */
static struct Method MC_entry_methods[] = {
	{ INIT_METHOD, Init_MC_entry_object },
	{ DRAW_METHOD, Draw_MC_entry_object },
	{ FEEDBACK_METHOD, Feedback_MC_entry_object },
	{ HIGHLIGHT_METHOD, Highlight_MC_entry_object },
	{ LEFT_METHOD, Left_MC_entry_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

/* Multiple-choice entry class description */
static struct Object_class MC_entry_Class = {
	0, sizeof(struct MC_entry_object),
	&MC_entry_methods[0]
};

/* Error messages */
static UNCHAR Dialogue_library_name[] = "Dialogue";

static UNCHAR *Dialogue_error_strings[] = {
	"Illegal error code.",
	"Illegal default multiple-choice entry index."
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_Dialogue
 * FUNCTION  : Start a dialogue.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.04.95 10:49
 * LAST      : 21.06.95 12:20
 * INPUTS    : UNSHORT Type - Dialogue type.
 *             UNSHORT Char_type - Character type.
 *             UNSHORT Char_index - Character index.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_Dialogue(UNSHORT Type, UNSHORT Char_type, UNSHORT Char_index)
{
	/* Store dialogue parameters */
	Dialogue_type = Dialogue_type;
	Dialogue_char_type = Char_type;
	Dialogue_char_index = Char_index;

	/* Enter dialogue screen */
	Push_module(&Dialogue_Mod);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dialogue_ModInit
 * FUNCTION  : Initialize Dialogue module.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.06.95 12:31
 * LAST      : 23.06.95 11:32
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dialogue_ModInit(void)
{
	static struct PA PA;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Clear flag */
	End_dialogue_flag = FALSE;

	/* Are we talking with a party character ? */
	Dialogue_party_member_index = 0xFFFF;
	if (Dialogue_char_type == PARTY_CHAR_TYPE)
	{
		/* Yes -> Check if this character is in the party */
		Dialogue_partner_in_party = Character_in_party(Dialogue_char_index);

		if (Dialogue_partner_in_party)
		{
			/* In party -> Determine party member index */
			for (i=0;i<6;i++)
			{
				if (PARTY_DATA.Member_nrs[i] == Dialogue_char_index)
				{
					Dialogue_party_member_index = i;
					break;
				}
			}
		}
	}

	/* Clear new words array */
	BASEMEM_FillMemByte(New_words_bit_array, (WORDS_MAX+7)/8, 0);

	/* Talking with someone in the party ? */
	if (Dialogue_partner_in_party)
	{
		/* Yes -> Set handles */
		Dialogue_char_handle = Party_char_handles[Dialogue_party_member_index];

		Dialogue_small_portrait_handle =
		 Small_portrait_handles[Dialogue_party_member_index];

		Dialogue_event_set_handles[0] =
		 Party_event_set_handles[Dialogue_party_member_index][0];
		Dialogue_event_set_handles[1] =
		 Party_event_set_handles[Dialogue_party_member_index][1];

		Dialogue_event_text_handles[0] =
		 Party_event_text_handles[Dialogue_party_member_index][0];
		Dialogue_event_text_handles[1] =
		 Party_event_text_handles[Dialogue_party_member_index][1];
	}
	else
	{
		struct Character_data *Char;
		UNSHORT Char_file_type;

		/* No -> Determine character data type */
		switch (Dialogue_char_type)
		{
			case PARTY_CHAR_TYPE:
			{
				Char_file_type = PARTY_CHAR;
				break;
			}
			case NPC_CHAR_TYPE:
			{
				Char_file_type = NPC_CHAR;
				break;
			}
			case MONSTER_CHAR_TYPE:
			{
				Char_file_type = MONSTER_CHAR;
				break;
			}
		}

		/* Load character data */
		Dialogue_char_handle =
		 Load_subfile(Char_file_type, Dialogue_char_index);

		/* Get pointer to character data */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Dialogue_char_handle);

		/* Load portrait */
		Dialogue_small_portrait_handle = Load_subfile(SMALL_PORTRAIT,
		 (UNSHORT) Char->Portrait_nr);

		/* First event set present ? */
		if (Char->Event_set_1_nr)
		{
			/* Yes -> Load first event set */
			Dialogue_event_set_handles[0] = Load_subfile(EVENT_SET,
			 (UNSHORT) Char->Event_set_1_nr);

			/* Load first event set texts */
			Dialogue_event_text_handles[0] = Load_subfile(EVENT_TEXT,
			 (UNSHORT) Char->Event_set_1_nr);
		}
		else
		{
			/* No -> Clear handles */
			Dialogue_event_set_handles[0] = NULL;
			Dialogue_event_text_handles[0] = NULL;
		}

		/* Second event set present ? */
		if (Char->Event_set_2_nr)
		{
			/* Yes -> Load second event set */
			Dialogue_event_set_handles[1] = Load_subfile(EVENT_SET,
			 (UNSHORT) Char->Event_set_2_nr);

			/* Load second event set texts */
			Dialogue_event_text_handles[1] = Load_subfile(EVENT_TEXT,
			 (UNSHORT) Char->Event_set_2_nr);
		}
		else
		{
			/* No -> Clear handles */
			Dialogue_event_set_handles[1] = NULL;
			Dialogue_event_text_handles[1] = NULL;
		}

		MEM_Free_pointer(Dialogue_char_handle);
	}

	/* Initialize dictionary */
	Init_dictionary();

	/* Current screen is 2D map ? */
	if (Current_screen_type(1) == MAP_2D_SCREEN)
	{
		/* Yes -> Prepare screen */
		Current_2D_OPM = &Main_OPM;
		Draw_2D_scroll_buffer(MAP_2D_X, MAP_2D_Y);
		Current_2D_OPM = NULL;
	}

	/* Build Print Area */
	PA.Area.left = DIALOGUE_WINDOW_X + 10;
	PA.Area.top = DIALOGUE_WINDOW_Y + 10;
	PA.Area.width = DIALOGUE_WINDOW_WIDTH - 20;
	PA.Area.height = DIALOGUE_WINDOW_HEIGHT - 20;

	/* Install stuff */
	Push_PA(&PA);
	Push_textstyle(&Default_text_style);
	Push_root(&Main_OPM);
	Push_mouse(&(Mouse_pointers[DEFAULT_MPTR]));

	/* Update OPM */
	Add_update_OPM(&Status_area_OPM);

	/* In dialogue */
	In_Dialogue = TRUE;

	/* Initialize display */
	Init_display();


/*
	moveq.l	#Init_DIAC,d0		; Start
	jsr	DIA_event_handler
	bpl.s	.Exit			; Anything ?
	tst.b	NPC_or_member		; Member ?
	bne.s	.Member
	move.w	#117,d0			; Yo!
	bra.s	.Do
.Member:	move.w	#119,d0			; OK
.Do:	jsr	Do_prompt
.Exit:	rts
*/


}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dialogue_ModExit
 * FUNCTION  : Exit Dialogue module.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.06.95 12:31
 * LAST      : 23.06.95 11:32
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dialogue_ModExit(void)
{
	/* Remove update OPM */
	Remove_update_OPM(&Status_area_OPM);

	/* Remove stuff */
	Pop_mouse();
	Pop_root();
	Pop_textstyle();
	Pop_PA();

	/* No longer in dialogue */
	In_Dialogue = FALSE;

	/* Exit dictionary */
	Exit_dictionary();

	/* Talking with someone in the party ? */
	if (!Dialogue_partner_in_party)
	{
		/* No -> Free data */
		MEM_Free_memory(Dialogue_char_handle);
		MEM_Free_memory(Dialogue_small_portrait_handle);

		/* First event set present ? */
		if (Dialogue_event_set_handles[0])
		{
			/* Yes -> Free memory */
			MEM_Free_memory(Dialogue_event_set_handles[0]);
			MEM_Free_memory(Dialogue_event_text_handles[0]);
		}

		/* Second event set present ? */
		if (Dialogue_event_set_handles[1])
		{
			/* Yes -> Free memory */
			MEM_Free_memory(Dialogue_event_set_handles[1]);
			MEM_Free_memory(Dialogue_event_text_handles[1]);
		}
	}

	/* Current screen is 2D map ? */
	if (Current_screen_type(0) == MAP_2D_SCREEN)
	{
		/* Yes -> Prepare screen */
		Draw_2D_scroll_buffer(MAP_2D_X, MAP_2D_Y);
		Switch_screens();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dialogue_DisInit
 * FUNCTION  : Initialize Dialogue display.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 17:35
 * LAST      : 23.06.95 13:33
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dialogue_DisInit(void)
{
	UNBYTE *Ptr;
	UNCHAR Name[CHAR_NAME_LENGTH + 1];

	/* Draw window's shadow */
	Put_recoloured_box(&Main_OPM, DIALOGUE_WINDOW_X + 10,
	 DIALOGUE_WINDOW_Y + DIALOGUE_WINDOW_HEIGHT - 5, DIALOGUE_WINDOW_WIDTH -
	 10, 5, &(Recolour_tables[0][0]));
	Put_recoloured_box(&Main_OPM, DIALOGUE_WINDOW_X +
	 DIALOGUE_WINDOW_WIDTH - 5, DIALOGUE_WINDOW_Y + 10, 5,
	 DIALOGUE_WINDOW_HEIGHT - 15, &(Recolour_tables[0][0]));

	/* Draw window */
	Put_recoloured_box(&Main_OPM, DIALOGUE_WINDOW_X + 7, DIALOGUE_WINDOW_Y + 7,
	 DIALOGUE_WINDOW_WIDTH - 14, DIALOGUE_WINDOW_HEIGHT - 14,
	 &(Recolour_tables[1][0]));
	Draw_window_border(&Main_OPM, DIALOGUE_WINDOW_X, DIALOGUE_WINDOW_Y,
	 DIALOGUE_WINDOW_WIDTH, DIALOGUE_WINDOW_HEIGHT);

	/* Add dialogue object */
	Dialogue_object = Add_object(0, &Dialogue_Class, NULL, 0, 0, Screen_width,
	 Panel_Y);

	/* Draw left portrait's shadow */
	Put_recoloured_box(&Main_OPM, 10 - 1 + 4, 5 - 1 + 12, 36, 39 + 4 - 12,
	 &(Recolour_tables[0][0]));

	/* Erase left portrait area */
	Draw_window_inside(&Main_OPM, 10 - 1, 5 - 1, 36, 39);

	/* Draw box around portrait */
	Draw_high_border(&Main_OPM, 10 - 1, 5 - 1, 36, 39);

	/* Display left portrait */
	Ptr = MEM_Claim_pointer(Small_portrait_handles[PARTY_DATA.Active_member
	 - 1]);
	Put_masked_block(&Main_OPM, 10, 5, 34, 37, Ptr);
	MEM_Free_pointer(Small_portrait_handles[PARTY_DATA.Active_member - 1]);

	/* Draw right portrait's shadow */
	Put_recoloured_box(&Main_OPM, 360 - 36 - 8 - 1 + 4, 5 - 1 + 4, 36, 39,
	 &(Recolour_tables[0][0]));

	/* Erase right portrait area */
	Draw_window_inside(&Main_OPM, 360 - 36 - 8 - 1, 5 - 1, 36, 39);

	/* Draw box around portrait */
	Draw_high_border(&Main_OPM, 360 - 36 - 8 - 1, 5 - 1, 36, 39);

	/* Display right portrait */
	Ptr = MEM_Claim_pointer(Dialogue_small_portrait_handle);
	Put_masked_X_mirrored_block(&Main_OPM, 360 - 36 - 8, 5, 34, 37, Ptr);
	MEM_Free_pointer(Dialogue_small_portrait_handle);

	/* Draw shadow of left character's name area */
	Put_recoloured_box(&Main_OPM, 45 + 4, 5 - 1 + 4, 80 - 1, 12,
	 &(Recolour_tables[0][0]));

	/* Draw main part of left character's name area */
	Draw_window_inside(&Main_OPM, 44, 5 - 1, 80, 12);

	/* Draw edges of left character's name area */
	Put_recoloured_box(&Main_OPM, 44, 5 - 1, 80 - 1, 1,
	 &(Recolour_tables[7][0]));
	Put_recoloured_box(&Main_OPM, 44, 5 - 1 + 12 - 1, 80, 1,
	 &(Recolour_tables[1][0]));
	Put_recoloured_box(&Main_OPM, 44 + 80 - 1, 5, 1, 10,
	 &(Recolour_tables[1][0]));

	/* Get left character's name */
	Get_char_name(Active_char_handle, Name);

	/* Print left character's name */
	Print_string(&Main_OPM, 46, 6, Name);

	/* Draw shadow of right character's name area */
	Put_recoloured_box(&Main_OPM, 360 - 36 - 8 - 80 + 4, 5 - 1 + 4, 80 - 5, 12,
	 &(Recolour_tables[0][0]));

	/* Draw main part of right character's name area */
	Draw_window_inside(&Main_OPM, 360 - 36 - 8 - 80, 5 - 1, 80, 12);

	/* Draw edges of right character's name area */
	Put_recoloured_box(&Main_OPM, 360 - 36 - 8 - 80, 5 - 1, 80, 1,
	 &(Recolour_tables[7][0]));
	Put_recoloured_box(&Main_OPM, 360 - 36 - 8 - 80, 5 - 1 + 12 - 1, 80, 1,
	 &(Recolour_tables[1][0]));
	Put_recoloured_box(&Main_OPM, 360 - 36 - 8 - 80, 5, 1, 10,
	 &(Recolour_tables[7][0]));

	/* Get right character's name */
	Get_char_name(Dialogue_char_handle, Name);

	/* Print right character's name */
	Print_string(&Main_OPM, 360 - 36 - 8 - 2 - Get_line_width(Name), 6, Name);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dialogue_DisExit
 * FUNCTION  : Exit Dialogue display.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 17:36
 * LAST      : 23.06.95 13:33
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dialogue_DisExit(void)
{
	/* Remove dialogue objects */
	Delete_object(Dialogue_object);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dialogue_MainLoop
 * FUNCTION  : Main loop of Dialogue module.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 15:47
 * LAST      : 25.06.95 15:47
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dialogue_MainLoop(void)
{
	struct MC_entry Dialogue_MC_list[MAX_MC_ENTRIES];
	UNSHORT Nr_MC_entries;
	UNSHORT Selected_MC_entry;
	UNCHAR *Text_ptr;

	/* Clear number of multiple-choice entries */
	Nr_MC_entries = 0;

	/* Anything special in the current dialogue node ? */
	if (Current_dialogue_node.Text_handle)
	{
		/* Yes -> Get text file address */
		Text_ptr = (UNCHAR *)
		 MEM_Claim_pointer(Current_dialogue_node.Text_handle);

		/* Find text block */
		Text_ptr = Find_text_block(Text_ptr,
		 Current_dialogue_node.Text_block_nr);

		/* Display text */
		Display_text_and_wait(&Main_OPM, Text_ptr);

		MEM_Free_pointer(Current_dialogue_node.Text_handle);

		/* Add normal multiple-choice entries */
		Nr_MC_entries = Add_normal_MC_entries(Dialogue_MC_list, Nr_MC_entries,
		 &Current_dialogue_node);

		/* Add default multiple-choice entries ? */
		if (!Current_dialogue_node.Flags & DIALOGUE_NO_DEFAULT)
		{
			/* Yes -> Add FAQ entries */
			Nr_MC_entries = Add_FAQ_MC_entries(Dialogue_MC_list, Nr_MC_entries);

			/* Add default entries */
			Nr_MC_entries = Add_default_MC_entries(Dialogue_MC_list,
			 Nr_MC_entries);
		}
		else
		{
			/* No -> Add some default entries */
			Nr_MC_entries = Add_some_default_MC_entries(Dialogue_MC_list,
			 Nr_MC_entries, &Current_dialogue_node);
		}
	}
	else
	{
		/* No -> Add FAQ entries */
		Nr_MC_entries = Add_FAQ_MC_entries(Dialogue_MC_list, Nr_MC_entries);

		/* Add default entries */
		Nr_MC_entries = Add_default_MC_entries(Dialogue_MC_list, Nr_MC_entries);
	}

	/* Destroy current dialogue node */
	Current_dialogue_node.Text_handle = NULL;

	/* Select a multiple-choice entry */
	Selected_MC_entry = Select_multiple_choice(Nr_MC_entries,
	 Dialogue_MC_list);

	/* Act depending on multiple-choice entry type */
	switch (Dialogue_MC_list[Selected_MC_entry].Type)
 	{
		/* Normal entry */
		case NORMAL_MC_ENTRY_TYPE:
		{
			/* Handle it */
			Handle_normal_MC_entry(Current_dialogue_node.Text_block_nr,
			 Dialogue_MC_list[Selected_MC_entry].Index);

			break;
		}
		/* FAQ entry */
		case FAQ_MC_ENTRY_TYPE:
		{
			/* Handle it */
			Handle_FAQ_MC_entry(Dialogue_MC_list[Selected_MC_entry].Index);

			break;
		}
		/* Default entry */
		case DEFAULT_MC_ENTRY_TYPE:
		{
			/* Handle it */
			Handle_default_MC_entry(Dialogue_MC_list[Selected_MC_entry].Index);

			break;
		}
	}

	/* End of dialogue ? */
	if (End_dialogue_flag)
	{
		/* Yes -> Exit */
		Exit_display();
		Pop_module();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_normal_MC_entry
 * FUNCTION  : Handle a normal dialogue multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 17:08
 * LAST      : 25.06.95 17:41
 * INPUTS    : UNSHORT Text_block_nr - Text block number.
 *             UNSHORT MC_index - Multiple-choice entry index.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_normal_MC_entry(UNSHORT Text_block_nr, UNSHORT MC_index)
{
	static struct Event_action Select_answer_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		SELECT_ANSWER_ACTION, 0, 0,
		NULL, NULL, NULL
	};

	/* Build event action data */
	Select_answer_action.Actor_index = PARTY_DATA.Active_member;
	Select_answer_action.Action_value = Text_block_nr;
	Select_answer_action.Action_extra = MC_index;

	/* Perform action */
	Perform_action(&Select_answer_action);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_FAQ_MC_entry
 * FUNCTION  : Handle a FAQ dialogue multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 17:11
 * LAST      :
 * INPUTS    : UNSHORT MC_index - Multiple-choice entry index.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_FAQ_MC_entry(UNSHORT MC_index)
{

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_default_MC_entry
 * FUNCTION  : Handle a default dialogue multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 17:11
 * LAST      :
 * INPUTS    : UNSHORT MC_index - Multiple-choice entry index.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_default_MC_entry(UNSHORT MC_index)
{
	struct Event_action Default_dialogue_action;
	BOOLEAN Result;

	/* Clear event action data */
	BASEMEM_FillMemByte((UNBYTE *) &Default_dialogue_action,
	 sizeof(struct Event_action), 0);

	/* Prepare event action data */
	Default_dialogue_action.Actor_type = PARTY_ACTOR_TYPE;
	Default_dialogue_action.Actor_index = PARTY_DATA.Active_member;

	/* Act depending on entry index */
	switch (MC_index)
	{
		/* Ask about word */
		case WORD_DEF_MC_ENTRY:
		{

			/* Write action data */
			Default_dialogue_action.Action_type = ASK_ABOUT_ACTION;
			Default_dialogue_action.Action_value = 0;

			/* Perform action */
			Result = Perform_action(&Default_dialogue_action);

			/* Any reaction ? */
			if (!Result)
			{
				/* No -> Do default reaction */
				Display_text_and_wait(&Main_OPM, System_text_ptrs[497]);
			}
			break;
		}
		/* Give item */
		case GIVE_ITEM_DEF_MC_ENTRY:
		{
			struct Character_data *Char;
			struct Item_data *Item_data;
			struct Item_packet *Packet;
			UNSHORT Item_slot_index;
			UNSHORT Item_index;
			UNSHORT Item_type;

			/* Select an item */
			Item_slot_index = Select_character_item(Active_char_handle,
			 System_text_ptrs[496], NULL);

			/* Any item selected ? */
			if (Item_slot_index != 0xFFFF)
			{
				/* Yes -> Get selected item packet */
				Char = (struct Character_data *)
				 MEM_Claim_pointer(Active_char_handle);

				if (Item_slot_index <= ITEMS_ON_BODY)
				{
					Packet = &(Char->Body_items[Item_slot_index - 1]);
				}
				else
				{
					Packet = &(Char->Backpack_items[Item_slot_index -
					 ITEMS_ON_BODY - 1]);
				}

				/* Get item index */
				Item_index = Packet->Index;

				/* Get item type */
				Item_data = Get_item_data(Packet);
				Item_type = (UNSHORT) Item_data->Type;
				Free_item_data();

				MEM_Free_pointer(Active_char_handle);

				/* Write action data */
				Default_dialogue_action.Action_type = GIVE_ITEM_ACTION;
				Default_dialogue_action.Action_value = Item_index;
				Default_dialogue_action.Action_extra = Item_type;
				Default_dialogue_action.Action_ptr = NULL;
				Default_dialogue_action.Action_data = (void *) NULL;

				/* Perform action */
				Result = Perform_action(&Default_dialogue_action);

				/* Any reaction ? */
				if (!Result)
				{
					/* No -> Do default reaction */
					Display_text_and_wait(&Main_OPM, System_text_ptrs[499]);
				}
			}
			break;
		}
		/* Ask to join */
		case JOIN_PARTY_DEF_MC_ENTRY:
		{
			/* Write action data */
			Default_dialogue_action.Action_type = ASK_TO_JOIN_ACTION;
			Default_dialogue_action.Action_ptr = NULL;
			Default_dialogue_action.Action_data = (void *) NULL;

			/* Forbidden by default! */
//			Default_dialogue_action

			/* Perform action */
			Result = Perform_action(&Default_dialogue_action);

			/* Any reaction ? */
			if (!Result)
			{
				/* No -> Do default reaction */
				Display_text_and_wait(&Main_OPM, System_text_ptrs[501]);
			}
			break;
		}
		/* Ask to leave */
		case LEAVE_PARTY_DEF_MC_ENTRY:
		{
			/* Write action data */
			Default_dialogue_action.Action_type = ASK_TO_LEAVE_ACTION;
			Default_dialogue_action.Action_ptr = NULL;
			Default_dialogue_action.Action_data = (void *) NULL;

			/* Perform action */
			Result = Perform_action(&Default_dialogue_action);

			/* Any reaction ? */
			if (!Result)
			{
				/* No -> Do default reaction */
				Display_text_and_wait(&Main_OPM, System_text_ptrs[502]);
			}
			break;
		}
		/* End dialogue */
		case END_DIALOGUE_DEF_MC_ENTRY:
		{
			/* Write action data */
			Default_dialogue_action.Action_type = DIALOGUE_END_ACTION;
			Default_dialogue_action.Action_ptr = NULL;
			Default_dialogue_action.Action_data = (void *) NULL;

			/* Perform action */
			Result = Perform_action(&Default_dialogue_action);

			/* Any reaction ? */
			if (!Result)
			{
				/* No -> Do default reaction */
				Display_text_and_wait(&Main_OPM, System_text_ptrs[503]);

				/* End dialogue */
				End_dialogue_flag = TRUE;
			}
			break;
		}
		/* Default is error */
		default:
		{
			Dialogue_error(DIALOGUE_ERR_ILLEGAL_DEFAULT_MC_INDEX);
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_current_dialogue_node
 * FUNCTION  : Set current dialogue node.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 15:54
 * LAST      : 25.06.95 15:54
 * INPUTS    : struct Dialogue_node *Node - Pointer to structure containing
 *              new dialogue node data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_current_dialogue_node(struct Dialogue_node *Node)
{
	/* Copy new data into current dialogue node */
	memcpy((UNBYTE *) &Current_dialogue_node, (UNBYTE *) Node,
	 sizeof(struct Dialogue_node));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_normal_MC_entries
 * FUNCTION  : Add normal multiple-choice entries to a list.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 16:18
 * LAST      : 25.06.95 16:50
 * INPUTS    : struct MC_entry *MC_list - Pointer to list.
 *             UNSHORT Nr_MC_entries - Number of entries in the list so far.
 *             struct Dialogue_node *Node_ptr - Pointer to dialogue node
 *              data.
 * RESULT    : UNSHORT : New number of entries in the list.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Add_normal_MC_entries(struct MC_entry *MC_list, UNSHORT Nr_MC_entries,
 struct Dialogue_node *Node_ptr)
{
	UNLONG Subblock_offsets[NR_NORMAL_MC_ENTRIES];
	UNSHORT i;
	UNCHAR *Text_ptr;

	/* Get text file address */
	Text_ptr = (UNCHAR *) MEM_Claim_pointer(Node_ptr->Text_handle);

	/* Find text block */
	Text_ptr = Find_text_block(Text_ptr, Node_ptr->Text_block_nr);

	/* Build sub-block catalogue */
	Build_subblock_catalogue(Text_ptr, Subblock_offsets, NR_NORMAL_MC_ENTRIES);

	MEM_Free_pointer(Node_ptr->Text_handle);

	/* Check all normal multiple-choice entries */
	for (i=FIRST_NORMAL_MC_ENTRY;i<NR_NORMAL_MC_ENTRIES;i++)
	{
		/* Is this entry present ? */
		if (Subblock_offsets[i])
		{
			/* Yes -> Write appropriate MC entry data */
			MC_list[Nr_MC_entries].Type = NORMAL_MC_ENTRY_TYPE;
			MC_list[Nr_MC_entries].Index = i;
			MC_list[Nr_MC_entries].Text_handle = Node_ptr->Text_handle;
			MC_list[Nr_MC_entries].Text_offset = Subblock_offsets[i];

			/* Count up */
			Nr_MC_entries++;
		}
	}

	return Nr_MC_entries;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_some_default_MC_entries
 * FUNCTION  : Add some default multiple-choice entries to a list,
 *              depending on dialogue node data.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 16:51
 * LAST      : 25.06.95 16:51
 * INPUTS    : struct MC_entry *MC_list - Pointer to list.
 *             UNSHORT Nr_MC_entries - Number of entries in the list so far.
 *             struct Dialogue_node *Node_ptr - Pointer to dialogue node
 *              data.
 * RESULT    : UNSHORT : New number of entries in the list.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Add_some_default_MC_entries(struct MC_entry *MC_list, UNSHORT Nr_MC_entries,
 struct Dialogue_node *Node_ptr)
{
	UNLONG Subblock_offsets[NR_NORMAL_MC_ENTRIES];
	UNSHORT i;
	UNCHAR *Text_ptr;

	/* Get text file address */
	Text_ptr = (UNCHAR *) MEM_Claim_pointer(Node_ptr->Text_handle);

	/* Find text block */
	Text_ptr = Find_text_block(Text_ptr, Node_ptr->Text_block_nr);

	/* Build sub-block catalogue */
	Build_subblock_catalogue(Text_ptr, Subblock_offsets, NR_NORMAL_MC_ENTRIES);

	MEM_Free_pointer(Node_ptr->Text_handle);

	/* Check all default multiple-choice entries */
	for (i=0;i<NR_DEFAULT_MC_ENTRIES;i++)
	{
		/* Is this entry present ? */
		if (Subblock_offsets[i])
		{
			/* Yes -> Write appropriate MC entry data */
			MC_list[Nr_MC_entries].Type = DEFAULT_MC_ENTRY_TYPE;
			MC_list[Nr_MC_entries].Index = i;
			MC_list[Nr_MC_entries].Text_handle = Node_ptr->Text_handle;
			MC_list[Nr_MC_entries].Text_offset = Subblock_offsets[i];

			/* Count up */
			Nr_MC_entries++;
		}
	}

	return Nr_MC_entries;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_FAQ_MC_entries
 * FUNCTION  : Add FAQ multiple-choice entries to a list.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 10:33
 * LAST      : 25.06.95 16:50
 * INPUTS    : struct MC_entry *MC_list - Pointer to list.
 *             UNSHORT Nr_MC_entries - Number of entries in the list so far.
 * RESULT    : UNSHORT : New number of entries in the list.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Add_FAQ_MC_entries(struct MC_entry *MC_list, UNSHORT Nr_MC_entries)
{
	UNLONG Dummy_offsets[NR_FAQ_MC_ENTRIES];
	UNSHORT i;
	UNCHAR *Text_ptr;

	/* Get pointer to first dialogue event text file */
	Text_ptr = (UNCHAR *) MEM_Claim_pointer(Dialogue_event_text_handles[0]);

	/* Get pointer to block 0 */
	Text_ptr = Find_text_block(Text_ptr, 0);

	/* Build sub-block catalogue to see if FAQ reply sub-blocks are present */
	Build_subblock_catalogue(Text_ptr, Dummy_offsets, NR_FAQ_MC_ENTRIES);

	MEM_Free_pointer(Dialogue_event_text_handles[0]);

	/* Check all FAQ multiple-choice entries */
	for (i=0;i<NR_FAQ_MC_ENTRIES;i++)
	{
		/* Is this entry present ? */
		if (Dummy_offsets[i])
		{
			/* Yes -> Write appropriate MC entry data */
			MC_list[Nr_MC_entries].Type = FAQ_MC_ENTRY_TYPE;
			MC_list[Nr_MC_entries].Index = i;
			MC_list[Nr_MC_entries].Text_handle = NULL;
			MC_list[Nr_MC_entries].Text_offset = (UNLONG)
			 System_text_ptrs[500 + i];

			/* Count up */
			Nr_MC_entries++;
		}
	}

	return Nr_MC_entries;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_default_MC_entries
 * FUNCTION  : Add default multiple-choice entries to the global list.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 17:02
 * LAST      : 25.06.95 16:50
 * INPUTS    : struct MC_entry *MC_list - Pointer to list.
 *             UNSHORT Nr_MC_entries - Number of entries in the list so far.
 * RESULT    : UNSHORT : New number of entries in the list.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Add_default_MC_entries(struct MC_entry *MC_list, UNSHORT Nr_MC_entries)
{
	UNSHORT i;

	/* Add all default multiple-choice entries */
	for (i=0;i<NR_DEFAULT_MC_ENTRIES;i++)
	{
		/* Skip if join party / character already in party */
		if ((i == JOIN_PARTY_DEF_MC_ENTRY) && Dialogue_partner_in_party)
			continue;

		/* Skip if leave party / character is not in party */
		if ((i == LEAVE_PARTY_DEF_MC_ENTRY) && !Dialogue_partner_in_party)
			continue;

		/* Write MC entry data */
		MC_list[Nr_MC_entries].Type = DEFAULT_MC_ENTRY_TYPE;
		MC_list[Nr_MC_entries].Index = i;
		MC_list[Nr_MC_entries].Text_handle = NULL;
		MC_list[Nr_MC_entries].Text_offset = (UNLONG)
		 System_text_ptrs[491 + i];

		/* Count up */
		Nr_MC_entries++;
	}

	return Nr_MC_entries;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_multiple_choice
 * FUNCTION  : Select an answer from a set of multiple-choice options.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 16:58
 * LAST      : 22.06.95 16:58
 * INPUTS    : UNSHORT Nr_entries - Number of entries.
 *             struct MC_entry *Entry_list - Pointer to list of multiple-
 *              choice entries.
 * RESULT    : UNSHORT : Select entry index / 0xFFFF = aborted.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Select_multiple_choice(UNSHORT Nr_entries, struct MC_entry *Entry_list)
{
	struct MC_window_OID OID;
	UNSHORT MC_window_object;
	UNSHORT Result = 0xFFFF;

	/* Build OID */
	OID.Nr_entries = Nr_entries;
	OID.Entry_list = Entry_list;
	OID.Result_ptr = &Result;

	Push_module(&Window_Mod);

	MC_window_object = Add_object(0, &MC_window_Class, (UNBYTE *) &OID, 10,
	 10, 100, 100);

	Execute_method(MC_window_object, DRAW_METHOD, NULL);

	Wait_4_object(MC_window_object);

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_MC_window_object
 * FUNCTION  : Initialize method of Multiple-choice window object.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 12:39
 * LAST      : 22.06.95 16:34
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_MC_window_object(struct Object *Object, union Method_parms *P)
{
	struct MC_window_object *MC_window;
	struct MC_window_OID *OID;
	struct MC_entry_OID Entry_OID;
	struct PA PA;
	UNSHORT Window_height;
	UNSHORT Entry_heights[MAX_MC_ENTRIES];
	UNLONG Text_height;
	UNSHORT Y;
	UNSHORT i;
	UNCHAR *Text_ptr;

	MC_window = (struct MC_window_object *) Object;
	OID = (struct MC_window_OID *) P;

	/* Copy data from OID */
	MC_window->Result_ptr = OID->Result_ptr;

	/* Build fake PA */
	PA.Area.left = 0;
	PA.Area.top = 0;
	PA.Area.width = MC_ENTRY_WIDTH - 4;
	PA.Area.height = 200;

	/* Install fake PA */
	Push_PA(&PA);

	/* Scan all entries and calculate window height */
	Window_height = 20;
	for (i=0;i<OID->Nr_entries;i++)
	{
		/* Anything in this entry ? */
		if (OID->Entry_list[i].Type != EMPTY_MC_ENTRY_TYPE)
		{
			/* Yes -> Calculate text size */
			Text_ptr = (UNCHAR *) MEM_Claim_pointer(OID->Entry_list[i].Text_handle)
			 + OID->Entry_list[i].Text_offset;

			Get_text_size(Text_ptr, NULL, &Text_height);

			MEM_Free_pointer(OID->Entry_list[i].Text_handle);

			/* Store entry height */
			Entry_heights[i] = Text_height + 1;
		}
		else
		{
			/* No -> Empty line */
			Entry_heights[i] = 8;
		}

		/* Add entry height to window height */
		Window_height += Entry_heights[i] + 1;
	}

	/* Remove fake PA */
	Pop_PA();

	/* Subtract the last in-between area */
	Window_height -= 1;

	/* Too large ? */
	if (Window_height > Panel_Y)
	{
		/* Yes -> Delete self */
		Delete_object(Object->Self);

		/* Exit */
		return 0;
	}

	/* Change window size */
	Change_object_size(Object->Self, MC_ENTRY_WIDTH + 23, Window_height);

	/* Change window position */
	Change_object_position(Object->Self, (Screen_width -
	 Object->Rect.width) / 2, Panel_Y - Object->Rect.height);

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Add entries to window */
	Y = 10;
	for (i=0;i<OID->Nr_entries;i++)
	{
		/* Anything in this entry ? */
		if (OID->Entry_list[i].Type != EMPTY_MC_ENTRY_TYPE)
		{
			/* Yes -> Build entry OID */
			memcpy((UNBYTE *) &(Entry_OID.Data), (UNBYTE *) &(OID->Entry_list[i]),
			 sizeof(struct MC_entry));
			Entry_OID.Index = i;

			/* Add entry */
			Add_object(Object->Self, &MC_entry_Class, (UNBYTE *) &Entry_OID, 11,
			 Y, MC_ENTRY_WIDTH, Entry_heights[i]);
		}

		/* Increase Y-coordinate */
		Y += Entry_heights[i] + 1;
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_MC_window_object
 * FUNCTION  : Draw method of Multiple-choice window object.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 13:07
 * LAST      : 25.06.95 16:50
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_MC_window_object(struct Object *Object, union Method_parms *P)
{
	struct MC_window_object *MC_window;
	struct OPM *OPM;
	UNSHORT W, H;

	MC_window = (struct MC_window_object *) Object;
	OPM = &(MC_window->Window_OPM);

	W = Object->Rect.width;
	H = Object->Rect.height;

	/* Draw window's shadow */
	Put_recoloured_box(OPM, 10, 10, W - 10, H - 10, &(Recolour_tables[1][0]));

	/* Draw window */
	Draw_window_inside(OPM, 7, 7, W - 14, H - 14);
	Draw_window_border(OPM, 0, 0, W, H);

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_MC_entry_object
 * FUNCTION  : Initialize method of Multiple-choice entry object.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 13:33
 * LAST      : 22.06.95 13:33
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_MC_entry_object(struct Object *Object, union Method_parms *P)
{
	struct MC_entry_object *MC_entry;
	struct MC_entry_OID *OID;

	MC_entry = (struct MC_entry_object *) Object;
	OID = (struct MC_entry_OID *) P;

	/* Copy data from OID */
	memcpy((UNBYTE *) &(MC_entry->Data), (UNBYTE *) &(OID->Data),
	 sizeof(struct MC_entry));
	MC_entry->Index = OID->Index;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_MC_entry_object
 * FUNCTION  : Draw method of Multiple-choice entry object.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 13:35
 * LAST      : 22.06.95 14:27
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_MC_entry_object(struct Object *Object, union Method_parms *P)
{
	struct MC_entry_object *MC_entry;
	struct MC_window_object *MC_window;
	struct OPM *OPM;

	MC_entry = (struct MC_entry_object *) Object;
	MC_window = (struct MC_window_object *) Get_object_data(Object->Parent);
	OPM = &(MC_window->Window_OPM);

	/* Clear entry area */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw entry */
	Draw_MC_entry(MC_entry);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_MC_entry_object
 * FUNCTION  : Feedback method of Multiple-choice entry object.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 14:27
 * LAST      : 22.06.95 14:27
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_MC_entry_object(struct Object *Object, union Method_parms *P)
{
	struct MC_entry_object *MC_entry;
	struct MC_window_object *MC_window;
	struct OPM *OPM;

	MC_entry = (struct MC_entry_object *) Object;
	MC_window = (struct MC_window_object *) Get_object_data(Object->Parent);
	OPM = &(MC_window->Window_OPM);

	/* Clear entry area */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Draw_deep_box(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw entry */
	Draw_MC_entry(MC_entry);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_MC_entry_object
 * FUNCTION  : Highlight method of Multiple-choice entry object.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 14:27
 * LAST      : 22.06.95 14:27
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_MC_entry_object(struct Object *Object, union Method_parms *P)
{
	struct MC_entry_object *MC_entry;
	struct MC_window_object *MC_window;
	struct OPM *OPM;

	MC_entry = (struct MC_entry_object *) Object;
	MC_window = (struct MC_window_object *) Get_object_data(Object->Parent);
	OPM = &(MC_window->Window_OPM);

	/* Clear entry area */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Put_recoloured_box(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height, &(Recolour_tables[6][0]));

	/* Draw entry */
	Draw_MC_entry(MC_entry);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_MC_entry_object
 * FUNCTION  : Left method of Multiple-choice entry object.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 16:20
 * LAST      : 23.06.95 19:14
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_MC_entry_object(struct Object *Object, union Method_parms *P)
{
	struct MC_entry_object *MC_entry;
	struct MC_window_object *MC_window;

	MC_entry = (struct MC_entry_object *) Object;
	MC_window = (struct MC_window_object *) Get_object_data(Object->Parent);

	/* Clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Select this entry */
		*(MC_window->Result_ptr) = MC_entry->Index;

		/* Close multiple-choice window */
		Close_Window_object(&(MC_window->Object), NULL);
	}
	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_MC_entry
 * FUNCTION  : Draw a multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 13:35
 * LAST      : 25.06.95 16:50
 * INPUTS    : struct MC_entry_object *MC_entry - Pointer to entry object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_MC_entry(struct MC_entry_object *MC_entry)
{
	struct MC_window_object *MC_window;
	struct OPM *OPM;
	struct PA PA;
	UNCHAR *Text_ptr;

	MC_window = (struct MC_window_object *)
	 Get_object_data(MC_entry->Object.Parent);
	OPM = &(MC_window->Window_OPM);

	/* Build PA */
	PA.Area.left = MC_entry->Object.X + 2;
	PA.Area.top = MC_entry->Object.Y + 1;
	PA.Area.width = MC_ENTRY_WIDTH - 4;
	PA.Area.height = MC_entry->Object.Rect.height - 1;

	/* Display MC entry text */
	Push_PA(&PA);

	Text_ptr = (UNCHAR *) MEM_Claim_pointer(MC_entry->Data.Text_handle) +
	 MC_entry->Data.Text_offset;
	Display_text(OPM, Text_ptr);
	MEM_Free_pointer(MC_entry->Data.Text_handle);

	Pop_PA();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dialogue_error
 * FUNCTION  : Report a dialogue error.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 17:18
 * LAST      : 25.06.95 17:18
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dialogue_error(UNSHORT Error_code)
{
	/* Push error on the error stack */
	ERROR_PushError(Dialogue_print_error, Dialogue_library_name,
	 sizeof(UNSHORT), (UNBYTE *) &Error_code);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dialogue_print_error
 * FUNCTION  : Print a dialogue error.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 17:18
 * LAST      : 25.06.95 17:18
 * INPUTS    : UNCHAR *buffer - Pointer to output buffer.
 *             UNBYTE *data - Pointer to error data written by Dialogue_error.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Output length should not exceed BBERROR_OUTSTRINGSIZE.
 * SEE ALSO  : BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dialogue_print_error(UNCHAR *buffer, UNBYTE *data)
{
	UNSHORT i;

	/* Get error code */
	i = *((UNSHORT *) data);

	/* Catch illegal errors */
	if (i>DIALOGUE_ERR_MAX)
		i = 0;

 	/* Print error */
	sprintf((char *)buffer, Dialogue_error_strings[i]);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_dictionary
 * FUNCTION  : Initialize the dictionary.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 19:47
 * LAST      :
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_dictionary(void)
{
	MEM_HANDLE Dictionary_handles[NR_DICTIONARIES];
	BOOLEAN Result;
	UNSHORT Batch[NR_DICTIONARIES];
	UNSHORT i;

	/* Build dictionary batch */
	for (i=0;i<NR_DICTIONARIES;i++)
		Batch[i] = i;

	/* Load dictionaries */
	Result = Load_full_batch(DICTIONARY, NR_DICTIONARIES, &Batch[0],
	 &Dictionary_handles[0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return;
	}

}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_dictionary
 * FUNCTION  : Exit the dictionary.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 19:47
 * LAST      :
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_dictionary(void)
{
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_word_in_dictionary
 * FUNCTION  : Find a word in a dictionary.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.06.95 09:58
 * LAST      : 26.06.95 09:58
 * INPUTS    : UNCHAR *Dictionary - Pointer to dictionary.
 *             UNSHORT Word_index - Word index (1...).
 * RESULT    : UNCHAR * : Pointer to word.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
Find_word_in_dictionary(UNCHAR *Dictionary, UNSHORT Word_index)
{
	static UNCHAR *Illegal_word = "ILLEGAL";
	UNSHORT Nr_words;
	UNSHORT i;
	UNCHAR *Word_ptr;

	/* Get number of words in dictionary */
	Nr_words = *((UNSHORT *) Dictionary);

	/* Legal word index ? */
	if (!Word_index || (Word_index > Nr_words))
	{
		/* No -> Exit */
		return Illegal_word;
	}

	/* Find word */
	Word_ptr = Dictionary + 2;
	for (i=1;i<Word_index;i++)
	{
		/* Skip current word */
		Word_ptr += (UNSHORT) *Word_ptr;
	}

	return Word_ptr;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_word_in_dictionary
 * FUNCTION  : Search a word in a dictionary.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.06.95 10:05
 * LAST      : 26.06.95 10:12
 * INPUTS    : UNCHAR *Dictionary - Pointer to dictionary.
 *             UNCHAR *Input_word - Pointer to word.
 * RESULT    : UNSHORT : Word index / 0 (not found).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_word_in_dictionary(UNCHAR *Dictionary, UNCHAR *Input_word)
{
	UNSHORT Nr_words;
	UNSHORT Word_index = 0;
	UNSHORT Input_word_length;
	UNSHORT Current_word_length;
	UNSHORT i;
	UNCHAR *Word_ptr;

	/* Get length of input word */
	Input_word_length = strlen(Input_word);

	/* Get number of words in dictionary */
	Nr_words = *((UNSHORT *) Dictionary);

	/* Scan dictionary */
	Word_ptr = Dictionary + 2;
	for (i=1;i<=Nr_words;i++)
	{
		/* Get length of current word */
		Current_word_length = (UNSHORT) *Word_ptr;
		Word_ptr++;

		/* Same length as input word ? */
		if (Current_word_length == Input_word_length)
		{
			/* Yes -> Same word ? */
			if (!strcmp(Word_ptr, Input_word))
			{
				/* Yes -> Found it */
				Word_index = i;
				break;
			}
		}

		/* Next word */
		Word_ptr += Current_word_length - 1;
	}

	return Word_index;
}

