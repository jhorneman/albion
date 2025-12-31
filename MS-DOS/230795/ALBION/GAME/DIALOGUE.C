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
#include <SORT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <GAMETEXT.H>
#include <FONT.H>
#include <BUTTONS.H>
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

#define LOG_ENTRIES_PER_LIST	(100)

#define MAX_MC_ENTRIES			(10)

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
#define DIALOGUE_ERR_ILLEGAL_FAQ_MC_INDEX			(2)
#define DIALOGUE_ERR_FILE_LOAD						(3)

#define DIALOGUE_ERR_MAX	(3)

/* structure definitions */

/* Give item data */
struct Give_item_data {
	MEM_HANDLE Char_handle;
	UNSHORT Item_slot_index;
};

/* Dialogue log entry */
struct Dialogue_log_entry {
	UNBYTE Char_type;
	UNSHORT Char_index;
	UNBYTE Action_type;		/* = Event action type */
	UNSHORT Action_value;
};

/* Dialogue log block */
struct Dialogue_log_list {
	MEM_HANDLE Next_list;
	struct Dialogue_log_entry Entries[LOG_ENTRIES_PER_LIST];
};

/* Multiple-choice entry */
struct MC_entry {
	UNSHORT Type;				/* See above */
	UNSHORT Index;
	MEM_HANDLE Text_handle;
	UNSHORT Text_block_nr;
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

/* Dialogue module functions */
void Dialogue_ModInit(void);
void Dialogue_ModExit(void);
void Dialogue_DisInit(void);
void Dialogue_DisExit(void);
void Dialogue_MainLoop(void);

/* Dialogue support functions */
BOOLEAN Init_dialogue_data(void);
void Exit_dialogue_data(void);

/* Multiple-choice entry handlers */
void Handle_normal_MC_entry(UNSHORT Text_block_nr, UNSHORT MC_index);
void Handle_FAQ_MC_entry(UNSHORT MC_index);
void Handle_default_MC_entry(UNSHORT MC_index);

/* Default multiple-choice entry handlers */
void Handle_ask_word_default_MC_entry(void);

void Handle_give_item_default_MC_entry(void);
BOOLEAN Do_give_item(struct Event_action *Action);

void Handle_join_party_default_MC_entry(void);
BOOLEAN Do_join_party(struct Event_action *Action);

void Handle_leave_party_default_MC_entry(void);
BOOLEAN Do_leave_party(struct Event_action *Action);

void Handle_end_dialogue_default_MC_entry(void);
BOOLEAN Do_end_dialogue(struct Event_action *Action);

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

void Restore_dialogue_background(void);

void Destroy_dialogue_log_list(MEM_HANDLE Handle);
void Add_dialogue_log_entry(UNSHORT Action_type, UNSHORT Action_value);

/* global variables */

static struct Module Dialogue_Mod = {
	LOCAL_MOD, SCREEN_MOD, DIALOGUE_SCREEN,
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

static MEM_HANDLE Word_bit_arrays_handle;

UNBYTE *Known_words_bit_array_ptr;
UNBYTE *New_words_bit_array_ptr;

struct Dialogue_node Current_dialogue_node;
struct Dialogue_node Default_dialogue_node;

static UNSHORT Dialogue_object;

static struct PA Dialogue_PA;
static MEM_HANDLE Dialogue_background_handle;
static struct OPM Dialogue_background_OPM;

/* Dialogue parameters */
UNSHORT Dialogue_type;
UNSHORT Dialogue_char_type;
UNSHORT Dialogue_char_index;
static UNSHORT Dialogue_party_member_index;
static UNSHORT Dialogue_NPC_index;

/* Dialogue log data */
MEM_HANDLE First_dialogue_log_list;
UNSHORT Nr_dialogue_log_entries;

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
	"Illegal default multiple-choice entry index.",
	"Illegal FAQ multiple-choice entry index.",
	"File loading error."
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_Dialogue
 * FUNCTION  : Start a dialogue.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.04.95 10:49
 * LAST      : 30.06.95 15:22
 * INPUTS    : UNSHORT Type - Dialogue type.
 *             UNSHORT Char_type - Character type.
 *             UNSHORT Char_index - Character index.
 *             UNSHORT NPC_index - NPC index / 0xFFFF = no NPC.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_Dialogue(UNSHORT Type, UNSHORT Char_type, UNSHORT Char_index,
 UNSHORT NPC_index)
{
	/* Store dialogue parameters */
	Dialogue_type = Dialogue_type;
	Dialogue_char_type = Char_type;
	Dialogue_char_index = Char_index;
	Dialogue_NPC_index = NPC_index;

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
 * LAST      : 05.07.95 10:12
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
	BOOLEAN Result;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Clear flag */
	End_dialogue_flag = FALSE;

	/* Destroy default dialogue node */
	Default_dialogue_node.Text_handle = NULL;

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
					Dialogue_party_member_index = i + 1;
					break;
				}
			}
		}
	}


	/* Initialize word arrays */
	Word_bit_arrays_handle = MEM_Allocate_memory(2 * ((WORDS_MAX + 7) / 8));

	Known_words_bit_array_ptr = MEM_Claim_pointer(Word_bit_arrays_handle);
	New_words_bit_array_ptr = Known_words_bit_array_ptr + (WORDS_MAX + 7) / 8;

	/* Clear word arrays */
	Clear_bit_array(KNOWN_WORDS_BIT_ARRAY);
	Clear_bit_array(NEW_WORDS_BIT_ARRAY);

	/* Initialize dialogue data */
	Result = Init_dialogue_data();
	if (!Result)
	{
		Pop_module();
	}

	/* Current screen is 2D map ? */
	if (Current_screen_type(1) == MAP_2D_SCREEN)
	{
		/* Yes -> Prepare screen */
		Current_2D_OPM = &Main_OPM;
		Draw_2D_scroll_buffer(MAP_2D_X, MAP_2D_Y);
		Current_2D_OPM = NULL;
	}

	/* Build Print Area */
	Dialogue_PA.Area.left = DIALOGUE_WINDOW_X + 10;
	Dialogue_PA.Area.top = DIALOGUE_WINDOW_Y + 10;
	Dialogue_PA.Area.width = DIALOGUE_WINDOW_WIDTH - 20;
	Dialogue_PA.Area.height = DIALOGUE_WINDOW_HEIGHT - 20;

	/* Install stuff */
	Push_PA(&Dialogue_PA);
	Push_textstyle(&Default_text_style);
	Push_root(&Main_OPM);
	Push_mouse(&(Mouse_pointers[DEFAULT_MPTR]));

	/* Update OPM */
	Add_update_OPM(&Status_area_OPM);

	/* In dialogue */
	In_Dialogue = TRUE;

	/* Initialize display */
	Init_display();

	/* Allocate memory for background */
	Dialogue_background_handle = MEM_Allocate_memory(Dialogue_PA.Area.width *
	 Dialogue_PA.Area.height);

	/* Create background OPM */
	Ptr = MEM_Claim_pointer(Dialogue_background_handle);
	OPM_New(Dialogue_PA.Area.width, Dialogue_PA.Area.height, 1,
	 &Dialogue_background_OPM, Ptr);
	MEM_Free_pointer(Dialogue_background_handle);

	/* Save background */
	OPM_CopyOPMOPM(&Main_OPM, &Dialogue_background_OPM, Dialogue_PA.Area.left,
	 Dialogue_PA.Area.top, Dialogue_PA.Area.width, Dialogue_PA.Area.height,
	 0, 0);

	/* Start of dialogue */
	{
		struct Event_action Dialogue_starts_action;
		BOOLEAN Result;

		/* Clear event action data */
		BASEMEM_FillMemByte((UNBYTE *) &Dialogue_starts_action,
		 sizeof(struct Event_action), 0);

		/* Write event action data */
		Dialogue_starts_action.Actor_type = PARTY_ACTOR_TYPE;
		Dialogue_starts_action.Actor_index = PARTY_DATA.Active_member;
		Dialogue_starts_action.Action_type = DIALOGUE_START_ACTION;

		/* Perform action */
		Result = Perform_dialogue_action(&Dialogue_starts_action);

		/* Any reaction ? */
		if (Result)
		{
			/* Yes -> Make a new entry in the dialogue log */
			Add_dialogue_log_entry(DIALOGUE_START_ACTION, 0);

			/* Has the dialogue already ended ? */
			if (End_dialogue_flag)
			{
				/* Yes -> Exit */
				Exit_display();
				Pop_module();
			}
		}
		else
		{
			/* No -> Set ink _*/
			Set_ink(GOLD_TEXT);

			/* Clear text window */
			Restore_dialogue_background();

			/* Character in party ? */
			if (Dialogue_partner_in_party)
			{
				/* Yes */
				Display_text_and_wait(&Main_OPM, System_text_ptrs[506]);
			}
			else
			{
				/* No */
				Display_text_and_wait(&Main_OPM, System_text_ptrs[505]);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dialogue_ModExit
 * FUNCTION  : Exit Dialogue module.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.06.95 12:31
 * LAST      : 05.07.95 10:12
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
	/* Was the Dialogue module initialized successfully ? */
	if (In_Dialogue)
	{
		/* Yes -> Remove update OPM */
		Remove_update_OPM(&Status_area_OPM);

		/* Remove stuff */
		Pop_mouse();
		Pop_root();
		Pop_textstyle();
		Pop_PA();

		/* No longer in dialogue */
		In_Dialogue = FALSE;

		/* Destroy background buffer and OPM */
		OPM_Del(&Dialogue_background_OPM);
		MEM_Free_memory(Dialogue_background_handle);
	}

	/* Destroy word arrays */
	MEM_Free_pointer(Word_bit_arrays_handle);
	MEM_Free_memory(Word_bit_arrays_handle);

	Known_words_bit_array_ptr = NULL;
	New_words_bit_array_ptr = NULL;

	/* Exit dialogue data */
	Exit_dialogue_data();

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
 * LAST      : 06.07.95 16:00
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
	 &(Recolour_tables[0][0]));
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
	Put_masked_block(&Main_OPM, 360 - 36 - 8, 5, 34, 37, Ptr);
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
 * LAST      : 28.06.95 10:49
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
	struct Dialogue_node Node;
	UNSHORT Nr_MC_entries;
	UNSHORT Selected_MC_entry;
	UNCHAR *Text_ptr;

	/* Clear number of multiple-choice entries */
	Nr_MC_entries = 0;

	/* Anything special in the current dialogue node ? */
	if (Current_dialogue_node.Text_handle)
	{
		/* Yes -> Use this node */
		memcpy((UNBYTE *) &Node, (UNBYTE *) &Current_dialogue_node,
		 sizeof(struct Dialogue_node));

		/* Get text file address */
		Text_ptr = (UNCHAR *) MEM_Claim_pointer(Node.Text_handle);

		/* Find text block */
		Text_ptr = Find_text_block(Text_ptr, Node.Text_block_nr);

		/* Clear text window */
		Restore_dialogue_background();

		/* Set ink colour */
		Set_ink(GOLD_TEXT);

		/* Display text and wait */
		Display_text_and_wait(&Main_OPM, Text_ptr);

		MEM_Free_pointer(Node.Text_handle);

		/* Destroy current dialogue node */
		Current_dialogue_node.Text_handle = NULL;
	}
	else
	{
		/* No -> Use default node */
		memcpy((UNBYTE *) &Node, (UNBYTE *) &Default_dialogue_node,
		 sizeof(struct Dialogue_node));
	}

	/* Anything special in the dialogue node ? */
	if (Node.Text_handle)
	{
		/* Yes -> Add normal multiple-choice entries */
		Nr_MC_entries = Add_normal_MC_entries(Dialogue_MC_list, Nr_MC_entries,
		 &Node);

		/* Add default multiple-choice entries ? */
		if (!Node.Flags & DIALOGUE_NO_DEFAULT)
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
			 Nr_MC_entries, &Node);

			/* Is the last entry empty ? */
			if (Nr_MC_entries && (Dialogue_MC_list[Nr_MC_entries - 1].Type ==
			 EMPTY_MC_ENTRY_TYPE))
			{
				/* Yes -> Remove it */
				Nr_MC_entries--;
			}
		}
	}
	else
	{
		/* No -> Add FAQ entries */
		Nr_MC_entries = Add_FAQ_MC_entries(Dialogue_MC_list, Nr_MC_entries);

		/* Add default entries */
		Nr_MC_entries = Add_default_MC_entries(Dialogue_MC_list, Nr_MC_entries);
	}

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
			Handle_normal_MC_entry(Node.Text_block_nr,
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
 * NAME      : Init_dialogue_data
 * FUNCTION  : Init dialogue data.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.07.95 16:35
 * LAST      : 01.07.95 16:35
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_dialogue_data(void)
{
	struct Character_data *Char;
	UNSHORT Char_file_type;

	/* Talking with someone in the party ? */
	if (Dialogue_partner_in_party)
	{
		/* Yes -> Set handles */
		Dialogue_char_handle =
		 Party_char_handles[Dialogue_party_member_index - 1];

		Dialogue_small_portrait_handle =
		 Small_portrait_handles[Dialogue_party_member_index - 1];

		Dialogue_event_set_handles[0] =
		 Party_event_set_handles[Dialogue_party_member_index - 1][0];
		Dialogue_event_set_handles[1] =
		 Party_event_set_handles[Dialogue_party_member_index - 1][1];

		Dialogue_event_text_handles[0] =
		 Party_event_text_handles[Dialogue_party_member_index - 1][0];
		Dialogue_event_text_handles[1] =
		 Party_event_text_handles[Dialogue_party_member_index - 1][1];
	}
	else
	{
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
		if (!Dialogue_char_handle)
		{
			Dialogue_error(DIALOGUE_ERR_FILE_LOAD);

			Exit_program();

			return FALSE;
		}

		/* Get pointer to character data */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Dialogue_char_handle);

		/* Load portrait */
		Dialogue_small_portrait_handle = Load_subfile(SMALL_PORTRAIT,
		 (UNSHORT) Char->Portrait_nr);

		if (!Dialogue_small_portrait_handle)
		{
			Dialogue_error(DIALOGUE_ERR_FILE_LOAD);

			Exit_program();

			return FALSE;
		}

		/* First event set present ? */
		if (Char->Event_set_1_nr)
		{
			/* Yes -> Load first event set */
			Dialogue_event_set_handles[0] = Load_subfile(EVENT_SET,
			 (UNSHORT) Char->Event_set_1_nr);
			if (!Dialogue_event_set_handles[0])
			{
				Dialogue_error(DIALOGUE_ERR_FILE_LOAD);

				Exit_program();

				return FALSE;
			}

			/* Load first event set texts */
			Dialogue_event_text_handles[0] = Load_subfile(EVENT_TEXT,
			 (UNSHORT) Char->Event_set_1_nr);
			if (!Dialogue_event_text_handles[0])
			{
				Dialogue_error(DIALOGUE_ERR_FILE_LOAD);

				Exit_program();

				return FALSE;
			}
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
			if (!Dialogue_event_set_handles[1])
			{
				Dialogue_error(DIALOGUE_ERR_FILE_LOAD);

				Exit_program();

				return FALSE;
			}

			/* Load second event set texts */
			Dialogue_event_text_handles[1] = Load_subfile(EVENT_TEXT,
			 (UNSHORT) Char->Event_set_2_nr);
			if (!Dialogue_event_text_handles[0])
			{
				Dialogue_error(DIALOGUE_ERR_FILE_LOAD);

				Exit_program();

				return FALSE;
			}
		}
		else
		{
			/* No -> Clear handles */
			Dialogue_event_set_handles[1] = NULL;
			Dialogue_event_text_handles[1] = NULL;
		}

		MEM_Free_pointer(Dialogue_char_handle);
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_dialogue_data
 * FUNCTION  : Exit dialogue data.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.07.95 16:38
 * LAST      : 01.07.95 16:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_dialogue_data(void)
{
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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_normal_MC_entry
 * FUNCTION  : Handle a normal dialogue multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 17:08
 * LAST      : 01.07.95 15:28
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
	BOOLEAN Result;

	/* Build event action data */
	Select_answer_action.Actor_index = PARTY_DATA.Active_member;
	Select_answer_action.Action_value = Text_block_nr;
	Select_answer_action.Action_extra = MC_index;

	/* Perform action */
	Result = Perform_dialogue_action(&Select_answer_action);

	/* Any reaction ? */
	if (Result)
	{
		/* Yes -> Make a new entry in the dialogue log */
		Add_dialogue_log_entry(SELECT_ANSWER_ACTION, Text_block_nr * 256 +
		 MC_index);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_FAQ_MC_entry
 * FUNCTION  : Handle a FAQ dialogue multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 17:11
 * LAST      : 26.06.95 14:10
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
	UNLONG Subblock_offsets[NR_FAQ_MC_ENTRIES];
	UNCHAR *Text_ptr;

	/* Get pointer to first dialogue event text file */
	Text_ptr = (UNCHAR *) MEM_Claim_pointer(Dialogue_event_text_handles[0]);

	/* Get pointer to block 0 */
	Text_ptr = Find_text_block(Text_ptr, 0);

	/* Build sub-block catalogue to see if FAQ reply sub-blocks are present */
	Build_subblock_catalogue(Text_ptr, Subblock_offsets, NR_FAQ_MC_ENTRIES);

	/* Is the given FAQ present ? */
	if (Subblock_offsets[MC_index])
	{
		/* Yes -> Clear text window */
		Restore_dialogue_background();

		/* Do reaction */
		Set_ink(GOLD_TEXT);
		Display_text_and_wait(&Main_OPM, Text_ptr +
		 Subblock_offsets[MC_index]);
	}
	else
	{
		/* No -> Error */
		Dialogue_error(DIALOGUE_ERR_ILLEGAL_FAQ_MC_INDEX);
	}

	MEM_Free_pointer(Dialogue_event_text_handles[0]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_default_MC_entry
 * FUNCTION  : Handle a default dialogue multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 17:11
 * LAST      : 28.06.95 14:46
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
	/* Clear text window */
	Restore_dialogue_background();

	/* Act depending on entry index */
	switch (MC_index)
	{
		/* Ask about word */
		case WORD_DEF_MC_ENTRY:
		{
			Handle_ask_word_default_MC_entry();
			break;
		}
		/* Give item */
		case GIVE_ITEM_DEF_MC_ENTRY:
		{
			Handle_give_item_default_MC_entry();
			break;
		}
		/* Ask to join */
		case JOIN_PARTY_DEF_MC_ENTRY:
		{
			Handle_join_party_default_MC_entry();
			break;
		}
		/* Ask to leave */
		case LEAVE_PARTY_DEF_MC_ENTRY:
		{
			Handle_leave_party_default_MC_entry();
			break;
		}
		/* End dialogue */
		case END_DIALOGUE_DEF_MC_ENTRY:
		{
			Handle_end_dialogue_default_MC_entry();
			break;
		}
		/* Error */
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
 * NAME      : Handle_ask_word_default_MC_entry
 * FUNCTION  : Handle the Ask Word default dialogue multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.06.95 14:44
 * LAST      : 01.07.95 15:29
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_ask_word_default_MC_entry(void)
{
	struct Event_action Default_dialogue_action;
	BOOLEAN Result;
	UNSHORT Word_instances[MAX_WORD_INSTANCES];
	UNSHORT Merged_word_index;
	UNSHORT Nr_instances;
	UNSHORT i;

	/* Clear event action data */
	BASEMEM_FillMemByte((UNBYTE *) &Default_dialogue_action,
	 sizeof(struct Event_action), 0);

	/* Prepare event action data */
	Default_dialogue_action.Actor_type = PARTY_ACTOR_TYPE;
	Default_dialogue_action.Actor_index = PARTY_DATA.Active_member;

	/* Select a word */
	Merged_word_index = Select_word();

	/* Set ink */
	Set_ink(GOLD_TEXT);

	/* Any word selected ? */
	if (Merged_word_index != 0xFFFF)
	{
		/* Yes -> Unknown word ? */
		if (Merged_word_index == 0xFFFE)
		{
			/* Yes -> Write action data */
			Default_dialogue_action.Action_type = UNKNOWN_WORD_ACTION;

			/* Perform action */
			Result = Perform_dialogue_action(&Default_dialogue_action);

			/* Any reaction ? */
			if (Result)
			{
				/* Yes -> Make a new entry in the dialogue log */
				Add_dialogue_log_entry(UNKNOWN_WORD_ACTION, 0);
			}
			else
			{
				/* No -> Character in party ? */
				if (Dialogue_partner_in_party)
				{
					/* Yes -> Default reaction */
					Display_text_and_wait(&Main_OPM, System_text_ptrs[510]);
				}
				else
				{
					/* No -> Default reaction */
					Display_text_and_wait(&Main_OPM, System_text_ptrs[509]);
				}
			}
		}
		else
		{
			/* No -> Prepare action data */
			Default_dialogue_action.Action_type = ASK_ABOUT_ACTION;

			/* Default is no reaction */
			Result = FALSE;

			/* Get references to selected word */
			Nr_instances = Search_references_to_merged_word(Merged_word_index,
			 Word_instances);

			/* Check all instances */
			for (i=0;i<Nr_instances;i++)
			{
				/* Write action data */
				Default_dialogue_action.Action_value = Word_instances[i];

				/* Perform action */
				Result = Perform_dialogue_action(&Default_dialogue_action);

				/* Any reaction ? */
				if (Result)
				{
					/* Yes -> Make this word known */
					Write_bit_array(KNOWN_WORDS_BIT_ARRAY, Word_instances[i],
					 SET_BIT_ARRAY);

					/* Make a new entry in the dialogue log */
					Add_dialogue_log_entry(ASK_ABOUT_ACTION, Merged_word_index);

					/* Break */
					break;
				}
			}

			/* Any reaction ? */
			if (!Result)
			{
				/* No -> Write action data */
				Default_dialogue_action.Action_type = ANY_WORD_ACTION;

				/* Perform action */
				Result = Perform_dialogue_action(&Default_dialogue_action);

				/* Any reaction ? */
				if (Result)
				{
					/* Yes -> Make a new entry in the dialogue log */
					Add_dialogue_log_entry(ANY_WORD_ACTION, 0);
				}
				else
				{
					/* No -> Character in party ? */
					if (Dialogue_partner_in_party)
					{
						/* Yes -> Default reaction */
						Display_text_and_wait(&Main_OPM, System_text_ptrs[498]);
					}
					else
					{
						/* No -> Default reaction */
						Display_text_and_wait(&Main_OPM, System_text_ptrs[497]);
					}
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_give_item_default_MC_entry
 * FUNCTION  : Handle the Give Item default dialogue multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.06.95 14:45
 * LAST      : 01.07.95 15:29
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_give_item_default_MC_entry(void)
{
	struct Give_item_data Give_item_data;
	struct Event_action Default_dialogue_action;
	struct Character_data *Char;
	struct Item_data *Item_data;
	struct Item_packet *Packet;
	BOOLEAN Result;
	UNSHORT Item_slot_index;
	UNSHORT Item_index;
	UNSHORT Item_type;

	/* Clear event action data */
	BASEMEM_FillMemByte((UNBYTE *) &Default_dialogue_action,
	 sizeof(struct Event_action), 0);

	/* Prepare event action data */
	Default_dialogue_action.Actor_type = PARTY_ACTOR_TYPE;
	Default_dialogue_action.Actor_index = PARTY_DATA.Active_member;

	/* Select an item */
	Item_slot_index = Select_character_item(Active_char_handle,
	 System_text_ptrs[496], Vital_item_evaluator);

	/* Set ink */
	Set_ink(GOLD_TEXT);

	/* Any item selected ? */
	if (Item_slot_index != 0xFFFF)
	{
		/* Yes -> Get selected item packet */
		Char = (struct Character_data *)	MEM_Claim_pointer(Active_char_handle);

		/* Body or backpack item ? */
		if (Item_slot_index <= ITEMS_ON_BODY)
		{
			/* Body -> Get pointer to packet */
			Packet = &(Char->Body_items[Item_slot_index - 1]);
		}
		else
		{
			/* Backpack -> Get pointer to packet */
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

		/* Build give item data */
		Give_item_data.Char_handle = Active_char_handle;
		Give_item_data.Item_slot_index = Item_slot_index;

		/* Write event action data */
		Default_dialogue_action.Action_type = GIVE_ITEM_ACTION;
		Default_dialogue_action.Action_value = Item_index;
		Default_dialogue_action.Action_extra = Item_type;
		Default_dialogue_action.Action_ptr = Do_give_item;
		Default_dialogue_action.Action_data = (void *) &Give_item_data;

		/* Forbidden by default! */
		Default_dialogue_action.Flags |= FORBIDDEN;

		/* Perform action */
		Result = Perform_dialogue_action(&Default_dialogue_action);

		/* Any reaction ? */
		if (Result)
		{
			/* Yes -> Make a new entry in the dialogue log */
			Add_dialogue_log_entry(GIVE_ITEM_ACTION, Item_index);
		}
		else
		{
			/* No -> Do default reaction */
			Display_text_and_wait(&Main_OPM, System_text_ptrs[499]);
		}
	}
}

BOOLEAN
Do_give_item(struct Event_action *Action)
{
	struct Give_item_data *Give_item_data;

	/* Get data */
	Give_item_data = (struct Give_item_data *) Action->Action_data;

	/* Remove item */
	Remove_item(Give_item_data->Char_handle,
	 Give_item_data->Item_slot_index, 1);

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_join_party_default_MC_entry
 * FUNCTION  : Handle the Join Party default dialogue multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.06.95 14:47
 * LAST      : 01.07.95 15:29
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_join_party_default_MC_entry(void)
{
	struct Event_action Default_dialogue_action;
	BOOLEAN Result;

	/* Set ink */
	Set_ink(GOLD_TEXT);

	/* Is the party full ? */
	if (Party_is_full())
	{
		/* Yes -> "No room!" */
		Display_text_and_wait(&Main_OPM, System_text_ptrs[508]);
	}

	/* Clear event action data */
	BASEMEM_FillMemByte((UNBYTE *) &Default_dialogue_action,
	 sizeof(struct Event_action), 0);

	/* Write event action data */
	Default_dialogue_action.Actor_type = PARTY_ACTOR_TYPE;
	Default_dialogue_action.Actor_index = PARTY_DATA.Active_member;
	Default_dialogue_action.Action_type = ASK_TO_JOIN_ACTION;
	Default_dialogue_action.Action_ptr = Do_join_party;
	Default_dialogue_action.Action_data = (void *) NULL;

	/* Forbidden by default! */
	Default_dialogue_action.Flags |= FORBIDDEN;

	/* Perform action */
	Result = Perform_dialogue_action(&Default_dialogue_action);

	/* Any reaction ? */
	if (Result)
	{
		/* Yes -> Make a new entry in the dialogue log */
		Add_dialogue_log_entry(ASK_TO_JOIN_ACTION, 0);
	}
	else
	{
		/* No -> Do default reaction */
		Display_text_and_wait(&Main_OPM, System_text_ptrs[501]);
	}
}

BOOLEAN
Do_join_party(struct Event_action *Action)
{
	BOOLEAN Result;

	/* Add a party member */
	Result = Add_party_member(Dialogue_char_index, Dialogue_NPC_index);
	if (!Result)
		return FALSE;

	/* The dialogue partner is now in the party */
	Dialogue_partner_in_party = TRUE;

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_leave_party_default_MC_entry
 * FUNCTION  : Handle the Leave Party default dialogue multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.06.95 14:49
 * LAST      : 01.07.95 15:29
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_leave_party_default_MC_entry(void)
{
	struct Event_action Default_dialogue_action;
	BOOLEAN Result;

	/* Set ink */
	Set_ink(GOLD_TEXT);

	/* Clear event action data */
	BASEMEM_FillMemByte((UNBYTE *) &Default_dialogue_action,
	 sizeof(struct Event_action), 0);

	/* Write event action data */
	Default_dialogue_action.Actor_type = PARTY_ACTOR_TYPE;
	Default_dialogue_action.Actor_index = PARTY_DATA.Active_member;
	Default_dialogue_action.Action_type = ASK_TO_LEAVE_ACTION;
	Default_dialogue_action.Action_ptr = Do_leave_party;
	Default_dialogue_action.Action_data = (void *) NULL;

	/* Forbidden by default! */
	Default_dialogue_action.Flags |= FORBIDDEN;

	/* Perform action */
	Result = Perform_dialogue_action(&Default_dialogue_action);

	/* Any reaction ? */
	if (Result)
	{
		/* Yes -> Make a new entry in the dialogue log */
		Add_dialogue_log_entry(ASK_TO_LEAVE_ACTION, 0);
	}
	else
	{
		/* No -> Do default reaction */
		Display_text_and_wait(&Main_OPM, System_text_ptrs[502]);

		/* Remove party member */
		Remove_party_member(Dialogue_party_member_index);

		/* The dialogue partner is now no longer in the party */
		Dialogue_partner_in_party = FALSE;

		/* End dialogue */
		End_dialogue_flag = TRUE;
	}
}

BOOLEAN
Do_leave_party(struct Event_action *Action)
{
	/* Remove party member */
	Remove_party_member(Dialogue_party_member_index);

	/* The dialogue partner is now no longer in the party */
	Dialogue_partner_in_party = FALSE;

	/* End dialogue */
	End_dialogue_flag = TRUE;

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_end_dialogue_default_MC_entry
 * FUNCTION  : Handle the End Dialogue default dialogue multiple-choice entry.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.06.95 14:50
 * LAST      : 01.07.95 15:29
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_end_dialogue_default_MC_entry(void)
{
	struct Event_action Default_dialogue_action;
	BOOLEAN Result;

	/* Set ink */
	Set_ink(GOLD_TEXT);

	/* Clear event action data */
	BASEMEM_FillMemByte((UNBYTE *) &Default_dialogue_action,
	 sizeof(struct Event_action), 0);

	/* Write event action data */
	Default_dialogue_action.Actor_type = PARTY_ACTOR_TYPE;
	Default_dialogue_action.Actor_index = PARTY_DATA.Active_member;
	Default_dialogue_action.Action_type = DIALOGUE_END_ACTION;
	Default_dialogue_action.Action_ptr = Do_end_dialogue;
	Default_dialogue_action.Action_data = (void *) NULL;

	/* Perform action */
	Result = Perform_dialogue_action(&Default_dialogue_action);

	/* Any reaction ? */
	if (Result)
	{
		/* Yes -> Make a new entry in the dialogue log */
		Add_dialogue_log_entry(DIALOGUE_END_ACTION, 0);
	}
	else
	{
		/* No -> Character in party ? */
		if (Dialogue_partner_in_party)
		{
			/* Yes */
			Display_text_and_wait(&Main_OPM, System_text_ptrs[504]);
		}
		else
		{
			/* No */
			Display_text_and_wait(&Main_OPM, System_text_ptrs[503]);
		}
	}
}

BOOLEAN
Do_end_dialogue(struct Event_action *Action)
{
	/* End dialogue */
	End_dialogue_flag = TRUE;

	return TRUE;
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
 * NAME      : Set_default_dialogue_node
 * FUNCTION  : Set default dialogue node.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.06.95 10:48
 * LAST      : 28.06.95 10:48
 * INPUTS    : struct Dialogue_node *Node - Pointer to structure containing
 *              new dialogue node data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_default_dialogue_node(struct Dialogue_node *Node)
{
	/* Copy new data into default dialogue node */
	memcpy((UNBYTE *) &Default_dialogue_node, (UNBYTE *) Node,
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
 * LAST      : 27.06.95 20:04
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
			MC_list[Nr_MC_entries].Text_block_nr = Node_ptr->Text_block_nr;
			MC_list[Nr_MC_entries].Text_offset = Subblock_offsets[i];

			/* Count up */
			Nr_MC_entries++;
		}
	}

	/* Were any entries added ? */
	if (Nr_MC_entries)
	{
		/* Yes -> Add an empty entry */
		MC_list[Nr_MC_entries].Type = EMPTY_MC_ENTRY_TYPE;

		/* Count up */
		Nr_MC_entries++;
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
			MC_list[Nr_MC_entries].Text_block_nr = Node_ptr->Text_block_nr;
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
			MC_list[Nr_MC_entries].Text_block_nr = 500 + i;
			MC_list[Nr_MC_entries].Text_offset = 0;

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
		MC_list[Nr_MC_entries].Text_block_nr = 491 + i;
		MC_list[Nr_MC_entries].Text_offset = 0;

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
			/* Yes -> Is the text in a text file ? */
			if (OID->Entry_list[i].Text_handle)
			{
				/* Yes -> Get pointer to text file */
				Text_ptr =
				 (UNCHAR *) MEM_Claim_pointer(OID->Entry_list[i].Text_handle);

				/* Get pointer to text block */
				Text_ptr = Find_text_block(Text_ptr,
				 OID->Entry_list[i].Text_block_nr);

				/* Add text offset */
				Text_ptr += OID->Entry_list[i].Text_offset;
			}
			else
			{
				/* No -> Get pointer to system message */
				Text_ptr = System_text_ptrs[OID->Entry_list[i].Text_block_nr];
			}

			/* Calculate text size */
			Get_text_size(Text_ptr, NULL, &Text_height);

			/* Free pointer if necessary */
			if (OID->Entry_list[i].Text_handle)
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

	/* Set ink depending on entry type */
	if (MC_entry->Data.Type == NORMAL_MC_ENTRY_TYPE)
	{
		Set_ink(GOLD_TEXT);
	}
	else
	{
		Set_ink(SILVER_TEXT);
	}

	/* Install Print Area */
	Push_PA(&PA);

	/* Is the text in a text file ? */
	if (MC_entry->Data.Text_handle)
	{
		/* Yes -> Get pointer to text file */
		Text_ptr = (UNCHAR *) MEM_Claim_pointer(MC_entry->Data.Text_handle);

		/* Get pointer to text block */
		Text_ptr = Find_text_block(Text_ptr, MC_entry->Data.Text_block_nr);

		/* Add text offset */
		Text_ptr += MC_entry->Data.Text_offset;
	}
	else
	{
		/* No -> Get pointer to system message */
		Text_ptr = System_text_ptrs[MC_entry->Data.Text_block_nr];
	}

	/* Display MC entry text */
	Display_text(OPM, Text_ptr);

	/* Free pointer if necessary */
	if (MC_entry->Data.Text_handle)
		MEM_Free_pointer(MC_entry->Data.Text_handle);

	/* Remove Print Area */
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
 * NAME      : Restore_dialogue_background
 * FUNCTION  : Restore the background of the dialogue text window.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 11:06
 * LAST      : 28.06.95 10:27
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Restore_dialogue_background(void)
{
	/* Restore background */
	OPM_CopyOPMOPM(&Dialogue_background_OPM, &Main_OPM, 0, 0,
	 Dialogue_PA.Area.width, Dialogue_PA.Area.height, Dialogue_PA.Area.left,
	 Dialogue_PA.Area.top);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_text_in_dialogue
 * FUNCTION  : Display a text block from a text file in the dialogue text
 *              window.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 21:51
 * LAST      : 28.06.95 10:25
 * INPUTS    : MEM_HANDLE Text_file_handle - Memory handle of text file.
 *             UNSHORT Text_block_nr - Text block number.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_text_in_dialogue(MEM_HANDLE Text_file_handle,
 UNSHORT Text_block_nr)
{
	UNCHAR *Text_ptr;

	/* Install Print Area */
	Push_PA(&Dialogue_PA);

	/* Get text file address */
	Text_ptr = (UNCHAR *) MEM_Claim_pointer(Text_file_handle);

	/* Find text block */
	Text_ptr = Find_text_block(Text_ptr, Text_block_nr);

	/* Clear text window */
	Restore_dialogue_background();

	/* Display text */
	Set_ink(GOLD_TEXT);
	Display_text_and_wait(&Main_OPM, Text_ptr);

	MEM_Free_pointer(Text_file_handle);

	/* Remove Print Area */
	Pop_PA();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_dialogue_log
 * FUNCTION  : Initialize dialogue log.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 11:13
 * LAST      : 30.06.95 11:13
 * INPUTS    : UNBYTE *Data - Pointer to start of dialogue log data.
 * RESULT    : UNBYTE * : Pointer to end of dialogue log data.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Init_dialogue_log(UNBYTE *Data)
{
	struct Dialogue_log_list *List;
	MEM_HANDLE Handle, Handle2;
	UNSHORT i;

	/* Read number of dialogue log entries */
	Nr_dialogue_log_entries = *((UNSHORT *) Data);
	Data += 2;

	/* Allocate first dialogue log list */
	Handle = MEM_Allocate_memory(sizeof(struct Dialogue_log_list));
	List = (struct Dialogue_log_list *) MEM_Claim_pointer(Handle);

	/* Store handle */
	First_dialogue_log_list = Handle;

	/* Clear next list entry */
	List->Next_list = NULL;

	/* Copy complete lists */
	for (i=0;i<Nr_dialogue_log_entries / LOG_ENTRIES_PER_LIST;i++)
	{
		/* Copy all log entries in this list */
		memcpy((UNBYTE *) &(List->Entries[0]), Data, LOG_ENTRIES_PER_LIST *
		 sizeof(struct Dialogue_log_entry));

		Data += LOG_ENTRIES_PER_LIST * sizeof(struct Dialogue_log_entry);

		/* Allocate next dialogue log list */
		Handle2 = MEM_Allocate_memory(sizeof(struct Dialogue_log_list));

		/* Insert handle in current list */
		List->Next_list = Handle2;

		/* Switch to next list */
		MEM_Free_pointer(Handle);
		Handle = Handle2;
		List = (struct Dialogue_log_list *) MEM_Claim_pointer(Handle);

		/* Clear next list entry */
		List->Next_list = NULL;
	}

	/* Any log entries left ? */
	if (Nr_dialogue_log_entries % LOG_ENTRIES_PER_LIST)
	{
		/* Yes -> Copy these as well */
		memcpy(&(List->Entries[0]), Data, (Nr_dialogue_log_entries %
		 LOG_ENTRIES_PER_LIST) * sizeof(struct Dialogue_log_entry));

		Data += (Nr_dialogue_log_entries % LOG_ENTRIES_PER_LIST) *
		 sizeof(struct Dialogue_log_entry);
	}

	MEM_Free_pointer(Handle);

	return Data;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_dialogue_log
 * FUNCTION  : Exit dialogue log.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 11:48
 * LAST      : 30.06.95 11:48
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_dialogue_log(void)
{
	Destroy_dialogue_log_list(First_dialogue_log_list);
}

void
Destroy_dialogue_log_list(MEM_HANDLE Handle)
{
	struct Dialogue_log_list *List;

	/* Get pointer to list */
	List = (struct Dialogue_log_list *) MEM_Claim_pointer(Handle);

	/* Is there a next list ? */
	if (List->Next_list)
	{
		/* Yes -> Destroy the next list first */
		Destroy_dialogue_log_list(List->Next_list);
	}

	/* Destroy this list */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Prepare_dialogue_log_for_saving
 * FUNCTION  : Prepare the dialogue log for saving.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 11:18
 * LAST      : 30.06.95 11:18
 * INPUTS    : None.
 * RESULT    : MEM_HANDLE : Memory handle of prepared dialogue log.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Prepare_dialogue_log_for_saving(void)
{
	struct Dialogue_log_list *List;
	MEM_HANDLE Output_handle;
	MEM_HANDLE Handle, Handle2;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Allocate memory */
	Output_handle = MEM_Allocate_memory(2 + Nr_dialogue_log_entries *
	 sizeof(struct Dialogue_log_entry));

	Ptr = MEM_Claim_pointer(Output_handle);

	/* Write number of log entries */
	*((UNSHORT *) Ptr) = Nr_dialogue_log_entries;
	Ptr += 2;

	/* Get first log list */
	Handle = First_dialogue_log_list;
	List = (struct Dialogue_log_list *) MEM_Claim_pointer(Handle);

	/* Copy complete lists */
	for (i=0;i<Nr_dialogue_log_entries / LOG_ENTRIES_PER_LIST;i++)
	{
		/* Copy all log entries in this list */
		memcpy(Ptr, &(List->Entries[0]), LOG_ENTRIES_PER_LIST *
		 sizeof(struct Dialogue_log_entry));
		Ptr += LOG_ENTRIES_PER_LIST * sizeof(struct Dialogue_log_entry);

		/* Switch to next list */
		Handle2 = List->Next_list;
		MEM_Free_pointer(Handle);
		Handle = Handle2;
		List = (struct Dialogue_log_list *) MEM_Claim_pointer(Handle);
	}

	/* Any log entries left ? */
	if (i % LOG_ENTRIES_PER_LIST)
	{
		/* Yes -> Copy these as well */
		memcpy(Ptr, &(List->Entries[0]), (i % LOG_ENTRIES_PER_LIST)
		 * sizeof(struct Dialogue_log_entry));
	}

	MEM_Free_pointer(Handle);
	MEM_Free_pointer(Output_handle);

	return Output_handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_dialogue_log_entry
 * FUNCTION  : Add a new dialogue log entry to the list.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 11:09
 * LAST      : 30.06.95 18:52
 * INPUTS    : UNSHORT Action_type - Dialogue action type.
 *             UNSHORT Action_value - Dialogue action value.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The current dialogue character type and index will be saved
 *              as well.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_dialogue_log_entry(UNSHORT Action_type, UNSHORT Action_value)
{
	struct Dialogue_log_list *List;
	MEM_HANDLE Handle, Handle2;
	BOOLEAN Exit = FALSE;
	UNSHORT i;

	/* Get first log list */
	Handle = First_dialogue_log_list;
	List = (struct Dialogue_log_list *) MEM_Claim_pointer(Handle);

	/* Check all current entries */
	for (i=0;i<Nr_dialogue_log_entries;i++)
	{
		/* Is this the same entry as the one we're adding ? */
		if ((List->Entries[i].Char_type == Dialogue_char_type) &&
		 (List->Entries[i].Char_index == Dialogue_char_index) &&
		 (List->Entries[i].Action_type == Action_type) &&
		 (List->Entries[i].Action_value == Action_value))
		{
			/* Yes -> Break */
			Exit = TRUE;
			break;
		}

		/* End of this list ? */
		if ((i % LOG_ENTRIES_PER_LIST) == LOG_ENTRIES_PER_LIST - 1)
		{
			/* Yes -> Switch to next list */
			Handle2 = List->Next_list;
			MEM_Free_pointer(Handle);
			Handle = Handle2;
			List = (struct Dialogue_log_list *) MEM_Claim_pointer(Handle);
		}
	}

	/* Do we still have to add ? */
	if (!Exit)
	{
		/* Yes -> Insert new dialogue log entry */
		i = Nr_dialogue_log_entries % LOG_ENTRIES_PER_LIST;
		List->Entries[i].Char_type = Dialogue_char_type;
		List->Entries[i].Char_index = Dialogue_char_index;
		List->Entries[i].Action_type = Action_type;
		List->Entries[i].Action_value = Action_value;

		/* Count up */
		Nr_dialogue_log_entries++;

		/* Time for a new list ? */
		if (!(Nr_dialogue_log_entries % LOG_ENTRIES_PER_LIST))
		{
			/* Yes -> Allocate next dialogue log list */
			Handle2 = MEM_Allocate_memory(sizeof(struct Dialogue_log_list));

			/* Insert handle in current list */
			List->Next_list = Handle2;

			/* Switch to next list */
			MEM_Free_pointer(Handle);
			Handle = Handle2;
			List = (struct Dialogue_log_list *) MEM_Claim_pointer(Handle);

			/* Clear next list entry */
			List->Next_list = NULL;
		}
	}

	MEM_Free_pointer(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_dialogue_log_entry
 * FUNCTION  : Find a dialogue log entry in the list.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 11:11
 * LAST      : 30.06.95 18:54
 * INPUTS    : UNSHORT Char_type - Character type.
 *             UNSHORT Char_index - Character index.
 *             UNSHORT Action_type - Dialogue action type.
 *             UNSHORT Action_value - Dialogue action value.
 * RESULT    : RESULT : TRUE (in log) or FALSE (not in log).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Find_dialogue_log_entry(UNSHORT Char_type, UNSHORT Char_index,
 UNSHORT Action_type, UNSHORT Action_value)
{
	struct Dialogue_log_list *List;
	MEM_HANDLE Handle, Handle2;
	BOOLEAN Found = FALSE;
	UNSHORT i;

	/* Get first log list */
	Handle = First_dialogue_log_list;
	List = (struct Dialogue_log_list *) MEM_Claim_pointer(Handle);

	/* Check all entries */
	for (i=0;i<Nr_dialogue_log_entries;i++)
	{
		/* Is this the entry we're looking for ? */
		if ((List->Entries[i].Char_type == Char_type) &&
		 (List->Entries[i].Char_index == Char_index) &&
		 (List->Entries[i].Action_type == Action_type) &&
		 (List->Entries[i].Action_value == Action_value))
		{
			/* Yes -> Found */
			Found = TRUE;
			break;
		}

		/* End of this list ? */
		if ((i % LOG_ENTRIES_PER_LIST) == LOG_ENTRIES_PER_LIST - 1)
		{
			/* Yes -> Switch to next list */
			Handle2 = List->Next_list;
			MEM_Free_pointer(Handle);
			Handle = Handle2;
			List = (struct Dialogue_log_list *) MEM_Claim_pointer(Handle);
		}
	}

	MEM_Free_pointer(Handle);

	return Found;
}

