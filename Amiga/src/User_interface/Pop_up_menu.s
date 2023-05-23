; Pop-up menus
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 19-1-1994

	XDEF	Pop_up_menu_class

	SECTION	Program,code
;***************************************************************************	
; [ Initialize method for pop-up menu objects ]
;   IN : a0 - Pointer to pop-up menu object (.l)
;        a1 - Pointer to pop-up menu OID (.l)
; All registers are restored
;***************************************************************************
Init_PUM:
	movem.l	d0-d7/a0-a3,-(sp)
	move.l	OID_PUM_data(a1),PUM_data(a0)	; Copy
	move.l	PUM_data(a0),a2		; Get PUM evaluator
	move.l	PUM_evaluator(a2),a2
	movem.l	a0/a1,-(sp)		; Execute
	jsr	(a2)
	movem.l	(sp)+,a0/a1
	move.l	a0,a2
; ---------- Scan all entries ---------------------
	movem.l	a0-a2,-(sp)
	move.l	PUM_data(a2),a1		; Get PUM data
	move.l	PUM_title(a1),a0		; Get title length
	jsr	Get_line_length
	move.w	d0,d4
	move.l	a1,a0
	move.w	PUM_nr_entries(a0),d7	; Get number of entries
	moveq.l	#0,d3			; Examine entries
	moveq.l	#0,d5
	moveq.l	#1,d6
	lea.l	PUM_header_size(a0),a0
	move.l	a0,a1
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
	movem.l	(sp)+,a0-a2
	LOCAL
	move.w	d5,PUM_current_entries(a2)	; Store real number of entries
	add.w	#PUM_edge*2,d4		; Calculate width & height
	move.w	d3,d5
	add.w	#PUM_title_height+3,d5
; ---------- Determine location -------------------
	move.l	d2,-(sp)
	move.w	OID_X(a1),d0		; Get top-left coordinates
	move.w	OID_Y(a1),d1
	add.w	X1(a2),d0
	add.w	Y1(a2),d1
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
.Ok_bott:	cmp.w	#Forbidden_Y,d5		; In forbidden zone ?
	bmi.s	.Ok
	cmp.w	#Forbidden_X,d4
	bmi.s	.Ok
	move.w	d4,d2			; Yes -> Can move left ?
	sub.w	#Forbidden_X,d2
	cmp.w	d2,d0
	bmi.s	.No
	sub.w	d2,d0			; Yes -> Move left
	sub.w	d2,d4
	bra.s	.Ok
.No:	move.w	d5,d2			; No -> Can move up ?
	sub.w	#Forbidden_Y,d2
	cmp.w	d2,d1
	bmi.s	.Ok
	sub.w	d2,d1			; Yes -> Move up
	sub.w	d2,d5
.Ok:	move.w	d0,X1(a2)			; Store rectangle
	move.w	d1,Y1(a2)
	move.w	d4,X2(a2)
	move.w	d5,Y2(a2)
	move.l	(sp)+,d2
	move.l	PUM_data(a2),a3
; ---------- Create window ------------------------
	movem.l	a0-a3,-(sp)
	sub.w	d0,d4			; Calculate width & height
	addq.w	#1,d4
	sub.w	d1,d5
	addq.w	#1,d5
	Make_OID	Window,a1			; Add window
	clr.w	OID_X(a1)
	clr.w	OID_Y(a1)
	move.w	d4,OID_width(a1)
	move.w	d5,OID_height(a1)
	lea.l	Window_class,a0
	move.w	Object_self(a2),d0
	jsr	Add_object
	Free_OID
	move.w	d0,d7
	Make_OID	HBox,a1			; Add border
	clr.w	OID_X(a1)
	clr.w	OID_Y(a1)
	move.w	d4,OID_width(a1)
	move.w	d5,OID_height(a1)
	lea.l	HBox_class,a0
	move.w	d7,d0
	jsr	Add_object
	Free_OID
	Make_OID	Text,a1			; Add title
	clr.w	OID_X(a1)
	move.w	#(PUM_title_height-Standard_text_height)/2,OID_Y(a1)
	move.w	d4,OID_Text_object_width(a1)
	move.l	PUM_title(a3),OID_Text_ptr(a1)
	move.w	#Gold,OID_Text_colour(a1)
	lea.l	Text_class,a0
	move.w	d7,d0
	jsr	Add_object
	Free_OID
	movem.l	(sp)+,a0-a3
	LOCAL
; ---------- Add pop-up menu entries --------------
	Make_OID	PUME,a1
	move.w	#4,OID_X(a1)		; Prepare OID
	move.w	#PUM_title_height,OID_Y(a1)
	subq.w	#8,d4
	move.w	d4,OID_width(a1)
	move.w	#PUME_height,OID_height(a1)
	move.l	#PUME_selected,OID_Button_function(a1)
	move.w	Object_self(a2),OID_PUM_handle(a1)
	move.w	PUM_current_entries(a2),d6	; Do all entries
	lea.l	PUME_list,a2
	lea.l	PUM_header_size(a3),a3
	bra.s	.Next
.Loop:	move.w	(a2)+,d0			; Get PUME number
	cmp.w	#-1,d0			; Selectable ?
	bne.s	.Yes
	addq.w	#3,OID_Y(a1)		; No -> Advance
	bra.s	.Next
.Yes:	move.w	d0,OID_PUME_nr(a1)		; Yes -> Store number
	move.l	a3,a0			; Get PUME data
	subq.w	#1,d0
	mulu.w	#PUME_data_size,d0
	add.w	d0,a0
	move.l	PUME_text(a0),OID_Button_data(a1)	; Copy text pointer
	btst	#PUME_blocked,PUME_flags(a0)	; Blocked ?
	bne.s	.Blocked
	move.w	#Gold+1,OID_Button_text_colour(a1)	; No
	move.w	#Gold+3,OID_Button_text_feedback_colour(a1)
	bra.s	.Go_on
.Blocked:	move.w	#Red+1,OID_Button_text_colour(a1)	; Yes
	move.w	#Red+3,OID_Button_text_feedback_colour(a1)
.Go_on:	lea.l	PUME_class,a0		; Add PUME
	move.w	d7,d0
	jsr	Add_object
	add.w	#PUME_height,OID_Y(a1)	; Advance
.Next:	dbra	d6,.Loop
	Free_OID
	movem.l	(sp)+,d0-d7/a0-a3
	rts

;***************************************************************************	
; [ Remove a pop-up menu ]
;   IN : a0 - Pointer to pop-up menu object (.l)
; All registers are restored
;***************************************************************************
Remove_pop_up_menu:
	jsr	Dehighlight
	jsr	Wait_4_rightunclick
	Pop	Root
	jmp	Update_screen

;***************************************************************************	
; [ Initialize method for pop-up menu entry objects ]
;   IN : a0 - Pointer to pop-up menu entry object (.l)
;        a1 - Pointer to pop-up menu entry OID (.l)
; All registers are restored
;***************************************************************************
Init_PUME:
	move.w	OID_PUME_nr(a1),PUME_nr(a0)	; Copy
	move.w	OID_PUM_handle(a1),PUM_handle(a0)
	rts

;***************************************************************************
; [ Pop-up menu entry selected mouse event ]
;   IN : a0 - Pointer to button object (.l)
; No registers are restored
;***************************************************************************
PUME_selected:
	move.l	a0,a1			; Get PUM data
	move.w	PUM_handle(a1),d0
	jsr	Get_object_data
	exg.l	a0,a1
	move.l	PUM_data(a1),a1		; Get PUME data
	move.w	PUME_nr(a0),d0
	subq.w	#1,d0
	mulu.w	#PUME_data_size,d0
	lea.l	PUM_header_size(a1,d0.w),a1
	btst	#PUME_not_selectable,PUME_flags(a1)	; Selectable ?
	bne.s	.Exit
	jsr	Normal_clicked		; Yes
	beq.s	.Exit
	btst	#PUME_blocked,PUME_flags(a1)	; Blocked ?
	bne.s	.Blocked
	move.l	PUME_function(a1),a2	; No -> Execute function
	jsr	(a2)
	btst	#PUME_auto_close,PUME_flags(a1)	; Close PUM ?
	beq.s	.Exit
	move.w	PUM_handle(a0),d0		; Yes
	jsr	Get_object_data
	jsr	Remove_pop_up_menu
	bra.s	.Exit
.Blocked:
	nop

.Exit:	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Pop_up_menu_class:
	dc.l 0
	dc.w PUM_object_size
	Method Init,Init_PUM
	Method Draw,Execute_child_methods
	Method Mev,.Mev
	dc.w -1

.Mev:	dc.w $2020
	dc.l Remove_pop_up_menu
	dc.w $3300
	dc.l Dehighlight
	dc.w -1

PUME_class:
	dc.l BT_Button_class
	dc.w PUME_object_size
	Method Init,Init_PUME
	Method Mev,.Mev
	dc.w -1

.Mev:	dc.w $0202
	dc.l PUME_selected
	dc.w $3300
	dc.l Normal_highlighted
	dc.w -1


	SECTION	Fast_BSS,bss
PUME_list:	ds.w Max_PUMES
