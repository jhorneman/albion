; Hull collective
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 4-3-1994

	MODULE	DDT_Hull

	OUTPUT	DDT:Objects/Hull.o


	XREF	Claim_pointer
	XREF	Free_pointer
	XREF	Allocate_CHIP
	XREF	Allocate_memory
	XREF	Free_memory
	XREF	Clear_memory
	XREF	Clear_all_claims

	XREF	Screen_flag
	XREF	Update_screen
	XREF	Switch_screens

	XREF	Push_CA
	XREF	Pop_CA
	XREF	Draw_box
	XREF	Get_block
	XREF	Put_masked_block
	XREF	Put_unmasked_block
	XREF	Put_line_buffer
	XREF	Put_line_buffer_shadow

	XREF	HLC_table
	XREF	Diagnostics_list1
	XREF	Diagnostics_list2
	XREF	Default_module

	XREF	Push_MA
	XREF	Push_Mptr
	XREF	Pop_MA
	XREF	Pop_Mptr
	XREF	Default_MA
	XREF	Mouse_X
	XREF	Mouse_Y
	XREF	Button_state

	XREF	Read_Mev
	XREF	Read_key

	XREF	Fatal_error
	XREF	My_vsync
	XREF	VBL_counter

	XREF	Default_Mptr
	XREF	Normal_font
	XREF	Techno_font


	incdir	DDT:Constants/
	include	Global.i
	include	Core.i
	include	Hull.i
	include	User_interface.i


	incdir	DDT:Hull/
	include	Dynamic_lists.s
	include	HDOBs.s
	include	Module_control.s
	include	Numeric.s
	include	OOUI.s
	include	Various.s
	include	Text/Text.s
