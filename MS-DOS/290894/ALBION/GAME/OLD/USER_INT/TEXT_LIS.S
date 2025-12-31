; OOUI Text List handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 7-3-1994

	XDEF	Text_list_class

	SECTION	Program,code
;***************************************************************************
; [ Initialize method for text list objects ]
;   IN : a0 - Pointer to text list object (.l)
;        a1 - Pointer to text list OID (.l)
; All registers are restored
;***************************************************************************
Init_text_list:
	movem.l	d0-d3/d6-d7/a0-a2,-(sp)
; ---------- Initialize object --------------------
	move.w	OID_TL_nr_texts(a1),TL_nr_texts(a0)	; Copy
	move.w	OID_TL_text_colour(a1),TL_text_colour(a0)
	move.w	OID_TL_text_feedback_colour(a1),TL_text_feedback_colour(a0)
	move.l	OID_TL_get_text(a1),TL_get_text(a0)
	move.l	OID_TL_touched(a1),TL_touched(a0)
	move.l	OID_TL_selected(a1),TL_selected(a0)
	move.l	a1,a2			; Protect
	move.w	OID_TL_width(a2),d2		; Calculate width & height
	move.w	OID_TL_height(a2),d3
	mulu.w	OID_width(a2),d2
	mulu.w	#TS_height,d3
	addq.w	#2,d3
	move.w	OID_X(a1),d0		; Set rectangle
	move.w	OID_Y(a1),d1
	add.w	X1(a0),d0
	add.w	Y1(a0),d1
	move.w	d0,X1(a0)
	move.w	d1,Y1(a0)
	add.w	d2,d0
	add.w	d3,d1
	add.w	#Between+Scroll_bar_width+2,d0
	subq.w	#1,d1
	move.w	d0,X2(a0)
	move.w	d1,Y2(a0)
; ---------- Make scroll bar ----------------------
	Make_OID	Scroll_bar,a1		; Make
	move.w	TL_nr_texts(a0),OID_Total_units(a1)
	move.w	OID_TL_width(a2),OID_Units_width(a1)
	move.w	OID_TL_height(a2),OID_Units_height(a1)
	move.w	d2,d0
	add.w	#Between,d0
	move.w	d0,OID_X(a1)
	clr.w	OID_Y(a1)
	move.w	d3,OID_Scroll_bar_height(a1)
	move.w	Object_self(a0),d0		; Add
	move.l	a0,-(sp)
	lea.l	Scroll_bar_class,a0
	jsr	Add_object
	move.l	(sp)+,a0
	Free_OID
; ---------- Make text slots ----------------------
	Make_OID	Text_slot,a1		; Prepare text slot OID
	move.w	OID_width(a2),d3
	move.w	d3,OID_width(a1)
	move.w	#1,OID_TS_nr(a1)
	move.w	#1,OID_Y(a1)		; Do
	move.w	OID_TL_height(a2),d7
	subq.w	#1,d7
.Loop_Y:	move.w	#1,OID_X(a1)
	move.w	OID_TL_width(a2),d6
	subq.w	#1,d6
.Loop_X:	movem.l	d0/a0,-(sp)		; Add text slot
	lea.l	Text_slot_class,a0
	jsr	Add_object
	movem.l	(sp)+,d0/a0
	addq.w	#1,OID_TS_nr(a1)		; Next
	add.w	d3,OID_X(a1)
	dbra	d6,.Loop_X
	add.w	#TS_height,OID_Y(a1)
	dbra	d7,.Loop_Y
	Free_OID
	movem.l	(sp)+,d0-d3/d6-d7/a0-a2
	rts

;***************************************************************************	
; [ Draw method for text list objects ]
;   IN : a0 - Pointer to text list object (.l)
; All registers are restored
;***************************************************************************
Draw_text_list:
	movem.l	d0-d3/a0,-(sp)
	move.w	X1(a0),d0			; Draw surrounding box
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	sub.w	d0,d2
	sub.w	d1,d3
	sub.w	#Between+Scroll_bar_width-2,d2
	addq.w	#2+1,d3
	subq.w	#1,d0
	subq.w	#1,d1
	jsr	Draw_deep_box
	move.w	Object_child(a0),d0		; Draw children
	moveq.l	#Draw_method,d1
	jsr	Execute_method
	movem.l	(sp)+,d0-d3/a0
	rts

;***************************************************************************	
; [ Update method for text list objects ]
;   IN : a0 - Pointer to text list object (.l)
; All registers are restored
;***************************************************************************
Update_text_list:
	movem.l	d0/d1,-(sp)
	move.w	Object_self(a0),d0		; Update all children
	moveq.l	#Update_method,d1
	jsr	Execute_child_methods
	movem.l	(sp)+,d0/d1
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Text_list_class:
	dc.l 0
	dc.w Text_list_object_size
	Method Init,Init_text_list
	Method Draw,Draw_text_list
	Method Update,Update_text_list
	Method Mev,.Mev
	dc.w -1

.Mev:	dc.w $3300
	dc.l Dehighlight
	dc.w -1
