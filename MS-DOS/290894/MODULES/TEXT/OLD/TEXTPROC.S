; Text routines
; Processing
; Written by J.Horneman (In Tune With The Universe)
; Start : 8-2-1994

	XREF	VScroll_block
	XREF	Read_Mev
	XREF	Mouse_Y
	XREF	Copy_screen
	XREF	Switch_screens
	XREF	Update_screen

	XDEF	Process_text
	XDEF	Process_text_list
	XDEF	Display_text
	XDEF	Display_text_list
	XDEF	Display_processed_text
	XDEF	Nr_of_lines
	XDEF	Text_height


	SECTION	Program,code
;*****************************************************************************
; [ Display a text ]
;   IN : a0 - Pointer to text (.l)
; All registers are restored
;*****************************************************************************
Display_text:
	movem.l	a0/a1,-(sp)
	lea.l	One_list,a1
	move.l	a0,(a1)
	move.l	a1,a0
	jsr	Process_text_list
	jsr	Display_processed_text
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Display a string list ]
;   IN : a0 - Pointer to string list (.l)
; All registers are restored
;*****************************************************************************
Display_text_list:
	jsr	Process_text_list

; !!! ROUTINE CONTINUES !!!

;*****************************************************************************
; [ Display a processed text ]
; All registers are restored
;*****************************************************************************
Display_processed_text:
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	PA_Sp,a5			; Get PA
	move.l	(a5),a5
	move.w	Y1(a5),d0			; Set initial scroll index
	sub.w	Y2(a5),d0
	move.w	d0,Current_scroll_index
	moveq.l	#9,d3			; Scrolling speed
	moveq.l	#0,d2			; Calculate number of lines
	move.w	Text_height,d2
	divu.w	d3,d2
.Loop:	move.w	Y1(a5),d0			; Display text
	move.w	Y2(a5),d1
	jsr	Refresh_text_window
	jsr	Switch_screens
	add.w	d3,Current_scroll_index	; Scroll
.Entry:	dbra	d2,.Loop
	jsr	Copy_screen		; Update
	jsr	Destroy_processed_text	; Destroy text
	tst.b	Text_wait_flag		; Wait for user ?
	beq.s	.No_wait
	jsr	Wait_4_user		; Yes
	jsr	Erase_PA			; Clear print area
	jsr	Update_screen
.No_wait:	movem.l	(sp)+,d0-d7/a0-a6
	rts


	ifne	FALSE
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	PA_Sp,a5
	move.l	(a5),a5
	move.l	a5,a0
	jsr	Prepare_PA

	moveq.l	#1,d3

	move.w	Y1(a5),d0
	sub.w	Y2(a5),d0
	move.w	d0,Current_scroll_index

	move.w	Y1(a5),d0
	move.w	Y2(a5),d1
	jsr	Refresh_text_window
	jsr	Switch_screens
	add.w	d3,Current_scroll_index
	jsr	Refresh_text_window
	jsr	Switch_screens

	move.w	Text_height,d2
	subq.w	#2,d2
.Loop:	move.w	X1(a5),d0
	and.w	#$fff0,d0
	move.w	Y1(a5),d1
	move.w	d3,d4
	add.w	d4,d4
	neg.w	d4
	moveq.l	#Screen_depth,d5
	move.w	X2(a5),d6
	sub.w	d0,d6
	add.w	#15,d6
	lsr.w	#4,d6
	move.w	Y2(a5),d7
	sub.w	d1,d7
	addq.w	#1,d7
	jsr	VScroll_block
	add.w	d3,Current_scroll_index
	move.w	Y2(a5),d1
	move.w	d1,d0
	sub.w	d3,d0
	sub.w	d3,d0
	subq.w	#1,d0
	jsr	Refresh_text_window
	jsr	Switch_screens
	dbra	d2,.Loop

	jsr	Copy_screen
	jsr	Destroy_processed_text
	tst.b	Text_wait_flag
	beq.s	.No_wait
	jsr	Wait_4_user
	jsr	Erase_PA
	jsr	Update_screen
.No_wait:
	movem.l	(sp)+,d0-d7/a0-a6
	rts
	endc

;*****************************************************************************
; [ Refresh text window ]
;   IN : d0 - Top screen Y-coordinate (.w)
;        d1 - Bottom screen Y-coordinate (.w)
; All registers are restored
;*****************************************************************************
Refresh_text_window:
	movem.l	d0-d2/d5-d7/a0-a2,-(sp)
	lea.l	Text_scroll_CA,a0		; Build CA
	move.l	PA_Sp,a1
	move.l	(a1),a1
	move.w	X1(a1),X1(a0)
	move.w	X2(a1),X2(a0)
	move.w	d0,Y1(a0)
	move.w	d1,Y2(a0)
	jsr	Push_CA
	jsr	Erase_PA			; Erase
	move.l	Font_Sp,a0		; Duplicate font
	move.l	(a0),a0
	jsr	Push_Font
	move.w	d1,d5			; Save
	move.w	d0,d6			; Get text Y-coordinate
	add.w	Current_scroll_index,d6
	lea.l	Processed_TPB_handles,a2	; Search first line
	Get	(a2),a0
	move.w	Y1(a1),d1
	moveq.l	#0,d2
	move.w	Nr_of_lines,d7
	cmp.w	d1,d6			; Start ?
	beq.s	.Found
	bra.s	.Entry
.Loop1:	cmp.b	#EOTPB,(a0)		; End of TPB ?
	bne.s	.Do1
	move.b	(a2)+,d0			; Yes -> Next TPB
	jsr	Free_pointer
	Get	(a2),a0
.Do1:	moveq.l	#0,d2			; Add line skip
	move.b	Line_skip(a0),d2
	add.w	d2,d1
	cmp.w	d6,d1			; Is this the line ?
	bpl.s	.Found
	moveq.l	#0,d0			; Next line
	move.b	String_length(a0),d0
	add.w	#Line_info_data_size+1,d0
	btst	#0,d0
	beq.s	.Even1
	addq.w	#1,d0
.Even1:	add.w	d0,a0
.Entry:	dbra	d7,.Loop1
	bra	.Done			; Done
.Found:	sub.w	d2,d1			; Get screen Y-coordinate
	sub.w	Current_scroll_index,d1
.Loop2:	move.w	X1(a1),d0		; Print line
	jsr	Put_processed_text_line
	moveq.l	#0,d0			; Next line
	move.b	String_length(a0),d0
	add.w	#Line_info_data_size+1,d0
	btst	#0,d0
	beq.s	.Even2
	addq.w	#1,d0
.Even2:	add.w	d0,a0
	cmp.b	#EOTPB,(a0)		; End of TPB ?
	bne.s	.Do2
	move.b	(a2)+,d0			; Yes -> Next TPB
	jsr	Free_pointer
	Get	(a2),a0
.Do2:	add.w	Current_line_skip,d1	; Add line skip
	cmp.w	d5,d1			; Outside area ?
	bpl.s	.Done
	dbra	d7,.Loop2
.Done:	move.b	(a2),d0			; The end
	jsr	Free_pointer
	Pop	Font
	Pop	CA
	movem.l	(sp)+,d0-d2/d5-d7/a0-a2
	rts

;*****************************************************************************
; [ Erase print area ]
; All registers are restored
;*****************************************************************************
Erase_PA:
	movem.l	d0-d7/a0/a1,-(sp)
	move.l	PA_Sp,a1			; Get PA
	move.l	(a1),a1
	move.w	PA_Paper(a1),d4		; Transparent paper ?
	cmp.w	#-1,d4
	bne.s	.No
	tst.b	PA_bg_handle(a1)		; Yes -> Background created ?
	bne.s	.Skip
	move.l	a1,a0			; No -> Create
	jsr	Prepare_PA
.Skip:	move.w	X1(a1),d0			; Restore background
	and.w	#$fff0,d0
	move.w	Y1(a1),d1
	moveq.l	#Screen_depth,d5
	move.w	X2(a1),d6
	sub.w	d0,d6
	add.w	#15,d6
	lsr.w	#4,d6
	move.w	PA_height,d7
	Get	PA_bg_handle(a1),a0
	jsr	Put_unmasked_block
	Free	PA_bg_handle(a1)
	bra.s	.Exit
.No:	move.w	X1(a1),d0			; No -> Draw paper box
	move.w	Y1(a1),d1
	move.w	X2(a1),d2
	move.w	Y2(a1),d3
	jsr	Draw_box
.Exit:	movem.l	(sp)+,d0-d7/a0/a1
	rts

;*****************************************************************************
; [ Prepare a PA for printing ]
;   IN : a0 - Pointer to PA (.l)
; All registers are restored
;*****************************************************************************
Prepare_PA:
	movem.l	d0/d1/d5-d7/a0/a1,-(sp)
	move.l	a0,a1
	cmp.w	#-1,PA_Paper(a1)		; Transparent paper ?
	bne.s	.Exit
	move.w	X1(a1),d0			; Yes
	and.w	#$fff0,d0
	move.w	X2(a1),d6
	sub.w	d0,d6
	add.w	#15,d6
	lsr.w	#4,d6
	move.w	PA_height,d7
	tst.b	PA_bg_handle(a1)		; Buffer created ?
	bne.s	.Skip
	move.w	d6,d0			; No -> Allocate
	mulu.w	d7,d0			;  background buffer
	mulu.w	#Screen_depth*2,d0
	jsr	Allocate_CHIP
	move.b	d0,PA_bg_handle(a1)
.Skip:	Get	PA_bg_handle(a1),a0		; Save background
	move.w	X1(a1),d0
	move.w	Y1(a1),d1
	moveq.l	#Screen_depth,d5
	jsr	Get_block
	move.b	PA_bg_handle(a1),d0
	jsr	Free_pointer
.Exit:	movem.l	(sp)+,d0/d1/d5-d7/a0/a1
	rts

;*****************************************************************************
; [ Process a text ]
;   IN : a0 - Pointer to text (.l)
; All registers are restored
;*****************************************************************************
Process_text:
	movem.l	a0/a1,-(sp)
	lea.l	One_list,a1
	move.l	a0,(a1)
	move.l	a1,a0
	jsr	Process_text_list
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Process a string list ]
;   IN : a0 - Pointer to string list (.l)
; All registers are restored
;*****************************************************************************
Process_text_list:
	movem.l	d0-d3/d7/a0-a5,-(sp)
	clr.w	Raw_TPB_index		; Clear
	clr.w	Raw_text_index
	clr.w	Processed_text_index
	clr.w	Processing_buffer_index
	clr.w	Text_height
	clr.w	Nr_of_lines
	movea.l	Font_Sp,a1		; Get current font
	move.l	(a1),a1
	move.l	a1,-(sp)			; Save
	move.b	#Print_justified,Current_justification
	move.l	#Normal_style,Current_text_style
; ---------- Merge all strings into one -----------
	move.l	a0,a2
	lea.l	Raw_TPB_handles,a3
	moveq.l	#1,d7
	move.l	#TPB_size,d0		; Allocate first RTPB
	jsr	Allocate_memory
	move.b	d0,(a3)
	jsr	Claim_pointer		; Get start & end address
	move.l	d0,a1
	lea.l	TPB_size(a1),a4
.Again2:	move.l	(a2)+,d0			; End of list ?
	beq	.End1
	move.l	d0,a0
.Again3:	move.b	(a0)+,d0			; Read character
	beq.s	.Again2			; End of line ?
	cmp.b	#Comstart_char,d0		; Command character ?
	bne.s	.No_com
	move.l	a0,d1			; Yes
	move.b	(a0)+,d0			; Get command
	lsl.w	#8,d0
	move.b	(a0)+,d0
	swap	d0
	move.b	(a0)+,d0
	lsl.w	#8,d0
	move.b	(a0)+,d0
	lea.l	HLC_table,a5		; Search
.Again4:	tst.l	(a5)			; End of command list ?
	beq.s	.End2
	cmp.l	(a5)+,d0			; No -> Found ?
	beq.s	.Found
	addq.l	#4,a5			; No -> Next
	bra.s	.Again4
.Found:	move.l	(a5),a5			; Yes -> Execute
	jsr	(a5)
.Seek1:	cmp.b	#Comend_char,(a0)+		; End of command ?
	bne.s	.Seek1
	bra.s	.Again3
.End2:	move.l	d1,a0			; Copy command
	move.b	#Comstart_char,d0
.No_com:	move.b	d0,(a1)+			; Copy character
	cmp.l	a4,a1			; End of RTPB ?
	bmi.s	.Again3
	move.b	(a3)+,d0			; Yes
	jsr	Free_pointer
	addq.w	#1,d7			; Next RTPB
	cmp.w	#Max_TPBs+1,d7		; Too long ?
	bpl.s	.Too_long
	move.l	#TPB_size,d0		; No -> Allocate RTPB
	jsr	Allocate_memory
	move.b	d0,(a3)
	jsr	Claim_pointer		; Get start & end address
	move.l	d0,a1
	lea.l	TPB_size(a1),a4
	bra	.Again3
.Too_long:	move.l	#TEXT_TOO_LONG_ERROR,d0	; Yes
	jmp	Fatal_error
.End1:	clr.b	(a1)+			; Insert EOL
	move.w	d7,Nr_raw_TPBs
	move.b	(a3),d0
	jsr	Free_pointer
	LOCAL
; ---------- Format text --------------------------

; Registers :
;  d1 - Line counter
;  d3 - Height of text in pixels
;  d7 - Processed TPB counter
;  a0 - Pointer to processing buffer
;  a1 - Pointer to start of line in processing buffer
;  a5 - Pointer to processed TPB handles

	moveq.l	#0,d1
	moveq.l	#0,d3
	moveq.l	#1,d7
	lea.l	Processed_TPB_handles,a5
	move.l	#TPB_size,d0		; Allocate first PTPB
	jsr	Allocate_memory
	move.b	d0,(a5)
.Next_line:
	jsr	Read_raw_line		; Read line of raw text
	addq.w	#1,d1
	lea.l	Processing_buffer,a0
	move.l	a0,a1
	jsr	Find_wrap_position		; Wrap where ?
	move.l	a0,d2
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
	bne.s	.No1
	jsr	Write_processed_line
	addq.w	#1,Processing_buffer_index
	bra	.Next_line
.No1:	cmp.b	#Hyphen1,d0		; Hyphen ?
	beq.s	.Yes
	cmp.b	#Hyphen2,d0
	bne.s	.No2
.Yes:	addq.l	#1,a0
	jsr	Write_processed_line
	bra	.Next_line
.No2:	cmp.b	#Separator,d0		; Separator ?
	bne.s	.No3
	move.b	#"-",(a0)+
	jsr	Write_processed_line
	bra	.Next_line
.No3:	cmpi.b	#Comend_char,d0		; Command	end ?
	bne.s	.No_com
.Seek:	cmpi.b	#Comstart_char,-(a0)	; Seek command start
	bne.s	.Seek
	bra.s	.Again
.No_com:	cmp.l	a0,a1			; Start of line ?
	bmi.s	.Again
	move.l	d2,a0			; Yes -> Break off
	subq.l	#1,a0
	jsr	Write_processed_line
	bra	.Next_line
.End:	cmp.l	a0,a1			; Last line empty ?
	beq.s	.Empty
	jsr	Write_processed_line	; No -> Write
.Empty:	move.w	d1,Nr_of_lines		; Store height
	move.l	(sp)+,a0			; Restore font
	jsr	Change_Font
	movem.l	(sp)+,d0-d3/d7/a0-a5
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
	movem.l	d0/d2/d4/d5/d7/a1/a2/a4/a6,-(sp)
	moveq.l	#0,d4			; Length is zero
	move.w	PA_width,d5
	subq.w	#1,d5
	moveq.l	#-1,d7			; No previous character
	movea.l	Font_Sp,a6		; Get current font
	move.l	(a6),a6
	move.l	([Current_text_style],a6.l),a6
	move.l	Font_translation(a6),a1	; Get font data
	move.l	Width_table(a6),a2
.Again1:	moveq.l	#0,d0			; Read byte from string
	move.b	(a0)+,d0
	beq	.Exit			; Exit if EOL
	cmp.b	#CR,d0			; Carriage return ?
	beq	.Exit
	cmp.b	#Comstart_char,d0		; Command character ?
	bne.s	.No_com
	cmp.w	d5,d4			; Wrap ?
	bmi.s	.No
	subq.l	#1,a0			; Yes
	bra	.Exit
.No:	move.l	a1,-(sp)			; No -> Handle command
	lea.l	LLC_table2,a1
	jsr	LLC_handler
	move.l	(sp)+,a1
	moveq.l	#-1,d7			; No previous character
	movea.l	Font_Sp,a6		; Get current font
	move.l	(a6),a6
	move.l	([Current_text_style],a6.l),a6
	move.l	Font_translation(a6),a1	; Get font data
	move.l	Width_table(a6),a2
	cmp.w	d5,d4			; Wrap ?
	bpl.s	.Exit
	bra.s	.Again1
.No_com:	cmp.b	#" ",d0			; Space or Solid space ?
	beq.s	.Space
	cmp.b	#Solid_space,d0
	bne.s	.No_space
.Space:	add.w	Width_of_space(a6),d4	; Yes -> Skip pixels
	cmp.w	d5,d4			; Wrap ?
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
	cmp.w	d5,d4			; Wrap ?
	bpl.s	.Exit
	add.w	Between_width(a6),d4	; Skip more pixels
	bra	.Again1
.Exit:	movem.l	(sp)+,d0/d2/d4/d5/d7/a1/a2/a4/a6
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
	movem.l	d0/d1/d5/d6/a0/a1/a6,-(sp)
	move.w	Processed_text_index,d5
	move.l	a0,d6			; How many characters ?
	sub.l	a1,d6
	move.w	d6,Processing_buffer_index
	move.b	(a0),d0			; Fake EOL
	clr.b	(a0)
	jsr	Analyse_line		; Analyse
	move.b	d0,(a0)
	lea.l	Current_line_info,a6
	cmp.b	#Print_justified,Justification(a6)	; Justified ?
	bne.s	.No1
	move.b	-1(a0),d0			; Read last character
	beq.s	.End			; End of text ?
	cmp.b	#CR,d0			; Carriage return ?
	bne.s	.No1
.End:	move.b	#Print_left,Justification(a6)	; Yes -> No justification
.No1:	moveq.l	#0,d0			; Will it fit ?
	move.b	String_length(a6),d0
	add.w	#Line_info_data_size+1,d0	; (+1 for EOL)
	btst	#0,d0
	beq.s	.Even1
	addq.w	#1,d0
.Even1:	add.w	d5,d0
	cmp.w	#TPB_size-1,d0
	bmi.s	.Ok2
	move.b	(a5),d0			; No -> Insert EOTPB
	jsr	Claim_pointer
	move.l	d0,a0
	move.b	#EOTPB,0(a0,d5.w)
	move.b	(a5)+,d0
	jsr	Free_pointer
	moveq.l	#0,d5			; Next PTPB
	addq.w	#1,d7
	cmp.w	#Max_TPBs+1,d7		; Too long ?
	bmi.s	.No2
	move.l	#TEXT_TOO_LONG_ERROR,d0	; Yes
	jmp	Fatal_error
.No2:	move.l	#TPB_size,d0		; Allocate PTPB
	jsr	Allocate_memory
	move.b	d0,(a5)
.Ok2:	move.b	(a5),d0			; Get processed text address
	jsr	Claim_pointer
	move.l	d0,a0
	add.w	d5,a0
	moveq.l	#0,d0			; Get string length
	move.b	String_length(a6),d0
	moveq.l	#Line_info_data_size,d1	; Copy line info
	add.w	d1,d5
	subq.w	#1,d1
.Loop:	move.b	(a6)+,(a0)+
	dbra	d1,.Loop
	jsr	Strncpy			; Copy actual text
	add.w	d0,d5
	add.w	d0,a0
	clr.b	(a0)+			; Write EOL
	addq.w	#1,d5
	btst	#0,d5			; Make even
	beq.s	.Even2
	addq.w	#1,d5
.Even2:	move.w	d5,Processed_text_index	; Store
	move.b	(a5),d0			; End
	jsr	Free_pointer
.Exit2:	movem.l	(sp)+,d0/d1/d5/d6/a0/a1/a6
	rts

;*****************************************************************************
; [ Read a line of raw text into the text processing buffer ]
; All registers are restored
;*****************************************************************************
Read_raw_line:
	movem.l	d0/d5-d7/a0-a3,-(sp)
	lea.l	Processing_buffer,a1	; Copy unused buffer down
	move.l	a1,a2
	move.w	#Max_line_length,d7
	move.w	Processing_buffer_index,d0
	beq.s	.Empty
	add.w	d0,a2
	move.w	d0,d7
	sub.w	#Max_line_length,d0
	neg.w	d0
	bra.s	.Entry1
.Loop1:	move.b	(a2)+,(a1)+
.Entry1:	dbra	d0,.Loop1
	clr.w	Processing_buffer_index
	tst.b	(a1)			; End of text ?
	beq	.Exit
.Empty:	lea.l	Raw_TPB_handles,a0		; Get raw text address
	move.w	Raw_TPB_index,d5
	add.w	d5,a0
	move.b	(a0),d0
	jsr	Claim_pointer
	move.l	d0,a2
	lea.l	TPB_size(a2),a3
	move.w	Raw_text_index,d6
	add.w	d6,a2
	bra.s	.Next2			; Fill rest of buffer
.Loop2:	move.b	(a2)+,d0			; Copy character
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
	bmi.s	.Next2
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
.Next2:	dbra	d7,.Loop2
	move.b	(a0),d0			; End
	jsr	Free_pointer
	move.w	d5,Raw_TPB_index		; Store indices
	move.w	d6,Raw_text_index
.Exit:	movem.l	(sp)+,d0/d5-d7/a0-a3
	rts

;***************************************************************************
; [ Analyse a text line ]
;   IN : a1 - Pointer to text line (.l)
;  OUT : a1 - Pointer to start of text line (.l)
; All registers are restored
; Notes :
;   - This routine will only be effective if all strings in a text are
;     analysed. [ Justification ] must be set to [ Print_justified ] before
;     this is done.
;   - The print method for the last line in the text or texts ending with
;     a carriage return must be manually set to [ Print_left ].
;***************************************************************************
Analyse_line:
	movem.l	d0-d7/a0/a2-a6,-(sp)
	movea.l	Font_Sp,a6		; Get current font
	move.l	(a6),a6
	move.l	a6,-(sp)			; Save
; ---------- Reset line info ----------------------
	lea.l	Current_line_info,a5
	move.l	a6,Initial_font(a5)
	move.w	Ink_colour,Initial_ink(a5)
	move.w	Shadow_colour,Initial_shadow(a5)
	clr.w	Line_width(a5)
	clr.w	Width_without_spaces(a5)
	clr.b	Line_height(a5)
	clr.b	Line_skip(a5)
	clr.b	Base_line(a5)
	clr.b	Nr_of_spaces(a5)
	move.l	Current_text_style,d0
	move.b	d0,Initial_style(a5)
	clr.b	String_length(a5)
; ---------- Trim ---------------------------------
.Skip1:	move.b	(a1)+,d0			; Skip leading spaces
	beq	.Exit
	cmp.b	#" ",d0
	beq.s	.Skip1
	subq.l	#1,a1
	move.l	a1,-(sp)			; Save start
	move.l	a1,a0
	jsr	Strlen			; Get string length
.Skip2:	cmp.b	#" ",-1(a1,d0.w)		; Ignore trailing spaces
	bne.s	.Done1
	subq.w	#1,d0
	bne.s	.Skip2
.Done1:	clr.b	0(a1,d0.w)		; Insert EOL
	move.b	d0,String_length(a5)	; Save string length
; ---------- Analyse ------------------------------
	move.l	([Current_text_style],a6.l),a6
	move.w	Raw_char_height(a6),d1	; Get initial heights
	move.w	Between_height(a6),d2
	move.w	Font_base_line(a6),d3
	sub.w	d3,d1
	moveq.l	#0,d4			; Length with spaces
	moveq.l	#0,d5			; Length without spaces
	moveq.l	#0,d6			; Number of spaces
	moveq.l	#-1,d7			; No previous character
	move.l	Font_translation(a6),a2	; Get font data
	move.l	Width_table(a6),a3
.Again1:	moveq.l	#0,d0			; Read byte from string
	move.b	(a1)+,d0
	beq	.Done2			; Exit if EOL
	cmp.b	#Comstart_char,d0		; Command character ?
	bne.s	.No_com
	jsr	LLCA_handler		; Yes -> Handle it
	moveq.l	#-1,d7			; No previous character
	movea.l	Font_Sp,a6		; Get current font
	move.l	(a6),a6
	move.l	([Current_text_style],a6.l),a6
	move.l	Font_translation(a6),a2	; Get font data
	move.l	Width_table(a6),a3
	move.w	Raw_char_height(a6),d0	; Get height under base-line
	sub.w	Font_base_line(a6),d0
	cmp.w	d0,d1			; Higher ?
	bpl.s	.No1
	move.w	d0,d1			; Yes
.No1:	move.w	Font_base_line(a6),d0	; Get base line
	cmp.w	d0,d3			; Higher ?
	bpl.s	.Again1
	move.w	d0,d3			; Yes
	move.w	Between_height(a6),d2
	bra.s	.Again1
.No_com:	cmp.b	#" ",d0			; Space or Solid space ?
	beq.s	.Space
	cmp.b	#Solid_space,d0
	bne.s	.No_space
.Space:	add.w	Width_of_space(a6),d4	; Yes -> Skip pixels
	addq.w	#1,d6			; Count up
	moveq.l	#-1,d7			; No previous character
	bra.s	.Again1
.No_space:	cmp.w	#32,d0			; Legal ?
	blo.s	.Again1
	move.b	-32(a2,d0.w),d0		; Yes -> Translate
	bmi.s	.Again1
	move.l	d1,-(sp)
	cmp.w	#-1,d7			; Is there a previous character ?
	beq.s	.No_kern
	move.l	Kerning_table(a6),d1	; Kerning table available ?
	beq.s	.No_kern
	move.l	d1,a4			; Yes
	addq.l	#2,a4
	lsl.w	#8,d7			; Build kerning pair
	move.b	d0,d7
.Again2:	move.w	(a4)+,d1			; End of table ?
	beq.s	.No_kern
	cmp.w	d1,d7			; No -> Found ?
	bne.s	.Again2
	subq.w	#1,d4			; Yes -> Remove pixel
	subq.w	#1,d5
.No_kern:	move.b	d0,d7			; Save
	moveq.l	#0,d1			; Get character width
	move.b	0(a3,d0.w),d1
	add.w	d1,d4			; Skip pixels
	add.w	d1,d5
	move.l	(sp)+,d1
	add.w	Between_width(a6),d4	; Skip more pixels
	add.w	Between_width(a6),d5
	bra	.Again1
.Done2:	sub.w	Between_width(a6),d4	; Remove last pixels
	sub.w	Between_width(a6),d5
	add.w	d3,d1			; Store data
	move.b	d1,Line_height(a5)
	add.w	d1,d2

; This is probably nonsense because Line_skip(a5) is always zero.
;	cmp.b	Line_skip(a5),d2		; Higher than current ?
;	bmi.s	.No2
;	move.b	d2,Line_skip(a5)		; Yes
;.No2:
	move.b	d3,Base_line(a5)
	move.w	d4,Line_width(a5)
	move.w	d5,Width_without_spaces(a5)
	move.b	d6,Nr_of_spaces(a5)
	move.l	(sp)+,a1
.Exit:	move.l	(sp)+,a0			; Restore font
	jsr	Change_Font
	move.b	Current_justification,Justification(a5)
	moveq.l	#0,d0			; Increase height
	move.b	Line_skip(a5),d0
	add.w	d0,Text_height
	movem.l	(sp)+,d0-d7/a0/a2-a6
	rts

;*****************************************************************************
; [ Destroy processed text ]
; All registers are restored
;*****************************************************************************
Destroy_processed_text:
	movem.l	d0/d7/a0,-(sp)
	lea.l	Processed_TPB_handles,a0
	move.w	Nr_raw_TPBs,d7
	bra.s	.Entry
.Loop:	move.b	(a0)+,d0
	jsr	Free_memory
.Entry:	dbra	d7,.Loop
	movem.l	(sp)+,d0/d7/a0
	rts







	ifne	FALSE
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	PA_Sp,a5
	move.l	(a5),a5

	moveq.l	#1,d3

	move.w	Y1(a5),d0
	sub.w	Y2(a5),d0
	move.w	d0,Current_scroll_index

	move.w	Text_height,d2
	subq.w	#2,d2
.Loop:	jsr	Erase_PA
	move.w	Y1(a5),d0
	move.w	Y2(a5),d1
	jsr	Refresh_text_window
	jsr	Switch_screens
	add.w	d3,Current_scroll_index
	dbra	d2,.Loop

	jsr	Copy_screen
	jsr	Destroy_processed_text
	jsr	Wait_4_click

	movem.l	(sp)+,d0-d7/a0-a6
	rts

;*****************************************************************************
; [ Refresh text window ]
;   IN : d0 - Vertical offset in pixels (.w)
; All registers are restored
;*****************************************************************************
Refresh_text_window:
	movem.l	d0-d2/d5-d7/a0,-(sp)
	move.w	Current_text_offset,d1
	bmi	.Show
	sub.w	d0,d1
	beq	.Exit
	bpl	.Up

	move.w	d0,d5			; Save input
	move.w	d1,d6
	move.l	PA_Sp,a0			; Get PA
	move.l	(a0),a0
	move.w	X1(a0),d0		; Load top-left coordinates
	move.w	Y1(a0),d1
; --------- Scroll in ? ---------------------------
	tst.w	d6			; Well ?
	bpl	.Ok_1
	move.w	d6,d7			; Completely out ?
	neg.w	d7
	cmp.w	PA_height,d7
	bpl.s	.Exit
	move.w	d7,d2			; Adjust Y-coordinate
	mulu.w	#Char_height+2,d2
	add.w	d2,d1
	moveq.l	#0,d6			; Print from line 0
	sub.w	PA_height,d7		; Calculate height
	neg.w	d7
	bra.s	.Cont
; --------- Scroll out ? --------------------------
.Ok_1:	cmp.w	Text_height,d6		; Completely out ?
	bpl.s	.Exit
	move.w	Text_height,d7		; Remaining lines
	sub.w	d6,d7
	cmp.w	PA_height,d7		; Well ?
	bmi.s	.Cont
.Ok_2:	move.w	PA_height,d7
; --------- Display text --------------------------
.Cont:	Get	d5,a0			; Get text address
	bra.s	.Entry1			; Skip unwanted lines
.Loop1:	tst.b	(a0)+
	bne.s	.Loop1
.Entry1:	dbra	d6,.Loop1
	moveq.l	#0,d6			; Display text
	bra.s	.Entry2
.Loop2:	move.b	(a0)+,d6			; Set ink
	subq.w	#1,d6
	move.w	d6,Ink_colour
	jsr	Put_text_line		; Print line
	addq.w	#Char_height+2,d1
.Entry2:	dbra	d7,.Loop2
	Free	d5			; Exit
.Exit:	movem.l	(sp)+,d0-d2/d5-d7/a0
	rts
	endc




;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_BSS,bss
One_list:	ds.l 2

Text_scroll_CA:	ds.b Recta_size
Current_scroll_index:	ds.w 1

Text_wait_flag:	ds.b 1
Current_justification:	ds.b 1
Current_line_info:	ds.b Line_info_data_size
	even
Current_text_style:	ds.l 1

Nr_of_lines:	ds.w 1
Text_height:	ds.w 1
Raw_TPB_index:	ds.w 1		; RTPB currently being read
Processed_text_index:	ds.w 1	; Processed characters written to current PTPB
Raw_text_index:	ds.w 1		; Raw characters read in current RTPB
Processing_buffer_index:	ds.w 1	; Processed characters in buffer

Nr_raw_TPBs:	ds.w 1		; Number of raw TPB's
Nr_processed_TPBs:	ds.w 1		; Number of processed TPB's

Processing_buffer:	ds.b Max_line_length

Raw_TPB_handles:	ds.b Max_TPBs	; Memory handles
Processed_TPB_handles:	ds.b Max_TPBs
	even
