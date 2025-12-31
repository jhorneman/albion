
	XDEF	Save_screen

	SECTION	Program,code
;***************************************************************************	
; [ Save the screen as an IFF-file ]
; All registers are restored
;***************************************************************************
Save_screen:
	movem.l	d0/d1/d5-d7/a0-a2,-(sp)
	lea.l	.Title,a0			; Select a filename
	lea.l	.Pattern,a1
	jsr	File_requester
	cmp.l	#0,a0			; Anything ?
	beq	.Exit
	move.l	a0,-(sp)			; Yes -> Save
	move.l	#100000,d0		; Allocate memory
	jsr	Allocate_memory
	move.l	d0,-(sp)
	jsr	Claim_pointer		; Build IFF-file
	move.l	d0,a0
	move.l	a0,-(sp)
	move.l	#"FORM",(a0)+		; Start FORM chunk
	addq.l	#4,a0
	move.l	a0,-(sp)
	move.l	#"ILBM",(a0)+		; Write FORM type : ILBM
; ---------- Write BMHD chunk ---------------------
	move.l	#"BMHD",(a0)+		; Start BMHD chunk
	addq.l	#4,a0
	move.l	a0,-(sp)
	move.w	#Screen_width,(a0)+		; Raster width
	move.w	#Screen_height,(a0)+	; Raster height
	clr.w	(a0)+			; X
	clr.w	(a0)+			; Y
	move.b	#Screen_depth,(a0)+		; Number of bitplanes
	clr.b	(a0)+			; Masking
	clr.b	(a0)+			; Compression
	clr.b	(a0)+			; Pad byte
	clr.w	(a0)+			; Transparent colour number
	move.b	#10,(a0)+			; X aspect
	move.b	#11,(a0)+			; Y aspect
	move.w	#320,(a0)+		; Page width
	move.w	#200,(a0)+		; Page height	
	move.l	a0,d0			; End BMHD chunk
	move.l	(sp)+,a1
	sub.l	a1,d0
	move.l	d0,-4(a1)
; ---------- Write CMAP chunk ---------------------
	move.l	#"CMAP",(a0)+		; Start CMAP chunk
	addq.l	#4,a0
	move.l	a0,-(sp)
	jsr	Get_full_palette		; Get palette
	move.w	#256*3-1,d7		; Copy
.Loop:	move.b	(a1)+,(a0)+
	dbra	d7,.Loop
	move.l	a0,d0			; End CMAP chunk
	move.l	(sp)+,a1
	sub.l	a1,d0
	move.l	d0,-4(a1)
; ---------- Write CAMG chunk ---------------------
	move.l	#"CAMG",(a0)+		; Start CAMG chunk
	addq.l	#4,a0
	move.l	a0,-(sp)
	move.l	My_mode_ID,(a0)+		; Write mode ID
	move.l	a0,d0			; End CAMG chunk
	move.l	(sp)+,a1
	sub.l	a1,d0
	move.l	d0,-4(a1)
; ---------- Write BODY chunk ---------------------
	move.l	#"BODY",(a0)+		; Start BODY chunk
	addq.l	#4,a0
	move.l	a0,-(sp)
	move.l	On_screen,a1		; Copy bitmap
	move.w	#Screen_height-1,d7
.Loop_Y:	moveq.l	#Screen_depth-1,d5
.Loop_Z:	move.l	a1,a2
	moveq.l	#Trunced_width-1,d6
.Loop_X:	move.w	(a2)+,(a0)+
	dbra	d6,.Loop_X
	lea.l	Bytes_per_plane(a1),a1
	dbra	d5,.Loop_Z
	dbra	d7,.Loop_Y
	move.l	a0,d0			; End BODY chunk
	move.l	(sp)+,a1
	sub.l	a1,d0
	move.l	d0,-4(a1)
; ---------- Save IFF-picture ---------------------
	move.l	a0,d0			; End FORM chunk
	move.l	(sp)+,a1
	sub.l	a1,d0
	move.l	d0,-4(a1)
	move.l	(sp)+,a0			; Get stuff
	move.l	(sp)+,d7
	move.l	(sp)+,a1
	addq.l	#8,d0			; Calculate file length
	moveq.l	#0,d1			; Save file
	jsr	Save_file
	move.b	d7,d0			; Free memory
	jsr	Free_pointer
	jsr	Free_memory
.Exit:	movem.l	(sp)+,d0/d1/d5-d7/a0-a2
	rts

	SECTION	Fast_DATA,data
.Title:	dc.b "Select an IFF-file...",0
.Pattern:	dc.b 0	;"#?.iff",0
	even
