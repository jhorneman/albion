
	XDEF	Execute_brother_methods

;*****************************************************************************
; [ Execute an object's brothers' method ]
;   IN : d0 - Object handle (.w)
;        d1 - Method number (.w)
; All registers are restored
; Notes :
;   - [ Execute_method ] is called for each brother object.
;   - The method will receive a pointer to the object in a0. All registers
;     are left intact by this routine, APART FROM a0 AND a6, which is
;     destroyed by [ Execute_method ].
;*****************************************************************************
Execute_brother_methods:
	movem.l	d0/a0,-(sp)
	tst.w	d0			; Exit if handle is zero
	beq.s	.Exit
	jsr	Get_object_data		; Get object data
	move.w	Object_next(a0),d0		; Any brothers ?
	beq.s	.Exit
.Again:	jsr	Execute_method		; Execute method
	jsr	Get_object_data		; Any more brothers ?
	move.w	Object_next(a0),d0
	bne.s	.Again
.Exit:	movem.l	(sp)+,d0/a0
	rts

