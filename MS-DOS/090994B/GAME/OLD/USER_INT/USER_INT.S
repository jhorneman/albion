; User interface collective
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 4-3-1994

	MODULE	DDT_User_interface

	OUTPUT	DDT:Objects/User_interface.o


	XREF	Add_object
	XREF	Delete_object
	XREF	Get_object_data
	XREF	Execute_method
	XREF	Execute_child_methods
	XREF	Push_Root
	XREF	Pop_Root
	XREF	Reset_root_stack
	XREF	Is_over_object
	XREF	Delete_self
	XREF	Wait_4_object

	XREF	Has_method
	XREF	Cycle_focus
	XREF	Move_highlight
	XREF	Dehighlight
	XREF	Current_highlighted_object
	XREF	Current_focussed_object

	XREF	Hide_HDOBs
	XREF	Show_HDOBs

	XREF	Mouse_Y
	XREF	Mouse_off
	XREF	Mouse_on
	XREF	Push_MA
	XREF	Pop_MA
	XREF	Button_state

	XREF	Random
	XREF	Wait_4_unclick
	XREF	Wait_4_rightunclick

	XREF	Update_screen
	XREF	Switch_screens

	XREF	Push_PA
	XREF	Pop_PA
	XREF	Erase_PA
	XREF	Process_text
	XREF	Process_text_list
	XREF	Display_text
	XREF	Display_text_list
	XREF	Display_processed_text
	XREF	Nr_of_lines
	XREF	Text_height
	XREF	Ink_colour
	XREF	Shadow_colour
	XREF	Put_text_line
	XREF	Put_centered_text_line
	XREF	Put_centered_box_text_line
	XREF	Get_line_length

	XREF	Get_block
	XREF	Put_masked_block
	XREF	Put_unmasked_block
	XREF	Plot_pixel
	XREF	Draw_hline
	XREF	Draw_vline
	XREF	Draw_box
	XREF	Darker_box
	XREF	Brighter_box
	XREF	Marmor_box
	XREF	Recolour_box

	XREF	Claim_pointer
	XREF	Free_pointer
	XREF	Allocate_CHIP
	XREF	Free_memory

	XREF	Change_Mptr
	XREF	Push_MA
	XREF	Pop_MA

	XREF	Help_line_handle
	XREF	Darker_table

	XREF	Default_Mptr
	XREF	Pop_up_Mptr
	XREF	Windows_H
	XREF	Windows_V
	XREF	Windows_corners


	incdir	DDT:Constants/
	include	Global.i
	include	Core.i
	include	Hull.i
	include	User_interface.i


	incdir	DDT:User_interface/
	include	Button.s
	include	General.s
	include	Interactor.s
;	include	Item_lists.s
;	include	Item_slots.s
	include	Pop_up_menu.s
	include	Primitives.s
	include	Scroll_bar.s
;	include	Text_lists.s
;	include	Text_slots.s
	include	Windows.s
