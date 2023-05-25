


		clr.l	player_x
		clr.l	player_y
		clr.l	y_angel
		move	#1*patt_size+patt_size/2,player_x
		move	#1*patt_size+patt_size/2,player_y
		move	#0,y_angel
		move	#0,head_height
		move	#-1,head_height_old

		move	#6*16,speed

		move	#scr_xsize/2,scrx
		move	#scr_ysize/2,scry

test_loop:
		move.l	#byte_screen,workbase

		move	#0,cx1
		move	#0,cy1
		move	#scr_xsize-1,cx2
		move	#scr_ysize-1,cy2

		bsr	draw_dungeon

		move.l	Work_screen,workbase

		bsr	convert_AGA


;*****************************************
;****** dungeon-ausschnitt zeichnen ******
;*****************************************
draw_dungeon:
;player_x  -> xpos im dungeon
;player_y  -> ypos im dungeon
;y_angel  -> blickwinkel  (0 - slang-1)

; &&& 
; Wall_height	Pre-management (from Lab data)	in cm
; Head_height	Pre-management (0 = floor)		in cm
; Lower_Y		3D drawing
; Upper_Y		3D drawing

	move.w	head_height,d4
; convert from cm to dungeon units
	neg.w	d4
	move	d4,lower_y
	move.w	wall_height,d4
	sub	head_height,d4
; convert from cm to dungeon units
	move.w	d4,upper_y

; &&&
; Recalc when headheight changes

	move	head_height,d0
	cmp	head_height_old,d0
	beq.s	.keep
	move	d0,head_height_old
	bsr	init_tm_coords
	bsr	calc_tm_coords	; &&& was after sort
.keep:

; &&&
; For 3D objects : take coordinates - head_height

	bsr	clear_dungeonscreen

		bsr	get_drawsize		;rechteck holen
;		movem.w	d0-d3,dminx

		bsr	make_rotatedpoints	;punktgitter erzeugen

		bsr	get_faces		;flaechen erzeugen

		lea	wall_list0,a0
		moveq	#8,d6
		move	dface_anz,d7
		cmp	#1,d7
		ble.s	.nosort
		bsr	quicksort		;flaechen sortieren
.nosort:
		bsr	draw_dfaces		;flaechen zeichnen
		rts

;*************************************
; [ precalc some tmap-coords ]
;
;*************************************
calc_tm_coords:	; %%%
		move	boden_y1,d6
		move	boden_ysize,d7
		bsr	calc_the_coords
		move	decke_y1,d6
		move	decke_ysize,d7
		bsr	calc_the_coords
		rts

;*** kanten erzeugen ***
calc_the_coords:
		lea	(prey_ktab,d6.w*4),a5
		add	d6,d6
		lea	(tmc_kantentab,d6.w*8),a0
;--- load sinus & cosinus
		move	y_angel,d0
		move	#slang/4,d1
		add	d0,d1
		and	#slang-1,d1
		lea	sintab,a2
		move.l	a2,a3
		add	d0,d0
		add	d1,d1
		add.w	d0,a2		;sinus
		add.w	d1,a3		;cosinus
;--- precalc some stuff
		move	(a2),d1		;EX=sin*EZ  (EZ=-proj_faktor)
		ext.l	d1		;
		asr.l	#14-proj_log,d1	;
		move	(a3),d2		;EZ=EZ*cos  (EZ=-proj_faktor)
		ext.l	d2		;
		neg.l	d2
		asr.l	#14-proj_log,d2	;
;--
		sub	#16,sp
		moveq	#14,d4
		move	#-scr_xsize/2,d0
		move	d0,d3		;WX_left
		muls	(a2),d3		;WZ=sin*WX
		asr.l	d4,d3
		muls	(a3),d0		;WX=WX*cos
		asr.l	d4,d0
		movem.w	d0-d3,(sp)	;WX,EX,EZ,WZ
;--
		move	#scr_xsize/2-1,d0
		move	d0,d3		;WX_right
		muls	(a2),d3		;WZ=sin*WX
		asr.l	d4,d3
		muls	(a3),d0		;WX=WX*cos
		asr.l	d4,d0
		movem.w	d0-d3,8(sp)
;--- ok, do it
		move.l	player_x,a2
		move.l	player_y,a3
.kant_loop:
		move.l	(a5)+,d5	;EY/(EY-WY) << 16
;-- LEFT
		movem.w	(sp),d0-d3	;load coordinates
;D0 = WX
;D1 = EX
;D2 = EZ
;D3 = WZ
;D5 = EY/(EY-WY)
;-- X
		sub.l	d1,d0		;WX-EX
		muls.l	d5,d0		;*(EY/(EY-WY))
		swap	d1
		clr.w	d1
		add.l	d1,d0		;EX+""""
		add.l	a2,d0		;+ player_x
 		asr.l	#3,d0		;64x64 please !
		move.l	d0,(a0)+	;save 3D_X
;-- Z
		sub.l	d2,d3		;WZ-EZ
		muls.l	d5,d3		;*(EY/(EY-WY))
		swap	d2
		clr.w	d2
		add.l	d2,d3		;EZ+""""
		add.l	a3,d3		;+ player_z
 		asr.l	#3,d3		;64x64 please !
		move.l	d3,(a0)+	;save 3D_Z
;-- RIGHT
		movem.w	8(sp),d0-d3	;load coordinates
;-- X
		sub.l	d1,d0		;WX-EX
		muls.l	d5,d0		;*(EY/(EY-WY))
		swap	d1
		clr.w	d1
		add.l	d1,d0		;EX+""""
		add.l	a2,d0		;+ player_x
 		asr.l	#3,d0		;64x64 please !
		move.l	d0,(a0)+	;save 3D_X
;-- Z
		sub.l	d2,d3		;WZ-EZ
		muls.l	d5,d3		;*(EY/(EY-WY))
		swap	d2
		clr.w	d2
		add.l	d2,d3		;EZ+""""
		add.l	a3,d3		;+ player_z
 		asr.l	#3,d3		;64x64 please !
		move.l	d3,(a0)+	;save 3D_Z
;--
		dbra	d7,.kant_loop
		add.w	#16,sp
		rts


boden_maxz = 22000
;*************************************
; [ init y-vals for tmap-coords ]
;
;*************************************
init_tm_coords:	; %%%
;****** zuerst den boden ******
;--- start_y ermitteln ---
		move	lower_y,d0		;0 = untere ebene
		neg.w	d0
		move	d0,beo_y		;-> real ypos
		ext.l	d0
		asl.l	#proj_log,d0
		add.l	#boden_maxz+proj_faktor-1,d0
		divs	#boden_maxz+proj_faktor,d0
		add	scry,d0
		move	d0,d6
		move	cy2,d7		;bottom
		sub	d0,d7		;anzahl zeilen -1
		sub	scry,d0		;in 3D-raum
		neg.w	d0		;uebernehmen
		add	beo_y,d0	;-> view_y
		move	d0,view_y
		move	#-1,viewy_dir
		move	d6,boden_y1
		move	d7,boden_ysize
		bsr	precalc_yvals
;****** dann auch die decke ******
;--- start_y ermitteln ---
		move	upper_y,d7		;0 = obere ebene
		move	d7,beo_y		;-> real ypos
		ext.l	d7
		asl.l	#proj_log,d7
		add.l	#boden_maxz+proj_faktor-1,d7
		divs	#boden_maxz+proj_faktor,d7
		neg.w	d7
		add	scry,d7
		move	cy1,d6		;top
		move	d6,d0
		sub	d0,d7		;anzahl zeilen -1
		sub	scry,d0		;in 3D-raum zurueck
		add	beo_y,d0	;-> view_y
		move	d0,view_y
		move	#1,viewy_dir
		move	d6,decke_y1
		move	d7,decke_ysize
		bsr	precalc_yvals
;****** Z-vals für shading erzeugen ******
		move	boden_y1,d6
		move	boden_ysize,d7
		bsr	calc_the_zvals
		move	decke_y1,d6
		move	decke_ysize,d7
		bsr	calc_the_zvals
		rts
precalc_yvals:
		lea	(prey_ktab,d6.w*4),a0
		move	view_y,d1
		move	viewy_dir,d2
		move	beo_y,d3
		swap	d3
		clr.w	d3		;EY << 16
		move	beo_y,d4
.kant_loop:	move.l	d3,d5		;EY
		move	d1,d6		;WY
		sub	d4,d6
		beq.s	.no_div
		neg.w	d6		;EY-WY
		ext.l	d6
		divs.l	d6,d5		;EY/(EY-WY) << 16
.no_div:		move.l	d5,(a0)+
		add	d2,d1		;dec view_y
		dbra	d7,.kant_loop
		rts

;*** z-werte erzeugen ***
calc_the_zvals:
		lea	(prey_ktab,d6.w*4),a5
		lea	(zvals_stab,d6.w*4),a0
		move	shade_faktor,d4
		move.l	d4,d1
		mulu	#63,d1
.kant_loop:
		move.l	(a5)+,d5	;EY/(EY-WY) << 16
;-- LEFT
		move.l	#-proj_faktor,d2
		moveq	#0,d3
;D2 = EZ
;D3 = WZ
;D5 = EY/(EY-WY)
;-- Z
		sub.l	d2,d3		;WZ-EZ
		muls.l	d5,d3		;*(EY/(EY-WY))
		swap	d3
		add.w	d2,d3		;EZ+""""

	sub	#512,d3
	bgt.s	.zok1
	moveq	#0,d3
.zok1:
	add	d4,d3			;+shade-faktor
	move.l	d1,d0			;63*shade-faktor
	divu	d3,d0

	lsl	#8,d0		;*256 bytes
	lea	shade_tab,a1
	add.w	d0,a1
		move.l	a1,(a0)+	;save ptr
;--
		dbra	d7,.kant_loop
		rts

shade_faktor:	dc.w	512



tmc_kantentab:	ds.l	4*scr_ysize
prey_ktab:	ds.l	scr_ysize
beo_y:		ds.w	1
boden_y1:	ds.w	1
boden_ysize:	ds.w	1
decke_y1:	ds.w	1
decke_ysize:	ds.w	1
view_y:		ds.w	1
viewy_dir:	ds.w	1

;***********************
;*** waende zeichnen ***
;***********************
draw_dfaces:		; %%%
;****** clipping setzen ******
		move	#0,cx1
		move	#0,cy1
		move	#scr_xsize-1,cx2
		move	#scr_ysize-1,cy2

		lea	wall_list0,a0
		lea	wall_cliptab,a4
		move	#-1,clip_tab	;tabelle leer
		move	dface_anz,d7
		beq	dd_nowalls
;****** pass 1,nur clipping checken ******
		subq	#1,d7	;dbra
gdfaces_loop:
		movem.l	d7/a0,-(sp)
		move.l	4(a0),a0
		cmp.w	#$face,(a0)
		beq.s	.chk_wall
		cmp.w	#"BP",(a0)		; &&& !!!
		beq.s	.chk_poly
		bra.s	.no_face

; &&& FROM HERE...
;--- boden_poly checken ---
.chk_poly:
		bsr	preclip_tmpoly
		move	d3,cx2
		bmi.s	.no_face
		move	d2,cx1
		move.l	a4,-(sp)
		addq.l	#2,a0
		addq	#1,tmp_count
		moveq	#4,d7
		move.l	#tmap_gfx,dpoly_gfx
		bsr	clip3d_newTM
		cmp	#3,d7
		blt	.tm_skip	;-> clip_out !
		bsr	draw_newTM
.tm_skip:	move.l	(sp)+,a4
		bra.s	.no_face

; &&& ...TO HERE

;--- wand checken ---
.chk_wall:	lea	12(a0),a0	;jump to 2d_koos
		bsr	check_vface
		tst	d7
		bmi.s	.out
		move.w	cx1,(a4)+
		move.w	cx2,(a4)+
		bra.s	.no_face
.out:		move.l	#-1,(a4)+
.no_face:	movem.l	(sp)+,d7/a0
		addq.l	#8,a0
		dbra	d7,gdfaces_loop
;****** pass 2, jetzt zeichnen ******
		move.l	a0,a1
		move	dface_anz,d7
		subq	#1,d7	;dbra
ddfaces_loop:
		subq.l	#8,a1
		move.l	4(a1),a0	;-> adresse der def
		move.w	(a0)+,d0	;flag
		cmp	#$face,d0
		beq.s	.draw_dface
		bra	.skip
;*** wand zeichnen ***
.draw_dface:
		move.w	-(a4),d1
		move.w	-(a4),d0
		bmi	.skip		;-> flaeche draussen
		move.w	d0,cx1	;clipping setzen
		move.w	d1,cx2	;
		movem.l	d7/a1/a4,-(sp)
		move	(a0)+,color
		lea	dpoly_koos3d,a1
		movem.w	(a0)+,d0-d3
		move.w	d0,(a1)+	;x1_3d
		move.w	d2,(a1)+	;x2_3d

		move	lower_y,(a1)+	;y1_3d
		move	upper_y,(a1)+	;y2_3d

		move.w	d1,(a1)+	;z1_3d
		move.w	d3,(a1)+	;z2_3d
		bsr	put_dpoly
.aga:		movem.l	(sp)+,d7/a1/a4
.skip:		dbra	d7,ddfaces_loop
dd_nowalls:
		rts

;check for overlaps
preclip_tmpoly:
;--- get minx/maxx ---
		move	#32767,d0	;min_x
		move	#-32768,d4	;max_x
		move.l	a0,a1
		addq.l	#2,a1		;skip flag
		moveq	#4-1,d2
.gx_loop:	move.w	(a1),d3
		cmp	d3,d0
		ble.s	.keep_minx
		move	d3,d0
.keep_minx:	cmp	d3,d4
		bge.s	.keep_maxx
		move	d3,d4
.keep_maxx:	add.w	#10,a1
		dbra	d2,.gx_loop
;d0 = xleft
;d4 = xright
		bsr	check_vface2
		rts

;*************************************
; [ do clip without change to cliptab ]
;d0 = xlinks
;d4 = xrechts
;-> d2,d3 = clips
;*************************************
check_vface2:	; %%%
;-- grob clippen
		tst	d4
		bmi.s	.mega_out
		cmp	#scr_xsize-1,d0
		bgt.s	.mega_out
		tst	d0
		bpl.s	.cok1
		moveq	#0,d0
.cok1:		cmp	#scr_xsize-1,d4
		ble.s	.cok2
		move	#scr_xsize-1,d4
.cok2:
;--
		lea	clip_tab,a5
		move	#0,d2		;default-clipping
		move	#scr_xsize-1,d3
.clip_loop:	move.w	(a5)+,d6	;clip_links
		bmi.s	.clipped	;-> ende der tabelle
		move.w	(a5)+,d7	;clip_rechts
;xlinks drin ?
		cmp	d6,d0
		blt.s	.not_first	;-> x1 links von clip
		cmp	d7,d0
		bgt.s	.clip_loop	;-> aber rechts davon
		move	d7,d2		;x1 also im clip
		cmp	d7,d4
		bgt.s	.clip_loop	;-> x2 nicht im clip
.mega_out:	moveq	#-1,d2		;-> beide im clip !
		moveq	#-1,d3		;also wech damit !
		bra.s	.clipped
;xrechts drin ?
.not_first:
		cmp	d6,d4
		blt.s	.clip_loop	;beide links davon
		move	d6,d3		;rechts geclipt !
.clipped:
		rts

;******************************
;*** ueberlappungen checken ***
;******************************
check_vface:	; %%%
;a0 = adresse der koos

		move	(a0),d0		;x1
		move	2(a0),d1	;x2
		subq	#1,d0		;nachfolgende flaeche soll nur
		addq	#1,d1		;angrenzen,nicht 1 pixel ueberlappen
;--- grob clippen
		tst	d0
		bpl.s	.ok0
		moveq	#0,d0
.ok0:
		cmp	#scr_xsize-1,d1
		ble.s	.ok1
		move	#scr_xsize-1,d1
.ok1:
;--
		lea	clip_tab,a1
		sub.l	a2,a2
		sub.l	a3,a3
vcheck_loop:
		move.w	(a1)+,d2	;vx1
		bmi	vschulz
		move.w	(a1)+,d3	;vx2
;--- x1 innerhalb ? ---
		cmp	d2,d0
		blt	.out1
		cmp	d3,d0
		bgt	.out1
;-> x1 ist innerhalb dieser flaeche
		lea	-4(a1),a2
.out1:
;--- x2 innerhalb ? ---
		cmp	d2,d1
		blt	.out2
		cmp	d3,d1
		bgt	.out2
;-> x2 ist innerhalb dieser flaeche
		lea	-4(a1),a3
.out2:
		bra	vcheck_loop
vschulz:
;******************************
		move.l	a2,d2
		move.l	a3,d3
		cmp.l	d2,d3
		beq	.insame

		tst.l	d2
		beq	.first_out
		tst.l	d3
		beq	.second_out
;*** diese 2 flaechen zusammenhaengen ***
		move	2(a2),cx1
		move	(a3),cx2
		move.w	(a2),(a3)
.move_loop:
		move.l	(a3)+,(a2)+	;flaeche ausfuegen
		bpl.s	.move_loop
		moveq	#0,d7
		rts
;*** erster punkt ausserhalb ***
.first_out:
		move	#0,cx1
		move	(a3),cx2
		move	d0,(a3)		;an x1 des naechsten anhaengen
		moveq	#0,d7
		rts
;*** zweiter punkt ausserhalb ***
.second_out:
		move	#scr_xsize-1,cx2
		move	2(a2),cx1
		move	d1,2(a2)	;an x1 des naechsten anhaengen
		moveq	#0,d7
		rts

;*** beide punkte ausserhalb oder in gleicher flaeche ***
.insame:
		tst.l	d2
		bne	v_out		;-> beide punkte in der gleichen flaeche
;*** flaeche hat nirgends geschnitten ***
		move	#0,cx1		;also normal clippen
		move	#scr_xsize-1,cx2
;--- einsortieren ---
		lea	clip_tab,a1
.is_loop:
		tst.w	(a1)		;ende erreicht ?
		bmi.s	.begin		;-> ja !
		cmp.w	(a1),d1
		blt.s	.begin
		beq.s	.replace
		addq.l	#4,a1
		bra.s	.is_loop
.replace:
		move	d0,(a1)+
		move	d1,(a1)+
		bra.s	.quit
.begin:
		move.l	(a1),d2
		move.w	d0,(a1)+
		move.w	d1,(a1)+
.iloop:
		move.l	(a1),d3
		move.l	d2,(a1)+
		bmi.s	.quit
		move.l	d3,d2
		bra.s	.iloop
.quit:
		moveq	#0,d7
		rts
;*** flaeche ganz ausserhalb ***
v_out:
		moveq	#-1,d7
		rts



;gnd_flag:	ds.w	1

;***********************************************
;****** halbwegs sichtbare flaechen holen ******
;***********************************************
get_faces:
		move.l	dungeon_addr,a0
		add.w	drawstartx,a0
		move	drawstarty,d0
		neg.w	d0
		add	dsizey,d0	;vflip fuer dungeon
		subq	#1,d0
		mulu	dsizex,d0
		add.w	d0,a0		;-> startpos im dungeon
		lea	proj_points,a1
		lea	rot_points,a2
		lea	wall_list0,a3	;zum speichern der adressen und entfernungen
		lea	wall_list1,a4	;zum speichern der wall-defs
		clr.w	dface_anz
		move	drawsizey,d6
		subq	#1,d6		;dbra
gf_yloop:
;	move	drawstartx,gnd_flag
		move	d6,-(sp)
		move.l	a0,-(sp)
		move	drawsizex,d7
		subq	#1,d7		;dbra
gf_xloop:
;	not	gnd_flag
		move	d7,-(sp)

; &&& CHANGE FROM HERE TO...

		moveq	#0,d0
		move.b	(a0),d0		;mauer oder leer ?
		cmp	#" ",d0		; &&& CHANGE
		bne	.wall
;--- boden-polygon erzeugen ---
		move	drawsizex,d3	;3d-koos holen
		addq	#1,d3
		move.l	(a2,d3.w*4),d4	;p1
		move.l	4(a2,d3.w*4),d5	;p2
		move.l	4(a2),d6	;p3
		move.l	(a2),d7		;p4	
		tst.w	d4
		bpl.s	.bp_visible
		tst.w	d5
		bpl.s	.bp_visible
		tst.w	d6
		bpl.s	.bp_visible
		tst.w	d7
		bmi	gf_nextblock	;-> ganz im negativen Z-bereich !
.bp_visible:
		addq	#1,tmp_count2
		move.l	a4,4(a3)		;adresse speichern
		addq	#1,dface_anz
		move.w	#"BP",(a4)+	;flag fuer polygon
		move	lower_y,d2
		move.l	(a1,d3.w*8),(a4)+	;2D_koos (x,y_low)
		move.l	d4,(a4)+		;3D_koos (x,z,y)
		move.w	d2,(a4)+
		move.l	8(a1,d3.w*8),(a4)+	;2D_koos
		move.l	d5,(a4)+		;3D_koos
		move.w	d2,(a4)+
		move.l	8(a1),(a4)+		;2D_koos
		move.l	d6,(a4)+		;3D_koos
		move.w	d2,(a4)+
		move.l	(a1),(a4)+		;2D_koos
		move.l	d7,(a4)+		;3D_koos
		move.w	d2,(a4)+

		moveq	#1,d0		; &&& Change
		and.w	gnd_flag,d0
		lsl	#7,d0
		add.l	#tmap_gfx,d0
		move.l	d0,(a4)+

		add.w	d7,d5		;mitte diagonal ermitteln
		asr.w	#1,d5
		sub.w	#patt_size/2,d5
		ext.l	d5
		move.l	d5,(a3)		;entfernung speichern
		addq.l	#8,a3
;--- decken-polygon erzeugen ---
;	bra	gf_nextblock

; &&& SKIP THIS WHEN NO CEILING

		move	drawsizex,d3	;3d-koos holen
		addq	#1,d3
		move.l	(a2),d4		;p1
		move.l	4(a2),d5	;p2
		move.l	4(a2,d3.w*4),d6	;p3
		move.l	(a2,d3.w*4),d7	;p4	
		tst.w	d4
		bpl.s	.bp2_visible
		tst.w	d5
		bpl.s	.bp2_visible
		tst.w	d6
		bpl.s	.bp2_visible
		tst.w	d7
		bmi	gf_nextblock	;-> ganz im negativen Z-bereich !
.bp2_visible:
		addq	#1,tmp_count2
		move.l	a4,4(a3)	;adresse speichern
		addq	#1,dface_anz
		move.w	#"BP",(a4)+	;flag fuer polygon
		move	upper_y,d2
		move.l	4(a1),(a4)+		;2D_koos (x,y_high)
		move.l	d4,(a4)+		;3D_koos
		move.w	d2,(a4)+
		move.l	12(a1),(a4)+		;2D_koos
		move.l	d5,(a4)+		;3D_koos
		move.w	d2,(a4)+
		move.l	12(a1,d3.w*8),(a4)+	;2D_koos
		move.l	d6,(a4)+		;3D_koos
		move.w	d2,(a4)+
		move.l	4(a1,d3.w*8),(a4)+	;2D_koos
		move.l	d7,(a4)+		;3D_koos
		move.w	d2,(a4)+

		moveq	#1,d0
		and.w	gnd_flag,d0
		lsl	#7,d0
		add.l	#tmap_gfx+32768,d0
		move.l	d0,(a4)+

		add.w	d7,d5		;mitte diagonal ermitteln
		asr.w	#1,d5
		sub.w	#patt_size/2,d5
		ext.l	d5
		move.l	d5,(a3)		;entfernung speichern
		addq.l	#8,a3

		bra	gf_nextblock	;-> hier keine mauer !

; &&& ...HERE

.wall:		bsr	conv_tocolor	;farbe holen
;-> alle 4 mauern checken !
		move	dsizex,d1
		neg.w	d1
;--- oben ---
		cmp.b	#" ",(a0,d1.w)
		bne	.blocko
;--- 2d-koos laden ---
		move	drawsizex,d7
		addq	#1,d7
		asl	#3,d7
		move	8(a1,d7.w),d0	;x1
		move	(a1,d7.w),d1	;x2
;-- 3d-koos laden (hi = x , lo = y)
		asr	#1,d7		;3d-koos holen
		move.l	4(a2,d7.w),d6
		move.l	(a2,d7.w),d7
		tst.w	d6
		bpl.s	.ok3d1
		tst.w	d7
		bmi	.blocko
.ok3d1:
;-- sichtbar ?
		cmp	d0,d1
		blt	.blocko		;-> hidden
		cmp	cx1,d1
		blt	.blocko		;links ausserhalb
		cmp	cx2,d0
		bgt	.blocko		;rechts ausserhalb
;-- sichtbar !
		move.l	a4,4(a3)	;adresse speichern
		addq	#1,dface_anz
		move.w	#$face,(a4)+	;flag fuer wand
		move	color,(a4)+	;farbe
		move.l	d6,(a4)+	;3d-koos speichern
		move.l	d7,(a4)+
		move	d0,(a4)+	;x1
		move	d1,(a4)+	;x2
		add	d7,d6
		asr	#1,d6
		ext.l	d6
		move.l	d6,(a3)		;entfernung speichern
		addq.l	#8,a3
		bra	.blocku		;gegenueberliegende flaeche ueberspringen
.blocko:
		move	dsizex,d1
;--- unten ---
		cmp.b	#" ",(a0,d1.w)
		bne	.blocku
;--- 2d-koos laden ---
		move	(a1),d0		;x1
		move	8(a1),d1	;x2
;-- 3d-koos laden
		move.l	(a2),d6
		move.l	4(a2),d7
		tst.w	d6
		bpl.s	.ok3d2
		tst.w	d7
		bmi	.blocku
.ok3d2:
;-- sichtbar ?
		cmp	d0,d1
		blt	.blocku		;-> hidden
		cmp	cx1,d1
		blt	.blocku		;links ausserhalb
		cmp	cx2,d0
		bgt	.blocku		;rechts ausserhalb
;-- sichtbar !
		move.l	a4,4(a3)	;adresse speichern
		addq	#1,dface_anz
		move.w	#$face,(a4)+	;flag fuer wand
		move	color,(a4)+	;farbe
		move.l	d6,(a4)+
		move.l	d7,(a4)+
		move	d0,(a4)+	;x1
		move	d1,(a4)+	;x2
		add	d7,d6
		asr	#1,d6
		ext.l	d6
		move.l	d6,(a3)		;entfernung speichern
		addq.l	#8,a3
.blocku:
;--- links ---
		cmp.b	#" ",-1(a0)
		bne	.blockl
;--- 2d-koos laden ---
		move	drawsizex,d7
		addq	#1,d7
		asl	#3,d7
		move	(a1,d7.w),d0	;x1
		move	(a1),d1		;x2
;-- 3d-koos laden
		asr	#1,d7		;3d-koos holen
		move.l	(a2,d7.w),d6
		move.l	(a2),d7
		tst.w	d6
		bpl.s	.ok3d3
		tst.w	d7
		bmi	.blockl
.ok3d3:
;-- sichtbar ?
		cmp	d0,d1
		blt	.blockl		;-> hidden
		cmp	cx1,d1
		blt	.blockl		;links ausserhalb
		cmp	cx2,d0
		bgt	.blockl		;rechts ausserhalb
;-- sichtbar !
		move.l	a4,4(a3)	;adresse speichern
		addq	#1,dface_anz
		move.w	#$face,(a4)+	;flag fuer wand
		move	color,(a4)+	;farbe
		move.l	d6,(a4)+
		move.l	d7,(a4)+
		move	d0,(a4)+	;x1
		move	d1,(a4)+	;x2
		add	d7,d6
		asr	#1,d6
		ext.l	d6
		move.l	d6,(a3)		;entfernung speichern
		addq.l	#8,a3
		bra	.blockr		;gegenueberliegende flaeche ueberspringen
.blockl:
;--- rechts ---
		cmp.b	#" ",1(a0)
		bne	.blockr
;--- 2d-koos laden ---
		move	drawsizex,d7
		addq	#1,d7
		asl	#3,d7
		move	8(a1),d0	;x1
		move	8(a1,d7.w),d1	;x2
;-- 3d-koos laden
		asr	#1,d7		;3d-koos holen
		move.l	4(a2),d6
		move.l	4(a2,d7.w),d7
		tst.w	d6
		bpl.s	.ok3d4
		tst.w	d7
		bmi	.blockr
.ok3d4:
;-- sichtbar ?
		cmp	d0,d1
		blt	.blockr		;-> hidden
		cmp	cx1,d1
		blt	.blockr		;links ausserhalb
		cmp	cx2,d0
		bgt	.blockr		;rechts ausserhalb
;-- sichtbar !
		move.l	a4,4(a3)	;adresse speichern
		addq	#1,dface_anz
		move.w	#$face,(a4)+	;flag fuer wand
		move	color,(a4)+	;farbe
		move.l	d6,(a4)+
		move.l	d7,(a4)+
		move	d0,(a4)+	;x1
		move	d1,(a4)+	;x2
		add	d7,d6
		asr	#1,d6
		ext.l	d6
		move.l	d6,(a3)		;entfernung speichern
		addq.l	#8,a3
.blockr:
gf_nextblock:
		move	(sp)+,d7
		cmp	#list_size,dface_anz
		bge	.break
		addq	#1,a0
		addq.l	#8,a1	;2d-koos weiter
		addq.l	#4,a2	;3d-koos weiter
		dbra	d7,gf_xloop
		addq.l	#4,a2
		addq.l	#8,a1
		move.l	(sp)+,a0
		sub.w	dsizex,a0	;naechste zeile im dungeon
		move	(sp)+,d6
		dbra	d6,gf_yloop
		rts
.break:
		move.l	(sp)+,a0
		move	(sp)+,d6
		rts

;*********************************
;*** ascii in farbcode wandeln ***
;*********************************
conv_tocolor:
		move.l	a0,-(sp)
		move.l	colorcodes,a0
;		lea	testc,a0
		moveq	#0,d2
.lp:
		move.b	(a0)+,d1
		beq.s	.schulz
		cmp.b	d0,d1
		beq.s	.take
		addq	#1,d2
		bra.s	.lp
.schulz:
		moveq	#0,d2
.take:
		move	d2,color
		move.l	(sp)+,a0
		rts


testc:		dc.b	"FfMm#TWcCb01B",0







;********************************************
;****** rotiertes punktgitter erzeugen ******
;********************************************

; &&& changed %%%

make_rotatedpoints:
;d0-d3 -> begrenzungsrechteck
		and	#-patt_size,d0
		and	#-patt_size,d1
		and	#-patt_size,d2	;?
		and	#-patt_size,d3	;?
		move	d0,d4		;minx -> startx
		move	d1,d5		;miny -> starty
		sub	d0,d2
		sub	d1,d3
		moveq	#psize_log,d6
		lsr	d6,d2
		lsr	d6,d3
;		addq	#1,d2		;einschliesslich !!
;		addq	#1,d3		;
		move	d2,drawsizex
		move	d3,drawsizey
		lsr	d6,d4
		lsr	d6,d5
		move	d4,drawstartx
		move	d5,drawstarty
		sub	player_x,d0
		sub	player_y,d1
;d0-d1 -> relative start-koo im dungeon
;d2-d3 -> anzahl punkte
;--- punkte-array fuellen ---
		move	d3,d6
		move	d2,d7
		addq	#1,d6
		addq	#1,d7
		mulu	d6,d7	;anzahl punkte
		lea	rot_points,a2
.yloop:
		move	d0,d4
		move	d1,d5
		move	d2,d6	;zaehler
.xloop:
		move	d4,(a2)+
		move	d5,(a2)+
		add	#patt_size,d4
		dbra	d6,.xloop
		add	#patt_size,d1
		dbra	d3,.yloop
;--- punkte-array rotieren ---
		lea	rot_points,a0
		move.l	a0,a1
		clr.w	xoffset
		clr.w	zoffset
		move	d7,-(sp)
		bsr	rotate_y
		move	(sp)+,d7
;--- punkte-array projezieren ---
		lea	rot_points,a0
		lea	proj_points,a1
		move	lower_y,d5
		move	upper_y,d6
		ext.l	d5
		ext.l	d6
		asl.l	#proj_log,d5		;y1*zp
		asl.l	#proj_log,d6		;y2*zp
		neg.l	d5
		neg.l	d6
		subq	#1,d7
.proj_loop:
		move.w	(a0)+,d0
		move.w	(a0)+,d1
		cmp	#-250,d1
		ble.s	.aga
		ext.l	d0
		asl.l	#proj_log,d0		;x*zp
		add	#proj_faktor,d1		;z+zp
		divs	d1,d0
		add	scrx,d0
		move.l	d5,d2			;y*zp
		move.l	d6,d3
		divs	d1,d2			;/(z+zp)
		divs	d1,d3
		add	scry,d2
		add	scry,d3
		move	d0,(a1)+		;x
		move	d2,(a1)+		;y_low
		move	d0,(a1)+		;x
		move	d3,(a1)+		;y_high
		dbra	d7,.proj_loop
		rts
.aga:
		add	scrx,d0
		move	d0,(a1)+
		clr.w	(a1)+
		move	d0,(a1)+
		clr.w	(a1)+
		dbra	d7,.proj_loop
		rts


;*****************************************
;************** quicksort ****************
;*****************************************
quicksort:
;a0 = tabelle		%%%
;d7 = anzahl werte
;d6 = laenge eines eintrages

		move.l	a0,a1
		move	d7,d0
		subq	#1,d0
		mulu	d6,d0
		add.w	d0,a1	;-> letzter eintrag

		move	d6,d4
		mulu	#9,d4	;ab 9 werte
;		mulu	d7,d4	;immer bubblesort
;d4.l = differenz ab der bubble-sort aktiv wird

quicksort_call:
		cmp.l	a0,a1
		ble	quicksort_leave
		move.l	a1,d0
		sub.l	a0,d0
		cmp.l	d4,d0
		ble	qs_bubbleit
		move.l	a0,a2
		move.l	a1,a3
		move.l	(a0),d7
		add.l	(a1),d7
		move.l	d7,d5
		addq.l	#1,d5
		asr.l	#1,d5
		asr.l	#1,d7		;-> mittelwert
quicksort_loop0:
quicksort_loop1:
		cmp.l	(a0),d5
		ble.s	qs_get1
		add.l	d6,a0
		bra	quicksort_loop1
qs_get1:
quicksort_loop2:
		cmp.l	(a1),d7
		bge.s	qs_get2
		sub.l	d6,a1
		bra	quicksort_loop2
qs_get2:
		cmp.l	a1,a0
		bge	quicksort_part
;--- vertauschen ---
		move.l	(a0),d0
		move.l	(a1),(a0)
		move.l	d0,(a1)
		move.l	4(a0),d0
		move.l	4(a1),4(a0)
		move.l	d0,4(a1)
		add.l	d6,a0
		sub.l	d6,a1
		cmp.l	a0,a1
		bgt	quicksort_loop0
quicksort_part:
;--- rekursionen durchfuehren ---
;a2 - a1
		movem.l a0-a3,-(sp)
		move.l	a2,a0
		bsr	quicksort_call
		movem.l (sp)+,a0-a3
;a0 - a3
;		movem.l a0-a3,-(sp)
;		move.l	a1,a0
		move.l	a3,a1
		bsr	quicksort_call
;		movem.l (sp)+,a0-a3
quicksort_leave:
		rts
 

;(einfuege-sort)
;*******************
;*** bubble-sort ***
;*******************
qs_bubbleit:
;a0 = start
;a1 = ende (inc.)
		move.l	a0,a2
qsb_loop0:
;*** 2 zahlen suchen die falsch sind ***
qsb_loop1:
		move.l	(a0),d0
		add.l	d6,a0
		cmp.l	(a0),d0
		bgt.s	qsb_toobig
		cmp.l	a0,a1
		bne.s	qsb_loop1
		bra	qsb_ready
qsb_toobig:
		move.l	(a0),d0
		move.l	4(a0),d1
		move.l	a0,a3
;*** solange zurueckgehen bis kleinere zahl gefunden ***
qsb_loop2:
		sub.l	d6,a0
		cmp.l	a2,a0
		blt.s	qsb_next
		move.l	(a0),(a0,d6.l)
		move.l	4(a0),4(a0,d6.l)
		cmp.l	(a0),d0
		blt.s	qsb_loop2	
qsb_next:
		move.l	d0,(a0,d6.l)
		move.l	d1,4(a0,d6.l)
		move.l	a3,a0
		cmp.l	a0,a1
		bne.s	qsb_loop0
qsb_ready:
		rts

;***********************************
;*** rotation nur um die y-achse ***
;***********************************
rotate_y:
;a0 = koos		%%%
;a1 = puffer zum speichern
;d7 = anzahl koos

		subq	#1,d7
		bmi	no_rotatey
		lea	sintab,a2
		and	#slang-1,y_angel
		move	y_angel,d6
		neg.w	d6
		add	#slang,d6
		and	#slang-1,d6
		add	d6,d6
		move.w	(a2,d6.w),d0	;sin(yw)
		add.w	#slang/2,d6
		and.w	#slang*2-1,d6
		move.w	(a2,d6.w),d1	;cos(yw)
		move.w	xoffset,a2
		move.w	zoffset,a3
rotatey_loop:
		move.w	(a0)+,d2	;x
		move.w	(a0)+,d3	;y
		move	d2,d4
		move	d3,d5
		muls	d0,d2	;x*sin(yw)
		muls	d0,d3	;y*sin(yw)
		muls	d1,d4	;x*cos(yw)
		muls	d1,d5	;y*cos(yw)
		sub.l	d3,d4
		add.l	d2,d5
		asl.l	#2,d4
		asl.l	#2,d5
		swap	d4
		swap	d5
		add.w	a2,d4	;+xoffset
		add.w	a3,d5	;+zoffset
		move.w	d4,(a1)+
		move.w	d5,(a1)+
		dbra	d7,rotatey_loop
no_rotatey:
		rts

;*************************************************
;*** rotation nur um die y-achse (nur 1 punkt) ***
;*************************************************
rotate_y1:
;d0-d2 -> 3d-koos	%%%
;d3 = y_angel

		movem.l	d3-d6/a2,-(sp)

		lea	sintab,a2
		move	d3,d6
		neg.w	d6
		add	#slang,d6
		and	#slang-1,d6
		add	d6,d6
		move.w	(a2,d6.w),d3	;sin(yw)
		add.w	#slang/2,d6
		and.w	#slang*2-1,d6
		move.w	(a2,d6.w),d4	;cos(yw)

		move	d0,d5
		move	d2,d6
		muls	d3,d5	;x*sin(yw)
		muls	d3,d6	;y*sin(yw)
		muls	d4,d0	;x*cos(yw)
		muls	d4,d2	;y*cos(yw)
		sub.l	d6,d0
		add.l	d5,d2
		asl.l	#2,d0
		asl.l	#2,d2
		swap	d0
		swap	d2
		ext.l	d0
		ext.l	d1
		ext.l	d2
		movem.l	(sp)+,d3-d6/a2
		rts

;**************************************************************
;****** groesse des zu verwaltenden rechteckes ermitteln ******
;**************************************************************
get_drawsize:

; &&& changed %%%

;--- sin und cos holen ---
		lea	sintab,a0
		move	y_angel,d1
		and	#slang-1,d1
		add	d1,d1
		move.w	(a0,d1.w),d0	;sin(yw)
		add	#slang/2,d1
		and	#slang*2-1,d1
		move.w	(a0,d1.w),d1	;cos(yw)
;--- vorderen punkte erzeugen ---
		lea	view_points,a0
		move	d0,d2
		move	d1,d3
		muls	#scr_xsize/2,d2	;sin(yw)*x
		muls	#scr_xsize/2,d3	;cos(yw)*x
		asl.l	#2,d2
		swap	d2
		asl.l	#2,d3
		swap	d3
		move	d0,d4
		move	d1,d5
		muls	#proj_faktor,d4	;sin(yw)*zp
		muls	#proj_faktor,d5	;cos(yw)*zp
		asl.l	#2,d4
		swap	d4
		asl.l	#2,d5
		swap	d5
		neg.w	d5		;-cos(yw)*zp
;-- punkt #1
		neg.w	d2
		move	d3,(a0)+	;cos(yw)*x
		move	d2,(a0)+	;-sin(yw)*x
;-- punkt #2
		move	d3,d6
		move	d2,d7
		sub	d4,d6		;deltax
		sub	d5,d7		;deltay
		muls	#patt_size/30,d6	;pattsize-abhaengig
		muls	#patt_size/30,d7
		add	d4,d6
		add	d5,d7
		move	d6,(a0)+
		move	d7,(a0)+
;-- punkt #3
		neg.w	d2
		neg.w	d3
		move	d3,(a0)+	;-cos(yw)*x
		move	d2,(a0)+	;sin(yw)*x
;-- punkt #4
		move	d3,d6
		move	d2,d7
		sub	d4,d6		;deltax
		sub	d5,d7		;deltay
		muls	#patt_size/30,d6	;pattsize-abhaengig
		muls	#patt_size/30,d7	;pattsize-abhaengig
		add	d4,d6
		add	d5,d7
		move	d6,(a0)+
		move	d7,(a0)+
;--- minx,miny,maxx,maxy suchen ---
		lea	view_points,a0
		move	#$7fff,d0	;minx
		move	#$7fff,d1	;miny
		moveq	#0,d2		;maxx
		moveq	#0,d3		;maxy
		moveq	#4-1,d7
.gv_loop:
		movem.w	(a0)+,d4-d5
;-- minx
		cmp	d4,d0
		ble.s	.new0
		move	d4,d0
.new0:
;-- miny
		cmp	d5,d1
		ble.s	.new1
		move	d5,d1
.new1:
;-- maxx
		cmp	d4,d2
		bge.s	.new2
		move	d4,d2
.new2:
;-- maxy
		cmp	d5,d3
		bge.s	.new3
		move	d5,d3
.new3:
		dbra	d7,.gv_loop
		add	#patt_size-1,d2	;runden
		add	#patt_size-1,d3
		add	player_x,d0	;minx
		add	player_y,d1	;miny
		add	player_x,d2	;maxx
		add	player_y,d3	;maxy
;--- bereich clippen ---
		tst	d0
		bpl.s	.clip1
		moveq	#0,d0
.clip1:
		tst	d1
		bpl.s	.clip2
		moveq	#0,d1
.clip2:
		cmp	dsizex2,d2
		ble.s	.clip3
		move	dsizex2,d2
.clip3:
		cmp	dsizey2,d3
		ble.s	.clip4
		move	dsizey2,d3
.clip4:
		rts

	rsreset
dpk_x1:		rs.w 1
dpk_x2:		rs.w 1
dpk_y1:		rs.w 1
dpk_y2:		rs.w 1
dpk_z1:		rs.w 1
dpk_z2:		rs.w 1


; STOP HERE %%%

;*************************************
; [ draw dungeon-polygon ]
;
;*************************************
put_dpoly:
		lea	walls_gfx,a0
		move	color,d0
		mulu	#128*80,d0
		add.l	d0,a0
		move.w	#128,dpoly_sizex	;now variable !
		move.w	#80,dpoly_sizey		;
		move.l	a0,dpoly_gfx

		lea	dpoly_koos3d,a0
;****** 3d-punkte clippen und projezieren ******
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
		move	dpoly_sizex,d0	;groesse des originals
		subq	#1,d0		;-> last pixel
		swap	d0
		clr.w	d0		;16 bit nachkomma
;*** 3d-clip beruecksichtigen ***
		moveq	#0,d5		;default_startx

		movem.w	dpoly_savez,d1-d2	;z1,z2
		move.l	d2,d3
		sub.l	d1,d3		;delta_z
		bpl.s	.dz_ok
		neg.l	d3
.dz_ok:
;--- erster punkt ? ---
		tst	d1
		bpl.s	.normal_z1
		neg.l	d1		;d1 einheiten ueberspringen
		muls.l	d0,d4:d1
		divs.l	d3,d4:d1	;-> soviel pixel ueberspringen
		sub.l	d1,d0		;source verkuerzen
		move.l	d1,d5		;-> start_x
		bra	.normal_z2
.normal_z1:
;--- zweiter punkt ? ---
		tst	d2
		bpl.s	.normal_z2
		neg.l	d2		;d2 einheiten ueberspringen
		muls.l	d0,d4:d2
		divs.l	d3,d4:d2	;-> soviel pixel weglassen
		sub.l	d2,d0		;source verkuerzen
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
		add.w	#proj_faktor*2,d2	;(z+zp)*2
		ext.l	d1
		add.l	d1,d1			;<<1
		asl.l	#proj_log,d1		;x*zp*2*2
		ext.l	d2
		beq.s	.no_div1
		divs.l	d2,d1			;/((z+zp)*2)
.no_div1:		move	scrx,d2
		ext.l	d2
		add.l	d2,d2
		add.l	d2,d1			;-> x<<1
		move	dpoly_xleft,d2
		move	dpoly_xright,d3
		add	d2,d2
		add	d3,d3
		sub	d2,d3			;-> b*2 (1bit nachkomma)
		sub	d2,d1			;-> q*2
;--- check for f...ing overflow ---
		move	d3,d2
		ext.l	d2
		divu	#3,d2			;b/3
		cmp	d2,d1			;q>b/3 ?
		bgt.s	.qbok1
		move	d2,d1
		addq	#1,d1
.qbok1:		neg	d2
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
		divs.l	d3,d2:d0		;-> f<<16
.no_div3:
		move.l	d0,d6			;c*f (c=1)
		muls.l	d0,d1:d4		;s*f 00|ii|FF|ff
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
		add	d0,d7		;shrink
		bmi	dpoly_skipall	;-> rechts ganz draussen
.no_clipright:
;--- links ?
		move	dpoly_xleft,d0
		sub	cx1,d0
		bge.s	.no_clipleft
		add	d0,d7		;shrink
		bmi	dpoly_skipall	;-> links ganz draussen
		move	cx1,dpoly_xleft
		neg.w	d0		;-> soviel spalten ueberspringen

		ext.l	d0
		move.l	d0,d1
		move.l	d0,d2

		mulu	d0,d0
		muls.l	d4,d0		;x*x*s
		muls.l	d6,d1		;x*c
		add.l	d1,d0
		add.l	d1,d0		;x*x*s+x*c*2
		asr.l	#1,d0		;/2
		add.l	d0,d5		;ueberspringen

		muls.l	d4,d2		;x*s
		add.l	d2,d6		;c=c+x*s

.no_clipleft:

;d4 = xvec_s
;d5 = startx
;d6 = xvec_c
;d7 = anzahl spalten

		move.l	d4,d0
		asr.l	#1,d0
		add.l	d0,d6		;c=c+s/2  (integral-correction)

		move.l	d6,dpoly_mxc
		move.l	d4,dpoly_mxs

		move.l	workbase,a1
		moveq	#0,d3
		move	dpoly_xleft,d0
		add.w	d0,a1		;-> screen_ptr

		lea	dpoly_ykoos,a3

		tst.l	dpoly_clipflag
		beq	xdpnc_loop
;********* zeichnen mit clipping *********
xdp_loop:
;*** get source-x ***
		move.l	d5,d0
		swap	d0
		muls	dpoly_sizey,d0
		move.l	dpoly_gfx,a0
		add.l	d0,a0		;-> adresse der source-gfx

;*** y-zoom holen ***
		move.l	(a3)+,d1	;y_oben
		move.l	(a3)+,d2	;y_unten
		move.l	d2,d4
		sub.l	d1,d4		;deltay-real
		swap	d1
		swap	d2
;------ oben/unten-clipping ------
;-- oben
		moveq	#0,d6
		cmp	cy2,d1		;cy2
		bgt	.skip		;unten raus
		move	d1,d0
		sub	cy1,d0		;cy1
		bge.s	.no_clipup
		neg.w	d0		;-> pixel oben raus
		move	d0,d6		;merken
		move	cy1,d1		;cy1
.no_clipup:
;-- unten
		cmp	cy1,d2		;cy1
		blt	.skip		;oben raus
		cmp	cy2,d2		;cy2
		ble.s	.no_clipdown
		move	cy2,d2		;cy2
.no_clipdown:
		move.l	(zvals_stab,d2.w*4),a5

		sub	d1,d2		;-> anzahl zeilen -1
		addq	#1,d2
;*** screen-pointer ***
		move.l	#256,d0		;screen_breite
		move	d1,d3
		mulu	d0,d3
		move.l	a1,a4
		add.l	d3,a4
		move.l	d0,a6

		add.l	#$10000,d4
		move.w	dpoly_sizey,d0
;		subq	#1,d0
		moveq	#0,d3
		divu.l	d4,d0:d3	;-> vector

		clr.w	d1		;nkomma_y1 <<16
		mulu.l	d3,d4:d1
		move.l	d4,d1		;-> offset
		not.l	d1
		add.l	d3,d1

		tst	d6
		beq.s	.no_yclip
		ext.l	d6
		mulu.l	d3,d6
		add.l	d6,d1
.no_yclip:
		move.l	d1,d4
		swap	d4
		move.w	d3,a2
		swap	d3
;d1 = y.lo
;d3 = adder.hi
;d4 = y.hi
;a2 = adder.lo

		moveq	#3,d0
		and	d2,d0		;and #3,d1
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
		move.b	(a0,d4.w),d0	;load pixel-byte
		add.w	a2,d1		;calc step
		addx.w	d3,d4		;
		move.b	(a5,d0.w),(a4)
		add.l	a6,a4		;to next line
	ENDR
		dbra	d2,.yloop

.skip:
;****** x-zoom weiterrechnen ******
		add.l	dpoly_mxc,d5	;move X_source
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
		add.l	d0,a0		;-> adresse der source-gfx

;*** y-zoom holen ***
		move.l	(a3)+,d1	;y_oben
		move.l	(a3)+,d2	;y_unten
		move.l	d2,d4
		sub.l	d1,d4		;deltay-real
		swap	d1
		swap	d2
		move.l	(zvals_stab,d2.w*4),a5
		sub	d1,d2		;-> anzahl zeilen -1
		addq	#1,d2
;*** screen-pointer ***
		move.l	#256,d0		;screen_breite
		move	d1,d3
		mulu	d0,d3
		move.l	a1,a4
		add.l	d3,a4
		move.l	d0,a6

		add.l	#$10000,d4
		move.w	dpoly_sizey,d0
;		subq	#1,d0
		moveq	#0,d3
		divu.l	d4,d0:d3	;-> vector

		clr.w	d1		;nkomma_y1 <<16
		mulu.l	d3,d4:d1
		move.l	d4,d1		;-> offset
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
		and	d2,d0		;and #7,d1
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
		move.b	(a0,d4.w),d0	;load pixel-byte
		add.w	a2,d1		;calc step
		addx.w	d3,d4		;
		move.b	(a5,d0.w),(a4)	;put pixel
		add.l	a6,a4		;to next line
	ENDR
		dbra	d2,.yloop

.skip:
;****** x-zoom weiterrechnen ******
		add.l	dpoly_mxc,d5	;move X_source
		move.l	dpoly_mxs,d0
		add.l	d0,dpoly_mxc
;***** naechste spalte ******
		addq.l	#1,a1

		dbra	d7,xdpnc_loop
		rts


;*************************************
; [ generate 2d-koos ]
;
;*************************************
make_dpoly_projection:
		lea	dpoly_koos2d,a1
		lea	dpoly_clipflag,a2
		move	scrx,d4
		move	scry,d5
		move.w	dpk_z1(a0),d0
		add.w	#proj_faktor,d0		;z1+zp
;--- x1 erzeugen ---
		move	dpk_x1(a0),d2
		ext.l	d2
		asl.l	#proj_log,d2
		divs	d0,d2
		add.w	d4,d2			;+scrx
		move.w	d2,(a1)+		;x1_2d
;--- y1 erzeugen ---
		move	dpk_y1(a0),d2
		ext.l	d2
		asl.l	#proj_log,d2
		divs	d0,d2
		neg.w	d2
		add.w	d5,d2			;+scry
		move.w	d2,(a1)+		;y1_2d
		cmp	cy2,d2
		sgt	(a2)+
;--- y2 erzeugen ---
		move	dpk_y2(a0),d2
		ext.l	d2
		asl.l	#proj_log,d2
		divs	d0,d2
		neg.w	d2
		add.w	d5,d2			;+scry
		move.w	d2,(a1)+		;y2_2d
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
		move.w	d2,(a1)+		;x2_2d
;--- y3 erzeugen ---
		move	dpk_y1(a0),d2
		ext.l	d2
		asl.l	#proj_log,d2
		divs	d0,d2
		neg.w	d2
		add.w	d5,d2			;+scry
		move.w	d2,(a1)+		;y3_2d
		cmp	cy2,d2
		sgt	(a2)+
;--- y4 erzeugen ---
		move	dpk_y2(a0),d2
		ext.l	d2
		asl.l	#proj_log,d2
		divs	d0,d2
		neg.w	d2
		add.w	d5,d2			;+scry
		move.w	d2,(a1)+		;y4_2d
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
.zc_allout:	moveq	#-1,d0
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
		sub	d0,d3		;deltax
		addq	#1,d3
		move	d3,d7		;anzahl spalten
;--- steigungen berechnen ---
		ext.l	d3
;unten
		sub	d1,d4		;deltay_down
		swap	d4		;<< 16
		clr.w	d4
		divs.l	d3,d4		;dy/dx
;oben
		sub	d2,d5		;deltay_up
		swap	d5		;<< 16
		clr.w	d5
		divs.l	d3,d5		;dy/dx
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
		bmi	.cancel		;-> ganz draussen
.no_clipright:
;--- links-clipping ---
		move	dpoly_xleft,d6
		sub	cx1,d6
		bge.s	.no_clipleft
		add	d6,d7
		bmi	.cancel		;-> ganz draussen
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


	cnop	0,4		;long_aligned
dpoly_ykoos:	ds.l scr_xsize*2
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
		bpl	.no_clip	;-> no zclip necessary
.do_clip:
;--- get last point ---
		subq	#1,d7
		move	d7,d0
		mulu	#10,d0
		lea	(a0,d0.w),a1	;-> pointer to previous
;--- loop ---
		lea	newpoly_koos,a2	; save new poly here
		clr.w	clip_count
.cliptm_loop:
		movem.w	4(a1),d0-d2	; x1,z1,y1
		movem.w	4(a0),d3-d5	; x2,z2,y2
		move	d1,d6
		eor	d4,d6		; any sign-change ?
		bpl.s	.may_save2	;-> NO !
;--- 3D-schnitt-punkt erzeugen ---
		sub	d0,d3		;delta_x
		sub	d1,d4		;delta_z
		sub	d2,d5		;delta_y
		neg.w	d4
		muls	d1,d3		;z1*(x2-x1)
		divs	d4,d3		;/(z1-z2)
		add	d0,d3		;+x1
		muls	d1,d5		;z1*(y2-y1)
		divs	d4,d5		;/(y1-y2)
		add	d2,d5		;+y1
		move	d3,d6
		add	scrx,d6
		move.w	d6,(a2)+	;save x_2d
		move	d5,d6
		neg.w	d6
		add	scry,d6
		move.w	d6,(a2)+	;save y_2d
		addq.l	#6,a2		;3D-data not needed
		addq	#1,clip_count
.may_save2:
;--- eventuell 2ten punkt speichern ---
		tst.w	6(a0)		; Z2 positive ?
		bmi.s	.next		;-> NO !
		move.l	(a0),(a2)+	; copy X_2d,Y_2d
		addq.l	#6,a2		;3D-data not needed
		addq	#1,clip_count
.next:		move.l	a0,a1		; next previous point
		add.w	#10,a0
		dbra	d7,.cliptm_loop
		move.l	(a0)+,(a2)+	; source_gfx
		move	clip_count,d7
		lea	newpoly_koos,a0	; new polygon
.no_clip:	rts

clip_count:	ds.w	1
newpoly_koos:	ds.w	12*5

min_y:		ds.w	1
max_y:		ds.w	1
kanten_links:	ds.w	scr_ysize
kanten_rechts:	ds.w	scr_ysize
tm_x2d:		ds.w	1
tm_y2d:		ds.w	1
tm_x2d2:	ds.w	1
tm_y2d2:	ds.w	1
tm_x2d_vec:	ds.l	1
tm_ycount:	ds.w	1
tm_deltax:	ds.w	1
oben_skip:	ds.w	1
unten_skip:	ds.w	1

;Limit: Hlines paralell zu X-Achse !
;*************************************
; [ Texture mapping zeichnen ]
; a0 = ptr to koos
; d7 = anzahl punkte
;
;*************************************
draw_newTM:
;*****************************
;****** kanten aufbauen ******
;*****************************
		move	#-32768,max_y
		move	#32767,min_y
		subq	#1,d7
		move.l	a0,a6		;for last
.kanten_loop:
;****** get 2D ******
		move.w	(a0)+,d0	;x1
		move.w	(a0)+,d1	;y1
		addq.l	#6,a0		;skip 3D
		move.l	a0,a1
		tst	d7
		bne.s	.nlast1
		move.l	a6,a1
.nlast1:	move.w	(a1)+,d2	;x2
		move.w	(a1)+,d3	;y2
;d0-d3 = 2d-linie
;--- make deltas ---
		moveq	#0,d6
		lea	kanten_rechts,a3
		sub	d0,d2		;delta_x
		sub	d1,d3		;delta_y
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
		swap	d2		;<< 16
		clr.w	d2
		ext.l	d3
		divs.l	d3,d2		;dx/dy
;d0 = x-start
;d1 = y-start
;d2 = x-vec
;d3 = dy-1
		move.l	d2,tm_x2d_vec

;****** do 2d-clipping ******
		movem.w	tm_x2d,d0-d3	;x1,y1,x2,y2
		clr.w	oben_skip
		clr.w	unten_skip
;*** y-clip checken ***
		move	d1,d4
		sub	cy1,d4
		bge.s	.no_topclip
		sub	d4,oben_skip	;anzahl pixel oben raus
		move	cy1,d1		;change Y
.no_topclip:
		move	d3,d4
		sub	cy2,d4
		ble.s	.no_downclip
		add	d4,unten_skip	;anzahl pixel unten raus
		move	cy2,d3		;change Y
.no_downclip:
		move	tm_ycount,d4
		sub	oben_skip,d4
		sub	unten_skip,d4
		bmi	.skip_kante	;-> kante nicht im Y-bereich
;*** check min_y & max_y ***
		cmp	min_y,d1
		bge.s	.keep_miny
		move	d1,min_y
.keep_miny:
		cmp	max_y,d3
		ble.s	.keep_maxy
		move	d3,max_y
.keep_maxy:
		add	d1,d1		;use Y1
		add.w	d1,a3		;pos in tab
;*** calc the line ***
		move.w	tm_x2d,d0	;load start_x
		swap	d0		;<< 16
		clr.w	d0
		move.l	tm_x2d_vec,a4	;x_vector
		moveq	#0,d3
		move	oben_skip,d3
		beq	.load_normal	;-> do fast loading
;--- clip X ---
		move.l	tm_x2d_vec,d1	;load x_vector
		muls.l	d3,d1
		add.l	d1,d0		;jump x-stuff
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
		lea	ntm_constants,a2
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
		add.w	d5,a1		;ptr to svals
		move	d6,d5
		lsl	#4,d5
		add.w	d5,a3
		mulu	#256,d6		;...
		move.l	workbase,a4
		add.l	d6,a4
		move.l	(a0)+,a0	;ptr to gfx
;a0 = source_gfx
;a4 = screen_ptr
;a5 = kanten links
;a6 = kanten rechts
;d7 = dbra-counter
NTM_hlineloop:
		pea	256(a4)
		pea	4(a1)
		move.w	(a5)+,d0	;screen_x1
		move.w	(a6)+,d1	;screen_x2
		move.l	a3,a2
		add.w	#16,a3
;--- x-clip ? ---
		cmp	cx2,d0
		bgt	.skip_hline
		cmp	cx1,d1
		blt	.skip_hline
		cmp	cx1,d0		;links-clip ?
		bge.s	.no_lclip
		move	cx1,d0
.no_lclip:	cmp	cx2,d1		;rechts clip ?
		ble.s	.no_rclip
		move	cx2,d1
.no_rclip:
		sub	d0,d1		;screen_deltax
		addq	#1,d1
		ble	.skip_hline
		move	d7,-(sp)

		move.l	(a2)+,d2	;start_sourcex
		move.l	(a2)+,d4	;start_sourcey
		move.l	(a2)+,d3	;dest_sourcex
		move.l	(a2)+,d5	;dest_sourcey
		add.w	d0,a4		;screen_ptr
		sub.l	d2,d3		;deltax
		sub.l	d4,d5		;deltay
		asr.l	#8,d3		;xvector
		asr.l	#8,d5		;yvector

		move.l	d3,d6
		move.l	d5,d7
		ext.l	d0
		muls.l	d0,d6		;skip pixels
		muls.l	d0,d7		;
		add.l	d6,d2
		add.l	d7,d4

		lea	ntm_constants(pc),a2
		add.l	(a2),d2		;$200000
		add.l	(a2)+,d4
		and.l	(a2),d2		;64-er pattern
		and.l	(a2)+,d4	;$3fffff

		tst.l	d3
		bpl.s	.xv_pos
		add.l	(a2),d2		;goto other side +$400000
.xv_pos:	tst.l	d5
		bpl.s	.yv_pos
		add.l	(a2),d4		;goto other side
.yv_pos:

		move.l	(a1),a1		;-> shading
;--- init paras ---
		move.l	d2,d0		;xwork-LO
		move.w	d3,a2		;xadd-LO
		move.w	d4,d0
		asl.w	#8,d0		;xwork_hi <= ywork_megaLO
		move.w	d5,d3
		asl.w	#8,d3		;xadd_hi <= yadd_megaLO
		swap	d0		;xwork-HI
		swap	d3		;xadd-HI
		asr.l	#8,d4		;16 bit -> 8 bit nkomma
		asr.l	#8,d5		;16 bit -> 8 bit nkomma
;d0 = x-HI (HI-word =y_LO)
;d2 = x-LO
;d3 = xadd-HI (HI-word = yadd_LO)
;d4 = ywork ( 8 bit fractional )
;a2 = xadd-LO
;d5 = yadd ( 8 bit fractional )
		moveq	#3,d7
		and	d1,d7		;and #3,d1
		lsr	#2,d1
		mulu	#18,d7
		neg.w	d7
		add.w	#4*18,d7
;--- draw it ! ---
		moveq	#0,d6
		jmp	.xloop(pc,d7.w)
.xloop:
	REPT	4
		move.w	d4,d7
		move.b	d0,d7		;!!
		move.b	(a0,d7.l),d6	;get pixel

		add.w	a2,d2		;x.LO
		addx.l	d3,d0		;x.HI ( und Y_LO )

		addx.w	d5,d4		;y_step (8bit)
		move.b	(a1,d6.w),(a4)+	;put pixel
	ENDR
		dbra	d1,.xloop

		move	(sp)+,d7
;--- next hline ---
.skip_hline:	move.l	(sp)+,a1
		move.l	(sp)+,a4	;next line !
		dbra	d7,NTM_hlineloop
skip_poly:
		rts

ntm_constants:	dc.l	$200000,$3fffff,$400000


;*************************************
; [ conv byte_screen to planes ]
;
;*************************************
convert_AGA:
	move.l	#$0f0f0f0f,a2
	move.l	#$f0f0f0f0,a3
	move.l	#$3333cccc,a4
	move.l	#$55005500,a5
	move.l	#$00aa00aa,a6
;--- zeile anzeigen ---
	lea	byte_screen,a0
	lea	fast_vscreen,a1
	move	#scr_ysize-1,d7
.pp_yloop:
	move	d7,-(sp)
	move.l	a1,-(sp)
	move	#scr_xsize/8-1,d7
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
	move.b	d1,32(a1)
	lsr.l	#8,d1
	move.b	d1,64(a1)
	lsr.w	#8,d1
	move.b	d1,96(a1)

	move.b	d0,128(a1)
	lsr.l	#8,d0
	move.b	d0,160(a1)
	lsr.l	#8,d0
	move.b	d0,192(a1)
	lsr.w	#8,d0
	move.b	d0,224(a1)

	addq.l	#1,a1

	dbra	d7,.pp_xloop
	move.l	(sp)+,a1
	move	(sp)+,d7
	lea	256(a1),a1
	dbra	d7,.pp_yloop
;*** 32-bittig runterkopieren ***
	lea	fast_vscreen,a0
	move.l	workbase,a1
	move	#scr_ysize*8-1,d7
.c32_loop:
	REPT	8
	move.l	(a0)+,(a1)+
	ENDR
	lea	40-32(a1),a1
	dbra	d7,.c32_loop
	rts


		data

sintab:		incbin	sin.tab

shade_tab:	incbin	dung2.shd

colorcodes:	ds.l	1


	SECTION	Mike_BSS,bss_f
fast_vscreen:	ds.b	scr_xsize*scr_ysize
zvals_stab:	ds.l	scr_ysize	;ptr to lightshade
byte_screen:	ds.b	scr_xsize*scr_ysize
wall_list1:	ds.w	32000

view_points:	ds.l 4
rot_points:	ds.l 32*32
proj_points:	ds.l 32*32*2
clip_tab:	ds.l list_size+3+16+1
dminx:		ds.w 1
dminy:		ds.w 1
dmaxx:		ds.w 1
dmaxy:		ds.w 1
drawstartx:	ds.w 1
drawstarty:	ds.w 1
drawsizex:	ds.w 1
drawsizey:	ds.w 1
dsizex:		ds.w 1
dsizey:		ds.w 1
dsizex2:	ds.w 1
dsizey2:	ds.w 1
player_x:	ds.l 1
player_y:	ds.l 1
y_angel:	ds.l 1
head_height:	ds.w 1
head_height_old:	ds.w 1
dface_anz:	ds.w 1
xoffset:	ds.w 1
zoffset:	ds.w 1
lower_y:	ds.w 1
upper_y:	ds.w 1

	ds.w 1

dungeon_addr:	ds.l 1
wall_list0:	ds.l (list_size+3)*2+16*2	;sortiertabelle
wall_list2:	ds.w 16*6
wall_cliptab:	ds.w (list_size+3)*2+16*10	;wall_clips + 16 objekte

vindex:		ds.l 1

wall_flen:	ds.l	1
map_flen:	ds.l	1
dfile_ptr:	ds.l	1
wfile_ptr:	ds.l	1
decke_color:	ds.w	1
boden_color:	ds.w	1

scrx:		ds.w 1
scry:		ds.w 1
cx1:		ds.w 1
cy1:		ds.w 1
cx2:		ds.w 1
cy2:		ds.w 1

workbase:	ds.l 1

color:		ds.w 1
speed:		ds.w 1




