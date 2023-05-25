;*************************************
; [ Shading erzeugen ]
;
;*************************************
produce_shading:
	jsr	Mouse_off

	lea	palette,a0
	lea	shade_tab,a6	;store infos here
	move	#256-1,d7
.loop0:
	move	d7,-(sp)
	move.l	a6,-(sp)
;*** load color ***
	move.l	(a0)+,d0	;load RRGGxxBB
	move.l	d0,d1
	move.l	d0,d2
	rol.l	#8,d0
	swap	d1
	and	#$fc,d0		;R
	and	#$fc,d1		;G
	and	#$fc,d2		;B
	move	d0,ps_red
	move	d1,ps_green
	move	d2,ps_blue
;****** Do 64 shade_levels ******
	moveq	#0,d7		;brightness-level
.shade_loop:
;*** compute color ***
	move	ps_red,d0
	move	ps_green,d1
	move	ps_blue,d2
	mulu	d7,d0		;use as contrast
	mulu	d7,d1
	mulu	d7,d2
	divu	#63,d0
	divu	#63,d1
	divu	#63,d2
	and	#$fc,d0		;reduce to 6 bit
	and	#$fc,d1
	and	#$fc,d2
;d0-d2 = needed color
;*** search color ***
	move	d7,-(sp)
	lea	palette,a1
	moveq	#0,d7		;start with color #0
	move.l	#$7fffffff,a2
.sloop1:
	move.l	(a1)+,d3	;load RRGGxxBB
	move.l	d3,d4
	move.l	d3,d5
	rol.l	#8,d3
	swap	d4
	and	#$fc,d3		;R
	and	#$fc,d4		;G
	and	#$fc,d5		;B
	sub	d0,d3		;delta_R
	sub	d1,d4		;delta_G
	sub	d2,d5		;delta_B
	muls	d3,d3
	muls	d4,d4
	muls	d5,d5
	add.l	d4,d3
	add.l	d5,d3		;-> color-distance
	cmp.l	a2,d3
	bge.s	.bigger
	move.l	d3,a2		;take this color
	move	d7,d6
	tst.l	d3
	beq.s	.found
.bigger:
	addq.b	#1,d7
	bne.s	.sloop1
.found:
	move	(sp)+,d7
	move.b	d6,(a6)		;store col_num
;*** next level ***
	add.w	#256,a6		;next level
	addq	#1,d7
	cmp	#64,d7
	bne	.shade_loop
;****** Next Color ******
	move.l	(sp)+,a6
	addq.l	#1,a6		;next color
	move	(sp)+,d7
	dbra	d7,.loop0

	jsr	Mouse_on
	rts

ps_red:		ds.w	1
ps_green:	ds.w	1
ps_blue:	ds.w	1

palette:	incbin dh1:Michael/WALLS.PAL
shade_tab:	ds.b 16384
