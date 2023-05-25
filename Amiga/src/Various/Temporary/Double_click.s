

	btst	#Left_clicked,d7		; Left clicked ?
	beq.s	.No_dleft
	move.l	Left_time,d0		; Yes -> Get previous time
	move.l	Left_time+4,d1
	move.l	ie_TimeStamp+tv_secs(a0),d2	; Get current time
	move.l	ie_TimeStamp+tv_micro(a0),d3
	move.l	d2,Left_time		; Is new previous
	move.l	d3,Left_time+4
	sub.l	d0,d2			; In the same second ?
	beq.s	.Same1
	cmp.l	#1,d2			; No -> Just one apart ?
	bgt.s	.No_dleft
	add.l	#1000,d3			; Yes -> Adapt
.Same1:	sub.l	d1,d3			; Within interval ?
	cmp.l	#Dclick_interval,d3
	bpl.s	.No_dleft
	bset	#Left_double,d7		; Yes
.No_dleft:
	btst	#Right_clicked,d7		; Right clicked ?
	beq.s	.No_dright
	move.l	Right_time,d0		; Yes -> Get previous time
	move.l	Right_time+4,d1
	move.l	ie_TimeStamp+tv_secs(a0),d2	; Get current time
	move.l	ie_TimeStamp+tv_micro(a0),d3
	move.l	d2,Right_time		; Is new previous
	move.l	d3,Right_time+4
	sub.l	d0,d2			; In the same second ?
	beq.s	.Same2
	cmp.l	#1,d2			; No -> Just one apart ?
	bgt.s	.No_dright
	add.l	#1000,d3			; Yes -> Adapt
.Same2:	sub.l	d1,d3			; Within interval ?
	cmp.l	#Dclick_interval,d3
	bpl.s	.No_dright
	bset	#Right_double,d7		; Yes
.No_dright:



	btst	#Left_clicked,d7		; Left clicked ?
	beq.s	.No_ldown
	tst.b	Left_dclick_flag		; Yes -> First or second ?
	bne.s	.Second
	move.l	ie_TimeStamp+tv_secs(a0),Left_time	; 1st -> Set time
	move.l	ie_TimeStamp+tv_micro(a0),Left_time+4
	bra.s	.Done1
.Second:	move.l	Left_time,d0		; 2nd -> Get previous time
	move.l	Left_time+4,d1
	move.l	ie_TimeStamp+tv_secs(a0),d2	; Get current time
	move.l	ie_TimeStamp+tv_micro(a0),d3
	jsr	Check_click_interval	; Was it a click ?
	beq.s	.Done1
	sf	Left_dclick_flag		; Yes
	bset	#Left_double,d7
	bra	.Down1
.No_ldown:	btst	#Left_released,d7		; Left released ?
	beq.s	.Done1
	move.l	Left_time,d0		; Yes -> Get previous time
	move.l	Left_time+4,d1
	move.l	ie_TimeStamp+tv_secs(a0),d2	; Get current time
	move.l	ie_TimeStamp+tv_micro(a0),d3
	move.l	d2,Left_time		; Is new previous
	move.l	d3,Left_time+4
	jsr	Check_click_interval	; Was it a click ?
	beq.s	.Done1
	st	Left_dclick_flag		; Yes
.Done1:

	btst	#Left_clicked,d7		; Left clicked ?
	beq.s	.No_dleft
	move.l	Left_time,d0		; Yes -> Get previous time
	move.l	Left_time+4,d1
	move.l	ie_TimeStamp+tv_secs(a0),d2	; Get current time
	move.l	ie_TimeStamp+tv_micro(a0),d3
	move.l	d2,Left_time		; Is new previous
	move.l	d3,Left_time+4
	movem.l	d1/a0/a1,-(sp)		; Double click ?
	kickINTU	DoubleClick
	movem.l	(sp)+,d1/a0/a1
	tst.l	d0
	beq.s	.No_left
	bset	#Left_double,d7		; Yes
.No_dleft:


	btst	#Right_pressed,Button_state	; Any change ?
	beq.s	.No_dright
	move.l	Right_time,d0		; Get previous time
	move.l	Right_time+4,d1
	move.l	ie_TimeStamp+tv_secs(a0),d2	; Get current time
	move.l	ie_TimeStamp+tv_micro(a0),d3
	move.l	d2,Right_time		; Is new previous
	move.l	d3,Right_time+4
	movem.l	d1/a0/a1,-(sp)		; Double click ?
	kickINTU	DoubleClick
	movem.l	(sp)+,d1/a0/a1
	tst.l	d0
	beq.s	.Go_on
	bset	#Right_double,d7		; Yes
.No_dright:
