; General objects
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 28-2-1994

	XDEF	Set_rectangle
	XDEF	Earth_class
	XDEF	Text_area_class
	XDEF	Init_text_area
	XDEF	Erase_text_area

	SECTION	Program,code
;***************************************************************************	
; [ Standard rectangle initialization ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to standard OID (.l)
; All registers are restored
;***************************************************************************
Set_rectangle:
	movem.l	d0/d1,-(sp)
	move.w	OID_X(a1),d0
	move.w	OID_Y(a1),d1
	add.w	X1(a0),d0
	add.w	Y1(a0),d1
	move.w	d0,X1(a0)
	move.w	d1,Y1(a0)
	add.w	OID_width(a1),d0
	add.w	OID_height(a1),d1
	subq.w	#1,d0
	subq.w	#1,d1
	move.w	d0,X2(a0)
	move.w	d1,Y2(a0)
	movem.l	(sp)+,d0/d1
	rts

;***************************************************************************	
; [ Initialize method for text area objects ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to OID (.l)
; All registers are restored
;***************************************************************************
Init_text_area:
	move.l	a2,-(sp)
	jsr	Set_rectangle
	lea.l	Text_area_PA(a0),a2		; Copy to PA
	move.w	X1(a0),X1(a2)
	move.w	Y1(a0),Y1(a2)
	move.w	X2(a0),X2(a2)
	move.w	Y2(a0),Y2(a2)
	move.w	OID_Ink(a1),PA_Ink(a2)	; Set rest of PA
	move.w	OID_Shadow(a1),PA_Shadow(a2)
	move.w	OID_Paper(a1),PA_Paper(a2)
	move.l	(sp)+,a2
	rts

;***************************************************************************	
; [ Print method for text area objects ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to text (.l)
; All registers are restored
;***************************************************************************
Print_text_area:
	movem.l	a0/a1,-(sp)
	lea.l	Text_area_PA(a0),a0
	jsr	Push_PA
	move.l	a1,a0
	jsr	Display_text
	Pop	PA
	movem.l	(sp)+,a0/a1
	rts

;***************************************************************************	
; [ Erase method for text objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Erase_text_area:
	movem.l	a0/a1,-(sp)
	lea.l	Text_area_PA(a0),a0
	jsr	Push_PA
	jsr	Erase_PA
	Pop	PA
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Earth_class:
	dc.l 0
	dc.w Earth_object_size
	Method Init,Set_rectangle
	Method Touched,Normal_touched
	dc.w -1
Text_area_class:
	dc.l 0
	dc.w Text_area_object_size
	Method Init,Init_text_area
	Method Print,Print_text_area
	Method Erase,Erase_text_area
	dc.w -1
