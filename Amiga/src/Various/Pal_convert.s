
	OPT AMIGA

	SECTION	Program,code
Start:	lea.l	Palette,a0
	lea.l	Out,a1
	move.w	#256-1,d7
.Loop:	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	addq.l	#1,a0
	move.b	(a0)+,(a1)+
	dbra	d7,.Loop
	rts

Palette:	incbin DH1:Michael/DUNG2.PAL
Out:	dcb.b 256*3-1,0
End:	dc.b 0
