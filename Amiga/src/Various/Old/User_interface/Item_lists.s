; Item List handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 25-1-1994

	MODULE	DDT_Item_lists

	OUTPUT	DDT:Objects/User_interface/Item_lists.o


	XREF	Draw_deep_box
	XREF	Add_scroll_bar

	XDEF	Add_IL
	XDEF	Remove_IL
	XDEF	Remove_all_ILs
	XDEF	Init_normal_IL
	XDEF	Draw_normal_IL
	XDEF	Display_all_items
	XDEF	Item_touched
	XDEF	Item_selected
	XDEF	Item_manipulated


	incdir	DDT:
	include	Constants/Global.i
	include	Constants/Core.i
	include	Constants/Hull.i
	include	Constants/User_interface.i


	SECTION	Program,code
;***************************************************************************
; [ Add an Item List ]
;   IN : a0 - Pointer to Item List data (.l)
; All registers are restored
;***************************************************************************
Add_IL:
	movem.l	d7/a1,-(sp)
	lea.l	IL_list,a1		; Search free entry
	moveq.l	#Max_ILs-1,d7
.Loop:	tst.l	(a1)+			; Free ?
	bne.s	.Next
	move.l	a0,-4(a1)			; Yes -> Insert
	bra.s	.Exit
.Next:	dbra	d7,.Loop			; Next
	; NO FREE ENTRY !
.Exit:	movem.l	(sp)+,d7/a1
	rts

;***************************************************************************
; [ Remove an Item List ]
;   IN : a0 - Pointer to Item List data (.l)
; All registers are restored
;***************************************************************************
Remove_IL:
	movem.l	d7/a1,-(sp)
	lea.l	IL_list,a1		; Search Item List
	moveq.l	#Max_ILs-1,d7
.Loop:	cmp.l	(a1)+,a0			; Is this the one ?
	bne.s	.Next
	clr.l	-4(a1)			; Yes -> Remove
	bra.s	.Exit
.Next:	dbra	d7,.Loop			; Next
	; NOT FOUND !
.Exit:	movem.l	(sp)+,d7/a1
	rts

;***************************************************************************
; [ Remove all Item Lists ]
; All registers are restored
;***************************************************************************
Remove_all_ILs:
	movem.l	d7/a1,-(sp)
	lea.l	IL_list,a1
	moveq.l	#Max_ILs-1,d7
.Loop:	clr.l	(a1)+			; Remove
	dbra	d7,.Loop			; Next
	movem.l	(sp)+,d7/a1
	rts

;***************************************************************************	
; [ Find an Item List ]
;   IN : d0 - Object ID (.l)
;  OUT : a0 - Pointer to Item List data / 0 (.l)
; Changed registers : a0
;***************************************************************************
Find_IL:
	movem.l	d7/a1,-(sp)
	lea.l	IL_list,a1		; Search Item List
	moveq.l	#Max_ILs-1,d7
.Loop:	tst.l	(a1)			; Anything there ?
	beq.s	.Next
	move.l	(a1),a0			; Yes -> Examine
	cmp.l	IL_object_ID(a0),d0		; Is this the one ?
	beq.s	.Exit
.Next:	addq.l	#4,a1			; Next
	dbra	d7,.Loop
	sub.l	a0,a0			; Not found
.Exit:	movem.l	(sp)+,d7/a1
	rts

;***************************************************************************
; [ Initialize a normal Item List ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Number of units per row (.w)
;        d3 - Number of rows on screen (.w)
;        a0 - Pointer to Item List data (.l)
; All registers are restored
; Notes :
;   - The Item List data must be complete. The object layer, the coordinate
;     list, the scroll bar and the scroll bar's object layer will all be
;     initialized by this routine (and are presumed to be filled with 0).
;***************************************************************************
Init_normal_IL:
	movem.l	d0-d3/d5-d7/a0-a4,-(sp)
	move.l	IL_scroll_bar(a0),a1
	move.l	IL_coordinates_list(a0),a2
	move.l	IL_object_layer(a0),a3
; ---------- Create second-level object -----------	
	movem.w	d0-d3,-(sp)
	move.w	d2,d6			; Calculate bottom-right
	move.w	d3,d7			;  coordinates
	mulu.w	#IS_outer_width,d6
	mulu.w	#IS_outer_height,d7
	add.w	d0,d6
	add.w	d1,d7
	subq.w	#IS_between_Y+1,d7
	add.w	#Between+Scroll_bar_width-IS_between_X-1,d6
	move.w	d0,Obj_X1(a3)		; Write coordinates
	move.w	d1,Obj_Y1(a3)
	move.w	d6,Obj_X2(a3)
	move.w	d7,Obj_Y2(a3)
	move.l	IL_object_ID(a0),d6		; Write ID
	lsr.w	#8,d6
	move.b	d6,Obj_ID(a3)
	lea.l	Obj_data_size(a3),a4	; Create child link
	move.l	a4,Obj_child(a3)
; ---------- Make coordinate list & object layer --
	move.w	d2,d6			; Prepare counters
	move.w	d3,d7
	subq.w	#1,d6
	subq.w	#1,d7
	move.w	d0,d2			; Bottom-right coordinates
	move.w	d1,d3
	add.w	#IS_inner_width-1,d2
	add.w	#IS_inner_height-1,d3
	moveq.l	#1,d5			; First third-level ID
.Loop_Y:	movem.w	d0/d2/d6,-(sp)
.Loop_X:	move.w	d0,(a2)+			; Write coordinates
	move.w	d1,(a2)+
	move.l	a4,a3
	move.w	d0,Obj_X1(a3)		; Write coordinates
	move.w	d1,Obj_Y1(a3)
	move.w	d2,Obj_X2(a3)
	move.w	d3,Obj_Y2(a3)
	move.b	d5,Obj_ID(a3)		; Write ID
	addq.w	#1,d5			; Next ID
	lea.l	Obj_data_size-4(a3),a4	; Get next object
	move.l	a4,Obj_brother(a3)		; Create brother link
	add.w	#IS_outer_width,d0		; Next column
	add.w	#IS_outer_width,d2
	dbra	d6,.Loop_X
	movem.w	(sp)+,d0/d2/d6		; Next row
	add.w	#IS_outer_height,d1
	add.w	#IS_outer_height,d3
	dbra	d7,.Loop_Y
	movem.w	(sp)+,d0-d3
; ---------- Make scroll bar & object layer -------
	move.w	IL_nr_items(a0),Total_units(a1)
	move.w	d2,Units_width(a1)
	move.w	d3,Units_height(a1)
	mulu.w	#IS_outer_height,d3
	subq.w	#IS_between_Y,d3
	move.w	d3,Scroll_bar_height(a1)
	mulu.w	#IS_outer_width,d2
	add.w	d2,d0
	addq.w	#Between-IS_between_X,d0
	move.w	d0,Scroll_bar_X(a1)
	move.w	d1,Scroll_bar_Y(a1)
	move.l	IL_draw(a0),Scroll_bar_draw_units(a1)
	move.b	#255,Obj_ID(a4)
	move.l	a4,Scroll_bar_object_layer(a1)
	move.l	IL_object_ID(a0),d0
	move.b	#255,d0
	move.l	d0,Scroll_bar_object_ID(a1)
	movem.l	(sp)+,d0-d3/d5-d7/a0-a4
	rts

;***************************************************************************
; [ Draw a normal Item List ]
;   IN : a0 - Pointer to Item List data (.l)
; All registers are restored
; Notes :
;   - This routine will use the second object layer data for the large box
;     and the scroll bar data for the scroll bar.
;   - The scroll bar and the items are drawn by calling [ Add_scroll_bar ].
;***************************************************************************
Draw_normal_IL:
	movem.l	d0-d3/a0/a1,-(sp)
	move.l	IL_object_layer(a0),a1	; Draw surrounding box
	move.w	Obj_X1(a1),d0
	move.w	Obj_Y1(a1),d1
	move.w	Obj_X2(a1),d2
	move.w	Obj_Y2(a1),d3
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#2+1,d2
	addq.w	#2+1,d3
	subq.w	#1,d0
	subq.w	#1,d1
	jsr	Draw_deep_box
	move.l	IL_scroll_bar(a0),a0	; Add scroll bar
	jsr	Add_scroll_bar
	movem.l	(sp)+,d0-d3/a0/a1
	rts

;***************************************************************************
; [ Display all item slots in an Item List ]
;   IN : a0 - Pointer to Item List data (.l)
; All registers are restored
;***************************************************************************
Display_all_items:
	movem.l	d0/d7/a1,-(sp)
	move.l	IL_scroll_bar(a0),a1	; Get first slot number
	move.w	Scroll_bar_result(a1),d0
	move.l	IL_draw_slot(a0),a1		; Get pointer to draw routine
	move.w	IL_nr_items(a0),d7		; Get number of items
	bra.s	.Entry
.Loop:	jsr	(a1)			; Draw item slot
	addq.w	#1,d0			; Next item slot
.Entry:	dbra	d7,.Loop
	movem.l	(sp)+,d0/d7/a1
	rts

;***************************************************************************
; [ Item touched handler ] Mouse Event
;   IN : d0 - Object ID (.l)
; No registers are restored
;***************************************************************************
Item_touched:
	and.l	#$00ffffff,d0		; Search
	jsr	Find_IL
	cmp.l	#0,a0			; Found ?
	beq	.Exit
	tst.b	d0			; Anything selected ?
	beq	.Exit
	cmp.b	#-1,d0			; Yes -> Scroll bar ?
	beq	.Exit
	move.l	IL_scroll_bar(a0),a1	; No -> Get real slot number
	and.w	#$00ff,d0
	subq.w	#1,d0
	add.w	Scroll_bar_result(a1),d0
	move.l	IL_touched(a0),a1		; Execute touched handler
	jsr	(a1)
.Exit:	rts

;***************************************************************************
; [ Item selected handler ] Mouse Event
;   IN : d0 - Object ID (.l)
; No registers are restored
;***************************************************************************
Item_selected:
	and.l	#$00ffffff,d0		; Search
	jsr	Find_IL
	cmp.l	#0,a0			; Found ?
	beq	.Exit
	tst.b	d0			; Anything selected ?
	beq	.Exit
	cmp.b	#-1,d0			; Yes -> Scroll bar ?
	beq	.Exit
	move.l	IL_scroll_bar(a0),a1	; No -> Get real slot number
	and.w	#$00ff,d0
	subq.w	#1,d0
	add.w	Scroll_bar_result(a1),d0
	move.l	IL_clicked_left(a0),a1	; Execute selected handler
	jsr	(a1)
.Exit:	rts

;***************************************************************************
; [ Item manipulated handler ] Mouse event
;   IN : d0 - Object ID (.l)
; No registers are restored
;***************************************************************************
Item_manipulated:
	and.l	#$00ffffff,d0		; Search
	jsr	Find_IL
	cmp.l	#0,a0			; Found ?
	beq	.Exit
	tst.b	d0			; Anything selected ?
	beq	.Exit
	cmp.b	#-1,d0			; Yes -> Scroll bar ?
	beq	.Exit
	move.l	IL_scroll_bar(a0),a1	; No -> Get real slot number
	and.w	#$00ff,d0
	subq.w	#1,d0
	add.w	Scroll_bar_result(a1),d0
	move.l	IL_clicked_right(a0),a1	; Execute manipulated handler
	jsr	(a1)
.Exit:	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_BSS,bss
IL_list:	ds.l Max_ILs
