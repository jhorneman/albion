
	rsreset
QNorth:	rs.b 1
QEast:	rs.b 1
QSouth:	rs.b 1
QWest:	rs.b 1
QFloor:	rs.b 1
QCeiling:	rs.b 1
QObject:	rs.b 1
	rs.b 1
Q_size:	rs.b 0				; Must be 8 !!!

	movem.l	d0/d1/d4-d7/a0-a2,-(sp)
	XGet	Mapdata_handle,a0
	lea.l	Map_data(a0),a0
	move.b	Q3D_handle,d0
	jsr	Clear_memory
	jsr	Claim_pointer
	move.l	d0,a1
	lea.l	Wall_transparencies,a2
	move.w	Width_of_map,d0		; Skip first line
	move.w	d0,d1
	add.w	d0,a0
	add.w	d0,d0
	add.w	d0,a0
	lsl.w	#3,d1
	add.w	d1,a1
	move.w	Height_of_map,d7		; Do (map height-2) lines
	subq.w	#2+1,d7
.Loop_Y:	addq.l	#3,a0			; Skip first column
	addq.l	#Q_size,a1
	move.w	Width_of_map,d6		; Do (map width-2) columns
	subq.w	#2+1,d6
.Loop_X:	moveq.l	#0,d0			; Get byte from map
	move.b	(a0),d0
	beq.s	.Floor			; If any
	cmp.b	#-1,d0			; Dummy wall ?
	beq	.Next_X
	cmp.w	#First_wall,d0		; No -> Object or wall ?
	bpl.s	.Wall
	move.b	d0,QObject(a1)		; Object
.Floor:	move.w	1(a0),QFloor(a1)		; Copy floor & ceiling
	bra	.Next_X
.Wall:	sub.w	#First_wall,d0		; Wall
	move.b	d0,d5
	addq.b	#1,d5			; !
	move.b	0(a2,d0.w),d4		; Transparent ?
	beq.s	.Opaque
	move.w	1(a0),QFloor(a1)		; Yes -> Copy floor & ceiling
; ---------- Check all four directions ------------
.Opaque:	move.w	Width_of_map,d1		; Get map byte north
	move.w	d1,d0
	add.w	d1,d1
	add.w	d0,d1
	neg.w	d1
	moveq.l	#0,d0
	move.b	0(a0,d1.w),d0
	beq.s	.Do1			; If any
	cmp.b	#-1,d0			; Dummy wall ?
	beq.s	.No1
	sub.w	#First_wall,d0		; No -> Object ?
	bcs.s	.Do1
	tst.b	d4			; Is the ORIGINAL wall
	bne.s	.No1			;  transparent ?
	tst.b	0(a2,d0.w)		; No -> Is THIS wall
	beq.s	.No1			;  transparent ?
.Do1:	move.b	d5,QNorth(a1)		; Do north wall
.No1:	moveq.l	#0,d0			; Get map byte east
	move.b	3(a0),d0
	beq.s	.Do2			; If any
	cmp.b	#-1,d0			; Dummy wall ?
	beq.s	.No2
	sub.w	#First_wall,d0		; No -> Object ?
	bcs.s	.Do2
	tst.b	d4			; Is the ORIGINAL wall
	bne.s	.No2			;  transparent ?
	tst.b	0(a2,d0.w)		; No -> Is THIS wall
	beq.s	.No2			;  transparent ?
.Do2:	move.b	d5,QEast(a1)		; Do east wall
.No2:	move.w	Width_of_map,d1		; Get map byte south
	move.w	d1,d0
	add.w	d1,d1
	add.w	d0,d1
	moveq.l	#0,d0
	move.b	0(a0,d1.w),d0
	beq.s	.Do3			; If any
	cmp.b	#-1,d0			; Dummy wall ?
	beq.s	.No3
	sub.w	#First_wall,d0		; No -> Object ?
	bcs.s	.Do3
	tst.b	d4			; Is the ORIGINAL wall
	bne.s	.No3			;  transparent ?
	tst.b	0(a2,d0.w)		; No -> Is THIS wall
	beq.s	.No3			;  transparent ?
.Do3:	move.b	d5,QSouth(a1)		; Do south wall
.No3:	moveq.l	#0,d0			; Get map byte west
	move.b	-3(a0),d0
	beq.s	.Do4			; If any
	cmp.b	#-1,d0			; Dummy wall ?
	beq.s	.No4
	sub.w	#First_wall,d0		; No -> Object ?
	bcs.s	.Do4
	tst.b	d4			; Is the ORIGINAL wall
	bne.s	.No4			;  transparent ?
	tst.b	0(a2,d0.w)		; No -> Is THIS wall
	beq.s	.No4			;  transparent ?
.Do4:	move.b	d5,QWest(a1)		; Do west wall
.No4:
.Next_X:	addq.l	#3,a0			; Next column
	addq.l	#Q_size,a1
	dbra	d6,.Loop_X
	addq.l	#3,a0			; Skip last column
	addq.l	#Q_size,a1
	dbra	d7,.Loop_Y
	XFree	Q3D_handle
	XFree	Mapdata_handle
	movem.l	(sp)+,d0/d1/d4-d7/a0-a2
	rts

; ---------- Do trick for non-blocking walls ------
	move.l	a0,a1			; Get byte at player's
	move.w	Map_Ycoord,d0		;  position
	subq.w	#1,d0
	mulu.w	dsizex,d0
	add.w	Map_Xcoord,d0
	subq.l	#1,d0
	add.l	d0,a1
	add.l	d0,d0
	add.l	d0,a1
	moveq.l	#0,d0			; Get
	move.b	(a1),d0
	move.b	d0,Saved_map_byte		; Save
	move.l	a1,Saved_map_ptr
	cmp.w	#First_wall,d0		; Is it a wall ?
	bmi.s	.Zero
	clr.b	(a1)			; Trick
.Zero:

; ---------- Reverse trick ------------------------
	move.l	Saved_map_ptr,a0		; Reset
	move.b	Saved_map_byte,(a0)

;*****************************************************************************
; [ Get visible dungeon walls, objects and floors ]
; No registers are restored
;*****************************************************************************
Get_faces:
	cmp.w	#Max_visible_objects,Visible_objects	; Too much objects already ?
	bpl	.Exit
	XGet	Q3D_handle,a0		; Get map address
	move.w	drawstarty,d0
	neg.w	d0
	add.w	dsizey,d0
	subq.w	#1,d0
	mulu.w	dsizex,d0
	add.w	drawstartx,d0
	lsl.w	#3,d0
	add.w	d0,a0
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
	moveq.l	#0,d3			; Clear
	move.w	drawsizey,d6		; Do
	subq.w	#1,d6
.Loop1:	move.w	d6,-(sp)
	move.w	drawstartx,d0
	addq.w	#1,d0
	move.w	d0,Yoghurt_X
	move.w	drawsizex,d7
	subq.w	#1,d7
.Loop2:	move.w	d7,-(sp)
	jsr	Create_floor_polygons
	moveq.l	#0,d0
	move.b	QObject(a0),d0
	beq.s	.No_obj
	jsr	Insert_object_group
; ---------- Check above --------------------------
.No_obj:	move.b	QNorth(a0),d3		; Get north wall
	beq	.blocko
	subq.b	#1,d3
;--- 2d-koos laden ---
	move	drawsizex,d7
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
	move.b	0(a5,d3.w),(a4)+		; Transparency
	move.b	d3,(a4)+			; Wall number
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
	move.b	QSouth(a0),d3		; Get south wall
	beq	.blocku
	subq.b	#1,d3
;--- 2d-koos laden ---
	move	(a1),d0			; x1
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
	move.b	0(a5,d3.w),(a4)+		; Transparency
	move.b	d3,(a4)+			; Wall number
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
	move.b	QWest(a0),d3		; Get west wall
	beq	.blockl
	subq.b	#1,d3
;--- 2d-koos laden ---
	move	drawsizex,d7
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
	move.b	0(a5,d3.w),(a4)+		; Transparency
	move.b	d3,(a4)+			; Wall number
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
	move.b	QEast(a0),d3		; Get east wall
	beq	.blockr
	subq.b	#1,d3
;--- 2d-koos laden ---
	move	drawsizex,d7
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
	move.b	0(a5,d3.w),(a4)+		; Transparency
	move.b	d3,(a4)+			; Wall number
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
	addq.l	#8,a0
	addq.l	#8,a1
	addq.l	#4,a2
	dbra	d7,.Loop2
	addq.w	#1,Yoghurt_Y		; Next Y
	move.w	Hash_number,d0
	sub.w	drawsizex,d0
	add.w	Width_of_map,d0
	move.w	d0,Hash_number
	move.w	dsizex,d0
	add.w	drawsizex,d0
	lsl.w	#3,d0
	sub.w	d0,a0
	addq.l	#8,a1
	addq.l	#4,a2
	move.w	(sp)+,d6
	dbra	d6,.Loop1
.Exit:	move.w	Visible_walls,d0		; Add
	add.w	Visible_objects,d0
	add.w	Visible_floors,d0
	move.w	d0,Visible_elements
	XFree	Q3D_handle
	rts
