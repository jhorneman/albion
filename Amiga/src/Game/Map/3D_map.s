; 3D map management routines
; Written by J.Horneman (In Tune With The Universe)
; Start : 24-3-1994

	SECTION	Program,code
;*****************************************************************************
; [ 3D map ModInit ]
; No registers are restored
;*****************************************************************************
M3_ModInit:
	jsr	Load_3D_map_data		; Load stuff
	jsr	Load_map_palette
	move.w	Map_nr,d7			; Make map modifications
	jsr	Make_modifications
; ---------- Initialize 3D stuff ------------------
	jsr	Analyze_labdata
	moveq.l	#psize_log,d7		; Initialize dimensions
	moveq.l	#0,d0
	move.w	Width_of_map,d0
	move.w	d0,dsizex
	asl.l	d7,d0
	move.l	d0,dsizex2
	moveq.l	#0,d0
	move.w	Height_of_map,d0
	move.w	d0,dsizey
	asl.l	d7,d0
	move.l	d0,dsizey2
;	clr.w	FX_camera_height		; Set variables
;	clr.w	Current_sky_level
;	clr.w	Current_light_level
	move.w	#-1,Camera_height_old
	move.w	#Map3D_width/2,scrx
	move.w	#Map3D_height*6/10,Horizon_Y
	move.w	#6*32,Dungeon_speed
	move.w	#160,shade_faktor
	move.w	#165,Camera_height
	jsr	Calculate_shade_function	; Initialize stuff
	jsr	Calculate_shade_table
	jsr	Initialize_3D_position
	LOCAL

	ifne	FALSE
; ---------- Do city/dungeon specifics ------------
	cmp.b	#City_3D,Current_map_type
	beq.s	.City
	move.w	#D3_horizon,Horizon_Y		; Set horizon
	move.b	#D3_animspeed,Anim_speed	; Set animation parameters
	move.b	#D3_animspeed,Anim_count
	move.w	#D3_animbias,Anim_bias
	jsr	Update_light_status		; Update light
	bra	.Done
.City:	move.w	#C3_horizon,Horizon_Y		; Set horizon
	move.b	#C3_animspeed,Anim_speed	; Set animation parameters
	move.b	#C3_animspeed,Anim_count
	move.w	#C3_animbias,Anim_bias
	move.w	Hour,d0			; Get current hour
	move.w	d0,d2			; Save
	subq.w	#1,d0			; Back in time
	bpl.s	.Ok
	moveq.l	#23,d0
.Ok:	move.w	d0,Hour
	jsr	Evaluate_C3_light		; Get old light level
	move.w	New_sky_level,d0
	move.w	New_light_level,d1
	move.w	d2,Hour			; Restore time
	jsr	Evaluate_C3_light		; Get current light level
	cmp.w	New_sky_level,d0		; Fade sky ?
	bne.s	.Yes
	cmp.w	New_light_level,d1		; Fade light ?
	bne.s	.Yes
	jsr	Update_light_status		; No -> Update
	bra.s	.Done
.Yes:	move.w	d0,Current_sky_level	; Yes
	move.w	d1,Current_light_level
	move.w	Minute,d0			; Adjust fade
	divu.w	#Minutes_per_step,d0
	bra.s	.Entry
.Loop:	jsr	Update_M3_light_level
.Entry:	dbra	d0,.Loop
.Done:	LOCAL
	endc

; ---------- Create transparent wall data ---------
	lea.l	Wall_data_offsets,a0
	lea.l	Wall_transparencies,a1
	XGet	Labdata_handle,a2
	move.w	Nr_of_walls,d7
	bra	.Next
.Loop:	sf	(a1)			; Clear
	move.l	a2,a3			; Get wall data address
	add.l	(a0)+,a3
	move.l	Wall_bits(a3),d0
	btst	#Transparent_wall_bit,d0	; Transparent ?
	beq	.No
	move.l	#256,d0			; Yes -> Allocate shading table
	jsr	Allocate_memory
	move.b	d0,d6
	jsr	Claim_pointer
	move.l	d0,a4
	move.w	Wall_transparent_colour(a3),d0	; Calculate table
	move.w	#67,d1
	jsr	Calculate_transparent_table
	move.b	d6,d0
	jsr	Free_pointer
	move.b	d0,(a1)			; Store memory handle
	bra.s	.Next
.No:	btst	#Masked_wall_bit,d0		; No -> Masked ?
	beq.s	.Next
	st	(a1)			; Yes (!!!)
.Next:	addq.l	#1,a1
	dbra	d7,.Loop
	XFree	Labdata_handle
	LOCAL
; ---------- More initialization ------------------
;	jsr	Init_persons_3D		; Initialize persons
;	move.w	Map_Xcoord,d0		; Get first status
;	move.w	Map_Ycoord,d1
;	jsr	Get_location_status
;	move.l	d0,Current_location_status

	ifne	FALSE
; ---------- Float up/down ------------------------
	tst.b	Float_up			; Up ?
	beq	.Not_up
	move.w	#-wall_height+16,d0		; Yes -> Set first height
	move.w	d0,FX_camera_height
	jsr	Init_display		; Initialize display
	jsr	Update_screen		; Go
	moveq.l	#2,d1
.Again1:	cmp.w	#0,d0			; In the middle ?
	bge.s	.Done1
	move.w	d0,FX_camera_height		; No -> Set new height
	jsr	M3_DisUpd			; Display
	jsr	Map_DisUpd
	jsr	Switch_screens
	add.w	d1,d0			; Move up
	addq.w	#2,d1			; Faster
	bra.s	.Again1
.Done1:	jsr	Copy_screen		; The end
	clr.w	FX_camera_height
	bra	.Go_on
.Not_up:	tst.b	Float_down		; Down ?
	beq.s	.Not_down
	XGet	Labdata_handle,a0		; Yes -> Set first height
	move.w	Wall_in_cm(a0),d1
	XFree	Labdata_handle
	lsr.w	#1,d1
	sub.w	Camera_height,d1
	move.w	d1,FX_camera_height
	jsr	Init_display		; Initialize display
	jsr	Update_screen		; Go
	moveq.l	#-2,d1
.Again2:	tst.w	d0			; In the middle ?
	ble.s	.Done2
	move.w	d0,FX_camera_height		; No -> Set new height
	jsr	M3_DisUpd			; Display
	jsr	Map_DisUpd
	jsr	Switch_screens
	add.w	d1,d0			; Move down
	subq.w	#2,d1			; Faster
	bra.s	.Again2
.Done2:	jsr	Copy_screen		; The end
	clr.w	FX_camera_height
	bra.s	.Go_on
.Not_down:	jsr	Init_display		; Initialize display
.Go_on:	sf	Float_up			; Clear
	sf	Float_down
	jsr	Init_automap		; Initialize automap
	jsr	Update_automap
	jsr	Save_coordinates		; First time
	jmp	After_move		; Handle first step
	endc
	rts

;*****************************************************************************
; [ Calculate a shading table for a transparent wall ]
;   IN : d0 - Target colour (4 bits per gun) (.w)
;        d1 - Shading percentage (.w)
;        a4 - Pointer to shading table (.l)
; All registers are restored
;*****************************************************************************
Calculate_transparent_table:
	movem.l	d0-d7/a0/a1/a4,-(sp)

	move.w	#$f00,d0

	move.w	d1,d6			; Save
	move.w	#$00f0,d1			; Convert colour
	moveq.l	#$f,d2
	and.w	d0,d1
	and.w	d0,d2
	and.w	#$f00,d0
	lsr.w	#4,d0
	move.w	d0,d3
	lsr.w	#4,d0
	or.w	d0,d3
	move.w	d3,Target_R
	move.w	d1,d3
	lsr.w	#4,d1
	or.w	d1,d3
	move.w	d3,Target_G
	move.w	d2,d3
	lsl.w	#4,d2
	or.w	d2,d3
	move.w	d3,Target_B
	lea.l	Current_palette,a0		; Make shading table
	moveq.l	#100,d4
	move.w	d1,d6
	move.w	#256,d7			; (for Find_closest_colour)
	move.l	a0,a1
	move.w	#192-1,d5
.Loop1:	moveq.l	#0,d0			; Load colour
	move.b	(a1)+,d0
	moveq.l	#0,d1
	move.b	(a1)+,d1
	moveq.l	#0,d2
	move.b	(a1)+,d2
	sub.w	Target_R,d0		; Calculate differences
	sub.w	Target_G,d1
	sub.w	Target_B,d2
	muls	d6,d0			; Scale
	muls	d6,d1
	muls	d6,d2
	divs	d4,d0
	divs	d4,d1
	divs	d4,d2
	add.w	Target_R,d0		; Add
	add.w	Target_G,d1
	add.w	Target_B,d2
	jsr	Find_closest_colour
	move.b	d0,(a4)+
	dbra	d5,.Loop1			; Next colour
	move.w	#192,d0			; Last 64 colours remain
.Loop2:	move.b	d0,(a4)+			;  the same
	addq.b	#1,d0
	bne.s	.Loop2
	movem.l	(sp)+,d0-d7/a0/a1/a4
	rts

;*****************************************************************************
; [ 3D map ModExit ]
; No registers are restored
;*****************************************************************************
M3_ModExit:
;	jsr	Exit_automap		; Save automap
	move.b	Mapdata_handle,d0		; Free memory
	jsr	Free_memory
;	moveq.l	#0,d0			; Stop music
;	jsr	Set_music
	jmp	Remove_3D_map_data

;*****************************************************************************
; [ Display 3D map ]
; All registers are restored
;*****************************************************************************
M3_DisUpd:
	movem.l	d0-d7/a0-a6,-(sp)
	clr.l	Timer			; Reset time

	jsr	Do_map_colour_cycling

	Push	CA,Map3D_CA
	move.w	Y_angle,d0		; Set view direction
	neg.w	d0
	sub.w	#slang/8,d0
	and.w	#slang-1,d0
	ext.l	d0
	divu.w	#slang/4,d0
	addq.w	#1,d0
	and.w	#$0003,d0
	move.w	d0,View_direction
; ---------- Test for complete darkness -----------
	tst.b	Stygian			; Complete darkness ?
	beq.s	.See
	move.w	#Map3D_X,d0		; Erase map area
	move.w	#Map3D_Y,d1
	move.w	d0,d2
	move.w	d1,d3
	addi.w	#Map3D_width-1,d2
	addi.w	#Map3D_height-1,d3
	moveq.l	#0,d4
	jsr	Draw_box
	bra	.Done
; ---------- Pre-calculate addresses --------------
.See:	move.b	Labdata_handle,d0		; Set LAB-data pointer
	jsr	Claim_pointer
	move.l	d0,Labdata_ptr
	tst.b	Wall_ptrs_invalid		; Wall pointers invalid ?
	beq.s	.Skip1
	lea.l	Wall_handles,a0		; Yes -> Set wall pointers
	lea.l	Wall_ptrs,a1
	move.w	Nr_of_walls,d7
	bra.s	.Entry1
.Loop1:	move.b	(a0)+,d0
	jsr	Get_pointer
	move.l	d0,(a1)+
.Entry1:	dbra	d7,.Loop1
	sf	Wall_ptrs_invalid		; Valid now
.Skip1:	tst.b	Floor_ptrs_invalid		; Floor pointers invalid ?
	beq	.Skip2
	lea.l	Floor_handles,a0		; Yes -> Set floor pointers
	lea.l	Floor_ptrs,a1
	move.l	Labdata_ptr,a2
	add.l	Floor_data_offset,a2
	moveq.l	#0,d5			; Get first pointer
	move.b	(a0)+,d0
	jsr	Get_pointer
	move.l	d0,a3
	move.w	Nr_of_floors,d7		; Do all floors
	bra.s	.Entry2
.Loop2:	move.l	a1,a4
	moveq.l	#0,d6			; Get number of frames
	move.b	Floor_nr_frames(a2),d6
	subq.w	#1,d6
.Loop3:	move.l	a3,(a4)+			; Store pointer to frame
	lea.l	64(a3),a3
	addq.w	#1,d5			; Skip to next buffer ?
	and.w	#$0003,d5
	bne.s	.Go_on
	move.b	(a0)+,d0			; Yes -> Get pointer
	jsr	Get_pointer
	move.l	d0,a3
.Go_on:	dbra	d6,.Loop3			; Next frame
	lea.l	8*4(a1),a1		; Next floor
	lea.l	Floor3D_data_size(a2),a2
.Entry2:	dbra	d7,.Loop2
	sf	Floor_ptrs_invalid		; Valid now
.Skip2:	move.b	Dungeon_screen_handle,d0	; Set screen pointer
	jsr	Clear_memory
	jsr	Claim_pointer
	move.l	d0,Dungeon_screen
; ---------- Draw & display dungeon ---------------
	jsr	Draw_dungeon
	moveq.l	#Map3D_X,d0		; Display dungeon
	moveq.l	#Map3D_Y,d1
	move.w	#Map3D_width/16,d6
	move.w	#Map3D_height,d7
	move.l	Dungeon_screen,a0
	jsr	Display_chunky_block
; ---------- Free pointers ------------------------
	XFree	Dungeon_screen_handle
	XFree	Labdata_handle
.Done:	LOCAL
	Pop	CA
	subq.b	#1,Anim_count		; Time for an update ?
	beq.s	.Update
	cmp.w	#10,Anim_update		; Not too slow ?
	bmi.s	.Go_on
	bra.s	.Do
.Update:	cmp.w	#5,Anim_update		; Not too fast ?
	bpl.s	.Do
	move.b	#1,Anim_count		; Yes -> try again next time
	bra.s	.Go_on
.Do:	jsr	Update_animation		; Update
	move.b	#Anim_speed_3D,Anim_count	; Reset counters
	clr.w	Anim_update
.Go_on:	move.l	Timer,d0			; Update speed
	addq.l	#1,d0			; Current VBL as well !
;	cmp.w	#Max_3D_VBLs,d0		; Clip
;	bmi.s	.Vbl_Ok
;	move.w	#Max_3D_VBLs,d0
.Vbl_Ok:
	move.l	d0,-(sp)
	move.w	d0,d2
	move.w	#160,d0
	move.w	#160,d1
	jsr	Print_number

	move.w	#160,d0
	move.w	#178,d1
	move.w	shade_faktor,d2
	jsr	Print_number

	ifne	FALSE
	move.w	Nr_samples,d0
	move.l	Total,d1

	ext.l	d2
	addq.w	#1,d0
	bvs.s	.Reset
	add.l	d2,d1
	bvc.s	.Skippy
.Reset:	moveq.l	#1,d0
	move.l	d2,d1
.Skippy:
	move.w	d0,Nr_samples
	move.l	d1,Total

	divu.w	d0,d1
	move.w	d1,d2

	move.w	#160,d0
	move.w	#169,d1
	jsr	Print_number

	moveq.l	#50,d0
	divu.w	d2,d0
	move.w	d0,d2

	move.w	#160,d0
	move.w	#178,d1
	jsr	Print_number
	endc

	move.l	(sp)+,d0

	mulu.w	#50,d0			; Store
	move.w	d0,Dungeon_speed

	ifne	FALSE
	move.w	#100,d0
	move.w	#160,d1
	move.w	Camera_height,d2
	jsr	Print_number

	move.w	#100,d0
	move.w	#169,d1
	move.w	Horizon_Y,d2
	jsr	Print_number

	move.w	#Map3D_X,d0
	move.w	Horizon_Y,d1
	add.w	#Map3D_Y,d1
	move.w	#Map3D_X+Map3D_width-1,d2
	move.w	#White,d4
	jsr	Draw_hline
	endc

	movem.l	(sp)+,d0-d7/a0-a6
	rts

Total:	dc.l 0
Nr_samples:	dc.w 0

;*****************************************************************************
; [ Timer for frame-rate-independent movement ]
; All registers are restored
;*****************************************************************************
Time_3D:
	addq.l	#1,Timer			; Count for movement
	addq.w	#1,Anim_update		; Count for animation
	rts

;*****************************************************************************
; [ Analyze LAByrinth data ]
; All registers are restored
;*****************************************************************************
Analyze_labdata:
	movem.l	d0/d1/d7/a2/a3,-(sp)
	XGet	Labdata_handle,a2
; ---------- Calculate work variables -------------
	moveq.l	#0,d0			; Get wall height
	move.w	Wall_height_in_cm(a2),d0
	moveq.l	#psize_log,d1		; Convert from cm to 3D
	lsl.l	d1,d0
	divu.w	#Square_size,d0
	move.w	d0,Wall_height_3D		; Store
; ---------- Calculate offsets --------------------
	move.l	a2,d1
	lea.l	Lab_data(a2),a2
	move.w	(a2)+,d0			; Skip object groups
	mulu.w	#Objectgroup3D_data_size,d0
	add.l	d0,a2
	move.w	(a2)+,d7			; Get number of floors
	move.l	a2,d0			; Store offset
	sub.l	d1,d0
	move.l	d0,Floor_data_offset
	mulu.w	#Floor3D_data_size,d7	; Skip floors
	add.l	d7,a2
	move.w	(a2)+,d7			; Get number of objects
	move.l	a2,d0			; Store offset
	sub.l	d1,d0
	move.l	d0,Object_data_offset
	mulu.w	#Object3D_data_size,d7	; Skip objects
	add.l	d7,a2
	lea.l	Wall_data_offsets,a3	; Calculate wall offsets
	move.w	(a2)+,d7
	bra.s	.Entry
.Loop:	move.l	a2,d0			; Store offset
	sub.l	d1,d0
	move.l	d0,(a3)+
	move.w	Wall_nr_overlays(a2),d0	; Skip overlay data
	mulu.w	#Overlay_data_size,d0
	add.l	d0,a2
	lea.l	Wall_data_size(a2),a2	; Next wall
.Entry:	dbra	d7,.Loop
	XFree	Labdata_handle
	movem.l	(sp)+,d0/d1/d7/a2/a3
	rts

;*****************************************************************************
; [ Load 3D map data ]
; All registers are restored
;*****************************************************************************
Load_3D_map_data:
	movem.l	d0-d7/a0-a6,-(sp)
	st	Wall_ptrs_invalid		; Pointers are now invalid
	st	Floor_ptrs_invalid
	jsr	Load_map_texts		; Load map texts
; ---------- Load LAByrinth data ------------------
	XGet	Mapdata_handle,a0		; Get LAB data number
	moveq.l	#0,d1
	move.b	Lab_data_nr(a0),d1
	XFree	Mapdata_handle
	move.w	d1,d0			; Load
	moveq.l	#Lab_data_file,d1
	jsr	Load_subfile
	move.b	d0,Labdata_handle
	jsr	Claim_pointer
	move.l	d0,a2
; ---------- Load background ----------------------
	sf	Lab_background_handle	; Clear
	move.w	Background_nr(a2),d0	; Get background number
	beq.s	.No			; If any
	moveq.l	#Lab_background_file,d1	; Load
	jsr	Load_subfile
	move.b	d0,Lab_background_handle
.No:	LOCAL
	lea.l	Lab_data(a2),a2		; Get number of object groups
	move.w	(a2)+,d0
	move.w	d0,Nr_of_groups
	mulu.w	#Objectgroup3D_data_size,d0	; Skip
	add.l	d0,a2
; ---------- Load & convert floors & ceilings -----
	move.w	(a2)+,d5			; Get number of floors
	move.w	d5,Nr_of_floors
	lea.l	Floor_handles,a1		; Load and convert floors
	moveq.l	#1,d4
	subq.w	#1,d5
	moveq.l	#0,d7
	move.l	#64*256,d0		; Allocate first buffer
	jsr	Allocate_memory
	move.b	d0,d6
	jsr	Claim_pointer
	move.l	d0,a3
	move.b	d6,(a1)+
.Loop1:	move.w	Floor_graphics_nr(a2),d0	; Load floor
	moveq.l	#Floor_3D_file,d1
	jsr	Load_subfile
	move.l	d0,d2
	jsr	Claim_pointer
	move.l	d0,a0
	moveq.l	#0,d3			; Get number of frames
	move.b	Floor_nr_frames(a2),d3
	subq.w	#1,d3
.Loop2:	lea.l	63*64(a0),a0		; Copy floor frame
	lea.l	0(a3,d7.w),a4		;  (with V-flip)
	moveq.l	#64-1,d0
.Loop_Y:	moveq.l	#64/4-1,d1
.Loop_X:	move.l	(a0)+,(a4)+
	dbra	d1,.Loop_X
	lea.l	-2*64(a0),a0
	lea.l	256-64(a4),a4
	dbra	d0,.Loop_Y
	lea.l	65*64(a0),a0
	add.w	#64,d7			; Current buffer full ?
	and.w	#$00ff,d7
	bne.s	.Next2
	move.b	d6,d0			; Yes
	jsr	Free_pointer
	move.l	#64*256,d0		; Allocate next buffer
	jsr	Allocate_memory
	move.b	d0,d6
	jsr	Claim_pointer
	move.l	d0,a3
	move.b	d6,(a1)+			; Store handle
	addq.w	#1,d4
.Next2:	dbra	d3,.Loop2			; Next frame
	move.l	d2,d0			; Destroy floor
	jsr	Free_pointer
	jsr	Free_memory
	lea.l	Floor3D_data_size(a2),a2	; Next floor
	dbra	d5,.Loop1
	move.w	d4,Nr_of_floor_buffers	; Store
	LOCAL
; ---------- Load objects -------------------------
	move.w	(a2)+,d7			; Get number of objects
	move.w	d7,Nr_of_objects
	move.l	a2,d0			; Save offset
	sub.l	a6,d0
	move.l	d0,Object_data_offset
	lea.l	Batch,a0			; Build batch
	moveq.l	#0,d0
	moveq.l	#Object_3D_file,d1
	lea.l	Object_handles,a3
	bra.s	.Entry
.Loop:	move.w	Object_graphics_nr(a2),(a0)+	; Insert object number
	addq.w	#1,d0			; Count up
	cmp.w	#Max_batch,d0		; Batch full ?
	bmi.s	.Next
	lea.l	Batch,a0			; Yes -> Load batch
	move.l	a3,a1
	jsr	Load_batch_of_subfiles
	add.w	d0,a3			; Start next batch
	moveq.l	#0,d0
.Next:	lea.l	Object3D_data_size(a2),a2
.Entry:	dbra	d7,.Loop			; Next object
	tst.w	d0			; Batch empty ?
	beq.s	.Exit
	lea.l	Batch,a0			; No -> Load batch
	move.l	a3,a1
	jsr	Load_batch_of_subfiles
.Exit:	LOCAL
; ---------- Load walls ---------------------------
	move.w	(a2)+,d7                      ; Get number of walls
	move.w	d7,Nr_of_walls
	move.l	a2,-(sp)			; Save
	lea.l	Batch,a0			; Build batch
	moveq.l	#0,d0
	moveq.l	#Wall_3D_file,d1
	lea.l	Wall_handles,a3
	bra	.Entry
.Loop:	move.w	Wall_graphics_nr(a2),d2	; Get wall number
	move.w	Wall_nr_overlays(a2),d3	; Any overlays ?
	beq.s	.Store
	bset	#15,d2			; Yes -> Unique
.Store:	move.w	d2,(a0)+			; Insert wall number
	addq.w	#1,d0			; Count up
	cmp.w	#Max_batch,d0		; Batch full ?
	bmi.s	.Next
	lea.l	Batch,a0			; Yes -> Load batch
	move.l	a3,a1
	jsr	Load_batch_of_unique_subfiles
	add.w	d0,a3			; Start next batch
	moveq.l	#0,d0
.Next:	mulu.w	#Overlay_data_size,d3	; Next wall
	lea.l	Wall_data_size(a2,d3.w),a2
.Entry:	dbra	d7,.Loop
	tst.w	d0			; Batch empty ?
	beq.s	.Exit
	lea.l	Batch,a0			; No -> Load batch
	move.l	a3,a1
	jsr	Load_batch_of_unique_subfiles
.Exit:	move.l	(sp)+,a2
	LOCAL
; ---------- Load overlays & put on wall ----------

; REGISTER CONTENTS :
;   d2 - Current wall size in bytes (.l)
;   d6 - Overlay counter (.w)
;   d7 - Wall counter (.w)
;   a1 - Pointer to wall graphics (.l)
;   a2 - Pointer to wall data (.l)
;   a3 - Pointer to wall memory handles list (.l)

	lea.l	Wall_handles,a3
	move.w	Nr_of_walls,d7
	bra	.Entry1
.Loop1:	move.w	Wall_nr_overlays(a2),d6	; Get number of overlays
	bne.s	.Skip
	lea.l	Wall_data_size(a2),a2	; If any
	bra	.Next1
.Skip:	move.b	Wall_nr_frames(a2),d5	; How many frames does
	cmp.b	#1,d5			;  the wall have ?
	bne	.Wall_more
	moveq.l	#1,d0			; One -> How many frames
	lea.l	Wall_data_size(a2),a4	;  do the overlays have ?
	move.w	d6,d1
	subq.w	#1,d1
.Loop5:	cmp.b	Overlay_nr_frames(a4),d0
	bpl.s	.Next5
	move.b	Overlay_nr_frames(a4),d0
.Next5:	lea.l	Overlay_data_size(a4),a4
	dbra	d1,.Loop5
	move.w	d0,d5			; Save
	cmp.w	#1,d5			; Only one frame ?
	beq	.Wall_more
	move.w	Wall_width(a2),d2		; No -> Get wall size
	mulu.w	Wall_height(a2),d2
	mulu.w	d2,d0			; Allocate wall frames
	jsr	Allocate_memory
	move.b	d0,d1
	jsr	Claim_pointer
	move.l	d0,a1
	move.l	a1,-(sp)
	move.b	(a3),d0			; Get wall address
	jsr	Claim_pointer
	move.l	d0,a0
	move.l	d2,d0			; Copy wall graphics
	move.w	d5,d3
	subq.w	#1,d3
.Loop4:	jsr	Copy_memory	
	add.l	d2,a1
	dbra	d3,.Loop4
	move.b	(a3),d0			; Destroy wall
	jsr	Free_pointer
	jsr	Free_memory
	move.b	d1,(a3)			; Store new handle
	move.l	(sp)+,a1
	bra.s	.Do_it
.Wall_more:
	move.w	Wall_width(a2),d2		; Get wall size
	mulu.w	Wall_height(a2),d2
	move.b	(a3),d0			; Invalidate wall
	jsr	Invalidate_memory
	jsr	Claim_pointer		; Get wall address
	move.l	d0,a1

; REGISTER CONTENTS :
;   d3 - Current overlay size in bytes (.l)
;   d4 - Current overlay frame (.w)
;   d5 - Wall animation frame counter (.w)
;   a0 - Pointer to current overlay frame (.l)
;   a4 - Pointer to overlay data (.l)
;   a5 - Pointer to overlay graphics (.l)

.Do_it:	lea.l	Wall_data_size(a2),a4	; Get overlay data
	bra	.Entry2
.Loop2:	move.l	a1,-(sp)
	move.w	Overlay_graphics_nr(a4),d0	; Load overlay graphics
	beq	.Next2
	moveq.l	#Overlay_3D_file,d1
	jsr	Load_subfile
	move.l	d0,-(sp)
	jsr	Claim_pointer		; Get overlay address
	move.l	d0,a5
	move.w	Overlay_width(a2),d3	; Get overlay size
	mulu.w	Overlay_height(a2),d3
	moveq.l	#0,d4			; Start with frame 0
	moveq.l	#0,d5			; Put overlay on walls
	move.b	Wall_nr_frames(a2),d5
	subq.w	#1,d5
.Loop3:	move.l	a5,a0			; Select frame
	move.w	d4,d0
	mulu.w	d3,d0
	add.l	d0,a0
	jsr	Put_overlay_on_wall		; Put on wall
	addq.w	#1,d4			; Next overlay frame
	cmp.b	Overlay_nr_frames(a4),d4	; Last frame ?
	bmi.s	.No
	moveq.l	#0,d4			; Yes -> Back to first
.No:	add.l	d2,a1			; Next wall frame
	dbra	d5,.Loop3
	move.l	(sp)+,d0			; Remove overlay
	jsr	Free_pointer
	jsr	Free_memory
.Next2:	lea.l	Overlay_data_size(a4),a4	; Next overlay
	move.l	(sp)+,a1
.Entry2:	dbra	d6,.Loop2
	move.b	(a3),d0			; Free
	jsr	Free_pointer
	move.l	a4,a2
.Next1:	addq.l	#1,a3			; Next wall
.Entry1:	dbra	d7,.Loop1
	XFree	Labdata_handle
	LOCAL
; ---------- Allocate buffers ---------------------
	move.l	#Map3D_width*Map3D_height,d0
	jsr	Allocate_memory
	move.b	d0,Dungeon_screen_handle
	movem.l	(sp)+,d0-d7/a0-a6
	rts

;*****************************************************************************
; [ Put overlay on 3D wall ]
;   IN : a0 - Overlay graphics (.l)
;        a1 - Wall graphics (.l)
;        a2 - Pointer to wall data (.l)
;        a4 - Pointer to overlay data (.l)
; All registers are restored
;*****************************************************************************
Put_overlay_on_wall:
	movem.l	d0/d1/d6/d7/a0/a1,-(sp)
	move.w	Overlay_X(a4),d0		; Calculate position on wall
	mulu.w	Wall_height(a2),d0
	add.l	d0,a1
	add.w	Overlay_Y(a4),a1
	move.w	Wall_height(a2),d0		; Calculate line offset
	sub.w	Overlay_height(a4),d0
	btst	#0,Overlay_mask_flag(a4)	; Block or mask ?
	bne.s	.Mask
	move.w	Overlay_width(a4),d7	; Block
	subq.w	#1,d7
.Loop_Y1:	move.w	Overlay_height(a4),d6
	subq.w	#1,d6
.Loop_X1:	move.b	(a0)+,(a1)+
	dbra	d6,.Loop_X1
	add.w	d0,a1
	dbra	d7,.Loop_Y1
	bra.s	.Exit
.Mask:	move.w	Overlay_width(a4),d7	; Mask
	subq.w	#1,d7
.Loop_Y2:	move.w	Overlay_height(a4),d6
	subq.w	#1,d6
.Loop_X2:	move.b	(a0)+,d1
	beq.s	.Next_X2
	move.b	d1,(a1)
.Next_X2:	addq.l	#1,a1
	dbra	d6,.Loop_X2
	add.w	d0,a1
	dbra	d7,.Loop_Y2
.Exit:	movem.l	(sp)+,d0/d1/d6/d7/a0/a1
	rts

;*****************************************************************************
; [ Remove 3D map data ]
; All registers are restored
;*****************************************************************************
Remove_3D_map_data:
	movem.l	d0/d7/a0/a1,-(sp)
	lea.l	Object_handles,a1		; Free object memory
	move.w	Nr_of_objects,d7
	bra.s	.Entry1
.Loop1:	move.b	(a1)+,d0
	jsr	Free_memory
.Entry1:	dbra	d7,.Loop1
	lea.l	Wall_handles,a1		; Free wall memory
	move.w	Nr_of_walls,d7
	bra.s	.Entry2
.Loop2:	move.b	(a1)+,d0
	jsr	Free_memory
.Entry2:	dbra	d7,.Loop2
	lea.l	Floor_handles,a1		; Free floor memory
	move.w	Nr_of_floor_buffers,d7
	bra.s	.Entry3
.Loop3:	move.b	(a1)+,d0
	jsr	Free_memory
.Entry3:	dbra	d7,.Loop3
	move.b	Lab_background_handle,d0	; Free background memory
	jsr	Free_memory
	move.b	Labdata_handle,d0		; Free labyrinth data
	jsr	Free_memory
	move.b	Maptext_handle,d0		; Free map texts
	jsr	Free_memory
	move.b	Dungeon_screen_handle,d0	; Free buffers
	jsr	Free_memory
	movem.l	(sp)+,d0/d7/a0/a1
	rts

;*****************************************************************************
; [ Adjust 3D position ]
; All registers are restored
; Notes :
;   - This routine will not do anything if called in a 2D map.
;*****************************************************************************
Jumped:
	tst.b	_2D_or_3D_map		; 3D map ?
	beq.s	.Exit
;	jsr	Update_automap		; Update !
	jsr	Initialize_3D_position
.Exit:	rts

;*****************************************************************************
; [ Initialize 3D position ]
; All registers are restored
;*****************************************************************************
Initialize_3D_position:
	movem.l	d0-d2/d5-d7/a0,-(sp)
	moveq.l	#0,d0			; Get dungeon position
	moveq.l	#0,d1
	move.w	Map_Xcoord,d0
	move.w	Map_Ycoord,d1
	jsr	Map_to_dungeon
	and.w	#-patt_size,d0		; Set to bottom-left corner
	and.w	#-patt_size,d1
	move.l	d0,d5			; Save clean position
	move.l	d1,d6

	ifne	FALSE
	lea.l	Alternate_positions,a0	; Try positions in map square
	moveq.l	#9-1,d7
.Loop:	move.l	d5,d0			; Add offset
	move.l	d6,d1
	add.w	(a0)+,d0
	add.w	(a0)+,d1
	swap	d0
	swap	d1
	jsr	Movement_check_3D		; Possible ?
	moveq.l	#0,d0
	beq.s	.Store
	dbra	d7,.Loop			; No -> Next
	endc

	move.l	d5,d0			; Centre anyway
	move.l	d6,d1
	add.l	#patt_size/2,d0
	add.l	#patt_size/2,d1
.Store:	move.l	d0,Player_X		; Store
	move.l	d1,Player_Y
	clr.w	Player_X+4
	clr.w	Player_Y+4
	moveq.l	#0,d0			; Initialize direction
	move.w	View_direction,d0
	mulu.w	#slang/4,d0
	neg.w	d0
	and.w	#slang-1,d0
	swap	d0
	move.l	d0,Y_angle
	movem.l	(sp)+,d0-d2/d5-d7/a0
	rts

;*****************************************************************************
; [ Calculate camera height ]
; All registers are restored
;*****************************************************************************
Calculate_camera_height:
	movem.l	d0-d2/a0,-(sp)
	XGet	Labdata_handle,a0		; Get height of wall in cm
	move.w	Wall_height_in_cm(a0),d1
	XFree	Labdata_handle

	ifne	FALSE
	XGet	Active_handle,a0		; Get race of active
	moveq.l	#0,d2			;  character
	move.b	Char_race(a0),d2
	XFree	Active_handle
	lea.l	Race_heights,a0		; Get height of race
	move.w	0(a0,d2.w*2),d2		; !!!
	sub.w	#15,d2			; - 15 cm
	cmp.w	d0,d2			; Too high ?
	bmi.s	.Ok
	move.w	d0,d2
.Ok:	endc

	move.w	#180-15,d2

	add.w	FX_camera_height,d2		; Add for effect
	move.w	d2,Camera_height		; Store
	movem.l	(sp)+,d0-d2/a0
	rts

;*****************************************************************************
; [ Convert map coordinates to dungeon coordinates ]
;   IN : d0 - Map X-coordinate (.w)
;        d1 - Map Y-coordinate (.w)
;  OUT : d0 - Dungeon X-coordinate (.l)
;        d1 - Dungeon Y-coordinate (.l)
; Changed registers : d0,d1
; Notes :
;  - The dungeon coordinates are given for the centre of the map square.
;*****************************************************************************
Map_to_dungeon:
	move.l	d2,-(sp)
	tst.w	d0			; Both zero ?
	bne.s	.Notzero
	tst.w	d1
	beq.s	.Exit
.Notzero:	subq.w	#1,d0			; Move to base 0
	sub.w	Height_of_map,d1		; Vertical flip
	neg.w	d1
	moveq.l	#psize_log,d2		; Scale up
	lsl.l	d2,d0
	lsl.l	d2,d1
	add.l	#patt_size/2,d0		; Move to centre
	add.l	#patt_size/2,d1
.Exit:	move.l	(sp)+,d2
	rts

;*****************************************************************************
; [ Convert dungeon coordinates to map coordinates ]
;   IN : d0 - Dungeon X-coordinate (.l)
;        d1 - Dungeon Y-coordinate (.l)
;  OUT : d0 - Map X-coordinate (.w)
;        d1 - Map Y-coordinate (.w)
; Changed registers : d0,d1
;*****************************************************************************
Dungeon_to_map:
	move.l	d2,-(sp)
	moveq.l	#psize_log,d2		; Scale down
	lsr.l	d2,d0
	lsr.l	d2,d1
	addq.w	#1,d0			; Move to base 1
	sub.w	Height_of_map,d1		; Vertical flip
	neg.w	d1
	move.l	(sp)+,d2
	rts

;*****************************************************************************
; [ Relocate 3D wall file ]
;   IN : d0 - Size of memory block (.l)
;        a0 - Source address (.l)
;        a1 - Target address (.l)
;        a3 - Pointer to source memory block descriptor (.l)
; All registers are	restored
;*****************************************************************************
Relocate_3D_wall:
	jsr	Copy_memory		; Copy
	btst	#Allocated,Block_flags(a3)	; Allocated ?
	beq.s	.Exit
	st	Wall_ptrs_invalid		; Yes -> Reset pointers
.Exit:	rts

;*****************************************************************************
; [ Relocate 3D floor buffer file ]
;   IN : d0 - Size of memory block (.l)
;        a0 - Source address (.l)
;        a1 - Target address (.l)
;        a3 - Pointer to source memory block descriptor (.l)
; All registers are restored
;*****************************************************************************
Relocate_3D_floor_buffer:
	jsr	Copy_memory		; Copy
	btst	#Allocated,Block_flags(a3)	; Allocated ?
	beq.s	.Exit
	st	Floor_ptrs_invalid		; Yes -> Reset pointers
.Exit:	rts

;*****************************************************************************
; [ Calculate shade function ]
; No registers are restored
;*****************************************************************************
Calculate_shade_function:
	lea	Shade_function,a0
	moveq.l	#63,d0
	move.w	#256-1,d7
.Loop1:	move.b	d0,(a0)+
	dbra	d7,.Loop1
	move.w	#64-1,d7
.Loop2:	move.w	#10-1,d6
.Loop3:	move.b	d0,(a0)+
	dbra	d6,.Loop3
	subq.w	#1,d0
	dbra	d7,.Loop2
;	moveq.l	#0,d0
;	move.w	#512-1,d7
;.Loop4:	move.b	d0,(a0)+
;	dbra	d7,.Loop4	
	rts

	ifne	FALSE
;	lea	Shade_function+4096,a0
	lea	Shade_function,a0
	move	#512,d6			;shade-faktor
	move.l	#512*63,d5
	moveq	#0,d4
	move	#4096-1,d7
.Loop:	move	d4,d0			;take Z
	add	d6,d0			;Z+shade-faktor
	move.l	d5,d1
	divu	d0,d1
;	moveq.l	#63,d2
;	sub.b	d1,d2
;	move.b	d2,-(a0)			;0...63
         	move.b	d1,(a0)+
	add.w	#32768/4096,d4
	dbra	d7,.Loop
	rts
	endc

;*****************************************************************************
; [ Calculate shade table ]
; No registers are restored
;*****************************************************************************
Calculate_shade_table:
	lea.l	Current_palette,a0		; Write levels 0...62
	lea.l	Shade_table,a1

;	lea.l	63*256(a1),a1

;	ifne	FALSE
	moveq.l	#63,d4
	move.w	#256,d7			; Parameter for Find_closest_colour
	moveq.l	#0,d6
.Loop4:	move.l	a0,a2			; Shade first 192 colours
;	move.w	#192-1,d5
	move.w	#256-1,d5
.Loop5:	moveq.l	#0,d0			; Load colour
	move.b	(a2)+,d0
	moveq.l	#0,d1
	move.b	(a2)+,d1
	moveq.l	#0,d2
	move.b	(a2)+,d2
	sub.w	Target_R,d0		; Calculate differences
	sub.w	Target_G,d1
	sub.w	Target_B,d2
	muls	d6,d0			; Scale
	muls	d6,d1
	muls	d6,d2
	divs	d4,d0
	divs	d4,d1
	divs	d4,d2
	add.w	Target_R,d0		; Add
	add.w	Target_G,d1
	add.w	Target_B,d2
	jsr	Find_closest_colour		; Find closest colour
	move.b	d0,(a1)+			; Store index
	dbra	d5,.Loop5			; Next colour
;	move.w	#192,d0			; Last 64 colours remain
;.Loop6:	move.b	d0,(a1)+			;  the same
;	addq.b	#1,d0
;	bne.s	.Loop6
	addq.w	#1,d6			; Next shade level
	cmp.w	#63,d6
	bmi.s	.Loop4
;	endc

	moveq.l	#0,d0			; Write last shade level
.Loop3:	move.b	d0,(a1)+
	addq.b	#1,d0
	bne.s	.Loop3
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************	
	SECTION	Fast_DATA,data
Map3D_CA:	dc.w Map3D_X,Map3D_X+Map3D_width-1
	dc.w Map3D_Y,Map3D_Y+Map3D_height-1
