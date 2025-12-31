
;*****************************************************************************
; [ Quicksort ]
;   IN : d7 - Number of values (.l)
;        a0 - Pointer to table (.l)
; No registers are restored
; Note :
;  - The order of elements with the same value will be changed !
;  - The first longword of each entry is compared.
;*****************************************************************************
Quicksort:
	moveq.l	#8,d6
	move.l	a0,a1
	move	d7,d0
	subq	#1,d0
	mulu	d6,d0
	add.w	d0,a1	;-> letzter eintrag

	move	d6,d4
	mulu	#9,d4	;ab 9 werte
;d4.l = differenz ab der bubble-sort aktiv wird

Quicksort_call:
	cmp.l	a0,a1
	ble	.Exit
	move.l	a1,d0
	sub.l	a0,d0
	cmp.l	d4,d0
	ble	qs_bubbleit
	move.l	a0,a2
	move.l	a1,a3
	move.l	(a0),d7
	add.l	(a1),d7
	move.l	d7,d5
	addq.l	#1,d5
	asr.l	#1,d5
	asr.l	#1,d7		;-> mittelwert
.Loop0:
.Loop1:	cmp.l	(a0),d5
	ble.s	.Get1
	add.l	d6,a0
	bra	.Loop1
.Get1:
.Loop2:	cmp.l	(a1),d7
	bge.s	.Get2
	sub.l	d6,a1
	bra	.Loop2
.Get2:	cmp.l	a1,a0
	bge	.Part
;--- vertauschen ---
	move.l	(a0),d0
	move.l	(a1),(a0)
	move.l	d0,(a1)
	move.l	4(a0),d0
	move.l	4(a1),4(a0)
	move.l	d0,4(a1)
	add.l	d6,a0
	sub.l	d6,a1
	cmp.l	a0,a1
	bgt	.Loop0
;--- rekursionen durchfuehren ---
.Part:	movem.l a0-a3,-(sp)		;a2 - a1
	move.l	a2,a0
	bsr	Quicksort_call
	movem.l (sp)+,a0-a3
	move.l	a3,a1			;a0 - a3
	bsr	Quicksort_call
.Exit:	rts
 
;(einfuege-sort)
;*******************
;*** bubble-sort ***
;*******************
qs_bubbleit:
;a0 = start
;a1 = ende (inc.)
	move.l	a0,a2
.Loop0:
;*** 2 zahlen suchen die falsch sind ***
.Loop1:	move.l	(a0),d0
	add.l	d6,a0
	cmp.l	(a0),d0
	bgt.s	.Too_big
	cmp.l	a0,a1
	bne.s	.Loop1
	bra	.Exit
.Too_big:	move.l	(a0),d0
	move.l	4(a0),d1
	move.l	a0,a3
;*** solange zurueckgehen bis kleinere zahl gefunden ***
.Loop2:	sub.l	d6,a0
	cmp.l	a2,a0
	blt.s	.Next
	move.l	(a0),(a0,d6.l)
	move.l	4(a0),4(a0,d6.l)
	cmp.l	(a0),d0
	blt.s	.Loop2	
.Next:	move.l	d0,(a0,d6.l)
	move.l	d1,4(a0,d6.l)
	move.l	a3,a0
	cmp.l	a0,a1
	bne.s	.Loop0
.Exit:	rts
