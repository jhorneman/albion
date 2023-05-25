; Hardware sprite management
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994


;*****************************************************************************
; These are the offsets for the Sprite structure
	rsreset
Sprite_on:	rs.b 1			; Flag
	rseven
Sprite_1_nr:	rs.l 1		; Hardware sprite numbers
Sprite_2_nr:	rs.l 1
Sprite_1:	rs.l 1			; Pointers to ExtSprite structures
Sprite_2:	rs.l 1
Sprite_data_size:	rs.b 0


	MODULE	DDT_Sprites

	OUTPUT	DDT:Objects/Core/Sprites.o

	OPT	O1+,O2+,O4+,O5+,O10+,O11+,OW-
	OPT	O7+,EVEN,USER,INCONCE
	OPT	P=68020


	XREF	Graphics_base
	XREF	My_screen
	XREF	Kickstart_version
	XREF	Set_tag_data
	XREF	Fatal_error

	XDEF	Set_sprite_image
	XDEF	Set_sprite_position
	XDEF	Add_sprite
	XDEF	Remove_sprite
	XDEF	Remove_all_sprites


	incdir	DDT:
	include	Constants/Global.i
	include	Constants/OS/OS.i
	include	Constants/Core.i


	SECTION	Program,code
;***************************************************************************
; [ Set a sprite's position ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Sprite number {1...Max_sprites} (.w)
; All registers are restored
;***************************************************************************
Set_sprite_position:
	movem.l	d0-d2/a0/a1,-(sp)
	cmp.w	#Max_sprites,d2		; Legal ?
	bgt.s	.Exit
	lea.l	Sprite_list,a0		; Yes -> Get data
	subq.w	#1,d2
	mulu.w	#Sprite_data_size,d2
	add.w	d2,a0
	tst.b	Sprite_on(a0)		; On ?
	beq.s	.Exit
	ext.l	d0			; Yes
	ext.l	d1
	movem.l	d0/d1/a0,-(sp)
	move.l	Sprite_1(a0),a1
	sub.l	a0,a0
	kickGFX	MoveSprite
	movem.l	(sp)+,d0/d1/a0

;	add.l	#17,d0

	move.l	Sprite_2(a0),a1
	sub.l	a0,a0
	kickGFX	MoveSprite
.Exit:	movem.l	(sp)+,d0-d2/a0/a1
	rts

;***************************************************************************
; [ Set a sprite's image ]
;   IN : d0 - Sprite number {1...Max_sprites} (.w)
;        a0 - Pointer to image (.l)
; All registers are restored
;***************************************************************************
Set_sprite_image:
	rts

	movem.l	d0/a0/a1,-(sp)
	cmp.w	#Max_sprites,d0		; Legal ?
	bgt.s	.Exit
	move.l	a0,a1			; Yes -> Save
	lea.l	Sprite_list,a0		; Get data
	subq.w	#1,d0
	mulu.w	#Sprite_data_size,d0
	add.w	d0,a0
	tst.b	Sprite_on(a0)		; On ?
	beq.s	.Exit
	nop
;	jsr	Exit_sprite		; Yes
;	move.l	a1,Sprite_image(a0)
;	jsr	Init_sprite
.Exit:	movem.l	(sp)+,d0/a0/a1
	rts

;***************************************************************************
; [ Add a hardware sprite ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d6 - Width of sprite in truncs (.w)
;        d7 - Height of sprite in pixels (.w)
;        a0 - Pointer to image (.l)
;  OUT : d0 - Sprite number {1...Max_sprites} (.w)
; Changed registers : d0
;***************************************************************************
Add_sprite:
	movem.l	d2/d3/a0/a1,-(sp)
	lea.l	Sprite_list,a1		; Search free entry
	moveq.l	#1,d2
	moveq.l	#Max_sprites-1,d3
.Loop:	tst.b	Sprite_on(a1)		; Free ?
	bne.s	.Next
	move.l	a1,a0			; Initialize sprite
	jsr	Init_sprite
	move.w	d2,d0			; Output number
	bra.s	.Exit
.Next:	addq.w	#1,d2			; Next
	lea.l	Sprite_data_size(a1),a1
	dbra	d3,.Loop
	; NO FREE ENTRY !
.Exit:	movem.l	(sp)+,d2/d3/a0/a1
	rts

;***************************************************************************
; [ Remove a sprite ]
;   IN : d0 - Sprite number {1...Max_sprites} (.w)
; All registers are restored
;***************************************************************************
Remove_sprite:
	movem.l	d0/a0,-(sp)
	cmp.w	#Max_sprites,d0		; Legal ?
	bgt.s	.Exit
	lea.l	Sprite_list,a0		; Yes -> Get data
	subq.w	#1,d0
	mulu.w	#Sprite_data_size,d0
	add.w	d0,a0
	tst.b	Sprite_on(a0)		; On ?
	beq.s	.Exit
	jsr	Exit_sprite		; Yes -> Remove
.Exit:	movem.l	(sp)+,d0/a0
	rts

;***************************************************************************
; [ Remove all sprites ]
; All registers are restored
;***************************************************************************
Remove_all_sprites:
	movem.l	d7/a0,-(sp)
	lea.l	Sprite_list,a0
	moveq.l	#Max_sprites-1,d7
.Loop:	tst.b	Sprite_on(a0)		; Sprite ?
	beq.s	.Next
	jsr	Exit_sprite		; Yes -> Remove
.Next:	lea.l	Sprite_data_size(a0),a0	; Next
	dbra	d7,.Loop
	movem.l	(sp)+,d7/a0
	rts

;***************************************************************************
; [ Initialize a hardware sprite ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d6 - Width of sprite in truncs (.w)
;        d7 - Height of sprite in pixels (.w)
;        a0 - Pointer to sprite data (.l)
;        a1 - Pointer to image (.l)
; All registers are restored
;***************************************************************************
Init_sprite:
	movem.l	d0-d7/a0-a2/a6,-(sp)

	lea.l	Grr_image,a1

	ext.l	d0			; Extend
	ext.l	d1
	move.w	d0,d4			; Save
	move.w	d1,d5
	move.l	a0,a6
	sf	Sprite_on(a6)		; Off
	clr.l	Sprite_1(a6)		; Clear data
	clr.l	Sprite_2(a6)
	move.l	#-1,Sprite_1_nr(a6)
	move.l	#-1,Sprite_2_nr(a6)
; ---------- Create BitMap ------------------------
	move.l	a1,-(sp)			; Initialize
	lea.l	Sprite_BitMap,a0
	moveq.l	#4,d0
	move.w	d6,d1
	lsl.w	#4,d1
	move.w	d7,d2
	kickGFX	InitBitMap
	move.l	(sp)+,a1
	lea.l	Sprite_BitMap,a0		; Set plane pointers
	move.l	a1,bm_Planes(a0)
	move.l	a1,bm_Planes+4(a0)
	move.l	a1,bm_Planes+8(a0)
	move.l	a1,bm_Planes+12(a0)
; ---------- Allocate sprites ---------------------
	lea.l	Alloc_sprite_A_tag_items,a1	; Allocate 1st sprite
	lea.l	Sprite_BitMap,a2
	kickGFX	AllocSpriteDataA
	tst.l	d0			; Success ?
	beq	.Exit
	move.l	d0,Sprite_1(a6)		; Yes
	lea.l	Alloc_sprite_B_tag_items,a1	; Allocate 2nd sprite
	lea.l	Sprite_BitMap,a2
	kickGFX	AllocSpriteDataA
	tst.l	d0			; Success ?
	beq	.Exit
	move.l	d0,Sprite_2(a6)		; Yes
	cmp.w	#39,Kickstart_version	; V39 ?
	bne.s	.No1
	move.l	Sprite_2(a6),a0		; Yes -> Fix bug
	move.w	2(a0),d0
	bset	#7,d0
	move.w	d0,2(a0)
; ---------- Set sprites --------------------------
.No1:
;	lea.l	Get_sprite_A_tag_items,a0	; Insert 2nd sprite pointer
;	move.l	#GSTAG_ATTACHED,d0		;  in first tag list
;	move.l	Sprite_2(a6),d1
;	jsr	Set_tag_data

	lea.l	Get_sprite_A_tag_items,a1	; Set 1st sprite
	move.l	Sprite_1(a6),a2
	kickGFX	GetExtSpriteA
	cmp.l	#-1,d0			; Success ?
	beq.s	.Exit
	move.l	d0,Sprite_1_nr(a6)		; Yes

	cmp.w	#39,Kickstart_version	; V39 ?
	bne.s	.No2

;	lea.l	Get_sprite_B_tag_items,a0	; Yes
;	move.l	#GSTAG_SPRITE_NUM,d0
;	move.l	Sprite_1_nr(a6),d1
;	addq.l	#1,d1
;	jsr	Set_tag_data

	lea.l	Get_sprite_B_tag_items,a1	; Yes -> Set 2nd sprite
	move.l	Sprite_2(a6),a2
	kickGFX	GetExtSpriteA
	cmp.l	#-1,d0			; Success ?
	beq.s	.Exit
	move.l	d0,Sprite_2_nr(a6)		; Yes

.No2:	st	Sprite_on(a6)		; On
; ---------- Set position -------------------------
	move.l	d4,d0			; Get coordinates
	move.l	d5,d1
	sub.l	a0,a0			; Move
	move.l	Sprite_1(a6),a1
	kickGFX	MoveSprite
	move.l	d4,d0			; Get coordinates
	move.l	d5,d1

;	add.l	#17,d0

	sub.l	a0,a0			; Move
	move.l	Sprite_2(a6),a1
	kickGFX	MoveSprite
.Exit:	movem.l	(sp)+,d0-d7/a0-a2/a6
	rts

;***************************************************************************
; [ Free a hardware sprite ]
;   IN : a0 - Pointer to sprite data (.l)
; All registers are restored
;***************************************************************************
Exit_sprite:
	movem.l	d0/d1/a0-a2/a6,-(sp)
	move.l	a0,a6			; Save
	move.l	Sprite_1_nr(a6),d0		; Get 1st sprite number
	cmp.l	#-1,d0			; Any ?
	beq.s	.No
	kickGFX	FreeSprite		; Yes -> Free sprite
	move.l	#-1,Sprite_1_nr(a6)
.No:	LOCAL
	move.l	Sprite_2_nr(a6),d0		; Get 2nd sprite number
	cmp.l	#-1,d0			; Any ?
	beq.s	.No
	kickGFX	FreeSprite		; Yes -> Free sprite
	move.l	#-1,Sprite_2_nr(a6)
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
	sf	Sprite_on(a6)		; Off
	movem.l	(sp)+,d0/d1/a0-a2/a6
	rts

;***************************************************************************	
; The DATA & BSS segments
;***************************************************************************
	SECTION	Fast_DATA,data
Grr_image:
;	dcb.l 8,$0000ffff
;	rept 4
;	dc.l 0,$ffffffff
;	endr
;	rept 2
;	dc.l 0,0
;	dc.l $ffffffff,$ffffffff
;	endr
;	dc.l 0,0,0,0
;	dc.l $ffffffff,$ffffffff,$ffffffff,$ffffffff

Alloc_sprite_A_tag_items:
	dc.l TAG_DONE,0

Alloc_sprite_B_tag_items:
	dc.l SPRITEA_Attached,0
	dc.l TAG_DONE,0

Get_sprite_A_tag_items:
	dc.l GSTAG_SPRITE_NUM,2
;	dc.l GSTAG_ATTACHED,0
	dc.l TAG_DONE,0

Get_sprite_B_tag_items:
	dc.l GSTAG_SPRITE_NUM,3
	dc.l TAG_DONE,0


	SECTION	Fast_BSS,bss
Sprite_BitMap:
	ds.b bm_data_size
Sprite_list:
	ds.b Max_sprites*Sprite_data_size
