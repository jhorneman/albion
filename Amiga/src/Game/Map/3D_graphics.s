; 3D dungeon polygon routine
; Written by Michael Bittner

	SECTION	Program,code
;*****************************************************************************
; [ Draw zoomed shape ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Z-coordinate (.w)
;        d4 - Source width (.w)
;        d5 - Source height (.w)
;        d6 - Target width (.w)
;        d7 - Target height (.w)
;        a0 - Pointer to graphics (.l)
; No registers are restored
*****************************************************************************
draw_zoomshape:
	move	d0,bz_xleft
	move	d1,bz_yup
	move.w	d4,bz_sizex
	move.w	d5,bz_sizey
	move.l	a0,source_gfx

	moveq	#8+3,d3
	muls	shade_faktor,d2
	lsr.l	d3,d2			;0...32768 -> 0...4096
	moveq	#0,d3
	move.b	(Shade_function,d2.w),d3
	lsl	#8,d3
	lea	Shade_table,a6
	add.w	d3,a6

;d6 = new xsize
;d7 = new ysize

	move	d7,d0
	move	d6,d7
;*** y-size berechnen ***
;	move	bz_sizey,d0		;ysize
;	mulu	d7,d0			;*new_xsize
;	divu	bz_sizex,d0		;/old_xsize
	move	d0,bz_hlines2
	move	d7,bz_vlines2

	clr.w	bz_ovjmph
	clr.w	bz_ovjmpv
;*** links clippen ***
	move	bz_xleft,d2
	sub	cx1,d2
	bpl.s	.no_clipl
	add	d2,d7			;ueberstehende pixel abziehen
	ble	bz_clipout
	move.w	cx1,bz_xleft
	neg.w	d2
	move	d2,bz_ovjmph		;horizontal zu ueberspringen
.no_clipl:
;*** unten clippen ***
	move	bz_yup,d2
	add	d0,d2
	subq	#1,d2			;-> ydown
	sub	cy2,d2
	ble.s	.no_clipd
	sub	d2,d0			;ueberstehende pixel abziehen
	ble	bz_clipout
.no_clipd:
;*** oben clippen ***
	move	bz_yup,d2
	sub	cy1,d2
	bpl.s	.no_clipup
	add	d2,d0			;ueberstehende pixel abziehen
	ble	bz_clipout
	neg.w	d2
	move	d2,bz_ovjmpv		;vertikal zu ueberspringen
	clr.w	bz_yup
.no_clipup:
;*** rechts clippen ***
	move	bz_xleft,d2
	add	d7,d2			;+ bz_vlines
	subq	#1,d2			;-> xright
	sub	cx2,d2
	ble.s	.no_clipr
	sub	d2,d7			;ueberstehende pixel abziehen
	ble	bz_clipout
.no_clipr:
	move	d0,bz_hlines
	move	d7,bz_vlines

	bsr	precalc_zooming

;************ jetzt zoomen ************
	move.l	source_gfx,a0
	move.l	Dungeon_screen,a1

	moveq	#0,d0
	move	bz_yup,d0
	mulu.w	#Map3D_width,d0
	add.l	d0,a1

	move	bz_xleft,d0
	add.w	d0,a1			;-> screen_ptr

;-- fuer clip_oben
	move	bz_sizex,d0
	mulu	bz_ovoutv,d0
	add.l	d0,a0			;source_gfx ueberspringen
;-- fuer clip_links
	add.w	bz_ovouth,a0		;entfallene pixel ueberspringen

;a0 = source
;a1 = screen

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
	moveq	#0,d0
.xloop:	move.b	(a4),d0			;load pixel
	adda.w	(a2)+,a4			;do xstep
	beq.s	.jump
	move.b	(a6,d0.w),(a5)+		;write shaded pixel
	dbra	d5,.xloop
	bra.s	.cont
.jump:	addq.l	#1,a5			;skip pixel
	dbra	d5,.xloop
.cont:	move.l	(sp)+,a2

	move.w	(a3)+,d0			;load ystep
	mulu	bz_sizex,d0
	add.l	d0,a0			;ystep source
	lea	Map3D_width(a1),a1

	dbra	d7,.yloop

bz_clipout:
	rts

;*************************************
; [ zoom_buffer fuellen ]
;*************************************
precalc_zooming:
;****** zoom-tabelle vertikal berechnen ******
	move	bz_hlines2,d7
	lea	zoom_buffer_v,a0
;--- init parameter ---
	move	bz_sizey,d0		;ysize of gfx
	subq	#1,d0
	ext.l	d0
	divs	d7,d0			;/new_ysize
	move	d0,d4			;-> add always
	swap	d0			;-> kdelta
	move	d7,d1			;-> gdelta
	move	d1,d2			;-> gdelta2
	addq	#1,d0
;--- overjump ---
	moveq	#0,d3
	move	bz_ovjmpv,d6
	beq.s	.no_ovjmp
	move	d4,d3
	mulu	d6,d3			;add_always*anz
	move	d0,d7
	mulu	d6,d7			;kdelta*anz
	divu	d2,d7			;/gdelta
	add	d7,d3
	swap	d7
	sub	d7,d2			;neues gdelta2
;d3 = pixel ausserhalb
.no_ovjmp:	subq	#1,d2
	move	d3,bz_ovoutv
;--- calc it ---
	move	bz_hlines,d6
	subq	#1,d6
.calc_loop:
	move	d4,d5			;add always
	sub	d0,d2			;gdelta2-kdelta
	bpl.s	.bresen
	add	d1,d2			;gdelta2+gdelta
	addq	#1,d5			;x'step
.bresen:	move.w	d5,(a0)+			;save step's
	dbra	d6,.calc_loop
precalc2:
;****** zoom-tabelle horizontal berechnen ******
	move	bz_vlines2,d7
	lea	zoom_buffer_h,a0
;--- init parameter ---
	move	bz_sizex,d0		;xsize of gfx
	subq	#1,d0
	ext.l	d0
	divs	d7,d0			;/new_xsize
	move	d0,d4			;-> add always
	swap	d0			;-> kdelta
	move	d7,d1				;-> gdelta
	move	d1,d2			;-> gdelta2
	addq	#1,d0
;--- overjump ---
	moveq	#0,d3
	move	bz_ovjmph,d6
	beq.s	.no_ovjmp
	move	d4,d3
	mulu	d6,d3			;add_always*anz
	move	d0,d7
	mulu	d6,d7			;kdelta*anz
	divu	d2,d7			;/gdelta
	add	d7,d3
	swap	d7
	sub	d7,d2			;neues gdelta2
;d3 = pixel ausserhalb
.no_ovjmp:	subq	#1,d2
	move	d3,bz_ovouth
;--- calc it ---
	move	bz_vlines,d6
	subq	#1,d6
.calc_loop:
	move	d4,d5			;add always
	sub	d0,d2			;gdelta2-kdelta
	bpl.s	.bresen
	add	d1,d2			;gdelta2+gdelta
	addq	#1,d5			;x'step
.bresen:	move.w	d5,(a0)+			;save step's
	dbra	d6,.calc_loop
	rts

;*****************************************************************************
; [ Draw dungeon-polygon ]
;   IN : d6 - Source width of polygon in pixels (.w)
;        d7 - Source height of polygon in pixels (.w)
;        a0 - Pointer to wall graphics (.l)
; No registers are restored
;*****************************************************************************
put_dpoly:
	move.w	d6,dpoly_sizex
	move.w	d7,dpoly_sizey
	move.l	a0,dpoly_gfx

;****** 3d-punkte clippen und projezieren ******
	lea	dpoly_koos3d,a0
	bsr	make_dpoly_clip3d
	tst	d0
	bmi	dpoly_skipall
	bsr	make_dpoly_projection
;****** poly-ykoos berechnen ******
	bsr	make_poly_ykoos
	tst	d0
	bmi	dpoly_skipall
;****** x-zoom ermitteln ******
	moveq	#0,d0
	move	dpoly_sizex,d0		;groesse des originals
	subq	#1,d0			;-> last pixel
	swap	d0
	clr.w	d0			;16 bit nachkomma
;*** 3d-clip beruecksichtigen ***
	moveq	#0,d5			;default_startx

	movem.w	dpoly_savez,d1-d2		;z1,z2
	move.l	d2,d3
	sub.l	d1,d3			;delta_z
	bpl.s	.dz_ok
	neg.l	d3
.dz_ok:
;--- erster punkt ? ---
	tst	d1
	bpl.s	.normal_z1
	neg.l	d1			;d1 einheiten ueberspringen
	muls.l	d0,d4:d1
	divs.l	d3,d4:d1			;-> soviel pixel ueberspringen
	sub.l	d1,d0			;source verkuerzen
	move.l	d1,d5			;-> start_x
	bra	.normal_z2
.normal_z1:
;--- zweiter punkt ? ---
	tst	d2
	bpl.s	.normal_z2
	neg.l	d2			;d2 einheiten ueberspringen
	muls.l	d0,d4:d2
	divs.l	d3,d4:d2			;-> soviel pixel weglassen
	sub.l	d2,d0			;source verkuerzen
.normal_z2:
;d0 = source_size
;d5 = start_x in source

;*** steigung berechnen ***
;--- mittel_3D erzeugen & projezieren ---
	lea	dpoly_koos3d,a0
	move.w	dpk_x1(a0),d1
	add.w	dpk_x2(a0),d1
	move.w	dpk_z1(a0),d2
	add.w	dpk_z2(a0),d2
	add.w	#proj_faktor*2,d2		;(z+zp)*2
	ext.l	d1
	add.l	d1,d1			;<<1
	asl.l	#proj_log,d1		;x*zp*2*2
	ext.l	d2
	beq.s	.no_div1
	divs.l	d2,d1			;/((z+zp)*2)
.no_div1:	move	scrx,d2
	ext.l	d2
	add.l	d2,d2
	add.l	d2,d1			;-> x<<1
	move	dpoly_xleft,d2
	move	dpoly_xright,d3
	add	d2,d2
	add	d3,d3
	sub	d2,d3			;-> b*2 (1bit nachkomma)
	sub	d2,d1			;-> q*2
;--- check for overflow ---
	move	d3,d2
	ext.l	d2
	divu	#3,d2			;b/3
	cmp	d2,d1			;q>b/3 ?
	bgt.s	.qbok1
	move	d2,d1
	addq	#1,d1
.qbok1:	neg	d2
	add	d3,d2			;b-b/3
	cmp	d2,d1
	blt.s	.qbok2
	move	d2,d1
	subq	#1,d1
.qbok2:
;--- get s ---
;normal:  s=(b*2-q*4)/(q*q*2-b*b)
;b*2,q*2: s=(b-q*2)/(q*q/2-b*b/4)
	move	d3,d4			;b
	move	d1,d6
	add	d6,d6			;q*2
	sub	d6,d4			;-> b-q*2
	swap	d4
	clr.w	d4			;<<16
	muls	d1,d1			;
	lsr.l	#1,d1			;q*q/2
	muls	d3,d3			;
	lsr.l	#2,d3			;b*b/4
	sub.l	d3,d1			;-> q*q/2-b*b/4
	beq.s	.no_div2
	divs.l	d1,d4			;-> s<<16
.no_div2:
;d4 = s
	move	dpoly_xright,d7
	sub	dpoly_xleft,d7		;-> b
	ext.l	d7
;--- get f ---
;f=p/(b*(s*b/2+c))
	move.l	d4,d3			;s
	muls.l	d7,d3			;s*b
	asr.l	#1,d3			;s*b/2
	add.l	#$10000,d3		;+c (c=1)
	muls.l	d7,d3			;-> b*(s*b/2+c)
;shift p << 16
	move.l	d0,d2			;xsize<<16
	clr.w	d2
	swap	d2			;00|ii
	swap	d0
	clr.w	d0			;ff|00
	tst.l	d3
	beq.s	.no_div3
	divs.l	d3,d2:d0			;-> f<<16
.no_div3:	move.l	d0,d6			;c*f (c=1)
	muls.l	d0,d1:d4			;s*f 00|ii|FF|ff
	clr.w	d4
	swap	d4
	swap	d1
	clr.w	d1
	or.l	d1,d4			;s*f ii|ff
;d4 = s
;d6 = c
;****** links/rechts-clipping ******
;--- rechts ?
	move	cx2,d0
	sub	dpoly_xright,d0
	bge.s	.no_clipright
	add	d0,d7			;shrink
	bmi	dpoly_skipall		;-> rechts ganz draussen
.no_clipright:
;--- links ?
	move	dpoly_xleft,d0
	sub	cx1,d0
	bge.s	.no_clipleft
	add	d0,d7			;shrink
	bmi	dpoly_skipall		;-> links ganz draussen
	move	cx1,dpoly_xleft
	neg.w	d0			;-> soviel spalten ueberspringen

	ext.l	d0
	move.l	d0,d1
	move.l	d0,d2

	mulu	d0,d0
	muls.l	d4,d0			;x*x*s
	muls.l	d6,d1			;x*c
	add.l	d1,d0
	add.l	d1,d0			;x*x*s+x*c*2
	asr.l	#1,d0			;/2
	add.l	d0,d5			;ueberspringen

	muls.l	d4,d2			;x*s
	add.l	d2,d6			;c=c+x*s
.no_clipleft:

;d4 = xvec_s
;d5 = startx
;d6 = xvec_c
;d7 = anzahl spalten

	move.l	d4,d0
	asr.l	#1,d0
	add.l	d0,d6			;c=c+s/2  (integral-correction)

	move.l	d6,dpoly_mxc
	move.l	d4,dpoly_mxs

	move.l	Dungeon_screen,a1
	moveq	#0,d3
	move	dpoly_xleft,d0
	add.w	d0,a1			;-> screen_ptr

	lea	dpoly_ykoos,a3

	tst.b	Masked_wall_flag
	bne	Masked_wall

	tst.l	dpoly_clipflag
	beq	xdpnc_loop
;********* zeichnen mit clipping *********
xdp_loop:
;*** get source-x ***
	move.l	d5,d0
	swap	d0
	muls	dpoly_sizey,d0
	move.l	dpoly_gfx,a0
	add.l	d0,a0			;-> adresse der source-gfx

;*** y-zoom holen ***
	move.l	(a3)+,d1			;y_oben
	move.l	(a3)+,d2			;y_unten
	move.l	d2,d4
	sub.l	d1,d4			;deltay-real
	swap	d1
	swap	d2
;--- get lightshade ---
	move	d2,d0
	cmp	#Map3D_height*3-1,d0
	ble.s	.no_lclip
	move	#Map3D_height*3-1,d0
.no_lclip:
	move.l	(zvals_stab,d0.w*4),a5
;------ oben/unten-clipping ------
;-- oben
	moveq	#0,d6
	cmp	cy2,d1			;cy2
	bgt	.skip			;unten raus
	move	d1,d0
	sub	cy1,d0			;cy1
	bge.s	.no_clipup
	neg.w	d0			;-> pixel oben raus
	move	d0,d6			;merken
	move	cy1,d1			;cy1
.no_clipup:
;-- unten
	cmp	cy1,d2			;cy1
	blt	.skip			;oben raus
	cmp	cy2,d2			;cy2
	ble.s	.no_clipdown
	move	cy2,d2			;cy2
.no_clipdown:

	sub	d1,d2			;-> anzahl zeilen -1
	addq	#1,d2
;*** screen-pointer ***
	moveq.l	#0,d3
	move	d1,d3
	lsl.w	#8,d3			;Map3D_width
	move.l	a1,a4
	add.l	d3,a4
	move.w	#256,a6

	add.l	#$10000,d4
	move.w	dpoly_sizey,d0
;	subq	#1,d0
	moveq	#0,d3
	divu.l	d4,d0:d3			;-> vector

	clr.w	d1			;nkomma_y1 <<16
	mulu.l	d3,d4:d1
	move.l	d4,d1			;-> offset
	not.l	d1
	add.l	d3,d1

	tst	d6
	beq.s	.no_yclip
	ext.l	d6
	mulu.l	d3,d6
	add.l	d6,d1
.no_yclip:	move.l	d1,d4
	swap	d4
	move.w	d3,a2
	swap	d3
;d1 = y.lo
;d3 = adder.hi
;d4 = y.hi
;a2 = adder.lo

	moveq	#3,d0
	and	d2,d0			;and #3,d1
	lsr	#2,d2
	mulu	#14,d0
	neg.w	d0
	add.w	#4*14,d0

;d0 = jump_offset
;d2 = anzahl zeilen/8
;a4 = screen
;a0 = source_gfx
;a6 = 256

	jmp	.yloop(pc,d0.w)
.yloop:
	REPT	4
	move.b	(a0,d4.w),d0		;load pixel-byte
	add.w	a2,d1			;calc step
	addx.w	d3,d4
	move.b	(a5,d0.w),(a4)
	add.l	a6,a4			;to next line
	ENDR
	dbra	d2,.yloop
.skip:
;****** x-zoom weiterrechnen ******
	add.l	dpoly_mxc,d5		;move X_source
	move.l	dpoly_mxs,d0
	add.l	d0,dpoly_mxc
;***** naechste spalte ******
	addq.l	#1,a1

	dbra	d7,xdp_loop

dpoly_skipall:
	rts

;********* das ganze nochmal ohne clipping *********
xdpnc_loop:
;*** get source-x ***
	move.l	d5,d0
	swap	d0
	muls	dpoly_sizey,d0
	move.l	dpoly_gfx,a0
	add.l	d0,a0			;-> adresse der source-gfx

;*** y-zoom holen ***
	move.l	(a3)+,d1			;y_oben
	move.l	(a3)+,d2			;y_unten
	move.l	d2,d4
	sub.l	d1,d4			;deltay-real
	swap	d1
	swap	d2
	move.l	(zvals_stab,d2.w*4),a5
	sub	d1,d2			;-> anzahl zeilen -1
	addq	#1,d2
;*** screen-pointer ***
	moveq.l	#0,d3
	move	d1,d3
	lsl.l	#8,d3			;Map3D_width
	move.l	a1,a4
	add.l	d3,a4
	move.w	#256,a6

	add.l	#$10000,d4
	move.w	dpoly_sizey,d0
;	subq	#1,d0
	moveq	#0,d3
	divu.l	d4,d0:d3			;-> vector

	clr.w	d1			;nkomma_y1 <<16
	mulu.l	d3,d4:d1
	move.l	d4,d1			;-> offset
	not.l	d1
	add.l	d3,d1

	move.l	d1,d4
	swap	d4
	move.w	d3,a2
	swap	d3
;d1 = y.lo
;d3 = adder.hi
;d4 = y.hi
;a2 = adder.lo

	moveq	#7,d0
	and	d2,d0			;and #7,d1
	lsr	#3,d2
	mulu	#14,d0
	neg.w	d0
	add.w	#8*14,d0

;d0 = jump_offset
;d2 = anzahl zeilen/8
;a4 = screen
;a0 = source_gfx
;a6 = 256

	jmp	.yloop(pc,d0.w)
.yloop:
	REPT	8
	move.b	(a0,d4.w),d0		;load pixel-byte
	add.w	a2,d1			;calc step
	addx.w	d3,d4
	move.b	(a5,d0.w),(a4)		;put pixel
	add.l	a6,a4			;to next line
	ENDR
	dbra	d2,.yloop
.skip:
;****** x-zoom weiterrechnen ******
	add.l	dpoly_mxc,d5		;move X_source
	move.l	dpoly_mxs,d0
	add.l	d0,dpoly_mxc
;***** naechste spalte ******
	addq.l	#1,a1
	dbra	d7,xdpnc_loop
	rts

Masked_wall:
	tst.l	dpoly_clipflag
	beq	xdpncm_loop

;********* zeichnen mit clipping *********
xdpm_loop:
;*** get source-x ***
	move.l	d5,d0
	swap	d0
	muls	dpoly_sizey,d0
	move.l	dpoly_gfx,a0
	add.l	d0,a0			;-> adresse der source-gfx

;*** y-zoom holen ***
	move.l	(a3)+,d1			;y_oben
	move.l	(a3)+,d2			;y_unten
	move.l	d2,d4
	sub.l	d1,d4			;deltay-real
	swap	d1
	swap	d2
;--- get lightshade ---
	move	d2,d0
	cmp	#Map3D_height*3-1,d0
	ble.s	.no_lclip
	move	#Map3D_height*3-1,d0
.no_lclip:
	move.l	(zvals_stab,d0.w*4),a5
;------ oben/unten-clipping ------
;-- oben
	moveq	#0,d6
	cmp	cy2,d1			;cy2
	bgt	.skip			;unten raus
	move	d1,d0
	sub	cy1,d0			;cy1
	bge.s	.no_clipup
	neg.w	d0			;-> pixel oben raus
	move	d0,d6			;merken
	move	cy1,d1			;cy1
.no_clipup:
;-- unten
	cmp	cy1,d2			;cy1
	blt	.skip			;oben raus
	cmp	cy2,d2			;cy2
	ble.s	.no_clipdown
	move	cy2,d2			;cy2
.no_clipdown:
	sub	d1,d2			;-> anzahl zeilen -1
	addq	#1,d2
;*** screen-pointer ***
	moveq.l	#0,d3
	move	d1,d3
	lsl.l	#8,d3			;Map3D_width
	move.l	a1,a4
	add.l	d3,a4
	move.w	#256,a6

	add.l	#$10000,d4
	move.w	dpoly_sizey,d0
;	subq	#1,d0
	moveq	#0,d3
	divu.l	d4,d0:d3			;-> vector

	clr.w	d1			;nkomma_y1 <<16
	mulu.l	d3,d4:d1
	move.l	d4,d1			;-> offset
	not.l	d1
	add.l	d3,d1

	tst	d6
	beq.s	.no_yclip
	ext.l	d6
	mulu.l	d3,d6
	add.l	d6,d1
.no_yclip:	move.l	d1,d4
	swap	d4
	move.w	d3,a2
	swap	d3
;d1 = y.lo
;d3 = adder.hi
;d4 = y.hi
;a2 = adder.lo

	moveq	#3,d0
	and	d2,d0			;and #3,d1
	lsr	#2,d2
	lsl.w	#4,d0
	neg.w	d0
	add.w	#4*16,d0

;d0 = jump_offset
;d2 = anzahl zeilen/8
;a4 = screen
;a0 = source_gfx
;a6 = 256

	jmp	.yloop(pc,d0.w)
.yloop:
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero1
	move.b	(a5,d0.w),(a4)
.Zero1:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero2
	move.b	(a5,d0.w),(a4)
.Zero2:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero3
	move.b	(a5,d0.w),(a4)
.Zero3:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero4
	move.b	(a5,d0.w),(a4)
.Zero4:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line

	dbra	d2,.yloop
.skip:
;****** x-zoom weiterrechnen ******
	add.l	dpoly_mxc,d5		;move X_source
	move.l	dpoly_mxs,d0
	add.l	d0,dpoly_mxc
;***** naechste spalte ******
	addq.l	#1,a1

	dbra	d7,xdpm_loop
	rts

;********* das ganze nochmal ohne clipping *********
xdpncm_loop:
;*** get source-x ***
	move.l	d5,d0
	swap	d0
	muls	dpoly_sizey,d0
	move.l	dpoly_gfx,a0
	add.l	d0,a0			;-> adresse der source-gfx

;*** y-zoom holen ***
	move.l	(a3)+,d1			;y_oben
	move.l	(a3)+,d2			;y_unten
	move.l	d2,d4
	sub.l	d1,d4			;deltay-real
	swap	d1
	swap	d2
	move.l	(zvals_stab,d2.w*4),a5
	sub	d1,d2			;-> anzahl zeilen -1
	addq	#1,d2
;*** screen-pointer ***
	moveq.l	#0,d3
	move	d1,d3
	lsl.l	#8,d3			;Map3D_width
	move.l	a1,a4
	add.l	d3,a4
	move.w	#256,a6

	add.l	#$10000,d4
	move.w	dpoly_sizey,d0
;	subq	#1,d0
	moveq	#0,d3
	divu.l	d4,d0:d3			;-> vector

	clr.w	d1			;nkomma_y1 <<16
	mulu.l	d3,d4:d1
	move.l	d4,d1			;-> offset
	not.l	d1
	add.l	d3,d1

	move.l	d1,d4
	swap	d4
	move.w	d3,a2
	swap	d3
;d1 = y.lo
;d3 = adder.hi
;d4 = y.hi
;a2 = adder.lo

	moveq	#7,d0
	and	d2,d0			;and #7,d1
	lsr	#3,d2
	lsl.w	#4,d0
	neg.w	d0
	add.w	#8*16,d0

;d0 = jump_offset
;d2 = anzahl zeilen/8
;a4 = screen
;a0 = source_gfx
;a6 = 256

	jmp	.yloop(pc,d0.w)
.yloop:
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero1
	move.b	(a5,d0.w),(a4)		;put pixel
.Zero1:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero2
	move.b	(a5,d0.w),(a4)		;put pixel
.Zero2:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero3
	move.b	(a5,d0.w),(a4)		;put pixel
.Zero3:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero4
	move.b	(a5,d0.w),(a4)		;put pixel
.Zero4:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero5
	move.b	(a5,d0.w),(a4)		;put pixel
.Zero5:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero6
	move.b	(a5,d0.w),(a4)		;put pixel
.Zero6:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero7
	move.b	(a5,d0.w),(a4)		;put pixel
.Zero7:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line
	move.b	(a0,d4.w),d0		;load pixel-byte
	beq.s	.Zero8
	move.b	(a5,d0.w),(a4)		;put pixel
.Zero8:	add.w	a2,d1			;calc step
	addx.w	d3,d4
	add.l	a6,a4			;to next line

	dbra	d2,.yloop
.skip:
;****** x-zoom weiterrechnen ******
	add.l	dpoly_mxc,d5		;move X_source
	move.l	dpoly_mxs,d0
	add.l	d0,dpoly_mxc
;***** naechste spalte ******
	addq.l	#1,a1
	dbra	d7,xdpncm_loop
	rts

;*************************************
; [ generate 2d-koos ]
;
;*************************************
make_dpoly_projection:
	lea	dpoly_koos2d,a1
	lea	dpoly_clipflag,a2
	move	scrx,d4
	move	Horizon_Y,d5
	move.w	dpk_z1(a0),d0
	add.w	#proj_faktor,d0		;z1+zp
;--- x1 erzeugen ---
	move	dpk_x1(a0),d2
	ext.l	d2
	asl.l	#proj_log,d2
	divs	d0,d2
	add.w	d4,d2			;+scrx
	move.w	d2,(a1)+			;x1_2d
;--- y1 erzeugen ---
	move	dpk_y1(a0),d2
	ext.l	d2
	asl.l	#proj_log,d2
	divs	d0,d2
	neg.w	d2
	add.w	d5,d2			;+Horizon_Y
	move.w	d2,(a1)+			;y1_2d
	cmp	cy2,d2
	sgt	(a2)+
;--- y2 erzeugen ---
	move	dpk_y2(a0),d2
	ext.l	d2
	asl.l	#proj_log,d2
	divs	d0,d2
	neg.w	d2
	add.w	d5,d2			;+Horizon_Y
	move.w	d2,(a1)+			;y2_2d
	cmp	cy1,d2
	slt	(a2)+
;--- next
	move.w	dpk_z2(a0),d0
	add.w	#proj_faktor,d0		;z2+zp
;--- x2 erzeugen ---
	move	dpk_x2(a0),d2
	ext.l	d2
	asl.l	#proj_log,d2
	divs	d0,d2
	add.w	d4,d2			;+scrx
	move.w	d2,(a1)+			;x2_2d
;--- y3 erzeugen ---
	move	dpk_y1(a0),d2
	ext.l	d2
	asl.l	#proj_log,d2
	divs	d0,d2
	neg.w	d2
	add.w	d5,d2			;+Horizon_Y
	move.w	d2,(a1)+			;y3_2d
	cmp	cy2,d2
	sgt	(a2)+
;--- y4 erzeugen ---
	move	dpk_y2(a0),d2
	ext.l	d2
	asl.l	#proj_log,d2
	divs	d0,d2
	neg.w	d2
	add.w	d5,d2			;+Horizon_Y
	move.w	d2,(a1)+			;y4_2d
	cmp	cy1,d2
	slt	(a2)+

	rts

;*************************************
; [ generate clipped 3d-polygon ]
;
;*************************************
make_dpoly_clip3d:
	move.l	dpk_z1(a0),dpoly_savez	;remember original !
	move.w	dpk_z1(a0),d5		;z1
	bmi	.zc_firstout
	move.w	dpk_z2(a0),d6		;z2
	bpl	.zc_allok
;*** z1 positiv, z2 negativ ***
	clr.w	dpk_z2(a0)		;z2 = 0
;--- neues x2 berechnen ---
	sub	d6,d5			;z1-z2
	move.w	dpk_x2(a0),d0
	sub.w	dpk_x1(a0),d0		;x2-x1
	muls	dpk_z1(a0),d0		;*z1
	divs	d5,d0			;/(z1-z2)
	add.w	dpk_x1(a0),d0		;+x1
	move.w	d0,dpk_x2(a0)		;neues x2
;--- fertig ---
	bra	.zc_allok
.zc_firstout:
	move.w	dpk_z2(a0),d6		;z2
	bmi	.zc_allout
;*** z1 negativ, z2 positiv ***
	clr.w	dpk_z1(a0)		;z1 = 0
;--- neues x1 berechnen ---
	sub	d5,d6			;z2-z1
	move.w	dpk_x1(a0),d0
	sub.w	dpk_x2(a0),d0		;x1-x2
	muls	dpk_z2(a0),d0		;*z2
	divs	d6,d0			;/(z2-z1)
	add.w	dpk_x2(a0),d0		;+x2
	move.w	d0,dpk_x1(a0)		;neues x1
;--- fertig ---
.zc_allok:	moveq	#0,d0
	rts
.zc_allout:
	moveq	#-1,d0
	rts

;*************************************
; [ generate y-koos for dpoly ]
; input: dpoly_koos2d
;
;*************************************
make_poly_ykoos:
	movem.w	dpoly_koos2d,d0-d5
	cmp	d0,d3
	bge.s	.ok
	exg	d0,d3
	exg	d1,d4
	exg	d2,d5
.ok:
;d0 = xleft
;d1 = yleft_down
;d2 = yleft_up
;d3 = xright
;d4 = yright_down
;d5 = yright_up

	move	d0,dpoly_xleft
	move	d3,dpoly_xright
	lea	dpoly_ykoos,a0
	sub	d0,d3			;deltax
	addq	#1,d3
	move	d3,d7			;anzahl spalten
;--- steigungen berechnen ---
	ext.l	d3
;unten
	sub	d1,d4			;deltay_down
	swap	d4			;<< 16
	clr.w	d4
	divs.l	d3,d4			;dy/dx
;oben
	sub	d2,d5			;deltay_up
	swap	d5			;<< 16
	clr.w	d5
	divs.l	d3,d5			;dy/dx
;d4 = steigung unten
;d5 = steigung oben
	move	d1,d0
	move	d2,d1
	swap	d0
	swap	d1
	move	#$8000,d0
	move	#$8000,d1
;d0 = start_y unten
;d1 = start_y oben

;--- rechts-clipping ---
	move	cx2,d6
	sub	dpoly_xright,d6
	bge.s	.no_clipright
	add	d6,d7
	bmi	.cancel			;-> ganz draussen
.no_clipright:
;--- links-clipping ---
	move	dpoly_xleft,d6
	sub	cx1,d6
	bge.s	.no_clipleft
	add	d6,d7
	bmi	.cancel			;-> ganz draussen
	neg.w	d6
;pixel ueberspringen
	ext.l	d6
	move.l	d6,-(sp)
	muls.l	d4,d6
	add.l	d6,d0
	move.l	(sp)+,d6
	muls.l	d5,d6
	add.l	d6,d1
.no_clipleft:

;d4 = steigung unten
;d5 = steigung oben

	subq	#1,d7
.do_loop:	move.l	d1,(a0)+
	add.l	d5,d1
	move.l	d0,(a0)+
	add.l	d4,d0
	dbra	d7,.do_loop
	moveq	#0,d0
	rts
.cancel:	moveq	#-1,d0
	rts

;*************************************
; [ Zclip Texture mapping ]
; a0 = ptr to koos
; d7 = anzahl punkte
;data: x_2d,y_2d,x_3d,z_3d,y_3d
;	0    2    4    6    8
;*************************************
clip3d_newTM:
;--- fast precheck ---
	cmp	#4,d7
	bne.s	.do_clip
	tst.w	6(a0)
	bmi.s	.do_clip
	tst.w	6+10(a0)
	bmi.s	.do_clip
	tst.w	6+10*2(a0)
	bmi.s	.do_clip
	tst.w	6+10*3(a0)
	bpl	.no_clip			;-> no zclip necessary
.do_clip:
;--- get last point ---
	subq	#1,d7
	move	d7,d0
	mulu	#10,d0
	lea	(a0,d0.w),a1		;-> pointer to previous
;--- loop ---
	lea	newpoly_koos,a2		; save new poly here
	clr.w	clip_count
.cliptm_loop:
	movem.w	4(a1),d0-d2		; x1,z1,y1
	movem.w	4(a0),d3-d5		; x2,z2,y2
	move	d1,d6
	eor	d4,d6			; any sign-change ?
	bpl.s	.may_save2		;-> NO !
;--- 3D-schnitt-punkt erzeugen ---
	sub	d0,d3			;delta_x
	sub	d1,d4			;delta_z
	sub	d2,d5			;delta_y
	neg.w	d4
	muls	d1,d3			;z1*(x2-x1)
	divs	d4,d3			;/(z1-z2)
	add	d0,d3			;+x1
	muls	d1,d5			;z1*(y2-y1)
	divs	d4,d5			;/(y1-y2)
	add	d2,d5			;+y1
	move	d3,d6
	add	scrx,d6
	move.w	d6,(a2)+			;save x_2d
	move	d5,d6
	neg.w	d6
	add	Horizon_Y,d6
	move.w	d6,(a2)+			;save y_2d
	addq.l	#6,a2			;3D-data not needed
	addq	#1,clip_count
.may_save2:
;--- eventuell 2ten punkt speichern ---
	tst.w	6(a0)			; Z2 positive ?
	bmi.s	.next			;-> NO !
	move.l	(a0),(a2)+		; copy X_2d,Y_2d
	addq.l	#6,a2			;3D-data not needed
	addq	#1,clip_count
.next:	move.l	a0,a1			; next previous point
	add.w	#10,a0
	dbra	d7,.cliptm_loop
	move.l	(a0)+,(a2)+		; source_gfx
	move	clip_count,d7
	lea	newpoly_koos,a0		; new polygon
.no_clip:	rts

;Limit: Hlines paralell zu X-Achse !

;*****************************************************************************
; [ Draw texture mapping polygon ]
;   IN : a0 - Pointer to coordinates (.l)
;        d7 - Number of points (.w)
; No registers are restored
;*****************************************************************************
draw_newTM:
;*****************************
;****** kanten aufbauen ******
;*****************************
	move	#-32768,max_y
	move	#32767,min_y
	subq	#1,d7
	move.l	a0,a6			;for last
.kanten_loop:
;****** get 2D ******
	move.w	(a0)+,d0			;x1
	move.w	(a0)+,d1			;y1
	addq.l	#6,a0			;skip 3D
	move.l	a0,a1
	tst	d7
	bne.s	.nlast1
	move.l	a6,a1
.nlast1:	move.w	(a1)+,d2			;x2
	move.w	(a1)+,d3			;y2
;d0-d3 = 2d-linie
;--- make deltas ---
	moveq	#0,d6
	lea	kanten_rechts,a3
	sub	d0,d2			;delta_x
	sub	d1,d3			;delta_y
	beq	.skip_kante
	bpl.s	.dy_pos
	lea	kanten_links,a3
	add	d2,d0
	add	d3,d1
	neg.w	d2
	neg.w	d3
.dy_pos:
;--- save real line ---
	move.w	d0,tm_x2d
	move.w	d1,tm_y2d
	move	d2,d4
	add	d0,d4
	move.w	d4,tm_x2d2
	move	d3,d4
	add	d1,d4
	move.w	d4,tm_y2d2
	move.w	d2,d4
	bpl.s	.dx_ok1
	neg.w	d4
.dx_ok1:	move.w	d4,tm_deltax
	move.w	d3,tm_ycount
;--- calc ---
	swap	d2			;<< 16
	clr.w	d2
	ext.l	d3
	divs.l	d3,d2			;dx/dy
;d0 = x-start
;d1 = y-start
;d2 = x-vec
;d3 = dy-1
	move.l	d2,tm_x2d_vec

;****** do 2d-clipping ******
	movem.w	tm_x2d,d0-d3		;x1,y1,x2,y2
	clr.w	oben_skip
	clr.w	unten_skip
;*** y-clip checken ***
	move	d1,d4
	sub	cy1,d4
	bge.s	.no_topclip
	sub	d4,oben_skip		;anzahl pixel oben raus
	move	cy1,d1			;change Y
.no_topclip:
	move	d3,d4
	sub	cy2,d4
	ble.s	.no_downclip
	add	d4,unten_skip		;anzahl pixel unten raus
	move	cy2,d3			;change Y
.no_downclip:
	move	tm_ycount,d4
	sub	oben_skip,d4
	sub	unten_skip,d4
	bmi	.skip_kante		;-> kante nicht im Y-bereich
;*** check min_y & max_y ***
	cmp	min_y,d1
	bge.s	.keep_miny
	move	d1,min_y
.keep_miny:
	cmp	max_y,d3
	ble.s	.keep_maxy
	move	d3,max_y
.keep_maxy:
	add	d1,d1			;use Y1
	add.w	d1,a3			;pos in tab
;*** calc the line ***
	move.w	tm_x2d,d0			;load start_x
	swap	d0			;<< 16
	clr.w	d0
	move.l	tm_x2d_vec,a4		;x_vector
	moveq	#0,d3
	move	oben_skip,d3
	beq	.load_normal		;-> do fast loading
;--- clip X ---
	move.l	tm_x2d_vec,d1		;load x_vector
	muls.l	d3,d1
	add.l	d1,d0			;jump x-stuff
;*** get data unclipped ***
.load_normal:
;d0 = x
;a4 = x_vec
;d4 = ycount-1

	move	tm_ycount,d4
	sub	oben_skip,d4
	sub	unten_skip,d4
.calc_loop:
;-- do X --
	move.l	d0,d3
	swap	d3
	move.w	d3,(a3)+
	add.l	a4,d0
	dbra	d4,.calc_loop
;--- next line ---
.skip_kante:
	dbra	d7,.kanten_loop
;*************************
;****** draw hlines ******
;*************************
	lea	tmc_kantentab,a3
	lea	kanten_links,a5
	lea	kanten_rechts,a6
	lea	zvals_stab,a1
	move	max_y,d7
	move	min_y,d6
	cmp	#32767,d6
	beq	skip_poly
	sub	d6,d7
	ble	skip_poly
	move	d6,d5
	add	d5,d5
	add.w	d5,a5
	add.w	d5,a6
	add	d5,d5
	add.w	d5,a1			;ptr to svals
	move	d6,d5
	lsl	#4,d5
	add.w	d5,a3
	lsl.w	#8,d6			;Map3D_width
	move.l	Dungeon_screen,a4
	add.l	d6,a4
	move.l	(a0)+,a0			;ptr to gfx
;a0 = source_gfx
;a4 = screen_ptr
;a5 = kanten links
;a6 = kanten rechts
;d7 = dbra-counter
NTM_hlineloop:
	pea	256(a4)
	pea	4(a1)
	move.w	(a5)+,d0			;screen_x1
	move.w	(a6)+,d1			;screen_x2
	move.l	a3,a2
	add.w	#16,a3
;--- x-clip ? ---
	cmp	cx2,d0
	bgt	.skip_hline
	cmp	cx1,d1
	blt	.skip_hline
	cmp	cx1,d0			;links-clip ?
	bge.s	.no_lclip
	move	cx1,d0
.no_lclip:	cmp	cx2,d1			;rechts clip ?
	ble.s	.no_rclip
	move	cx2,d1
.no_rclip:	sub	d0,d1			;screen_deltax
	addq	#1,d1
	ble	.skip_hline
	move.w	d7,-(sp)

	move.l	(a2)+,d2			;start_sourcex
	move.l	(a2)+,d4			;start_sourcey
	move.l	(a2)+,d3			;dest_sourcex
	move.l	(a2)+,d5			;dest_sourcey
	add.w	d0,a4			;screen_ptr
	sub.l	d2,d3			;deltax
	sub.l	d4,d5			;deltay
	asr.l	#8,d3			;xvector = delta_x / screen_xwidth (=256)
	asr.l	#8,d5			;yvector = delta_x / screen_xwidth (=256)

	move.l	d3,d6
	move.l	d5,d7
	ext.l	d0
	muls.l	d0,d6			;skip pixels
	muls.l	d0,d7
	add.l	d6,d2
	add.l	d7,d4

	move.l	(a1),a1			;-> shading
;--- init paras ---
	move.l	d2,d0			;xwork-LO
	move.w	d3,a2			;xadd-LO
	move.w	d4,d0
	asl.w	#8,d0			;xwork_hi <= ywork_megaLO
	move.w	d5,d3
	asl.w	#8,d3			;xadd_hi <= yadd_megaLO
	swap	d0			;xwork-HI
	swap	d3			;xadd-HI
	asr.l	#8,d4			;16 bit -> 8 bit nkomma
	asr.l	#8,d5			;16 bit -> 8 bit nkomma
;d0 = x-HI (HI-word =y_LO)
;d2 = x-LO
;d3 = xadd-HI (HI-word = yadd_LO)
;d4 = ywork ( 8 bit fractional )
;a2 = xadd-LO
;d5 = yadd ( 8 bit fractional )
	moveq	#3,d7
	and	d1,d7			;and #3,d1
	lsr	#2,d1
	mulu	#22,d7
	neg.w	d7
	add.w	#4*22,d7
;--- draw it ! ---
	moveq.l	#0,d6
	jmp	.xloop(pc,d7.w)
.xloop:
	REPT	4
	move.w	d4,d7
	move.b	d0,d7			;!!
	and.w	#$3f3f,d7			;mask to pattern
	move.b	(a0,d7.l),d6		;get pixel

	add.w	a2,d2			;x.LO
	addx.l	d3,d0			;x.HI ( und Y_LO )

	addx.w	d5,d4			;y_step (8bit)
	move.b	(a1,d6.w),(a4)+		;put pixel
	ENDR
	dbra	d1,.xloop

	move.w	(sp)+,d7
;--- next hline ---
.skip_hline:
	move.l	(sp)+,a1
	move.l	(sp)+,a4			;next line !
	dbra	d7,NTM_hlineloop
skip_poly:	rts

;*****************************************************************************
; [ Draw transparent dungeon-polygon ]
;   IN : d6 - Source width of polygon in pixels (.w)
;        d7 - Source height of polygon in pixels (.w)
;        a0 - Pointer to conversiontable
; No registers are restored
;*****************************************************************************
put_transparent_dpoly:
	move.w	d6,dpoly_sizex
	move.w	d7,dpoly_sizey
	move.l	a0,dpoly_gfx

;****** 3d-punkte clippen und projezieren ******
	lea	dpoly_koos3d,a0
	bsr	make_dpoly_clip3d
	tst	d0
	bmi	tdpoly_skipall
	bsr	make_dpoly_projection
;****** poly-ykoos berechnen ******
	bsr	make_poly_ykoos
	tst	d0
	bmi	tdpoly_skipall

	move	dpoly_xright,d7
	sub	dpoly_xleft,d7		;-> b
;****** links/rechts-clipping ******
;--- rechts ?
	move	cx2,d0
	sub	dpoly_xright,d0
	bge.s	.no_clipright
	add	d0,d7			;shrink
	bmi	tdpoly_skipall		;-> rechts ganz draussen
.no_clipright:
;--- links ?
	move	dpoly_xleft,d0
	sub	cx1,d0
	bge.s	.no_clipleft
	add	d0,d7			;shrink
	bmi	tdpoly_skipall		;-> links ganz draussen
	move	cx1,dpoly_xleft
.no_clipleft:

	move.l	Dungeon_screen,a1
	moveq	#0,d3
	add.w	dpoly_xleft,a1		;-> screen_ptr

	lea	dpoly_ykoos,a3
	move.l	dpoly_gfx,a5

	tst.l	dpoly_clipflag
	beq	txdpnc_loop
;********* zeichnen mit clipping *********
txdp_loop:
;*** y-zoom holen ***
	move.l	(a3)+,d1			;y_oben
	move.l	(a3)+,d2			;y_unten
	swap	d1
	swap	d2
;------ oben/unten-clipping ------
;-- oben
	cmp	cy2,d1			;cy2
	bgt	.skip			;unten raus
	move	d1,d0
	sub	cy1,d0			;cy1
	bge.s	.no_clipup
	move	cy1,d1			;cy1
.no_clipup:
;-- unten
	cmp	cy1,d2			;cy1
	blt	.skip			;oben raus
	cmp	cy2,d2			;cy2
	ble.s	.no_clipdown
	move	cy2,d2			;cy2
.no_clipdown:
	sub	d1,d2			;-> anzahl zeilen -1
	addq	#1,d2
;*** screen-pointer ***
	moveq	#0,d3
	move	d1,d3
	lsl.l	#8,d3
	move.l	a1,a4
	add.l	d3,a4

	moveq	#3,d0
	and	d2,d0			;and #3,d1
	lsr	#2,d2
	lsl	#3,d0			;instead of mulu #8,d0
	neg.w	d0
	add.w	#4*8,d0

	move.w	#256,a6
;d0 = jump_offset
;d2 = anzahl zeilen/4
;a4 = screen
;a6 = 256

	jmp	.yloop(pc,d0.w)
.yloop:
	REPT	4
	move.b	(a4),d0			;load pixel-byte from screen
	move.b	(a5,d0.w),(a4)		;convert & put
	add.l	a6,a4			;goto next line
	ENDR
	dbra	d2,.yloop
.skip:
;***** naechste spalte ******
	addq.l	#1,a1
	dbra	d7,txdp_loop
tdpoly_skipall:
	rts

;********* das ganze nochmal ohne clipping *********
txdpnc_loop:
;*** y-zoom holen ***
	move.l	(a3)+,d1			;y_oben
	move.l	(a3)+,d2			;y_unten
	swap	d1
	swap	d2
	sub	d1,d2			;-> anzahl zeilen -1
	addq	#1,d2
;*** screen-pointer ***
	moveq	#0,d3
	move	d1,d3
	lsl.l	#8,d3
	move.l	a1,a4
	add.l	d3,a4

	moveq	#15,d0
	and	d2,d0			;and #15,d1
	lsr	#4,d2
	lsl	#3,d0			;instead of mulu #8,d0
	neg.w	d0
	add.w	#16*8,d0

	move.w	#256,a6
;d0 = jump_offset
;d2 = anzahl zeilen/16
;a4 = screen
;a6 = 256

	jmp	.yloop(pc,d0.w)
.yloop:
	REPT	16
	move.b	(a4),d0			;load pixel-byte from screen
	move.b	(a5,d0.w),(a4)		;convert & put
	add.l	a6,a4			;goto next line
	ENDR
	dbra	d2,.yloop
.skip:
;***** naechste spalte ******
	addq.l	#1,a1
	dbra	d7,txdpnc_loop
	rts


	SECTION	Longword_bss,bss
	CNOP 0,4
dpoly_ykoos:	ds.l Map3D_width*2
dpoly_koos3d:	ds.w 6
dpoly_koos2d:	ds.w 6
dpoly_savez:	ds.w 2		;original z1/z2
dpoly_xleft:	ds.w 1
dpoly_xright:	ds.w 1
dpoly_sizex:	ds.w 1
dpoly_sizey:	ds.w 1
dpoly_gfx:	ds.l 1
dpoly_clipflag:	ds.l 1
dpoly_mxc:	ds.l 1
dpoly_mxs:	ds.l 1

newpoly_koos:	ds.w	12*5

min_y:		ds.w	1
max_y:		ds.w	1
kanten_links:	ds.w	Map3D_height
kanten_rechts:	ds.w	Map3D_height
tm_x2d:		ds.w	1
tm_y2d:		ds.w	1
tm_x2d2:	ds.w	1
tm_y2d2:	ds.w	1
tm_x2d_vec:	ds.l	1
tm_ycount:	ds.w	1
tm_deltax:	ds.w	1
oben_skip:	ds.w	1
unten_skip:	ds.w	1
clip_count:	ds.w	1

bz_xleft:	ds.w 1			;xpos
bz_yup:		ds.w 1		;ypos
bz_sizex:	ds.w 1			;x-groesse von source_gfx
bz_sizey:	ds.w 1			;y-groesse von source_gfx
bz_hlines2:	ds.w 1		;x-groesse
bz_vlines2:	ds.w 1		;y-groesse
bz_hlines:	ds.w 1			;x-groesse nach clipping
bz_vlines:	ds.w 1			;y-groesse nach clipping
bz_ovjmpv:	ds.w 1			;pixel nach oben raus
bz_ovoutv:	ds.w 1			;ergebniss des ueberspringens
bz_ovjmph:	ds.w 1			;pixel nach rechts raus
bz_ovouth:	ds.w 1			;ergebniss des ueberspringens
source_gfx:	ds.l 1
zoom_buffer_h:	ds.w 320		;zoom-buffer horizontal
zoom_buffer_v:	ds.w 200		;zoom-buffer vertikal
