; Various routines
; Written by J.Horneman (In Tune With The Universe)
; Start : 15-2-1994

	XDEF	Get_random_50_100
	XDEF	Shuttlesort
	XDEF	Shellsort
	XDEF	Delay
	XDEF	Wait_4_click
	XDEF	Wait_4_unclick
	XDEF	Wait_4_rightclick
	XDEF	Wait_4_rightunclick
	XDEF	Wait_4_key
	XDEF	Reset_keyboard
	XDEF	Reset_mouse_buffer
	XDEF	Crypt_block
	XDEF	Fill_memory
	XDEF	Copy_memory
	XDEF	Random
	XDEF	Square_root

	SECTION	Program,code
;*****************************************************************************
; [ Delay	]
;   IN : d0 - Number of VBL's	to delay (.w)
; All registers are	restored
;*****************************************************************************
Delay:    
	move.l	d0,-(sp)
	bra.s	.Entry
.Loop:	jsr	My_vsync
.Entry:	dbra	d0,.Loop
	move.l	(sp)+,d0
	rts

;***************************************************************************
; [ Fill memory ]
;   IN : d0 - Number of bytes to clear (must be even) (.l)
;        d1 - Fill value (.l)
;        a0 - Address at which to clear (must be even) (.l)
; All registers are restored
;***************************************************************************
Fill_memory:
	movem.l	d0-d2/a0,-(sp)
	add.l	d0,a0			; Work from back to front
	move.l	d1,d2
	move.l	d0,d1			; Do chunks of 32 bytes
	lsr.l	#5,d1
	beq.s	.Skip
	movem.l	d3-d7/a1/a2,-(sp)
	move.l	d2,d3			; Clear registers
	move.l	d2,d4
	move.l	d2,d5
	move.l	d2,d6
	move.l	d2,d7
	move.l	d2,a1
	move.l	d2,a2
.Loop1:	movem.l	d2-d7/a1/a2,-(a0)
	subq.l	#1,d1
	bne.s	.Loop1
	movem.l	(sp)+,d3-d7/a1/a2
.Skip:	and.w	#$001f,d0			; Do remaining words
	lsr.w	#1,d0
	beq.s	.Exit
	subq.w	#1,d0			; DBRA correction
.Loop2:	move.w	d2,-(a0)
	dbra	d0,.Loop2
.Exit:	movem.l	(sp)+,d0-d2/a0
	rts

;*****************************************************************************
; [ Copy memory ]
;   IN : d0 - Size of memory block (.l)
;        a0 - Source address (.l)
;        a1 - Target address (.l)
; All registers are	restored
; NOTE :
;  - The size is made even.
;  - Normal 32-bit moves are used (no movems).
;*****************************************************************************
Copy_memory:
	movem.l	d0/d1/a0-a2,-(sp)
	addq.l	#1,d0			; Make size even
	and.b	#$fe,d0
	tst.l	d0			; Copy anything ?
	beq	.Exit
	cmpa.l	a0,a1			; Copy to	same address ?
	beq	.Exit			; In which direction ?
	bmi.s	.Forwards
	lea.l	0(a0,d0.l),a2		; Overlapping ?
	cmpa.l	a1,a2
	bgt.s	.Backwards
.Forwards:
	move.l	d0,d1			; Forwards
	lsr.l	#5,d1			; 32 byte blocks
	beq.s	.Skip1
.Loop1:
	rept 8
	move.l	(a0)+,(a1)+
	endr
	subq.l	#1,d1
	bne.s	.Loop1
.Skip1:	and.w	#$001f,d0			; 2 byte blocks
	lsr.w	#1,d0
	bra.s	.Entry2
.Loop2:	move.w	(a0)+,(a1)+
.Entry2:	dbra	d0,.Loop2
	bra	.Exit
.Backwards:
	adda.l	d0,a0			; Copy backwards
	adda.l	d0,a1
	move.l	d0,d1
	lsr.l	#5,d1			; 32 byte blocks
	beq.s	.Skip2
.Loop3:
	rept 8
	move.l	-(a0),-(a1)
	endr
	subq.l	#1,d1
	bne.s	.Loop3
.Skip2:	and.w	#$001f,d0			; 2 byte blocks
	lsr.w	#1,d0
	bra.s	.Entry4
.Loop4:	move.w	-(a0),-(a1)
.Entry4:	dbra	d0,.Loop4
.Exit:	movem.l	(sp)+,d0/d1/a0-a2
	rts

;*****************************************************************************
; [ Get random number ]
;  OUT : d0 - Random number (.w)
; Changed registers : d0
;*****************************************************************************
Random:
	move.l	d1,-(sp)
	movem.w	Seed,d0-d1		; Get seed
	ror.w	#1,d0			; Calculate next seed
	addq.w	#7,d0
	eor.w	d1,d0
	exg.l	d0,d1
	movem.w	d0-d1,Seed		; Store
	and.l	#$0000ffff,d0
	move.l	(sp)+,d1
	rts

;*****************************************************************************
; [ Get random value between 50% of 100% of input value ]
;   IN : d0 - Input value (.w)
;  OUT : d0 - Random value between 50% of 100% of input value (.w)
; Changed registers : d0
;*****************************************************************************
Get_random_50_100:
	move.l	d1,-(sp)
	move.w	d0,d1			; Save input value
	jsr	Random			; Get random value [0-65535]
	mulu.w	#51,d0			; Transform to [0-50]
	swap	d0
	add.w	#50,d0			; Transform to [50-100]
	mulu.w	d1,d0			; Transform input value
	divu.w	#100,d0
	move.l	(sp)+,d1
	rts

;*****************************************************************************
; [ Generic Shuttlesort ]
;   IN : d7 - Number of elements (.w)
;        a0 - Pointer to Compare routine (.l)
;        a1 - Pointer to Swap routine (.l)
; All registers are restored
; Notes :
;   - Unlike Shellsort, this sorting routine will not change the order
;     of elements with an equal value.
;*****************************************************************************
Shuttlesort:
	movem.l	d0/d1/d5/d6,-(sp)
	cmp.w	#1,d7			; More than 1 element ?
	bls.s	.Exit
	moveq.l	#1,d0			; For L = 1 to N-1
.Loop1:	move.w	d0,d5			; If (L) > (L+1)
	move.w	d0,d6
	addq.w	#1,d6
	jsr	(a0)
	ble.s	.Next1
	jsr	(a1)			; Swap (L,L+1)
	move.w	d0,d1			; PS = L-1
.Loop2:	tst.w	d1			; If PS > 0
	beq.s	.Next1
	move.w	d1,d5			; If (PS) > (PS+1)
	move.w	d1,d6
	addq.w	#1,d6
	jsr	(a0)
	ble.s	.Next1
	jsr	(a1)			; Swap (PS,PS+1)
	subq.w	#1,d1			; PS = PS-1
	bra.s	.Loop2
.Next1:	addq.w	#1,d0			; Next L
	cmp.w	d7,d0
	bmi	.Loop1
.Exit:	movem.l	(sp)+,d0/d1/d5/d6
	rts

;*****************************************************************************
; [ Generic Shellsort ]
;   IN : d7 - Number of elements (.w)
;        a0 - Pointer to Compare routine (.l)
;        a1 - Pointer to Swap routine (.l)
; All registers are restored
;*****************************************************************************
Shellsort:
	movem.l	d0-d2/d5/d6,-(sp)
	cmp.w	#1+1,d7			; More than 1 element ?
	bmi	.Done1
	move.w	d7,d0			; Inc = Count
.Again1:	cmp.w	#1,d0			; While (Inc>1)
	bls.s	.Done1
	lsr.w	#1,d0			; Inc = Inc/2
	moveq.l	#1,d1			; L = 1
.Again2:	move.w	d7,d2			; While (L <= Count - Inc)
	sub.w	d0,d2
	cmp.w	d1,d2
	bmi.s	.Done2
	move.w	d1,d5			; If (L) > (L+Inc)
	move.w	d1,d6
	add.w	d0,d6
	jsr	(a0)
	ble.s	.Endif1
	jsr	(a1)			; Swap(L,L+Inc)
	move.w	d1,d2			; Ps = L-Inc
	sub.w	d0,d2
.Again3:	tst.w	d2			; While (Ps>0)
	ble.s	.Done3
	move.w	d2,d5			; If (Ps) > (Ps+Inc)
	move.w	d2,d6
	add.w	d0,d6
	jsr	(a0)
	ble.s	.Else2
	jsr	(a1)			; Swap(Ps,Ps+Inc)
	sub.w	d0,d2			; Ps = Ps-Inc
	bra.s	.Endif2
.Else2:	moveq.l	#0,d2			; Ps = 0
.Endif2:	bra.s	.Again3			; }
.Done3:
.Endif1:	addq.w	#1,d1			; L++
	bra.s	.Again2			; }
.Done2:	bra.s	.Again1			; }
.Done1:	movem.l	(sp)+,d0-d2/d5/d6
	rts

;***************************************************************************
; [ Wait for a mouse click ]
; All registers are restored
;***************************************************************************
Wait_4_click:
	move.l	d0,-(sp)
.Wait:	move.b	Button_state,d0
	btst	#Left_clicked,d0
	bne.s	.Exit
	jsr	Switch_screens
	bra.s	.Wait
.Exit:	jsr	Wait_4_unclick
	move.l	(sp)+,d0
	rts

;***************************************************************************
; [ Wait for mouse button to be released ]
; All registers are restored
;***************************************************************************
Wait_4_unclick:
	move.l	d0,-(sp)
.Wait:	move.b	Button_state,d0
	btst	#Left_pressed,d0
	beq.s	.Exit
	jsr	Switch_screens
	bra.s	.Wait
.Exit:	move.l	(sp)+,d0
	rts

;***************************************************************************
; [ Wait for a right mouse click ]
; All registers are restored
;***************************************************************************
Wait_4_rightclick:
	move.l	d0,-(sp)
.Wait:	move.b	Button_state,d0
	btst	#Right_clicked,d0
	bne.s	.Exit
	jsr	Switch_screens
	bra.s	.Wait
.Exit:	jsr	Wait_4_rightunclick
	move.l	(sp)+,d0
	rts

;***************************************************************************
; [ Wait for right mouse button to be released ]
; All registers are restored
;***************************************************************************
Wait_4_rightunclick:
	move.l	d0,-(sp)
.Wait:	move.b	Button_state,d0		; Wait
	btst	#Right_pressed,d0
	beq.s	.Exit
	jsr	Switch_screens
	bra.s	.Wait
.Exit:	move.l	(sp)+,d0
	rts

;***************************************************************************
; [ Wait for a key ]
; All registers are restored
;***************************************************************************
Wait_4_key:
	move.l	d0,-(sp)
.Again:	jsr	Read_key			; Read a key
	tst.l	d0			; None available ?
	beq.s	.Again			; No -> again
	move.l	(sp)+,d0
	rts

;***************************************************************************
; [ Clear the keyboard buffer ]
; All registers are restored
;***************************************************************************
Reset_keyboard:
	move.l	d0,-(sp)
.Again:	jsr	Read_key			; Read a key
	tst.l	d0			; None available ?
	bne.s	.Again			; Yes -> again
	move.l	(sp)+,d0
	rts

;***************************************************************************
; [ Clear the mouse event buffer ]
; All registers are restored
;***************************************************************************
Reset_mouse_buffer:
	movem.l	d0-d2,-(sp)
.Again:	jsr	Read_Mev			; Read a mouse event
	tst.w	d2			; None available ?
	bne.s	.Again			; Yes -> again
	movem.l	(sp)+,d0-d2
	rts

;*****************************************************************************
; [ Encrypt/decrypt memory block ]
;   IN : d0 - Encryption seed (.w)
;        d7 - Length of block (.l)
;        a0 - Pointer to start of block (.l)
;  OUT : d0 - Encryption seed (.w)
; Changed registers : d0
; NOTE :
;  - The length of the block is made even.
;*****************************************************************************
Crypt_block:
	movem.l	d1/d2/d7/a0,-(sp)
	move.w	#87,d2
	addq.l	#1,d7
	asr.l	#1,d7
	bra	.Entry
.Loop:	eor.w	d0,(a0)+			; Encrypt
	move.w	d0,d1			; Make next seed
	lsl.w	#4,d0			; (Seed =	Seed x 17	+ 87)
	add.w	d1,d0
	add.w	d2,d0
.Entry:	subq.l	#1,d7			; Next word
	bpl.s	.Loop
	movem.l	(sp)+,d1/d2/d7/a0
	rts

;*****************************************************************************
; [ 32-bit square root ]
;   IN : d0 - Value (.l)
;  OUT : d0 - Square root of Value (.w)
; Changed registers : d0
; Original routine by C. Jungen
; Notes :
;  - This routine uses interval minimizing.
;  - It costs about 3 scan-lines of processing time.
;*****************************************************************************
Square_root:
	movem.l	d1-d3,-(sp)
	moveq.l	#0,d1			; Xroot=0
	tst.l	d0			; Legal value ?
	bls	.Exit
	move.l	#$10000000,d2		; m2=2^32
.Iteration:
	move.l	d1,d3
	add.l	d2,d3			; x2=xroot+m2
	asr.l	#1,d1			; SHR(Xroot,1)
	cmp.l	d3,d0			; x2<=x?
	blt.s	.X2_is_greater
	sub.l	d3,d0			; x=x-x2
	add.l	d2,d1			; xroot=xroot+m2
.X2_is_greater:
	asr.l	#2,d2			; SHR(m2,2)
	bne.s	.Iteration
.Exit:	move.l	d1,d0			; Output
	movem.l	(sp)+,d1-d3
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************
	SECTION	Fast_DATA,data
Seed:	dc.l $12345678
