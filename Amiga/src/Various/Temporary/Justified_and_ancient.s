
;***************************************************************************
; [ Prepare a text line for justification ]
;   IN : a0 - Pointer to text line (.l)
;  OUT : d0 - Length of string / 0 = Justification impossible (.w)
;        a0 - Pointer to string after leading spaces (.l)
; Changed registers : d0,a0
;***************************************************************************
Prepare_justification:
	movem.l	d1/d2/d5-d7/a1,-(sp)
.Again1:	move.b	(a0)+,d0			; Skip leading spaces
	beq	.Exit
	cmp.b	#" ",d0
	beq.s	.Again1
	jsr	Strlen			; Get string length
.Again2:	cmp.b	#" ",-1(a0,d0.w)		; Ignore trailing spaces
	bne.s	.Done2
	subq.w	#1,d0
	bne.s	.Again2
.Done2:	move.w	d0,d7			; Save string length
	jsr	Get_line_length		; Calculate free pixels
	move.w	PA_width,d6
	sub.w	d0,d6

; Registers :
;  d6 - Number of pixels to be divided (.w)
;  d7 - String length (.w)
;  a0 - Pointer to string (.l)

	move.l	a0,a1			; Count spaces
	moveq.l	#0,d1
	move.w	d7,d5
	bra.s	.Entry1
.Loop1:	move.b	(a1)+,d0			; Read character
	cmp.b	#Command_char,d0		; Command character ?
	bne.s	.No_com1
	subq.w	#1,d5
	bmi.s	.End1
.Seek1:	cmpi.b	#Command_char,(a1)+		; Seek end of command
	beq.s	.Entry1
	dbra	d5,.Seek1
	bra.s	.End1
.No_com1:	cmp.b	#" ",d0			; Is it a space ?
	bne.s	.Entry1
	addq.w	#1,d1			; Yes, count
.Entry1:	dbra	d5,.Loop1
.End1:	tst.w	d1			; Any spaces ?
	beq	.Ok
	moveq.l	#0,d7			; No -> Exit
	bra	.Exit
.Ok:	moveq.l	#0,d2			; Divide
	move.w	d6,d2
	divu.w	d1,d2

; Registers :
;  d1 - Number of spaces in string (.w)
;  d2 - Divided pixels (.l)
;  d6 - Number of pixels to be divided (.w)
;  d7 - String length (.w)
;  a0 - Pointer to string (.l)

	lea.l	Space_table,a1		; Divide main pixels
	move.w	d1,d5
	subq.w	#1,d5
.Loop:	move.b	d2,(a1)+
	dbra	d5,.Loop
	lea.l	Space_table,a1		; Divide remaining pixels
	swap	d2
	bra.s	.Entry
.Loop:	jsr	Random
	mulu.w	d1,d0
	swap	d0
	addq.b	#1,0(a1,d0.w)
.Entry:	dbra	d2,.Loop
.Exit:	move.w	d7,d0			; Output
	movem.l	(sp)+,d1/d2/d5-d7/a1
	rts





Space_table:	ds.b 100		; Should always be enough
	even
