	move.l	a2,-(sp)
	lea.l	Batch,a0			; Build batch
	moveq.l	#2,d0
	moveq.l	#Floor_3D_file,d1
	lea.l	Object_handles,a3
	bra.s	.Next
.Loop:	moveq.l	#0,d2			; Insert floor number
	move.b	1(a2),d2
	move.w	d2,(a0)+
	addq.l	#2,a2
	addq.w	#1,d0			; Count up
	cmp.w	#Max_batch,d0		; Batch full ?
	bmi.s	.Next
	lea.l	Batch,a0			; Yes -> Load batch
	move.l	a3,a1
	jsr	Load_batch_of_subfiles
	add.w	d0,a3			; Start next batch
	moveq.l	#0,d0
.Next:	dbra	d7,.Loop			; Next object
	tst.w	d0			; Batch empty ?
	beq.s	.Exit
	lea.l	Batch,a0			; No -> Load batch
	move.l	a3,a1
	jsr	Load_batch_of_subfiles
.Exit:	move.l	(sp)+,a2
	LOCAL
; ---------- Convert floors & ceilings ------------
	lea.l	Object_handles,a1
	lea.l	Floor_handles,a
	moveq.l	#0,d7
	bra.s	.Entry
.Again:	moveq.l	#3,d0			; Allocate next buffer ?
	and.w	d7,d0
	bne.s	.No
	move.b	d6,d0			; Yes
	jsr	Free_pointer
.Entry:	move.l	#64*256,d0		; Allocate next buffer
	jsr	Allocate_memory
	move.b	d0,d6
	jsr	Claim_pointer
	move.l	d0,a3
.No:	move.b	(a1),d0			; Get floor address
	jsr	Claim_pointer
	move.l	d0,a0
	move.l	a3,a4			; Copy floor
	moveq.l	#64-1,d0
.Loop_Y:	moveq.l	#64/4-1,d1
.Loop_X:	move.l	(a0)+,(a4)+
	dbra	d1,.Loop_X
	lea.l	256-64(a4),a4
	dbra	d0,.Loop_Y

	move.b	(a1),d0			; Destroy floor
	jsr	Free_pointer
	jsr	Free_memory
	move.b	d6,(a1)+			; Store converted handle
	lea.l	64(a3),a3			; Next floor
	addq.w	#1,d7
	cmp.w	Nr_of_floors,d7
	bmi.s	.Again
	LOCAL
