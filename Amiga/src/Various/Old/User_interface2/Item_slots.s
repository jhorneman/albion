; OOUI Item Slot handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 23-2-1994

	XDEF	Item_slot_class

	SECTION	Program,code
;***************************************************************************
; [ Initialize method for item slot objects ]
;   IN : a0 - Pointer to item slot object (.l)
;        a1 - Pointer to item slot OID (.l)
; All registers are restored
;***************************************************************************
Init_item_slot:
	movem.l	d0/d1,-(sp)
	move.w	OID_IS_nr(a1),IS_nr(a0)	; Copy
	move.w	OID_X(a1),d0		; Set rectangle
	move.w	OID_Y(a1),d1
	add.w	X1(a0),d0
	add.w	Y1(a0),d1
	move.w	d0,X1(a0)
	move.w	d1,Y1(a0)
	add.w	#IS_inner_width-1,d0
	add.w	#IS_inner_height-1,d1
	move.w	d0,X2(a0)
	move.w	d1,Y2(a0)
	movem.l	(sp)+,d0/d1
	rts

;***************************************************************************
; [ Draw AND Update method for item slot objects ]
;   IN : a0 - Pointer to item slot object (.l)
; All registers are restored
;***************************************************************************
Draw_item_slot:
	movem.l	d0-d3,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.w	d0,d2
	move.w	d1,d3
	add.w	#IS_inner_width-1,d2
	add.w	#IS_inner_height-1,d3
	jsr	Darker_box
	jsr	Draw_item
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************
; [ Feedback method for item slot objects ]
;   IN : a0 - Pointer to item slot object (.l)
; All registers are restored
;***************************************************************************
Feedback_item_slot:
	movem.l	d0-d3,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	moveq.l	#IS_inner_width,d2
	moveq.l	#IS_inner_height,d3
	jsr	Draw_deep_box
	addq.w	#1,Y1(a0)
	jsr	Draw_item
	subq.w	#1,Y1(a0)
	jsr	Update_screen
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************
; [ Highlight method for item slot objects ]
;   IN : a0 - Pointer to item slot object (.l)
; All registers are restored
;***************************************************************************
Highlight_item_slot:
	movem.l	d0-d3,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.w	d0,d2
	move.w	d1,d3
	add.w	#IS_inner_width-1,d2
	add.w	#IS_inner_height-1,d3
	jsr	Brighter_box
	jsr	Draw_item
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************
; [ Draw an item ]
;   IN : a0 - Pointer to item slot object (.l)
; All registers are restored
;***************************************************************************
Draw_item:
	movem.l	d0-d2/d4-d7/a0/a1,-(sp)
	jsr	Get_IS_data
	move.w	d0,d2
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.w	#192,d4
	moveq.l	#6,d5
	moveq.l	#1,d6
	moveq.l	#16,d7
;	lea.l	Object_graphics,a0
	and.w	#$0003,d2
	mulu.w	#192,d2
	add.w	d2,a0
	jsr	Put_masked_block
	movem.l	(sp)+,d0-d2/d4-d7/a0/a1
	rts

;***************************************************************************
; [ Item slot touched mouse event ]
;   IN : a0 - Pointer to item slot object (.l)
; No registers are restored
;***************************************************************************
Item_touched:
	jsr	Normal_highlighted		; Highlight
	jsr	Get_IS_data
	move.l	IL_touched(a2),a1		; Call touched handler
	jmp	(a1)

;***************************************************************************
; [ Item slot selected mouse event ]
;   IN : a0 - Pointer to item slot object (.l)
; No registers are restored
;***************************************************************************
Item_selected:
	jsr	Normal_clicked
	beq.s	.Exit
	jsr	Get_IS_data
	move.l	IL_selected(a2),a1		; Call selected handler
	jsr	(a1)
.Exit:	rts

;***************************************************************************
; [ Item slot manipulated mouse event ]
;   IN : a0 - Pointer to item slot object (.l)
; No registers are restored
;***************************************************************************
Item_manipulated:
	jsr	Normal_right_clicked
	beq	.Exit
	move.w	Object_self(a0),d0		; Highlight slot
	moveq.l	#Highlight_method,d1
	jsr	Execute_method
	jsr	Update_screen
	jsr	Get_IS_data
	Make_OID	PUM,a1			; Make pop-up menu
	move.w	X2(a0),OID_X(a1)
	move.w	Y2(a0),OID_Y(a1)
	move.l	IL_PUM(a2),OID_PUM_data(a1)
	move.l	a0,-(sp)
	lea.l	Pop_up_menu_class,a0	; Add
	moveq.l	#-1,d0
	jsr	Add_object
	move.l	(sp)+,a0
	Free_OID
	move.w	#Draw_method,d1		; Display
	jsr	Execute_method
	jsr	Update_screen
	jsr	Wait_4_object		; Wait
	move.w	Object_self(a0),d0		; Redraw slot
	moveq.l	#Draw_method,d1
	jsr	Execute_method
	jsr	Update_screen
.Exit:	rts

;***************************************************************************
; [ Get item slot data ]
;   IN : a0 - Pointer to item slot object (.l)
;  OUT : d0 - Real slot number (.w)
;        a2 - Pointer to item list object (.l)
; Changed registers : d0,a2
;***************************************************************************
Get_IS_data:
	movem.l	a0/a1,-(sp)
	move.l	a0,a1
	move.w	Object_parent(a1),d0	; Get item list data
	jsr	Get_object_data
	move.l	a0,a2
	move.w	Object_child(a2),d0		; Get real slot number
	jsr	Get_object_data
	move.w	Scroll_bar_result(a0),d0
	add.w	IS_nr(a1),d0
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Item_slot_class:
	dc.l 0
	dc.w Item_slot_object_size
	Method Init,Init_item_slot
	Method Draw,Draw_item_slot
	Method Update,Draw_item_slot
	Method Feedback,Feedback_item_slot
	Method Highlight,Highlight_item_slot
	Method Mev,.Mev
	dc.w -1

.Mev:	dc.w $0202
	dc.l Item_selected
	dc.w $2020
	dc.l Item_manipulated
	dc.w $3300
	dc.l Item_touched
	dc.w -1
