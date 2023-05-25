



.Repeat:	moveq.l	#0,d0			; Pass 1
	moveq.l	#0,d1
	moveq.l	#0,d2
	moveq.l	#0,d7
	exg.l	a1,a2
.Loop1:	move.l	a1,a0			; Get current cell
	jsr	Get_colour_cell
	cmp.l	#-1,d3			; Out of bounds ?
	beq.s	.Next1
	tst.w	d3			; Empty ?
	bne.s	.Full1
	st	d7			; Yes -> Mark
	bra.s	.Next1
.Full1:	move.l	a2,a0			; No -> Set neighbour cells
	subq.w	#1,d0			; R - 1
	jsr	Set_colour_cell
	addq.w	#2,d0			; R + 1
	jsr	Set_colour_cell
	subq.w	#1,d0			; G - 1
	subq.w	#1,d1
	jsr	Set_colour_cell
	addq.w	#2,d1			; G + 1
	jsr	Set_colour_cell
	subq.w	#1,d1			; B - 1
	subq.w	#1,d2
	jsr	Set_colour_cell
	addq.w	#2,d2			; B + 1
	jsr	Set_colour_cell
	subq.w	#1,d2
.Next1:	addq.w	#1,d0			; Next cell
	and.w	#$0007,d0
	bne.s	.Loop1
	addq.w	#1,d1
	and.w	#$0007,d1
	bne.s	.Loop1
	addq.w	#1,d2
	and.w	#$0007,d2
	bne.s	.Loop1
	tst.w	d7			; Any empty cells left ?
	beq	.Done

	moveq.l	#0,d0			; Pass 2
	moveq.l	#0,d1
	moveq.l	#0,d2
	moveq.l	#0,d7
	exg.l	a1,a2
.Loop2:	move.l	a1,a0			; Get current cell
	jsr	Get_colour_cell
	cmp.l	#-1,d3			; Out of bounds ?
	beq.s	.Next2
	tst.w	d3			; Empty ?
	bne.s	.Full2
	st	d7			; Yes -> Mark
	bra.s	.Next2
.Full2:	move.l	a2,a0			; No -> Set neighbour cells
	subq.w	#1,d0			; R - 1, G - 1
	subq.w	#1,d1
	jsr	Set_colour_cell
	addq.w	#2,d0			; R + 1, G - 1
	jsr	Set_colour_cell
	subq.w	#2,d0			; R - 1, G + 1
	addq.w	#2,d1
	jsr	Set_colour_cell
	addq.w	#2,d0			; R + 1, G + 1
	jsr	Set_colour_cell
	subq.w	#1,d0			;        G + 1, B + 1
	addq.w	#1,d2
	jsr	Set_colour_cell
	subq.w	#2,d1			;        G - 1, B + 1
	jsr	Set_colour_cell
	addq.w	#2,d1			;        G + 1, B - 1
	subq.w	#2,d2
	jsr	Set_colour_cell
	subq.w	#2,d1			;        G - 1, B - 1
	jsr	Set_colour_cell
	addq.w	#1,d0			; R + 1,        B - 1
	addq.w	#1,d1
	jsr	Set_colour_cell
	subq.w	#2,d0			; R - 1,        B - 1
	jsr	Set_colour_cell
	addq.w	#2,d0			; R + 1,        B + 1
	addq.w	#2,d2
	jsr	Set_colour_cell
	subq.w	#2,d0			; R - 1,        B + 1
	jsr	Set_colour_cell
	addq.w	#1,d0
	subq.w	#1,d2
.Next2:	addq.w	#1,d0			; Next cell
	and.w	#$0007,d0
	bne.s	.Loop2
	addq.w	#1,d1
	and.w	#$0007,d1
	bne.s	.Loop2
	addq.w	#1,d2
	and.w	#$0007,d2
	bne.s	.Loop2
	tst.w	d7			; Any empty cells left ?
	beq	.Done

	moveq.l	#0,d0			; Pass 3
	moveq.l	#0,d1
	moveq.l	#0,d2
	moveq.l	#0,d7
	exg.l	a1,a2
.Loop3:	move.l	a1,a0			; Get current cell
	jsr	Get_colour_cell
	cmp.l	#-1,d3			; Out of bounds ?
	beq.s	.Next3
	tst.w	d3			; Empty ?
	bne.s	.Full3
	st	d7			; Yes -> Mark
	bra.s	.Next3
.Full3:	move.l	a2,a0			; No -> Set neighbour cells
	subq.w	#1,d0			; R - 1, G - 1, B - 1
	subq.w	#1,d1
	subq.w	#1,d2
	jsr	Set_colour_cell
	addq.w	#2,d2			; R - 1, G - 1, B + 1
	jsr	Set_colour_cell
	addq.w	#2,d1			; R - 1, G + 1, B + 1
	jsr	Set_colour_cell
	subq.w	#2,d2			; R - 1, G + 1, B - 1
	jsr	Set_colour_cell
	addq.w	#2,d0			; R + 1, G + 1, B - 1
	jsr	Set_colour_cell
	addq.w	#2,d2			; R + 1, G + 1, B + 1
	jsr	Set_colour_cell
	subq.w	#2,d1			; R + 1, G - 1, B + 1
	jsr	Set_colour_cell
	subq.w	#2,d2			; R + 1, G - 1, B - 1
	jsr	Set_colour_cell
	subq.w	#1,d0
	addq.w	#1,d1
	addq.w	#1,d2
.Next3:	addq.w	#1,d0			; Next cell
	and.w	#$0007,d0
	bne.s	.Loop3
	addq.w	#1,d1
	and.w	#$0007,d1
	bne.s	.Loop3
	addq.w	#1,d2
	and.w	#$0007,d2
	bne.s	.Loop3
	tst.w	d7			; Any empty cells left ?
	bne	.Repeat
.Done:

; a2 is the final automatus


; [ Get a colour cell's value ]
;   IN : d0 - R-coordinate (.w)
;        d1 - G-coordinate (.w)
;        d2 - B-coordinate (.w)
;        a0 - Pointer to automatus (.l)
;  OUT : d3 - Cell value / -1 = out of bounds (.l)
; Changed registers : d3
; Notes :
;   - This routine will take care of boundary checking.
Get_colour_cell:
	move.l	d0,-(sp)
	moveq.l	#-1,d3			; Default
	cmp.w	#7,d0			; Out of bounds ?
	bhi.s	.Exit
	cmp.w	#7,d1
	bhi.s	.Exit
	cmp.w	#7,d2
	bhi.s	.Exit
	lsl.w	#3,d0			; No -> Calculate index
	or.w	d1,d0
	lsl.w	#3,d0
	or.w	d2,d0
	add.w	d0,d0
	moveq.l	#0,d3			; Get cell
	move.w	d3,0(a0,d0.w)
.Exit:	move.l	(sp)+,d0
	rts

; [ Set a colour cell's value ]
;   IN : d0 - R-coordinate (.w)
;        d1 - G-coordinate (.w)
;        d2 - B-coordinate (.w)
;        d3 - New value (.w)
;        a0 - Pointer to automatus (.l)
; All registers are restored
; Notes :
;   - This routine will take care of boundary checking.
;   - A cell will only be set when the current value is zero.
Set_colour_cell:
	move.l	d0,-(sp)
	cmp.w	#7,d0			; Out of bounds ?
	bhi.s	.Exit
	cmp.w	#7,d1
	bhi.s	.Exit
	cmp.w	#7,d2
	bhi.s	.Exit
	lsl.w	#3,d0			; No -> Calculate index
	or.w	d1,d0
	lsl.w	#3,d0
	or.w	d2,d0
	add.w	d0,d0
	tst.w	0(a0,d0.w)		; Is the cell empty ?
	bne.s	.Exit
	move.w	d3,0(a0,d0.w)		; Yes -> Set cell
.Exit:	move.l	(sp)+,d0
	rts



Automatus1:	ds.w 512
Automatus2:	ds.w 512

