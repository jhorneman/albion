;*****************************************************************************
; [ Forbid multitasking ]
; All registers are	restored
;*****************************************************************************
Forbid_it:
	movem.l	d0/d1/a0,-(sp)
	kickEXEC	Forbid
	movem.l	(sp)+,d0/d1/a0
	rts

;*****************************************************************************
; [ Permit multitasking ]
; All registers are	restored
;*****************************************************************************
Permit_it:
	movem.l	d0/d1/a0,-(sp)
	kickEXEC	Permit
	movem.l	(sp)+,d0/d1/a0
	rts

