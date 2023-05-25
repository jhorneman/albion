; Game collective
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 10-3-1994

	MODULE	DDT_Game

	OUTPUT	DDT:Objects/Game.o


	XREF	Exit_program
	XREF	Fatal_error
	XREF	Clear_cache
	XREF	My_vsync
	XREF	Dummy
	XREF	File_requester

	XREF	DDT_name
	XREF	Processor_type
	XREF	VBLs_per_second
	XREF	VBL_counter
	XREF	Extended_error_code

	XREF	Find_closest_colour
	XREF	Find_closest_colour2
	XREF	Set_full_palette
	XREF	Set_palette_part
	XREF	Current_palette

	XREF	Load_subfile	
	XREF	Load_unique_subfile
	XREF	Load_batch_of_subfiles
	XREF	Load_batch_of_unique_subfiles
	XREF	Load_file
	XREF	Save_file
	XREF	Save_encrypted_file
	XREF	Save_subfile
	XREF	Decompress
	XREF	Batch

	XREF	Begin_graphics_ops
	XREF	End_graphics_ops
	XREF	Push_CA
	XREF	Pop_CA
	XREF	Reset_CA_stack
	XREF	Coord_convert
	XREF	CA_Sp
	XREF	Put_unmasked_icon
	XREF	Put_masked_icon
	XREF	Put_unmasked_block
	XREF	Put_masked_block
	XREF	Put_masked_block2
	XREF	Put_masked_silhouette
	XREF	Put_line_buffer
	XREF	Put_line_buffer_shadow
	XREF	Duplicate_box
	XREF	Get_block
	XREF	VScroll_block
	XREF	Display_chunky_block
	XREF	Plot_pixel
	XREF	Get_pixel
	XREF	Draw_box
	XREF	Draw_vline
	XREF	Draw_hline
	XREF	Darker_box
	XREF	Brighter_box
	XREF	Marmor_box
	XREF	Recolour_box

	XREF	Clear_all_claims
	XREF	Get_pointer
	XREF	Claim_pointer
	XREF	Free_pointer
	XREF	Reallocate_memory
	XREF	Duplicate_memory
	XREF	Allocate_CHIP
	XREF	Allocate_FAST
	XREF	Allocate_memory
	XREF	File_allocate
	XREF	Shrink_memory
	XREF	Free_memory
	XREF	Kill_memory
	XREF	Clear_memory
	XREF	Get_memory_length
	XREF	Invalidate_memory
	XREF	Kill_unclaimed_memory
	XREF	Total_FAST_memory
	XREF	Default_memory_type

	XREF	Read_key
	XREF	Read_Mev
	XREF	Button_state
	XREF	Mouse_X
	XREF	Mouse_Y

	XREF	Mouse_on
	XREF	Mouse_off
	XREF	Reset_mouse
	XREF	Push_MA
	XREF	Pop_MA
	XREF	Reset_MA_stack
	XREF	Push_Mptr
	XREF	Push_busy_Mptr
	XREF	Pop_Mptr
	XREF	Reset_Mptr_stack
	XREF	Change_Mptr
	XREF	MA_Sp
	XREF	Default_MA

	XREF	Update_screen
	XREF	Copy_screen
	XREF	Clear_screen
	XREF	Switch_screens
	XREF	Work_screen
	XREF	On_screen
	XREF	Off_screen
	XREF	Screen_flag


	XREF	Create_DL_chain
	XREF	Destroy_DL_chain
	XREF	Add_DL_entry
	XREF	Delete_DL_entry
	XREF	For_all_DL
	XREF	Abort_for_all_DL

	XREF	Hide_HDOBs
	XREF	Show_HDOBs
	XREF	Reset_HDOBs
	XREF	Add_HDOB
	XREF	Remove_HDOB
	XREF	Clear_all_HDOBs
	XREF	Draw_HDOBs
	XREF	Erase_HDOBs

	XREF	Push_Module
	XREF	Pop_Module
	XREF	Reset_module_stack
	XREF	Handle_input
	XREF	Init_display
	XREF	Exit_display
	XREF	Update_display
	XREF	Get_module_ID
	XREF	Get_under_module_ID
	XREF	Find_module
	XREF	Key_or_mouse

	XREF	String_to_number
	XREF	Hex_convert
	XREF	SDecL_convert
	XREF	DecL_convert
	XREF	SDecR_convert
	XREF	DecR_convert
	XREF	Insert_sign

	XREF	Add_object
	XREF	Delete_object
	XREF	Get_object_data
	XREF	Execute_method
	XREF	Execute_child_methods
	XREF	Push_Root
	XREF	Pop_Root
	XREF	Reset_root_stack
	XREF	Is_over_object
	XREF	Dehighlight
	XREF	Delete_self
	XREF	Wait_4_object

	XREF	Print_number
	XREF	Print_large_number
	XREF	Get_random_50_100
	XREF	Shuttlesort
	XREF	Shellsort
	XREF	Delay
	XREF	Wait_4_user
	XREF	Wait_4_click
	XREF	Wait_4_unclick
	XREF	Wait_4_rightclick
	XREF	Wait_4_rightunclick
	XREF	Wait_4_key
	XREF	Reset_keyboard
	XREF	Reset_mouse_buffer
	XREF	Crypt_block
	XREF	Fill_memory
	XREF	Copy_memory
	XREF	Random
	XREF	Square_root

	XREF	Push_PA
	XREF	Pop_PA
	XREF	Erase_PA
	XREF	Reset_PA_stack
	XREF	Push_Font
	XREF	Pop_Font
	XREF	Reset_Font_stack
	XREF	Change_Font
	XREF	Strcpy
	XREF	Strncpy
	XREF	Strlen
	XREF	Put_text_line
	XREF	Put_centered_text_line
	XREF	Put_centered_box_text_line
	XREF	Process_text
	XREF	Get_line_length
	XREF	Get_line_size
	XREF	Ink_colour
	XREF	Shadow_colour
	XREF	Line_buffer
	XREF	Default_PA
	XREF	Display_text
	XREF	Display_text_list

	XREF	Horizontal_wipe
	XREF	Vertical_wipe
	XREF	Random_wipe


	XREF	Normal_touched
	XREF	Normal_clicked
	XREF	Normal_rightclicked

	XREF	Set_rectangle
	XREF	Earth_class
	XREF	Window_class
	XREF	Text_area_class
	XREF	Init_text_area
	XREF	Erase_text_area

	XREF	BT_Button_class
	XREF	BS_Button_class
	XREF	TS_Button_class
	XREF	BT_Switch_class
	XREF	BS_Switch_class
	XREF	TS_Switch_class
	XREF	Radio_class
	XREF	BT_Radio_button_class
	XREF	BS_Radio_button_class
	XREF	TS_Radio_button_class

	XREF	Item_list_class
	XREF	Item_slot_class
	XREF	Do_PUM
	XREF	Scroll_bar_class
	XREF	Text_list_class
	XREF	Text_slot_class

	XREF	Draw_symbol
	XREF	Draw_rectangle
	XREF	Draw_high_border
	XREF	Draw_deep_border
	XREF	Draw_normal_box
	XREF	Draw_high_box
	XREF	Draw_deep_box


	XREF	Main_palette
	XREF	Sine_table


	XDEF	Init_program
	XDEF	Default_module
	XDEF	Wait_4_user_Mod
	XDEF	Number
	XDEF	Relocate_3D_wall
	XDEF	Relocate_3D_floor_buffer
	XDEF	Help_line_handle
	XDEF	Darker_table


	incdir	DDT:Constants/
	include	Global.i
	include	Hardware_registers.i
	include	OS/OS.i
	include	Core.i
	include	Hull.i
	include	User_interface.i
	include	Game/Game.i


	incdir	DDT:Game/
	include	Main.s
	include	Modules.s
	include	Map/Map.s
;	include	Map/2D_map.s
	include	Map/3D_map.s
	include	Map/3D_dungeon.s
	include	Map/3D_graphics.s
	include	Map/Animation.s

