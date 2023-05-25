;*****************************************************************************
; [ Find an object in the current tree ]
;   IN : d0 - Handle of object (.w)
;  OUT : a0 - Pointer to object / 0 (ERROR) (.l)
; Changed registers : a0
;*****************************************************************************
Find_object:
	movem.l	d0/d1/a1,-(sp)
	move.l	Root_Sp,a0		; Search current tree
	move.w	(a0),d1			; Any objects in tree ?
	beq.s	.Error
	lea.l	Object_ptrs,a1		; Yes
.Again1:	cmp.w	d1,d0			; Search layer 1
	beq.s	.Found
	move.l	-4(a1,d1.w*4),a0
	move.w	Object_child(a0),d1		; Has children ?
	beq.s	.Next1
	move.l	a0,a2			;    Yes
.Again2:	cmp.w	d1,d0			;    Search layer 2
	beq.s	.Found
	move.l	-4(a1,d1.w*4),a0
	move.w	Object_next(a0),d1
	bne.s	.Again2
	move.l	a2,a0			;    Not here
.Next1:	move.w	Object_next(a0),d1		; Next object
	bne.s	.Again1
.Error:	sub.l	a0,a0			; Error
	bra.s	.Exit
.Found:	move.l	-4(a1,d1.w*4),a0		; Found parent object
.Exit:	movem.l	(sp)+,d0/d1/a1
	rts
