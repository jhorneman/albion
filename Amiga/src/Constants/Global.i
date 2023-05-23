; Global contants & macros
; Written by J.Horneman (In Tune With The Universe)
; Start : 27-1-1994

	OPT	GENSYM

TRUE	EQU -1
FALSE	EQU 0


; This is a rectangular area structure
	rsreset
X1:	rs.w 1
X2:	rs.w 1
Y1:	rs.w 1
Y2:	rs.w 1
Recta_size:	rs.b 0

LOCAL	macro
Local\@:
	endm

rseven	macro
	rs.w 0
	endm

qmulu	macro
	mulu.w	\1,\2
	endm

Push	macro
	lea.l	\2,a0
	jsr	Push_\1
	endm

Pop	macro
	jsr	Pop_\1
	endm

Get	macro
	move.l	d0,-(sp)
	move.b	\1,d0
	jsr	Claim_pointer
	move.l	d0,\2
	move.l	(sp)+,d0
	endm

Free	macro
	move.w	d0,-(sp)
	move.b	\1,d0
	jsr	Free_pointer
	move.w	(sp)+,d0
	endm


	incdir	DDT:
	include	Error_codes.i
