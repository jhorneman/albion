
Kev_method:	rs.b 1

	jsr	Handle_object_Kevs		; No -> Do normal keys
	bne.s	.Done

;*****************************************************************************
; [ Handle key interaction with objects ]
;   IN : d0 - Key event (.l)
;  OUT : ne - Action
;        eq - No action
; All registers are restored
;*****************************************************************************
Handle_object_Kevs:
	movem.l	d3/a6,-(sp)
	move.l	Root_Sp,a6		; Is the root empty ?
	move.w	(a6),d3
	beq.s	.Exit1
	lea.l	Object_ptrs,a6		; No -> Do
	jsr	.Do
.Exit1:	movem.l	(sp)+,d3/a6
	rts

;   IN : d0 - Key event (.l)
;        d3 - First object handle (.w)
;        a6 - Pointer to [ Object_ptrs ] (.l)
;  OUT : ne - Action
;        eq - No action
; All registers are restored
.Do:
	movem.l	d3/d7/a0-a2,-(sp)
	moveq.l	#-1,d7			; Default is action
.Again:	move.l	-4(a6,d3.w*4),a0		; Get object address
	move.w	Object_child(a0),d3		; Has children ?
	beq.s	.No
	jsr	.Do			; Yes -> Search
	bne.s	.Exit2
.No:	move.l	a0,a1			; No -> Search Kev
	move.l	Object_class(a0),a2
	jsr	Search_Kev
	bne.s	.Exit2
.Next:	move.w	Object_next(a0),d3		; Next object
	bne.s	.Again
	moveq.l	#0,d7			; No action
.Exit2:	tst.w	d7			; Any luck ?
	movem.l	(sp)+,d3/d7/a0-a2
	rts

; [ Search & handle Kev ]
;   IN : d0 - Key event (.l)
;        a1 - Pointer to object (.l)
;        a2 - Pointer to object class definition (.l)
;  OUT : ne - Something was done
;        eq - Nothing was done
; All registers are restored
Search_Kev:
	movem.l	d5/d6/a0/a2,-(sp)
	moveq.l	#0,d6			; Default is no luck
	lea.l	Class_methods(a2),a0	; Search for Mev method
.Again1:	cmp.w	#-1,(a0)			; End ?
	beq.s	.Done
	cmp.w	#Kev_method,(a0)		; Found ?
	bne.s	.Next1
	move.l	2(a0),a0			; Yes -> Handle list
.Again2:	tst.l	(a0)			; End of list ?
	beq.s	.Done
	move.l	(a0)+,d5			; Mask
	and.l	d0,d5
	cmp.l	(a0)+,d5			; Compare
	bne.s	.Next2
	movem.l	d0-d7/a0-a6,-(sp)		; Execute
	movea.l	(a0),a2
	move.l	a1,a0
	jsr	(a2)
	movem.l	(sp)+,d0-d7/a0-a6
	moveq.l	#-1,d6			; Yay!
	bra.s	.Exit
.Next2:	addq.l	#4,a0			; Next event
	bra.s	.Again2
.Next1:	addq.l	#6,a0			; No -> Next method
	bra.s	.Again1
.Done:	tst.l	Class_parent_class(a2)	; Has a parent class ?
	beq.s	.Exit
	move.l	Class_parent_class(a2),a2	; Yes -> Do
	jsr	Search_Kev
	sne	d6
.Exit:	tst.w	d6			; Any luck ?
	movem.l	(sp)+,d5/d6/a0/a2
	rts
