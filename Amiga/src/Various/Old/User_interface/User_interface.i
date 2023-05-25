; User interface constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 17-2-1994

Max_scroll_bars	EQU 4
Scroll_bar_width	EQU 6
Scroll_bar_shadow_colour	EQU 1
Scroll_bar_inside_colour	EQU 2

Max_ILs	EQU 3

Between	EQU 4			; Space between slots and scroll bar
IS_inner_width	EQU 16		; Inner dimensions of slot
IS_inner_height	EQU 20
IS_between_X	EQU 2		; Space between slots
IS_between_Y	EQU 1
IS_outer_width	EQU IS_inner_width+IS_between_X
IS_outer_height	EQU IS_inner_height+IS_between_Y
Quantity_offset	EQU 12		; Vertical offset to quantity

;*****************************************************************************
; This is the Scrollbar structure
	rsreset
Total_units:	rs.w 1		; Total amount of units
Units_width:	rs.w 1		; Number of units per row
Units_height:	rs.w 1		; Number of rows on screen
Scroll_bar_height:	rs.w 1		; Height of scroll bar in pixels
Scroll_bar_X:	rs.w 1		; X-coordinate of scroll bar
Scroll_bar_Y:	rs.w 1		; Y-coordinate of scroll bar
Scroll_bar_draw_units:	rs.l 1	; Pointer to unit draw routine
Scroll_bar_object_layer:	rs.l 1	; Pointer to empty object layer
Scroll_bar_object_ID:	rs.l 1	; Object ID of scroll bar

	; Initialized by scroll bar handler :
Slider_height:	rs.w 1		; Height of slider in pixels
Slider_Y:	rs.w 1			; Y-coordinate of slider
Row_height:	rs.w 1		; Height of row in pixels
Current_row:	rs.w 1		; Current row
Scroll_bar_max_height:	rs.w 1	; Maximum Y-coordinate of slider
Scroll_bar_result:	rs.w 1		; Current unit selected by scroll bar {0...}
Scroll_bar_data_size:	rs.b 0

Scroll_bar_extra_data	EQU Scroll_bar_data_size-Slider_height

;*****************************************************************************
; This is the Item List structure
	rsreset
IL_nr_items:	rs.w 1	; Number of item slots in inventory
IL_slots_handle:	rs.b 1	; Memory handle of inventory (0=absolute address)
	rseven
IL_slots_offset:	rs.l 1	; Long offset to inventory
IL_coordinates_list:	rs.l 1	; Pointer to a list of coordinates for each item slot
IL_draw:	rs.l 1		; Pointer to a routine which draws the entire item list
IL_draw_slot:	rs.l 1	; Pointer to a routine which draws one item slot
IL_touched:	rs.l 1	; Pointer to a routine which is called when the player moves the mouse over an item slot
IL_clicked_left:	rs.l 1	; Pointer to a routine which is called when the player left-clicks on an item slot
IL_clicked_right:	rs.l 1	; Pointer to a routine which is called when the player right-clicks on an item slot
IL_take_out:	rs.l 1	; Pointer to a routine which takes an item out of the inventory
IL_put_in:	rs.l 1		; Pointer to a routine which puts an item in the inventory
IL_scroll_bar:	rs.l 1	; Pointer to a scroll-bar data structure
IL_PUM:	rs.l 1		; Pointer to a pop-up menu data structure
IL_object_layer:	rs.l 1	; Pointer to an object layer (2nd level)
IL_object_ID:	rs.l 1	; Second-level object layer ID (00xxxx00)
IL_data_size:	rs.b 0

;*****************************************************************************
; These are the button types
	rsreset
BT_button:	rs.b 1			; Border + text
BS_button:	rs.b 1			; Border + symbol
TS_button:	rs.b 1			; Two symbols

;*****************************************************************************
; This is the Button structure
	rsreset
Button_type:	rs.b 1		; Button type
	even
Button_ID:	rs.l 1			; Object ID of button
Button_ptr:	rs.l 1		; Pointer to text or symbol data
Button_X:	rs.w 1			; Coordinates of top-left corner of border / symbol
Button_Y:	rs.w 1
	; The following data is only present for button types 1 and 2
Border_width:	rs.w 1		; Width & height of border in pixels
Border_height:	rs.w 1

;*****************************************************************************
; This is the Symbol structure
	rsreset
Symbol_width:	rs.w 1		; Width & height of symbol in pixels
Symbol_height:	rs.w 1
Symbol_depth:	rs.w 1		; Depth & base colour
Symbol_base_colour:	rs.w 1
Symbol_ptr:	rs.l 1		; Pointer to graphics
