; Convert 3D wall
; Written by J.Horneman (In Tune With The Universe)
; Start : 7-3-1994

	SECTION	Program,code
;***************************************************************************
; [ Convert a 3D wall to Michael's odd format ]
;   IN : d6 - Width in pixels (.w)
;        d7 - Height in pixels (.w)
;        a0 - Pointer to source buffer (.l)
;        a1 - Pointer to destination buffer (.l)
; All registers are restored
;***************************************************************************
Convert_3D_wall:
	movem.l	d0/d5-d7/a0-a2,-(sp)
	move.w	d6,d0
	subq.w	#1,d6
.Loop_Y:	move.l	a0,a2
	move.w	d7,d5
	subq.w	#1,d5
.Loop_X:	move.b	(a2),(a1)+
	add.w	d0,a2
	dbra	d5,.Loop_X
	addq.l	#1,a0
	dbra	d6,.Loop_Y
	movem.l	(sp)+,d0/d5-d7/a0-a2
	rts
