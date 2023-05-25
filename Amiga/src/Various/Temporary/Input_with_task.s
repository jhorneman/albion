; Input handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	MODULE	DDT_Input

	OUTPUT	Objects/Input.o

	OPT	O1+,O2+,O4+,O5+,O10+,O11+,OW-
	OPT	O7+,EVEN,USER,INCONCE
	OPT	P=68020


	XREF	Create_port
	XREF	Destroy_port
	XREF	Create_IO_request
	XREF	Destroy_IO_request
	XREF	Extended_error_code
	XREF	Fatal_error
	XREF	My_window
	XREF	Forbid_it
	XREF	Permit_it
	XREF	DDT_name
	XREF	MA_Sp

	XDEF	Init_input
	XDEF	Exit_input
	XDEF	Read_key
	XDEF	Read_Mev
	XDEF	Button_state
	XDEF	Mouse_X
	XDEF	Mouse_Y


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
; ---------- Open Console device ------------------
	jsr	Create_port		; Create port
	move.l	d0,Console_port
	jsr	Create_IO_request		; Create I/O request
	move.l	d0,Console_IO_req
	lea.l	Console_device_name,a0	; Open Console device
	move.l	d0,a1
	move.l	#CONU_LIBRARY,d0
	moveq.l	#0,d1
	kickEXEC	OpenDevice
	tst.b	d0			; Success ?
	beq.s	.Ok
	and.l	#$000000ff,d0		; No -> Exit
	move.l	d0,Extended_error_code
	move.l	#OPEN_DEVICE_ERROR,d0
	jmp	Fatal_error
.Ok:	st	Console_device_opened
	LOCAL
	move.l	Console_IO_req,a0		; Store library base
	move.l	io_Device(a0),Console_base
; ---------- Add input handler task ---------------
	lea.l	My_input_task,a1
	lea.l	Input_handler,a2
	sub.l	a3,a3
	kickEXEC	AddTask
	movem.l	(sp)+,d0/d1/a0-a3
	rts

;***************************************************************************
; [ Exit input handling ]
; All registers are restored
;***************************************************************************
Exit_input:
	movem.l	d0/d1/a0/a1,-(sp)
; --------- Remove input handler task -------------
	lea.l	My_input_task,a1
	kickEXEC	RemTask
; ---------- Close Console device -----------------
	tst.b	Console_device_opened	; Console device opened ?
	beq.s	.No
	move.l	Console_IO_req,a1		; Close device
	kickEXEC	CloseDevice
	sf	Console_device_opened
.No:	LOCAL
	move.l	Console_IO_req,a0		; Destroy I/O request
	jsr	Destroy_IO_request
	clr.l	Console_IO_req
	move.l	Console_port,a0		; Destroy port
	jsr	Destroy_port
	clr.l	Console_port
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************
; [ Input handler task ]
;***************************************************************************
Input_handler:
.Wait:	move.l	My_window,a0		; Wait for first message
	move.l	wd_UserPort(a0),a0
	kickEXEC	WaitPort
.Again:	move.l	My_window,a0		; Get message
	move.l	wd_UserPort(a0),a0
	kickEXEC	GetMsg
	tst.l	d0			; Anything ?
	beq	.Wait
	move.l	d0,a0			; Yes
	move.w	im_Class(a0),d0		; Get class
; ---------- Handle mouse movement ----------------
	cmp.w	#IDCMP_MOUSEMOVE,d0		; Mouse movement ?
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
.No_1:	cmp.w	#IDCMP_MOUSEBUTTONS,d0	; Mouse buttons ?
	bne	.No_2
	move.b	Button_state,d0		; Get old button state
	moveq.l	#0,d1
	move.w	im_Code(a0),d2		; Get OS button state
	btst	#SELECTDOWN,d2		; Check left mouse button
	beq.s	.No_left
	bset	#Left_pressed,d1
.No_left:	btst	#MENUDOWN,d2		; Check right mouse button
	beq.s	.No_right
	bset	#Right_pressed,d1
.No_right:	move.b	d0,d2			; Calculate click state
	not.b	d2
	and.b	#$11,d2
	and.b	d1,d2
	move.b	d1,d3			; Calculate release state
	not.b	d3
	and.b	#$11,d3
	and.b	d0,d3
	lsl.w	#1,d2			; Calculate new button state
	lsl.w	#2,d3
	or.b	d2,d1
	or.b	d3,d1
	move.b	d1,Button_state		; Store
	moveq.l	#0,d2			; Write mouse event
	move.b	d1,d2
	move.w	Mouse_X,d0
	move.w	Mouse_Y,d1
	jsr	Write_Mev
	bra	.Next
; ---------- Handle raw keys ----------------------
.No_2:	cmp.w	#IDCMP_RAWKEY,d0		; Raw key ?
	bne	.No_3
	move.w	im_Code(a0),d0		; Get raw key code
	btst	#7,d0			; Released ?
	bne	.Next
	movem.l	d1/a0/a1,-(sp)		; Convert raw key
	lea.l	Key_convert_buffer,a1
	sub.l	a2,a2
	moveq.l	#1,d1
	kickCON	RawKeyConvert
	movem.l	(sp)+,d1/a0/a1
	cmp.l	#1,d0			; Error ?
	beq.s	.Ok
	clr.b	Key_convert_buffer		; ASCII code 0
.Ok:	move.w	ie_Qualifier(a0),d0		; Convert qualifier
	moveq.l	#0,d1
	lea.l	Qualifier_conversion_table,a1
	moveq.l	#7,d7
.Loop2:	move.b	(a1)+,d2
	btst	d7,d0
	beq.s	.Next2
	bset	d2,d1
.Next2:	dbra	d7,.Loop2
	moveq.l	#0,d0			; Build complete key code
	move.b	ie_Code+1(a0),d0
	swap	d0
	or.b	Key_convert_buffer,d0
	or.l	d1,d0
	jsr	Write_key			; Write to buffer
	bra	.Next
; ---------- Handle disk inserted -----------------
.No_3:	cmp.w	#IDCMP_DISKINSERTED,d0	; Disk inserted ?
	bne	.Next
	moveq.l	#0,d0			; Build key event
	bset	#Diskinserted_key,d0
	jsr	Write_key			; Write to buffer
.Next:	move.l	a0,a1			; Reply to message
	kickEXEC	ReplyMsg
	bra	.Again

;***************************************************************************
; [ Read a key from the keyboard buffer ]
;  OUT : d0 - Key code / 0 (.l)
; Changed register : d0
;***************************************************************************
Read_key:
	move.l	a0,-(sp)
	jsr	Forbid_it
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
.Exit:	jsr	Permit_it
	move.l	(sp)+,a0
	rts

;***************************************************************************	
; [ Write a key to the keyboard buffer ]
;   IN : d0 - Key code (.l)
; All registers are restored
;***************************************************************************
Write_key:
	movem.l	a0/a1,-(sp)
	jsr	Forbid_it
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
.Exit:	jsr	Permit_it
	movem.l	(sp)+,a0/a1
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
	jsr	Forbid_it
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
.Exit:	jsr	Permit_it
	move.l	(sp)+,a0
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
	jsr	Forbid_it
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
.Exit:	jsr	Permit_it
	movem.l	(sp)+,a0/a1
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************
	SECTION	Fast_DATA,data
Console_device_name:	dc.b "console.device",0
	even

My_input_task:
	dc.l 0,0
	dc.b NT_TASK
	dc.b 50
	dc.l DDT_name
	dc.b 0,0,0,0
	dc.l 0,0,0,0
	dc.w 0,0
	dc.l 0,0,0,0
	dc.l Stack_upper,Stack_lower,Stack_upper
	dc.l 0,0
	dcb.b lh_data_size,0
	dc.l 0

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
Console_device_opened:	ds.b 1
Button_state:	ds.b 1
	even
Console_port:	ds.l 1
Console_IO_req:	ds.l 1
Console_base:	ds.l 1
Input_port:	ds.l 1
Input_IO_req:	ds.l 1
Input_base:	ds.l 1

Mouse_X:	ds.w 1			; Current mouse coordinates
Mouse_Y:	ds.w 1

Key_start:	ds.l Max_keys		; Key event buffer
Key_end:
Key_convert_buffer:	ds.w 1

Mev_start:	ds.w Max_Mevs*3		; Mouse event buffer
Mev_end:

Stack_lower:
	ds.b 1000
Stack_upper:
