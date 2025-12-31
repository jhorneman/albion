; Dungeon movement routines
; Written by J.Horneman (In Tune With The Universe)
; Start : 10-5-94

	SECTION	Program,code
;*****************************************************************************
; [ Touched method for 3D map window object ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
; Notes :
;   - I assume the Earth objects around the 3D map window will switch back
;      to the default mouse pointers.
;*****************************************************************************
M3_touched:
	movem.l	d0-d2/a0,-(sp)
	move.w	Object_self(a0),d0
	move.w	Current_highlighted_object,d2	; Still the same object ?
	cmp.w	d0,d2
	beq.s	.Exit
	move.w	d0,Current_highlighted_object	; No -> Change
	exg.l	d0,d2
	moveq.l	#Draw_method,d1		; Redraw old object
	jsr	Execute_method
	move.w	d2,d0			; Highlight new object
	moveq.l	#Highlight_method,d1
	jsr	Execute_method
	moveq.l	#Help_method,d1		; Has on-line help text ?
	jsr	Has_method
	beq.s	.No_help
	jsr	Execute_method		; Yes -> Get it
	move.w	Help_line_handle,d0		; Print it
	moveq.l	#Print_method,d1
	jsr	Execute_method
	move.w	Object_self(a0),d0
.No_help:	jsr	Update_screen
.Exit:	jsr	Change_3D_mouse		; Change mouse pointer
	movem.l	(sp)+,d0-d2/a0
	rts

;*****************************************************************************
; [ Change the mouse pointer in the 3D map window ]
; All registers are restored
;*****************************************************************************
Change_3D_mouse:
	movem.l	d0-d2/a0,-(sp)
	move.w	Mouse_X,d0
	move.w	Mouse_Y,d1
	subi.w	#Map3D_X,d0		; Translate
	subi.w	#Map3D_Y,d1
	ext.l	d0
	ext.l	d1
	tst.l	d0			; Clip (just to be sure)
	bne.s	.Skip1
	moveq.l	#0,d0
	bra.s	.X_Ok
.Skip1:	divu.w	#Map3D_width/3,d0
	cmp.w	#3,d0
	bmi.s	.X_Ok
	moveq.l	#2,d0
.X_Ok:	tst.l	d1
	bne.s	.Skip2
	moveq.l	#0,d1
	bra.s	.Y_Ok
.Skip2:	divu.w	#Map3D_height/3,d1
	cmp.w	#3,d1
	bmi.s	.Y_Ok
	moveq.l	#2,d1
.Y_Ok:	add.w	d1,d0			; Calculate index
	add.w	d1,d1
	add.w	d0,d1
	move.w	d1,Mouse_direction		; Store number
	move.l	.Mice(pc,d1.w*4),a0		; Set new	mouse pointer
	jsr	Change_Mptr
	movem.l	(sp)+,d0-d2/a0
	rts

.Mice:	dc.l Turnleft_3D_Mptr,Up_3D_Mptr,Turnright_3D_Mptr
	dc.l Left_3D_Mptr,Pop_up_Mptr,Right_3D_Mptr
	dc.l Turn180right_3D_Mptr,Down_3D_Mptr,Turn180left_3D_Mptr

;*****************************************************************************
; [ Clicked method for 3D map window object ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;*****************************************************************************
M3_clicked:
	movem.l	d0/a0,-(sp)

;	include	DDT:Constants/Hardware_registers.i
;	XX	$fff

	move.w	Mouse_direction,d0
	cmp.w	#4,d0			; Wait ?
	bne.s	.No_wait
;	jsr	Zzz			; Yes
	bra	.Exit
.No_wait:	move.w	Dungeon_speed,d0		; Halve initial speed
	lsr.w	#1,d0
;	move.w	d0,Dungeon_speed
;	st	Double_pixel		; Switch to double-pixel
	Push	MA,M3_window_MA
	bra.s	.Entry
.Again:	jsr	Update_display		; Draw
	btst	#Left_pressed,Button_state	; Still pressed ?
	beq.s	.Done
	jsr	Switch_screens		; Yes
.Entry:	jsr	Change_3D_mouse		; Change mouse
	lea.l	.Ptrs,a0			; Move
	move.w	Mouse_direction,d0
	move.l	0(a0,d0.w*4),a0
	jsr	(a0)
	bra	.Again
.Done:	Pop	MA			; Exit
	sf	Double_pixel
.Exit:	movem.l	(sp)+,d0/a0
	rts

.Ptrs:	dc.l .Turnleft,.Forward,.Turnright
	dc.l .Left,Dummy,.Right
	dc.l .Backleft,.Back,.Backright

.Turnleft:
	move.w	Mouse_X,d0		; Get mouse X
	sub.w	#Map3D_X+Map3D_width/3-1,d0	; Negate
	neg.w	d0
	jsr	.Clip			; Clip and scale
	mulu.w	Dungeon_speed,d0		; Increase angle
	lsr.l	#4,d0
	moveq.l	#13,d1
	lsl.l	d1,d0
	add.l	d0,Y_angle
	jmp	.Slowforward

.Forward:
	move.w	Mouse_Y,d0		; Get mouse Y
	sub.w	#Map3D_Y+Map3D_height/3-1,d0	; Negate
	neg.w	d0
	jsr	.Clip			; Clip and scale
	jsr	.Get_scaled_vector		; Get vector
	move.l	Player_X+2,d2		; Move
	move.l	Player_Y+2,d3
	move.w	Player_X,d4
	move.w	Player_Y,d5
	neg.l	d0
	tst.l	d0
	smi	d6
	ext.w	d6
	add.l	d0,d2
	addx.w	d6,d4
	tst.l	d1
	smi	d6
	ext.w	d6
	add.l	d1,d3
	addx.w	d6,d5
	jmp	Try_3D_move		; Try

.Slowforward:
	move.w	Mouse_Y,d0		; Get mouse Y
	sub.w	#Map3D_Y+Map3D_height/3-1,d0	; Negate
	neg.w	d0
	sub.w	#16,d0
	jsr	.Clip			; Clip and scale
	jsr	.Get_scaled_vector		; Get vector
	move.l	Player_X+2,d2		; Move
	move.l	Player_Y+2,d3
	move.w	Player_X,d4
	move.w	Player_Y,d5
	neg.l	d0
	tst.l	d0
	smi	d6
	ext.w	d6
	add.l	d0,d2
	addx.w	d6,d4
	tst.l	d1
	smi	d6
	ext.w	d6
	add.l	d1,d3
	addx.w	d6,d5
	jmp	Try_3D_move		; Try

.Turnright:
	move.w	Mouse_X,d0		; Get mouse X
	subi.w	#Map3D_X+2*(Map3D_width/3),d0
	jsr	.Clip			; Clip and scale
	mulu.w	Dungeon_speed,d0		; Decrease angle
	lsr.l	#4,d0
	moveq.l	#13,d1
	lsl.l	d1,d0
	sub.l	d0,Y_angle
	jmp	.Slowforward

.Left:
	move.w	Mouse_X,d0		; Get mouse X
	subi.w	#Map3D_X+Map3D_width/3-1,d0	; Negate
	neg.w	d0
	lsr.w	#2,d0
	jsr	.Clip			; Clip and scale
	jsr	.Get_scaled_vector		; Get vector
	move.l	Player_X+2,d2		; Move
	move.l	Player_Y+2,d3
	move.w	Player_X,d4
	move.w	Player_Y,d5
	neg.l	d0
	tst.l	d0
	smi	d6
	ext.w	d6
	add.l	d0,d3
	addx.w	d6,d5
	neg.l	d1
	tst.l	d1
	smi	d6
	ext.w	d6
	add.l	d1,d2
	addx.w	d6,d4
	jmp	Try_3D_move		; Try

.Right:
	move.w	Mouse_X,d0		; Get mouse X
	subi.w	#Map3D_X+2*(Map3D_width/3),d0
	lsr.w	#2,d0
	jsr	.Clip			; Clip and scale
	jsr	.Get_scaled_vector		; Get vector
	move.l	Player_X+2,d2		; Move
	move.l	Player_Y+2,d3
	move.w	Player_X,d4
	move.w	Player_Y,d5
	tst.l	d0
	smi	d6
	ext.w	d6
	add.l	d0,d3
	addx.w	d6,d5
	tst.l	d1
	smi	d6
	ext.w	d6
	add.l	d1,d2
	addx.w	d6,d4
	jmp	Try_3D_move		; Try

.Back:
	move.w	Mouse_Y,d0		; Get mouse Y
	subi.w	#Map3D_Y+2*(Map3D_height/3),d0
	lsr.w	#1,d0
	jsr	.Clip			; Clip and scale
	jsr	.Get_scaled_vector		; Get vector
	move.l	Player_X+2,d2		; Move
	move.l	Player_Y+2,d3
	move.w	Player_X,d4
	move.w	Player_Y,d5
	tst.l	d0
	smi	d6
	ext.w	d6
	add.l	d0,d2
	addx.w	d6,d4
	neg.l	d1
	tst.l	d1
	smi	d6
	ext.w	d6
	add.l	d1,d3
	addx.w	d6,d5
	jmp	Try_3D_move		; Try

.Slowback:
	move.w	Mouse_Y,d0		; Get mouse Y
	subi.w	#Map3D_Y+2*(Map3D_height/3)+16,d0
	asr.w	#1,d0
	jsr	.Clip			; Clip and scale
	jsr	.Get_scaled_vector		; Get vector
	move.l	Player_X+2,d2		; Move
	move.l	Player_Y+2,d3
	move.w	Player_X,d4
	move.w	Player_Y,d5
	tst.l	d0
	smi	d6
	ext.w	d6
	add.l	d0,d2
	addx.w	d6,d4
	neg.l	d1
	tst.l	d1
	smi	d6
	ext.w	d6
	add.l	d1,d3
	addx.w	d6,d5
	jmp	Try_3D_move		; Try

.Backright:
	move.w	Mouse_X,d0		; Get mouse X
	subi.w	#Map3D_X+2*(Map3D_width/3),d0
	lsr.w	#1,d0
	jsr	.Clip			; Clip and scale
	mulu.w	Dungeon_speed,d0		; Decrease angle
	lsr.l	#4,d0
	moveq.l	#13,d1
	lsl.l	d1,d0
	sub.l	d0,Y_angle
	jmp	.Slowback

.Backleft:
	move.w	Mouse_X,d0		; Get mouse X
	subi.w	#Map3D_X+Map3D_width/3-1,d0	; Negate
	neg.w	d0
	lsr.w	#1,d0
	jsr	.Clip			; Clip and scale
	mulu.w	Dungeon_speed,d0		; Increase angle
	lsr.l	#4,d0
	moveq.l	#13,d1
	lsl.l	d1,d0
	add.l	d0,Y_angle
	jmp	.Slowback

.Clip:
	tst.w	d0			; Clip bottom
	bpl.s	.Ok1
	moveq.l	#0,d0
	bra.s	.Ok2
.Ok1:	cmp.w	#Map3D_width/3,d0		; Clip top
	bmi.s	.Ok2
	moveq.l	#Map3D_width/3-1,d0
.Ok2:	lsr.w	#1,d0			; Divide
	addq.w	#1,d0
	rts

; [ Get scaled movement vector ]
;   IN : d0 - Scale factor x 16 (.w)
;  OUT : d0 - Vector X-component (.l)
;        d1 - Vector Y-component (.l)
; Changed registers : d0,d1
.Get_scaled_vector:
	movem.l	d2/a0,-(sp)
	mulu.w	Dungeon_speed,d0		; Scale
	lsr.l	#4,d0
	move.w	d0,d2
	lea.l	Sine_table,a0
	move.w	Y_angle,d1
	and.w	#slang-1,d1
	add.w	d1,d1
	move.w	0(a0,d1.w),d0		; Sine
	add.w	#slang/2,d1
	and.w	#slang*2-1,d1
	move.w	0(a0,d1.w),d1		; Cosine
	muls.w	d2,d0
	muls.w	d2,d1
	asl.l	#2,d0			; 16 bit nachkomma !
	asl.l	#2,d1
	movem.l	(sp)+,d2/a0
	rts

Try_3D_move:
	move.l	d2,d0
	move.l	d3,d1
	move.w	d4,d0
	move.w	d5,d1
	swap	d0
	swap	d1
	jsr	Dungeon_to_map
	cmp.w	#1,d0
	ble	.Exit
	cmp.w	#1,d1
	ble	.Exit
	cmp.w	Width_of_map,d0
	bpl	.Exit
	cmp.w	Height_of_map,d1
	bpl	.Exit
	move.l	d2,Player_X+2
	move.l	d3,Player_Y+2
	move.w	d4,Player_X
	move.w	d5,Player_Y
.Exit:	rts

;*****************************************************************************
; [ DLeft method for 3D map window object ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;*****************************************************************************
M3_dleft:
	movem.l	d0/a1,-(sp)
	jsr	Change_3D_mouse		; Change mouse
	move.w	Mouse_direction,d0		; Do
	move.l	.Ptrs(pc,d0.w*4),a1
	jsr	(a1)
	movem.l	(sp)+,d0/a1
	rts

.Ptrs:	dc.l Left90_3D,Dummy,Right90_3D
	dc.l Dummy,Dummy,Dummy
	dc.l Left180_3D,Dummy,Right180_3D

;*****************************************************************************
; [ 3D map - 180 degrees left ]
; All registers are restored
;*****************************************************************************
Left180_3D:
	movem.l	d0/d7,-(sp)
	move.w	Dungeon_speed,d0		; Halve initial speed
	lsr.w	#1,d0
;	move.w	d0,Dungeon_speed
;	st	Double_pixel		; Switch to double-pixel
	moveq.l	#Fullturn_steps-1,d7
	bra.s	.Entry
.Loop:	jsr	Switch_screens
	add.l	#slang*32768/Fullturn_steps,Y_angle	; Rotate
.Entry:	jsr	M3_DisUpd			; Display
	dbra	d7,.Loop
	sf	Double_pixel
	movem.l	(sp)+,d0/d7
	rts

;*****************************************************************************
; [ 3D map - 180 degrees right ]
; All registers are restored
;*****************************************************************************
Right180_3D:
	movem.l	d0/d7,-(sp)
	move.w	Dungeon_speed,d0		; Halve initial speed
	lsr.w	#1,d0
;	move.w	d0,Dungeon_speed
;	st	Double_pixel		; Switch to double-pixel
	moveq.l	#Fullturn_steps-1,d7
	bra.s	.Entry
.Loop:	jsr	Switch_screens
.Entry:	sub.l	#slang*32768/Fullturn_steps,Y_angle	; Rotate
	jsr	M3_DisUpd			; Display
	dbra	d7,.Loop
	sf	Double_pixel
	movem.l	(sp)+,d0/d7
	rts

;*****************************************************************************
; [ 3D map - 90 degrees left ]
; All registers are restored
;*****************************************************************************
Left90_3D:
	movem.l	d0/d7,-(sp)
	move.w	Dungeon_speed,d0		; Halve initial speed
	lsr.w	#1,d0
;	move.w	d0,Dungeon_speed
;	st	Double_pixel		; Switch to double-pixel
	moveq.l	#Fullturn_steps/2-1,d7
	bra.s	.Entry
.Loop:	jsr	Switch_screens
	add.l	#slang*32768/Fullturn_steps,Y_angle	; Rotate
.Entry:	jsr	M3_DisUpd			; Display
	dbra	d7,.Loop
	sf	Double_pixel
	movem.l	(sp)+,d0/d7
	rts

;*****************************************************************************
; [ 3D map - 90 degrees right ]
; All registers are restored
;*****************************************************************************
Right90_3D:
	movem.l	d0/d7,-(sp)
	move.w	Dungeon_speed,d0		; Halve initial speed
	lsr.w	#1,d0
;	move.w	d0,Dungeon_speed
;	st	Double_pixel		; Switch to double-pixel
	moveq.l	#Fullturn_steps/2-1,d7
	bra.s	.Entry
.Loop:	jsr	Switch_screens
.Entry:	sub.l	#slang*32768/Fullturn_steps,Y_angle	; Rotate
	jsr	M3_DisUpd			; Display
	dbra	d7,.Loop
	sf	Double_pixel
	movem.l	(sp)+,d0/d7
	rts

;*****************************************************************************
; [ Get direction vector from current angle ]
;  OUT : d0 - Vector X-component (.l)
;        d1 - Vector Y-component (.l)
; Changed registers : d0,d1
;*****************************************************************************
Get_3D_direction:
	move.l	a0,-(sp)
	lea.l	Sine_table,a0
	move.w	Y_angle,d1
	and.w	#slang-1,d1
	add.w	d1,d1
	move.w	0(a0,d1.w),d0		; Sine
	add.w	#slang/2,d1
	and.w	#slang*2-1,d1
	move.w	0(a0,d1.w),d1		; Cosine
	muls.w	Dungeon_speed,d0
	muls.w	Dungeon_speed,d1
	asl.l	#2,d0			; 16 bit nachkomma !
	asl.l	#2,d1
	move.l	(sp)+,a0
	rts
