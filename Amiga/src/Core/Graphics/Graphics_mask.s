; Graphics routines
; Mask calculations
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 18-2-1994

	SECTION	Program,code
;***************************************************************************
; [ Calculate mask / any planes ]
;   IN : d5 - Number of planes {1...Screen_depth} (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to graphics (.l)
;        a6 - Pointer to custom chip register base (.l)
;  OUT : a2 - Pointer to mask (.l)
; Changed registers : a2
; NOTE :
;  - I assume the blitter is owned.
;  - This routine will allocate memory for the mask if the mask size is
;    larger than {Mask_buffer_size}.
;***************************************************************************
Calculate_mask:
	cmp.w	#Screen_depth,d5		; All planes ?
	beq	Calculate_mask_all
	cmp.w	#1,d5			; Only one ?
	bne.s	.More
	move.l	a0,a2			; Yes -> Graphics is mask
	rts
.More:	movem.l	d0/d1/d5-d7/a0/a6,-(sp)
	move.w	d6,d0			; Calculate mask buffer size
	add.w	d0,d0
	mulu.w	d7,d0
	cmp.l	#Mask_buffer_size,d0	; Too large ?
	bls.s	.No
	jsr	Allocate_CHIP		; Yes -> Allocate
	move.b	d0,Mask_buffer_handle
	jsr	Claim_pointer
	move.l	d0,a2
	bra.s	.Go_on
.No:	lea.l	Chip_mask_buffer,a2		; No -> Use solid buffer
	clr.b	Mask_buffer_handle
.Go_on:	Wait_4_blitter			; Wait
	move.l	#-1,bltafwm(a6)		; Set mask
	lsl.w	#6,d7			; Calculate blit size
	add.w	d6,d7
	add.w	d6,d6			; Width x 2
	move.w	d6,d0			; Set modulo A
	mulu.w	d5,d0			;  = (W x 2) x (D - 1)
	sub.w	d6,d0
	move.w	d0,bltamod(a6)
	move.w	#0,bltbmod(a6)		; Set modulo B & D
	move.w	#0,bltdmod(a6)
	move.l	#$09f00000,bltcon0(a6)	; First plane is copied
	move.l	a0,bltapt(a6)
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)
	add.w	d6,a0			; Next plane
	subq.w	#2,d5			; Next planes are OR-ed
	bmi.s	.Exit
	Wait_4_blitter
	move.w	#$0dfc,bltcon0(a6)		; Set blitter control
.Loop:	Wait_4_blitter
	move.l	a0,bltapt(a6)		; Set pointers
	move.l	a2,bltbpt(a6)
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	add.w	d6,a0			; Next plane
	dbra	d5,.Loop
.Exit:	movem.l	(sp)+,d0/d1/d5-d7/a0/a6
	rts

;***************************************************************************
; [ Calculate mask / all planes ]
;   IN : d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to graphics (.l)
;        a6 - Pointer to custom chip register base (.l)
;  OUT : a2 - Pointer to mask (.l)
; Changed registers : a2
; NOTE :
;  - I assume the blitter is owned.
;  - This routine will allocate memory for the mask if the mask size is
;    larger than {Mask_buffer_size}.
;***************************************************************************
Calculate_mask_all:
	movem.l	d0/d1/d6/d7/a0/a6,-(sp)
	move.w	d6,d0			; Calculate mask buffer size
	add.w	d0,d0
	mulu.w	d7,d0
	cmp.l	#Mask_buffer_size,d0	; Too large ?
	bls.s	.No
	jsr	Allocate_CHIP		; Yes -> Allocate
	move.b	d0,Mask_buffer_handle
	jsr	Claim_pointer
	move.l	d0,a2
	bra.s	.Go_on
.No:	lea.l	Chip_mask_buffer,a2		; No -> Use solid buffer
	clr.b	Mask_buffer_handle
.Go_on:	Wait_4_blitter			; Wait
	move.l	#-1,bltafwm(a6)		; Set mask
	lsl.w	#6,d7			; Calculate blit size
	add.w	d6,d7
	add.w	d6,d6			; Width x 2
	move.w	d6,d0			; Set modulo A,B & C
	mulu.w	#Screen_depth-1,d0		;  = (W x 2) x (D - 1)
	move.w	d0,bltamod(a6)
	move.w	d0,bltbmod(a6)
	move.w	d0,bltcmod(a6)
	move.w	#0,bltdmod(a6)		; Set modulo D
	move.l	#$0ffe0000,bltcon0(a6)	; Do first three planes
	move.l	a0,bltapt(a6)
	add.w	d6,a0
	move.l	a0,bltbpt(a6)
	add.w	d6,a0
	move.l	a0,bltcpt(a6)
	add.w	d6,a0
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)
	Wait_4_blitter
	move.w	#0,bltcmod(a6)		; Set modulo C
	move.l	a0,bltapt(a6)		; Do fourth & fifth planes
	add.w	d6,a0
	move.l	a0,bltbpt(a6)
	add.w	d6,a0
	move.l	a2,bltcpt(a6)
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)
	Wait_4_blitter
	move.l	a0,bltapt(a6)		; Do sixth & seventh planes
	add.w	d6,a0
	move.l	a0,bltbpt(a6)
	add.w	d6,a0
	move.l	a2,bltcpt(a6)
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)
	Wait_4_blitter
	move.w	#$0bfa,bltcon0(a6)		; Do eighth plane
	move.l	a0,bltapt(a6)
	move.l	a2,bltcpt(a6)
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)
.Exit:	movem.l	(sp)+,d0/d1/d6/d7/a0/a6
	rts

;***************************************************************************
; [ Calculate mask / CPU / any planes ]
;   IN : d5 - Number of planes {1...Screen_depth} (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to graphics (.l)
;        a6 - Pointer to custom chip register base (.l)
;  OUT : a2 - Pointer to mask (.l)
; Changed registers : d0
; NOTE :
;  - I assume the blitter is owned.
;  - This routine will allocate memory for the mask if the mask size is
;    larger than {Mask_buffer_size}.
;***************************************************************************
Calculate_mask_CPU:
	cmp.w	#Screen_depth,d5		; All planes ?
	beq	Calculate_mask_all_CPU
	cmp.w	#1,d5			; Only one ?
	bne.s	.More
	move.l	a0,a2			; Yes -> Graphics is mask
	rts
.More:	movem.l	d0-d7/a0/a1,-(sp)
	Wait_4_blitter
	move.w	d6,d0			; Calculate mask buffer size
	add.w	d0,d0
	mulu.w	d7,d0
	cmp.l	#Mask_buffer_size,d0	; Too large ?
	bls.s	.No
	jsr	Allocate_memory		; Yes -> Allocate
	move.b	d0,Mask_buffer_handle
	jsr	Claim_pointer
	move.l	d0,a2
	bra.s	.Go_on
.No:	lea.l	Fast_mask_buffer,a2		; No -> Use solid buffer
	clr.b	Mask_buffer_handle
.Go_on:	move.l	a2,-(sp)			; Save
	move.w	d6,d2			; Calculate variables
	add.w	d2,d2
	move.w	d5,d3
	subq.w	#1,d3
	mulu.w	d2,d3
	subq.w	#1,d5
	btst	#0,d6			; Odd or even ?
	bne	.Odd
	lsr.w	#1,d6			; Even number of truncs
	subq.w	#1,d7
.Loop_Y1:	move.w	d6,d4			; Do one line
.Loop_X1:	move.l	a0,a1
	move.l	(a0)+,d0
	move.w	d5,d1
	bra.s	.Entry_Z1
.Loop_Z1:	add.w	d2,a1
	or.l	(a1),d0
.Entry_Z1:	dbra	d1,.Loop_Z1
	move.l	d0,(a2)+
	dbra	d4,.Loop_X1
	add.w	d3,a0			; Next line
	dbra	d7,.Loop_Y1
	bra.s	.Done
.Odd:	lsr.w	#1,d6			; Odd number of truncs
	subq.w	#1,d7
.Loop_Y2:	move.l	a0,a1			; Do first trunc
	move.w	(a0)+,d0
	move.w	d5,d1
	bra.s	.Entry_Z2
.Loop_Z2:	add.w	d2,a1
	or.w	(a1),d0
.Entry_Z2:	dbra	d1,.Loop_Z2
	move.w	d0,(a2)+
	move.w	d6,d4			; Do rest of line
.Loop_X2:	move.l	a0,a1
	move.l	(a0)+,d0
	move.w	d5,d1
	bra.s	.Entry_Z3
.Loop_Z3:	add.w	d2,a1
	or.l	(a1),d0
.Entry_Z3:	dbra	d1,.Loop_Z3
	move.l	d0,(a2)+
	dbra	d4,.Loop_X2
	add.w	d3,a0			; Next line
	dbra	d7,.Loop_Y2
.Done:	move.l	(sp)+,a2			; Output
.Exit:	movem.l	(sp)+,d0-d7/a0/a1
	rts

;***************************************************************************
; [ Calculate mask / CPU / all planes ]
;   IN : d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to graphics (.l)
;        a6 - Pointer to custom chip register base (.l)
;  OUT : a2 - Pointer to mask (.l)
; Changed registers : d0
; NOTE :
;  - I assume the blitter is owned.
;  - This routine will allocate memory for the mask if the mask size is
;    larger than {Mask_buffer_size}.
;***************************************************************************
Calculate_mask_all_CPU:
	movem.l	d0-d4/d6/d7/a0/a1,-(sp)
	Wait_4_blitter
	move.w	d6,d0			; Calculate mask buffer size
	add.w	d0,d0
	mulu.w	d7,d0
	cmp.l	#Mask_buffer_size,d0	; Too large ?
	bls.s	.No
	jsr	Allocate_memory		; Yes -> Allocate
	move.b	d0,Mask_buffer_handle
	jsr	Claim_pointer
	move.l	d0,a2
	bra.s	.Go_on
.No:	lea.l	Fast_mask_buffer,a2		; No -> Use solid buffer
	clr.b	Mask_buffer_handle
.Go_on:	move.l	a2,-(sp)			; Save
	move.w	d6,d2			; Calculate variables
	add.w	d2,d2
	move.w	d2,d3
	mulu.w	#Screen_depth-1,d3
	btst	#0,d6			; Odd or even ?
	bne	.Odd
	lsr.w	#1,d6			; Even number of truncs
	subq.w	#1,d6
	subq.w	#1,d7
.Loop_Y1:	move.w	d6,d4			; Do one line
.Loop_X1:	move.l	a0,a1
	move.l	(a0)+,d0
	rept	Screen_depth-1
	add.w	d2,a1
	or.l	(a1),d0
	endr
	move.l	d0,(a2)+
	dbra	d4,.Loop_X1
	add.w	d3,a0			; Next line
	dbra	d7,.Loop_Y1
	bra.s	.Done
.Odd:	lsr.w	#1,d6			; Odd number of truncs
	subq.w	#1,d7
.Loop_Y2:	move.l	a0,a1			; Do first trunc
	move.w	(a0)+,d0
	rept	Screen_depth-1
	add.w	d2,a1
	or.w	(a1),d0
	endr
	move.w	d0,(a2)+
	move.w	d6,d4			; Do rest of line
	bra.s	.Entry_X2
.Loop_X2:	move.l	a0,a1
	move.l	(a0)+,d0
	rept	Screen_depth-1
	add.w	d2,a1
	or.l	(a1),d0
	endr
	move.l	d0,(a2)+
.Entry_X2:	dbra	d4,.Loop_X2
	add.w	d3,a0			; Next line
	dbra	d7,.Loop_Y2
.Done:	move.l	(sp)+,a2			; Output
.Exit:	movem.l	(sp)+,d0-d4/d6/d7/a0/a1
	rts

;***************************************************************************
; [ Calculate small mask / any planes ]
;   IN : d5 - Number of planes {0...Screen_depth} (.w)
;        a0 - Pointer to graphics (.l)
;        a6 - Pointer to custom chip register base (.l)
;  OUT : a2 - Pointer to mask (.l)
; Changed registers : a2
; NOTE :
;  - I assume the blitter is owned.
;***************************************************************************
Calculate_small_mask:
	cmp.w	#Screen_depth,d5		; All planes ?
	beq	Calculate_small_mask_all
	cmp.w	#1,d5			; Only one ?
	bne.s	.More
	move.l	a0,a2			; Yes -> Graphics is mask
	rts
.More:	movem.l	d0/d5/d7/a0,-(sp)
	lea.l	Chip_mask_buffer,a2
	Wait_4_blitter			; Wait
	move.l	#-1,bltafwm(a6)		; Set blitter mask
	move.w	#16*64+1,d7		; Calculate blit size
	move.w	d5,d0			; Set modulo A
	subq.w	#1,d0
	add.w	d0,d0
	move.w	d0,bltamod(a6)
	move.w	#0,bltbmod(a6)		; Set modulo B & D
	move.w	#0,bltdmod(a6)
	move.l	#$09f00000,bltcon0(a6)	; First plane is copied
	Wait_4_blitter
	move.l	a0,bltapt(a6)
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)
	subq.w	#2,d5			; Next planes are OR-ed
	bmi.s	.Exit
	Wait_4_blitter
	addq.l	#2,a0			; Next plane
	move.w	#$0dfc,bltcon0(a6)		; Set blitter control
	bra.s	.Entry
.Loop:	Wait_4_blitter
.Entry:	move.l	a0,bltapt(a6)		; Set pointers
	move.l	a2,bltbpt(a6)
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	addq.l	#2,a0			; Next plane
	dbra	d5,.Loop
.Exit:	movem.l	(sp)+,d0/d5/d7/a0
	rts

;***************************************************************************
; [ Calculate small mask / all planes ]
;   IN : a0 - Pointer to graphics (.l)
;        a6 - Pointer to custom chip register base (.l)
;  OUT : a2 - Pointer to mask (.l)
; Changed registers : a2
; NOTE :
;  - I assume the blitter is owned.
;***************************************************************************
Calculate_small_mask_all:
	movem.l	d0/d7/a0,-(sp)
	lea.l	Chip_mask_buffer,a2
	Wait_4_blitter			; Wait
	move.l	#-1,bltafwm(a6)		; Set mask
	move.w	#16*64+1,d7		; "Calculate" blit size
	move.w	#Screen_depth,d0		; Set modulo A,B & C
	move.w	d0,bltamod(a6)
	move.w	d0,bltbmod(a6)
	move.w	d0,bltcmod(a6)
	move.w	#0,bltdmod(a6)		; Set modulo D
	move.l	#$0ffe0000,bltcon0(a6)	; Do first three planes
	move.l	a0,bltapt(a6)
	addq.l	#2,a0
	move.l	a0,bltbpt(a6)
	addq.l	#2,a0
	move.l	a0,bltcpt(a6)
	addq.l	#2,a0
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)
	Wait_4_blitter
	move.w	#0,bltcmod(a6)		; Set modulo C
	move.l	a0,bltapt(a6)		; Do fourth & fifth planes
	addq.l	#2,a0
	move.l	a0,bltbpt(a6)
	addq.l	#2,a0
	move.l	a2,bltcpt(a6)
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)
	Wait_4_blitter
	move.l	a0,bltapt(a6)		; Do sixth & seventh planes
	addq.l	#2,a0
	move.l	a0,bltbpt(a6)
	addq.l	#2,a0
	move.l	a2,bltcpt(a6)
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)
	Wait_4_blitter
	move.w	#$0bfa,bltcon0(a6)		; Do eighth plane
	move.l	a0,bltapt(a6)
	move.l	a2,bltcpt(a6)
	move.l	a2,bltdpt(a6)
	move.w	d7,bltsize(a6)
.Exit:	movem.l	(sp)+,d0/d7/a0
	rts

;***************************************************************************
; [ Calculate small mask / any planes / CPU ]
;   IN : d5 - Number of planes {0...Screen_depth} (.w)
;        a0 - Pointer to graphics (.l)
;        a6 - Pointer to custom chip register base (.l)
;  OUT : a2 - Pointer to mask (.l)
; Changed registers : a2
; NOTE :
;  - I assume the blitter is owned.
;***************************************************************************
Calculate_small_mask_CPU:
	cmp.w	#Screen_depth,d5		; All planes ?
	beq	Calculate_small_mask_all_CPU
	cmp.w	#1,d5			; Only one ?
	bne.s	.More
	move.l	a0,a2			; Yes -> Graphics is mask
	rts
.More:	movem.l	d0/d1/d5/d7/a0,-(sp)
	Wait_4_blitter
	lea.l	Fast_mask_buffer,a2		; Just do it
	subq.w	#1,d5
	moveq.l	#16-1,d7
.Loop_Y:	move.w	(a0)+,d0
	move.w	d5,d1
	bra.s	.Entry_Z
.Loop_Z:	or.w	(a0)+,d0
.Entry_Z:	dbra	d1,.Loop_Z
	move.w	d0,(a2)+
	dbra	d7,.Loop_Y
	lea.l	Fast_mask_buffer,a2		; Output
	movem.l	(sp)+,d0/d1/d5/d7/a0
	rts

;***************************************************************************
; [ Calculate small mask / all planes / CPU ]
;   IN : a0 - Pointer to graphics (.l)
;        a6 - Pointer to custom chip register base (.l)
;  OUT : a2 - Pointer to mask (.l)
; Changed registers : a2
; NOTE :
;  - I assume the blitter is owned.
;  - This routine will only work if {Screen_depth} is even.
;***************************************************************************
Calculate_small_mask_all_CPU:
	movem.l	d0/d7/a0,-(sp)
	Wait_4_blitter
	lea.l	Fast_mask_buffer,a2		; Just do it
	moveq.l	#16-1,d7
.Loop_Y:	move.l	(a0)+,d0
	rept	(Screen_depth/2)-1
	or.l	(a0)+,d0
	endr
	or.w	d0,(a2)
	swap	d0
	move.w	d0,(a2)+
	dbra	d7,.Loop_Y
	lea.l	Fast_mask_buffer,a2		; Output
	movem.l	(sp)+,d0/d7/a0
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************	
	SECTION	Chip_BSS,bss_c
Fast_mask_buffer:
Chip_mask_buffer:	ds.w Mask_buffer_size/2


	SECTION	Fast_BSS,bss
Mask_buffer_handle:	ds.b 1
	even
