;        d2 - Mask flag (.w)

	move	d2,bz_knuepf


;-- fuer clip_links
	add.w	bz_ovouth,a0		;entfallene pixel ueberspringen

;a0 = source
;a1 = screen
...
	tst	bz_knuepf
	beq	do_bzblocking


;*** das ganze nochmal geblockt ***
do_bzblocking:
	lea	zoom_buffer_h,a2
	lea	zoom_buffer_v,a3
	move	bz_vlines,d6
	move	bz_hlines,d7
	subq	#1,d6
	subq	#1,d7
.yloop:	move.l	a0,a4
	move.l	a1,a5
	move	d6,d5
	move.l	a2,-(sp)
.xloop:	move.b	(a4),(a5)+		;copy pixel
	add.w	(a2)+,a4			;do xstep
	dbra	d5,.xloop
	move.l	(sp)+,a2
	move.w	(a3)+,d0			;load ystep
	mulu	bz_sizex,d0
	add.l	d0,a0			;ystep source
	lea	Map3D_width(a1),a1
	dbra	d7,.yloop
	rts

bz_knuepf:	ds.w 1			;knuepfen ?
