;*************************************
; [ conv byte_screen to planes ]
;
;*************************************
convert_AGA:
	move.l	workbase,a1
	move.l	#$0f0f0f0f,a2
	move.l	#$f0f0f0f0,a3
	move.l	#$3333cccc,a4
	move.l	#$55005500,a5
	move.l	#$00aa00aa,a6
;--- zeile anzeigen ---
	lea	byte_screen,a0
	move	#scr_ysize-1,d7
.pp_yloop:
	move	d7,-(sp)
	move.l	a1,-(sp)
	move	#scr_xsize/8-1,d7
	moveq	#1,d6
.pp_xloop:

	move.l	(a0)+,d0	;abcd
	move.l	(a0)+,d1	;efgh

;erstes partielles swappen:
;alle 4x4 im 8x8 grid

;d0 = a7a6a5a4a3a2a1a0|b7b6b5b4b3b2b1b0|c7c6c5c4c3c2c1c0|d7d6d5d4d3d2d1d0
;             ^^^^^^^^         ^^^^^^^^         ^^^^^^^^         ^^^^^^^^
;d1 = e7e6e5e4e3e2e1e0|f7f6f5f4f3f2f1f0|g7g6g5g4g3g2g1g0|h7h6h5h4h3h2h1h0
;     ^^^^^^^^         ^^^^^^^^         ^^^^^^^^         ^^^^^^^^

	move.l	a2,d4		;$0f0f0f0f
	move.l	a3,d5		;$f0f0f0f0
	and.l	d0,d4		;get things to swap
	and.l	d1,d5		;
	eor.l	d4,d0		;remove old stuff
	eor.l	d5,d1		;
	lsl.l	#4,d4
	lsr.l	#4,d5
	or.l	d5,d0		;put in new stuff
	or.l	d4,d1		;

;zweites partielles swappen:
;alle 2x2 im 4x4 grid

;d0 = a7a6a5a4e7e6e5e4|b7b6b5b4f7f6f5f4|c7c6c5c4g7g6g5g4|d7d6d5d4h7h6h5h4
;         1111    1111     1111    1111 2222    2222     2222    2222

;d1 = a3a2a1a0e3e2e1e0|b3b2b1b0f3f2f1f0|c3c2c1c0g3g2g1g0|d3d2d1d0h3h2h1h0
;         1111    1111     1111    1111 2222    2222     2222    2222

	move.l	a4,d4		;$3333cccc
	and.l	d0,d4		;get things to swap
	eor.l	d4,d0		;remove old stuff
	lsr.w	#2,d4		;>> 4
	swap	d4		;swap the stuff
	lsl.w	#2,d4		;<< 4
	or.l	d4,d0		;put in swapped stuff

	move.l	a4,d4		;$3333cccc
	and.l	d1,d4		;get things to swap
	eor.l	d4,d1		;remove old stuff
	lsr.w	#2,d4		;>> 4
	swap	d4		;swap the stuff
	lsl.w	#2,d4		;<< 4
	or.l	d4,d1		;put in swapped stuff

;d0 = a7a6c7c6e7e6g7g6|b7b6d7d6f7f6h7h6|a5a4c5c4e5e4g5g4|b5b4d5d4f5f4h5h4
;       11  11  11  11 22  22  22  22     11  11  11  11 22  22  22  22

;d1 = a3a2c3c2e3e2g3g2|b3b2d3d2f3f2h3h2|a1a0c1c0e1e0g1g0|b1b0d1d0f1f0h1h0
;       11  11  11  11 22  22  22  22     11  11  11  11 22  22  22  22

;drittes partielles swappen:
;alle 1x1 im 2x2 grid

	move.l	a5,d4		;$55005500
	move.l	a6,d5		;$00aa00aa
	and.l	d0,d4		;get 2 things to swap
	and.l	d0,d5		;
	eor.l	d4,d0		;remove old stuff
	eor.l	d5,d0		;
	lsr.l	#7,d4		;>>7
	lsl.l	#7,d5		;<<7
	or.l	d4,d0		;put in swapped stuff
	or.l	d5,d0		;

	move.l	a5,d4		;$55005500
	move.l	a6,d5		;$00aa00aa
	and.l	d1,d4		;get 2 things to swap
	and.l	d1,d5		;
	eor.l	d4,d1		;remove old stuff
	eor.l	d5,d1		;
	lsr.l	#7,d4		;>>7
	lsl.l	#7,d5		;<<7
	or.l	d4,d1		;put in swapped stuff
	or.l	d5,d1		;

;d0 = a7b7c7d7e7f7g7h7|a6b6c6d6e6f6g6h6|a5b5c5d5e5f5g5h5|a4b4c4d4e4f4g4h4
;d1 = a3b3c3d3e3f3g3h3|a2b2c2d2e2f2g2h2|a1b1c1d1e1f1g1h1|a0b0c0d0e0f0g0h0

	move.b	d1,(a1)
	lsr.l	#8,d1
	move.b	d1,2(a1)
	lsr.l	#8,d1
	move.b	d1,4(a1)
	lsr.w	#8,d1
	move.b	d1,6(a1)

	move.b	d0,8(a1)
	lsr.l	#8,d0
	move.b	d0,10(a1)
	lsr.l	#8,d0
	move.b	d0,12(a1)
	lsr.w	#8,d0
	move.b	d0,14(a1)

	add.w	d6,a1
	eor	#14,d6		;toggle 1/15

	dbra	d7,.pp_xloop
	move.l	(sp)+,a1
	move	(sp)+,d7
	lea	320(a1),a1
	dbra	d7,.pp_yloop

	rts
