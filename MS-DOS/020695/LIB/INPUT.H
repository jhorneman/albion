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
#include <BBOPM.H>
#include <TEXT.H>
#include <CONTROL.H>
#include <USERFACE.H>

/*
 ** Defines ****************************************************************
 */

#define INPUTNR_WIDTH	(200)
#define INPUTNR_HEIGHT	(92)

#define BOOLEANREQ_WIDTH	(200)
#define BOOLEANREQ_HEIGHT	(74)

#define MEMBERSELWIN_WIDTH		(270)
#define MEMBERSELWIN_HEIGHT	(99)

#define ITEMSELWIN_WIDTH	(213)
#define ITEMSELWIN_HEIGHT	(120)

#define ADD_DELTA			(8)
#define FIRST_ADD			(256 - (20 * ADD_DELTA))
#define MAX_DELTA_ADD	(16384)

#define NUMBER_BAR_HEIGHT			(12)
#define NUMBER_BAR_SLIDER_WIDTH	(29)

/* Text window parameters */
#define TEXT_WINDOW_X			(20)
#define TEXT_WINDOW_Y			(0)
#define TEXT_WINDOW_WIDTH		(320)
#define TEXT_WINDOW_HEIGHT		(182)

/*
 ** Type definitions *******************************************************
 */

typedef UNSHORT (*Item_evaluator)(struct Character_data *Char,
 struct Item_packet *Packet);
typedef UNSHORT (*Member_evaluator)(MEM_HANDLE Char_handle);

/*
 ** Structure definitions **************************************************
 */

/* Number input OID */
struct InputNr_OID {
	SILONG *Value_ptr;
	SILONG Minimum, Maximum;

	UNCHAR *Text;

	UNSHORT Symbol_width, Symbol_height;
	MEM_HANDLE Graphics_handle;
	UNLONG Graphics_offset;
};

/* Number input object */
struct InputNr_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	SILONG *Value_ptr;
	SILONG Minimum, Maximum;

	UNCHAR *Text;

	UNSHORT Symbol_width, Symbol_height;
	MEM_HANDLE Graphics_handle;
	UNLONG Graphics_offset;

};

/* Number bar OID */
struct Number_bar_OID {
	SILONG *Value_ptr, Minimum, Maximum;
	void (*Update)(struct Number_bar_object *Number_bar);
};

/* Number bar object */
struct Number_bar_object {
	struct Object Object;
	UNSHORT Flags;

	SILONG *Value_ptr, Minimum, Maximum;

	void (*Update)(struct Number_bar_object *Number_bar);

	UNSHORT Slider_X;				/* X-coordinate of slider */
	SISHORT Delta_add;
};

/* Value display object */
struct Value_display_object {
	struct Object Object;
	UNLONG Value;
};

/* Boolean requester OID */
struct BooleanReq_OID {
	UNCHAR *Text;
	BOOLEAN *Result_ptr;
};

/* Boolean requester object */
struct BooleanReq_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNCHAR *Text;
	BOOLEAN *Result_ptr;
};

/* Party member select window OID */
struct MemberSel_window_OID {
	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;
};

/* Party member select window object */
struct MemberSel_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;
};

/* Selectable party member OID */
struct SelMember_OID {
	UNSHORT Member_index;
};

/* Selectable party member object */
struct SelMember_object {
	struct Object Object;
	UNSHORT Member_index;
};

/* Item select window OID */
struct ItemSel_window_OID {
	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;

	MEM_HANDLE Slots_handle;
	UNLONG Slots_offset;
};

/* Item select window object */
struct ItemSel_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;
};

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

/* Input function */
void Input_string(UNSHORT X, UNSHORT Y, UNSHORT Width, UNSHORT Max_length,
 UNCHAR *Buffer);

/* Input module functions */
void Init_input(void);
void Exit_input(void);
void Display_input(void);

/* Input object methods */
void Customkeys_input(struct Object *Object, union Method_parms *P);
void Right_input(struct Object *Object, union Method_parms *P);

/* Number input functions */
SILONG Input_number(SILONG Value, SILONG Minimum, SILONG Maximum,	UNCHAR *Text);
SILONG Input_number_with_symbol(SILONG Value, SILONG Minimum, SILONG Maximum,
 UNCHAR *Text, UNSHORT Width, UNSHORT Height, MEM_HANDLE Graphics_handle,
 UNLONG Graphics_offset);

/* Number input object methods */
void Init_InputNr_object(struct Object *Object, union Method_parms *P);
void Exit_InputNr_object(struct Object *Object, union Method_parms *P);
void Draw_InputNr_object(struct Object *Object, union Method_parms *P);

/* Number input button handler */
void InputNr_Exit(struct Button_object *Button);

/* Number bar object methods */
void Init_Number_bar_object(struct Object *Object, union Method_parms *P);
void Draw_Number_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Get_Number_bar_object(struct Object *Object, union Method_parms *P);
void Set_Number_bar_object(struct Object *Object, union Method_parms *P);
void Left_Number_bar_object(struct Object *Object, union Method_parms *P);

/* Number bar support functions */
void Number_bar_page_up(struct Number_bar_object *Number_bar);
void Number_bar_page_down(struct Number_bar_object *Number_bar);
void Light_number_slider(struct Number_bar_object *Number_bar);
void Update_number_slider(struct Number_bar_object *Number_bar, UNSHORT X);
void Do_draw_number_bar(struct Number_bar_object *Number_bar);

void Number_bar_1_up(struct Button_object *Button);
void Number_bar_1_down(struct Button_object *Button);

/* Text window functions */
void Do_text_file_window(MEM_HANDLE Text_file_handle, UNSHORT Text_block_nr);
void Do_text_window(UNBYTE *Ptr);

/* Boolean requester function */
BOOLEAN Boolean_requester(UNCHAR *Text);

/* Boolean requester object methods */
void Init_BooleanReq_object(struct Object *Object, union Method_parms *P);
void Exit_BooleanReq_object(struct Object *Object, union Method_parms *P);
void Draw_BooleanReq_object(struct Object *Object, union Method_parms *P);

/* Boolean requester button handlers */
void BooleanReq_Yes(struct Button_object *Button);
void BooleanReq_No(struct Button_object *Button);

/* Party member select function */
UNSHORT Select_party_member(UNCHAR *Text, Member_evaluator Evaluator);

/* Party member select window object methods */
void Init_MemberSel_object(struct Object *Object, union Method_parms *P);
void Exit_MemberSel_object(struct Object *Object, union Method_parms *P);
void Draw_MemberSel_object(struct Object *Object, union Method_parms *P);
void Close_MemberSel_object(struct Object *Object, union Method_parms *P);

/* Selectable party member object methods */
void Init_SelMember_object(struct Object *Object, union Method_parms *P);
void Draw_SelMember_object(struct Object *Object, union Method_parms *P);
void Feedback_SelMember_object(struct Object *Object, union Method_parms *P);
void Highlight_SelMember_object(struct Object *Object, union Method_parms *P);
void Help_SelMember_object(struct Object *Object, union Method_parms *P);
void Left_SelMember_object(struct Object *Object, union Method_parms *P);

/* Item select function */
UNSHORT Select_character_item(MEM_HANDLE Char_handle, UNCHAR *Text,
 Item_evaluator Evaluator);

/* Item select window object methods */
void Init_ItemSel_object(struct Object *Object, union Method_parms *P);
void Exit_ItemSel_object(struct Object *Object, union Method_parms *P);
void Draw_ItemSel_object(struct Object *Object, union Method_parms *P);
void Close_ItemSel_object(struct Object *Object, union Method_parms *P);

/* Selectable item object methods */
void Draw_SelItem_object(struct Object *Object, union Method_parms *P);
void Update_SelItem_object(struct Object *Object, union Method_parms *P);
void Feedback_SelItem_object(struct Object *Object, union Method_parms *P);
void Highlight_SelItem_object(struct Object *Object, union Method_parms *P);
void Left_SelItem_object(struct Object *Object, union Method_parms *P);

#endif

