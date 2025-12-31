; OOUI Text Slot handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 23-2-1994

	XDEF	Text_slot_class

	SECTION	Program,code
;***************************************************************************
; [ Initialize method for text slot objects ]
;   IN : a0 - Pointer to text slot object (.l)
;        a1 - Pointer to text slot OID (.l)
; All registers are restored
;***************************************************************************
Init_text_slot:
	movem.l	d0/d1,-(sp)
	move.w	OID_TS_nr(a1),TS_nr(a0)	; Copy
	move.w	OID_X(a1),d0		; Set rectangle
	move.w	OID_Y(a1),d1
	add.w	X1(a0),d0
	add.w	Y1(a0),d1
	move.w	d0,X1(a0)
	move.w	d1,Y1(a0)
	add.w	OID_width(a1),d0
	sub.w	#1,d0
	add.w	#TS_height-1,d1
	move.w	d0,X2(a0)
	move.w	d1,Y2(a0)
	movem.l	(sp)+,d0/d1
	rts

;***************************************************************************
; [ Draw AND Update method for text slot objects ]
;   IN : a0 - Pointer to text slot object (.l)
; All registers are restored
;***************************************************************************
Draw_text_slot:
	movem.l	d0-d3/a0-a3,-(sp)
	jsr	Get_TS_data		; Get data
	move.l	a0,a2
	move.l	TL_get_text(a1),a3		; Get text
	jsr	(a3)
	move.w	X1(a2),d0			; Draw box
	move.w	Y1(a2),d1
	move.w	X2(a2),d2
	move.w	Y2(a2),d3
	jsr	Darker_box
	move.w	TL_text_colour(a1),Ink_colour	; Print text
	addq.w	#2,d0
	addq.w	#1,d1
	jsr	Put_text_line
	movem.l	(sp)+,d0-d3/a0-a3
	rts

;***************************************************************************
; [ Feedback method for text slot objects ]
;   IN : a0 - Pointer to text slot object (.l)
; All registers are restored
;***************************************************************************
Feedback_text_slot:
	movem.l	d0-d3/a0-a3,-(sp)
	jsr	Get_TS_data		; Get data
	move.l	a0,a2
	move.l	TL_get_text(a1),a3		; Get text
	jsr	(a3)
	move.w	X1(a2),d0			; Draw box
	move.w	Y1(a2),d1
	move.w	X2(a2),d2
	move.w	Y2(a2),d3
	jsr	Darker_box
	move.w	TL_text_feedback_colour(a1),Ink_colour	; Print text
	move.w	Shadow_colour,d2
	move.w	#-1,Shadow_colour
	addq.w	#2+1,d0
	addq.w	#2,d1
	jsr	Put_text_line
	move.w	d2,Shadow_colour
	jsr	Update_screen
	movem.l	(sp)+,d0-d3/a0-a3
	rts

;***************************************************************************
; [ Highlight method for text slot objects ]
;   IN : a0 - Pointer to text slot object (.l)
; All registers are restored
;***************************************************************************
Highlight_text_slot:
	movem.l	d0-d3/a0-a3,-(sp)
	jsr	Get_TS_data		; Get data
	move.l	a0,a2
	move.l	TL_get_text(a1),a3		; Get text
	jsr	(a3)
	move.w	X1(a2),d0			; Draw box
	move.w	Y1(a2),d1
	move.w	X2(a2),d2
	move.w	Y2(a2),d3
	jsr	Darker_box
	move.w	#Highlight_colour,Ink_colour	; Print text
	addq.w	#2,d0
	addq.w	#1,d1
	jsr	Put_text_line
	jsr	Update_screen
	movem.l	(sp)+,d0-d3/a0-a3
	rts

;***************************************************************************
; [ Text slot touched mouse event ]
;   IN : a0 - Pointer to text slot object (.l)
; No registers are restored
;***************************************************************************
Text_touched:
	jsr	Normal_highlighted		; Highlight
	jsr	Get_TS_data		; Call selected handler
	move.l	TL_touched(a1),a2
	jmp	(a2)

;***************************************************************************
; [ Text slot selected mouse event ]
;   IN : a0 - Pointer to text slot object (.l)
; No registers are restored
;***************************************************************************
Text_selected:
	jsr	Normal_clicked
	beq.s	.Exit
	jsr	Get_TS_data		; Call selected handler
	move.l	TL_selected(a1),a2
	jsr	(a2)
.Exit:	rts

;***************************************************************************
; [ Get text slot data ]
;   IN : a0 - Pointer to text slot object (.l)
;  OUT : d0 - Real slot number (.w)
;        a1 - Pointer to text list object (.l)
; Changed registers : d0,a1
;***************************************************************************
Get_TS_data:
	movem.l	a0/a2,-(sp)
	move.l	a0,a2
	move.w	Object_parent(a2),d0	; Get text list data
	jsr	Get_object_data
	move.l	a0,a1
	move.w	Object_child(a1),d0		; Get real slot number
	jsr	Get_object_data
	move.w	Scroll_bar_result(a0),d0
	add.w	TS_nr(a2),d0
	movem.l	(sp)+,a0/a2
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Text_slot_class:
	dc.l 0
	dc.w Text_slot_object_size
	Method Init,Init_text_slot
	Method Draw,Draw_text_slot
	Method Update,Draw_text_slot
	Method Feedback,Feedback_text_slot
	Method Highlight,Highlight_text_slot
	Method Mev,.Mev
	dc.w -1

.Mev:	dc.w $0202
	dc.l Text_selected
	dc.w $3300
	dc.l Text_touched
	dc.w -1
