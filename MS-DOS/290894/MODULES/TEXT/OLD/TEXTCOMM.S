; Text routines
; Text commands
; Written by J.Horneman (In Tune With The Universe)
; Start : 7-2-1994

	SECTION	Program,code
;*****************************************************************************
; [ Low level text command handler ]
;   IN : d4 - Screen X-coordinate (.w)
;       (d5 - X-position in buffer (.w))
;        a0 - Pointer to string (after command character) (.l)
;        a1 - Pointer to LLC table (.l)
;  OUT : a0 - Pointer to string after command (.l)
; Changed registers : d4,d5,a0
;*****************************************************************************
LLC_handler:
	movem.l	d0/a1,-(sp)
	move.b	(a0)+,d0			; Get command
	lsl.w	#8,d0
	move.b	(a0)+,d0
	swap	d0
	move.b	(a0)+,d0
	lsl.w	#8,d0
	move.b	(a0)+,d0
.Again:	tst.l	(a1)			; End of command list ?
	beq.s	.Seek			; Yes -> Skip command
	cmp.l	(a1)+,d0			; Found ?
	beq.s	.Found
	addq.l	#4,a1			; No -> Next
	bra.s	.Again
.Found:	move.l	(a1),a1			; Yes -> Execute
	jsr	(a1)
.Seek:	cmp.b	#Comend_char,(a0)+		; End of command ?
	bne.s	.Seek
	movem.l	(sp)+,d0/a1
	rts

;*****************************************************************************
; [ Low level text commands ]
;   IN : d4 - Screen X-coordinate (.w)
;       (d5 - X-position in buffer (.w))
;        a0 - Pointer to string (after command character) (.l)
;  OUT : a0 - Pointer to string after command (.l)
; Changed registers : d4,d5,a0
; NOTE :
;  - a0 may be changed but must be kept VALID in any case.
;*****************************************************************************
LLC_Set_ink:
	move.l	d0,-(sp)
	jsr	Print_buffer
	jsr	Get_LLC_nr3
	and.w	#Pal_size-1,d0
	move.w	d0,Ink_colour
	move.l	(sp)+,d0
	rts

LLC_Set_shadow:
	move.l	d0,-(sp)
	jsr	Print_buffer
	jsr	Get_LLC_nr3
	and.w	#Pal_size-1,d0
	move.w	d0,Shadow_colour
	move.l	(sp)+,d0
	rts

LLC_No_shadow:
	jsr	Print_buffer
	move.w	#-1,Shadow_colour
	rts

LLC_Set_normal_style:
	move.l	#Normal_style,Current_text_style
	rts

LLC_Set_fat_style:
	move.l	#Fat_style,Current_text_style
	rts

LLC_Set_high_style:
	move.l	#High_style,Current_text_style
	rts

LLC_Set_fat_high_style:
	move.l	#Fat_high_style,Current_text_style
	rts

LLC_Set_normal_font:
	move.l	a0,-(sp)
	lea.l	Default_Font,a0
	jsr	Change_Font
	move.l	(sp)+,a0
	rts

LLC_Set_techno_font:
	move.l	a0,-(sp)
	lea.l	Techno_Font,a0
	jsr	Change_Font
	move.l	(sp)+,a0
	rts

LLC_Set_capital:
	movem.l	d0/a0,-(sp)
	jsr	Print_buffer
	move.l	#Fat_high_style,Current_text_style
	move.w	#Capital_colour,Ink_colour
	move.l	PA_Sp,a0			; Get left border
	move.l	(a0),a0
	move.w	X1(a0),d4
	add.w	#10,d4			; Jump
	move.w	d4,d0			; Print X = Screen X
	and.w	#$fff0,d0
	move.w	d0,Print_X
	move.w	d4,d5			; Buffer X = Screen X AND $000f
	and.w	#$000f,d5
	movem.l	(sp)+,d0/a0
	rts

LLC_Set_capital2:
	move.l	#Fat_high_style,Current_text_style
	moveq.l	#10,d4			; Jump
	rts

LLC_Unset_capital:
	move.l	a0,-(sp)
	jsr	Print_buffer
	move.l	#Normal_style,Current_text_style
	move.l	PA_Sp,a0
	move.l	(a0),a0
	move.w	PA_Ink(a0),Ink_colour
	move.l	(sp)+,a0
	rts

LLC_Unset_capital2:
	move.l	#Normal_style,Current_text_style
	rts

LLC_HJump:
	movem.l	d0/a1,-(sp)
	jsr	Get_LLC_nr3		; Get jump
	cmp.w	PA_width,d0		; Too far ?
	bpl.s	.Exit
	jsr	Print_buffer
	move.l	PA_Sp,a1			; Get left border
	move.l	(a1),a1
	move.w	X1(a1),d4
	add.w	d0,d4			; Jump
	move.w	d4,d0			; Print X = Screen X
	and.w	#$fff0,d0
	move.w	d0,Print_X
	move.w	d4,d5			; Buffer X = Screen X AND $000f
	and.w	#$000f,d5
.Exit:	movem.l	(sp)+,d0/a1
	rts

LLC_HJump2:
	movem.l	d0/a1,-(sp)
	jsr	Get_LLC_nr3		; Get jump
	cmp.w	PA_width,d0		; Too far ?
	bpl.s	.Exit
	move.w	d0,d4			; Jump
.Exit:	movem.l	(sp)+,d0/a1
	rts

LLC_Default:
	move.l	a0,-(sp)
	jsr	Print_buffer
	lea.l	Default_Font,a0		; Default font
	jsr	Change_Font
	move.l	#Normal_style,Current_text_style
	move.l	PA_Sp,a0			; Get current PA
	move.l	(a0),a0
	move.w	PA_Ink(a0),Ink_colour	; Default colours
	move.w	PA_Shadow(a0),Shadow_colour
	move.l	(sp)+,a0
	rts

LLC_Default2:
	move.l	a0,-(sp)
	lea.l	Default_Font,a0		; Default font
	jsr	Change_Font
	move.l	#Normal_style,Current_text_style
	move.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Low level text analysis command handler ]
;   IN : d4 - Screen X-coordinate (.w)
;       (d5 - X-position in buffer (.w))
;        a1 - Pointer to string (after command character) (.l)
;  OUT : a1 - Pointer to string after command (.l)
; Changed registers : d4,d5,a1
;*****************************************************************************
LLCA_handler:
	movem.l	d0/a0,-(sp)
	lea.l	LLCA_table,a0
	exg.l	a0,a1
	move.b	(a0)+,d0			; Get command
	lsl.w	#8,d0
	move.b	(a0)+,d0
	swap	d0
	move.b	(a0)+,d0
	lsl.w	#8,d0
	move.b	(a0)+,d0
.Again:	tst.l	(a1)			; End of command list ?
	beq.s	.Seek			; Yes -> Skip command
	cmp.l	(a1)+,d0			; Found ?
	beq.s	.Found
	addq.l	#4,a1			; No -> Next
	bra.s	.Again
.Found:	move.l	(a1),a1			; Yes -> Execute
	jsr	(a1)
.Seek:	cmp.b	#Comend_char,(a0)+		; End of command ?
	bne.s	.Seek
	move.l	a0,a1
	movem.l	(sp)+,d0/a0
	rts

;*****************************************************************************
; [ Low level text analysis commands ]
;   IN : d4 - Screen X-coordinate (.w)
;       (d5 - X-position in buffer (.w))
;        a0 - Pointer to string (after command character) (.l)
;  OUT : a0 - Pointer to string after command (.l)
; Changed registers : d4,d5,a0
; NOTE :
;  - a0 may be changed but must be kept VALID in any case.
;*****************************************************************************
LLCA_Set_ink:
	move.l	d0,-(sp)
	jsr	Get_LLC_nr3
	and.w	#Pal_size-1,d0
	move.w	d0,Ink_colour
	move.l	(sp)+,d0
	rts

LLCA_Set_shadow:
	move.l	d0,-(sp)
	jsr	Get_LLC_nr3
	and.w	#Pal_size-1,d0
	move.w	d0,Shadow_colour
	move.l	(sp)+,d0
	rts

LLCA_No_shadow:
	move.w	#-1,Shadow_colour
	rts

LLCA_Set_capital:
	move.l	#Fat_high_style,Current_text_style
	move.w	#Capital_colour,Ink_colour
	moveq.l	#10,d4
	moveq.l	#10,d5
	rts

LLCA_Unset_capital:
	move.l	a0,-(sp)
	move.l	#Normal_style,Current_text_style
	move.l	PA_Sp,a0
	move.l	(a0),a0
	move.w	PA_Ink(a0),Ink_colour
	move.l	(sp)+,a0
	rts

LLCA_HJump:
	move.l	d0,-(sp)
	jsr	Get_LLC_nr3		; Get jump
	cmp.w	PA_width,d0		; Too far ?
	bpl.s	.Exit
	move.w	d0,d4			; Jump
	move.w	d0,d5
.Exit:	move.l	(sp)+,d0
	rts

LLCA_VJump:
	move.l	d0,-(sp)
	jsr	Get_LLC_nr3		; Get jump
	cmp.b	Line_skip(a5),d0		; Higher than current ?
	bmi.s	.Exit
	move.b	d0,Line_skip(a5)		; Yes
.Exit:	move.l	(sp)+,d0
	rts

LLCA_Centre:
	move.b	#Print_centered,Current_justification
	rts

LLCA_Left:
	move.b	#Print_left,Current_justification
	rts

LLCA_Right:
	move.b	#Print_right,Current_justification
	rts

LLCA_Justified:
	move.b	#Print_justified,Current_justification
	rts

LLCA_Default:
	move.l	a0,-(sp)
	lea.l	Default_Font,a0		; Default font
	jsr	Change_Font
	move.l	#Normal_style,Current_text_style
	move.l	PA_Sp,a0			; Get current PA
	move.l	(a0),a0
	move.w	PA_Ink(a0),Ink_colour	; Default colours
	move.w	PA_Shadow(a0),Shadow_colour
	move.b	#Print_justified,Current_justification
	move.l	(sp)+,a0
	rts

;***************************************************************************
; [ Get 3-digit number from LLC command ]
;   IN : a0 - Pointer to string after command (.l)
;  OUT : d0 - Number (.w)
;        a0 - Pointer to string after number (.l)
; Changed registers : d0,a0
;***************************************************************************
Get_LLC_nr3:
	move.l	d1,-(sp)
	moveq.l	#0,d0			; Convert three bytes
	move.b	(a0)+,d0			;  to number
	sub.b	#"0",d0
	mulu.w	#100,d0
	moveq.l	#0,d1
	move.b	(a0)+,d1
	sub.b	#"0",d1
	mulu.w	#10,d1
	add.w	d1,d0
	moveq.l	#0,d1
	move.b	(a0)+,d1
	sub.b	#"0",d1
	add.w	d1,d0
	move.l	(sp)+,d1
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
LLC_table:
	dc.l "INK ",LLC_Set_ink		; For actual printing
	dc.l "SHAD",LLC_Set_shadow
	dc.l "NSHA",LLC_No_shadow
	dc.l "NORS",LLC_Set_normal_style
	dc.l "FAT ",LLC_Set_fat_style
	dc.l "HIGH",LLC_Set_high_style
	dc.l "FAHI",LLC_Set_fat_high_style
	dc.l "NORF",LLC_Set_normal_font
	dc.l "TECF",LLC_Set_techno_font
	dc.l "CAPI",LLC_Set_capital
	dc.l "UNCA",LLC_Unset_capital
	dc.l "DFAU",LLC_Default
	dc.l "HJMP",LLC_HJump
	dc.l 0
LLC_table2:
	dc.l "NORS",LLC_Set_normal_style	; For word-wrapping
	dc.l "FAT ",LLC_Set_fat_style
	dc.l "HIGH",LLC_Set_high_style
	dc.l "FAHI",LLC_Set_fat_high_style
	dc.l "NORF",LLC_Set_normal_font
	dc.l "TECF",LLC_Set_techno_font
	dc.l "CAPI",LLC_Set_capital2
	dc.l "UNCA",LLC_Unset_capital2
	dc.l "DFAU",LLC_Default2
	dc.l "HJMP",LLC_HJump2
	dc.l 0
LLCA_table:
	dc.l "INK ",LLCA_Set_ink		; For line analysis
	dc.l "SHAD",LLCA_Set_shadow
	dc.l "NSHA",LLCA_No_shadow
	dc.l "NORS",LLC_Set_normal_style
	dc.l "FAT ",LLC_Set_fat_style
	dc.l "HIGH",LLC_Set_high_style
	dc.l "FAHI",LLC_Set_fat_high_style
	dc.l "NORF",LLC_Set_normal_font
	dc.l "TECF",LLC_Set_techno_font
	dc.l "CAPI",LLCA_Set_capital
	dc.l "UNCA",LLCA_Unset_capital
	dc.l "DFAU",LLCA_Default
	dc.l "HJMP",LLCA_HJump
	dc.l "VJMP",LLCA_VJump
	dc.l "CNTR",LLCA_Centre
	dc.l "LEFT",LLCA_Left
	dc.l "RIGH",LLCA_Right
	dc.l "JUST",LLCA_Justified		
	dc.l 0
