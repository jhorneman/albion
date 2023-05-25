
Mapbuf_width	EQU 20
Mapbuf_height	EQU 11
Mapbuf_size	EQU Mapbuf_width*Mapbuf_height


	rsreset
Underlay:	rs.w 1
Overlay:	rs.w 1
Transport:	rs.w 1
Matrix_underlay:	rs.w 1
Matrix_overlay:	rs.w 1
People:	rs.w 1
Sky_underlay:	rs.w 1
Sky_overlay:	rs.w 1
Mapbuf_entry_size:	rs.b 0



	move.l	#Mapbuf_size*Mapbuf_entry_size,d0
	moveq.l	#0,d1
	lea.l	Map_buffers,a0
	jsr	Fill_memory

; d0 - X
; d1 - Y
; a0 - Map

	mulu.w	Width_of_map,d1		; Get map address
	add.w	d1,d0
	move.w	d0,d1
	add.w	d1,d1
	add.w	d1,d0
	add.w	d0,a0
	lea.l	Map_buffers,a1		; Copy map part to buffer
	Get	Icondata_handle,a2
	move.w	Width_of_map,d5
	sub.w	#Mapbuf_width,d5
	move.w	d5,d1
	add.w	d5,d5
	add.w	d1,d5
	moveq.l	#Mapbuf_height-1,d7
	btst	#0,d0			; Odd or even ?
	bne.s	.Odd
; ---------- EVEN loop ----------------------------
.Loop_Y1:	moveq.l	#Mapbuf_width/2-1,d6
.Loop_X1:	move.w	(a0)+,d0			; Get underlay
	move.b	d0,d1
	and.w	#%0111111111110000,d0
	lsr.w	#4,d0
	lsl.w	#8,d1			; Get overlay
	move.b	(a0)+,d1
	and.w	#%0000111111111111,d1
	move.l	-8+2(a2,d0.w*8),d2		; Get icon data
	move.l	-8+2(a2,d1.w*8),d3
	btst	#Matrix_bit,d2		; Matrix underlay ?
	beq.s	.No1A1
	move.w	d0,Matrix_underlay(a1)
	btst	#In_sky_bit,d3
	bne.s	.No1A0
	move.w	d1,Matrix_overlay(a1)
	bra.s	.Next_X1A
.No1A0:	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X1A
.No1A1:	btst	#In_sky_bit,d2		; Underlay in sky ?
	beq.s	.No1A2
	move.w	d0,Sky_underlay(a1)
	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X1A
.No1A2:	move.w	d0,Underlay(a1)		; Normal underlay
	btst	#Matrix_bit,d3		; Matrix overlay ?
	beq.s	.No1A3
	move.w	d1,Matrix_overlay(a1)
	bra.s	.Next_X1A
.No1A3:	btst	#In_sky_bit,d3		; Overlay in sky ?
	beq.s	.No1A4
	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X1A
.No1A4:	move.w	d1,Overlay(a1)		; Normal overlay
.Next_X1A:	lea.l	Mapbuf_entry_size(a1),a1	; Next
	move.b	(a0)+,d0			; Get underlay
	lsl.w	#8,d0
	move.b	(a0),d0
	and.w	#%0111111111110000,d0
	lsr.w	#4,d0
	move.w	(a0)+,d1			; Get overlay
	and.w	#%0000111111111111,d1
	move.l	-8+2(a2,d0.w*8),d2		; Get icon data
	move.l	-8+2(a2,d1.w*8),d3
	btst	#Matrix_bit,d2		; Matrix underlay ?
	beq.s	.No1B1
	move.w	d0,Matrix_underlay(a1)
	btst	#In_sky_bit,d3
	bne.s	.No1B0
	move.w	d1,Matrix_overlay(a1)
	bra.s	.Next_X1B
.No1B0:	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X1B
.No1B1:	btst	#In_sky_bit,d2		; Underlay in sky ?
	beq.s	.No1B2
	move.w	d0,Sky_underlay(a1)
	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X1B
.No1B2:	move.w	d0,Underlay(a1)		; Normal underlay
	btst	#Matrix_bit,d3		; Matrix overlay ?
	beq.s	.No1B3
	move.w	d1,Matrix_overlay(a1)
	bra.s	.Next_X1B
.No1B3:	btst	#In_sky_bit,d3		; Overlay in sky ?
	beq.s	.No1B4
	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X1B
.No1B4:	move.w	d1,Overlay(a1)		; Normal overlay
.Next_X1B:	lea.l	Mapbuf_entry_size(a1),a1	; Next
	dbra	d6,.Loop_X1
	add.w	d5,a0
	dbra	d7,.Loop_Y1
	bra	.Done
; ---------- ODD loop -----------------------------
.Odd:
.Loop_Y2:	moveq.l	#Mapbuf_width/2-1,d6
.Loop_X2:	move.b	(a0)+,d0			; Get underlay
	lsl.w	#8,d0
	move.b	(a0),d0
	and.w	#%0111111111110000,d0
	lsr.w	#4,d0
	move.w	(a0)+,d1			; Get overlay
	and.w	#%0000111111111111,d1
	move.l	-8+2(a2,d0.w*8),d2		; Get icon data
	move.l	-8+2(a2,d1.w*8),d3
	btst	#Matrix_bit,d2		; Matrix underlay ?
	beq.s	.No2A1
	move.w	d0,Matrix_underlay(a1)
	btst	#In_sky_bit,d3
	bne.s	.No2A0
	move.w	d1,Matrix_overlay(a1)
	bra.s	.Next_X2A
.No2A0:	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X2A
.No2A1:	btst	#In_sky_bit,d2		; Underlay in sky ?
	beq.s	.No2A2
	move.w	d0,Sky_underlay(a1)
	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X2A
.No2A2:	move.w	d0,Underlay(a1)		; Normal underlay
	btst	#Matrix_bit,d3		; Matrix overlay ?
	beq.s	.No2A3
	move.w	d1,Matrix_overlay(a1)
	bra.s	.Next_X2A
.No2A3:	btst	#In_sky_bit,d3		; Overlay in sky ?
	beq.s	.No2A4
	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X2A
.No2A4:	move.w	d1,Overlay(a1)		; Normal overlay
.Next_X2A:	lea.l	Mapbuf_entry_size(a1),a1	; Next
	move.w	(a0)+,d0			; Get underlay
	move.b	d0,d1
	and.w	#%0111111111110000,d0
	lsr.w	#4,d0
	lsl.w	#8,d1			; Get overlay
	move.b	(a0)+,d1
	and.w	#%0000111111111111,d1
	move.l	-8+2(a2,d0.w*8),d2		; Get icon data
	move.l	-8+2(a2,d1.w*8),d3
	btst	#Matrix_bit,d2		; Matrix underlay ?
	beq.s	.No2B1
	move.w	d0,Matrix_underlay(a1)
	btst	#In_sky_bit,d3
	bne.s	.No2B0
	move.w	d1,Matrix_overlay(a1)
	bra.s	.Next_X2B
.No2B0:	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X2B
.No2B1:	btst	#In_sky_bit,d2		; Underlay in sky ?
	beq.s	.No2B2
	move.w	d0,Sky_underlay(a1)
	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X2B
.No2B2:	move.w	d0,Underlay(a1)		; Normal underlay
	btst	#Matrix_bit,d3		; Matrix overlay ?
	beq.s	.No2B3
	move.w	d1,Matrix_overlay(a1)
	bra.s	.Next_X2B
.No2B3:	btst	#In_sky_bit,d3		; Overlay in sky ?
	beq.s	.No2B4
	move.w	d1,Sky_overlay(a1)
	bra.s	.Next_X2B
.No2B4:	move.w	d1,Overlay(a1)		; Normal overlay
.Next_X2B:	lea.l	Mapbuf_entry_size(a1),a1	; Next
	dbra	d6,.Loop_X2
	add.w	d5,a0
	dbra	d7,.Loop_Y2
.Done:



; BOTTOM HORIZONTAL LAYER

	tst.w	Underlay(a1)
	beq	.Done1
; Draw underlay
	tst.w	Overlay(a1)
	beq	.Done1
; Draw overlay
; Set flag : no upper
.Done1:



; MIDDLE VERTICAL LAYER

; Draw transports
; Draw people
	tst.w	Matrix_underlay(a1)
	beq	.No1
; Draw underlay
.No1:	tst.w	Matrix_overlay(a1)
	beq	.Done2
; Draw overlay
; Set flag : no upper
.Done2:


; TOP HORIZONTAL LAYER

; Test flag : no upper
	tst.w	Sky_underlay(a1)
	beq	.No2
; Draw underlay
.No2:	tst.w	Sky_overlay(a1)
	beq	.Done3
; Draw overlay
.Done3:









Map_buffers:
	ds.b Mapbuf_size*Mapbuf_entry_size
