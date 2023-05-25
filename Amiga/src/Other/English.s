; English in-game texts
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 24-3-1994

	MODULE	DDT_English

	OUTPUT	DDT:Objects/English.o

	XDEF	Dictionary_filename

	SECTION	Data_FAST,data
Dictionary_filename:
	dc.b "Dictionary.german",0
	even
