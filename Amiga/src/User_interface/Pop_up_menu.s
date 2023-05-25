; Pop-up menus
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 19-1-1994

	XDEF	Do_PUM

	SECTION	Program,code
;***************************************************************************	
; [ Show a pop-up menu ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        a0 - Pointer to pop-up menu data (.l)
; All registers are restored
;***************************************************************************
Do_PUM:
	movem.l	d0-d7/a0-a3/a5/a6,-(sp)
	move.l	a0,a6
	Make_OID PUM,a5			; Make OID
	move.w	d0,OID_X(a5)		; Store coordinates
	move.w	d1,OID_Y(a5)
	move.l	a0,OID_PUM_data(a5)
	move.l	PUM_evaluator(a6),a1	; Evaluate pop-up menu entries
	jsr	(a1)
; ---------- Scan all entries ---------------------
	move.l	PUM_title(a6),a0		; Get title length
	jsr	Get_line_length
	move.w	d0,d4
	move.w	PUM_nr_entries(a6),d7	; Get number of entries
	moveq.l	#0,d3			; Examine entries
	moveq.l	#0,d5
	moveq.l	#1,d6
	lea.l	PUM_header_size(a6),a1
	lea.l	PUME_list,a2
	bra	.Entry
.Loop:	btst	#PUME_absent,PUME_flags(a1)	; Absent ?
	bne.s	.Next
	addq.w	#1,d5			; No -> Count up
	btst	#PUME_not_selectable,PUME_flags(a1)	; Selectable ?
	beq.s	.Yes
	move.w	#-1,(a2)+			; No
	addq.w	#4,d3
	bra.s	.Next
.Yes:	move.w	d6,(a2)+			; Yes
	add.w	#PUME_height,d3
	move.l	PUME_text(a1),a0		; Get line length
	jsr	Get_line_length
	cmp.w	d0,d4			; New maximum ?
	bpl.s	.Next
	move.w	d0,d4			; Yes
.Next:	addq.w	#1,d6			; Next entry
	lea.l	PUME_data_size(a1),a1
.Entry:	dbra	d7,.Loop
	LOCAL
	move.w	d5,OID_PUM_current_entries(a5)	; Store real number of entries
	add.w	#PUM_edge*2,d4		; Calculate width & height
	move.w	d3,d5
	add.w	#PUM_title_height+3,d5
; ---------- Determine location -------------------
	move.w	OID_X(a5),d0		; Get top-left coordinates
	move.w	OID_Y(a5),d1
	tst.w	d0			; Off screen ?
	bpl.s	.Ok_left
	moveq.l	#0,d0
.Ok_left:	tst.w	d1
	bpl.s	.Ok_top
	moveq.l	#0,d1
.Ok_top:	add.w	d0,d4			; Get bottom-right coordinates
	add.w	d1,d5
	subq.w	#1,d4
	subq.w	#1,d5
	cmp.w	#Screen_width-1,d4		; Off screen ?
	bmi.s	.Ok_right
	sub.w	#Screen_width-2,d4
	sub.w	d4,d0
	move.w	#Screen_width-2,d4
.Ok_right:	cmp.w	#Screen_height-1,d5
	bmi.s	.Ok_bott
	sub.w	#Screen_height-2,d5
	sub.w	d5,d1
	move.w	#Screen_height-2,d5
.Ok_bott:	cmp.w	#Forbidden_Y-1,d5		; Y in forbidden zone ?
	bmi.s	.Ok
	cmp.w	#Forbidden_X-1,d4		; Yes -> X as well ?
	bmi.s	.Ok
	move.w	d5,d2			; Yes -> Can move up ?
	sub.w	#Forbidden_Y-1,d2
	cmp.w	d2,d1
	bmi.s	.Ok
	sub.w	d2,d1			; Yes -> Move up
	sub.w	d2,d5
.Ok:	move.w	d0,OID_X(a5)		; Store coordinates
	move.w	d1,OID_Y(a5)
	sub.w	d0,d4
	sub.w	d1,d5
	addq.w	#1,d4
	addq.w	#1,d5
	move.w	d4,OID_width(a5)
	move.w	d5,OID_height(a5)
; ---------- Do PUM -------------------------------
	lea.l	PUM_class,a0		; Add object
	move.l	a5,a1
	moveq.l	#-1,d0
	jsr	Add_object
	Free_OID
	moveq.l	#Draw_method,d1		; Draw
	jsr	Execute_method
	jsr	Wait_4_object		; Wait
	movem.l	(sp)+,d0-d7/a0-a3/a5/a6
	rts

;***************************************************************************	
; [ Initialize method for pop-up menu objects ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to OID (.l)
; All registers are restored
;***************************************************************************
Init_PUM:
	movem.l	d0/d7/a0-a3,-(sp)
	move.l	OID_PUM_data(a1),PUM_data(a0)	; Copy
	move.w	OID_PUM_current_entries(a1),PUM_current_entries(a0)
; ---------- Add pop-up menu entries --------------
	move.l	a0,a2
	move.w	OID_width(a1),d0
	Make_OID	PUME,a1
	move.w	#4,OID_X(a1)		; Prepare OID
	move.w	#PUM_title_height,OID_Y(a1)
	subq.w	#8,d0
	move.w	d0,OID_width(a1)
	move.w	#PUME_height,OID_height(a1)
	lea.l	PUME_class,a0		; Do all entries
	lea.l	PUME_list,a3
	move.w	PUM_current_entries(a2),d7
	bra.s	.Next
.Loop:	move.w	(a3)+,d0			; Get PUME number
	cmp.w	#-1,d0			; Selectable ?
	bne.s	.Yes
	addq.w	#3,OID_Y(a1)		; No -> Advance
	bra.s	.Next
.Yes:	move.w	d0,OID_PUME_nr(a1)		; Yes -> Store number
	move.w	Object_self(a2),d0		; Add PUME
	jsr	Add_object
	add.w	#PUME_height,OID_Y(a1)	; Advance
.Next:	dbra	d7,.Loop
	Free_OID
	movem.l	(sp)+,d0/d7/a0-a3
	rts

;***************************************************************************	
; [ Draw method for pop-up menu objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Draw_PUM:
	movem.l	d0-d3/a0/a1,-(sp)
	move.l	a0,a1
	move.w	X1(a1),d0			; Draw box
	move.w	Y1(a1),d1
	move.w	X2(a1),d2
	move.w	Y2(a1),d3
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#1,d2
	addq.w	#1,d3
	jsr	Draw_high_box
	move.w	#Gold,Ink_colour		; Draw title
	add.w	#(PUM_title_height-Standard_text_height)/2,d1
	move.l	PUM_data(a1),a0
	move.l	PUM_title(a0),a0
	jsr	Put_centered_text_line
	move.w	Object_self(a1),d0		; Draw children
	moveq.l	#Draw_method,d1
	jsr	Execute_child_methods
	movem.l	(sp)+,d0-d3/a0/a1
	jmp	Update_screen

;***************************************************************************	
; [ Close method for pop-up menu object ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Close_PUM:
	jsr	Dehighlight
	jsr	Wait_4_rightunclick
	Pop	Root
	jsr	Delete_self
	jmp	Update_screen

;***************************************************************************	
; [ Initialize method for pop-up menu entry object ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to OID (.l)
; All registers are restored
;***************************************************************************
Init_PUME:
	move.w	OID_PUME_nr(a1),PUME_nr(a0)	; Copy
	jmp	Set_rectangle

;***************************************************************************	
; [ Draw method for pop-up menu entry object ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Draw_PUME:
	movem.l	d0-d3/a0/a1,-(sp)
	move.w	X1(a0),d0			; Draw border
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#1,d2
	addq.w	#1,d3
	jsr	Draw_high_box
	jsr	Get_PUME_data
	btst	#PUME_blocked,PUME_flags(a1)	; Blocked ?
	bne.s	.Blocked
	move.w	#Gold+1,Ink_colour		; No
	bra.s	.Go_on
.Blocked:	move.w	#Red+1,Ink_colour		; Yes
.Go_on:	move.l	PUME_text(a1),a0		; Print
	jsr	Put_centered_box_text_line
	movem.l	(sp)+,d0-d3/a0/a1
	rts

;***************************************************************************	
; [ Feedback method for pop-up menu entry object ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Feedback_PUME:
	movem.l	d0-d3/a0/a1,-(sp)
	move.w	X1(a0),d0			; Draw border
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#1,d2
	addq.w	#1,d3
	jsr	Draw_deep_box
	jsr	Get_PUME_data
	btst	#PUME_blocked,PUME_flags(a1)	; Blocked ?
	bne.s	.Blocked
	move.w	#Gold+3,Ink_colour		; No
	bra.s	.Go_on
.Blocked:	move.w	#Red+3,Ink_colour		; Yes
.Go_on:	addq.w	#1,d1
	move.l	PUME_text(a1),a0		; Print
	jsr	Put_centered_box_text_line
	movem.l	(sp)+,d0-d3/a0/a1
	rts

;***************************************************************************	
; [ Highlight method for pop-up menu entry object ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Highlight_PUME:
	movem.l	d0-d3/a0/a1,-(sp)
	move.w	X1(a0),d0			; Draw border
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#1,d2
	addq.w	#1,d3
	jsr	Draw_high_box
	jsr	Get_PUME_data
	btst	#PUME_blocked,PUME_flags(a1)	; Blocked ?
	bne.s	.Blocked
	move.w	#White,Ink_colour		; No
	bra.s	.Go_on
.Blocked:	move.w	#Red,Ink_colour		; Yes
.Go_on:	move.l	PUME_text(a1),a0		; Print
	jsr	Put_centered_box_text_line
	movem.l	(sp)+,d0-d3/a0/a1
	rts

;***************************************************************************
; [ Left method for pop-up menu entry ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
PUME_selected:
	movem.l	d0/d1/a1/a2,-(sp)
	jsr	Normal_clicked
	beq.s	.Exit
	jsr	Get_PUME_data
	btst	#PUME_blocked,PUME_flags(a1)	; Blocked ?
	bne.s	.Blocked
	move.l	PUME_function(a1),a2	; No -> Execute function
	jsr	(a2)
	btst	#PUME_auto_close,PUME_flags(a1)	; Close PUM ?
	beq.s	.Exit
	move.w	Object_parent(a0),d0		; Yes
	moveq.l	#Close_method,d1
	jsr	Execute_method
	bra.s	.Exit
.Blocked:
	nop

.Exit:	movem.l	(sp)+,d0/d1/a1/a2
	rts

;***************************************************************************
; [ Get pop-up menu entry data ]
;   IN : a0 - Pointer to object (.l)
;  OUT : a1 - Pointer to PUME data (.l)
; Changed registers : a1
;***************************************************************************
Get_PUME_data:
	move.l	d0,-(sp)
	move.l	a0,a1			; Get PUM data
	move.w	Object_parent(a1),d0
	jsr	Get_object_data
	exg.l	a0,a1
	move.l	PUM_data(a1),a1		; Get PUME data
	move.w	PUME_nr(a0),d0
	subq.w	#1,d0
	mulu.w	#PUME_data_size,d0
	lea.l	PUM_header_size(a1,d0.w),a1	
	move.l	(sp)+,d0
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
PUM_class:
	dc.l Window_class
	dc.w PUM_object_size
	Method Init,Init_PUM
	Method Draw,Draw_PUM
	Method Right,Close_PUM
	Method Touched,Normal_touched
	Method Close,Close_PUM
	dc.w -1

PUME_class:
	dc.l 0
	dc.w PUME_object_size
	Method Init,Init_PUME
	Method Draw,Draw_PUME
	Method Feedback,Feedback_PUME
	Method Highlight,Highlight_PUME
	Method Left,PUME_selected
	Method Touched,Normal_touched
	dc.w -1


	SECTION	Fast_BSS,bss
PUME_list:	ds.w Max_PUMES
