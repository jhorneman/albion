; Palettes
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 17-2-1994

	MODULE	DDT_Palettes

	OUTPUT	DDT:Objects/Palettes.o


	XDEF	Main_palette

	incdir	DDT:Graphics/Ready/

	SECTION	Fast_DATA,data
Main_palette:
	incbin	Palettes/Main_palette
