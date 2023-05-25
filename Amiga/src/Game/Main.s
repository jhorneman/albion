
	SECTION	Program,code

	lea.l	Party_data_start,a0		; Clear party data
	move.l	#Party_data_size,d0
	moveq.l	#0,d1
	jsr	Fill_memory
	clr.w	Nr_of_modifications		; Clear variable data
	lea.l	Initial_date,a0		; Set initial time
	lea.l	Year,a1
	moveq.l	#5-1,d7
.Loop:	move.w	(a0)+,(a1)+
	dbra	d7,.Loop
	move.w	#1,Active_member		; Insert first party member
	move.w	#1,Member_nrs
	move.b	#18,Order
; Execute Game Starts event chain

;*****************************************************************************
; [ Initialize program ]
; No registers are restored
; NOTE :
;  - Do not call this routine directly, but call [ Restart_game ] instead.
;*****************************************************************************
Init_program:
; ---------- Clear game data ----------------------
	lea.l	Game_data_start,a0
	move.l	#Game_data_size,d0
	moveq.l	#0,d1
	jsr	Fill_memory

	ifne	FALSE
; ---------- Load party characters' data ----------
	lea.l	Batch,a0			; Build batch
	lea.l	Member_nrs,a1
	moveq.l	#0,d0
	moveq.l	#6-1,d7
.Loop:	move.w	(a1)+,d1
	beq.s	.Next
	move.w	d1,(a0)+
	addq.w	#1,d0
.Next:	dbra	d7,.Loop
	move.w	d0,Nr_members		; Correction
	lea.l	Batch,a0			; Load batch
	lea.l	Party_handles,a1
	moveq.l	#Party_char_file,d1
	jsr	Load_batch_of_subfiles
	LOCAL
	endc

; ---------- Load party member portraits ----------
	lea.l	Batch,a2
	moveq.l	#6-1,d7
.Loop:
;	moveq.l	#0,d0			; Default
;	move.b	(a1)+,d1			; Anyone there ?
;	beq.s	.Next
;	Get	d1,a0			; Get portrait number
;	move.b	Portrait_nr(a0),d0
;	Free	d1

	moveq.l	#6,d0
	sub.w	d7,d0

.Next:	move.w	d0,(a2)+			; Store
	dbra	d7,.Loop			; Next party member
	lea.l	Batch,a0
	lea.l	Portrait_handles,a1
	moveq.l	#6,d0
	moveq.l	#Small_portraits_file,d1
	jsr	Load_batch_of_subfiles
	LOCAL

	lea.l	Black_colour,a0
	moveq.l	#0,d0
	moveq.l	#1,d1
	jsr	Set_palette_part

	move.w	#$c0,d0
	move.w	#$c0,d1
	move.w	#$c0,d2
	lea.l	Main_palette,a0
	move.b	d0,(a0)
	move.b	d1,1(a0)
	move.b	d2,2(a0)
	move.w	d0,Target_R
	move.w	d1,Target_G
	move.w	d2,Target_B

	move.w	#192,d0
	moveq.l	#64,d1
	jsr	Set_palette_part

	lea.l	Earth_class,a0
	lea.l	Main_screen_OID,a1
	moveq.l	#0,d0
	jsr	Add_object
	move.w	d0,Main_screen_handle

	lea.l	Earth_class,a0
	lea.l	Permanent_area_OID,a1
	moveq.l	#0,d0
	jsr	Add_object
	move.w	d0,Permanent_area_handle

	moveq.l	#0,d0
	moveq.l	#0,d1
	move.w	#Screen_width-1,d2
	move.w	#Screen_height-49,d3
	jsr	Marmor_box

	moveq.l	#0,d0
	move.w	#Screen_height-48,d1
	move.w	#160,d2
	move.w	#48,d3
	jsr	Draw_high_border

	lea.l	Help_line_class,a0
	lea.l	Help_line_OID,a1
	move.w	Permanent_area_handle,d0
	jsr	Add_object
	move.w	d0,Help_line_handle

	lea.l	Permanent_text_class,a0
	lea.l	Permanent_text_OID,a1
	move.w	Permanent_area_handle,d0
	jsr	Add_object
	move.w	d0,Permanent_text_handle
	moveq.l	#Draw_method,d1
	jsr	Execute_method

	lea.l	Portrait_handles,a1
	moveq.l	#2,d0
	move.w	#Screen_height-38,d1
	move.w	#192,d4
	moveq.l	#6,d5
	moveq.l	#2,d6
	moveq.l	#37,d7
	moveq.l	#6-1,d3
.Loop:	Get	(a1),a0
	jsr	Put_masked_block
	Free	(a1)+
	add.w	#25,d0
	dbra	d3,.Loop

	moveq.l	#Map3D_X-1,d0
	moveq.l	#Map3D_Y-1,d1
	move.w	#Map3D_width+2,d2
	move.w	#Map3D_height+2,d3
	jsr	Draw_deep_border

	addq.w	#1,d0
	addq.w	#1,d1
	subq.w	#2,d2
	subq.w	#2,d3
	jsr	Draw_high_border

	moveq.l	#0,d0
	moveq.l	#0,d1
	move.w	#Screen_width/8,d2
	move.w	#(Screen_height-48)/8,d3
	moveq.l	#35,d4
	jsr	Random_wipe

	jsr	Update_screen

	lea.l	FL_text,a1
	move.w	Help_line_handle,d0
	moveq.l	#Print_method,d1
	jsr	Execute_method

	lea.l	PTW_text,a1
	move.w	Permanent_text_handle,d0
	moveq.l	#Print_method,d1
	jsr	Execute_method

	lea.l	M3_window_class,a0
	lea.l	M3_window_OID,a1
	move.w	Main_screen_handle,d0
	jsr	Add_object

	move.w	#1,Map_nr
	move.w	#23,Map_Xcoord
	move.w	#17,Map_Ycoord
	move.w	#North,View_direction
	jsr	Init_map

	LOCAL
	lea.l	Current_palette,a0
	lea.l	Darker_table,a1
	move.l	a0,a2
	move.w	#256,d7
	move.w	#256-1,d6
.Loop:	moveq.l	#0,d0
	move.b	(a2)+,d0
	moveq.l	#0,d1
	move.b	(a2)+,d1
	moveq.l	#0,d2
	move.b	(a2)+,d2
	mulu.w	#55,d0
	mulu.w	#55,d1
	mulu.w	#55,d2
	divu.w	#100,d0
	divu.w	#100,d1
	divu.w	#100,d2
	jsr	Find_closest_colour
	move.b	d0,(a1)+
	dbra	d6,.Loop

	jsr	Init_display

	rts

FL_text:	dc.b "Permanent feedback",0
PTW_text:	dc.b "This is the permanent text window.",0
	even

Black_colour:	dc.w 0

M3_draw:
	movem.l	d0-d3,-(sp)
	moveq.l	#Map3D_X-1,d0
	moveq.l	#Map3D_Y-1,d1
	move.w	#Map3D_width+2,d2
	move.w	#Map3D_height+2,d3
	jsr	Draw_deep_border
;	move.w	Object_self(a0),d0
;	moveq.l	#Update_method,d1
;	jsr	Execute_method
	movem.l	(sp)+,d0-d3
	rts

M3_focus:
	movem.l	d0-d4,-(sp)
	moveq.l	#Map3D_X-1,d0
	moveq.l	#Map3D_Y-1,d1
	move.w	#Map3D_width+2,d2
	move.w	#Map3D_height+2,d3
	move.w	#Focus_colour,d4
	jsr	Draw_rectangle
	movem.l	(sp)+,d0-d4
	rts

M3_DisInit:
	jsr	M3_DisUpd

	moveq.l	#Map3D_X,d0
	moveq.l	#Map3D_Y,d1
	move.w	#Map3D_width/8,d2
	move.w	#Map3D_height/8+1,d3
	moveq.l	#35,d4
	jsr	Random_wipe

	rts

Triple_X:	lea.l	.Kev,a1
	rts

.Kev:	dc.l $00ff0000,$004c0000,Shade_up
	dc.l $00ff0000,$004d0000,Shade_down
	dc.l $000000ff," ",Leave
	dc.l $000000ff,"1",Test_window
	dc.l $000000ff,"2",Calc_3D_memory
	dc.l 0

	XREF	Text_window_class

Test_window:
	rts

	lea.l	Text_window_class,a0
	lea.l	.OID,a1
	moveq.l	#0,d0
	jsr	Add_object
	move.w	d0,d7
	moveq.l	#Draw_method,d1
	jsr	Execute_method
	jsr	Update_screen
	lea.l	.Text,a1
	move.w	d7,d0
	moveq.l	#Print_method,d1
	jsr	Execute_method
	jsr	Update_screen
	jsr	Wait_4_click
	move.w	d7,d0
	jsr	Delete_object
	jsr	Update_screen
	rts

.OID:	dc.w 16,32,300,112
	dc.w Brightest,Marmor,-1
.Text:	dc.b "One of the improvements made to the Amiga's operating system for "
	dc.b "Workbench Release 2.1 is programmatic control of AGFA's IntelliFont "
	dc.b "scaling engine. With this engine, application programs can fully "
	dc.b "utilize Compugraphic (CG) typefaces installed by Fountain (the CG "
	dc.b "typeface install that comes with 2.04 and 2.1). Some of the features "
	dc.b "that the font scaling engine offers include:",0
	even

Yahoo:
	movem.l	d0/d1/a0,-(sp)
	move.w	Mouse_X,d0
	move.w	Mouse_Y,d1
	lea.l	A_PUM,a0
	jsr	Do_PUM
	movem.l	(sp)+,d0/d1/a0
	rts

Leave:
	move.l	Off_screen,a0
	jsr	Clear_screen

	moveq.l	#0,d0
	moveq.l	#0,d1
	move.w	#Screen_width/8,d2
	move.w	#Screen_height/8,d3
	moveq.l	#35,d4
	jsr	Random_wipe

	jmp	Exit_program

M3_clicked:
	movem.l	d0/d1/a0,-(sp)

	Push	MA,M3_window_MA
	bra.s	.Entry
.Again:	jsr	M3_DisUpd
	jsr	Switch_screens
.Entry:	move.w	Mouse_X,d0
	move.w	Mouse_Y,d1
	sub.w	#Map3D_X,d0
	sub.w	#Map3D_Y,d1
	cmp.w	#Map3D_width,d0
	bhi	.Exit
	cmp.w	#Map3D_height,d1
	bhi	.Exit

	cmp.w	#80,d0
	bpl.s	.Not_L
	moveq.l	#0,d0
	bra.s	.Do_Y
.Not_L:	cmp.w	#256-80,d0
	bpl.s	.Not_M1
	moveq.l	#1,d0
	bra.s	.Do_Y
.Not_M1:	moveq.l	#2,d0

.Do_Y:	cmp.w	#40,d1
	bmi.s	.Done
	cmp.w	#134-40,d1
	bpl.s	.Not_M2
	addq.w	#3,d0
	bra.s	.Done
.Not_M2:	addq.w	#6,d0
.Done:
	lea.l	Move_ptrs,a0
	move.l	0(a0,d0.w*4),a0
	jsr	(a0)
.Exit:	btst	#Left_pressed,Button_state
	bne	.Again
	jsr	Copy_screen
	Pop	MA

	movem.l	(sp)+,d0/d1/a0
	rts

Move_ptrs:	dc.l Move0,Move1,Move2
	dc.l Hordown,Dummy,Horup
	dc.l Movedown,Move7,Moveup

Hordown:	move.w	Horizon_Y,d0
	sub.w	#4,d0
	cmp.w	#16,d0
	bmi.s	.Exit
	move.w	d0,Horizon_Y
.Exit:	rts

Horup:	move.w	Horizon_Y,d0
	add.w	#4,d0
	cmp.w	#Map3D_height-16,d0
	bpl.s	.Exit
	move.w	d0,Horizon_Y
.Exit:	rts

Movedown:	move.w	Camera_height,d0
	sub.w	#8,d0
	cmp.w	#4,d0
	bmi.s	.Exit
	move.w	d0,Camera_height
.Exit:	rts

Moveup:	move.w	Camera_height,d0
	add.w	#8,d0
	cmp.w	#Square_size-4,d0
	bpl.s	.Exit
	move.w	d0,Camera_height
.Exit:	rts

Move0:	move.w	Dungeon_speed,d0		; Increase angle by speed
	ext.l	d0
	moveq.l	#13+1,d1
	lsl.l	d1,d0
	add.l	d0,Y_angle
	rts

Move2:	move.w	Dungeon_speed,d0		; Increase angle by speed
	ext.l	d0
	moveq.l	#13+1,d1
	lsl.l	d1,d0
	sub.l	d0,Y_angle
	rts

Move1:	jsr	Get_3D_direction		; Get vector
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
	jmp	Detect

Move7:	jsr	Get_3D_direction		; Get vector
	asr.l	#1,d0
	asr.l	#1,d1
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
	jmp	Detect

Detect:	move.l	d2,d0
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

	ifne	FALSE
	movem.l	d0-d7/a0,-(sp)
	lea.l	Number,a0
	moveq.l	#"0",d6
	moveq.l	#3,d7
	jsr	DecR_convert
	move.b	#":",(a0)+
	move.w	d1,d0
	jsr	DecR_convert

	moveq.l	#0,d0
	move.w	#160,d1
	moveq.l	#39,d2
	move.w	#205,d3
	moveq.l	#0,d4
	jsr	Draw_box

	lea.l	Number,a0
	moveq.l	#0,d0
	move.w	#160,d1
	jsr	Put_text_line

	lea.l	Number,a0
	move.b	#"W",(a0)+
	move.b	#":",(a0)+
	move.w	Visible_walls,d0
	jsr	DecL_convert
	lea.l	Number,a0
	moveq.l	#0,d0
	move.w	#169,d1
	jsr	Put_text_line

	lea.l	Number,a0
	move.b	#"O",(a0)+
	move.b	#":",(a0)+
	move.w	Visible_objects,d0
	jsr	DecL_convert
	lea.l	Number,a0
	moveq.l	#0,d0
	move.w	#178,d1
	jsr	Put_text_line

	lea.l	Number,a0
	move.b	#"F",(a0)+
	move.b	#":",(a0)+
	move.w	Visible_floors,d0
	jsr	DecL_convert
	lea.l	Number,a0
	moveq.l	#0,d0
	move.w	#187,d1
	jsr	Put_text_line

	lea.l	Number,a0
	move.b	#"E",(a0)+
	move.b	#":",(a0)+
	move.w	Visible_elements,d0
	jsr	DecL_convert
	lea.l	Number,a0
	moveq.l	#0,d0
	move.w	#196,d1
	jsr	Put_text_line

	movem.l	(sp)+,d0-d7/a0
	endc

	move.l	d2,Player_X+2
	move.l	d3,Player_Y+2
	move.w	d4,Player_X
	move.w	d5,Player_Y
	rts

.Exit:	rts

Shade_up:
	move.w	shade_faktor,d0
	addq.w	#8,d0
	cmp.w	#256+1,d0
	bpl.s	.Skip
	move.w	d0,shade_faktor
	jsr	Init_tm_coords
.Skip:	rts

Shade_down:
	move.w	shade_faktor,d0
	subq.w	#8,d0
	bmi.s	.Skip
	move.w	d0,shade_faktor
	jsr	Init_tm_coords
.Skip:	rts

Calc_3D_memory:
	moveq.l	#0,d6
	lea.l	Object_handles,a1		; Check object memory
	move.w	Nr_of_objects,d7
	bra.s	.Entry1
.Loop1:	move.b	(a1)+,d0
	jsr	Get_memory_length
	add.l	d0,d6
.Entry1:	dbra	d7,.Loop1
	lea.l	Wall_handles,a1		; Check wall memory
	move.w	Nr_of_walls,d7
	bra.s	.Entry2
.Loop2:	move.b	(a1)+,d0
	jsr	Get_memory_length
	add.l	d0,d6
.Entry2:	dbra	d7,.Loop2
	lea.l	Floor_handles,a1		; Check floor memory
	move.w	Nr_of_floor_buffers,d7
	bra.s	.Entry3
.Loop3:	move.b	(a1)+,d0
	jsr	Get_memory_length
	add.l	d0,d6
.Entry3:	dbra	d7,.Loop3
;	move.b	Lab_background_handle,d0	; Check background memory
;	jsr	Get_memory_length
;	add.l	d0,d6
	move.b	Labdata_handle,d0		; Check labyrinth data
	jsr	Get_memory_length
	add.l	d0,d6
;	move.b	Maptext_handle,d0		; Check map texts
;	jsr	Get_memory_length
;	add.l	d0,d6
;	move.b	Dungeon_screen_handle,d0	; Check buffer
;	jsr	Get_memory_length
;	add.l	d0,d6
	move.w	#0,d0
	move.w	#160,d1
	move.l	d6,d2
	jsr	Print_large_number
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

;*****************************************************************************
Make_modifications:
	rts

;*****************************************************************************
Do_map_colour_cycling:
	lea.l	Shade_table,a0
	moveq.l	#64-1,d7
.Loop:	move.w	#64,d0
	moveq.l	#4,d1
	jsr	Cycle_forward
	move.w	#68,d0
	moveq.l	#12,d1
	jsr	Cycle_forward
	lea.l	256(a0),a0
	dbra	d7,.Loop
	rts

Cycle_forward:
	movem.l	d0/d1/a0,-(sp)
	add.w	d1,d0
	add.w	d0,a0
	move.l	a0,a1
	move.b	-(a0),d2
	subq.w	#2,d1
	bmi.s	.Exit
.Loop:	move.b	-(a0),-(a1)
	dbra	d1,.Loop
	move.b	d2,(a0)+
.Exit:	movem.l	(sp)+,d0/d1/a0
	rts

;***************************************************************************	
; [ Print method for help line objects ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to text (.l)
; All registers are restored
;***************************************************************************
Print_help_line:
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
	lea.l	Text_area_PA(a0),a0		; Print
	jsr	Push_PA
	move.l	a1,a0
	jsr	Put_centered_box_text_line
	Pop	PA
	jsr	Update_screen
	movem.l	(sp)+,d0-d3/a0/a1
	rts

Draw_permanent_text:
	movem.l	d0-d3,-(sp)
	move.w	X1(a0),d0			; Draw border
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#1,d2
	addq.w	#1,d3
	jsr	Draw_high_box
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************	
	SECTION	Fast_DATA,data
Help_line_class:
	dc.l 0
	dc.w Text_area_object_size
	Method Init,Init_text_area
	Method Print,Print_help_line
	Method Erase,Erase_text_area
	dc.w -1

Permanent_text_class:
	dc.l Text_area_class
	dc.w Text_area_object_size
	Method Draw,Draw_permanent_text
	dc.w -1

Main_screen_OID:	dc.w 0,0,Screen_width,Screen_height-49
Permanent_area_OID:	dc.w 0,Screen_height-48,Screen_width,48
Help_line_OID:
	dc.w 160,0
	dc.w Screen_width-160,12
	dc.w White,Marmor,-1
Permanent_text_OID:
	dc.w 160,12
	dc.w Screen_width-160,35
	dc.w White,Marmor,-1

A_PUM:
	dc.w 4
	dc.l .Title
	dc.l Dummy
	dc.w $0000
	dc.w 0
	dc.l .One
	dc.l Dummy
	dc.w $0000
	dc.w 0
	dc.l .Two
	dc.l Dummy
	dc.w $0000
	dc.w 0
	dc.l .Three
	dc.l Dummy
	dc.w $0000
	dc.w 0
	dc.l .Four
	dc.l Dummy

.Title:	dc.b "Yo-ho",0
.One:	dc.b "One",0
.Two:	dc.b "Two",0
.Three:	dc.b "Three",0
.Four:	dc.b "Four",0
	even

M3_window_class:
	dc.l 0
	dc.w Object_header_size
	Method Init,Set_rectangle
	Method Left,M3_clicked
	Method Right,Normal_rightclicked
	Method Touched,Normal_touched
	Method Focus,M3_focus
	Method Draw,M3_draw
	Method Update,M3_DisUpd
	Method Pop_up,Yahoo
	Method Customkey,Triple_X
	dc.w -1

M3_window_OID:
	dc.w Map3D_X,Map3D_Y,Map3D_width,Map3D_height
M3_window_MA:
	dc.w Map3D_X,Map3D_X+Map3D_width-1,Map3D_Y,Map3D_Y+Map3D_height-1


	SECTION	Fast_BSS,bss
; ********* G A M E   D A T A *********************
Game_data_start:
; ---------- General ------------------------------
Darker_table:	ds.b 256
Nr_members:	ds.w 1

; ---------- Object handles -----------------------
Main_screen_handle:	ds.w 1
Permanent_area_handle:	ds.w 1

Help_line_handle:	ds.w 1
Permanent_text_handle:	ds.w 1

; ---------- Memory handles -----------------------
Party_handles:	ds.b 6
Portrait_handles:	ds.b 6
Active_handle:	ds.b 1
Inventory_handle:	ds.b 1

; ---------- Flags --------------------------------
Number:	ds.b 11

; ---------- Map ----------------------------------
Current_map_type:	ds.b 1
_2D_or_3D_map:	ds.b 1
Anim_count:	ds.b 1
Stygian:	ds.b 1
Map_has_changed:	ds.b 1

Mapdata_handle:	ds.b 1
Maptext_handle:	ds.b 1
	even

Width_of_map:	ds.w 1
Height_of_map:	ds.w 1
Size_of_map:	ds.w 1

Nr_goto_points:	ds.w 1

Anim_update:	ds.w 1
Circle_anim:	ds.l 7*8			; Animation tables
Wave_anim:	ds.l 6*8

Map_palette:	ds.b 192*3
Backup_palette:	ds.b 192*3
	even

Event_entry_offset:	ds.l 1			; Offsets	to map data
Event_data_offset:	ds.l 1
NPC_path_base_offset:	ds.l 1
Goto_point_offset:	ds.l 1
Event_automap_offset:	ds.l 1

CD_value:	ds.l 1
VNPC_data:

; ---------- 2D map -------------------------------

; ---------- 3D map -------------------------------
Dungeon_screen_handle:	ds.b 1
Labdata_handle:	ds.b 1
Lab_background_handle:	ds.b 1
Floor_handles:	ds.b Max_3D_floors
Object_handles:	ds.b Max_3D_objects
Wall_handles:	ds.b Max_3D_walls
Wall_transparencies:	ds.b Max_3D_walls
Saved_map_byte:	ds.b 1
Masked_wall_flag:	ds.b 1
Transparent_wall_flag:	ds.b 1
Wall_ptrs_invalid:	ds.b 1
Floor_ptrs_invalid:	ds.b 1
Shade_table:	ds.b 64*256
Shade_function:	ds.b 16384/4
	even
Target_R:	ds.w 1
Target_G:	ds.w 1
Target_B:	ds.w 1

Nr_of_groups:	ds.w 1
Nr_of_floors:	ds.w 1
Nr_of_floor_buffers:	ds.w 1
Nr_of_objects:	ds.w 1
Nr_of_walls:	ds.w 1
Dungeon_speed:	ds.w 1
Wall_height_3D:	ds.w 1
FX_camera_height:	ds.w 1
Hash_number:	ds.w 1
Yoghurt_X:	ds.w 1
Yoghurt_Y:	ds.w 1
Labdata_ptr:	ds.l 1
Saved_map_ptr:	ds.l 1
Floor_data_offset:	ds.l 1
Object_data_offset:	ds.l 1
Wall_data_offsets:	ds.l Max_3D_walls
Wall_ptrs:	ds.l Max_3D_walls
Floor_ptrs:	ds.l Max_3D_floors*8
Dungeon_screen:	ds.l 1
Timer:	ds.l 1
Visible_walls:	ds.w 1
Visible_objects:	ds.w 1
Visible_floors:	ds.w 1
Visible_elements:	ds.w 1

; --- Dungeon stuff ---
tmc_kantentab:	ds.l 4*Map3D_height
prey_ktab:	ds.l Map3D_height*3
beo_y:		ds.w 1
boden_y1:	ds.w 1
boden_ysize:	ds.w 1
decke_y1:	ds.w 1
decke_ysize:	ds.w 1
view_y:		ds.w 1
viewy_dir:	ds.w 1

shade_faktor:	ds.w 1
zvals_stab:	ds.l Map3D_height*3		;ptr to lightshade

view_points:	ds.l 4
rot_points:	ds.l 32*32
proj_points:	ds.l 32*32*2
dminx:		ds.l 1
dminy:		ds.l 1
dmaxx:		ds.l 1
dmaxy:		ds.l 1
drawstartx:	ds.w 1
drawstarty:	ds.w 1
drawsizex:	ds.w 1
drawsizey:	ds.w 1
dsizex:	ds.w 1
dsizey:	ds.w 1
dsizex2:	ds.l 1
dsizey2:	ds.l 1
Player_X:	ds.w 3
Player_Y:	ds.w 3
Y_angle:	ds.l 1
Camera_height:	ds.w 1
Camera_height_old:	ds.w 1
Horizon_Y:		ds.w 1
Horizon_Y_old:	ds.w 1

xoffset:	ds.w 1
zoffset:	ds.w 1
lower_y:	ds.w 1
upper_y:	ds.w 1
scrx:		ds.w 1
cx1:		ds.w 1
cy1:		ds.w 1
cx2:		ds.w 1
cy2:		ds.w 1
color:		ds.w 1

Dungeon_sort_list:	ds.l Max_visible_elements*2
Dungeon_wall_list:	ds.w (Max_visible_walls+1)*9+(Max_visible_floors+1)*23
Dungeon_object_list:	ds.w Max_visible_objects*6
Dungeon_draw_list:	ds.w (Max_visible_walls+1)*2+Max_visible_objects*11
	ds.w (Max_visible_floors/2)*2
Dungeon_clipping_table:	ds.l Max_visible_walls+1

Game_data_size	EQU *-Game_data_start


; ********* P A R T Y   D A T A *******************
Party_data:
Year:	ds.w 1			; Current date & time
Month:	ds.w 1
Day:	ds.w 1
Hour:	ds.w 1
Minute:	ds.w 1
Map_nr:	ds.w 1			; Current position
Map_Xcoord:	ds.w 1		; (is set by Map Exit)
Map_Ycoord:	ds.w 1
View_direction:	ds.w 1
Spell_1_duration:	ds.w 1		; Light
Spell_1_data:	ds.w 1
Spell_2_duration:	ds.w 1		;
Spell_2_data:	ds.w 1
Spell_3_duration:	ds.w 1		;
Spell_3_data:	ds.w 1
Spell_4_duration:	ds.w 1		; Magical Attack
Spell_4_data:	ds.w 1
Spell_5_duration:	ds.w 1		; Magical Defense
Spell_5_data:	ds.w 1
Spell_6_duration:	ds.w 1		; Magical Defense agains
Spell_6_data:	ds.w 1		;  magic

Active_member:	ds.w 1
Member_nrs:	ds.w 6

Time_data_year:	ds.w 1		; Relative year
Travel_mode:	ds.w 1
Special_item_flags:	ds.w 1
Internal_flags:	ds.w 1
Camp_counter:	ds.w 1		; Hours since last camp
Trans_data:	ds.b Max_transports*Trans_data_size	; Transport data

Quest:	dsbit Max_quest			;      Quest solved boolean array
Event:	dsbit Max_event			;    Event executed boolean array
CD:	dsbit Max_CD			; Character removed boolean array
WD:	dsbit Max_words			;       Known words boolean array
Goto_points:	dsbit Max_Goto_points	; Known Goto-points boolean array
Chest_open:	dsbit Max_chests		;     Opened chests boolean array
Door_open:	dsbit Max_doors			;      Opened doors boolean array
Byte_counters:	ds.b Max_byte_counters	; Byte counters

Order:	ds.b 6			; Combat positions for each member

; Variable data follows in file
Party_data_size	EQU *-Party_data

