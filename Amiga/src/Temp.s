
	MODULE	DDT_Temporary

	OUTPUT	DDT:Objects/Temporary.o

	XDEF	Music_off
	XDEF	Remove_map_data
	XDEF	HLC_table
	XDEF	Set_music
	XDEF	Diagnostics_list1
	XDEF	Diagnostics_list2
	XDEF	Reload_map_data
	XDEF	Removing_music
	XDEF	Relocate_music
	XDEF	Music_flag
	XDEF	Out_of_memory
	XDEF	Disk_error_handler

	XDEF	Icons
	XDEF	Mannetjes
	XDEF	Test_pal

	XREF	Fatal_error
	XREF	Exit_program
	XREF	Save_screen

	incdir	DDT:Constants/
	include	Global.i


	SECTION	Program,code
Music_off:
Set_music:
Removing_music:
Relocate_music:
Out_of_memory:
Remove_map_data:
Reload_map_data:
	rts

Disk_error_handler:
	move.l	#DISK_ERROR,d0
	jmp	Fatal_error

	incdir	DDT:Graphics/Ready/Test/

	SECTION	Chip_DATA,data_c
Icons:	incbin	Underlay1
	incbin	Underlay2
	incbin	Overlay1
	incbin	Overlay2
	incbin	Overlay3
Mannetjes:	incbin	Mannetje_up
	incbin	Mannetje_right
	incbin	Mannetje_down
	incbin	Mannetje_left


	SECTION	Fast_DATA,data
Test_pal:	incbin	Test_192.pal

Diagnostics_list1:
	dc.l $000000ff," ",Exit_program
	dc.l $000000ff,"q",Save_screen
	dc.l 0
Darkness:
HLC_table:
Diagnostics_list2:
Music_flag:
Googa:
	dcb.l 20,0
