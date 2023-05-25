; Bitmaps
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 17-2-1994

	MODULE	DDT_Bitmaps

	OUTPUT	DDT:Objects/Bitmaps.o


	XDEF	Normal_font
	XDEF	Techno_font
	XDEF	Default_Mptr
	XDEF	Pop_up_Mptr
	XDEF	Memory_Mptr
	XDEF	Disk_Mptr
	XDEF	Click_Mptr
	XDEF	Marmor_slab
	XDEF	Windows_H
	XDEF	Windows_V
	XDEF	Windows_corners

	incdir	DDT:Graphics/Ready/


	SECTION	Chip_DATA,data_c
Windows_H:
	incbin	Windows/Main_H
	incbin	Windows/Random_H1
	incbin	Windows/Random_H2
	incbin	Windows/Random_H3
	incbin	Windows/Random_H4
Windows_V:
	incbin	Windows/Main_V
	incbin	Windows/Random_V1
	incbin	Windows/Random_V2
	incbin	Windows/Random_V3
	incbin	Windows/Random_V4
Windows_corners:
	incbin	Windows/Normal_TL
	incbin	Windows/Normal_TR
	incbin	Windows/Normal_BL
	incbin	Windows/Normal_BR


	SECTION	Fast_DATA,data
Default_Mptr:
	dc.w 1,1,192
	incbin	Mouse_pointers/Default.mptr
Pop_up_Mptr:
	dc.w 1,1,192
	incbin	Mouse_pointers/Pop_up.mptr
Memory_Mptr:
	dc.w 16,16,208
	incbin	Mouse_pointers/Memory.mptr
Disk_Mptr:
	dc.w 16,16,208
	incbin	Mouse_pointers/Disk.mptr
Click_Mptr:
	dc.w 16,16,208
	incbin	Mouse_pointers/Click.mptr

Normal_font:
	incbin	Fonts/Normal_font
Techno_font:
	incbin	Fonts/Techno_font

Marmor_slab:
	incbin	Marmor_slab
