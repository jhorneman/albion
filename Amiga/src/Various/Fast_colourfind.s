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
	movem.l	d1-d7/a0-a2,-(sp)
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
	and.w	#$00e0,d2
	lsr.w	#5,d2
	or.w	d2,d0
	or.w	d1,d0
	lea.l	Remap_indices,a0		; Find colours
	add.w	d0,d0
	add.w	d0,a0
	moveq.l	#0,d0			; Get index
	move.b	(a0)+,d0
	moveq.l	#0,d6			; Get number of colours
	move.b	(a0),d6
	lea.l	Remap_table,a0		; Get colour info
	add.w	d0,a0
	lea.l	Current_palette,a1		; Find closest in set
	moveq.l	#0,d4
	move.l	#$7fffffff,d5
	bra.s	.Entry
.Loop:	moveq.l	#0,d0			; Get colour number
	move.b	(a0),d0
	move.w	d0,d1			; Get colour
	add.w	d0,d0
	add.w	d1,d0
	lea.l	0(a1,d0.w),a2
	moveq.l	#0,d0
	move.b	(a2)+,d0
	moveq.l	#0,d1
	move.b	(a2)+,d1
	moveq.l	#0,d2
	move.b	(a2),d2
	sub.w	Target_R,d0		; Calculate differences
	sub.w	Target_G,d1
	sub.w	Target_B,d2
	muls.w	d0,d0			; Calculate total difference
	muls.w	d1,d1
	muls.w	d2,d2
	add.l	d1,d0
	add.l	d2,d0
	cmp.l	d0,d5			; No -> New maximum ?
	bmi.s	.Next
	move.b	(a0),d4			; Yes -> Get colour number
	move.l	d0,d5
	beq.s	.Exit			; Is it the colour ?
.Next:	addq.l	#1,a0			; No -> Next colour
.Entry:	dbra	d6,.Loop
.Exit:	move.w	d4,d0			; Output
	movem.l	(sp)+,d1-d7/a0-a2
	rts

;***************************************************************************	
; [ Build colour remapping table ]
; All register are restored
;***************************************************************************	
Build_remap_table:
	movem.l	d0-d7/a0-a3/a6,-(sp)
	move.l	#Pal_size*2+512*5,d0	; Create workspace
	jsr	Allocate_memory
	move.l	d0,-(sp)
	jsr	Claim_pointer
	move.l	d0,a6
; ---------- Create work table --------------------
	lea.l	Current_palette,a0		; Create work table
	lea.l	Remap_table,a1
	move.l	a6,a2
	moveq.l	#0,d3
	move.w	#Pal_size-1,d7
.Loop:	moveq.l	#0,d0			; Get RGB values
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
	and.w	#$00e0,d2
	lsr.w	#5,d2
	or.w	d0,d2
	or.w	d1,d2
	move.b	d3,(a1)+			; Store colour index &
	move.w	d2,(a2)+			;  number
	addq.w	#1,d3			; Next colour
	dbra	d7,.Loop
	LOCAL
; ---------- Sort table ---------------------------
	move.w	#Pal_size,d7
	lea.l	Compare_colours,a0
	lea.l	Swap_colours,a1
	lea.l	Remap_table,a2
	jsr	Shellsort
; ---------- Build temporary table ----------------
	move.l	a6,a1
	lea.l	Pal_size*2(a1),a2
	move.l	a2,a0
	moveq.l	#0,d0			; Coordinates
	moveq.l	#0,d1
	moveq.l	#0,d2
	moveq.l	#0,d3			; Offset
	moveq.l	#0,d6			; Counter
.Loop:	move.w	d0,d7			; Calculate colour index
	lsl.w	#3,d7
	or.w	d1,d7
	lsl.w	#3,d7
	or.w	d2,d7
	moveq.l	#0,d5			; Clear counter
.Again:	cmp.w	(a1),d7			; Is this the current index ?
	bne.s	.No
	addq.w	#1,d5			; Yes -> Count up
	addq.l	#2,a1
	cmp.l	a2,a1
	bmi.s	.Again
	move.b	d0,(a0)+			; Write last entry
	move.b	d1,(a0)+
	move.b	d2,(a0)+
	move.b	d3,(a0)+
	move.b	d5,(a0)+
	addq.w	#1,d6
	bra.s	.Done
.No:	tst.b	d5			; Counter zero ?
	beq.s	.Next
	move.b	d0,(a0)+			; No -> Write entry
	move.b	d1,(a0)+
	move.b	d2,(a0)+
	move.b	d3,(a0)+
	move.b	d5,(a0)+
	add.w	d5,d3			; New offset
	addq.w	#1,d6			; Count up
.Next:	addq.w	#1,d2			; Next cell
	and.w	#$0007,d2
	bne.s	.Loop
	addq.w	#1,d1
	and.w	#$0007,d1
	bne.s	.Loop
	addq.w	#1,d0
	and.w	#$0007,d0
	bne.s	.Loop
.Done:	LOCAL
; ---------- Build remap table --------------------
	lea.l	Remap_indices,a0
	move.l	a6,a1
	lea.l	Pal_size*2(a1),a1
	moveq.l	#0,d0
	moveq.l	#0,d1
	moveq.l	#0,d2
	subq.w	#1,d6
.Loop1:	move.l	a1,a2			; Search closest index
	move.l	d6,-(sp)
	move.l	#$7fffffff,d7
.Loop2:	moveq.l	#0,d3			; Get coordinates
	move.b	(a2)+,d3
	moveq.l	#0,d4
	move.b	(a2)+,d4
	moveq.l	#0,d5
	move.b	(a2)+,d5
	sub.w	d0,d3			; Calculate differences
	sub.w	d1,d4
	sub.w	d2,d5
	muls.w	d3,d3			; Calculate total difference
	muls.w	d4,d4
	muls.w	d5,d5
	add.l	d4,d3
	add.l	d5,d3
	cmp.l	d7,d3			; New record ?
	bpl.s	.Next2
	move.l	a2,a3			; Yes
	move.l	d3,d7
	beq.s	.Done
.Next2:	addq.l	#2,a2			; Next
	dbra	d6,.Loop2
.Done:	move.l	(sp)+,d6
	move.w	(a3),(a0)+		; Store counter & offset
.Next1:	addq.w	#1,d2			; Next cell
	and.w	#$0007,d2
	bne.s	.Loop1
	addq.w	#1,d1
	and.w	#$0007,d1
	bne.s	.Loop1
	addq.w	#1,d0
	and.w	#$0007,d0
	bne.s	.Loop1
	move.l	(sp)+,d0			; Destroy workspace
	jsr	Free_pointer
	jsr	Free_memory
	st	Remap_table_valid		; Valid now!
	movem.l	(sp)+,d0-d7/a0-a3/a6
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
Compare_colours:
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
Swap_colours:
	move.l	d0,-(sp)
	move.b	-1(a2,d5.w),d0
	move.b	-1(a2,d6.w),-1(a2,d5.w)
	move.b	d0,-1(a2,d6.w)
	move.w	-2(a6,d5.w*2),d0
	move.w	-2(a6,d6.w*2),-2(a6,d5.w*2)
	move.w	d0,-2(a6,d6.w*2)
	move.l	(sp)+,d0
	rts

	SECTION	Fast_BSS,bss
Target_R:	ds.w 1
Target_G:	ds.w 1
Target_B:	ds.w 1

Remap_table_valid:	ds.b 1
	even
Remap_table:	ds.b Pal_size	; Contains colour numbers
Remap_indices:	ds.w 512		; Contains indices & counters
