; Animation routines
; Written by J.Horneman (In Tune With The Universe)
; Start : 25-3-1994

	SECTION	Program,code
;*****************************************************************************
; [ Update animation arrays ]
; All registers are restored
; Notes :
;   - Length 1 (circle) and lengths 1 & 2 (wave) are missing. This should
;     be considered by the actual animation routine.
;*****************************************************************************
Update_animation:
	movem.l	d0-d3/d6/d7/a0/a1,-(sp)
	lea.l	Async_array,a1
	moveq.l	#0,d1
	moveq.l	#0,d2
; ---------- Update circle animation --------------
	lea.l	Circle_anim,a0
	moveq.l	#2,d7			; Start at length 2 (!)
.Loop1:	moveq.l	#8-1,d6
.Loop2:	cmp.w	#8-1,d6			; First frame ?
	beq.s	.Animate
	btst	d2,0(a1,d1.w)		; No -> Animating ?
	bne.s	.Animate
	jsr	Random			; No -> Start ?
	cmp.w	#Async_anim_bias,d0
	bpl.s	.Next2
	bset	d2,0(a1,d1.w)		; Yes
.Animate:	move.b	(a0),d0			; Increase frame
	addq.b	#1,d0
	cmp.b	d0,d7			; End of animation ?
	bne.s	.Skip1
	jsr	Reset_random_animation	; Yes -> Reset random
	moveq.l	#0,d0			; Back to first frame
	bclr	d2,0(a1,d1.w)		; Stop animating
.Skip1:	move.b	d0,(a0)			; Store new frame
.Next2:	addq.w	#1,d2			; Next
	cmp.w	#8,d2
	bmi.s	.Skip2
	addq.w	#1,d1
	moveq.l	#0,d2
.Skip2:	addq.l	#4,a0
	dbra	d6,.Loop2
	addq.w	#1,d7			; Next animation length
	cmpi.w	#8+1,d7
	bmi.s	.Loop1
	LOCAL
; ---------- Update wave animation ----------------
	lea.l	Wave_anim,a0
	moveq.l	#3,d7			; Start at length 3 (!!)
.Loop1:	moveq.l	#8-1,d6
.Loop2:	cmp.w	#8-1,d6			; First frame ?
	beq.s	.Animate
	btst	d2,0(a1,d1.w)		; No -> Animating ?
	bne.s	.Animate
	jsr	Random			; No -> Start ?
	cmp.w	#Async_anim_bias,d0
	bpl.s	.Next2
	bset	d2,0(a1,d1.w)		; Yes
.Animate:	move.b	(a0),d0
	move.b	d0,d3			; Up or down ?
	and.b	#$80,d3	
	bclr	#7,d0
	bne.s	.Down
	addq.b	#1,d0			; Up -> Increase frame
	cmp.b	d0,d7			; Maximum frame reached ?
	bne.s	.Go_on
	subq.b	#2,d0			; Yes -> Reverse
	bset	#7,d3
	bra.s	.Go_on
.Down:	subq.b	#1,d0			; Down -> Decrease frame
	bne.s	.Go_on			; End of animation ?
	jsr	Reset_random_animation	; Yes -> Reset random
	moveq.l	#0,d0			; Back to first frame
	moveq.l	#0,d3
	bclr	d2,0(a1,d1.w)		; Stop animating
.Go_on:	or.b	d3,d0			; Add direction flag
	move.b	d0,(a0)			; Store new frame
.Next2:	addq.w	#1,d2			; Next
	cmp.w	#8,d2
	bmi.s	.Skip
	addq.w	#1,d1
	moveq.l	#0,d2
.Skip:	addq.l	#4,a0
	dbra	d6,.Loop2
	addq.w	#1,d7			; Next animation length
	cmpi.w	#8+1,d7
	bmi.s	.Loop1
	movem.l	(sp)+,d0-d3/d6/d7/a0/a1
	rts

; [ Reset random animation parameters ]
;   IN : a0 - Pointer to animation table (.l)
; All registers are restored
Reset_random_animation:
	movem.l	d0/d1/d7,-(sp)
	moveq.l	#0,d1			; Reset random bitlist
	moveq.l	#Random_anim_bias-1,d7
.Loop:	jsr	Random
	andi.w	#$000f,d0
	bset	d0,d1
	dbra	d7,.Loop
	move.w	d1,Anim_rnd_bitlist(a0)	; Store
	jsr	Random			; Reset offset
	move.b	d0,Anim_rnd_offset(a0)
	movem.l	(sp)+,d0/d1/d7
	rts

;*****************************************************************************
; [ Get current animation frame ]
;   IN : d2 - Icon bits (.l)
;        d4 - Number of animation frames (.b)
;        d5 - Hash number (.w)
;  OUT : d4 - Current animation frame (0-7) (.l)
; Changed registers : d4
;*****************************************************************************
Get_animation_frame:
	movem.l	d0/d1/a0,-(sp)
	ext.w	d4
	cmp.w	#1,d4			; One frame ?
	beq.s	.Exit
	lea.l	Circle_anim-32,a0		; No -> Circle or wave ?
	btst	#Circle_wave_bit,d2
	beq.s	.Circle
	cmp.w	#2,d4			; Wave -> Two frames ?
	beq.s	.Circle
	lea.l	Wave_anim-64,a0		; No
.Circle:	lsl.w	#5,d4			; Get animation data
	adda.w	d4,a0
	btst	#Random_anim_bit,d2		; Random animation ?
	beq.s	.No_rnd
	btst	#Async_anim_bit,d2		; Yes -> Asynchronous ?
	beq.s	.No_async
	move.w	d5,d0			; Yes
	and.w	#$001c,d0
	add.w	d0,a0
.No_async:	moveq.l	#0,d4			; Default is no animation
	moveq.l	#0,d0			; Select random group
	move.b	1(a0),d0
	add.w	d5,d0	
	andi.w	#$000f,d0
	move.w	2(a0),d1			; Animate ?
	btst	d0,d1
	beq.s	.Exit
.No_rnd:	move.b	(a0),d4			; Yes -> Get current frame
	and.w	#$007f,d4
	ext.l	d4
.Exit:	movem.l	(sp)+,d0/d1/a0
	rts

Async_array:	ds.b 13
