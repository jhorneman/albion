/************
 * NAME     : INPUT.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 11-1-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef INPUT_H
#define INPUT_H

#include <BBDEF.H>
#include <BBMEM.H>
#include <CONTROL.H>
#include <GAME.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Type definitions *******************************************************
 */

typedef UNSHORT (*Item_evaluator)(struct Character_data *Char,
 struct Item_packet *Packet);
typedef UNSHORT (*Member_evaluator)(MEM_HANDLE Char_handle);

/*
 ** Structure definitions **************************************************
 */

/* Value display object */
struct Value_display_object {
	struct Object Object;
	UNLONG Value;
};

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

/* Input function */
void Input_string_in_window(UNSHORT Max_length, UNCHAR *Text_ptr,
 UNCHAR *Buffer_ptr);
void Input_string(UNSHORT X, UNSHORT Y, UNSHORT Width, UNSHORT Max_length,
 UNCHAR *Buffer_ptr);

/* Number input functions */
SILONG Input_number(SILONG Value, SILONG Minimum, SILONG Maximum,
 UNCHAR *Text);
SILONG Input_number_with_symbol(SILONG Value, SILONG Minimum, SILONG Maximum,
 UNCHAR *Text, UNSHORT Width, UNSHORT Height, MEM_HANDLE Graphics_handle,
 UNLONG Graphics_offset);

/* Text window functions */
void Do_text_file_window(MEM_HANDLE Text_file_handle, UNSHORT Text_block_nr);
void Do_text_window(UNBYTE *Ptr);

/* Boolean requester functions */
BOOLEAN Boolean_requester(UNCHAR *Text);
BOOLEAN Boolean_requester_with_buttons(UNCHAR *Text, UNCHAR *Yes_button_text,
 UNCHAR *No_button_text);

/* Party member select function */
UNSHORT Select_party_member(UNCHAR *Text, Member_evaluator Evaluator);

/* Item select function */
UNSHORT Select_character_item(MEM_HANDLE Char_handle, UNCHAR *Text,
 Item_evaluator Evaluator);

#endif

