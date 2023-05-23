; Pack a file

	xdef	_Pack

	xref	_unpacked_length
	xref	_unpacked_address
	xref	_pack_bss
	xref	_packed_length
	xref	_packed_address
	
	section code
_Pack:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	_unpacked_length,d0
	addq.l	#1,d0
	and.l	#$fffffffe,d0
	move.l	_unpacked_address,a0
	move.l	_pack_bss,a4

	move.l	a0,a3
	move.l	d0,d3

	move.l	a3,a0
	move.l	d3,d0
	move.l	a0,a1
	add.l	d0,a0
	move.l	a0,a2
	move.l	a1,a3
	move.l	d0,d7
Loop:	move.b	(a3)+,(a2)+
	subq.l	#1,d7
	bne.s	Loop

;a0 = daten-compare
;a1 = daten-original  (wird ueberschrieben)
;a4 = zeiger auf buffer (26124 bytes gross)
;d0 = laenge

l01fd:	bra.s	l0202
l01fe:	dc.w 0,0
l01ff:	dc.w 0,0
l0200:	dc.w 0,0
l0201:	dc.w 0,0
l0202:	lea	l0200(pc),a2
	move.l	a1,(a2)
	lea	l01fe(pc),a2
	move.l	a0,(a2)
	lea	l01ff(pc),a2
	move.l	d0,(a2)
	move.l	d0,(a1)
	move.b	#6,(a1)
	addq.l	#8,a1
;	lea	l0409(pc),a4	;***
	movea.l a4,a5
	adda.l	#$2204,a5
	movea.l a5,a6
	adda.l	#$2204,a6
	bsr	l020e
	lea	l0227(pc),a3
	lea	l0227(pc),a2
	move.b	#0,(a2)+
	moveq	#-$80,d2
	moveq	#0,d1
	bsr	l0210
l0203:	moveq	#0,d7
	move.w	-2(a3),d7
	cmp.l	d7,d0
	bgt.s	l0204
	move.w	d0,-2(a3)
l0204:	cmpi.w	#2,-2(a3)
	bgt.s	l0205
	move.w	#1,-2(a3)
	or.b	d2,(a3)
	move.b	(a0),(a2)+
	bra.s	l0206
l0205:	move.w	-2(a3),d6
	subq.l	#3,d6
	andi.w	#$0f,d6
	move.w	-4(a3),d7
	ror.w	#8,d7
	lsl.b	#4,d7
	or.b	d7,d6
	rol.w	#8,d7
	move.b	d6,(a2)+
	move.b	d7,(a2)+
;	cmpa.l	l015f,a0	;***
;	blt.s	l0206
;	bsr	l0159		;***
l0206:	lsr.b	#1,d2
	bne.s	l0208
l0207:	move.b	(a3)+,(a1)+
	cmpa.l	a3,a2
	bne.s	l0207
	lea	l0227(pc),a3
	lea	l0227(pc),a2
	move.b	#0,(a2)+
	moveq	#-$80,d2
l0208:	moveq	#0,d7
	move.w	-2(a3),d7
	sub.l	d7,d0
	beq.s	l020a
	subq.w	#1,d7
l0209:	addq.l	#1,a0
	addq.l	#1,d1
	andi.w	#$0fff,d1
	bsr	l021d
	bsr	l0210
	dbra	d7,l0209
	bra	l0203
l020a:	cmp.b	#-$80,d2
	beq.s	l020c
l020b:	move.b	(a3)+,(a1)+
	cmpa.l	a3,a2
	bne.s	l020b
l020c:	move.l	a1,d1
	btst	#0,d1
	beq.s	l020d
	move.b	#0,(a1)+
l020d:	lea	l0201(pc),a3
	suba.l	l0200(pc),a1
	move.l	a1,0(a3)
	movea.l l0200(pc),a3
	suba.l	#8,a1
	move.l	a1,4(a3)
	
;	rts
	bra	Exit

l020e:	move.w	#$2204,d7
l020f:	move.w	#$2000,0(a4,d7.w)
	move.w	#$2000,0(a5,d7.w)
	move.w	#$2000,0(a6,d7.w)
	subq.l	#2,d7
	bpl.s	l020f
	rts
l0210:	movem.l d0-d7/a0-a2,-(sp)
	lsl.w	#1,d1
	moveq	#0,d2
	move.b	(a0),d2
	addi.w	#$1001,d2
	lsl.w	#1,d2
	move.w	#$2000,0(a4,d1.w)
	move.w	#$2000,0(a5,d1.w)
	move.w	#0,-2(a3)
	moveq	#1,d3
l0211:	tst.w	d3
	bmi.s	l0213
	cmpi.w	#$2000,0(a5,d2.w)
	beq.s	l0212
	move.w	0(a5,d2.w),d2
	bra.s	l0215
l0212:	move.w	d1,0(a5,d2.w)
	move.w	d2,0(a6,d1.w)
	bra	l021c
l0213:	cmpi.w	#$2000,0(a4,d2.w)
	beq.s	l0214
	move.w	0(a4,d2.w),d2
	bra.s	l0215
l0214:	move.w	d1,0(a4,d2.w)
	move.w	d2,0(a6,d1.w)
	bra	l021c
l0215:	move.l	d1,d0
	sub.w	d2,d0
	asr.w	#1,d0
	andi.w	#$0fff,d0
	movea.l a0,a2
	suba.l	d0,a2
	addq.l	#1,a2
	movea.l a0,a1
	addq.l	#1,a1
	moveq	#-1,d3
	move.w	#$10,d4
l0216:	cmpm.b	(a1)+,(a2)+
	dbne	d4,l0216
	bhi.s	l0217
	moveq	#1,d3
l0217:	move.w	#$11,d5
	sub.w	d4,d5
	cmp.w	-2(a3),d5
	blt.s	l0211
	bgt.s	l0218
	cmp.w	-4(a3),d0
	bge.s	l0211
l0218:	move.w	d0,-4(a3)
	move.w	d5,-2(a3)
	cmp.w	#$12,d5
	blt.s	l0211
	move.w	0(a6,d2.w),0(a6,d1.w)
	move.w	0(a4,d2.w),0(a4,d1.w)
	move.w	0(a5,d2.w),0(a5,d1.w)
	move.w	0(a4,d2.w),d0
	move.w	d1,0(a6,d0.w)
	move.w	0(a5,d2.w),d0
	move.w	d1,0(a6,d0.w)
	move.w	0(a6,d2.w),d0
	cmp.w	0(a5,d0.w),d2
	beq.s	l0219
	cmp.w	0(a4,d0.w),d2
	beq.s	l021a
	illegal
l0219:	move.w	d1,0(a5,d0.w)
	bra.s	l021b
l021a:	move.w	d1,0(a4,d0.w)
l021b:	move.w	#$2000,0(a6,d2.w)
l021c:	movem.l (sp)+,d0-d7/a0-a2
	rts
l021d:	movem.l d0-d2,-(sp)
	lsl.w	#1,d1
	cmpi.w	#$2000,0(a6,d1.w)
	beq	l0226
	cmpi.w	#$2000,0(a5,d1.w)
	bne.s	l021e
	move.w	0(a4,d1.w),d2
	bra.s	l0222
l021e:	cmpi.w	#$2000,0(a4,d1.w)
	bne.s	l021f
	move.w	0(a5,d1.w),d2
	bra.s	l0222
l021f:	move.w	0(a4,d1.w),d2
	cmpi.w	#$2000,0(a5,d2.w)
	beq.s	l0221
l0220:	move.w	0(a5,d2.w),d2
	cmpi.w	#$2000,0(a5,d2.w)
	bne.s	l0220
	move.w	0(a6,d2.w),d0
	move.w	0(a4,d2.w),0(a5,d0.w)
	move.w	0(a4,d2.w),d0
	move.w	0(a6,d2.w),0(a6,d0.w)
	move.w	0(a4,d1.w),0(a4,d2.w)
	move.w	0(a4,d1.w),d0
	move.w	d2,0(a6,d0.w)
l0221:	move.w	0(a5,d1.w),0(a5,d2.w)
	move.w	0(a5,d1.w),d0
	move.w	d2,0(a6,d0.w)
l0222:	move.w	0(a6,d1.w),0(a6,d2.w)
	move.w	0(a6,d1.w),d0
	cmp.w	0(a5,d0.w),d1
	beq.s	l0223
	cmp.w	0(a4,d0.w),d1
	beq	l0224
	illegal
l0223:	move.w	d2,0(a5,d0.w)
	bra.s	l0225
l0224:	move.w	d2,0(a4,d0.w)
l0225:	move.w	#$2000,0(a6,d1.w)
l0226:	movem.l (sp)+,d0-d2
	rts
	dc.w 0,0
l0227:	dc.w 0,0,0,0,0,0,0,0,0

Exit:	move.l	l0200(pc),a0			;adresse
	move.l	l0201(pc),d0			;laenge
	move.l	d0,_packed_length
	move.l	a0,_packed_address
	movem.l	(a7)+,d0-d7/a0-a6
	rts

	end
