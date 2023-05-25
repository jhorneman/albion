; Text routines
; Display
; Written by J.Horneman (In Tune With The Universe)
; Start : 7-2-1994


; Notes :
;   - [ Process_text ] doesn't handle the text justification anymore.
;     Instead, two routines are available :
;         1 - [ Put_text_line ]
;             which prints a line of text as it is.
;         2 - [ Put_justified_text_line ]
;             which justifies the line of text and then prints it.
;     This way no memory is wasted on inserted spaces and no special skip
;     commands are needed, just the usual space-table.
;   - The text routines as they are now can handle any font as long as it
;     isn't wider than 8 pixels. To make sure it handles wider fonts, a new
;     routine is needed which inserts a character in the line buffer.
;   - [ Line_buffer ] should be always be 320 pixels wide, 1 bitplane deep,
;     and as high as the highest font, plus a safety area at the end which
;     is just as wide as the widest font.

	SECTION	Program,code
;***************************************************************************
; [ Put a text line on the screen : centered ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        a0 - Pointer to text line (.l)
; All registers are restored
;***************************************************************************
Put_text_line_centered:
	movem.l	d0/d2/d3,-(sp)
	move.w	d0,d2
	jsr	Get_line_length		; Get line length
	move.w	PA_width,d3		; Centre
	sub.w	d0,d3
	bmi.s	.Skip			; Possible ?
	lsr.w	#1,d3			; Yes
	add.w	d3,d2
.Skip:	move.w	d2,d0			; Print
	jsr	Put_text_line
	movem.l	(sp)+,d0/d2/d3
	rts

;***************************************************************************
; [ Put a text line on the screen : right justified ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        a0 - Pointer to text line (.l)
; All registers are restored
;***************************************************************************
Put_text_line_right:
	movem.l	d0/d2/d3,-(sp)
	move.w	d0,d2
	jsr	Get_line_length		; Get line length
	move.w	PA_width,d3		; Centre
	sub.w	d0,d3
	bmi.s	.Skip			; Possible ?
	add.w	d3,d2			; Yes
.Skip:	move.w	d2,d0			; Print
	jsr	Put_text_line
	movem.l	(sp)+,d0/d2/d3
	rts

;***************************************************************************
; [ Put a text line on the screen : left justified ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        a0 - Pointer to text line (.l)
; All registers are restored
;***************************************************************************
Put_text_line:
	movem.l	d0-d7/a0-a4/a6,-(sp)
	jsr	Get_line_height		; Get height
	move.w	d0,d4			; Set Screen X
	and.w	#$fff0,d0			; Set Print X
	move.w	d0,Print_X
	moveq.l	#-1,d7			; No previous character
	jsr	Clear_print_buffer		; Clear print buffer
	move.w	d4,d5			; Buffer X = Screen X AND $000f
	and.w	#$000f,d5
	moveq.l	#0,d6			; Counter = 0
.Reset:	movea.l	Font_Sp,a6		; Get current font
	move.l	(a6),a6
	move.l	Font_translation(a6),a1	; Get font data
	move.l	Width_table(a6),a2
	move.l	Insert_char(a6),a3
.Again1:	cmp.w	#Screen_width,d5		; Exit if line too long
	bpl	.Exit
	moveq.l	#0,d0			; Read byte from string
	move.b	(a0)+,d0
	beq	.Exit			; Exit if EOL
	cmp.b	#Comstart_char,d0		; Command character ?
	bne.s	.No_com
	move.l	a1,-(sp)			; Yes -> Handle command
	lea.l	LLC_table,a1
	jsr	LLC_handler
	move.l	(sp)+,a1
	bra.s	.Reset
.No_com:	cmp.b	#" ",d0			; Space or Solid space ?
	beq.s	.Space
	cmp.b	#Solid_space,d0
	bne.s	.No_space
.Space:	add.w	Width_of_space(a6),d4	; Yes -> Skip pixels
	add.w	Width_of_space(a6),d5
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
	subq.w	#1,d5
.No_kern:	move.b	d0,d7			; Save
	jsr	(a3)			; Put character in buffer
	moveq.l	#0,d2			; Get character width
	move.b	0(a2,d0.w),d2
	add.w	d2,d4			; Skip pixels
	add.w	d2,d5
	add.w	Between_width(a6),d4	; Skip more pixels
	add.w	Between_width(a6),d5
	addq.w	#1,d6			; Increase counter
	bra	.Again1
.Exit:	jsr	Print_buffer		; Print what's left
	movem.l	(sp)+,d0-d7/a0-a4/a6
	rts

;***************************************************************************
; [ Put a text line on the screen : left AND right justified ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        a0 - Pointer to text line (.l)
; All registers are restored
;***************************************************************************
Put_justified_text_line:
	movem.l	d0-d7/a0-a6,-(sp)
	jsr	Prepare_justification	; Prepare
	tst.w	d7			; Any use ?
	bne	.Yes
	movem.l	(sp)+,d0-d7/a0-a6		; No -> Just do it
	jmp	Put_text_line
.Yes:	jsr	Get_line_height		; Yes -> Get height
	lea.l	Space_table,a5
	move.w	d7,d3			; Save string length
	move.w	d0,d4			; Set Screen X
	and.w	#$fff0,d0			; Set Print X
	move.w	d0,Print_X
	moveq.l	#-1,d7			; No previous character
	jsr	Clear_print_buffer		; Clear print buffer
	move.w	d4,d5			; Buffer X = Screen X AND $000f
	and.w	#$000f,d5
	moveq.l	#0,d6			; Counter = 0
	movea.l	Font_Sp,a6		; Get current font
	move.l	(a6),a6
	move.l	Font_translation(a6),a1	; Get font data
	move.l	Width_table(a6),a2
	move.l	Insert_char(a6),a3
.Loop1:	cmp.w	#Screen_width,d5		; Exit if line too long
	bpl	.Exit
	moveq.l	#0,d0			; Read byte from string
	move.b	(a0)+,d0
	beq	.Exit			; Exit if EOL
	cmp.b	#Comstart_char,d0		; Command character ?
	bne.s	.No_com
	move.l	a1,-(sp)			; Yes -> Handle command
	lea.l	LLC_table,a1
	jsr	LLC_handler
	move.l	(sp)+,a1
	movea.l	Font_Sp,a6		; Font may have changed
	move.l	(a6),a6
	move.l	Font_translation(a6),a1	; Get font data
	move.l	Width_table(a6),a2
	move.l	Insert_char(a6),a3
	bra.s	.Next1
.No_com:	cmp.b	#" ",d0			; Space or Solid space ?
	beq.s	.Space
	cmp.b	#Solid_space,d0
	bne.s	.No_space
.Space:	moveq.l	#0,d2			; Yes -> Get space width
	move.b	(a5)+,d2
	add.w	d2,d4			; Yes -> Skip pixels
	add.w	d2,d5
	moveq.l	#-1,d7			; No previous character
	bra.s	.Next1
.No_space:	cmp.w	#32,d0			; Legal ?
	blo.s	.Next1
	move.b	-32(a1,d0.w),d0		; Yes -> Translate
	bmi.s	.Next1
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
	subq.w	#1,d5
.No_kern:	move.b	d0,d7			; Save
	jsr	(a3)			; Put character in buffer
	moveq.l	#0,d2			; Get character width
	move.b	0(a2,d0.w),d2
	add.w	d2,d4			; Skip pixels
	add.w	d2,d5
	add.w	Between_width(a6),d4	; Skip more pixels
	add.w	Between_width(a6),d5
	addq.w	#1,d6			; Increase counter
.Next1:	dbra	d3,.Loop1
.Exit:	jsr	Print_buffer		; Print what's left
	movem.l	(sp)+,d0-d7/a0-a6
	rts

;***************************************************************************
; [ Display & reset print buffer ]
;   IN : d1 - Screen Y-coordinate (.w)
;        d4 - Screen X-coordinate (.w)
;        d5 - Buffer X-coordinate (.w)
;        d6 - Number of characters in buffer (.w)
;  OUT : d5 - New buffer X-coordinate (.w)
;        d6 - 0 (.w)
; Changed registers : d5,d6
;***************************************************************************
Print_buffer:
	tst.w	d6			; Any characters in buffer ?
	beq.s	.Exit
	movem.l	d0/d4/d7/a0,-(sp)		; Yes -> Print buffer
	move.w	Print_X,d0		; Get X-coordinate
	move.w	d5,d6			;     width in truncs
	add.w	#15,d6
	and.w	#$fff0,d6
	lsr.w	#4,d6
	move.w	Current_line_height,d7	;     height in pixels
	jsr	Put_line_buffer
	jsr	Clear_print_buffer		; Clear print buffer
	move.w	d4,d0			; Print X = Screen X
	and.w	#$fff0,d0
	move.w	d0,Print_X
	move.w	d4,d5			; Buffer X = Screen X AND $000f
	and.w	#$000f,d5
	moveq.l	#0,d6			; Counter = 0
	movem.l	(sp)+,d0/d4/d7/a0
.Exit:	rts

;***************************************************************************
; [ Clear print buffer ]
; All registers are restored
;***************************************************************************
Clear_print_buffer:
	movem.l	d0/d1/a0,-(sp)
	move.l	#Line_buffer_size,d0	; Clear print buffer
	moveq.l	#0,d1
	lea.l	Line_buffer,a0
	jsr	Fill_memory
	movem.l	(sp)+,d0/d1/a0
	rts

;***************************************************************************
; [ Insert 8 pixel wide character in line buffer ]
;   IN : d0 - Character number (.w)
;        d5 - X-position in buffer (.w)
;        a6 - Pointer to Font data structure (.l)
; All registers are restored
;***************************************************************************
Insert_8w:
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
	move.w	Current_base_line,d2
	sub.w	Base_line(a6),d2
	mulu.w	#Bytes_per_plane,d2
	add.w	d2,a1
	move.w	Raw_char_height(a6),d7
	btst	#0,d0			; Odd or even ?
	beq.s	.Even
	bra.s	.Entry1
.Loop1:	moveq.l	#0,d0			; Load character line
	move.b	(a0),d0
	ror.w	d1,d0			; Shift it
	or.b	d0,(a1)+			; Write it
	lsr.w	#8,d0	
	or.b	d0,(a1)
	addq.l	#2,a0			; Next line
	lea.l	Bytes_per_plane-1(a1),a1
.Entry1:	dbra	d7,.Loop1
	bra.s	.Done
.Even: 	subq.w	#8,d1			; Reverse scroll value
	neg.w	d1
	bra.s	.Entry2
.Loop2:	moveq.l	#0,d0			; Load character line
	move.b	(a0),d0
	rol.w	d1,d0			; Shift it
	or.w	d0,(a1)			; Write it
	addq.l	#2,a0			; Next line
	lea.l	Bytes_per_plane(a1),a1
.Entry2:	dbra	d7,.Loop2
.Done:	movem.l	(sp)+,d0-d2/d7/a0/a1
	rts

;***************************************************************************
; [ Insert fat 8 pixel wide character in line buffer ]
;   IN : d0 - Character number (.w)
;        d5 - X-position in buffer (.w)
;        a6 - Pointer to Font data structure (.l)
; All registers are restored
;***************************************************************************
Insert_fat_8w:
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
	move.w	Current_base_line,d2
	sub.w	Base_line(a6),d2
	mulu.w	#Bytes_per_plane,d2
	add.w	d2,a1
	move.w	Raw_char_height(a6),d7
	btst	#0,d0			; Odd or even ?
	beq.s	.Even
	bra.s	.Entry1
.Loop1:	moveq.l	#0,d0			; Load character line
	move.b	(a0),d0
	ror.w	d1,d0			; Shift it
	move.w	d0,d2			; Make it fat
	ror.w	#1,d2
	or.w	d2,d0
	or.b	d0,(a1)+			; Write it
	lsr.w	#8,d0	
	or.b	d0,(a1)
	addq.l	#2,a0			; Next line
	lea.l	Bytes_per_plane-1(a1),a1
.Entry1:	dbra	d7,.Loop1
	bra.s	.Done
.Even: 	subq.w	#8,d1			; Reverse scroll value
	neg.w	d1
	bra.s	.Entry2
.Loop2:	moveq.l	#0,d0			; Load character line
	move.b	(a0),d0
	rol.w	d1,d0			; Shift it
	move.w	d0,d2			; Make it fat
	ror.w	#1,d2
	or.w	d2,d0
	or.w	d0,(a1)			; Write it
	addq.l	#2,a0			; Next line
	lea.l	Bytes_per_plane(a1),a1
.Entry2:	dbra	d7,.Loop2
.Done:	movem.l	(sp)+,d0-d2/d7/a0/a1
	rts

;***************************************************************************
; [ Insert high 8 pixel wide character in line buffer ]
;   IN : d0 - Character number (.w)
;        d5 - X-position in buffer (.w)
;        a6 - Pointer to Font data structure (.l)
; All registers are restored
;***************************************************************************
Insert_high_8w:
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
	move.w	Current_base_line,d2
	sub.w	Base_line(a6),d2
	mulu.w	#Bytes_per_plane,d2
	add.w	d2,a1
	move.w	Raw_char_height(a6),d7
	lsr.w	#1,d7
	btst	#0,d0			; Odd or even ?
	beq.s	.Even
	bra.s	.Entry1
.Loop1:	moveq.l	#0,d0			; Load character line
	move.b	(a0),d0
	ror.w	d1,d0			; Shift it
	or.b	d0,Bytes_per_plane(a1)		; Write it
	or.b	d0,(a1)+
	lsr.w	#8,d0	
	or.b	d0,(a1)
	or.b	d0,Bytes_per_plane(a1)
	addq.l	#2,a0			; Next line
	lea.l	2*Bytes_per_plane-1(a1),a1
.Entry1:	dbra	d7,.Loop1
	bra.s	.Done
.Even: 	subq.w	#8,d1			; Reverse scroll value
	neg.w	d1
	bra.s	.Entry2
.Loop2:	moveq.l	#0,d0			; Load character line
	move.b	(a0),d0
	rol.w	d1,d0			; Shift it
	or.w	d0,(a1)			; Write it
	or.w	d0,Bytes_per_plane(a1)
	addq.l	#2,a0			; Next line
	lea.l	2*Bytes_per_plane(a1),a1
.Entry2:	dbra	d7,.Loop2
.Done:	movem.l	(sp)+,d0-d2/d7/a0/a1
	rts

;***************************************************************************
; [ Insert fat high 8 pixel wide character in line buffer ]
;   IN : d0 - Character number (.w)
;        d5 - X-position in buffer (.w)
;        a6 - Pointer to Font data structure (.l)
; All registers are restored
;***************************************************************************
Insert_fat_high_8w:
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
	move.w	Current_base_line,d2
	sub.w	Base_line(a6),d2
	mulu.w	#Bytes_per_plane,d2
	add.w	d2,a1
	move.w	Raw_char_height(a6),d7
	lsr.w	#1,d7
	btst	#0,d0			; Odd or even ?
	beq.s	.Even
	bra.s	.Entry1
.Loop1:	moveq.l	#0,d0			; Load character line
	move.b	(a0),d0
	ror.w	d1,d0			; Shift it
	move.w	d0,d2			; Make it fat
	ror.w	#1,d2
	or.w	d2,d0
	or.b	d0,Bytes_per_plane(a1)	; Write it
	or.b	d0,(a1)+
	lsr.w	#8,d0	
	or.b	d0,(a1)
	or.b	d0,Bytes_per_plane(a1)
	addq.l	#2,a0			; Next line
	lea.l	2*Bytes_per_plane-1(a1),a1
.Entry1:	dbra	d7,.Loop1
	bra.s	.Done
.Even: 	subq.w	#8,d1			; Reverse scroll value
	neg.w	d1
	bra.s	.Entry2
.Loop2:	moveq.l	#0,d0			; Load character line
	move.b	(a0),d0
	rol.w	d1,d0			; Shift it
	move.w	d0,d2			; Make it fat
	ror.w	#1,d2
	or.w	d2,d0
	or.w	d0,(a1)			; Write it
	or.w	d0,Bytes_per_plane(a1)
	addq.l	#2,a0			; Next line
	lea.l	2*Bytes_per_plane(a1),a1
.Entry2:	dbra	d7,.Loop2
.Done:	movem.l	(sp)+,d0-d2/d7/a0/a1
	rts

;***************************************************************************
; [ Prepare a text line for justification ]
;   IN : a0 - Pointer to text line (.l)
;  OUT : d7 - Length of string / 0 = Justification impossible (.w)
;        a0 - Pointer to string after leading spaces (.l)
; Changed registers : d0,a0
;***************************************************************************
Prepare_justification:
	movem.l	d0-d2/d5/d6/a1,-(sp)
	moveq.l	#0,d7
.Again1:	move.b	(a0),d0			; Skip leading spaces
	beq	.Exit
	cmp.b	#" ",d0
	bne.s	.Done1
	addq.l	#1,a0
	beq.s	.Again1
.Done1:	jsr	Strlen			; Get string length
.Again2:	cmp.b	#" ",-1(a0,d0.w)		; Ignore trailing spaces
	bne.s	.Done2
	subq.w	#1,d0
	bne.s	.Again2
.Done2:	move.w	d0,d7			; Save string length

; Registers :
;  d7 - String length (.w)
;  a0 - Pointer to string (.l)

	move.l	a0,a1			; Count spaces
	moveq.l	#0,d1
	moveq.l	#0,d2
	move.w	d7,d5
	bra.s	.Entry1
.Loop1:	move.b	(a1)+,d0			; Read character
	cmp.b	#Comstart_char,d0		; Command character ?
	bne.s	.No_com1
	bra.s	.Next2
.Loop2:	cmpi.b	#Comend_char,(a1)+		; End of command ?
	beq.s	.Entry1
.Next2:	dbra	d5,.Loop2
	bra.s	.End1
.No_com1:	cmp.b	#" ",d0			; Is it a space ?
	beq.s	.Space
	addq.w	#1,d2			; No
	bra.s	.Entry1
.Space:	addq.w	#1,d1			; Yes
.Entry1:	dbra	d5,.Loop1
.End1:	tst.w	d1			; Any spaces ?
	beq.s	.Leave
	tst.w	d2
	bne.s	.Ok1
.Leave:	moveq.l	#0,d7			; No -> Exit
	bra	.Exit
.Ok1:	lea.l	0(a0,d7.w),a1		; Get length of line
	move.b	(a1),d2
	clr.b	(a1)
	jsr	Get_line_length_without_spaces
	move.b	d2,(a1)
	move.w	PA_width,d6		; Calculate free pixels
	subq.w	#1,d6
	sub.w	d0,d6
	bpl.s	.Ok2			; Any ?
	moveq.l	#0,d7			; No -> Exit
	bra	.Exit
.Ok2:	moveq.l	#0,d2			; Divide
	move.w	d6,d2
	divu.w	d1,d2
	cmp.w	#2,d2			; Too small ?
	bpl.s	.Ok3
	moveq.l	#0,d7			; Yes -> Exit
	bra	.Exit
.Ok3:

; Registers :
;  d1 - Number of spaces in string (.w)
;  d2 - Divided pixels (.l)
;  d6 - Number of pixels to be divided (.w)
;  d7 - String length (.w)
;  a0 - Pointer to string (.l)

	lea.l	Space_table,a1		; Divide main pixels
	move.w	d1,d5
	subq.w	#1,d5
.Loop3:	move.b	d2,(a1)+
	dbra	d5,.Loop3
	lea.l	Space_table,a1		; Divide remaining pixels
	swap	d2
	bra.s	.Entry4
.Loop4:	jsr	Random
	mulu.w	d1,d0
	swap	d0
	addq.b	#1,0(a1,d0.w)
.Entry4:	dbra	d2,.Loop4
.Exit:	movem.l	(sp)+,d0-d2/d5/d6/a1
	rts

;***************************************************************************
; [ Get the length in pixels of a text line WITHOUT spaces ]
;   IN : a0 - Pointer to text line (.l)
;  OUT : d0 - Length of text line in pixels (.w)
; Changed registers : d0
;***************************************************************************
Get_line_length_without_spaces:
	movem.l	d2/d4/d7/a0-a2/a4/a6,-(sp)
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
	cmp.b	#Comstart_char,d0		; Command character ?
	bne.s	.No_com
	move.l	a1,-(sp)			; Yes -> Handle command
	lea.l	LLC_table2,a1
	jsr	LLC_handler
	move.l	(sp)+,a1
	movea.l	Font_Sp,a6		; Get current font
	move.l	(a6),a6
	move.l	Font_translation(a6),a1	; Get font data
	move.l	Width_table(a6),a2
	bra.s	.Again1
.No_com:	cmp.b	#" ",d0			; Space or Solid space ?
	beq.s	.Space
	cmp.b	#Solid_space,d0
	bne.s	.No_space
.Space:	moveq.l	#-1,d7			; Yes -> No previous character
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
	add.w	Between_width(a6),d4	; Skip more pixels
	bra	.Again1
.Exit:	sub.w	Between_width(a6),d4	; Remove last pixels
	move.l	(sp)+,a0			; Restore font
	jsr	Change_Font
	move.w	d4,d0			; Output
	movem.l	(sp)+,d2/d4/d7/a0-a2/a4/a6
	rts

;***************************************************************************
; [ Get the length in pixels of a text line ]
;   IN : a0 - Pointer to text line (.l)
;  OUT : d0 - Length of text line in pixels (.w)
; Changed registers : d0
;***************************************************************************
Get_line_length:
	movem.l	d2/d4/d7/a0-a2/a4/a6,-(sp)
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
	cmp.b	#Comstart_char,d0		; Command character ?
	bne.s	.No_com
	move.l	a1,-(sp)			; Yes -> Handle command
	lea.l	LLC_table2,a1
	jsr	LLC_handler
	move.l	(sp)+,a1
	movea.l	Font_Sp,a6		; Get current font
	move.l	(a6),a6
	move.l	Font_translation(a6),a1	; Get font data
	move.l	Width_table(a6),a2
	bra.s	.Again1
.No_com:	cmp.b	#" ",d0			; Space or Solid space ?
	beq.s	.Space
	cmp.b	#Solid_space,d0
	bne.s	.No_space
.Space:	add.w	Width_of_space(a6),d4	; Yes -> Skip pixels
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
	add.w	Between_width(a6),d4	; Skip more pixels
	bra	.Again1
.Exit:	sub.w	Between_width(a6),d4	; Remove last pixels
	move.l	(sp)+,a0			; Restore font
	jsr	Change_Font
	move.w	d4,d0			; Output
	movem.l	(sp)+,d2/d4/d7/a0-a2/a4/a6
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Chip_BSS,bss_c
Line_buffer:	ds.b Line_buffer_size
	even


	SECTION	Fast_BSS,bss
Print_X:	ds.w 1
Ink_colour:	ds.w 1
Shadow_colour:	ds.w 1
Space_table:	ds.b Max_line_length	; Should always be enough
	even
