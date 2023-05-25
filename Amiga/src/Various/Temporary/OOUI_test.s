
	rsreset
	rs.b 1
Init_method:	rs.b 1
Exit_method:	rs.b 1
Draw_method:	rs.b 1
Update_method:	rs.b 1
Feedback_method:	rs.b 1
Clicked_left_method:	rs.b 1
Clicked_right_method:	rs.b 1
Print_method:	rs.b 1

; *** Visible class ***
	rsreset
	rs.b Base_class_size
Draw_method:	rs.l 1		; Draw object
Visible_class_size:	rs.b 0

; *** Interaction class ***
	rsreset
	rs.b Visible_class_size
Update_method:	rs.l 1		; Update object
Feedback_method:	rs.l 1
Object_KevL:	rs.l 1		; Key Event List
Object_MevL:	rs.l 1		; Mouse Event List
Interaction_class_size:	rs.b 0

; *** Box class ***
	rsreset
	rs.b Visible_class_size
Box_colour:	rs.w 1
Box_class_size:	rs.w 1

; *** Bitmap class ***
	rsreset
	rs.b Visible_class_size
Bitmap_bitmap:	rs.b Bitmap_data_size
Bitmap_class_size:	rs.b 0

; *** Button class ***
	rs.b Interaction_class_size
Button_data_ptr:	rs.l 1
Button_type:	rs.b 1
Button_state:	rs.b 1		; 0 = Up, 1 = Down
Button_data_size:	rs.b 0

; ...

	rsreset
Bitmap_width:	rs.w 1		; Width in truncs
Bitmap_height:	rs.w 1		; Height in pixels
Bitmap_depth:	rs.w 1		; Depth in planes
Bitmap_base_colour:	rs.w 1		; Base colour
Bitmap_handle:	rs.b 1		; Graphics handle (0 = absolute)
	rseven
Bitmap_offset:	rs.l 1		; Offset to graphics
Bitmap_data_size:	rs.b 0























; Notes :
;   The scroll bar Update method will call the Update method of
;   OID_Link_object_handle.

;***************************************************************************	
; Button object
; OID :
	rsreset
OID_Button_X:	rs.w 1		; Coordinates of button
OID_Button_Y:	rs.w 1
OID_Button_data_ptr:	rs.l 1	; Pointer to text or Symbol structure
Button_OID_size:	rs.b 0

; Class :
	rsreset
	rs.b Base_class_size
Button_data_ptr:	rs.l 1
Button_type:	rs.b 1
Button_state:	rs.b 1		; 0 = Up, 1 = Down
Button_class_size:	rs.b 0

;***************************************************************************	
; Border Button object
; OID :
	rsreset
	rs.b Button_OID_size
OID_Border_width:	rs.w 1		; Width & height of border in pixels
OID_Border_height:	rs.w 1
Border_button_OID_size:	rs.b 0

; Class :
	rsreset
	rs.b Button_class_size
Border_width:	rs.w 1		; Width & height of border in pixels
Border_height:	rs.w 1
Border_button_class_size:	rs.b 0

;***************************************************************************	
; Item list object
; OID :
	rsreset
OID_IL_X:	rs.w 1			; Coordinates of item list
OID_IL_Y:	rs.w 1
OID_IL_draw:	rs.l 1		; Pointer to a routine which draws the entire item list
OID_IL_touched:	rs.l 1		; Pointer to a routine which is called when the player moves the mouse over an item slot
OID_IL_clicked_left:	rs.l 1	; Pointer to a routine which is called when the player left-clicks on an item slot
OID_IL_clicked_right:	rs.l 1	; Pointer to a routine which is called when the player right-clicks on an item slot
OID_IL_nr_items:	rs.w 1		; Number of item slots in inventory
OID_IL_slots_handle:	rs.b 1	; Memory handle of inventory (0=absolute address)
	rseven
OID_IL_slots_offset:	rs.l 1	; Long offset to inventory
OID_IL_take_out:	rs.l 1		; Pointer to a routine which takes an item out of the inventory
OID_IL_put_in:	rs.l 1		; Pointer to a routine which puts an item in the inventory
OID_IL_PUM:	rs.l 1		; Pointer to a pop-up menu data structure
Item_list_OID_size:	rs.b 0

; Class :
	rsreset
	rs.b Base_class_size
IL_nr_items:	rs.w 1
IL_slots_handle:	rs.b 1
	rseven
IL_slots_offset:	rs.l 1
IL_take_out:	rs.l 1
IL_put_in:	rs.l 1
IL_scroll_bar_handle:	rs.w 1
IL_PUM:	rs.l 1
Item_list_class_size:	rs.b 0

;***************************************************************************	
; Item slot object
; OID :
	rsreset
OID_Item_X:	rs.w 1
OID_Item_Y:	rs.w 1
Item_slot_OID_size:	rs.b 0

; Class :
	rsreset
	rs.b Base_class_size
Item_slot_class_size:	rs.b 0

;***************************************************************************	
; Scroll bar object
; OID :
	rsreset
OID_Total_units:	rs.w 1		; Total amount of units
OID_Units_width:	rs.w 1		; Number of units per row
OID_Units_height:	rs.w 1		; Number of rows on screen
OID_Scroll_bar_X:	rs.w 1		; Coordinates of scroll bar
OID_Scroll_bar_Y:	rs.w 1
OID_Scroll_bar_height:	rs.w 1	; Height of scroll bar in pixels
OID_Sub_object_handle:	rs.w 1
Scroll_bar_OID_size:	rs.b 0

; Class :
	rsreset
	rs.b Base_class_size
Total_units:	rs.w 1		; Total amount of units
Units_width:	rs.w 1		; Number of units per row
Units_height:	rs.w 1		; Number of rows on screen
Scroll_bar_height:	rs.w 1		; Height of scroll bar in pixels
Slider_height:	rs.w 1		; Height of slider in pixels
Slider_Y:	rs.w 1			; Y-coordinate of slider
Row_height:	rs.w 1		; Height of row in pixels
Current_row:	rs.w 1		; Current row
Scroll_bar_max_height:	rs.w 1	; Maximum Y-coordinate of slider
Scroll_bar_result:	rs.w 1		; Current unit selected by scroll bar {0...}
Scroll_bar_class_size:	rs.b 0





; Base class definition :

	Define_class Base,0
	dc.w 0
	End_class Base

; Button class definition :

	Define_class Button,Base
	dc.l Init_button,0,Draw_button,Draw_button
	dc.l 0,.Mev
	End_class Button

.Mev:	dc.l 0

; Border Button class definition :

	Define_class Border_button,Button
	dc.l Init_border_button,0,0,0
	dc.l 0,0
	End_class Border_button

; Border + Text Button class definition :

	Define_class BT_button,Border_button
	dc.l Init_BT_button,0,0,0
	dc.l 0,0
	End_class BT_button

; Border + Symbol Button class definition :

	Define_class BS_button,Border_button
	dc.l Init_BS_button,0,0,0
	dc.l 0,0
	End_class BS_button

; Two Symbols Button class definition :

	Define_class TS_button,Button
	dc.l Init_TS_button,0,0,0
	dc.l 0,0
	End_class TS_button

; Item list class definition :

	Define_class Item_list,Base
	dc.l Init_item_list,Exit_item_list,0,0
	dc.l 0,0
	End_class Item_list

; Item slot class definition :

	Define_class Item_slot,Base
	dc.l 0,0,Draw_item_slot,Draw_item_slot
	dc.l 0,.Mev
	End_class Item_slot

.Mev:	dc.l 0

; Scroll bar class definition :

	Define_class Scroll_bar,Base
	dc.l Init_scroll_bar,Exit_scroll_bar,Draw_scroll_bar,0
	dc.l 0,.Mev
	End_class Scroll_bar

.Mev:	dc.l 0
