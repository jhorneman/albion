; Input handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	dc.l WA_IDCMP,IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_DELTAMOVE|IDCMP_VANILLAKEY|IDCMP_RAWKEY|IDCMP_DISKINSERTED

	MODULE	DDT_Input

	OUTPUT	DDT:Objects/Core/Input.o

	OPT	O1+,O2+,O4+,O5+,O10+,O11+,OW-
	OPT	O7+,EVEN,USER,INCONCE
	OPT	P=68020


	XREF	Intuition_base
	XREF	Create_port
	XREF	Destroy_port
	XREF	Create_IO_request
	XREF	Destroy_IO_request
	XREF	Extended_error_code
	XREF	Fatal_error
	XREF	My_window
	XREF	MA_Sp

	XDEF	Init_input
	XDEF	Exit_input
	XDEF	Input_handler
	XDEF	Read_key
	XDEF	Read_Mev
	XDEF	Button_state
	XDEF	Mouse_X
	XDEF	Mouse_Y


	incdir	DDT:
	include	Constants/Global.i
	include	Constants/OS/OS.i
	include	Constants/Core.i


	SECTION	Program,code
;***************************************************************************
; [ Initialize input handling ]
; All registers are restored
;***************************************************************************
Init_input:
	movem.l	d0/d1/a0-a3,-(sp)
	jsr	Create_port		; Create port
	move.l	d0,Input_port
	jsr	Create_IO_request		; Create I/O request
	move.l	d0,Input_IO_req
	lea.l	Input_device_name,a0	; Open Input device
	move.l	d0,a1
	moveq.l	#0,d0
	moveq.l	#0,d1
	kickEXEC	OpenDevice
	tst.b	d0			; Success ?
	beq.s	.Ok
	and.l	#$000000ff,d0		; No -> Exit
	move.l	d0,Extended_error_code
	move.l	#OPEN_DEVICE_ERROR,d0
	jmp	Fatal_error
.Ok:	st	Input_device_opened
	LOCAL
	move.l	Input_IO_req,a0		; Store library base
	move.l	io_Device(a0),Input_base
	movem.l	(sp)+,d0/d1/a0-a3
	rts

;***************************************************************************
; [ Exit input handling ]
; All registers are restored
;***************************************************************************
Exit_input:
	movem.l	d0/d1/a0/a1,-(sp)
	tst.b	Input_device_opened		; Input device opened ?
	beq.s	.No
	move.l	Input_IO_req,a1		; Yes -> Close device
	kickEXEC	CloseDevice
	sf	Input_device_opened
.No:	LOCAL
	move.l	Input_IO_req,a0		; Destroy I/O request
	jsr	Destroy_IO_request
	clr.l	Input_IO_req
	move.l	Input_port,a0		; Destroy port
	jsr	Destroy_port
	clr.l	Input_port
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************
; [ Input handler ]
; All registers are restored
;***************************************************************************
Input_handler:
	movem.l	d0-d3/d6/d7/a0-a2,-(sp)
.Again:	move.l	My_window,a0		; Get message
	move.l	wd_UserPort(a0),a0
	kickEXEC	GetMsg
	tst.l	d0			; Anything ?
	beq	.Exit
	move.l	d0,a0			; Yes
	move.l	im_Class(a0),d0		; Get class
; ---------- Handle mouse movement ----------------
	cmp.l	#IDCMP_MOUSEMOVE,d0		; Mouse movement ?
	bne	.No_1
	tst.l	MA_Sp			; Stack initialized ?
	beq	.Next
	move.l	MA_Sp,a1			; Get area address
	move.l	(a1),a1	
	move.w	im_MouseX(a0),d0		; Get relative coordinates
	move.w	im_MouseY(a0),d1
	add.w	Mouse_X,d0		; Calculate absolute coordinates
	add.w	Mouse_Y,d1
	cmp.w	MA_X1(a1),d0		; X too low ?
	bpl.s	.Left_OK
	move.w	MA_X1(a1),d0
	bra.s	.Check_Y
.Left_OK:	cmp.w	MA_X2(a1),d0		; X too high ?
	blt.s	.Check_Y
	move.w	MA_X2(a1),d0
.Check_Y:	cmp.w	MA_Y1(a1),d1		; Y too low ?
	bpl.s	.Top_OK
	move.w	MA_Y1(a1),d1
	bra.s	.Done
.Top_OK:	cmp.w	MA_Y2(a1),d1		; Y too high ?
	blt.s	.Done
	move.w	MA_Y2(a1),d1
.Done:	move.w	d0,Mouse_X		; Store coordinates
	move.w	d1,Mouse_Y
	bra	.Next
; ---------- Handle mouse buttons -----------------
.No_1:	cmp.l	#IDCMP_MOUSEBUTTONS,d0	; Mouse buttons ?
	bne	.No_2
	moveq.l	#0,d7			; Yes -> Build button state
	move.w	im_Code(a0),d6
	cmp.w	#SELECTDOWN,d6		; Left button down ?
	bne.s	.No_ldown
	bset	#Left_pressed,d7		; Yes
	move.l	Left_time,d0		; Get previous time
	move.l	Left_time+4,d1
	move.l	im_Seconds(a0),d2		; Get current time
	move.l	im_Micros(a0),d3
	move.l	d2,Left_time		; Is new previous
	move.l	d3,Left_time+4
	movem.l	d1/a0/a1,-(sp)		; Double click ?
	kickINTU	DoubleClick
	movem.l	(sp)+,d1/a0/a1
	tst.l	d0
	beq	.Go_on
	bset	#Left_double,d7		; Yes
	bra	.Go_on
.No_ldown:	cmp.w	#MENUDOWN,d6		; Right button down ?
	bne.s	.Go_on
	bset	#Right_pressed,d7		; Yes
	move.l	Right_time,d0		; Get previous time
	move.l	Right_time+4,d1
	move.l	im_Seconds(a0),d2		; Get current time
	move.l	im_Micros(a0),d3
	move.l	d2,Right_time		; Is new previous
	move.l	d3,Right_time+4
	movem.l	d1/a0/a1,-(sp)		; Double click ?
	kickINTU	DoubleClick
	movem.l	(sp)+,d1/a0/a1
	tst.l	d0
	beq	.Go_on
	bset	#Right_double,d7		; Yes
	bra	.Go_on
.Go_on:	move.b	Button_state,d0		; Get old button state
	move.b	d0,d2			; Calculate click state
	not.b	d2
	and.b	#$11,d2
	and.b	d7,d2
	move.b	d7,d3			; Calculate release state
	not.b	d3
	and.b	#$11,d3
	and.b	d0,d3
	lsl.w	#1,d2			; Calculate new button state
	lsl.w	#2,d3
	or.b	d2,d7
	or.b	d3,d7
	move.b	d7,Button_state		; Store
	moveq.l	#0,d2			; Write mouse event
	move.b	d7,d2
	move.w	Mouse_X,d0
	move.w	Mouse_Y,d1
	jsr	Write_Mev
	bra	.Next
; ---------- Handle vanilla keys ------------------
.No_2:	cmp.l	#IDCMP_VANILLAKEY,d0	; Vanilla key ?
	bne	.No_3
	movem.l	d1/a0/a1,-(sp)		; Get qualifiers
	kickINP	PeekQualifier
	movem.l	(sp)+,d1/a0/a1
	jsr	Convert_qualifiers		; Convert
	move.b	im_Code+1(a0),d0		; Build complete key code
	jsr	Write_key			; Write to buffer
	bra	.Next
; ---------- Handle raw keys ----------------------
.No_3:	cmp.l	#IDCMP_RAWKEY,d0		; Raw key ?
	bne	.No_4
	move.w	im_Code(a0),d0		; Get raw key code
	btst	#7,d0			; Released ?
	bne	.Next
	move.w	im_Qualifier(a0),d0		; Convert qualifiers
	jsr	Convert_qualifiers
	moveq.l	#0,d1			; Build complete key code
	move.b	im_Code+1(a0),d1
	swap	d1
	or.l	d1,d0
	jsr	Write_key			; Write to buffer
	bra	.Next
; ---------- Handle disk inserted -----------------
.No_4:	cmp.l	#IDCMP_DISKINSERTED,d0	; Disk inserted ?
	bne	.Next
	moveq.l	#0,d0			; Build key event
	bset	#Diskinserted_key,d0
	jsr	Write_key			; Write to buffer
.Next:	move.l	a0,a1			; Reply to message
	kickEXEC	ReplyMsg
	bra	.Again
.Exit:	movem.l	(sp)+,d0-d3/d6/d7/a0-a2
	rts

;***************************************************************************
; [ Convert qualifiers ]
;   IN : d0 - Amiga OS qualifiers (.w)
;  OUT : d0 - Internal OS qualifiers (.l)
; Changed register : d0
;***************************************************************************
Convert_qualifiers:
	movem.l	d1/d2/d7/a1,-(sp)
	lea.l	Qualifier_conversion_table,a1
	moveq.l	#0,d1
	moveq.l	#7,d7
.Loop:	move.b	(a1)+,d2
	btst	d7,d0
	beq.s	.Next
	bset	d2,d1
.Next:	dbra	d7,.Loop
	move.l	d1,d0			; Output
	movem.l	(sp)+,d1/d2/d7/a1
	rts

;***************************************************************************
; [ Read a key from the keyboard buffer ]
;  OUT : d0 - Key code / 0 (.l)
; Changed register : d0
;***************************************************************************
Read_key:
	move.l	a0,-(sp)
	move.l	Key_read_ptr,a0		; Get pointer
	cmp.l	Key_write_ptr,a0		; Buffer empty ?
	bne.s	.Not_empty
	moveq.l	#0,d0			; No key
	bra.s	.Exit
.Not_empty:
	move.l	(a0)+,d0			; Read key
	cmp.l	#Key_end,a0		; End of buffer ?
	blt.s	.Done
	lea.l	Key_start,a0		; Back to start
.Done:	move.l	a0,Key_read_ptr		; Store pointer
.Exit:	move.l	(sp)+,a0
	rts

;***************************************************************************	
; [ Write a key to the keyboard buffer ]
;   IN : d0 - Key code (.l)
; All registers are restored
;***************************************************************************
Write_key:
	movem.l	a0/a1,-(sp)
	move.l	Key_write_ptr,a0		; Get pointer
	move.l	a0,a1
	addq.l	#4,a1
	cmp.l	#Key_end,a1		; End of buffer ?
	blt.s	.Done
	lea.l	Key_start,a1		; Back to start
.Done:	cmp.l	Key_read_ptr,a1		; Buffer full ?
	beq.s	.Exit
	move.l	d0,(a0)			; Write key
	move.l	a1,Key_write_ptr		; Store pointer
.Exit:	movem.l	(sp)+,a0/a1
	rts

;***************************************************************************
; [ Read a mouse event from the buffer ]
;  OUT : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Click state / 0 (.w)
; Changed registers : d0,d1,d2
;***************************************************************************
Read_Mev:
	move.l	a0,-(sp)
	move.l	Mev_read_ptr,a0		; Get pointer
	cmp.l	Mev_write_ptr,a0		; Buffer empty ?
	bne.s	.Not_empty
	moveq.l	#0,d2			; Yes
	bra.s	.Exit
.Not_empty:
	move.w	(a0)+,d0			; Read mouse event
	move.w	(a0)+,d1
	move.w	(a0)+,d2
	cmp.l	#Mev_end,a0		; End of buffer ?
	blt.s	.Done
	lea.l	Mev_start,a0		; Back to start
.Done:	move.l	a0,Mev_read_ptr		; Store pointer
.Exit:	move.l	(sp)+,a0
	rts

;***************************************************************************	
; [ Write a mouse event to the buffer ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Click state (.w)
; All registers are restored
;***************************************************************************
Write_Mev:
	movem.l	a0/a1,-(sp)
	tst.w	d2			; Empty ?
	beq.s	.Exit
	move.l	Mev_write_ptr,a0		; Get pointer
	move.l	a0,a1
	addq.l	#6,a1
	cmp.l	#Mev_end,a1		; End of buffer ?
	blt.s	.Done
	lea.l	Mev_start,a1		; Back to start
.Done:	cmp.l	Mev_read_ptr,a1		; Buffer full ?
	beq.s	.Exit
	move.w	d0,(a0)+			; Write mouse event
	move.w	d1,(a0)+
	move.w	d2,(a0)+
	move.l	a1,Mev_write_ptr		; Store pointer
.Exit:	movem.l	(sp)+,a0/a1
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************
	SECTION	Fast_DATA,data
Mouse_X:	dc.w 160			; Current mouse coordinates
Mouse_Y:	dc.w 100

Input_device_name:	dc.b "input.device",0
	even

Qualifier_conversion_table:
	dc.b Amiga_key		; IEQUALIFIERB_RCOMMAND
	dc.b Amiga_key		; IEQUALIFIERB_LCOMMAND
	dc.b Alternate_key		; IEQUALIFIERB_RALT
	dc.b Alternate_key		; IEQUALIFIERB_LALT
	dc.b Control_key		; IEQUALIFIERB_CONTROL
	dc.b CapsLock_key		; IEQUALIFIERB_CAPSLOCK
	dc.b Shift_key		; IEQUALIFIERB_RSHIFT
	dc.b Shift_key		; IEQUALIFIERB_LSHIFT
	even

Key_read_ptr:	dc.l Key_start
Key_write_ptr:	dc.l Key_start

Mev_read_ptr:	dc.l Mev_start
Mev_write_ptr:	dc.l Mev_start



	SECTION	Fast_BSS,bss
Input_device_opened:	ds.b 1
Button_state:	ds.b 1
	even
Input_port:	ds.l 1
Input_IO_req:	ds.l 1
Input_base:	ds.l 1

Right_time:	ds.l 2		; For double clicks
Left_time:	ds.l 2

Key_start:	ds.l Max_keys		; Key event buffer
Key_end:

Mev_start:	ds.w Max_Mevs*3		; Mouse event buffer
Mev_end:
