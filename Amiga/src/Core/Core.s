; Core collective
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 4-3-1994

	MODULE	DDT_Core

	OUTPUT	DDT:Objects/Core.o


	XREF	Init_program

	XREF	Clear_all_HDOBs
	XREF	Draw_HDOBs
	XREF	Erase_HDOBs

	XREF	VblQ_handler
	XREF	ScrQ_handler
	XREF	Update_display
	XREF	Handle_input
	XREF	Reset_root_stack
	XREF	Reset_module_stack

	XREF	Random
	XREF	Fill_memory
	XREF	Copy_memory

	XREF	Ink_colour
	XREF	Shadow_colour
	XREF	Line_buffer
	XREF	Reset_PA_stack
	XREF	Reset_Font_stack

	XREF	Marmor_slab
	XREF	Default_Mptr
	XREF	Memory_Mptr

	XREF	Get_file_memory_type
	XREF	Get_file_priority
	XREF	File_relocators

	XREF	Reload_map_data
	XREF	Remove_map_data
	XREF	Music_off
	XREF	Out_of_memory
	XREF	Music_flag
	XREF	Set_music
	XREF	Removing_music


	incdir	DDT:Constants/
	include	Global.i
	include	OS/OS.i
	include	Core.i
	include	Hardware_registers.i


	incdir	DDT:Core/
	include	Startup.s
	include	Graphics/Graphics.s
	include	Memory/Memory.s
	include	Colours.s
	include	Files.s
	include	Input.s
	include	Mouse.s
	include	Screen_slow.s
