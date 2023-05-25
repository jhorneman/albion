; Text routines
; Processing
; Written by J.Horneman (In Tune With The Universe)
; Start : 8-2-1994

	XREF	HLC_table

	SECTION	Program,code
;*****************************************************************************
; [ Process a text ]
;   IN : a0 - Pointer to string list (.l)
; All registers are restored
; Notes :
;   - Whenever a line of text shouldn't be justified, it will start with
;     a special {No_just} character. This character cannot be printed.
;*****************************************************************************
Process_text:
	movem.l	d0/d1/d7/a0-a5,-(sp)
; ---------- Merge all strings into one -----------
	move.l	a0,a2
	lea.l	Raw_TPB_handles,a3
	moveq.l	#1,d7
.Again1:	move.l	#TPB_size,d0		; Allocate RTPB
	jsr	Allocate_memory
	move.b	d0,(a3)
	jsr	Claim_pointer		; Get start & end address
	move.l	d0,a0
	lea.l	TPB_size(a0),a1
.Again2:	move.l	(a2)+,d0			; End of list ?
	beq.s	.End
	move.l	d0,a4
.Again3:	move.b	(a4)+,d0			; Read character
	beq.s	.Again2			; End of line ?
	cmp.b	#Command_char,d0		; Command character ?
	bne.s	.No_com
	move.l	a1,-(sp)			; Yes -> Handle command
	lea.l	HLC_table,a1
	jsr	Text_command_handler
	move.l	(sp)+,a1
	bra.s	.Again3
.No_com:	move.b	d0,(a0)+			; Copy character
	cmp.l	a1,a0			; End of RTPB ?
	bmi.s	.Again3
	move.b	(a3)+,d0			; Yes
	jsr	Free_pointer
	addq.w	#1,d7			; Next RTPB
	cmp.w	#Max_TPBs+1,d7		; Too long ?
	bmi.s	.Again1
	move.l	#TEXT_TOO_LONG_ERROR,d0	; Yes
	jmp	Fatal_error
.End:	clr.b	(a0)+			; Insert EOL
	move.w	d7,Nr_raw_TPBs
	move.b	(a3),d0
	jsr	Free_pointer
	LOCAL
; ---------- Format text --------------------------

; Registers :
;  d1 - Line counter
;  d7 - Processed TPB counter
;  a0 - Pointer to processing buffer
;  a1 - Pointer to start of line in processing buffer
;  a5 - Pointer to processed TPB handles

	moveq.l	#1,d7
	lea.l	Processed_TPB_handles,a5
	move.l	#TPB_size,d0		; Allocate first PTPB
	jsr	Allocate_memory
	move.b	d0,(a5)
	jsr	Claim_pointer		; Get start & end address
	move.l	d0,a0
	lea.l	TPB_size(a0),a1
.Next_line:
	jsr	Read_raw_line		; Read line of raw text
	addq.w	#1,d1
	lea.l	Processing_buffer,a0
.Space:	cmp.b	#" ",(a0)+		; Remove spaces at the
	beq.s	.Space			;  start of a line
	subq.l	#1,a0
	move.l	a0,a1
	jsr	Find_wrap_position		; Wrap
	move.b	-1(a0),d0			; Read last character
	beq	.End			; End of text ?
	cmp.b	#CR,d0			; Carriage return ?
	bne.s	.No_CR
	jsr	Write_processed_line	; Yes -> Write line
	bra.s	.Next_line
.No_CR:	subq.l	#1,a0			; No -> Find last space
	bra.s	.Entry
.Again:	move.b	-(a0),d0			; Read character
.Entry:	cmp.b	#" ",d0			; Space ?
	beq.s	.Found
	cmpi.b	#Command_char,d0		; Command	end ?
	bne.s	.No_com
.Seek:	cmpi.b	#Command_char,-(a0)		; Seek command start
	bne.s	.Seek
	move.b	(a0),d0
	bra.s	.Entry
	cmp.l	a0,a1			; Start of line ?
	bmi.s	.Again
.Found:	jsr	Write_processed_line	; Wrap
	bra	.Next_line
.End:	cmp.l	a0,a1			; Last line empty ?
	beq.s	.Empty
	jsr	Write_processed_line	; No -> Write
	bra.s	.Go_on
.Empty:	subq.w	#1,d1			; Yes -> One line less
.Go_on:	move.w	d1,Text_height		; Store height
	movem.l	(sp)+,d0/d1/d7/a0-a5
	rts

;***************************************************************************
; [ Find the position where a line should be wrapped ]
;   IN : a0 - Pointer to text line (.l)
;  OUT : a0 - Pointer to position where the text line should be wrapped (.l)
; Changed registers : a0
; Notes :
;   - This routine will exit if EOL or a CR was encountered, so a check
;     of -1(a0) should be made.
;***************************************************************************
Find_wrap_position:
	movem.l	d0/d2/d4/d7/a1/a2/a4/a6,-(sp)
	movea.l	Font_Sp,a6		; Get current font
	move.l	(a6),a6
	move.l	a6,-(sp)			; Save
	move.l	Font_translation(a6),a1	; Get font data
	move.l	Width_table(a6),a2
	moveq.l	#0,d4			; Length is zero
	moveq.l	#-1,d7			; No previous character
.Again1:	moveq.l	#0,d0			; Read byte from string
	move.b	(a0)+,d0
	beq	.Exit			; Exit if EOL
	cmp.b	#CR,d0			; Carriage return ?
	beq	.Exit
	cmp.b	#Command_char,d0		; Command character ?
	bne.s	.No_com
	move.l	a1,-(sp)			; Yes -> Handle command
	lea.l	LLC_table2,a1
	jsr	Text_command_handler
	move.l	(sp)+,a1
	movea.l	Font_Sp,a6		; Font may have changed
	move.l	(a6),a6
	move.l	Font_translation(a6),a1	; Get font data
	move.l	Width_table(a6),a2
	bra.s	.Again1
.No_com:	cmp.b	#" ",d0			; Space or Solid space ?
	beq.s	.Space
	cmp.b	#Solid_space,d0
	bne.s	.No_space
.Space:	add.w	Width_of_space(a6),d4	; Yes -> Skip pixels
	cmp.w	PA_width,d4		; Wrap ?
	bpl	.Exit
	moveq.l	#-1,d7			; No previous character
	bra.s	.Again1
.No_space:	cmp.w	#32,d0			; Legal ?
	blo.s	.Again1
	move.b	-32(a1,d0.w),d0		; Yes -> Translate
	bmi.s	.Again1
	cmp.w	#-1,d7			; Is there a previous character ?
	beq.s	.No_kern
	move.l	Kerning_table(a6),d2	; Kerning table available ?
	beq.s	.No_kern
	move.l	d2,a4			; Yes
	addq.l	#2,a4
	lsl.w	#8,d7			; Build kerning pair
	move.b	d0,d7
.Again2:	move.w	(a4)+,d2			; End of table ?
	beq.s	.No_kern
	cmp.w	d2,d7			; No -> Found ?
	bne.s	.Again2
	subq.w	#1,d4			; Yes -> Remove pixel
.No_kern:	move.b	d0,d7			; Save
	moveq.l	#0,d2			; Get character width
	move.b	0(a2,d0.w),d2
	add.w	d2,d4			; Skip pixels
	cmp.w	PA_width,d4		; Wrap ?
	bpl.s	.Exit
	add.w	Between_width(a6),d4	; Skip more pixels
	bra.s	.Again1
.Exit:	move.l	a0,a1			; Restore font
	move.l	(sp)+,a0
	jsr	Change_Font
	move.l	a1,a0
	movem.l	(sp)+,d0/d2/d4/d7/a1/a2/a4/a6
	rts

;*****************************************************************************
; [ Write a line of processed text from the text processing buffer ]
;   IN : d7 - Processed TPB counter (.w)
;        a0 - Pointer to processing buffer (.l)
;        a1 - Pointer to start of line in processing buffer (.l)
;        a5 - Pointer to processed TPB handles (.l)
;  OUT : d7 - Processed TPB counter (.l)
;        a5 - Pointer to processed TPB handles (.l)
; Changed registers : d7,a0
; Notes :
;   - Characters will be written from a1 to a0.
;*****************************************************************************
Write_processed_line:
	movem.l	d0/d1/d5/d6/a0-a2,-(sp)
	move.l	a0,d6			; How many characters ?
	sub.l	a1,d6
	move.w	d6,Processing_buffer_index
	move.b	-1(a0),d1			; Read last character
	move.b	(a5),d0			; Get processed text address
	jsr	Claim_pointer
	move.l	d0,a0
	lea.l	TPB_size(a0),a2
	move.w	Processed_text_index,d5
	add.w	d5,a0
	tst.b	d1			; End of text ?
	beq.s	.Yes
	cmp.b	#CR,d1			; Carriage return ?
	bne.s	.Next
.Yes:	move.b	#No_just,(a0)+		; Yes -> No justification
	addq.w	#1,d6
	bra.s	.Entry
.Loop:	move.b	(a1)+,d0			; Copy character
	move.b	d0,(a0)+
.Entry:	addq.w	#1,d5
	tst.b	d0			; End of string ?
	beq.s	.Exit
	cmp.l	a2,a0			; End of PTPB ?
	bmi.s	.Next
	moveq.l	#0,d5			; Yes
	move.b	(a5)+,d0
	jsr	Free_pointer
	addq.w	#1,d7			; Next PTPB
	cmp.w	#Max_TPBs+1,d7		; Too long ?
	bmi.s	.No
	move.l	#TEXT_TOO_LONG_ERROR,d0	; Yes
	jmp	Fatal_error
.No:	move.l	#TPB_size,d0		; Allocate PTPB
	jsr	Allocate_memory
	move.b	d0,(a5)
	jsr	Claim_pointer		; Get start & end address
	move.l	d0,a0
	lea.l	TPB_size(a0),a2
.Next:	dbra	d6,.Loop
.Exit:	move.b	(a5),d0			; End
	jsr	Free_pointer
	move.w	d5,Processed_text_index	; Store indices
	movem.l	(sp)+,d0/d1/d5/d6/a0-a2
	rts

;*****************************************************************************
; [ Read a line of raw text into the text processing buffer ]
; All registers are restored
;*****************************************************************************
Read_raw_line:
	movem.l	d0/d5-d7/a0-a3,-(sp)
	lea.l	Processing_buffer,a1	; Copy unused buffer down
	move.l	a1,a2
	move.w	Processing_buffer_index,d0
	add.w	d0,a2
	sub.w	#Max_line_length,d0
	neg.w	d0
	move.w	d0,d7
	bra.s	.Entry
.Loop:	move.b	(a2)+,(a1)+
.Entry:	dbra	d0,.Loop
	clr.w	Processing_buffer_index
	LOCAL
	tst.b	(a1)			; End of text ?
	beq	.Exit
	lea.l	Raw_TPB_handles,a0		; Get raw text address
	move.w	Raw_TPB_index,d5
	add.w	d5,a0
	move.b	(a0),d0
	jsr	Claim_pointer
	move.l	d0,a2
	lea.l	TPB_size(a2),a3
	move.w	Raw_text_index,d6
	add.w	d6,a2
	sub.w	#Max_line_length,d7		; Fill rest of buffer
	neg.w	d7
	bra.s	.Next
.Loop:	move.b	(a2)+,d0			; Copy character
	move.b	d0,(a1)+
	addq.w	#1,d6
	tst.b	d0			; End of string ?
	bne.s	.Not_end
	move.b	(a0),d0			; Yes -> Exit
	jsr	Free_pointer
	jsr	Free_memory
	clr.b	(a0)
	bra.s	.Exit
.Not_end:	cmp.l	a3,a2			; End of RTPB ?
	bmi.s	.Next
	addq.w	#1,d5			; Yes -> Next RTPB
	moveq.l	#0,d6
	move.b	(a0),d0
	jsr	Free_pointer
	jsr	Free_memory
	clr.b	(a0)+
	move.b	(a0),d0			; Get start & end address
	jsr	Claim_pointer
	move.l	d0,a2
	lea.l	TPB_size(a2),a3
.Next:	dbra	d7,.Loop
	move.b	(a0),d0			; End
	jsr	Free_pointer
	move.w	d5,Raw_TPB_index		; Store indices
	move.w	d6,Raw_text_index
.Exit:	movem.l	(sp)+,d0/d5-d7/a0-a3
	rts



Raw_TPB_index:	ds.w 1		; RTPB currently being read
Processed_buffer_index:	ds.w1	; Processed characters written to current PTPB
Raw_text_index:	ds.w 1		; Raw characters read in current RTPB
Processing_buffer_index:	ds.w 1	; Processed characters in buffer

Nr_raw_TPBs:	ds.w 1		; Number of raw TPB's
Nr_processed_TPBs:	ds.w 1		; Number of processed TPB's

Processing_buffer:	ds.b Max_line_length

Raw_TPB_handles:	ds.b Max_TPBs	; Memory handles
Processed_TPB_handles:	ds.b Max_TPBs
	even
