

	XREF	Icons
	XREF	Mannetjes
	XREF	Test_pal


;	move.l	Icon_bits(a1),d2
;	move.b	Icon_nr_frames(a1),d4
;	jsr	Get_animation_frame
;	add.w	Icon_1st_frame(a1),d4
;	lsl.l	#8,d4


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

;	moveq.l	#0,d0
;	moveq.l	#0,d1
;	move.w	#Screen_width-1,d2
;	move.w	#Screen_height-1,d3
;	move.w	#White,d4
;	jsr	Draw_box
;	jsr	Update_screen

;	lea.l	Yo_ho_HDOB,a0
;	lea.l	Test_block,a1
;	move.w	(a1)+,d0
;	add.w	#15,d0
;	lsr.w	#4,d0
;	move.w	d0,HDOB_symbol+Symbol_width(a0)
;	move.w	(a1)+,HDOB_symbol+Symbol_height(a0)
;	move.b	(a1)+,HDOB_symbol+Symbol_depth(a0)
;	jsr	Add_HDOB

	lea.l	Object2D_list,a0
	move.l	#Mannetje_object,(a0)+
	move.l	#Object_twee,(a0)+
	move.l	#Object_drie,(a0)+
	move.l	#Object_vier,(a0)+
	move.l	#Object_vijf,(a0)+
	move.w	#5,Nr_2D_objects

	lea.l	Test_map,a0
	moveq.l	#30/2-1,d7
.Loop_Y:	moveq.l	#30/2-1,d6
.Loop_X1:	or.b	#$10,1(a0)
	or.b	#$20,4(a0)
	addq.l	#6,a0
	dbra	d6,.Loop_X1
	moveq.l	#30/2-1,d6
.Loop_X2:	or.b	#$20,1(a0)
	or.b	#$10,4(a0)
	addq.l	#6,a0
	dbra	d6,.Loop_X2
	dbra	d7,.Loop_Y
	LOCAL

	lea.l	Test_map,a0
	moveq.l	#0,d1
	moveq.l	#5-1,d7
.Loop_Y:	move.w	d7,d0
	and.w	#$0001,d0
	addq.w	#1,d0
	moveq.l	#8-1,d6
.Loop_X:	move.w	d1,d2
	mulu.w	#30,d2
	add.w	d0,d2
	mulu.w	#3,d2
	lea.l	0(a0,d2.w),a1
	move.b	#5,2(a1)
	move.b	#6,92(a1)
	move.b	#4,182(a1)
	move.b	#3,272(a1)
	addq.w	#3,d0
	dbra	d6,.Loop_X
	addq.w	#2,d1
	dbra	d7,.Loop_Y

	jsr	Read_2D_map

	move.l	#Update_buffer1-4,Update_buffer1-4
	move.l	#Update_buffer2-4,Update_buffer2-4

	lea.l	Update_buffer1,a0
	jsr	Update_all
	lea.l	Update_buffer2,a0
	jsr	Update_all
	jsr	Zong
	jsr	Switch_screens
	jsr	Zong
	jsr	Switch_screens

.Again:	jsr	Zong
	jsr	Switch_screens

.Wait:	jsr	Read_Mev
	btst	#Left_double,d2
	bne	.Exit
	jsr	Read_key
	tst.l	d0
	beq.s	.Again
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
	cmp.w	#Mapbuf_height+2-1,Party_Y
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
	mulu.w	#Mapbuf_depth*4,d2
	tst.w	2(a0,d2.w)
	bne.s	.Skip
	tst.w	Mapbuf_depth*4+2(a0,d2.w)
	bne.s	.Skip
	move.w	d0,Party_X
	move.w	d1,Party_Y
.Skip:	rts

;**************************************************
Zong:
	lea.l	Mannetje_object,a0
	move.w	Party_X,d0
	move.w	Party_Y,d1
	lsl.w	#4,d0
	lsl.w	#4,d1
	add.w	#15,d1
	move.w	d0,Object2D_X(a0)
	move.w	d1,Object2D_Y(a0)
	lea.l	Mannetjes,a1
	move.w	View_direction,d0
	mulu.w	#6*256,d0
	add.l	d0,a1
	move.l	a1,Object2D_gfx_offset(a0)

	jsr	Update_2D_map_display

	rts

;*****************************************************************************
; [ Read the 2D map ]
; No registers are restored
;*****************************************************************************
Read_2D_map:
	moveq.l	#0,d0
	moveq.l	#2,d1
	lea.l	Test_map,a0

	movem.l	d0/d1/a0,-(sp)		; Clear map buffers
	move.l	#((Mapbuf_height+3)*Mapbuf_width)*Mapbuf_depth*4,d0
	moveq.l	#0,d1
	lea.l	Map_buffers,a0
	jsr	Fill_memory
	movem.l	(sp)+,d0/d1/a0
	mulu.w	Width_of_map,d1		; Get map address
	add.w	d1,d0
	move.w	d0,d1
	add.w	d1,d1
	add.w	d1,d0
	add.w	d0,a0
	lea.l	Map_buffers,a1		; Copy map part to buffer
	lea.l	Icon_data,a2
	move.w	Width_of_map,d5
	sub.w	#Mapbuf_width,d5
	move.w	d5,d1
	add.w	d5,d5
	add.w	d1,d5
	moveq.l	#Mapbuf_height-1,d7
.Loop_Y:	move.l	a0,d0			; Odd or even ?
	btst	#0,d0
	bne	.Odd
; ---------- EVEN loop ----------------------------
	moveq.l	#Mapbuf_width/2-1,d6
.Loop_X1:	move.w	(a0)+,d0			; Get underlay
	move.b	d0,d1
	and.w	#%1111111111110000,d0
	lsr.w	#4,d0
	move.l	-6(a2,d0.w*8),d2		; Get underlay level
	lsr.l	#Icon_height,d2
	and.w	#$0003,d2
	move.w	d2,d4			; Store underlay
	mulu.w	#Mapbuf_width*Mapbuf_depth*4+4,d4
	move.w	d0,0(a1,d4.w)
	lsl.w	#8,d1			; Get overlay
	move.b	(a0)+,d1
	and.w	#%0000111111111111,d1
	beq.s	.Next_X1A			; If any
	move.l	-6(a2,d1.w*8),d3		; Get overlay level
	lsr.l	#Icon_height,d3
	and.w	#$0003,d3
	cmp.w	d2,d3			; Underlay over overlay ?
	bpl.s	.Ok1A
	move.w	d2,d3			; Yes -> Bump
.Ok1A:	move.w	d3,d4			; Store overlay
	mulu.w	#Mapbuf_width*Mapbuf_depth*4+4,d4
	move.w	d1,2(a1,d4.w)
.Next_X1A:	lea.l	Mapbuf_depth*4(a1),a1	; Next
	move.b	(a0)+,d0			; Get underlay
	lsl.w	#8,d0
	move.b	(a0),d0
	and.w	#%1111111111110000,d0
	lsr.w	#4,d0
	move.l	-6(a2,d0.w*8),d2		; Get underlay level
	lsr.l	#Icon_height,d2
	and.w	#$0003,d2
	move.w	d2,d4			; Store underlay
	mulu.w	#Mapbuf_width*Mapbuf_depth*4+4,d4
	move.w	d0,0(a1,d4.w)
	move.w	(a0)+,d1			; Get overlay
	and.w	#%0000111111111111,d1
	beq.s	.Next_X1B			; If any
	move.l	-6(a2,d1.w*8),d3		; Get overlay level
	lsr.l	#Icon_height,d3
	and.w	#$0003,d3
	cmp.w	d2,d3			; Underlay over overlay ?
	bpl.s	.Ok1B
	move.w	d2,d3			; Yes -> Bump
.Ok1B:	move.w	d3,d4			; Store overlay
	mulu.w	#Mapbuf_width*Mapbuf_depth*4+4,d4
	move.w	d1,2(a1,d4.w)
.Next_X1B:	lea.l	Mapbuf_depth*4(a1),a1	; Next
	dbra	d6,.Loop_X1
	add.w	d5,a0
	dbra	d7,.Loop_Y
	bra	.Done
; ---------- ODD loop -----------------------------
.Odd:	moveq.l	#Mapbuf_width/2-1,d6
.Loop_X2:	move.b	(a0)+,d0			; Get underlay
	lsl.w	#8,d0
	move.b	(a0),d0
	and.w	#%1111111111110000,d0
	lsr.w	#4,d0
	move.l	-6(a2,d0.w*8),d2		; Get underlay level
	lsr.l	#Icon_height,d2
	and.w	#$0003,d2
	move.w	d2,d4			; Store underlay
	mulu.w	#Mapbuf_width*Mapbuf_depth*4+4,d4
	move.w	d0,0(a1,d4.w)
	move.w	(a0)+,d1			; Get overlay
	and.w	#%0000111111111111,d1
	beq.s	.Next_X2A			; If any
	move.l	-6(a2,d1.w*8),d3		; Get overlay level
	lsr.l	#Icon_height,d3
	and.w	#$0003,d3
	cmp.w	d2,d3			; Underlay over overlay ?
	bpl.s	.Ok2A
	move.w	d2,d3			; Yes -> Bump
.Ok2A:	move.w	d1,2(a1,d3.w)		; Store overlay
.Next_X2A:	lea.l	Mapbuf_depth*2(a1),a1	; Next
	move.w	(a0)+,d0			; Get underlay
	move.b	d0,d1
	and.w	#%1111111111110000,d0
	lsr.w	#4,d0
	move.l	-6(a2,d0.w*8),d2		; Get underlay level
	lsr.l	#Icon_height,d2
	and.w	#$0003,d2
	move.w	d2,d4			; Store underlay
	mulu.w	#Mapbuf_width*Mapbuf_depth*4+4,d4
	move.w	d0,0(a1,d4.w)
	lsl.w	#8,d1			; Get overlay
	move.b	(a0)+,d1
	and.w	#%0000111111111111,d1
	beq.s	.Next_X2B			; If any
	move.l	-6(a2,d1.w*8),d3		; Get overlay level
	lsr.l	#Icon_height,d3
	and.w	#$0003,d3
	cmp.w	d2,d3			; Underlay over overlay ?
	bpl.s	.Ok2B
	move.w	d2,d3			; Yes -> Bump
.Ok2B:	move.w	d3,d4			; Store overlay
	mulu.w	#Mapbuf_width*Mapbuf_depth*4+4,d4
	move.w	d1,2(a1,d4.w)
.Next_X2B:	lea.l	Mapbuf_depth*2(a1),a1	; Next
	dbra	d6,.Loop_X2
	add.w	d5,a0
	dbra	d7,.Loop_Y
.Done:
	rts

;*****************************************************************************
; [ Update the 2D map display ]
; No registers are restored
;*****************************************************************************
Update_2D_map_display:
	move.l	VBL_counter,Timer_start

	ifne	FALSE
	move.w	#Map2D_X,d0
	move.w	#Map2D_Y,d1
	moveq.l	#-16,d4
	moveq.l	#Screen_depth,d5
	moveq.l	#Mapbuf_width,d6
	moveq.l	#Mapbuf_height,d7
	lsl.w	#4,d7
	jsr	VScroll_block
	endc

	tst.b	Toggle			; Toggle update buffers
	beq.s	.Two
	lea.l	Update_buffer1,a0
	bra.s	.Go_on
.Two:	lea.l	Update_buffer2,a0
.Go_on:	move.l	a0,Update_ptr
	not.b	Toggle
	LOCAL
; ---------- Prepare 2D objects -------------------
	jsr	Set_erase_rectangles	; Erase previous objects
	lea.l	Object2D_list,a0		; Update current objects
	move.w	Nr_2D_objects,d5
	bra.s	.Entry
.Loop:	move.l	(a0)+,a1			; Get object data
	move.w	Object2D_X(a1),d0		; Get coordinates
	moveq.l	#0,d1
	move.b	Object2D_level(a1),d1
	lsl.w	#4,d1
	neg.w	d1
	add.w	Object2D_Y(a1),d1
	move.w	Object2D_width(a1),d6	; Set update rectangle
	move.w	Object2D_height(a1),d7
	jsr	Set_update_rectangle
	move.w	Object2D_Y(a1),d1		; Calculate display index
	asr.w	#4,d0
	asr.w	#4,d1
	subq.w	#1,d0
	add.w	d6,d0
	mulu.w	#Mapbuf_width,d1
	add.w	d1,d0
	move.w	d0,Object2D_index(a1)
.Entry:	dbra	d5,.Loop
	move.w	Nr_2D_objects,d7		; Sort 2D objects
	lea.l	Compare_2D_objects,a0
	lea.l	Swap_2D_objects,a1
	jsr	Shellsort
	LOCAL
; ---------- Update 2D display --------------------
	Push	CA,Map2D_CA		; Begin
	jsr	Begin_graphics_ops
	lea.l	Map_buffers,a1
	lea.l	Icon_data,a2
	move.l	Update_ptr,a3
	lea.l	Object2D_list,a4
	clr.w	Display_index_2D
	move.w	Nr_2D_objects,Remaining_2D_objects
	moveq.l	#Map2D_Y,d2
	moveq.l	#Mapbuf_height+3-1,d7
.Loop_Y:	moveq.l	#Map2D_X,d0
	moveq.l	#Mapbuf_width-1,d6
.Loop_X:	move.b	(a3),d4			; Update ?
	beq	.Next_X
	clr.b	(a3)			; Yes
	move.l	a1,-(sp)
	move.w	d2,d1			; Do all layers
	moveq.l	#0,d5
.Loop_Z:	btst	d5,d4			; Update this layer ?
	beq.s	.Next_Z
	moveq.l	#0,d3			; Underlay ?
	move.w	(a1),d3
	beq.s	.No_U
	move.w	First_frame-6(a2,d3.w*8),d3	; Yes -> Display
	lea.l	Icons,a0
	lsl.l	#8,d3
	add.l	d3,a0
	jsr	Put_unmasked_icon
.No_U:	moveq.l	#0,d3			; Overlay ?
	move.w	2(a1),d3
	beq.s	.Next_Z
	move.w	First_frame-6(a2,d3.w*8),d3	; Yes -> Display
	lea.l	Icons,a0
	lsl.l	#8,d3
	add.l	d3,a0
	jsr	Put_masked_icon
.Next_Z:	addq.l	#4,a1			; Next layer
	sub.w	#16,d1
	addq.w	#1,d5
	moveq.l	#Mapbuf_height+3+2,d3	; First lines ?
	sub.w	d7,d3
	cmp.w	#Mapbuf_depth,d3
	bpl.s	.Not_1st
	cmp.w	d4,d5			; Yes -> Less layers
	bmi	.Loop_Z
	bra.s	.Done
.Not_1st:	cmp.w	#Mapbuf_depth,d5		; No -> All layers
	bmi	.Loop_Z
.Done:	move.l	(sp)+,a1
.Next_X:	tst.w	Remaining_2D_objects	; Any objects left ?
	beq.s	.No_obj
	move.l	a1,-(sp)
	move.w	Display_index_2D,d3		; Yes
.Again:	move.l	(a4),a1
	cmp.w	Object2D_index(a1),d3	; Draw now ?
	bmi.s	.No
	jsr	Draw_2D_object		; Yes -> Draw
	addq.l	#4,a4
	subq.w	#1,Remaining_2D_objects	; Count down
	bne.s	.Again
.No:	move.l	(sp)+,a1
.No_obj:	lea.l	Mapbuf_depth*4(a1),a1	; Next X
	addq.l	#1,a3
	add.w	#16,d0
	addq.w	#1,Display_index_2D
	dbra	d6,.Loop_X
	add.w	#16,d2			; Next Y
	dbra	d7,.Loop_Y
	jsr	End_graphics_ops		; End
	Pop	CA

	move.l	VBL_counter,d0
	sub.l	Timer_start,d0
	addq.w	#1,d0
	lea.l	Number,a0
	moveq.l	#"0",d6
	moveq.l	#3,d7
	jsr	DecR_convert
	moveq.l	#0,d0
	moveq.l	#0,d1
	moveq.l	#15,d2
	moveq.l	#9,d3
	moveq.l	#0,d4
	jsr	Draw_box
	lea.l	Number,a0
	moveq.l	#0,d0
	moveq.l	#0,d1
	jsr	Put_text_line
	rts

;*****************************************************************************
; [ Draw a 2D object ]
;   IN : a1 - Pointer to 2D object data (.l)
; All registers are	restored
;*****************************************************************************
Draw_2D_object:
	movem.l	d0/d1/d5-d7/a0,-(sp)
	moveq.l	#Screen_depth,d5
	move.w	Object2D_width(a1),d6
	move.w	Object2D_height(a1),d7
	move.w	Object2D_X(a1),d0
	moveq.l	#0,d1
	move.b	Object2D_level(a1),d1
	lsl.w	#4,d1
	neg.w	d1
	add.w	Object2D_Y(a1),d1
	add.w	#Map2D_X,d0
	add.w	#Map2D_Y+1,d1
	sub.w	d7,d1
	Get	Object2D_gfx_handle(a1),a0
	add.l	Object2D_gfx_offset(a1),a0
	jsr	Put_masked_block
	Free	Object2D_gfx_handle(a1)
	movem.l	(sp)+,d0/d1/d5-d7/a0
	rts

;*****************************************************************************
; [ Set the entire update map ]
;   IN : a0 - Pointer to update map (.l)
; All registers are	restored
;*****************************************************************************
Update_all:
	movem.l	d7/a0,-(sp)
	move.w	#Mapbuf_size-1,d7
.Loop:	or.b	#2,Mapbuf_width(a0)
	or.b	#4,Mapbuf_width*2(a0)
	or.b	#8,Mapbuf_width*3(a0)
	or.b	#1,(a0)+
	dbra	d7,.Loop
	movem.l	(sp)+,d7/a0
	rts

;*****************************************************************************
; [ Set update rectangle ]
;   IN : d0 - Left X-coordinate (.w)
;        d1 - Bottom Y-coordinate (.w)
;        d6 - Width in truncs (.w)
;        d7 - Height in pixels (.w)
; All registers are	restored
;*****************************************************************************
Set_update_rectangle:
	movem.l	d0-d2/d5-d7/a0/a1,-(sp)
	lsl.w	#4,d6			; Calculate X2
	add.w	d0,d6
	subq.w	#1,d6
	asr.w	#4,d0
	add.w	#15,d6			; Calculate width
	lsr.w	#4,d6
	sub.w	d0,d6
	move.w	d1,d2			; Calculate Y2
	sub.w	d7,d1
	addq.w	#1,d1
	asr.w	#4,d1
	move.w	d2,d7			; Calculate height
	add.w	#15,d7
	lsr.w	#4,d7
	sub.w	d1,d7
	tst.w	d0			; Clip left
	bpl.s	.Left_OK
	add.w	d0,d6
	moveq.l	#0,d0
.Left_OK:	move.w	d0,d2			; Clip right
	add.w	d6,d2
	cmp.w	#Mapbuf_width+1,d2
	bmi.s	.Right_OK
	sub.w	#Mapbuf_width,d2
	sub.w	d2,d6
.Right_OK:	tst.w	d1			; Clip top
	bpl.s	.Top_OK
	add.w	d1,d7
	moveq.l	#0,d1
.Top_OK:	move.w	d1,d2			; Clip bottom
	add.w	d7,d2
	cmp.w	#Mapbuf_height+1,d2
	bmi.s	.Bott_OK
	sub.w	#Mapbuf_height,d2
	sub.w	d2,d7
.Bott_OK:	tst.w	d6			; Anything left ?
	beq	.Exit
	tst.w	d7
	beq	.Exit
	move.l	Update_ptr,a0		; Yes -> Save for next frame
	move.l	-4(a0),a1
	move.b	d7,-(a1)
	move.b	d6,-(a1)
	move.b	d1,-(a1)
	move.b	d0,-(a1)
	move.l	a1,-4(a0)
	move.l	Update_ptr,a0		; Calculate buffer address
	mulu.w	#Mapbuf_width,d1
	add.w	d0,d1
	add.w	d1,a0
	move.w	#Mapbuf_width,d2		; Set rectangle
	sub.w	d6,d2
	subq.w	#1,d6
	subq.w	#1,d7
.Loop_Y:	move.w	d6,d5
.Loop_X:	or.b	#2,Mapbuf_width(a0)
	or.b	#4,Mapbuf_width*2(a0)
	or.b	#8,Mapbuf_width*3(a0)
	or.b	#1,(a0)+
	dbra	d5,.Loop_X
	add.w	d2,a0
	dbra	d7,.Loop_Y
.Exit:	movem.l	(sp)+,d0-d2/d5-d7/a0/a1
	rts

;*****************************************************************************
; [ Set update rectangles from previous frame ]
; All registers are restored
;*****************************************************************************
Set_erase_rectangles:
	movem.l	d0-d2/d5-d7/a0/a2/a3,-(sp)
	move.l	Update_ptr,a2
	subq.l	#4,a2
	move.l	(a2),a3
	bra.s	.Entry
.Again:	moveq.l	#0,d0
	move.b	(a3)+,d0
	moveq.l	#0,d1
	move.b	(a3)+,d1
	moveq.l	#0,d6
	move.b	(a3)+,d6
	moveq.l	#0,d7
	move.b	(a3)+,d7
	move.l	Update_ptr,a0		; Calculate buffer address
	mulu.w	#Mapbuf_width,d1
	add.w	d1,d0
	add.w	d0,a0
	move.w	#Mapbuf_width,d2		; Set rectangle
	sub.w	d6,d2
	subq.w	#1,d6
	subq.w	#1,d7
.Loop_Y:	move.w	d6,d5
.Loop_X:	or.b	#2,Mapbuf_width(a0)
	or.b	#4,Mapbuf_width*2(a0)
	or.b	#8,Mapbuf_width*3(a0)
	or.b	#1,(a0)+
	dbra	d5,.Loop_X
	add.w	d2,a0
	dbra	d7,.Loop_Y
.Entry:	cmp.l	a2,a3
	bne.s	.Again
	move.l	a2,(a2)
	movem.l	(sp)+,d0-d2/d5-d7/a0/a2/a3
	rts

;*****************************************************************************
; [ Compare two 2D objects ]
;   IN : d5 - Source index {...} (.w)
;        d6 - Destination index {1...} (.w)
;  OUT : eq - Source  = Destination
;        gs - Source >= Destination
;        ls - Source <= Destination
; All registers are restored
;*****************************************************************************
Compare_2D_objects:
	movem.l	d0/a0/a1,-(sp)
	lea.l	Object2D_list,a0
	move.l	-4(a0,d6.w*4),a1
	move.l	-4(a0,d5.w*4),a0
	move.w	Object2D_index(a0),d0
	cmp.w	Object2D_index(a1),d0
	bne.s	.Exit
	move.w	Object2D_Y(a0),d0
	cmp.w	Object2D_Y(a1),d0
	bne.s	.Exit
	move.w	Object2D_X(a0),d0
	cmp.w	Object2D_X(a1),d0
	bne.s	.Exit
	move.b	Object2D_level(a0),d0
	cmp.b	Object2D_level(a1),d0
.Exit:	movem.l	(sp)+,d0/a0/a1
	rts

;*****************************************************************************
; [ Swap two words ]
;   IN : d5 - Source index {1...} (.w)
;        d6 - Destination index {1...} (.w)
; All registers are restored
;*****************************************************************************
Swap_2D_objects:
	movem.l	d0/a0,-(sp)
	lea.l	Object2D_list,a0
	move.l	-4(a0,d5.w*4),d0
	move.l	-4(a0,d6.w*4),-4(a0,d5.w*4)
	move.l	d0,-4(a0,d6.w*4)
	movem.l	(sp)+,d0/a0
	rts


;	SECTION	Chip_DATA,data_c
;Test_block::
;	incbin	DINO.BLK


	SECTION	Fast_DATA,data
Party_X:	dc.w 10
Party_Y:	dc.w 5
View_direction:	dc.w South

Map2D_CA:	dc.w Map2D_X,Map2D_X+Mapbuf_width*16-1
	dc.w Map2D_Y,Map2D_Y+Mapbuf_height*16-1

Mannetje_object:
	dc.w 0
	dc.w 0,0
	dc.w 2,48
	dc.b 0,0
	dc.l 0
Object_twee:
	dc.w 0
	dc.w 48,128+32+14
	dc.w 2,48
	dc.b 0,0
	dc.l Mannetjes+12*256
Object_drie:
	dc.w 0
	dc.w 80,128+14
	dc.w 2,48
	dc.b 0,0
	dc.l Mannetjes+12*256
Object_vier:
	dc.w 0
	dc.w 128,128+16+14
	dc.w 2,48
	dc.b 0,0
	dc.l Mannetjes+12*256
Object_vijf:
	dc.w 0
	dc.w 176,128+14
	dc.w 2,48
	dc.b 0,0
	dc.l Mannetjes+12*256

Icon_data:
	dc.w 0
;	      33222222222211111111110000000000
;	      10987654321098765432109876543210
	dc.l %00000000000000000000000000000000
	dc.w 0
	dc.b 1,0
	dc.l %00000000000000000000000000000000
	dc.w 1
	dc.b 1,0
	dc.l %00000000000000000000000000000100
	dc.w 4
	dc.b 1,0
	dc.l %00000000000000000000000000100100
	dc.w 3
	dc.b 1,0
	dc.l %00000000000000000000000001100100
	dc.w 2
	dc.b 1,0
	dc.l %00000000000000000000000001000100
	dc.w 3
	dc.b 1,0

Width_of_map:	dc.w 30
Height_of_map:	dc.w 40

Test_map:	dcb.b 30*40*3,0

;Yo_ho_HDOB:
;	dc.w 0,0
;	dc.w 0,0,0
;	dc.b 0,0
;	dc.l Test_block+6
;	dc.b %00001000
;	dc.b 0
;	dcb.w 6,0



	SECTION	Fast_BSS,bss
Toggle:	ds.b 1
Number:	ds.b 11
	even
Timer_start:	ds.l 1
Display_index_2D:	ds.w 1
Nr_2D_objects:	ds.w 1
Remaining_2D_objects:	ds.w 1

Update_ptr:	ds.l 1
Object2D_list:	ds.l Max_2D_objects

	ds.l Max_2D_objects
	ds.l 1
Update_buffer1:	ds.b (Mapbuf_height+3)*Mapbuf_width
	ds.l Max_2D_objects
	ds.l 1
Update_buffer2:	ds.b (Mapbuf_height+3)*Mapbuf_width
Map_buffers:
	ds.l ((Mapbuf_height+3)*Mapbuf_width)*Mapbuf_depth
