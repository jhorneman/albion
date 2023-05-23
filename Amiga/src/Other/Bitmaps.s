; Bitmaps
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 17-2-1994

	MODULE	DDT_Bitmaps

	OUTPUT	DDT:Objects/Bitmaps.o


	XDEF	Normal_font
	XDEF	Techno_font
	XDEF	Memory_Mptr
	XDEF	Default_Mptr
	XDEF	Marmor_slab

	incdir	DDT:Graphics/Ready/

	SECTION	Fast_DATA,data
Default_Mptr:
	dc.w 1,1
	incbin	Mouse_pointers/Sword.mptr
Memory_Mptr:
	dc.w 7,7
	incbin	Mouse_pointers/Memory.mptr

Normal_font:
	incbin	Fonts/Normal_font
Techno_font:
	incbin	Fonts/Techno_font

Marmor_slab:
	incbin	Marmor_slab
