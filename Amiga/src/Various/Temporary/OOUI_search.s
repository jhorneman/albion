
	lea.l	Object_ptrs,a3		; Yes

; [ Search an object and the object referring to it ]
;   IN : d0 - First object handle (.w)
;        d7 - Target object handle (.w)
;        a0 - Pointer to object referring to first object (.l)
;        a3 - Pointer to [ Object_ptrs ] (.l)
;  OUT : a0 - Pointer to target object (.l)
;        a1 - Pointer to object referring to target object / 0 (.l)
; Changed registers : a0,a1

Search_object:
	sub.l	a1,a1			; Default
.Search:	move.l	d0,-(sp)
.Again:	cmp.w	d0,d7			; Is this the one ?
	bne.s	.No1
	move.l	a0,a1			; Yes -> Exit
	move.l	-4(a3,d0.w*4),a0
	bra.s	.Exit
.No1:	move.w	Object_child(a0),d0		; No -> Has children ?
	beq.s	.Next
	move.l	a0,-(sp)			; Yes -> Search
	jsr	.Search
	cmp.l	#0,a1			; Found anything ?
	beq.s	.No2
	addq.l	#4,sp			; Yes -> Exit
	bra.s	.Exit
.No2:	move.l	(sp)+,a0
.Next1:	move.w	Object_next(a0),d0		; Next object
	bne.s	.Again
.Exit:	move.l	(sp)+,d0
	rts
