; OOUI Item List handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 25-1-1994

	XDEF	Item_list_class

	SECTION	Program,code
;***************************************************************************
; [ Initialize method for item list objects ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to OID (.l)
; All registers are restored
;***************************************************************************
Init_item_list:
	movem.l	d0-d3/d6-d7/a0-a2,-(sp)
; ---------- Initialize object --------------------
	move.w	OID_IL_width(a1),IL_width(a0)	; Copy
	move.w	OID_IL_height(a1),IL_height(a0)
	move.w	OID_IL_nr_items(a1),IL_nr_items(a0)
	move.b	OID_IL_slots_handle(a1),IL_slots_handle(a0)
	move.l	OID_IL_slots_offset(a1),IL_slots_offset(a0)
	move.l	OID_IL_touched(a1),IL_touched(a0)
	move.l	OID_IL_selected(a1),IL_selected(a0)
	move.l	OID_IL_PUM(a1),IL_PUM(a0)
	move.l	a1,a2			; Protect
	move.w	OID_IL_width(a2),d2		; Calculate width & height
	move.w	OID_IL_height(a2),d3
	mulu.w	#IS_outer_width,d2
	mulu.w	#IS_outer_height,d3
	addq.w	#1,d3
	move.w	OID_X(a1),d0		; Set rectangle
	move.w	OID_Y(a1),d1
	add.w	X1(a0),d0
	add.w	Y1(a0),d1
	move.w	d0,X1(a0)
	move.w	d1,Y1(a0)
	add.w	d2,d0
	add.w	d3,d1
	add.w	#Between+Scroll_bar_width-IS_between_X+2,d0
	subq.w	#1,d1
	move.w	d0,X2(a0)
	move.w	d1,Y2(a0)
; ---------- Make scroll bar ----------------------
	Make_OID	Scroll_bar,a1		; Make
	move.w	IL_nr_items(a0),OID_Total_units(a1)
	move.w	OID_IL_width(a2),OID_Units_width(a1)
	move.w	OID_IL_height(a2),OID_Units_height(a1)
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
; ---------- Make item slots ----------------------
	Make_OID	Item_slot,a1		; Prepare item slot OID
	move.w	#1,OID_IS_nr(a1)
	move.w	#1,OID_Y(a1)		; Do
	move.w	OID_IL_height(a2),d7
	subq.w	#1,d7
.Loop_Y:	move.w	#1,OID_X(a1)
	move.w	OID_IL_width(a2),d6
	subq.w	#1,d6
.Loop_X:	movem.l	d0/a0,-(sp)		; Add item slot
	lea.l	Item_slot_class,a0
	jsr	Add_object
	movem.l	(sp)+,d0/a0
	addq.w	#1,OID_IS_nr(a1)		; Next
	add.w	#IS_outer_width,OID_X(a1)
	dbra	d6,.Loop_X
	add.w	#IS_outer_height,OID_Y(a1)
	dbra	d7,.Loop_Y
	Free_OID
	movem.l	(sp)+,d0-d3/d6-d7/a0-a2
	rts

;***************************************************************************	
; [ Focus method for item list objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Focus_item_list:
	movem.l	d0-d4/a0,-(sp)
	move.w	X1(a0),d0			; Display focus
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	sub.w	d0,d2
	sub.w	d1,d3
	subq.w	#1,d0
	subq.w	#1,d1
	addq.w	#2,d2
	addq.w	#2,d3
	move.w	#Focus_colour,d4
	jsr	Draw_rectangle
	tst.w	Current_highlighted_object	; Anything highlighted ?
	bne.s	.Yes
	move.w	Object_child(a0),d0		; No -> Select
	jsr	Get_object_data
	move.w	Object_child(a0),d0
	move.w	d0,Current_highlighted_object
	moveq.l	#Highlight_method,d1
	jsr	Execute_method
	jsr	Update_screen
	movem.l	(sp)+,d0-d4/a0
	rts

	ifne	FALSE
;***************************************************************************	
; [ Move method for item list objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Move_item_list:
	movem.l	d0/d1/d7/a0-a2,-(sp)
	move.w	Current_highlighted_object,d7	; Get currently highlighted
	move.l	a0,a1			;  object
	move.w	d7,d0
	jsr	Get_object_data
	exg.l	a0,a1
	move.w	Key_parameter,d0		; Get parameter
	move.l	.Ptrs(pc,d0.w*4),a2		; Execute move function
	jsr	(a2)
	cmp.w	Current_highlighted_object,d7	; Any change ?
	beq.s	.Exit1
	move.w	d7,d0			; Yes -> Redraw old
	moveq.l	#Draw_method,d1
	jsr	Execute_method
	move.w	Current_highlighted_object,d0	; Highlight new
	moveq.l	#Highlight_method,d1
	jsr	Execute_method
	jsr	Update_screen
.Exit1:	movem.l	(sp)+,d0/d1/d7/a0-a2
	rts

.Ptrs:	dc.l .Up,.Right,.Down,.Left

.Up:	move.w	IS_nr(a1),d0		; Can move up ?
	move.w	IL_width(a0),d1
	sub.w	d1,d0
	bmi.s	.Exit2
	move.w	d0,Current_highlighted_object	; Yes
.Exit2:	rts

.Right:
	rts

.Down:
	rts

.Left:
	rts
	endc

;***************************************************************************	
; [ Draw method for item list objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Draw_item_list:
	movem.l	d0-d3/a0,-(sp)
	move.w	X1(a0),d0			; Draw surrounding box
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	sub.w	d0,d2
	sub.w	d1,d3
	sub.w	#Between+Scroll_bar_width-IS_between_X-2,d2
	addq.w	#2+1,d3
	subq.w	#1,d0
	subq.w	#1,d1
	jsr	Draw_deep_box
	move.w	Object_self(a0),d0		; Draw children
	moveq.l	#Draw_method,d1
	jsr	Execute_child_methods
	movem.l	(sp)+,d0-d3/a0
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Item_list_class:
	dc.l Intergroup_class
	dc.w Item_list_object_size
	Method Init,Init_item_list
	Method Focus,Focus_item_list
;	Method Move,Move_item_list
	Method Draw,Draw_item_list
	Method Update,Execute_child_methods
	dc.w -1
