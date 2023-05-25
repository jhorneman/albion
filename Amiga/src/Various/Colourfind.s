
	OUTPUT	RAM:DDT_test

	OPT	Amiga

Pal_size	EQU 256

	SECTION	Program,code
	move.l	(sp)+,XXXX
	move.l	sp,YYYY

	move.w	#$9b,d0
	move.w	#$77,d1
	move.w	#$4f,d2
	move.w	#256,d7
	jsr	Find_closest_colour

EXIT::	move.l	YYYY,sp
	move.l	XXXX,a0
	jmp	(a0)

YYYY:	dc.l 0
XXXX:	dc.l 0

Allocate_memory:
Free_memory:
Free_pointer:
	rts

Claim_pointer:
	move.l	#Workspace,d0
	rts

;***************************************************************************	
; [ Find the closest colour ]
;   IN : d0 - Target R value (.w)
;        d1 - Target G value (.w)
;        d2 - Target B value (.w)
;        d7 - Number of colours (.l)
;  OUT : d0 - Closest colour index (.w)
; Changed registers : d0
;***************************************************************************	
Find_closest_colour:
	movem.l	d1/d2/d4-d7/a0-a3,-(sp)
	tst.b	Remap_table_valid		; Remap table valid ?
	bne.s	.Valid
	jsr	Build_remap_table		; No -> Build
.Valid:	move.w	d0,Target_R		; Store target colour
	move.w	d1,Target_G
	move.w	d2,Target_B
	add.b	#16,d0			; Round up
	bcc.s	.Ok1
	sub.b	#16,d0
.Ok1:	add.b	#16,d1
	bcc.s	.Ok2
	sub.b	#16,d1
.Ok2:	add.b	#16,d2
	bcc.s	.Ok3
	sub.b	#16,d2
.Ok3:	and.w	#$00e0,d0			; Calculate colour index
	add.w	d0,d0
	and.w	#$00e0,d1
	lsr.w	#2,d1
	lsr.w	#5,d2
	or.w	d2,d0
	or.w	d1,d0
	lea.l	Remap_indices,a0		; Find colours
	moveq.l	#0,d1
	moveq.l	#1,d2
.Again:	moveq.l	#0,d6			; Anything for this index ?
	move.b	1(a0,d0.w*2),d6
	bne.s	.Found
.Oops:	neg.w	d1			; No -> Look around
	neg.w	d2
	add.w	d2,d1
	add.w	d1,d0
	cmp.w	#512-1,d0			; Out of range ?
	bhi.s	.Oops
	bra.s	.Again
.Found:	movem.l	d0-d2,-(sp)
	move.b	0(a0,d0.w*2),d0		; Get index
	and.w	#$00ff,d0
	lea.l	Remap_table,a1		; Get colour info
	add.w	d0,a1
	lea.l	Current_palette,a2		; Find closest in set
	moveq.l	#-1,d4
	move.l	#$7fffffff,d5
	bra.s	.Entry
.Loop:	moveq.l	#0,d0			; Get colour number
	move.b	(a1),d0
	cmp.w	d7,d0			; In range ?
	bpl.s	.Next
	move.w	d0,d1			; Get colour
	add.w	d0,d0
	add.w	d1,d0
	lea.l	0(a2,d0.w),a3
	moveq.l	#0,d0
	move.b	(a3)+,d0
	moveq.l	#0,d1
	move.b	(a3)+,d1
	moveq.l	#0,d2
	move.b	(a3),d2
	sub.w	Target_R,d0		; Calculate differences
	sub.w	Target_G,d1
	sub.w	Target_B,d2
	muls.w	d0,d0			; Calculate total difference
	muls.w	d1,d1
	muls.w	d2,d2
	add.l	d1,d0
	add.l	d2,d0
	bne.s	.No			; Is it the colour ?
	movem.l	(sp)+,d0-d2		; Yes
	moveq.l	#0,d0			; Get colour number
	move.b	(a1),d0
	bra.s	.Exit
.No:	cmp.l	d0,d5			; No -> New maximum ?
	bmi.s	.Next
	move.l	d0,d5			; Yes
	moveq.l	#0,d4			; Get colour number
	move.b	(a1),d4
.Next:	addq.l	#1,a1			; Next colour
.Entry:	dbra	d6,.Loop
	movem.l	(sp)+,d0-d2
	tst.w	d4			; Found anything ?
	bmi	.Oops
	move.w	d4,d0			; Yes
.Exit:	movem.l	(sp)+,d1/d2/d4-d7/a0-a3
	rts

;***************************************************************************	
; [ Build colour remapping table ]
; All register are restored
;***************************************************************************	
Build_remap_table:
	movem.l	d0-d3/d7/a0-a2/a6,-(sp)
	move.l	#Pal_size*2,d0		; Create workspace
	jsr	Allocate_memory
	move.l	d0,-(sp)
	jsr	Claim_pointer
	move.l	d0,a6
	lea.l	Current_palette,a0		; Create work table
	lea.l	Remap_table,a1
	move.l	a6,a2
	moveq.l	#0,d3
	move.w	#Pal_size-1,d7
.Loop1:	moveq.l	#0,d0			; Get RGB values
	move.b	(a0)+,d0
	moveq.l	#0,d1
	move.b	(a0)+,d1
	moveq.l	#0,d2
	move.b	(a0)+,d2
	add.b	#16,d0			; Round up
	bcc.s	.Ok1
	sub.b	#16,d0
.Ok1:	add.b	#16,d1
	bcc.s	.Ok2
	sub.b	#16,d1
.Ok2:	add.b	#16,d2
	bcc.s	.Ok3
	sub.b	#16,d2
.Ok3:	and.w	#$00e0,d0			; Calculate colour index
	add.w	d0,d0
	and.w	#$00e0,d1
	lsr.w	#2,d1
	lsr.w	#5,d2
	or.w	d0,d2
	or.w	d1,d2
	move.b	d3,(a1)+			; Store colour index &
	move.w	d2,(a2)+			;  number
	addq.w	#1,d3			; Next colour
	dbra	d7,.Loop1
	move.w	#Pal_size,d7		; Sort table
	lea.l	.Compare,a0
	lea.l	.Swap,a1
	lea.l	Remap_table,a2
	jsr	Shellsort
	lea.l	Remap_indices,a0		; Build remap table
	move.l	a6,a1
	lea.l	Pal_size*2(a6),a6
	moveq.l	#0,d0
	moveq.l	#0,d1
	move.w	#512-1,d7
.Loop2:	move.b	d1,(a0)+			; Store index
	moveq.l	#0,d2			; Clear counter
.Again:	cmp.w	(a1),d0			; Is this the current index ?
	bne.s	.No
	addq.w	#1,d1			; Yes -> Count up
	addq.w	#1,d2
	addq.l	#2,a1
	cmp.l	a6,a1
	bmi.s	.Again
	move.b	d2,(a0)+			; Store last counter
	bra.s	.Done
.No:	move.b	d2,(a0)+			; Store counter
	addq.w	#1,d0			; Next colour index
	dbra	d7,.Loop2
.Done:	move.l	(sp)+,d0			; Destroy workspace
	jsr	Free_pointer
	jsr	Free_memory
	st	Remap_table_valid		; Valid now!
	movem.l	(sp)+,d0-d3/d7/a0-a2/a6
	rts

; [ Compare two colour indices ]
;   IN : d5 - Source index {1...} (.w)
;        d6 - Destination index {1...} (.w)
;        a2 - Pointer to remap table (.l)
;        a6 - Pointer to colour workspace (.l)
;  OUT : eq - Source  = Destination
;        gs - Source >= Destination
;        ls - Source <= Destination
; All registers are restored
.Compare:
	movem.l	d0/d1,-(sp)
	move.w	-2(a6,d5.w*2),d0
	move.w	-2(a6,d6.w*2),d1
	cmp.w	d1,d0
	movem.l	(sp)+,d0/d1
	rts

; [ Swap two colour indices ]
;   IN : d5 - Source index {1...} (.w)
;        d6 - Destination index {1...} (.w)
;        a2 - Pointer to remap table (.l)
;        a6 - Pointer to colour workspace (.l)
; All registers are restored
.Swap:
	move.l	d0,-(sp)
	move.b	-1(a2,d5.w),d0
	move.b	-1(a2,d6.w),-1(a2,d5.w)
	move.b	d0,-1(a2,d6.w)
	move.w	-2(a6,d5.w*2),d0
	move.w	-2(a6,d6.w*2),-2(a6,d5.w*2)
	move.w	d0,-2(a6,d6.w*2)
	move.l	(sp)+,d0
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



	SECTION	Fast_DATA,data
Current_palette:
	incbin	DDT:Graphics/Ready/Test/Test_192.pal
	incbin	DDT:Graphics/Ready/Palettes/Main_palette


	SECTION	Fast_BSS,bss
Workspace:	ds.w Pal_size

Target_R:	ds.w 1
Target_G:	ds.w 1
Target_B:	ds.w 1

;Current_palette:	ds.b 3*Pal_size
Remap_table_valid:	ds.b 1
	even
Remap_table:	ds.b Pal_size	; Contains colour numbers
Remap_indices:	ds.w 512		; Contains indices & counters
