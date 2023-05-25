; Data
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 25-3-1994

	MODULE	DDT_Data

	OUTPUT	DDT:Objects/Data.o


	XDEF	Sine_table

	incdir	DDT:Data/

	SECTION	Fast_DATA,data
Sine_table:
	incbin	Sine_table
