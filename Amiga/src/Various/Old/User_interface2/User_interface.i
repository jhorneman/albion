; User interface constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 17-2-1994

Forbidden_X	EQU 160
Forbidden_Y	EQU 156

Scroll_bar_width	EQU 6

Between	EQU 5			; Space between slots and scroll bar

IS_inner_width	EQU 16		; Inner dimensions of slot
IS_inner_height	EQU 20
IS_between_X	EQU 2		; Space between slots
IS_between_Y	EQU 1
IS_outer_width	EQU IS_inner_width+IS_between_X
IS_outer_height	EQU IS_inner_height+IS_between_Y
Quantity_offset	EQU 12		; Vertical offset to quantity

Max_PUMES	EQU 20
PUM_title_height	EQU Standard_text_height+6
PUME_height	EQU Standard_text_height+4
PUM_edge	EQU 9

TS_height	EQU Standard_text_height+3

;*****************************************************************************
; These are the top 64 colours
	rsreset
	rs.b 192
Cycling_1:	rs.b 1			; Cycling all the time
Cycling_2:	rs.b 1
White:	rs.b 1			; White
Brightest:	rs.b 1			; Standard user-interface colours
Brighter:	rs.b 1
Bright:	rs.b 1
Normal:	rs.b 1
Dark:	rs.b 1
Darker:	rs.b 1
Darkest:	rs.b 1
Indigo:	rs.b 3			; Other colour groups
Purple:	rs.b 3
Red:	rs.b 5
Green:	rs.b 6
Gold:	rs.b 5
Turquoise:	rs.b 4
Blue:	rs.b 4
Skin:	rs.b 8
Marmor:	rs.b 16

Highlight_colour	EQU Brightest
Capital_colour	EQU Indigo

;***************************************************************************	
; Standard OID :
	rsreset
OID_X:	rs.w 1
OID_Y:	rs.w 1
OID_width:	rs.w 1
OID_height:	rs.w 1
Standard_OID_size:	rs.b 0

;***************************************************************************	
; Scroll bar object
; OID :
	rsreset
	rs.w 1			; Coordinates of scroll bar
	rs.w 1
OID_Total_units:	rs.w 1		; Total amount of units
OID_Units_width:	rs.w 1		; Number of units per row
OID_Units_height:	rs.w 1		; Number of rows on screen
OID_Scroll_bar_height:	rs.w 1	; Height of scroll bar in pixels
Scroll_bar_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
Total_units:	rs.w 1		; Total amount of units
Units_width:	rs.w 1		; Number of units per row
Units_height:	rs.w 1		; Number of rows on screen
Slider_height:	rs.w 1		; Height of slider in pixels
Slider_Y:	rs.w 1			; Y-coordinate of slider
Row_height:	rs.w 1		; Height of row in pixels
Current_row:	rs.w 1		; Current row
Scroll_bar_height:	rs.w 1		; Height of scroll bar in pixels
Scroll_bar_max_height:	rs.w 1	; Maximum Y-coordinate of slider
Scroll_bar_result:	rs.w 1		; Current unit selected by scroll bar {0...}
Scroll_bar_object_size:	rs.b 0

;***************************************************************************	
; Item list object
; OID :
	rsreset
	rs.w 1			; Coordinates of item list
	rs.w 1
OID_IL_width:	rs.w 1		; Number of slots per row
OID_IL_height:	rs.w 1		; Number of rows on screen
OID_IL_nr_items:	rs.w 1
OID_IL_slots_handle:	rs.b 1
	rseven
OID_IL_slots_offset:	rs.l 1
OID_IL_touched:	rs.l 1
OID_IL_selected:	rs.l 1
OID_IL_PUM:	rs.l 1
Item_list_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
IL_nr_items:	rs.w 1		; Number of item slots in inventory
IL_slots_handle:	rs.b 1		; Memory handle of inventory
				;  (0=absolute address)
	rseven
IL_slots_offset:	rs.l 1		; Long offset to inventory
IL_touched:	rs.l 1		; Pointer to a routine which is
				;  called when the player moves the
				;  mouse over an item slot
IL_selected:	rs.l 1		; Pointer to a routine which is
				;  called when the player left-clicks
				;  on an item slot
IL_PUM:	rs.l 1			; Pointer to a pop-up menu data
				;  structure
Item_list_object_size:	rs.b 0

;***************************************************************************	
; Item slot object
; OID :
	rsreset
	rs.w 1			; Coordinates of item slot
	rs.w 1
OID_IS_nr:	rs.w 1
Item_slot_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
IS_nr:	rs.w 1
Item_slot_object_size:	rs.b 0

;***************************************************************************	
; Earth object
; Object :
	rsreset
	rs.b Object_header_size
Earth_object_size:	rs.b 0

;***************************************************************************	
; Box object
; OID :
	rsreset
	rs.b Standard_OID_size
Box_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
Box_object_size:	rs.b 0

;***************************************************************************	
; HBox object
; OID :
	rsreset
	rs.b Box_OID_size
HBox_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Box_object_size
HBox_object_size:	rs.b 0

;***************************************************************************	
; DBox object
; OID :
	rsreset
	rs.b Box_OID_size
DBox_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Box_object_size
DBox_object_size:	rs.b 0

;***************************************************************************	
; Symbol object
; OID :
	rsreset
	rs.w 1
	rs.w 1
OID_Symbol:	rs.b Symbol_data_size
Symbol_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
Object_symbol:	rs.b Symbol_data_size
Symbol_object_size:	rs.b 0

;***************************************************************************	
; Text object
; OID :
	rsreset
	rs.w 1
	rs.w 1
OID_Text_object_width:	rs.w 1
OID_Text_ptr:	rs.l 1
OID_Text_colour:	rs.w 1
OID_Text_feedback_colour:	rs.w 1
Text_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
Text_object_width:	rs.w 1
Text_ptr:	rs.l 1
Text_colour:	rs.w 1
Text_feedback_colour:	rs.w 1
Text_object_size:	rs.b 0

;***************************************************************************	
; Window object
; OID :
	rsreset
	rs.b Standard_OID_size
Window_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
Window_bg_handle:	rs.b 1
	rseven
Window_object_size:	rs.b 0

;***************************************************************************	
; Text area object
; OID :
	rsreset
	rs.b Standard_OID_size
OID_Ink:	rs.w 1
OID_Shadow:	rs.w 1
OID_Paper:	rs.w 1
Text_area_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
Text_area_PA:	rs.b PA_data_size
Text_area_object_size:	rs.b 0

;***************************************************************************	
; Button object
; OID :
	rsreset
	rs.b Standard_OID_size
OID_Button_data:	rs.l 1
OID_Button_function:	rs.l 1
Button_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
Button_data:	rs.l 1
Button_function:	rs.l 1
Button_object_size:	rs.b 0

;***************************************************************************	
; Button (or switch) + text object
; OID :
	rsreset
	rs.b Button_OID_size
OID_Button_text_colour:	rs.w 1
OID_Button_text_feedback_colour:	rs.w 1
Button_text_OID_size:	rs.b 0

;***************************************************************************	
; Switch object
; Object :
	rsreset
	rs.b Button_object_size
Switch_state:	rs.b 1
	rseven
Switch_object_size:	rs.b 0

;***************************************************************************	
; Radio object
; Object :
	rsreset
	rs.b Object_header_size
Current_button:	rs.w 1
Radio_object_size:	rs.b 0

;***************************************************************************	
; Radio button object
; OID :
	rsreset
	rs.b Button_text_OID_size
OID_Radio_button_nr:	rs.w 1
OID_Radio_handle:	rs.w 1
Radio_button_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Switch_object_size
Radio_button_nr:	rs.w 1
Radio_handle:	rs.w 1
Radio_button_object_size:	rs.b 0

;***************************************************************************	
; Pop-up menu object
; OID :
	rsreset
	rs.w 1
	rs.w 1
OID_PUM_data:	rs.l 1
PUM_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
PUM_data:	rs.l 1
PUM_current_entries:	rs.w 1
PUM_object_size:	rs.b 0

;***************************************************************************	
; Pop-up menu entry object
; OID :
	rsreset
	rs.b Button_text_OID_size
OID_PUME_nr:	rs.w 1
OID_PUM_handle:	rs.w 1
PUME_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Button_object_size
PUME_nr:	rs.w 1
PUM_handle:	rs.w 1
PUME_object_size:	rs.b 0

;***************************************************************************	
; Pop-up menu structure
	rsreset
PUM_nr_entries:	rs.w 1
PUM_title:	rs.l 1
PUM_evaluator:	rs.l 1
PUM_header_size:	rs.b 0

;***************************************************************************	
; Pop-up menu entry structure
	rsreset
PUME_flags:	rs.b 1
	rseven
PUME_blocked_prompt:	rs.w 1
PUME_text:	rs.l 1
PUME_function:	rs.l 1
PUME_data_size:	rs.b 0

;***************************************************************************	
; Pop-up menu entry flags
	rsreset
PUME_absent:	rs.b 1
PUME_not_selectable:	rs.b 1
PUME_blocked:	rs.b 1
PUME_auto_close:	rs.b 1

;***************************************************************************	
; Text list object
; OID :
	rsreset
	rs.w 1			; Coordinates of text list
	rs.w 1
	rs.w 1			; Width of text slots
OID_TL_width:	rs.w 1		; Number of slots per row
OID_TL_height:	rs.w 1		; Number of rows on screen
OID_TL_nr_texts:	rs.w 1
OID_TL_text_colour:	rs.w 1
OID_TL_text_feedback_colour:	rs.w 1
OID_TL_get_text:	rs.l 1
OID_TL_touched:	rs.l 1
OID_TL_selected:	rs.l 1
Text_list_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
TL_nr_texts:	rs.w 1		; Number of texts in list
TL_text_colour:	rs.w 1		; Text colours
TL_text_feedback_colour:	rs.w 1
TL_get_text:	rs.l 1		; Pointer to routine which is called
				;  to get the text for a slot
TL_touched:	rs.l 1		; Pointer to a routine which is
				;  called when the player moves the
				;  mouse over an item slot
TL_selected:	rs.l 1		; Pointer to a routine which is
				;  called when the player left-clicks
				;  on a text slot
Text_list_object_size:	rs.b 0

;***************************************************************************	
; Text slot object
; OID :
	rsreset
	rs.w 1			; Coordinates of text slot
	rs.w 1
	rs.w 1			; Width of text slot
OID_TS_nr:	rs.w 1
Text_slot_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
TS_nr:	rs.w 1
Text_slot_object_size:	rs.b 0
