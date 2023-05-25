; Pop-Up Menu handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 19-1-1994

	XDEF	Do_PUM
	XDEF	Close_PUM
	XDEF	Reset_PUM_stack

	XREF	Push_Module
	XREF	Print_centered_string
	XREF	Put_text_line
	XREF	Draw_high_box
	XREF	Open_naked_window
	XREF	Update_screen
	XREF	Find_object
	XREF	Get_line_length
	XREF	Print_feedback_prompt

Max_PUMs	EQU 3
Max_PUMES	EQU 20
PUM_title_height	EQU 12
PUME_height	EQU 10
PUM_edge	EQU 4
PUM_ID	EQU 255
Up_PUME_colour	EQU
Down_PUME_colour	EQU
Up_blocked_PUME_colour	EQU
Down_blocked_PUME_colour	EQU

	rsreset
PUM_X:	rs.w 1
PUM_Y:	rs.w 1
PUM_nr_entries:	rs.w 1
PUM_title:	rs.l 1
PUM_header_size:	rs.b 0

	rsreset
PUME_flags:	rs.b 1
	rseven
PUME_blocked_prompt:	rs.w 1
PUME_text:	rs.l 1
PUME_function:	rs.l 1
PUME_datasize:	rs.b 0

	rsreset
PUME_absent:	rs.b 1
PUME_not_selectable:	rs.b 1
PUME_blocked:	rs.b 1
PUME_auto_close:	rs.b 1

	SECTION	Program,code
;************************************************************************************
; [ Show & handle a pop-up menu ]
;   IN : a0 - Pointer to PUM structure (.l)
; All registers are restored
;************************************************************************************
Do_PUM:
	move.l	a0,-(sp)
	jsr	Push_PUM
	Push	Module,PUM_Mod
	move.l	(sp)+,a0
	rts

;************************************************************************************
; [ Close a Pop-Up Menu ]
; All registers are restored
;************************************************************************************
Close_PUM:
	Pop	Module
	jmp	Pop_PUM

;************************************************************************************
; [ Push a Pop-Up Menu on the PUM stack ]
;   IN : a0 - Pointer to PUM structure (.l)
; All registers are restored
;************************************************************************************
Push_PUM:
	move.l	a1,-(sp)
	movea.l	PUM_Sp,a1
	addq.l	#4,a1
	cmpa.l	#PUMStack_end,a1		; Possible ?
	beq.s	.Exit
	move.l	a0,(a1)			; Push
	move.l	a1,PUM_Sp
	jsr	Init_PUM			; Initialize new PUM
	jsr	Draw_PUM
.Exit:	movea.l	(sp)+,a1
	rts

;************************************************************************************
; [ Pop a Pop-Up Menu from the PUM stack ]
; All registers are restored
;************************************************************************************
Pop_PUM:
	move.l	a0,-(sp)
	movea.l	PUM_Sp,a0
	cmpa.l	#PUMStack_start,a0		; Possible ?
	beq.s	.Exit
	jsr	Close_window		; Pop
	subq.l	#4,a0
	move.l	a0,PUM_Sp
	cmpa.l	#PUMStack_start,a0		; At the base ?
	beq.s	.Exit
	move.l	(a0),a0			; No -> Initialize old PUM
	jsr	Init_PUM
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Reset the PUM stack ]
; All registers are	restored
;*****************************************************************************
Reset_PUM_stack:
	move.l	#PUMStack_start,PUM_Sp
	rts

;************************************************************************************
; [ Initialize a Pop-Up Menu ]
;   IN : a0 - Pointer to PUM structure (.l)
; All registers are restored
;************************************************************************************
Init_PUM:
	movem.l	d0-d7/a0-a2,-(sp)
; ---------- Scan all entries ---------------------
	move.w	PUM_nr_entries(a0),d7	; Get number of entries
	moveq.l	#-1,d4			; Examine entries
	moveq.l	#0,d5
	moveq.l	#1,d6
	addq.l	#PUM_header_size,a0
	move.l	a0,a1
	lea.l	PUME_list,a2
	bra	.Entry
.Loop:	btst	#PUME_absent,PUME_flags(a1)	; Absent ?
	bne.s	.Next
	addq.w	#1,d5			; No -> Count up
	move.w	d6,(a2)+			; Insert in PUME list
	move.l	PUME_text(a1),a0		; Get line length
	jsr	Get_line_length
	cmp.w	d0,d4			; New maximum ?
	bpl.s	.Next
	move.w	d0,d4			; Yes
.Next:	addq.w	#1,d6			; Next entry
	lea.l	PUME_datasize(a1),a1
.Entry:	dbra	d7,.Loop
	add.w	#PUM_edge*2,d4		; Store width of menu
	move.w	d4,PUM_width
	move.w	d5,Current_PUM_entries	; Store real number of entries
	LOCAL
; ---------- Build first object layer -------------
	lea.l	PUM_L1,a1
	subq.w	#1,d4			; Calculate & store X2
	move.w	d4,Obj_X2(a1)
	mulu.w	#PUME_height,d5		; Calculate height of menu
	add.w	#PUM_title_height,d5
	move.w	d5,PUM_height
	subq.w	#1,d5			; Calculate & store Y2
	move.w	d5,Obj_Y2(a1)
; ---------- Build second object layer ------------
	lea.l	PUM_L2,a0
	moveq.l	#PUM_title_height,d0
	move.w	d0,d1
	moveq.l	#PUME_height,d2
	add.w	d2,d1
	subq.w	#1,d1
	moveq.l	#1,d6
	move.w	Current_PUM_entries,d7
	bra.s	.Entry
.Loop:	lea.l	Obj_datasize(a0),a1		; Get pointer to next object
	move.w	d4,Obj_X2(a0)		; Insert coordinates
	move.w	d0,Obj_Y1(a0)
	move.w	d1,Obj_Y2(a0)
	move.b	d6,Obj_ID(a0)		; Insert ID
	tst.w	d7			; Last entry ?
	beq.s	.Last
	move.l	a1,Obj_brother(a0)		; No -> Link to next
	bra.s	.Go_on
.Last:	clr.l	Obj_brother(a0)		; Yes -> End chain
.Go_on:	move.l	a1,a0			; Next entry
	add.w	d2,d0
	add.w	d2,d1
	addq.w	#1,d6
.Entry:	dbra	d7,.Loop
	movem.l	(sp)+,d0-d7/a0-a2
	rts

;************************************************************************************
; [ Draw a Pop-Up Menu ]
;   IN : a0 - Pointer to PUM structure (.l)
; All registers are restored
; Notes :
;   - The Pop-Up Menu must have been initialized by [ Init_PUM ].
;************************************************************************************
Draw_PUM:
	movem.l	d0-d3/d7/a0/a1,-(sp)
	move.l	a0,a1			; Save
; ---------- Draw Pop-Up Menu ---------------------
	move.w	PUM_X(a1),d0		; Get data
	move.w	PUM_Y(a1),d1
	move.w	PUM_width,d2
	move.w	PUM_height,d3
	lea.l	PUM_L1,a0			; Open window
	jsr	Open_naked_window
	moveq.l	#PUM_title_height,d3	; Draw title box
	jsr	Draw_high_box
	tst.l	PUM_title(a1)		; Any title ?
	bne.s	.Title
	add.w	d3,d1			; No
	bra.s	.Go_on
.Title:	addq.w	#2,d1			; Yes -> Print it
	move.l	PUM_title(a1),a0
	jsr	Print_centered_string
	add.w	#PUM_title_height-2,d1
.Go_on:	sub.w	PUM_height,d3		; Draw main box
	neg.w	d3
	jsr	Draw_high_box
; ---------- Display entries ----------------------
	move.w	PUM_nr_entries(a1),d7	; Get number of entries
	lea.l	PUM_header_size(a1),a0
	moveq.l	#1,d0
	bra	.Entry
.Loop:	btst	#PUME_absent,PUME_flags(a0)	; Absent ?
	bne.s	.Next
	jsr	Display_PUME		; No -> Display
	addq.w	#1,d0
.Next:	lea.l	PUME_datasize(a1),a1	; Next entry
.Entry:	dbra	d7,.Loop
	movem.l	(sp)+,d0-d3/d7/a0/a1
	rts

;************************************************************************************
; [ Pop-Up Menu handler ] Mouse Event
;   IN : d0 - PUME object ID (.l)
; No registers are restored
;************************************************************************************
PUME_clicked:
; ---------- Determine entry ----------------------
	move.l	d0,Current_PUME_ID		; Store object ID
	lsr.w	#8,d0			; Calculate & store PUME index
	move.w	d0,Current_PUME_index
	lea.l	PUME_list,a0		; Get the real number
	move.w	0(a0,d0.w*2),d0
	move.l	PUM_Sp,a1			; Get current PUM data
	move.l	(a1),a1
	addq.l	#PUM_header_size,a1		; Get PUME address
	mulu.w	#PUME_datasize,d0
	add.w	d0,a1
	btst	#PUME_not_selectable,PUME_flags(a1)	; Selectable ?
	bne	.Exit
	move.l	a1,Current_PUME_ptr		; Store
; ---------- Feedback until released --------------
	sf	PUME_down			; Entry is up
	Push	Module,PUME_Mod		; Handle it
	tst.b	PUME_down			; Entry down ?
	beq	.Exit
; ---------- Restore entry ------------------------
	move.l	a1,a0
	move.w	Current_PUME_index,d0
	jsr	Display_PUME
	jsr	Update_screen
; ---------- Execute function ---------------------
	btst	#PUME_blocked,PUME_flags(a1)	; Blocked ?
	bne	.Blocked
	move.l	PUME_function(a1),a0	; No -> Execute function
	jsr	(a0)
	btst	#PUME_auto_close,PUME_flags(a1)	; Close PUM ?
	beq	.Exit
	jsr	Close_PUM			; Yes
	bra	.Exit
.Blocked:
	move.w	PUME_blocked_prompt(a1),d0	; Print blocked prompt
	jsr	Print_feedback_prompt
.Exit:	rts

PUM_DisUpd:
	move.w	Mouse_X,d0		; Get object under mouse-pointer
	move.w	Mouse_Y,d1
	jsr	Find_object
	cmp.l	Current_PUME_ID,d2		; Over entry ?
	seq	d0
	cmp.b	PUME_down,d0		; State changed ?
	beq	.Exit
	move.b	d0,PUME_down		; Yes
	move.l	Current_PUME_ptr,a0		; Get current entry data
	tst.b	d0			; Up or down ?
	bne.s	.Down
	move.w	Current_PUME_index,d0	; Up
	jsr	Display_PUME
	bra.s	.Go_on
	move.w	Current_PUME_index,d0	; Down
	jsr	Display_PUME_down
.Go_on:	jsr	Update_screen
.Exit:	rts

;************************************************************************************
; [ Display a Pop-Up Menu Entry (up) ]
;   IN : d0 - PUME index {1...} (.w)
;        a0 - Pointer to PUME data (.l)
; All registers are restored
;************************************************************************************
Display_PUME:
	movem.l	d0/d1/a0/a1,-(sp)
	move.l	PUM_Sp,a1			; Get current PUM data
	move.l	(a1),a1
	move.w	d0,d1			; Get PUME coordinates
	subq.w	#1,d1
	mulu.w	#PUME_height,d1
	add.w	#PUM_title_height+2,d1
	add.w	PUM_Y(a1),d1
	move.w	PUM_X(a1),d0
	addq.w	#PUM_edge,d0
	btst	#PUME_blocked,PUME_flags(a0)	; Blocked ?
	bne.s	.No
	move.w	#Up_blocked_PUME_colour,Ink_colour	; Yes
	bra.s	.Go_on
.No:	move.w	#Up_PUME_colour,Ink_colour	; No
.Go_on:	move.l	PUME_text(a0),a0		; Get PUME text
	jsr	Put_text_line		; Print PUME
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;************************************************************************************
; [ The DATA & BSS segments ]
;************************************************************************************
	SECTION	Fast_DATA,data
PUM_L1:	dc.w 0,0
	dc.w 0,0,PUM_title_height,0
	dc.b PUM_ID
	even
	dc.l 0,PUM_L2

	SECTION	Fast_BSS,bss
PUM_L2:	rept Max_PUMEs
	ds.w Obj_datasize/2
	endr

Current_PUME_index:	ds.w 1		; Data for currently selected PUME
Current_PUME_ID:	ds.l 1
Current_PUME_ptr:	ds.l 1
PUME_down:	ds.b 1
	even
PUM_width:	ds.w 1			; Data for current PUM
PUM_height:	ds.w 1
Current_PUM_entries:	ds.w 1

PUME_list:	ds.w Max_PUMEs		; Index list

PUM_Sp:	ds.l 1			; PUM stack
PUMStack_start:
	ds.l Max_PUMs
PUMStack_end:
