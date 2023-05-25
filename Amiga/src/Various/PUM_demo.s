
	; Clicked on slot ?
	; Is slot empty ?

	jsr	Evaluate_item_PUM	; Do pop-up menu
	lea.l	Item_PUM,a0
	jsr	Do_PUM

Drop_item:

Learn_spell_from_item:

Examine_item:

Drink_item:

Use_spell_in_item:

Read_text_in_item:


Item_PUM:
Current_item_name:
	dc.l 0				; Address of menu title
	dc.w 6				; Number of entries
	dc.w 0,0			; Flags, blocked prompt number
	dc.l Drop_txt,Drop_item		; Text, function
	dc.w 0,0
	dc.l Learn_spell_txt,Learn_spell_from_item
	dc.w 0,0
	dc.l Examine_txt,Examine_item
	dc.w 0,0
	dc.l Drink_txt,Drink_item
	dc.w 0,0
	dc.l Use_spell_txt,Use_spell_in_item
	dc.w 0,0
	dc.l Read_txt,Read_text_in_item

Drop_txt:
	dc.b "Drop",0
Learn_spell_txt:
	dc.b "Learn spell",0
Examine_txt:
	dc.b "Examine",0
Drink_txt:
	dc.b "Drink",0
Use_spell_txt:
	dc.b "Use spell",0
Read_txt:
	dc.b "Read",0
	even
