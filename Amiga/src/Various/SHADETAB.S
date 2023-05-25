	OPT	D+,X+,K+
	OPT	E-
	OPT	A+,C+,T+,Q+,P+,S+,B+,M+,L+,Z+,J+,V+
	MAC_BUFFL 10000
	LOCOFF

	MC68030


ABSTUFFUNGEN=16



s:
	bsr	init_rgb_split_list	; mundgerechters Format erzeugen

	bsr	generate_helligkeits_liste ; Liste erzeugen
	trap	#0



generate_helligkeits_liste:
	clr.w	d0			; Farb Index setzten
	lea	helligkeits_table(pc),a6 ; Zielarray

nxt_color_nr:
	move.w	d0,-(sp)

	move.l	a6,a5			; Zielarray kopieren

	lea	rgb_split(pc),a0
	movem.w	(a0,d0.w*8),d0-d2	; RGB der Sollfarbe laden

	IFNE 1=1
; fr von 0-Farbe:
	move.l	#$00008000,d7
	move.l	d7,r_value		; Startwerte auf 0.5 setzten
	move.l	d7,g_value
	move.l	d7,b_value
	ELSE
; fr von Farbe-Weiss:
	move.l	#$00008000,d7
	move.w	d0,r_value		; Startwerte auf 0.5 setzten
	move.w	d1,g_value
	move.w	d2,b_value
	sub.w	#$3f,d0			; Weiss
	neg.w	d0
	sub.w	#$3f,d1
	neg.w	d1
	sub.w	#$3f,d2
	neg.w	d2
	ENDIF

	swap	d0			; ins Fixkomma-Format wandeln
	swap	d1
	swap	d2
	clr.w	d0
	clr.w	d1
	clr.w	d2
	divs.l	#ABSTUFFUNGEN-1,d0	; Delta-Werte berechnen
	divs.l	#ABSTUFFUNGEN-1,d1
	divs.l	#ABSTUFFUNGEN-1,d2
	movem.l	d0-d2,delta_values	; und speichern

;----------------------------------------
	moveq	#ABSTUFFUNGEN-1,d7
nxt_color_shade:
	move.w	d7,-(sp)

	move.w	r_value(pc),d0		; = Sollfarbe
	move.w	g_value(pc),d1
	move.w	b_value(pc),d2
	bsr	find_closest_color	; => d0.w = am n„chsten liegende Farbe

;-------
	move.b	d0,(a5)			; Farbnummmer schreiben
	lea	256(a5),a5		; Ptr auf n„chste Helligkeitsgruppe
;-------

	movem.l	delta_values(pc),d0-d2	; rgb=rgb+delta_rgb
	lea	rgb_values(pc),a0
	add.l	d0,(a0)+
	add.l	d1,(a0)+
	add.l	d2,(a0)+

	move.w	(sp)+,d7
	dbra	d7,nxt_color_shade
;----------------------------------------

	addq.l	#1,a6			; n„chste Farbspalte

	move.w	(sp)+,d0
	addq.w	#1,d0			; Farbnummer erh”hen
	cmp.w	#255+1,d0
	bne	nxt_color_nr

	rts


rgb_values:
r_value:DC.L 0
g_value:DC.L 0
b_value:DC.L 0

delta_values:
delta_r:DC.L 0
delta_g:DC.L 0
delta_b:DC.L 0





;*****************************************************************************
; find_closest_color:
;
; Parameter:
;  d0-d2.w = Sollfarbe RGB
; Ergebnis:
;  d0.w= Index der n„chsten Farbe
;*****************************************************************************
find_closest_color:
	lea	rgb_split(pc),a0
	move.l	#$7fffffff,d6		; Min Color Dist DEFAULT

	move.w	#256-1,d7
find_closest_color_loop:
	movem.w	(a0),d3-d5
	sub.w	d0,d3			; Dreidimensoinaler Abstand berechnen
	sub.w	d1,d4
	sub.w	d2,d5
	muls	d3,d3
	muls	d4,d4
	muls	d5,d5
	add.l	d4,d3
	add.l	d5,d3

	cmp.l	d6,d3
	bge.s	.no_new_min
	move.l	a0,a1			; Index merken
	move.l	d3,d6			; New Min merken
.no_new_min:

	addq.l	#8,a0			; Zeiger auf n„chste Farbe
	dbra	d7,find_closest_color_loop

	lea	rgb_split(pc),a0
	sub.l	a0,a1
	move.l	a1,d0
	lsr.w	#3,d0			; /8

	rts




; Palette in R-G-B Words aufteilen
init_rgb_split_list:
	lea	game_colors(pc),a0
	lea	rgb_split(pc),a1
	move.w	#256-1,d7
	clr.w	d1
split_loop:
	move.l	(a0)+,d0		; VGA-RGB laden
	lsl.l	#8,d0

	moveq	#3-1,d6
split_rgb_loop:
	rol.l	#8,d0
	move.b	d0,d1
	move.w	d1,(a1)+
	dbra	d6,split_rgb_loop
	clr.w	(a1)+			; letzter Wert auslassen

	dbra	d7,split_loop
	rts


game_colors:
	DC.L $000000,$020202,$060606,$090909,$0d0d0d,$111111,$151515,$191919
	DC.L $1d1d1d,$212121,$252525,$292929,$2d2d2d,$313131,$353535,$393939
	DC.L $030201,$050302,$060503,$080604,$0a0805,$0c0906,$0e0a07,$0f0c08
	DC.L $110d0a,$130f0b,$15110d,$17130e,$191410,$1b1611,$1d1813,$1f1a15
	DC.L $211c17,$231e19,$25201b,$27221e,$29241f,$2b2622,$2c2824,$2e2a26
	DC.L $302c28,$322e2a,$34312d,$363330,$383532,$3a3735,$3c3937,$3e3c3a
	DC.L $040200,$060300,$080500,$0b0600,$0d0800,$100900,$120b00,$140d00
	DC.L $170f00,$191000,$1c1200,$1e1400,$201600,$231800,$251a00,$281c00
	DC.L $2a1e00,$2c2100,$2e2400,$312600,$332900,$352c00,$372f00,$393200
	DC.L $3b3500,$3e3700,$3e390b,$3e3b14,$3e3c1e,$3e3e29,$3f3f34,$3f3f3f
	DC.L $010500,$010700,$020900,$030b00,$040d00,$050f00,$061100,$071301
	DC.L $081501,$0a1701,$0b1902,$0c1b02,$0e1d03,$0f1f04,$112104,$122305
	DC.L $142506,$162607,$192809,$1c2a0b,$1e2c0e,$212d10,$242f12,$263115
	DC.L $293217,$2c341a,$2e361d,$313820,$343923,$363b27,$393d2a,$3c3f2e
	DC.L $030100,$050200,$080300,$0b0400,$0e0500,$110500,$130600,$160600
	DC.L $190700,$1c0700,$1f0700,$210600,$240600,$270600,$2a0500,$2d0500
	DC.L $2e0700,$300900,$310b00,$330e00,$341000,$361301,$371601,$391901
	DC.L $3a1f04,$3b2407,$3b290a,$3c2e0d,$3d3311,$3d3714,$3e3a17,$3f3e1b
	DC.L $050202,$070303,$080403,$0a0504,$0c0605,$0e0706,$100807,$120908
	DC.L $140b08,$160c09,$170d0b,$190f0d,$1b100e,$1d1210,$1f1310,$211512
	DC.L $231714,$241914,$261a15,$281d16,$291f17,$2b2119,$2d231a,$2f251c
	DC.L $31271e,$332a20,$352c23,$372e26,$393128,$3b342b,$3d362e,$3f3931
	DC.L $040608,$06080b,$080b0e,$0a0e12,$0d1115,$101418,$12181c,$151b1f
	DC.L $181e22,$1b2126,$1e2529,$22282c,$252c30,$292f33,$2d3336,$31373a
	DC.L $040705,$060a07,$080d0a,$0b100c,$0d130f,$101712,$131a15,$171d18
	DC.L $1a201b,$1d231e,$212721,$242a25,$282d29,$2c302c,$303330,$353735
	DC.L $00050d,$000912,$000c15,$000f18,$01121c,$01141e,$021821,$031c24
	DC.L $042128,$08252b,$0d2a2e,$122e32,$183234,$1f3638,$253a3a,$293d3d
	DC.L $08020e,$0b0311,$0e0514,$110717,$150a1a,$190c1e,$1d0f21,$211324
	DC.L $251627,$291a2a,$2d1f2e,$312331,$342833,$372d36,$393238,$3c373b
	DC.L $000013,$000015,$010118,$01011b,$02031e,$040421,$050624,$070827
	DC.L $090a29,$0b0c2c,$0d0e2f,$101132,$121435,$151738,$181a3b,$1b1d3d

	BSS

rgb_split:
	DS.L 2*256

helligkeits_table:
a:
	DS.B 256*ABSTUFFUNGEN
b:

