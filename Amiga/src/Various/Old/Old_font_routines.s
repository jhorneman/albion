;***************************************************************************
; [ Insert 8 pixel wide character in line buffer ]
;   IN : d0 - Character number (.w)
;        d5 - X-position in buffer (.w)
;        a6 - Pointer to Font data structure (.l)
; All registers are restored
;***************************************************************************
Insert_8:
	movem.l	d0/d1/d7/a0/a1,-(sp)
	move.l	Font_graphics(a6),a0	; Calculate character address
	mulu.w	Raw_char_size(a6),d0
	add.w	d0,a0
	move.w	d5,d0			; Calculate scroll value
	moveq.l	#7,d1
	and.w	d0,d1
	lea.l	Line_buffer,a1		; Calculate buffer address
	lsr.w	#3,d0
	add.w	d0,a1
	move.w	Raw_char_height(a6),d7	; Load counter
	subq.w	#1,d7
	btst	#0,d0			; Odd or even ?
	beq.s	.Even
.Loop1:	moveq.l	#0,d0			; Load character line
	move.b	(a0)+,d0
	ror.w	d1,d0			; Shift it
	or.b	d0,(a1)+			; Write it
	lsr.w	#8,d0	
	or.b	d0,(a1)
	lea.l	Bytes_per_plane-1(a1),a1
	dbra	d7,.Loop1
	bra.s	.Done
.Even: 	subq.w	#8,d1			; Reverse scroll value
	neg.w	d1
.Loop2:	moveq.l	#0,d0			; Load character line
	move.b	(a0)+,d0
	rol.w	d1,d0			; Shift it
	or.w	d0,(a1)			; Write it
	lea.l	Bytes_per_plane(a1),a1	; Next line
	dbra	d7,.Loop2
.Done:	movem.l	(sp)+,d0/d1/d7/a0/a1
	rts

;***************************************************************************
; [ Insert fat 8 pixel wide character in line buffer ]
;   IN : d0 - Character number (.w)
;        d5 - X-position in buffer (.w)
;        a6 - Pointer to Font data structure (.l)
; All registers are restored
;***************************************************************************
Insert_fat_8:
	movem.l	d0-d2/d7/a0/a1,-(sp)
	move.l	Font_graphics(a6),a0	; Calculate character address
	mulu.w	Raw_char_size(a6),d0
	add.w	d0,a0
	move.w	d5,d0			; Calculate scroll value
	moveq.l	#7,d1
	and.w	d0,d1
	lea.l	Line_buffer,a1		; Calculate buffer address
	lsr.w	#3,d0
	add.w	d0,a1
	move.w	Raw_char_height(a6),d7	; Load counter
	subq.w	#1,d7
	btst	#0,d0			; Odd or even ?
	beq.s	.Even
.Loop1:	moveq.l	#0,d0			; Load character line
	move.b	(a0)+,d0
	ror.w	d1,d0			; Shift it
	move.w	d0,d2			; Make it fat
	ror.w	#1,d2
	or.w	d2,d0
	or.b	d0,(a1)+			; Write it
	lsr.w	#8,d0	
	or.b	d0,(a1)
	lea.l	Bytes_per_plane-1(a1),a1
	dbra	d7,.Loop1
	bra.s	.Done
.Even: 	subq.w	#8,d1			; Reverse scroll value
	neg.w	d1
.Loop2:	moveq.l	#0,d0			; Load character line
	move.b	(a0)+,d0
	rol.w	d1,d0			; Shift it
	move.w	d0,d2			; Make it fat
	ror.w	#1,d2
	or.w	d2,d0
	or.w	d0,(a1)			; Write it
	lea.l	Bytes_per_plane(a1),a1	; Next line
	dbra	d7,.Loop2
.Done:	movem.l	(sp)+,d0-d2/d7/a0/a1
	rts
