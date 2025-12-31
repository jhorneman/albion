	moveq	#8+3,d3
	muls	shade_faktor,d2
	lsr.l	d3,d2			;0...32768 -> 0...4096
	moveq	#0,d3
	move.b	(Shade_function,d2.w),d3
	lsl	#8,d3
	lea	Shade_table,a6
	add.w	d3,a6
