
Map2D_X	EQU 16
Map2D_Y	EQU 0
Mapbuf_width	EQU 20
Mapbuf_height	EQU 12
Mapbuf_size	EQU Mapbuf_width*Mapbuf_height

	rsreset
Icon_bits:	rs.l 1
First_frame:	rs.w 1
Nr_frames:	rs.b 1
Minimap_colour:	rs.b 1
; Data size must be 8 bytes because of scaled addressing modes !!!

	rsreset
Circle_wave_bit:	rs.b 1
Underlay_priority_bit:	rs.b 1
Overlay_bit:	rs.b 1
Asynchronous_bit:	rs.b 1
Random_anim_bit:	rs.b 1
Icon_height:	rs.b 2
Blocked_direction:	rs.b 4
Blocked_travelmodes:	rs.b 6
	rs.b 4
Person_invisible_bit:	rs.b 1
Add_next_icon_bit:	rs.b 1
Sit_sleep_state:	rs.b 4
Combat_background_nr:	rs.b 5

	rsreset
North:	rs.b 1
East:	rs.b 1
South:	rs.b 1
West:	rs.b 1

	rsreset
	rs.w 8
Mapbuf_entry_size:	rs.b 0

Line_size	equ Mapbuf_width*Mapbuf_entry_size


	XREF	Icons
	XREF	Mannetjes
	XREF	Test_pal


	SECTION	Program,code
Init_program:
	lea.l	Test_pal,a0
	moveq.l	#0,d0
	move.w	#192,d1
	jsr	Set_palette_part

	lea.l	Main_palette,a0
	move.w	#192,d0
	moveq.l	#64,d1
	jsr	Set_palette_part

.Again:	jsr	Zong
.Wait:	jsr	Read_key
	tst.l	d0
	beq.s	.Wait
	jsr	Reset_keyboard
	cmp.b	#" ",d0
	beq	.Exit
	swap	d0
	cmp.b	#$4e,d0
	bne.s	.Not_left
	cmp.w	#Mapbuf_width-2,Party_X
	beq.s	.Wait
	move.w	#East,View_direction
	moveq.l	#1,d0
	moveq.l	#0,d1
	jsr	Kerchung
	bra.s	.Again
.Not_left:
	cmp.b	#$4f,d0
	bne.s	.Not_right
	tst.w	Party_X
	beq.s	.Wait
	move.w	#West,View_direction
	moveq.l	#-1,d0
	moveq.l	#0,d1
	jsr	Kerchung
	bra.s	.Again
.Not_right:
	cmp.b	#$4c,d0
	bne.s	.Not_up
	tst.w	Party_Y
	beq.s	.Wait
	move.w	#North,View_direction
	moveq.l	#0,d0
	moveq.l	#-1,d1
	jsr	Kerchung
	bra	.Again
.Not_up:
	cmp.b	#$4d,d0
	bne	.Again
	cmp.w	#Mapbuf_height-1,Party_Y
	beq	.Wait
	move.w	#South,View_direction
	moveq.l	#0,d0
	moveq.l	#1,d1
	jsr	Kerchung
	bra	.Again

.Exit:	jmp	Exit_program

Kerchung:
	add.w	Party_X,d0
	add.w	Party_Y,d1
	lea.l	Map_buffers,a0
	move.w	d1,d2
	mulu.w	#Mapbuf_width,d2
	add.w	d0,d2
	mulu.w	#Mapbuf_entry_size,d2
	tst.w	Overlay(a0,d2.w)
	bne.s	.Skip
	tst.w	Overlay+Mapbuf_entry_size(a0,d2.w)
	bne.s	.Skip
	tst.b	People(a0,d2.w)
	bne.s	.Skip
	tst.w	d0
	beq.s	.Left
	tst.b	People-Mapbuf_entry_size(a0,d2.w)
	bne.s	.Skip
.Left:	cmp.w	#Mapbuf_width-1,d0
	bpl.s	.Right
	tst.b	People+Mapbuf_entry_size(a0,d2.w)
	bne.s	.Skip
.Right:	move.w	d0,Party_X
	move.w	d1,Party_Y
.Skip:	rts

Zong:
	jsr	Begin_graphics_ops

	lea.l	Map_buffers,a2
	move.w	Party_Y,d0
	mulu.w	#Mapbuf_width,d0
	add.w	Party_X,d0
	mulu.w	#Mapbuf_entry_size,d0
	add.w	d0,a2
	move.w	View_direction,d1
	addq.w	#1,d1
	move.b	d1,People(a2)

;	move.b	#2,Update(a2)
;	move.b	#2,Update+Mapbuf_entry_size(a2)
;	move.b	#2,Update-Line_size(a2)
;	move.b	#2,Update+Mapbuf_entry_size-Line_size(a2)
;	move.b	#2,Update-2*Line_size(a2)
;	move.b	#2,Update+Mapbuf_entry_size-2*Line_size(a2)

	lea.l	Map_buffers,a1
	moveq.l	#Map2D_Y,d1
	moveq.l	#Mapbuf_height-1,d7
.Loop_Y:	moveq.l	#Map2D_X,d0
	moveq.l	#Mapbuf_width-1,d6
.Loop_X:
;	tst.b	Update(a1)
;	beq.s	.Next_X
	move.w	Underlay(a1),d2
	beq.s	.No
	lea.l	Underlays,a0
	subq.w	#1,d2
	lsl.w	#8,d2
	add.w	d2,a0
	jsr	Put_unmasked_icon
.No:	move.w	Overlay(a1),d2
	beq.s	.Next_X
	lea.l	Overlays,a0
	subq.w	#1,d2
	lsl.w	#8,d2
	add.w	d2,a0
	jsr	Put_masked_icon
.Next_X:	lea.l	Mapbuf_entry_size(a1),a1
	add.w	#16,d0
	dbra	d6,.Loop_X
	add.w	#16,d1
	dbra	d7,.Loop_Y

	LOCAL

	lea.l	Map_buffers+Line_size,a1
	moveq.l	#Map2D_Y+16,d1
	moveq.l	#Mapbuf_height-2,d7
.Loop_Y:	moveq.l	#Map2D_X,d0
	moveq.l	#Mapbuf_width-1,d6
.Loop_X:
;	tst.b	Update(a1)
;	beq.s	.Next_X
	move.b	People(a1),d2
	beq.s	.No
	movem.l	d0/d1/d5-d7,-(sp)
	sub.w	#32,d1
	moveq.l	#Screen_depth,d5
	moveq.l	#2,d6
	moveq.l	#48,d7
	lea.l	Mannetjes,a0
	and.w	#$00ff,d2
	subq.w	#1,d2
	mulu.w	#6*256,d2
	add.w	d2,a0
	jsr	Put_masked_block
	movem.l	(sp)+,d0/d1/d5-d7
.No:	move.w	Matrix_overlay-Line_size(a1),d2
	beq.s	.Next_X
	move.l	d1,-(sp)
	sub.w	#16,d1
	lea.l	Overlays,a0
	subq.w	#1,d2
	lsl.w	#8,d2
	add.w	d2,a0
	jsr	Put_masked_icon
	move.l	(sp)+,d1
.Next_X:	lea.l	Mapbuf_entry_size(a1),a1
	add.w	#16,d0
	dbra	d6,.Loop_X
	add.w	#16,d1
	dbra	d7,.Loop_Y

	LOCAL

	lea.l	Map_buffers,a1
	moveq.l	#Map2D_Y,d1
	moveq.l	#Mapbuf_height-1,d7
.Loop_Y:	moveq.l	#Map2D_X,d0
	moveq.l	#Mapbuf_width-1,d6
.Loop_X:
;	tst.b	Update(a1)
;	beq.s	.Next_X
;	subq.b	#1,Update(a1)
	move.w	Sky_underlay(a1),d2
	beq.s	.No
	lea.l	Underlays,a0
	subq.w	#1,d2
	lsl.w	#8,d2
	add.w	d2,a0
	jsr	Put_unmasked_icon
.No:	move.w	Sky_overlay(a1),d2
	beq.s	.Next_X
	lea.l	Overlays,a0
	subq.w	#1,d2
	lsl.w	#8,d2
	add.w	d2,a0
	jsr	Put_masked_icon
.Next_X:	lea.l	Mapbuf_entry_size(a1),a1
	add.w	#16,d0
	dbra	d6,.Loop_X
	add.w	#16,d1
	dbra	d7,.Loop_Y

	jsr	End_graphics_ops
	jsr	Update_screen
	clr.b	People(a2)
	rts

	SECTION	Fast_DATA,data
Party_X:	dc.w 10
Party_Y:	dc.w 4
View_direction:	dc.w South
Icon_data:
;	      33222222222211111111110000000000
;	      10987654321098765432109876543210
	dc.l %00000000000000000000000000000000
	dc.w 0
	dc.b 1,0
	dc.l %00000000000000000000000000000000
	dc.w 0
	dc.b 2,0
	dc.l %00000000000000000000000000000100
	dc.w 0
	dc.b 3,0
	dc.l %00000000000000000000000000100100
	dc.w 0
	dc.b 4,0
	dc.l %00000000000000000000000001000100
	dc.w 0
	dc.b 5,0

Map_buffers:
	rept Mapbuf_width/2
	dc.w 1,0,0,0,0,0,0,0
	dc.w 2,0,0,0,0,0,0,0
	endr
	rept Mapbuf_width/2
	dc.w 2,0,0,0,0,0,0,0
	dc.w 1,0,0,0,0,0,0,0
	endr
	rept Mapbuf_width/2
	dc.w 1,0,0,0,0,0,0,0
	dc.w 2,0,0,0,0,0,0,0
	endr
	rept Mapbuf_width/2
	dc.w 2,0,0,0,0,0,0,0
	dc.w 1,0,0,0,0,0,0,0
	endr
	rept Mapbuf_width/2
	dc.w 1,0,0,0,0,0,0,0
	dc.w 2,0,0,0,0,0,0,0
	endr

	rept Mapbuf_width/4
	dc.w 2,0,0,0,0,0,0,0
	dc.w 1,0,0,0,0,0,0,0
	dc.w 2,0,0,0,0,0,0,0
	dc.w 1,0,0,0,0,0,0,0
	endr
	rept Mapbuf_width/4
	dc.w 1,0,0,0,0,0,0,0
	dc.w 2,0,0,0,0,0,0,0
	dc.w 1,0,0,0,0,0,0,0
	dc.w 2,0,0,0,0,0,0,0
	endr
	rept Mapbuf_width/4
	dc.w 2,0,0,0,0,0,0,0
	dc.w 1,0,0,0,0,0,0,0
	dc.w 2,0,0,0,0,0,0,0
	dc.w 1,0,0,0,0,0,0,0
	endr
	rept Mapbuf_width/4
	dc.w 1,0,0,0,0,0,0,0
	dc.w 2,0,0,0,0,0,0,0
	dc.w 1,0,0,0,0,0,0,0
	dc.w 2,0,0,0,0,0,0,0
	endr
	rept Mapbuf_width/4
	dc.w 2,0,0,0,0,0,0,0
	dc.w 1,0,0,0,0,0,0,0
	dc.w 2,0,0,0,0,0,0,0
	dc.w 1,0,0,0,0,0,0,0
	endr

	rept Mapbuf_width/2
	dc.w 1,0,0,0,0,0,0,0
	dc.w 2,0,0,0,0,0,0,0
	endr
	rept Mapbuf_width/2
	dc.w 2,0,0,0,0,0,0,0
	dc.w 1,0,0,0,0,0,0,0
	endr
