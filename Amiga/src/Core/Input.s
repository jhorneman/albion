; Input handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	XDEF	Read_key
	XDEF	Read_Mev
	XDEF	Button_state
	XDEF	Mouse_X
	XDEF	Mouse_Y

	SECTION	Program,code
;***************************************************************************
; [ Initialize input handling ]
; All registers are restored
;***************************************************************************
Init_input:
	movem.l	d0/d1/a0/a1,-(sp)
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
; ---------- Install Input handler ----------------
	jsr	Create_port		; Create port
	move.l	d0,Input_port
	jsr	Create_IO_request		; Create I/O request
	move.l	d0,Input_IO_req
	lea.l	Input_device_name,a0	; Open device
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
	move.l	Input_IO_req,a1		; Do I/O
	move.w	#IND_ADDHANDLER,io_Command(a1)
	move.l	#Input_handler,io_Data(a1)
	kickEXEC	DoIO
	tst.b	d0			; Success ?
	beq.s	.Ok
	and.l	#$000000ff,d0		; No -> Exit
	move.l	d0,Extended_error_code
	move.l	#ADD_INPUT_HANDLER_ERROR,d0
	jmp	Fatal_error
.Ok:	st	Input_handler_added
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************
; [ Exit input handling ]
; All registers are restored
;***************************************************************************
Exit_input:
	movem.l	d0/d1/a0/a1,-(sp)
; --------- Remove input handler ------------------
	tst.b	Input_handler_added		; Input handler added ?
	beq.s	.No
	move.l	Input_IO_req,a1		; Yes -> Remove
	move.w	#IND_REMHANDLER,io_Command(a1)
	move.l	#Input_handler,io_Data(a1)
	kickEXEC	DoIO
	sf	Input_handler_added
.No:	LOCAL
	tst.b	Input_device_opened		; Input device opened ?
	beq.s	.No
	move.l	Input_IO_req,a1		; Close device
	kickEXEC	CloseDevice
	sf	Input_device_opened
.No:	LOCAL
	move.l	Input_IO_req,a0		; Destroy I/O request
	jsr	Destroy_IO_request
	clr.l	Input_IO_req
	move.l	Input_port,a0		; Destroy port
	jsr	Destroy_port
	clr.l	Input_port
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
; [ My input handler ]
;   IN : a0 - Pointer to input event (.l)
;        a1 - Pointer to interrupt data (.l)
;  OUT : d0 - Pointer to input event / 0 (.l)
; Changed registers : d0
;***************************************************************************
My_input_handler:
	movem.l	d0-d7/a0-a2,-(sp)
.Again:	cmp.l	#0,a0			; Event ?
	beq	.Exit
	moveq.l	#0,d0			; Get event class
	move.b	ie_Class(a0),d0
; ---------- Handle raw mouse ---------------------
	cmp.w	#IECLASS_RAWMOUSE,d0	; Raw mouse ?
	bne	.No_mouse
	tst.l	MA_Sp			; Stack initialized ?
	beq	.Next
	move.l	MA_Sp,a1			; Get area address
	move.l	(a1),a1
	move.w	ie_X(a0),d0		; Get mouse coordinates
	move.w	ie_Y(a0),d1
	add.w	Mouse_X,d0
	add.w	Mouse_Y,d1
	cmp.w	X1(a1),d0			; X too low ?
	bpl.s	.Left_OK
	move.w	X1(a1),d0
	bra.s	.Check_Y
.Left_OK:	cmp.w	X2(a1),d0			; X too high ?
	blt.s	.Check_Y
	move.w	X2(a1),d0
.Check_Y:	cmp.w	Y1(a1),d1			; Y too low ?
	bpl.s	.Top_OK
	move.w	Y1(a1),d1
	bra.s	.Done
.Top_OK:	cmp.w	Y2(a1),d1			; Y too high ?
	blt.s	.Done
	move.w	Y2(a1),d1
.Done:	move.w	d0,Mouse_X		; Store coordinates
	move.w	d1,Mouse_Y
	move.w	ie_Qualifier(a0),d6		; Any button news ?
	and.w	#ie_button_mask,d6
	moveq.l	#0,d7			; Build button state
	btst	#IEQUALIFIERB_LEFTBUTTON,d6	; Left button down ?
	beq.s	.No_left
	bset	#Left_pressed,d7		; Yes
.No_left:	btst	#IEQUALIFIERB_RBUTTON,d6	; Right button down ?
	beq.s	.Go_on
	bset	#Right_pressed,d7		; Yes
.Go_on:	move.b	Button_state,d0		; Get old button state
	move.b	d0,d2			; Calculate click state
	not.b	d2
	and.b	#$11,d2
	and.b	d7,d2
	move.b	d7,d3			; Calculate release state
	not.b	d3
	and.b	#$11,d3
	and.b	d0,d3
	add.w	d2,d2			; Calculate new button state
	lsl.w	#2,d3
	or.b	d2,d7
	or.b	d3,d7
	btst	#Left_clicked,d7		; Left clicked ?
	beq.s	.No_dleft
	move.l	Left_time,d0		; Yes -> Get previous time
	move.l	VBL_counter,d1		; Get current time
	move.l	d1,Left_time		; Is new previous
	sub.l	d0,d1			; Within interval ?
	cmp.l	#Dclick_interval,d1
	bpl.s	.No_dleft
	bset	#Left_double,d7		; Yes
	clr.l	Left_time
.No_dleft:
	btst	#Right_clicked,d7		; Right clicked ?
	beq.s	.No_dright
	move.l	Right_time,d0		; Yes -> Get previous time
	move.l	VBL_counter,d1		; Get current time
	move.l	d1,Right_time		; Is new previous
	sub.l	d0,d1			; Within interval ?
	cmp.l	#Dclick_interval,d1
	bpl.s	.No_dright
	bset	#Right_double,d7		; Yes
	clr.l	Right_time
.No_dright:
	cmp.b	Button_state,d7		; Any change ?
	beq	.Next
	move.b	d7,Button_state		; Yes -> Store
	moveq.l	#0,d2			; Write mouse event
	move.b	d7,d2
	move.w	Mouse_X,d0
	move.w	Mouse_Y,d1
	jsr	Write_Mev
	bra	.Next
; ---------- Handle raw keys ----------------------
.No_mouse:	cmp.w	#IECLASS_RAWKEY,d0		; Raw key ?
	bne	.No_key
	move.w	ie_Code(a0),d0		; Get raw key code
	btst	#7,d0			; Released ?
	bne	.No_key
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
; ---------- Handle disk inserts ------------------
.No_key:	cmp.w	#IECLASS_DISKINSERTED,d0	; Disk inserted ?
	bne.s	.Next
	moveq.l	#0,d0			; Build key event
	bset	#Diskinserted_key,d0
	jsr	Write_key			; Write to buffer
.Next:	move.l	ie_NextEvent(a0),a0		; Link to next event
	bra	.Again
.Exit:	movem.l	(sp)+,d0-d7/a0-a2
	move.l	a0,d0
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
Mouse_X:	dc.w Screen_width/2		; Current mouse coordinates
Mouse_Y:	dc.w Screen_height/2

Console_device_name:	dc.b "console.device",0
Input_device_name:	dc.b "input.device",0
	even

Input_handler:
	dc.l 0,0
	dc.b NT_INTERRUPT
	dc.b 127			; Priority = maximum
	dc.l DDT_name
	dc.l 0
	dc.l My_input_handler

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
Input_device_opened:	ds.b 1
Input_handler_added:	ds.b 1
Button_state:	ds.b 1
	even
Console_port:	ds.l 1
Console_IO_req:	ds.l 1
Console_base:	ds.l 1
Input_port:	ds.l 1
Input_IO_req:	ds.l 1
Input_base:	ds.l 1

Right_time:	ds.l 1		; For double clicks
Left_time:	ds.l 1

Key_start:	ds.l Max_keys		; Key event buffer
Key_end:
Key_convert_buffer:	ds.w 1

Mev_start:	ds.w Max_Mevs*3		; Mouse event buffer
Mev_end:
