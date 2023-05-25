; Mouse-pointer handling (display only)
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	XDEF	Mouse_on
	XDEF	Mouse_off
	XDEF	Reset_mouse
	XDEF	Push_MA
	XDEF	Pop_MA
	XDEF	Reset_MA_stack
	XDEF	Push_Mptr
	XDEF	Push_busy_Mptr
	XDEF	Pop_Mptr
	XDEF	Pop_busy_Mptr
	XDEF	Reset_Mptr_stack
	XDEF	Change_Mptr
	XDEF	MA_Sp
	XDEF	Default_MA

	SECTION	Program,code
;*****************************************************************************
; [ Switch the mouse on ]
; All registers are	restored
;*****************************************************************************
Mouse_on:
	tst.b	Suppress_mouse		; Already on ?
	beq.s	.Exit
	subq.b	#1,Suppress_mouse		; Decrease flag
	bne.s	.Exit			; Switch on ?
	jsr	Update_mouse		; Yes -> Update position
	move.l	a0,-(sp)			; Restore image
	move.l	Mptr_Sp,a0
	move.l	(a0),a0
	jsr	Init_Mptr
	move.l	(sp)+,a0
.Exit:	rts

;*****************************************************************************
; [ Switch the mouse off ]
; All registers are	restored
;*****************************************************************************
Mouse_off:
	cmpi.b	#-1,Suppress_mouse		; At maximum ?
	beq.s	.Exit
	tst.b	Suppress_mouse		; Switch off ?
	bne.s	.No
	jsr	Erase_Mptr		; Yes
.No:	addq.b	#1,Suppress_mouse		; Increase flag
.Exit:	rts

;*****************************************************************************
; [ Reset the mouse state ]
; All registers are	restored
;*****************************************************************************
Reset_mouse:
	clr.b	Suppress_mouse		; Turn it on
	jsr	Update_mouse		; Update position
	move.l	a0,-(sp)			; Restore image
	move.l	Mptr_Sp,a0
	move.l	(a0),a0
	jsr	Init_Mptr
	move.l	(sp)+,a0
	rts
	
;*****************************************************************************
; [ Initialize the mouse ]
; All registers are	restored
; Notes :
;   - This routine MUST be called BEFORE the screen is opened.
;*****************************************************************************
Init_mouse:
	movem.l	d0/d1/d6/d7/a0/a1,-(sp)
	moveq.l	#4,d5			; Prepare bitmap
	moveq.l	#Mptr_width,d6
	moveq.l	#Mptr_height,d7
	lea.l	Mouse_bitmap,a0
	lea.l	Mouse_buffer,a1
	jsr	Prepare_bitmap
	lea.l	Default_Mptr,a1		; Set hotspot coordinates
	move.w	(a1)+,HotSpot_X
	move.w	(a1)+,HotSpot_Y
	addq.l	#2,a1			; Skip colour base
	jsr	Copy_to_bitmap		; Copy graphics
	move.w	Mouse_X,d0		; Create sprites
	move.w	Mouse_Y,d1
	moveq.l	#Mptr_width,d6
	move.l	a0,a1
	lea.l	Mouse_sprite,a0
	jsr	Init_sprite
	st	Sprites_created		; Go!
	movem.l	(sp)+,d0/d1/d6/d7/a0/a1
	rts

;*****************************************************************************
; [ Free the mouse ]
; All registers are	restored
;*****************************************************************************
Exit_mouse:
	move.l	a0,-(sp)
	sf	Sprites_created		; Stop
	lea.l	Mouse_sprite,a0		; Remove sprites
	jsr	Exit_sprite
	move.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Push a MA on the stack ]
;   IN : a0 - Pointer to MA (.l)
; All registers are	restored
;*****************************************************************************
Push_MA:
	move.l	a1,-(sp)
	movea.l	MA_Sp,a1
	addq.l	#4,a1
	cmpa.l	#MAStack_end,a1		; Possible ?
	beq.s	.Exit
	move.l	a0,(a1)			; Push
	move.l	a1,MA_Sp
	jsr	Init_MA			; Initialize new MA
.Exit:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Pop a MA from the stack ]
; All registers are	restored
;*****************************************************************************
Pop_MA:
	move.l	a0,-(sp)
	movea.l	MA_Sp,a0
	cmpa.l	#MAStack_start,a0		; Possible ?
	beq.s	.Exit
	subq.l	#4,a0			; Pop
	move.l	a0,MA_Sp
	move.l	(a0),a0			; Initialize old MA
	jsr	Init_MA
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Reset the MA stack ]
; All registers are	restored
;*****************************************************************************
Reset_MA_stack:
	movem.l	a0/a1,-(sp)
	lea.l	Default_MA,a0		; Reset stack
	lea.l	MAStack_start,a1
	move.l	a1,MA_Sp
	move.l	a0,(a1)
	jsr	Init_MA			; Initialize default MA
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Initialize a MA ]
;   IN : a0 - Pointer to MA (.l)
; All registers are	restored
;*****************************************************************************
Init_MA:
	movem.l	d0/d1,-(sp)
	move.w	Mouse_X,d0		; Load coordinates
	move.w	Mouse_Y,d1
	cmp.w	X1(a0),d0			; X too low ?
	bpl.s	.Left_OK
	move.w	X1(a0),d0
	bra.s	.Check_Y
.Left_OK:	cmp.w	X2(a0),d0			; X too high ?
	blt.s	.Check_Y
	move.w	X2(a0),d0
.Check_Y:	cmp.w	Y1(a0),d1			; Y too low ?
	bpl.s	.Top_OK
	move.w	Y1(a0),d1
	bra.s	.Done
.Top_OK:	cmp.w	Y2(a0),d1			; Y too high ?
	blt.s	.Done
	move.w	Y2(a0),d1
.Done:	move.w	d0,Mouse_X		; Store coordinates
	move.w	d1,Mouse_Y
	movem.l	(sp)+,d0/d1
	rts

;*****************************************************************************
; [ Change the current mouse pointer ]
;   IN : a0 - Pointer to mouse pointer (.l)
; All registers are	restored
;*****************************************************************************
Change_Mptr:
	move.l	a1,-(sp)
	movea.l	Mptr_Sp,a1
	cmp.l	(a1),a0			; Change ?
	beq.s	.Skip
	move.l	a0,(a1)			; Change !
	jsr	Init_Mptr			; Initialize new mouse
.Skip:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Push a mouse pointer on the stack ]
;   IN : a0 - Pointer to mouse pointer (.l)
; All registers are	restored
;*****************************************************************************
Push_Mptr:
	move.l	a1,-(sp)
	movea.l	Mptr_Sp,a1
	addq.l	#4,a1
	cmpa.l	#MptrStack_end,a1		; Possible ?
	beq.s	.Exit
	clr.w	Busy_timer		; Too late!
	sf	Switch_to_busy
	move.l	a0,(a1)			; Push
	move.l	a1,Mptr_Sp
	jsr	Init_Mptr			; Initialize new mouse
.Exit:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Push a delayed mouse pointer on the stack ]
;   IN : a0 - Pointer to mouse pointer (.l)
; All registers are	restored
;*****************************************************************************
Push_busy_Mptr:
	move.l	a1,-(sp)
	movea.l	Mptr_Sp,a1
	addq.l	#4,a1
	cmpa.l	#MptrStack_end,a1		; Possible ?
	beq.s	.Exit
	move.l	-4(a1),(a1)		; Yes -> Duplicate current
	move.l	a1,Mptr_Sp
	cmp.l	Busy_Mptr,a0		; The same ?
	beq.s	.Exit
	clr.w	Busy_timer		; No
	sf	Switch_to_busy
	move.l	a0,Busy_Mptr		; Store busy pointer
	move.w	#Busy_interval,Busy_timer	; Set timer
.Exit:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Pop a mouse pointer from the stack ]
; All registers are	restored
;*****************************************************************************
Pop_Mptr:
	move.l	a0,-(sp)
	movea.l	Mptr_Sp,a0
	cmpa.l	#MptrStack_start,a0		; Possible ?
	beq.s	.Exit
	clr.w	Busy_timer		; Too late!
	sf	Switch_to_busy
	subq.l	#4,a0			; Pop
	move.l	a0,Mptr_Sp
	move.l	(a0),a0			; Initialize old mouse
	jsr	Init_Mptr
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Pop a delayed mouse pointer from the stack ]
; All registers are	restored
;*****************************************************************************
Pop_busy_Mptr:
	move.l	a0,-(sp)
	movea.l	Mptr_Sp,a0
	cmpa.l	#MptrStack_start,a0		; Possible ?
	beq.s	.Exit
	clr.w	Busy_timer		; Yes
	sf	Switch_to_busy
	subq.l	#4,a0			; Pop
	move.l	a0,Mptr_Sp
	move.l	(a0),Busy_Mptr		; Store busy pointer
	move.l	4(a0),(a0)		; Duplicate current
	move.w	#Busy_interval,Busy_timer	; Set timer
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Reset the mouse pointer stack ]
; All registers are	restored
;*****************************************************************************
Reset_Mptr_stack:
	movem.l	a0/a1,-(sp)
	lea.l	Default_Mptr,a0		; Reset stack
	lea.l	MptrStack_start,a1
	move.l	a0,(a1)
	move.l	a1,Mptr_Sp
	jsr	Init_Mptr			; Initialize default mouse
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Initialize a mouse pointer ]
;   IN : a0 - Pointer to mouse pointer (.l)
; All registers are	restored
; Notes :
;   - This routine MUST be called AFTER the screen is opened.
;*****************************************************************************
Init_Mptr:
	movem.l	d0/d1/a0-a2,-(sp)
	move.l	a0,a2
	move.w	(a2)+,HotSpot_X		; Set hot-spot coordinates
	move.w	(a2)+,HotSpot_Y
	lea.l	Videocontrol_tag_items,a0	; Insert colour base
	move.l	#VTAG_SPODD_BASE_SET,d0	;  in tag list
	moveq.l	#0,d1
	move.w	(a2)+,d1
	jsr	Set_tag_data
	move.l	#VTAG_SPEVEN_BASE_SET,d0
	jsr	Set_tag_data
	lea.l	Mouse_sprite,a0		; Change sprite image
	move.l	a2,a1
	jsr	Change_sprite_image
	move.l	#-1,Video_control_flag	; Set flag
	move.l	My_screen,a0		; Set colour base
	lea.l	sc_ViewPort(a0),a0
	move.l	vp_ColorMap(a0),a0
	lea.l	Videocontrol_tag_items,a1
	kickGFX	VideoControl
	tst.l	Video_control_flag		; Success ?
	beq.s	.Exit
	move.l	My_screen,a0		; No -> Do it yourself
	kickINTU	MakeScreen
	kickINTU	RethinkDisplay
.Exit:	movem.l	(sp)+,d0/d1/a0-a2
	rts

;*****************************************************************************
; [ Initialize an empty mouse pointer ]
; All registers are	restored
;*****************************************************************************
Erase_Mptr:
	movem.l	d0/d7/a0/a1,-(sp)
	move.l	#(Mptr_width/8)*Mptr_height*4,d0	; Get empty graphics
	jsr	Allocate_memory
	jsr	Clear_memory
	move.b	d0,d7
	jsr	Claim_pointer
	move.l	d0,a1
	lea.l	Mouse_sprite,a0		; Change sprite image
	jsr	Change_sprite_image
	move.b	d7,d0			; Destroy empty graphics
	jsr	Free_pointer
	jsr	Free_memory
	movem.l	(sp)+,d0/d7/a0/a1
	rts

;*****************************************************************************
; [ Update the mouse pointer ]
; All registers are	restored
;*****************************************************************************
Update_mouse:
	movem.l	d0-d3/a0-a2,-(sp)
	tst.b	Sprites_created		; Sprites created ?
	beq	.Exit
; ---------- Update sprite position ---------------
	move.w	Mouse_X,d2		; Get mouse coordinates
	move.w	Mouse_Y,d3
	sub.w	HotSpot_X,d2		; Add hot-spot coordinates
	sub.w	HotSpot_Y,d3
	ext.l	d2
	ext.l	d3
	lea.l	Mouse_sprite,a2
	move.l	d2,d0			; Move sprite 1
	move.l	d3,d1
	move.l	My_screen,a0
	lea.l	sc_ViewPort(a0),a0
	move.l	Sprite_1(a2),a1
	kickGFX	MoveSprite
	move.l	d2,d0			; Move sprite 2
	move.l	d3,d1
	move.l	My_screen,a0
	lea.l	sc_ViewPort(a0),a0
	move.l	Sprite_2(a2),a1
	kickGFX	MoveSprite
	cmp.w	#39,Kickstart_version	; V39 ?
	bne.s	.Exit
	move.l	Sprite_2(a2),a0		; Yes -> Fix bug
	move.l	ss_posctldata(a0),a0
	move.w	Sprite_width(a2),d0
	lsr.w	#3,d0
	bset	#7,1(a0,d0.w)
; ---------- Handle delayed mouse-pointer ---------
	tst.w	Busy_timer		; Timing ?
	beq.s	.Exit
	subq.w	#1,Busy_timer		; Yes -> Is it time ?
	bgt.s	.Exit
	st	Switch_to_busy		; Yes -> Signal
.Exit:	movem.l	(sp)+,d0-d3/a0-a2
	rts

;*****************************************************************************
; [ Handle delayed busy mouse pointers ]
; All registers are	restored
;*****************************************************************************
Handle_busy_pointer:
	tst.b	Switch_to_busy		; Switch ?
	beq.s	.Exit
	movem.l	a0/a1,-(sp)
	move.l	Busy_Mptr,a0		; Yes
	movea.l	Mptr_Sp,a1
	move.l	a0,(a1)
	jsr	Init_Mptr
	sf	Switch_to_busy
	movem.l	(sp)+,a0/a1
.Exit:	rts

;***************************************************************************
; [ Initialize a hardware sprite ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d6 - Width of sprite in pixels (.w)
;        a0 - Pointer to sprite data structure (.l)
;        a1 - Pointer to bitmap (.l)
; All registers are restored
;***************************************************************************
Init_sprite:
	movem.l	d0-d7/a0-a3/a6,-(sp)
	move.w	d0,d4			; Save
	move.w	d1,d5
	ext.l	d4
	ext.l	d5
	move.l	a0,a6
	clr.l	Sprite_1(a6)		; Clear data
	clr.l	Sprite_2(a6)
	move.w	#-1,Sprite_1_nr(a6)
	move.w	#-1,Sprite_2_nr(a6)
	add.w	#15,d6			; Adjust width
	and.w	#$fff0,d6
	move.w	d6,Sprite_width(a6)		; Store
	move.l	a1,Sprite_bitmap(a6)
	move.l	#SPRITEA_Width,d0		; Insert in tag lists
	moveq.l	#0,d1
	move.w	d6,d1
	lea.l	Alloc_sprite_A_tag_items,a0
	jsr	Set_tag_data
	lea.l	Alloc_sprite_B_tag_items,a0
	jsr	Set_tag_data
; ---------- Allocate sprites ---------------------
	move.l	a1,a2			; Allocate 1st sprite
	lea.l	Alloc_sprite_A_tag_items,a1
	kickGFX	AllocSpriteDataA
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#ALLOC_SPRITE_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,Sprite_1(a6)		; Yes
	LOCAL
	lea.l	Alloc_sprite_B_tag_items,a1	; Allocate 2nd sprite
	kickGFX	AllocSpriteDataA
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#ALLOC_SPRITE_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,Sprite_2(a6)		; Yes
	LOCAL
; ---------- Get sprites --------------------------
	cmp.w	#39,Kickstart_version	; V39 ?
	bne.s	.No
	lea.l	Get_sprite_A_tag_items_V39,a3	; Yes
	bra.s	.Go_on
.No:	lea.l	Get_sprite_A_tag_items_V40,a3	; No
	move.l	a3,a0			; Insert 2nd sprite pointer
	move.l	#GSTAG_ATTACHED,d0		;  in first tag list
	move.l	Sprite_2(a6),d1
	jsr	Set_tag_data
.Go_on:	LOCAL
	moveq.l	#2,d7			; Start at 2
.Again:	move.l	a3,a0			; Insert desired sprite number
	move.l	#GSTAG_SPRITE_NUM,d0
	move.l	d7,d1
	jsr	Set_tag_data
	move.l	a3,a1			; Get 1st sprite
	move.l	Sprite_1(a6),a2
	kickGFX	GetExtSpriteA
	cmp.l	#-1,d0			; Success ?
	bne.s	.Ok
	addq.l	#2,d7			; No -> Try next pair
	cmp.l	#8,d7			; Maximum sprite reached ?
	bmi.s	.Again
	move.l	#GET_SPRITE_ERROR,d0	; Yes -> Exit
	jmp	Fatal_error
.Ok:	move.w	d0,Sprite_1_nr(a6)
	LOCAL
	cmp.w	#39,Kickstart_version	; V39 ?
	bne.s	.No
	lea.l	Get_sprite_B_tag_items,a0	; Yes -> Insert desired sprite number
	move.l	d0,d1
	addq.l	#1,d1
	move.l	#GSTAG_SPRITE_NUM,d0
	jsr	Set_tag_data
	lea.l	Get_sprite_B_tag_items,a1	; Get 2nd sprite
	move.l	Sprite_2(a6),a2
	kickGFX	GetExtSpriteA
	cmp.l	#-1,d0			; Success ?
	bne.s	.Ok
	move.l	#GET_SPRITE_ERROR,d0	; Yes -> Exit
	jmp	Fatal_error
.Ok:	move.w	d0,Sprite_2_nr(a6)		; Yes
.No:	LOCAL
; ---------- Set position -------------------------
	move.w	d4,d0			; Get coordinates
	move.w	d5,d1
	sub.l	a0,a0			; Move
	move.l	Sprite_1(a6),a1
	kickGFX	MoveSprite
	move.l	d4,d0			; Get coordinates
	move.l	d5,d1
	sub.l	a0,a0			; Move
	move.l	Sprite_2(a6),a1
	kickGFX	MoveSprite
	cmp.w	#39,Kickstart_version	; V39 ?
	bne.s	.No
	move.l	Sprite_2(a6),a0		; Yes -> Fix bug
	move.l	ss_posctldata(a0),a0
	move.w	d6,d0
	lsr.w	#3,d0
	bset	#7,1(a0,d0.w)
.No:	movem.l	(sp)+,d0-d7/a0-a3/a6
	rts

;***************************************************************************
; [ Free a hardware sprite ]
;   IN : a0 - Pointer to sprite data (.l)
; All registers are restored
;***************************************************************************
Exit_sprite:
	movem.l	d0/d1/a0-a2/a6,-(sp)
	move.l	a0,a6
	move.w	Sprite_1_nr(a6),d0		; Get 1st sprite number
	cmp.w	#-1,d0			; Any ?
	beq.s	.No
	ext.l	d0			; Yes -> Free sprite
	kickGFX	FreeSprite
	move.w	#-1,Sprite_1_nr(a6)
.No:	LOCAL
	move.w	Sprite_2_nr(a6),d0		; Get 2nd sprite number
	cmp.w	#-1,d0			; Any ?
	beq.s	.No
	ext.l	d0			; Yes -> Free sprite
	kickGFX	FreeSprite
	move.w	#-1,Sprite_2_nr(a6)
.No:	LOCAL
	move.l	Sprite_1(a6),d0		; 1st sprite allocated ?
	beq.s	.No
	move.l	d0,a2			; Yes -> Free it
	kickGFX	FreeSpriteData
	clr.l	Sprite_1(a6)
.No:	LOCAL
	move.l	Sprite_2(a6),d0		; 2nd sprite allocated ?
	beq.s	.No
	move.l	d0,a2			; Yes -> Free it
	kickGFX	FreeSpriteData
	clr.l	Sprite_2(a6)
.No:	LOCAL
	movem.l	(sp)+,d0/d1/a0-a2/a6
	rts

;***************************************************************************
; [ Change a hardware sprite's image ]
;   IN : a0 - Pointer to sprite data (.l)
;        a1 - Pointer to new graphics (.l)
; All registers are restored
;***************************************************************************
Change_sprite_image:
	movem.l	d0/d1/a0-a3/a6,-(sp)
	move.l	a0,a6			; Save
	move.l	Sprite_bitmap(a6),a0	; Put graphics in BitMap
	jsr	Copy_to_bitmap
	move.l	#SPRITEA_Width,d0		; Insert width in tag lists
	moveq.l	#0,d1
	move.w	Sprite_width(a6),d1
	lea.l	Alloc_sprite_A_tag_items,a0
	jsr	Set_tag_data
	lea.l	Alloc_sprite_B_tag_items,a0
	jsr	Set_tag_data
	lea.l	Alloc_sprite_A_tag_items,a1	; Allocate 1st sprite
	move.l	Sprite_bitmap(a6),a2
	kickGFX	AllocSpriteDataA
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#ALLOC_SPRITE_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,Temp_sprite_1		; Yes
	LOCAL
	lea.l	Alloc_sprite_B_tag_items,a1	; Allocate 2nd sprite
	move.l	Sprite_bitmap(a6),a2
	kickGFX	AllocSpriteDataA
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	Temp_sprite_1,a2		; No -> Free 1st sprite
	kickGFX	FreeSpriteData
	move.l	#ALLOC_SPRITE_ERROR,d0	; Exit
	jmp	Fatal_error
.Ok:	move.l	d0,Temp_sprite_2		; Yes
	LOCAL
	move.l	My_screen,a0		; Change images
	lea.l	sc_ViewPort(a0),a0
	move.l	Sprite_1(a6),a1
	move.l	Temp_sprite_1,a2
	lea.l	Empty_tags,a3
	kickGFX	ChangeExtSpriteA
	move.l	My_screen,a0
	lea.l	sc_ViewPort(a0),a0
	move.l	Sprite_2(a6),a1
	move.l	Temp_sprite_2,a2
	lea.l	Empty_tags,a3
	kickGFX	ChangeExtSpriteA
	cmp.w	#39,Kickstart_version	; V39 ?
	bne.s	.No
	move.l	Temp_sprite_2,a0		; Yes -> Fix bug
	move.l	ss_posctldata(a0),a0
	move.w	Sprite_width(a6),d0
	lsr.w	#3,d0
	bset	#7,1(a0,d0.w)
.No:	move.l	Sprite_1(a6),a2		; Destroy old sprites
	kickGFX	FreeSpriteData		;  & insert new ones
	move.l	Temp_sprite_1,Sprite_1(a6)
	move.l	Sprite_2(a6),a2
	kickGFX	FreeSpriteData
	move.l	Temp_sprite_2,Sprite_2(a6)
	movem.l	(sp)+,d0/d1/a0-a3/a6
	rts

;***************************************************************************
; [ Prepare bitmap ]
;   IN : d5 - Depth of bitmap in planes (.w)
;        d6 - Width of bitmap in pixels (.w)
;        d7 - Height of bitmap in pixels (.w)
;        a0 - Pointer to empty BitMap structure (.l)
;        a1 - Pointer to graphics buffer (.l)
; All registers are restored
;***************************************************************************
Prepare_bitmap:
	movem.l	d0-d2/d5/a0/a1,-(sp)
	movem.l	a0/a1,-(sp)		; Initialize bitmap
	move.w	d5,d0
	move.w	d6,d1
	move.w	d7,d2
	kickGFX	InitBitMap
	movem.l	(sp)+,a0/a1
	lea.l	bm_Planes(a0),a0		; Set plane pointers
	move.w	d6,d0
	lsr.w	#3,d0
	mulu.w	d7,d0
	subq.w	#1,d5
.Loop:	move.l	a1,(a0)+
	add.w	d0,a1
	dbra	d5,.Loop
	movem.l	(sp)+,d0-d2/d5/a0/a1
	rts

;***************************************************************************
; [ Copy graphics to bitmap ]
;   IN : a0 - Pointer to prepared (!) BitMap structure (.l)
;        a1 - Pointer to graphics (.l)
; All registers are restored
; Notes :
;   - This will only work for non-interleaved BitMaps.
;***************************************************************************
Copy_to_bitmap:
	movem.l	d0/d5-d7/a0-a4,-(sp)
	moveq.l	#0,d5			; Get depth
	move.b	bm_Depth(a0),d5
	subq.w	#1,d5			; -1
	move.w	d5,d0			; Calculate offset
	mulu.w	bm_BytesPerRow(a0),d0
	lea.l	bm_Planes(a0),a2		; Copy
.Loop_Z:	move.l	(a2)+,a3
	move.l	a1,a4
	move.w	bm_Rows(a0),d7
	subq.w	#1,d7
.Loop_Y:	move.w	bm_BytesPerRow(a0),d6
	lsr.w	#1,d6
	subq.w	#1,d6
.Loop_X:	move.w	(a4)+,(a3)+
	dbra	d6,.Loop_X
	add.w	d0,a4
	dbra	d7,.Loop_Y
	add.w	bm_BytesPerRow(a0),a1
	dbra	d5,.Loop_Z
	movem.l	(sp)+,d0/d5-d7/a0-a4
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************
	SECTION	Fast_DATA,data
Suppress_mouse:	dc.b -1
	even
Default_MA:
	dc.w 0,Screen_width-1
	dc.w 0,Screen_height-1

Videocontrol_tag_items:
	dc.l VTAG_SPODD_BASE_SET,0	; Will be inserted
	dc.l VTAG_SPEVEN_BASE_SET,0	; Will be inserted
	dc.l VTAG_IMMEDIATE,Video_control_flag
	dc.l TAG_DONE,0

Alloc_sprite_A_tag_items:
	dc.l SPRITEA_Width,0	; Will be inserted
	dc.l TAG_DONE,0

Alloc_sprite_B_tag_items:
	dc.l SPRITEA_Width,0	; Will be inserted
	dc.l SPRITEA_Attached,TRUE
	dc.l TAG_DONE,0

Get_sprite_A_tag_items_V39:
	dc.l GSTAG_SPRITE_NUM,0	; Will be inserted
	dc.l TAG_DONE,0

Get_sprite_A_tag_items_V40:
	dc.l GSTAG_SPRITE_NUM,0	; Will be inserted
	dc.l GSTAG_ATTACHED,0	; Will be inserted
	dc.l TAG_DONE,0

Get_sprite_B_tag_items:
	dc.l GSTAG_SPRITE_NUM,0	; Will be inserted
	dc.l TAG_DONE,0


	SECTION	Fast_BSS,bss
Sprites_created:	ds.b 1
Switch_to_busy:	ds.b 1
Mouse_sprite:	ds.b Sprite_data_size
Temp_sprite_1:	ds.l 1
Temp_sprite_2:	ds.l 1

Video_control_flag:	ds.l 1
Mouse_bitmap:
	ds.b bm_data_size
Mouse_buffer:
	ds.w 2*(Mptr_width/8)*Mptr_height

HotSpot_X:	ds.w 1
HotSpot_Y:	ds.w 1

Mptr_Sp:	ds.l 1			; Mouse pointer stack
MptrStack_start:
	ds.l Max_Mptr
MptrStack_end:

Busy_timer:	ds.w 1
Busy_Mptr:	ds.l 1
	
MA_Sp:	ds.l 1			; MA stack
MAStack_start:
	ds.l Max_MA
MAStack_end:
