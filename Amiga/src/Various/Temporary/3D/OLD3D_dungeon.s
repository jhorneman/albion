; Dungeon display routines
; Written by Michael Bittner
; Adapted by J.Horneman (In Tune With The Universe)
; Start : 25-3-94

	XREF	Shade_table

	SECTION	Program,code
;*****************************************************************************
; [ Draw dungeon in chunky screen ]
; No registers are restored
; Notes :
;   - At this point, several pointers must have been claimed.
;*****************************************************************************
Draw_dungeon:
	clr.w	Visible_walls		; Reset counters
	clr.w	Visible_objects
	clr.w	Visible_floors
	clr.w	Visible_elements
	move.w	#0,cx1			; Reset clipping variables
	move.w	#0,cy1
	move.w	#Map3D_width-1,cx2
	move.w	#Map3D_height-1,cy2
; ---------- Set camera variables -----------------
	move.w	#165,head_height

	moveq.l	#0,d0			; Get camera height
	move.w	head_height,d0
	moveq.l	#psize_log,d1		; Convert from cm to 3D
	lsl.l	d1,d0
	divu.w	#Square_size,d0
	move.w	d0,d1			; Save
	neg.w	d0			; Set lower Y
	move.w	d0,lower_y
	move.w	Wall_height_3D,d0		; Get wall height - camera
	sub.w	d1,d0			;  height
	move.w	d0,upper_y		; Set upper Y

	move.w	head_height,d0		; Did camera Y change ?
	cmp.w	head_height_old,d0
	beq.s	.No
	move.w	d0,head_height_old		; Yes -> Recalculate
	jsr	Init_tm_coords
.No:	LOCAL
; ---------- Do it --------------------------------
	jsr	Clear_dungeon_screen	; Clear
	jsr	Get_draw_size		; Get view rectangle
	jsr	Make_rotated_points		; Create point grid
;	jsr	Insert_NPCs		; Insert NPCs
	jsr	Get_faces			; Create faces
	jsr	Calc_tm_coords
	lea	Dungeon_sort_list,a0	; Sort faces
	moveq.l	#8,d6
	move.w	Visible_elements,d7
	cmp.w	#1,d7			; Any ?
	ble.s	.No
	jsr	Quicksort
.No:	jsr	Draw_faces		; Draw faces
	rts

;*****************************************************************************
; [ Clear dungeon screen ]
; No registers are restored
;*****************************************************************************
Clear_dungeon_screen:
	tst.b	Lab_background_handle	; Background ?
	beq	.No

; Draw background

	bra	.Exit
.No:	move	scry,d6			; No -> Just clear
	move	scry,d7
	sub	#9,d6
	add	#9,d7
	sub	d6,d7
	move.l	Dungeon_screen,a0
	lsl	#8,d6
	add.w	d6,a0
	moveq	#0,d0
	addq	#1,d7
	lsl	#3,d7			; *256/32
	subq	#1,d7
.Loop2:
	REPT	8
	move.l	d0,(a0)+
	ENDR
	dbra	d7,.Loop2
.Exit:	rts

;*****************************************************************************
; [ Initialize y-values for tmap-coordinates ]
; No registers are restored
;*****************************************************************************
Init_tm_coords:
;****** zuerst den boden ******
;--- start_y ermitteln ---
	move	lower_y,d0		;0 = untere ebene
	neg.w	d0
	move	d0,beo_y			;-> real ypos
	ext.l	d0
	asl.l	#proj_log,d0
	add.l	#boden_maxz+proj_faktor-1,d0
	divs	#boden_maxz+proj_faktor,d0
	add	scry,d0
	move	d0,d6
	move	cy2,d7			;bottom
	sub	d0,d7			;anzahl zeilen -1
	sub	scry,d0			;in 3D-raum
	neg.w	d0			;uebernehmen
	add	beo_y,d0			;-> view_y
	move	d0,view_y
	move	#-1,viewy_dir
	move	d6,boden_y1
	move	d7,boden_ysize
	bsr	precalc_yvals
;****** dann auch die decke ******
;--- start_y ermitteln ---
	move	upper_y,d7		;0 = obere ebene
	move	d7,beo_y			;-> real ypos
	ext.l	d7
	asl.l	#proj_log,d7
	add.l	#boden_maxz+proj_faktor-1,d7
	divs	#boden_maxz+proj_faktor,d7
	neg.w	d7
	add	scry,d7
	move	cy1,d6			;top
	move	d6,d0
	sub	d0,d7			;anzahl zeilen -1
	sub	scry,d0			;in 3D-raum zurueck
	add	beo_y,d0			;-> view_y
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
	clr.w	d3			;EY << 16
	move	beo_y,d4
.kant_loop:
	move.l	d3,d5			;EY
	move	d1,d6			;WY
	sub	d4,d6
	beq.s	.no_div
	neg.w	d6			;EY-WY
	ext.l	d6
	divs.l	d6,d5			;EY/(EY-WY) << 16
.no_div:	move.l	d5,(a0)+
	add	d2,d1			;dec view_y
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
	move.l	(a5)+,d5			;EY/(EY-WY) << 16
	move.l	#-proj_faktor,d2		;-- LEFT
	moveq	#0,d3
;D2 = EZ
;D3 = WZ
;D5 = EY/(EY-WY)
	sub.l	d2,d3			;--Z : WZ-EZ
	muls.l	d5,d3			;*(EY/(EY-WY))
	swap	d3
	add.w	d2,d3			;EZ+""""

	sub	#512,d3
	bgt.s	.zok1
	moveq	#0,d3
.zok1:	add	d4,d3			;+shade-faktor
	move.l	d1,d0			;63*shade-faktor
	divu	d3,d0
	lsl	#8,d0			;*256 bytes

	lea	Shade_table+63*256,a1
;	add.w	d0,a1

	move.l	a1,(a0)+			;save ptr
	dbra	d7,.kant_loop
	rts

;*****************************************************************************
; [ Pre-calculate tmap-coordinates ]
; No registers are restored
;*****************************************************************************
Calc_tm_coords:
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
	move	Y_angle,d0
	move	#slang/4,d1
	add	d0,d1
	and	#slang-1,d1
	lea	Sine_table,a2
	move.l	a2,a3
	add	d0,d0
	add	d1,d1
	add.w	d0,a2			;sinus
	add.w	d1,a3			;cosinus
;--- precalc some stuff
	move	(a2),d1			;EX=sin*EZ  (EZ=-proj_faktor)
	ext.l	d1
	asr.l	#14-proj_log,d1
	move	(a3),d2			;EZ=EZ*cos  (EZ=-proj_faktor)
	ext.l	d2
	neg.l	d2
	asr.l	#14-proj_log,d2
;--
	sub	#16,sp
	moveq	#14,d4
	move	#-Map3D_width/2,d0
	move	d0,d3			;WX_left
	muls	(a2),d3			;WZ=sin*WX
	asr.l	d4,d3
	muls	(a3),d0			;WX=WX*cos
	asr.l	d4,d0
	movem.w	d0-d3,(sp)		;WX,EX,EZ,WZ
;--
	move	#Map3D_width/2-1,d0
	move	d0,d3			;WX_right
	muls	(a2),d3			;WZ=sin*WX
	asr.l	d4,d3
	muls	(a3),d0			;WX=WX*cos
	asr.l	d4,d0
	movem.w	d0-d3,8(sp)
	move.l	Player_X,a2		;--- ok, do it
	move.l	Player_Y,a3
.kant_loop:
	move.l	(a5)+,d5			;EY/(EY-WY) << 16
;-- LEFT
	movem.w	(sp),d0-d3		;load coordinates
;D0 = WX
;D1 = EX
;D2 = EZ
;D3 = WZ
;D5 = EY/(EY-WY)
;-- X
	sub.l	d1,d0			;WX-EX
	muls.l	d5,d0			;*(EY/(EY-WY))
	swap	d1
	clr.w	d1
	add.l	d1,d0			;EX+""""
	add.l	a2,d0			;+ Player_X
 	asr.l	#3,d0			;64x64 please !
	move.l	d0,(a0)+			;save 3D_X
;-- Z
	sub.l	d2,d3			;WZ-EZ
	muls.l	d5,d3			;*(EY/(EY-WY))
	swap	d2
	clr.w	d2
	add.l	d2,d3			;EZ+""""
	add.l	a3,d3			;+ player_z
 	asr.l	#3,d3			;64x64 please !
	move.l	d3,(a0)+			;save 3D_Z
;-- RIGHT
	movem.w	8(sp),d0-d3		;load coordinates
;-- X
	sub.l	d1,d0			;WX-EX
	muls.l	d5,d0			;*(EY/(EY-WY))
	swap	d1
	clr.w	d1
	add.l	d1,d0			;EX+""""
	add.l	a2,d0			;+ Player_X
 	asr.l	#3,d0			;64x64 please !
	move.l	d0,(a0)+			;save 3D_X
;-- Z
	sub.l	d2,d3			;WZ-EZ
	muls.l	d5,d3			;*(EY/(EY-WY))
	swap	d2
	clr.w	d2
	add.l	d2,d3			;EZ+""""
	add.l	a3,d3			;+ player_z
 	asr.l	#3,d3			;64x64 please !
	move.l	d3,(a0)+			;save 3D_Z
	dbra	d7,.kant_loop
	add.w	#16,sp
	rts

;*****************************************************************************
; [ Get view rectangle ]
;  OUT : d0-d3 - View rectangle (.w)
; No registers are restored
;*****************************************************************************
Get_draw_size:
;--- sin und cos holen ---
	lea	Sine_table,a0
	move	Y_angle,d1
	and	#slang-1,d1
	add	d1,d1
	move.w	(a0,d1.w),d0		;sin(yw)
	add	#slang/2,d1
	and	#slang*2-1,d1
	move.w	(a0,d1.w),d1		;cos(yw)
;--- vorderen punkte erzeugen ---
	lea	view_points,a0
	move	d0,d2
	move	d1,d3
	muls	#Map3D_width/2,d2		;sin(yw)*x
	muls	#Map3D_width/2,d3		;cos(yw)*x
	asl.l	#2,d2
	swap	d2
	asl.l	#2,d3
	swap	d3
	move	d0,d4
	move	d1,d5
	muls	#proj_faktor,d4		;sin(yw)*zp
	muls	#proj_faktor,d5		;cos(yw)*zp
	asl.l	#2,d4
	swap	d4
	asl.l	#2,d5
	swap	d5
	neg.w	d5			;-cos(yw)*zp
	neg.w	d2			;-- punkt #1
	move	d3,(a0)+			;cos(yw)*x
	move	d2,(a0)+			;-sin(yw)*x
	move	d3,d6			;-- punkt #2
	move	d2,d7
	sub	d4,d6			;deltax
	sub	d5,d7			;deltay
	muls	#patt_size/30,d6		;pattsize-abhaengig
	muls	#patt_size/30,d7		;pattsize-abhaengig
	add	d4,d6
	add	d5,d7
	move	d6,(a0)+
	move	d7,(a0)+
	neg.w	d2			;-- punkt #3
	neg.w	d3
	move	d3,(a0)+			;-cos(yw)*x
	move	d2,(a0)+			;sin(yw)*x
	move	d3,d6			;-- punkt #4
	move	d2,d7
	sub	d4,d6			;deltax
	sub	d5,d7			;deltay
	muls	#patt_size/30,d6		;pattsize-abhaengig
	muls	#patt_size/30,d7		;pattsize-abhaengig
	add	d4,d6
	add	d5,d7
	move	d6,(a0)+
	move	d7,(a0)+
;--- minx,miny,maxx,maxy suchen ---
	lea	view_points,a0
	move	#$7fff,d0			;minx
	move	#$7fff,d1			;miny
	moveq	#0,d2			;maxx
	moveq	#0,d3			;maxy
	moveq	#4-1,d7
.gv_loop:	movem.w	(a0)+,d4-d5
	cmp	d4,d0			;-- minx
	ble.s	.new0
	move	d4,d0
.new0:	cmp	d5,d1			;-- miny
	ble.s	.new1
	move	d5,d1
.new1:	cmp	d4,d2			;-- maxx
	bge.s	.new2
	move	d4,d2
.new2:	cmp	d5,d3			;-- maxy
	bge.s	.new3
	move	d5,d3
.new3:	dbra	d7,.gv_loop
	add	#patt_size-1,d2		;runden
	add	#patt_size-1,d3
	add	Player_X,d0		;minx
	add	Player_Y,d1		;miny
	add	Player_X,d2		;maxx
	add	Player_Y,d3		;maxy
;--- bereich clippen ---
	tst	d0
	bpl.s	.clip1
	moveq	#0,d0
.clip1:	tst	d1
	bpl.s	.clip2
	moveq	#0,d1
.clip2:	cmp	dsizex2,d2
	ble.s	.clip3
	move	dsizex2,d2
.clip3:	cmp	dsizey2,d3
	ble.s	.clip4
	move	dsizey2,d3
.clip4:	movem.w	d0-d3,dminx
	rts

;*****************************************************************************
; [ Rotate points of view rectangle ]
;   IN : d0-d3 - View rectangle (.w)
; No registers are restored
;*****************************************************************************
Make_rotated_points:
	and	#-patt_size,d0
	and	#-patt_size,d1
	and	#-patt_size,d2		;?
	and	#-patt_size,d3		;?
	move	d0,d4			;minx -> startx
	move	d1,d5			;miny -> starty
	sub	d0,d2
	sub	d1,d3
	moveq.l	#psize_log,d6
	lsr	d6,d2
	lsr	d6,d3
;	addq	#1,d2			;einschliesslich !!
;	addq	#1,d3
	move	d2,drawsizex
	move	d3,drawsizey
	lsr	d6,d4
	lsr	d6,d5
	move	d4,drawstartx
	move	d5,drawstarty
	sub	Player_X,d0
	sub	Player_Y,d1
;d0-d1 -> relative start-koo im dungeon
;d2-d3 -> anzahl punkte
;--- punkte-array fuellen ---
	move	d3,d6
	move	d2,d7
	addq	#1,d6
	addq	#1,d7
	mulu	d6,d7			;anzahl punkte
	lea	rot_points,a2
.yloop:	move	d0,d4
	move	d1,d5
	move	d2,d6			;zaehler
.xloop:	move	d4,(a2)+
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
	move	d0,(a1)+			;x
	move	d2,(a1)+			;y_low
	move	d0,(a1)+			;x
	move	d3,(a1)+			;y_high
	dbra	d7,.proj_loop
	rts

.aga:	add	scrx,d0
	move	d0,(a1)+
	clr.w	(a1)+
	move	d0,(a1)+
	clr.w	(a1)+
	dbra	d7,.proj_loop
	rts

;*****************************************************************************
; [ Get visible dungeon walls, objects and floors ]
; No registers are restored
;*****************************************************************************
Get_faces:
	Get	Mapdata_handle,a0
	lea.l	Map_data(a0),a0
; ---------- Do trick for non-blocking walls ------
	move.l	a0,-(sp)
	move.w	Map_Xcoord,d0		; Get byte at player's
	subq.w	#1,d0			;  position
	add.w	d0,a0
	move.w	Map_Ycoord,d0
	subq.w	#1,d0
	mulu.w	dsizex,d0
	add.l	d0,a0
	moveq.l	#0,d0			; Get
	move.b	(a0),d0
	move.b	d0,Saved_map_byte		; Save
	move.l	a0,Saved_map_ptr
	cmp.w	#First_wall,d0		; Is it a wall ?
	bmi.s	.Done
	clr.b	(a0)			; Trick
.Done:	move.l	(sp)+,a0
; ---------- Do -----------------------------------
	cmp.w	#Max_visible_objects,Visible_objects	; Too much objects already ?
	bpl	.Exit
	move.w	drawstartx,d0		; Get map address
	add.w	d0,a0
	move.w	drawstarty,d0
	neg.w	d0
	add.w	dsizey,d0
	subq.w	#1,d0
	mulu.w	dsizex,d0
	add.l	d0,a0
	lea.l	proj_points,a1
	lea.l	rot_points,a2
	lea.l	Dungeon_sort_list,a3	; Addresses & distances
	move.w	Visible_objects,d0		;  (skip NPC objects)
	lsl.w	#3,d0
	add.w	d0,a3
	lea.l	Dungeon_wall_list,a4	; Wall definitions
	lea.l	Wall_transparencies,a5	; Transparency flags
	lea.l	Dungeon_object_list,a6	; Object definitions
	move.w	Visible_objects,d0		;  (skip NPC objects)
	mulu.w	#12,d0			; !!!
	add.w	d0,a6
	move.w	drawstarty,d0		; Set yoghurt coordinate
	addq.w	#1,d0
	move.w	d0,Yoghurt_Y
	mulu.w	Width_of_map,d0		; Calculate hash number
	add.w	drawstartx,d0
	addq.w	#1,d0
	move.w	d0,Hash_number
	move.w	drawsizey,d6		; Do
	subq.w	#1,d6
.Loop1:	move.w	d6,-(sp)
	move.w	drawstartx,d0
	addq.w	#1,d0
	move.w	d0,Yoghurt_X
	move.w	drawsizex,d7
	subq.w	#1,d7
.Loop2:	move.w	d7,-(sp)
; ---------- Analyze map byte ---------------------
	moveq.l	#0,d0			; Get byte from map
	move.b	(a0),d0
	beq.s	.Floor			; If any
	cmp.b	#-1,d0			; Dummy wall ?
	beq	.Next2
	cmp.w	#First_wall,d0		; No -> Object or wall ?
	bpl.s	.Wall
	jsr	Insert_object_group		; Do object group
.Floor:	jsr	Create_floor_polygons	; Create floor & ceiling
	bra	.Next2
; ---------- Check walls --------------------------
.Wall:	sub.w	#First_wall,d0
	move.b	d0,color+1		; Yes -> Store
	move.b	0(a5,d0.w),color		; Transparent ?
	beq.s	.No
	moveq.l	#0,d0			; Yes -> Use defaults
	jsr	Create_floor_polygons	; Create floor & ceiling
; ---------- Check above --------------------------
.No:	move	dsizex,d1			; Get map byte above
	neg.w	d1
	moveq.l	#0,d0
	move.b	0(a0,d1.w),d0
	beq.s	.do_o			; If any
	cmp.b	#-1,d0			; Dummy wall ?
	beq	.blocko
	sub.w	#First_wall,d0		; No -> Object ?
	bcs.s	.do_o
	tst.b	0(a5,d0.w)		; No -> Transparent wall ?
	beq	.blocko
;--- 2d-koos laden ---
.do_o:	move	drawsizex,d7
	addq	#1,d7
	asl	#3,d7
	move	8(a1,d7.w),d0		; x1
	move	(a1,d7.w),d1		; x2
;-- 3d-koos laden (hi = x , lo = y)
	asr	#1,d7			; 3d-koos holen
	move.l	4(a2,d7.w),d6
	move.l	(a2,d7.w),d7
	tst.w	d6
	bpl.s	.ok3d1
	tst.w	d7
	bmi	.blocko
.ok3d1:
;-- sichtbar ?
	cmp	d0,d1
	blt	.blocko			; -> hidden
	cmp	cx1,d1
	blt	.blocko			; links ausserhalb
	cmp	cx2,d0
	bgt	.blocko			; rechts ausserhalb
;-- sichtbar !
	move.l	a4,4(a3)			; adresse speichern
	addq.w	#1,Visible_walls
	move.w	#$face,(a4)+		; flag fuer wand
	move.w	color,(a4)+		; farbe
	move.l	d6,(a4)+			; 3d-koos speichern
	move.l	d7,(a4)+
	move.w	Hash_number,(a4)+		; Save hash number
	move	d0,(a4)+			; x1
	move	d1,(a4)+			; x2
	add	d7,d6
	asr	#1,d6
	move.w	d6,(a3)+			; entfernung speichern
	clr.w	(a3)+
	addq.l	#4,a3
	bra	.blocku			; gegenueberliegende flaeche ueberspringen
.blocko:
; ---------- Check below --------------------------
	move	dsizex,d1			; Get map byte below
	moveq.l	#0,d0
	move.b	0(a0,d1.w),d0
	beq.s	.do_u			; If any
	cmp.b	#-1,d0			; Dummy wall ?
	beq	.blocku
	sub.w	#First_wall,d0		; No -> Object ?
	bcs.s	.do_u
	tst.b	0(a5,d0.w)		; No -> Transparent wall ?
	beq	.blocku
;--- 2d-koos laden ---
.do_u:	move	(a1),d0			; x1
	move	8(a1),d1			; x2
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
	blt	.blocku			; -> hidden
	cmp	cx1,d1
	blt	.blocku			; links ausserhalb
	cmp	cx2,d0
	bgt	.blocku			; rechts ausserhalb
;-- sichtbar !
	move.l	a4,4(a3)			; adresse speichern
	addq.w	#1,Visible_walls
	move.w	#$face,(a4)+		; flag fuer wand
	move.w	color,(a4)+		; farbe
	move.l	d6,(a4)+
	move.l	d7,(a4)+
	move.w	Hash_number,(a4)+		; Save hash number
	move	d0,(a4)+			; x1
	move	d1,(a4)+			; x2
	add	d7,d6
	asr	#1,d6
	move.w	d6,(a3)+			; entfernung speichern
	clr.w	(a3)+
	addq.l	#4,a3
.blocku:
; ---------- Check to the left --------------------
	moveq.l	#0,d0			; Get map byte to the left
	move.b	-1(a0),d0
	beq.s	.do_l			; If any
	cmp.b	#-1,d0			; Dummy wall ?
	beq	.blockl
	sub.w	#First_wall,d0		; No -> Object ?
	bcs.s	.do_l
	tst.b	0(a5,d0.w)		; No -> Transparent wall ?
	beq	.blockl
;--- 2d-koos laden ---
.do_l:	move	drawsizex,d7
	addq	#1,d7
	asl	#3,d7
	move	(a1,d7.w),d0		; x1
	move	(a1),d1			; x2
;-- 3d-koos laden
	asr	#1,d7			; 3d-koos holen
	move.l	(a2,d7.w),d6
	move.l	(a2),d7
	tst.w	d6
	bpl.s	.ok3d3
	tst.w	d7
	bmi	.blockl
.ok3d3:
;-- sichtbar ?
	cmp	d0,d1
	ble	.blockl			; -> hidden
	cmp	cx1,d1
	blt	.blockl			; links ausserhalb
	cmp	cx2,d0
	bgt	.blockl			; rechts ausserhalb
;-- sichtbar !
	move.l	a4,4(a3)			; adresse speichern
	addq.w	#1,Visible_walls
	move.w	#$face,(a4)+		; flag fuer wand
	move.w	color,(a4)+		; farbe
	move.l	d6,(a4)+
	move.l	d7,(a4)+
	move.w	Hash_number,(a4)+		; Save hash number
	move	d0,(a4)+			; x1
	move	d1,(a4)+			; x2
	add	d7,d6
	asr	#1,d6
	move.w	d6,(a3)+			; entfernung speichern
	clr.w	(a3)+
	addq.l	#4,a3
	bra	.blockr			; gegenueberliegende flaeche ueberspringen
.blockl:
; ---------- Check to the right -------------------
	moveq.l	#0,d0			; Get map byte to the right
	move.b	1(a0),d0
	beq.s	.do_r			; If any
	cmp.b	#-1,d0			; Dummy wall ?
	beq	.blockr
	sub.w	#First_wall,d0		; No -> Object ?
	bcs.s	.do_r
	tst.b	0(a5,d0.w)		; No -> Transparent wall ?
	beq	.blockr
;--- 2d-koos laden ---
.do_r:	move	drawsizex,d7
	addq	#1,d7
	asl	#3,d7
	move	8(a1),d0			; x1
	move	8(a1,d7.w),d1		; x2
;-- 3d-koos laden
	asr	#1,d7			; 3d-koos holen
	move.l	4(a2),d6
	move.l	4(a2,d7.w),d7
	tst.w	d6
	bpl.s	.ok3d4
	tst.w	d7
	bmi	.blockr
.ok3d4:
;-- sichtbar ?
	cmp	d0,d1
	ble	.blockr			; -> hidden
	cmp	cx1,d1
	blt	.blockr			; links ausserhalb
	cmp	cx2,d0
	bgt	.blockr			; rechts ausserhalb
;-- sichtbar !
	move.l	a4,4(a3)			; adresse speichern
	addq.w	#1,Visible_walls
	move.w	#$face,(a4)+		; flag fuer wand
	move.w	color,(a4)+		; farbe
	move.l	d6,(a4)+
	move.l	d7,(a4)+
	move.w	Hash_number,(a4)+		; Save hash number
	move	d0,(a4)+			; x1
	move	d1,(a4)+			; x2
	add	d7,d6
	asr	#1,d6
	move.w	d6,(a3)+			; entfernung speichern
	clr.w	(a3)+
	addq.l	#4,a3
.blockr:
; ---------- Next map byte ------------------------
.Next2:	move.w	(sp)+,d7
	cmp	#Max_visible_walls,Visible_walls	; Too much ?
	bpl.s	.Overflow
	cmp	#Max_visible_objects,Visible_objects
	bpl.s	.Overflow
	cmp	#Max_visible_floors,Visible_floors
	bmi.s	.All_OK
.Overflow:	move.w	(sp)+,d6			; Yes -> break
	bra.s	.Exit
.All_OK:	addq.w	#1,Yoghurt_X		; Next X
	addq.w	#1,Hash_number
	addq.l	#8,a1
	addq.l	#4,a2
	addq.l	#1,a0
	dbra	d7,.Loop2
	addq.w	#1,Yoghurt_Y		; Next Y
	move.w	Hash_number,d0
	sub.w	drawsizex,d0
	add.w	Width_of_map,d0
	move.w	d0,Hash_number
	addq.l	#8,a1
	addq.l	#4,a2
	move.w	drawsizex,d0
	add.w	dsizex,d0
	sub.w	d0,a0
	move.w	(sp)+,d6
	dbra	d6,.Loop1
.Exit:	move.w	Visible_walls,d0		; Add
	add.w	Visible_objects,d0
	add.w	Visible_floors,d0
	move.w	d0,Visible_elements
	Free	Mapdata_handle
; ---------- Reverse trick ------------------------
	move.l	Saved_map_ptr,a0		; Reset
	move.b	Saved_map_byte,(a0)
	rts

;*****************************************************************************
; [ Create floor & ceiling polygons ]
;   IN : d0 - 0 / Object group number (.w)
; Changed registers : d2,d3,d4,d5,d6,d7,a3,a4
; Note :
;  - This subroutine is a part of [ Get_faces ].
;*****************************************************************************
Create_floor_polygons:
	movem.l	d0/a0,-(sp)
	lea.l	Floor_ptrs,a0
	lsl.w	#3,d0
	add.w	d0,a0
; ---------- Create floor polygon -----------------
	move	drawsizex,d3		;3d-koos holen
	addq	#1,d3
	move.l	(a2,d3.w*4),d4		;p1
	move.l	4(a2,d3.w*4),d5		;p2
	move.l	4(a2),d6			;p3
	move.l	(a2),d7			;p4	
	tst.w	d4
	bpl.s	.bp_visible
	tst.w	d5
	bpl.s	.bp_visible
	tst.w	d6
	bpl.s	.bp_visible
	tst.w	d7
	bmi	.Exit			;-> ganz im negativen Z-bereich !
.bp_visible:
	move.l	a4,4(a3)			;adresse speichern
	addq.w	#1,Visible_floors
	move.w	#"BP",(a4)+		;flag fuer polygon
	move.w	lower_y,d2
	move.l	(a1,d3.w*8),(a4)+		;2D_koos (x,y_low)
	move.l	d4,(a4)+			;3D_koos (x,z,y)
	move.w	d2,(a4)+
	move.l	8(a1,d3.w*8),(a4)+		;2D_koos
	move.l	d5,(a4)+			;3D_koos
	move.w	d2,(a4)+
	move.l	8(a1),(a4)+		;2D_koos
	move.l	d6,(a4)+			;3D_koos
	move.w	d2,(a4)+
	move.l	(a1),(a4)+		;2D_koos
	move.l	d7,(a4)+			;3D_koos
	move.w	d2,(a4)+
;	move.l	(a0),(a4)+		; Store graphics address

	move.l	#Test_floor,(a4)+

	add.w	d7,d5			;mitte diagonal ermitteln
	asr.w	#1,d5
	sub.w	#patt_size/2,d5
	move.w	d5,(a3)+			;entfernung speichern
	clr.w	(a3)+
	addq.l	#4,a3
; ---------- Create ceiling polygon ---------------
	tst.l	4(a0)			; Any ceiling ?
	bne.s	.Yes
	cmp.b	#City_map_type,Current_map_type	; No -> City ?
	beq	.Exit
	lea.l	Floor_ptrs,a0		; No -> Use default
.Yes:	move	drawsizex,d3		;3d-koos holen
	addq	#1,d3
	move.l	(a2),d4			;p1
	move.l	4(a2),d5			;p2
	move.l	4(a2,d3.w*4),d6		;p3
	move.l	(a2,d3.w*4),d7		;p4	
	tst.w	d4
	bpl.s	.bp2_visible
	tst.w	d5
	bpl.s	.bp2_visible
	tst.w	d6
	bpl.s	.bp2_visible
	tst.w	d7
	bmi	.Exit			;-> ganz im negativen Z-bereich !
.bp2_visible:
	move.l	a4,4(a3)			;adresse speichern
	addq.w	#1,Visible_floors
	move.w	#"BP",(a4)+		;flag fuer polygon
	move.w	upper_y,d2
	move.l	4(a1),(a4)+		;2D_koos (x,y_high)
	move.l	d4,(a4)+			;3D_koos
	move.w	d2,(a4)+
	move.l	12(a1),(a4)+		;2D_koos
	move.l	d5,(a4)+			;3D_koos
	move.w	d2,(a4)+
	move.l	12(a1,d3.w*8),(a4)+		;2D_koos
	move.l	d6,(a4)+			;3D_koos
	move.w	d2,(a4)+
	move.l	4(a1,d3.w*8),(a4)+		;2D_koos
	move.l	d7,(a4)+			;3D_koos
	move.w	d2,(a4)+
;	move.l	4(a0),(a4)+		; Store graphics address

	move.l	#Test_floor,(a4)+
;	move.l	#Test_floor+128,(a4)+

	add.w	d7,d5			;mitte diagonal ermitteln
	asr.w	#1,d5
	sub.w	#patt_size/2,d5
	move.w	d5,(a3)+			;entfernung speichern
	clr.w	(a3)+
	addq.l	#4,a3
.Exit:	movem.l	(sp)+,d0/a0
	rts

;*****************************************************************************
; [ Quicksort ]
;   IN : d6 - Length of entry (.w)
;        d7 - Number of values (.l)
;        a0 - Pointer to table (.l)
; No registers are restored
; Note :
;  - The order of elements with the same value will be changed !
;  - The first longword of each entry is compared.
;*****************************************************************************
Quicksort:
	move.l	a0,a1
	move	d7,d0
	subq	#1,d0
	mulu	d6,d0
	add.w	d0,a1	;-> letzter eintrag

	move	d6,d4
	mulu	#9,d4	;ab 9 werte
;	mulu	d7,d4	;immer bubblesort
;d4.l = differenz ab der bubble-sort aktiv wird

Quicksort_call:
	cmp.l	a0,a1
	ble	.Exit
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
.Loop0:
.Loop1:	cmp.l	(a0),d5
	ble.s	.Get1
	add.l	d6,a0
	bra	.Loop1
.Get1:
.Loop2:	cmp.l	(a1),d7
	bge.s	.Get2
	sub.l	d6,a1
	bra	.Loop2
.Get2:	cmp.l	a1,a0
	bge	.Part
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
	bgt	.Loop0
;--- rekursionen durchfuehren ---
.Part:	movem.l a0-a3,-(sp)		;a2 - a1
	move.l	a2,a0
	bsr	Quicksort_call
	movem.l (sp)+,a0-a3
	move.l	a3,a1			;a0 - a3
	bsr	Quicksort_call
.Exit:	rts
 
;(einfuege-sort)
;*******************
;*** bubble-sort ***
;*******************
qs_bubbleit:
;a0 = start
;a1 = ende (inc.)
	move.l	a0,a2
.Loop0:
;*** 2 zahlen suchen die falsch sind ***
.Loop1:	move.l	(a0),d0
	add.l	d6,a0
	cmp.l	(a0),d0
	bgt.s	.Too_big
	cmp.l	a0,a1
	bne.s	.Loop1
	bra	.Exit
.Too_big:	move.l	(a0),d0
	move.l	4(a0),d1
	move.l	a0,a3
;*** solange zurueckgehen bis kleinere zahl gefunden ***
.Loop2:	sub.l	d6,a0
	cmp.l	a2,a0
	blt.s	.Next
	move.l	(a0),(a0,d6.l)
	move.l	4(a0),4(a0,d6.l)
	cmp.l	(a0),d0
	blt.s	.Loop2	
.Next:	move.l	d0,(a0,d6.l)
	move.l	d1,4(a0,d6.l)
	move.l	a3,a0
	cmp.l	a0,a1
	bne.s	.Loop0
.Exit:	rts

;*****************************************************************************
; [ Draw dungeon faces ]
; No registers are restored
;*****************************************************************************
Draw_faces:
	move	#0,cx1			; Reset clipping variables
	move	#0,cy1
	move	#Map3D_width-1,cx2
	move	#Map3D_height-1,cy2
; ---------- Clip all visible elements ------------
	move.w	#-1,Dungeon_clipping_table	; Clear clipping table
	lea	Dungeon_sort_list,a0
	lea	Dungeon_draw_list,a4
	move.w	Visible_elements,d7		; Any elements ?
	beq	.Exit
	subq.w	#1,d7			; Yes
.Loop1:	movem.l	d7/a0,-(sp)
	move.l	4(a0),a0			; Get pointer to data
	cmp.w	#$face,(a0)		; Is wall ?
	beq.s	.Wall1
	cmp.w	#"BP",(a0)		; No -> Is floor ?
	beq.s	.Floor
; ---------- Clip object --------------------------
	movem.w	2(a0),d0-d4		; No -> Is object
	jsr	make_3dzoom0		; Clip
	bra	.Next1
; ---------- Clip & draw (!) floor ----------------
.Floor:	bsr	preclip_tmpoly
	move	d3,cx2
	bmi.s	.Next1
	move	d2,cx1
	move.l	a4,-(sp)
	addq.l	#2,a0
	moveq	#4,d7			; Clip
	bsr	clip3d_newTM
	cmp.w	#3,d7
	blt	.Skip			;-> clip_out !
	bsr	draw_newTM
.Skip:	move.l	(sp)+,a4
	bra.s	.Next1
; ---------- Clip wall ----------------------------
.Wall1:	lea	14(a0),a0			;jump to 2d_koos
	bsr	check_vface
	tst.w	d7
	bmi.s	.out
	move.w	cx1,(a4)+
	move.w	cx2,(a4)+
	bra.s	.Next1
.out:	move.l	#-1,(a4)+
.Next1:	movem.l	(sp)+,d7/a0		; Next element
	addq.l	#8,a0
	dbra	d7,.Loop1
; ---------- Draw all visible elements ------------
	move.l	a0,a1
	move.w	Visible_elements,d7
	subq.w	#1,d7
.Loop2:	subq.l	#8,a1			; Backwards !
	move.l	4(a1),a0			; -> adresse der def
	move.w	(a0)+,d0			; flag
	cmp	#$face,d0
	beq	.Wall2
	cmp	#$abed,d0
	bne	.Next2
; ---------- Draw object --------------------------
	movem.l	d7/a1,-(sp)		; Used to be a1 !!!
	jsr	make_3dzoom2
	movem.l	(sp)+,d7/a1
	bra	.Next2
; ---------- Draw wall ----------------------------
.Wall2:	move.w	-(a4),d1
	move.w	-(a4),d0
	bmi	.Next2			; -> flaeche draussen
	move.w	d0,cx1			; clipping setzen
	move.w	d1,cx2
	movem.l	d7/a1/a4,-(sp)
	move.w	(a0)+,color
	lea	dpoly_koos3d,a1
	movem.w	(a0)+,d0-d3
	move.w	d0,(a1)+			; x1_3d
	move.w	d2,(a1)+			; x2_3d
	move	lower_y,(a1)+		; y1_3d
	move	upper_y,(a1)+		; y2_3d
	move.w	d1,(a1)+			; z1_3d
	move.w	d3,(a1)+			; z2_3d
	move.w	color,d0			; Get wall address
	jsr	Get_wall_address
	jsr	put_dpoly			; Draw
	movem.l	(sp)+,d7/a1/a4
.Next2:	dbra	d7,.Loop2
.Exit:	rts

;check for overlaps
preclip_tmpoly:
;--- get minx/maxx ---
	move	#32767,d0			;min_x
	move	#-32768,d4		;max_x
	move.l	a0,a1
	addq.l	#2,a1			;skip flag
	moveq	#4-1,d2
.gx_loop:	move.w	(a1),d3
	cmp	d3,d0
	ble.s	.keep_minx
	move	d3,d0
.keep_minx:
	cmp	d3,d4
	bge.s	.keep_maxx
	move	d3,d4
.keep_maxx:
	add.w	#10,a1
	dbra	d2,.gx_loop
;d0 = xleft
;d4 = xright
	bsr	check_vface2
	rts

;*****************************************************************************
; [ Handle 3D zoom object ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Z-coordinate (.w)
;        d3 - Object number (.w)
;        d4 - Extra value (.w)
;        a4 - Pointer to {Dungeon_draw_list} (.l)
; No registers are restored
;*****************************************************************************
make_3dzoom0:
	subq.w	#1,d3
	cmp.w	Nr_of_objects,d3		; Legal ?
	bmi.s	.Ok1
	moveq.l	#-1,d2			; Exit
	bra	.Done
.Ok1:	lea.l	Object_handles,a1		; Get graphics address
	move.b	0(a1,d3.w),d5
	Get	d5,a1
	Free	d5
	move.l	Labdata_ptr,a0		; Get object data
	adda.l	Object_data_offset,a0
	mulu.w	#Object3D_data_size,d3
	add.l	d3,a0
	tst.w	d4			; Normal object or NPC ?
	bpl.s	.Normal
	tst.b	d4			; NPC -> Animate ?
	bne.s	.Yes
	moveq.l	#NPC_stat_frame,d3		; No
	bra.s	.Go_on
.Yes:	moveq.l	#0,d4			; Yes
.Normal:	move.l	Object_bits(a0),d2		; Normal object -> Get
	move.w	d4,d5			;  animation frame
	move.b	Object_nr_frames(a0),d4
	jsr	Get_animation_frame
	move.w	d4,d3
.Go_on:	moveq.l	#0,d4			; Get width & height
	move.b	Object_width(a0),d4
	moveq.l	#0,d5
	move.b	Object_height(a0),d5
	move.w	d4,-(sp)			; Save
	move.w	d5,-(sp)
	and.w	#$fff0,d4			; Calculate size of one block
	lsr.w	#1,d4
	mulu.w	d5,d4
	mulu.w	d4,d3			; Calculate frame address
	adda.l	d3,a1
	ext.l	d0			; Do it
	ext.l	d1
	move	d2,d5
	add	#proj_faktor,d5		; z+zp
	asl.l	#proj_log,d0		; x*zp
	asl.l	#proj_log,d1		; y*zp
	move.l	d1,d4
	divs	d5,d0
	divs	d5,d1
	neg.w	d1
	add.w	scrx,d0			; -> screen_x
	add.w	scry,d1			; -> screen_y
;d2 = z
;d4 = y*zp
	btst	#Distort_bit,d6		; verzerr_flag
	beq	.normal_3dzoom
;****** ysize perspektivisch erzeugen ******
	moveq.l	#0,d6
	move.w	Object_dungeon_width(a0),d6
	moveq.l	#0,d7
	move.w	Object_dungeon_height(a0),d7
;-- xsize
	asl.l	#proj_log,d6		; xs*zp
	divs	d5,d6			; -> xsize
;-- ysize
	asr	#1,d7

; REGISTER CONTENTS :
;  d0 - Screen X
;  d1 - Screen Y
;  d2 - Z
;  d4 - Y x Zp
;  d5 - Projection factor (Z + Zp)
;  d6 - Projected width
;  d7 - Height in dungeon / 2

	add	d7,d2			; -> z1
	add	#proj_faktor,d2		; z1+zp
	bne.s	.Ok2
	moveq.l	#-1,d2			; Exit
	bra	.Done
.Ok2:	divs	d2,d4
;  d4 = y2
	move	d4,d7
	neg	d7
	add	scry,d7
	sub	d1,d7			; ysize / 2
	bpl.s	.ysok
	neg.w	d7
.ysok:
;x zentrieren
	move	d6,d2
	lsr	#1,d2
	sub	d2,d0
;y zentrieren
	sub	d7,d1			; ytop = midy-ysize/2
	add	d7,d7			; real ysize

	cmp.w	#2,d7			; Never less than 2
	bpl.s	.Not_too_small
	moveq.l	#2,d7
.Not_too_small:
	bra	.do_zoomclip
;****** einfach nur 3d-zooming ******
.normal_3dzoom:
	moveq.l	#0,d6
	move.w	Object_dungeon_width(a0),d6
	moveq.l	#0,d7
	move.w	Object_dungeon_height(a0),d7
	asl.l	#proj_log,d6		; xs*zp
	asl.l	#proj_log,d7		; ys*zp
	divs	d5,d6			; -> xsize
	divs	d5,d7			; -> ysize
	sub	d7,d1			; hotspot nach unten
	move	d6,d2
	lsr	#1,d2
	sub	d2,d0			; mitte
.do_zoomclip:
	move	d0,d4
	add	d6,d4

	move.w	Object_dungeon_height(a0),d7	; Larger than wall ?
	cmp.w	Wall_height_3D,d7
	bpl.s	.Above
	jsr	check_vface2		; No -> Clip
	bra.s	.Done
.Above:
	tst.w	d4			; Yes -> No wall clipping
	bmi.s	.Out
	cmp.w	#Map3D_width-1,d0
	bmi.s	.Ok3
.Out:	moveq.l	#-1,d2
	moveq.l	#-1,d3
	bra.s	.Done
.Ok3:	move.w	d0,d2
	move.w	d4,d3
	tst.w	d2
	bpl.s	.Ok4
	moveq.l	#0,d2
.Ok4:	cmp.w	#Map3D_width-1,d3
	ble.s	.Ok5
	move.w	#Map3D_width-1,d3
.Ok5:
;--- speichern ---
.Done:	move	(sp)+,d5
	move	(sp)+,d4
.Done2:	tst.w	d2
	bmi.s	.skip
	move.w	d0,(a4)+			; x
	move.w	d1,(a4)+			; y
	move.w	d4,(a4)+			; old_sizex
	move.w	d5,(a4)+			; old_sizey
	move.w	d6,(a4)+			; xsize
	move.w	d7,(a4)+			; ysize
	move.l	a1,(a4)+			; source_gfx
	move.w	d3,(a4)+			; clip_rechts
.skip:	move.w	d2,(a4)+			; clip_links
.Exit:	rts

;*****************************************************************************
; [ Draw zoom object ]
; No registers are restored
;*****************************************************************************
make_3dzoom2:
	move.w	-(a4),cx1
	bmi.s	.Exit
	move.w	-(a4),cx2
	move.l	-(a4),a0			; Graphics address
	move.w	-(a4),d7			; New height
	move.w	-(a4),d6			; New width
	move.w	-(a4),d5			; Source height
	move.w	-(a4),d4			; Source width
	move.w	-(a4),d1			; Y-position
	move.w	-(a4),d0			; X-position
	tst.w	d6			; Too small ?
	beq.s	.Exit
	tst.w	d7
	beq.s	.Exit
	moveq	#-1,d2			; Mask
	move.l	a4,-(sp)			; Draw zoom object
	jsr	draw_zoomshape
	move.l	(sp)+,a4
.Exit:	rts

;*****************************************************************************
; [ Rotate around Y-axis ]
;   IN : d7 - Number of coordinates (.w)
;        a0 - Pointer to input coordinate list (.l)
;        a1 - Pointer to output coordinate list (.l)
; No registers are restored
;*****************************************************************************
rotate_y:
	subq	#1,d7
	bmi	.Exit
	lea	Sine_table,a2
	and	#slang-1,Y_angle
	move	Y_angle,d6
	neg.w	d6
	add	#slang,d6
	and	#slang-1,d6
	add	d6,d6
	move.w	(a2,d6.w),d0		;sin(yw)
	add.w	#slang/2,d6
	and.w	#slang*2-1,d6
	move.w	(a2,d6.w),d1		;cos(yw)
	move.w	xoffset,a2
	move.w	zoffset,a3
.Loop:	move.w	(a0)+,d2			;x
	move.w	(a0)+,d3			;y
	move	d2,d4
	move	d3,d5
	muls	d0,d2			; x*sin(yw)
	muls	d0,d3			; y*sin(yw)
	muls	d1,d4			; x*cos(yw)
	muls	d1,d5			; y*cos(yw)
	sub.l	d3,d4
	add.l	d2,d5
	asl.l	#2,d4
	asl.l	#2,d5
	swap	d4
	swap	d5
	add.w	a2,d4			; +xoffset
	add.w	a3,d5			; +zoffset
	move.w	d4,(a1)+
	move.w	d5,(a1)+
	dbra	d7,.Loop
.Exit:	rts

;*****************************************************************************
; [ Rotate single point around Y-axis ]
;   IN : d0-d2 - 3D coordinates (.w)
;        d3 - Angle (.w)
;  OUT : d0-d2 - Rotated 3D coordinates (.w)
; Changed registers : d0-d2
;*****************************************************************************
rotate_y1:
	movem.l	d3-d6/a2,-(sp)
	lea	Sine_table,a2
	move	d3,d6
	neg.w	d6
	add	#slang,d6
	and	#slang-1,d6
	add	d6,d6
	move.w	(a2,d6.w),d3		; sin(yw)
	add.w	#slang/2,d6
	and.w	#slang*2-1,d6
	move.w	(a2,d6.w),d4		; cos(yw)
	move	d0,d5
	move	d2,d6
	muls	d3,d5			; x*sin(yw)
	muls	d3,d6			; y*sin(yw)
	muls	d4,d0			; x*cos(yw)
	muls	d4,d2			; y*cos(yw)
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

;*****************************************************************************
; [ Check overlaps ]
;   IN : a0 - Pointer to coordinates (.w)
; No registers are restored
;*****************************************************************************
check_vface:
	move	(a0),d0			; x1
	move	2(a0),d1			; x2
	subq	#1,d0			; nachfolgende flaeche soll
	addq	#1,d1			; nur angrenzen, nicht 1
					; pixel ueberlappen
;--- grob clippen
	tst	d0
	bpl.s	.ok0
	moveq	#0,d0
.ok0:	cmp	#Map3D_width-1,d1
	ble.s	.ok1
	move	#Map3D_width-1,d1
.ok1:
	lea	Dungeon_clipping_table,a1
	sub.l	a2,a2
	sub.l	a3,a3
vcheck_loop:
	move.w	(a1)+,d2			; vx1
	bmi	vschulz
	move.w	(a1)+,d3			; vx2
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
	move.l	(a3)+,(a2)+		; flaeche ausfuegen
	bpl.s	.move_loop
	moveq	#0,d7
	rts
;*** erster punkt ausserhalb ***
.first_out:
	move	#0,cx1
	move	(a3),cx2
	move	d0,(a3)			; an x1 des naechsten anhaengen
	moveq	#0,d7
	rts
;*** zweiter punkt ausserhalb ***
.second_out:
	move	#Map3D_width-1,cx2
	move	2(a2),cx1
	move	d1,2(a2)			; an x1 des naechsten anhaengen
	moveq	#0,d7
	rts

;*** beide punkte ausserhalb oder in gleicher flaeche ***
.insame:
	tst.l	d2
	bne	v_out			; -> beide punkte in der gleichen flaeche
;*** flaeche hat nirgends geschnitten ***
	move	#0,cx1			; also normal clippen
	move	#Map3D_width-1,cx2
;--- einsortieren ---
	lea	Dungeon_clipping_table,a1
.is_loop:
	tst.w	(a1)			; ende erreicht ?
	bmi.s	.begin			; -> ja !
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

;*****************************************************************************
; [ Do clipping without changing the clipping table ]
;   IN : d0 - Left X-coordinate (.w)
;        d4 - Right X-coordinate (.w)
;  OUT : d2 - Clipped left X-coordinate (.w)
;        d3 - Clipped right X-coordinate (.w)
; Changed registers : d2,d3
;*****************************************************************************
check_vface2:
	movem.l	d0/d1/d4-d7/a5,-(sp)
;-- grob clippen
	tst	d4
	bmi.s	.mega_out
	cmp	#Map3D_width-1,d0
	bgt.s	.mega_out
	tst	d0
	bpl.s	.cok1
	moveq	#0,d0
.cok1:	cmp	#Map3D_width-1,d4
	ble.s	.cok2
	move	#Map3D_width-1,d4
.cok2:	lea	Dungeon_clipping_table,a5
	move	#0,d2			;default-clipping
	move	#Map3D_width-1,d3
.clip_loop:
	move.w	(a5)+,d6			;clip_links
	bmi.s	.clipped			;-> ende der tabelle
	move.w	(a5)+,d7			;clip_rechts
;xlinks drin ?
	cmp	d6,d0
	blt.s	.not_first		;-> x1 links von clip
	cmp	d7,d0
	bgt.s	.clip_loop		;-> aber rechts davon
	move	d7,d2			;x1 also im clip
	cmp	d7,d4
	bgt.s	.clip_loop		;-> x2 nicht im clip
.mega_out:	moveq	#-1,d2			;-> beide im clip !
	moveq	#-1,d3			;also wech damit !
	bra.s	.clipped
;xrechts drin ?
.not_first:
	cmp	d6,d4
	blt.s	.clip_loop		;beide links davon
	move	d6,d3			;rechts geclipt !
.clipped:	movem.l	(sp)+,d0/d1/d4-d7/a5
	rts

;*****************************************************************************
; [ Insert NPC objects ]
; No registers are restored
;*****************************************************************************
Insert_NPCs:
	movem.l	d0-d7/a0-a3/a6,-(sp)
	move.l	Labdata_ptr,a1		; Labyrinth data
	lea.l	VNPC_data,a2		; Extended NPC data
	lea.l	Dungeon_sort_list,a3	; Addresses & distances
	lea.l	Dungeon_object_list,a6	; Object definitions
	move.l	CD_value,d2
	moveq.l	#0,d3
.Loop1:	tst.b	NPC_char_nr(a2)		; Anyone there ?
	beq	.Next1
	btst	d3,d2			; Deleted	?
	bne	.Next1
	move.w	NPC_graphics_nr(a2),d0	; Get object group number
	subq.w	#1,d0			; Get object group data
	mulu.w	#Objectgroup3D_data_size,d0
	lea.l	Lab_data+4(a1,d0.w),a0
	move.b	NPC_status_bits(a2),d0	; What kind of NPC ?
	and.b	#$03,d0
	cmp.b	#Object_NPC,d0		; An object ?
	bne.s	.No
	st	.Animate			; Yes -> Animate always
	bra.s	.Skip
.No:	btst	#Movement_ended,VFlags(a2)	; No -> Moving ?
	seq	.Animate
.Skip:	move.w	VSource_X(a2),d5		; Get coordinates
	move.w	VSource_Y(a2),d6
	tst.w	d5			; Visible ?
	bne.s	.Do
	tst.w	d6
	beq	.Next1
.Do:	sub.w	#patt_size/2,d5		; Correction
	sub.w	#patt_size/2,d6
	movem.l	d2/d3,-(sp)
	moveq.l	#Objects_per_group-1,d7	; Do each object in group
.Loop2:	move.w	Object_nr(a0),d4		; Anything there ?
	beq	.Next2
	move.w	Object_X(a0),d0		; Get coordinates in map
	move.w	Object_Z(a0),d1		;  square
	move.w	Object_Y(a0),d2
	add.w	d5,d0			; Translate to dungeon
	add.w	d6,d2
	move.w	dminx,d3			; Check minimum X
	sub.w	#patt_size,d3
	cmp.w	d3,d0
	blt	.Next2
	move.w	dminx+2,d3		; Check minimum Y
	sub.w	#patt_size,d3
	cmp.w	d3,d2
	blt	.Next2
	move.w	dminx+4,d3		; Check maximum X
	add.w	#patt_size,d3
	cmp.w	d3,d0
	bgt.s	.Next2
	move.w	dminx+6,d3		; Check maximum Y
	add.w	#patt_size,d3
	cmp.w	d3,d2
	bgt.s	.Next2
	sub.w	Player_X,d0		; Adapt coordinates
	sub.w	head_height,d1		;  to camera position
	sub.w	Player_Y,d2
	move.w	Y_angle,d3		; Rotate
	jsr	rotate_y1
	tst.w	d2			; Behind camera ?
	bmi.s	.Next2			; Yes -> exit !
	move.w	d2,(a3)+			; Save data in sort list
	move.w	d7,(a3)+
	move.l	a6,(a3)+
	addq.w	#1,Visible_objects		; Count up !!!
	move.w	#$abed,(a6)+		; Save object data
	move.w	d0,(a6)+
	move.w	d1,(a6)+
	move.w	d2,(a6)+
	move.w	d4,(a6)+
	move.b	#$80,(a6)+		; Is NPC object
	move.b	.Animate,(a6)+		; Animation flag
	cmp	#Max_visible_objects,Visible_objects	; Too much objects ?
	bmi.s	.Next2
	movem.l	(sp)+,d2/d3		; Yes -> Break
	bra.s	.Exit
.Next2:	lea.l	Objectingroup_data_size(a0),a0	; Next object in
	dbra	d7,.Loop2				;  group
	movem.l	(sp)+,d2/d3
.Next1:	lea.l	VNPC_data_size(a2),a2	; Next NPC
	addq.w	#1,d3
	cmpi.w	#NPCs_per_map,d3
	bmi	.Loop1
.Exit:	movem.l	(sp)+,d0-d7/a0-a3/a6
	rts

.Animate:	dc.b 0
	even

;*****************************************************************************
; [ Insert object group ]
;   IN : d0 - Byte from map (.w (don't ask))
;        a3 - Pointer to {Dungeon_sort_list} (.l)
;        a6 - Pointer to {Dungeon_object_list} (.l)
; Changed registers : a3,a6
; Notes :
;  - This routine assumes d0 is not zero.
;*****************************************************************************
Insert_object_group:
	movem.l	d0-d7/a0,-(sp)
	cmp.w	Nr_of_groups,d0		; Legal ?
	bgt	.Exit
	move.l	Labdata_ptr,a0		; Get object group data
	subq.w	#1,d0
	mulu.w	#Objectgroup3D_data_size,d0
	lea.l	Lab_data+4(a0,d0.w),a0
	move.w	Yoghurt_X,d5		; Get map square coordinates
	move.w	Yoghurt_Y,d6
	moveq.l	#psize_log,d0		; Convert to dungeon coordinates
	subq.w	#1,d5
	subq.w	#1,d6
	lsl.w	d0,d5
	lsl.w	d0,d6
	moveq.l	#Objects_per_group-1,d7	; Do each object in group
.Loop:	move.w	Object_nr(a0),d4		; Anything there ?
	beq.s	.Next
	move.w	Object_X(a0),d0		; Get coordinates in map
	move.w	Object_Z(a0),d1		;  square
	move.w	Object_Y(a0),d2
	add.w	d5,d0			; Translate to dungeon
	add.w	d6,d2
	sub.w	Player_X,d0		; Adapt coordinates
	sub.w	head_height,d1		;  to camera position
	sub.w	Player_Y,d2
	move.w	Y_angle,d3		; Rotate
	jsr	rotate_y1
	tst.w	d2			; Behind camera ?
	bmi.s	.Next			; Yes -> Skip
	move.w	d2,(a3)+			; Save data in sort list
	move.w	d7,(a3)+
	move.l	a6,(a3)+
	addq.w	#1,Visible_objects		; Count up !!!
	move.w	#$abed,(a6)+		; Save object data
	move.w	d0,(a6)+
	move.w	d1,(a6)+
	move.w	d2,(a6)+
	move.w	d4,(a6)+
	move.w	Hash_number,d0		; Save hash number
	bclr	#15,d0			;  (bit is unused)
	eor.w	d7,d0
	move.w	d0,(a6)+
	cmp	#Max_visible_objects,Visible_objects	; Too much objects ?
	bpl.s	.Exit
.Next:	lea.l	Objectingroup_data_size(a0),a0	; Next object in
	dbra	d7,.Loop				;  group
.Exit:	movem.l	(sp)+,d0-d7/a0
	rts

;*****************************************************************************
; [ Convert {color}-variable to wall address ]
;   IN : d0 - {Color}-variable (.w)
;        a0 - Pointer to hash number in {Dungeon_wall_list} (.l)
;  OUT : d6 - Width of wall in pixels (.w)
;        d7 - Height of wall in pixels (.w)
;        a0 - Wall address (.l)
; Changed registers : d0,d2,d4,d5,d6,d7,a0,a1
; Note :
;  - {color} contains {seethru_flag} in the high byte and the wall number
;     in the low byte.
;  - The {Wall_ptrs} array must NOT take care of animation.
;*****************************************************************************
Get_wall_address:
	cmp.w	#256,d0			; Transparent ?
	shi	seethru_flag
	and.w	#$00ff,d0
	lea.l	Wall_data_offsets,a1	; Get wall data
	move.l	0(a1,d0.w*4),a1
	add.l	Labdata_ptr,a1
	move.l	Wall_bits(a1),d2		; Get animation frame 
	move.b	Wall_nr_frames(a1),d4
	move.w	(a0),d5
	jsr	Get_animation_frame
	lea.l	Wall_ptrs,a0		; Get wall address
	move.l	0(a0,d0.w*4),a0
	move.w	Wall_width(a1),d6		; Get wall dimensions
	move.w	Wall_height(a1),d7

;	move.w	#144,d6
;	move.w	#197,d7

	tst.w	d4			; First frame ?
	beq.s	.Exit
	mulu.w	d6,d4			; No -> Select frame
	mulu.w	d7,d4
	add.l	d4,a0
.Exit:	rts

	SECTION	Fast_DATA,data
Test_floor:
	incbin DDT:Omnidata/3D_floors/Floor.001
	even
