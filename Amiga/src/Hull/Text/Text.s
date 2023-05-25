; Text routines
; Main
; Written by J.Horneman (In Tune With The Universe)
; Start : 7-2-1994

	XDEF	Push_PA
	XDEF	Pop_PA
	XDEF	Erase_PA
	XDEF	Reset_PA_stack
	XDEF	Push_Font
	XDEF	Pop_Font
	XDEF	Reset_Font_stack
	XDEF	Change_Font
	XDEF	Strcpy
	XDEF	Strncpy
	XDEF	Strlen
	XDEF	Put_text_line
	XDEF	Put_centered_text_line
	XDEF	Put_centered_box_text_line
	XDEF	Process_text
	XDEF	Get_line_length
	XDEF	Get_line_size
	XDEF	Ink_colour
	XDEF	Shadow_colour
	XDEF	Line_buffer
	XDEF	Default_PA

	SECTION	Program,code
;*****************************************************************************
; [ Push a PA on the stack ]
;   IN : a0 - Pointer to PA (.l)
; All registers are restored
;*****************************************************************************
Push_PA:     
	move.l	a1,-(sp)
	movea.l	PA_Sp,a1
	addq.l	#4,a1
	cmpa.l	#PAStack_end,a1		; Possible ?
	beq.s	.Exit
	move.l	a0,(a1)			; Push
	move.l	a1,PA_Sp
	jsr	Init_PA			; Initialize new PA
.Exit:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Pop a PA from the stack ]
; All registers are restored
;*****************************************************************************
Pop_PA:      
	movem.l	d0/a0/a1,-(sp)
	movea.l	PA_Sp,a0
	cmpa.l	#PAStack_start,a0		; Possible ?
	beq.s	.Exit
	move.l	(a0),a1			; Yes
	cmp.w	#-1,PA_Paper(a1)		; Transparent paper ?
	bne.s	.No
	move.b	PA_bg_handle(a1),d0		; Yes -> Free memory
	jsr	Free_memory
	clr.b	PA_bg_handle(a1)
.No:	subq.l	#4,a0			; Pop
	move.l	a0,PA_Sp
	move.l	(a0),a0			; Initialize old PA
	jsr	Init_PA
.Exit:	movem.l	(sp)+,d0/a0/a1
	rts

;*****************************************************************************
; [ Reset the PA stack ]
; All registers are restored
;*****************************************************************************
Reset_PA_stack:    
	movem.l	d0/a0/a1,-(sp)
	lea.l	PAStack_start,a0
	move.l	PA_Sp,a1
.Again:	cmp.l	a0,a1			; Back at the start ?
	bmi.s	.End
	cmp.w	#-1,PA_Paper(a1)		; No -> Transparent paper ?
	bne.s	.Next
	move.b	PA_bg_handle(a1),d0		; Yes -> Free memory
	jsr	Free_memory
	clr.b	PA_bg_handle(a1)
.Next:	subq.l	#4,a1			; Next PA
	bra.s	.Again
.End:	lea.l	Default_PA,a0		; Set default PA
	lea.l	PAStack_start,a1
	move.l	a1,PA_Sp
	move.l	a0,(a1)
	jsr	Init_PA			; Initialize default PA
	movem.l	(sp)+,d0/a0/a1
	rts

;*****************************************************************************
; [ Initialize a PA ]
;   IN : a0 - Pointer to PA (.l)
; All registers are restored
;*****************************************************************************
Init_PA:
	move.l	d0,-(sp)
	move.w	X2(a0),d0			; Calculate PA dimensions
	sub.w	X1(a0),d0
	addq.w	#1,d0
	move.w	d0,PA_width
	move.w	Y2(a0),d0
	sub.w	Y1(a0),d0
	addq.w	#1,d0
	move.w	d0,PA_height
	move.w	PA_Ink(a0),Ink_colour	; Set colours
	move.w	PA_Shadow(a0),Shadow_colour
.Exit:	move.l	(sp)+,d0
	rts

;*****************************************************************************
; [ Push a Font on the stack ]
;   IN : a0 - Pointer to Font (.l)
; All registers are	restored
;*****************************************************************************
Push_Font:
	move.l	a1,-(sp)
	movea.l	Font_Sp,a1
	addq.l	#4,a1
	cmpa.l	#FontStack_end,a1		; Possible ?
	beq.s	.Exit
	move.l	a0,(a1)			; Push
	jsr	Init_Font
	move.l	a1,Font_Sp
.Exit:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Pop a Font from the stack ]
; All registers are	restored
;*****************************************************************************
Pop_Font:
	move.l	a0,-(sp)
	movea.l	Font_Sp,a0
	cmpa.l	#FontStack_start,a0		; Possible ?
	beq.s	.Exit
	subq.l	#4,a0			; Pop
	move.l	a0,Font_Sp
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Reset the Font stack ]
; All registers are	restored
;*****************************************************************************
Reset_Font_stack:
	movem.l	a0/a1,-(sp)
	lea.l	Default_Font,a0		; Reset stack
	lea.l	FontStack_start,a1
	move.l	a0,(a1)
	jsr	Init_Font
	move.l	a1,Font_Sp
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Change the current Font ]
;   IN : a0 - Pointer to Font (.l)
; All registers are	restored
;*****************************************************************************
Change_Font:
	move.l	a1,-(sp)
	movea.l	Font_Sp,a1
	move.l	a0,(a1)
	jsr	Init_Font
	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Initialize a Font ]
;   IN : a0 - Pointer to Font (.l)
; All registers are	restored
;*****************************************************************************
Init_Font:
	movem.l	d0/d1/d7/a0-a3,-(sp)
	moveq.l	#Max_styles-1,d7
.Loop:	move.l	(a0)+,a1			; Load font style
	move.l	Kerning_table(a1),d0	; Kerning table available ?
	beq.s	.Next
	move.l	d0,a2			; Yes
	tst.w	(a2)			; Already converted ?
	bne.s	.Next
	move.w	#-1,(a2)+			; No -> Convert
	moveq.l	#0,d0
	move.l	Font_translation(a1),a3
.Again:	tst.w	(a2)			; End of table ?
	beq.s	.Next
	move.b	(a2),d0			; No -> Convert kerning pair
	move.b	-32(a3,d0.w),d0
	move.b	d0,(a2)+
	move.b	(a2),d0
	move.b	-32(a3,d0.w),d0
	move.b	d0,(a2)+
	bra.s	.Again
.Next:	dbra	d7,.Loop
.Skip:	movem.l	(sp)+,d0/d1/d7/a0-a3
	rts

;*****************************************************************************
; [ Strcpy function ]
;   IN : a0 - Pointer to destination string (.l)
;        a1 - Pointer to source string (.l)
; Changed registers : a0,a1
; Note :
;   - This routine copies the EOL as well.
;*****************************************************************************
Strcpy:
	move.l	d0,-(sp)
.Again:	move.b	(a1)+,d0			; Read character
	move.b	d0,(a0)+			; Copy character
	bne.s	.Again			; End of line ?
	move.l	(sp)+,d0
	rts

;*****************************************************************************
; [ Strncpy function ]
;   IN : d0 - Number of characters to copy (.w)
;        a0 - Pointer to destination string (.l)
;        a1 - Pointer to source string (.l)
; All registers are restored
;*****************************************************************************
Strncpy:
	movem.l	d0/a0/a1,-(sp)
	bra.s	.Entry
.Loop:	move.b	(a1)+,(a0)+		; Copy character
.Entry:	dbra	d0,.Loop
	movem.l	(sp)+,d0/a0/a1
	rts

;*****************************************************************************
; [ Strlen function ]
;   IN : a0 - Pointer to string (.l)
;  OUT : d0 - String length (.l)
; Changed registers : d0
;*****************************************************************************
Strlen:
	move.l	a0,-(sp)
	moveq.l	#-1,d0
.Again:	addq.l	#1,d0			; Count up
	tst.b	(a0)+			; End of string ?
	bne.s	.Again			; No, repeat
	move.l	(sp)+,a0
	rts

;***************************************************************************	
; The other sources
;***************************************************************************	

	incdir	DDT:
	include	Hull/Text/Text_display.s
	include	Hull/Text/Text_commands.s
	include	Hull/Text/Text_processing.s

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************	
	SECTION	Fast_DATA,data
Default_PA:
	dc.w 0,Screen_width-1
	dc.w 0,Screen_height-1
	dc.w Brightest,Darkest,Normal,0

Default_Font:
	dc.l .Normal,.Fat,.High,.Fat_high

.Normal:	dc.w 8			;  8 pixels high
	dc.w 16			; 16 bytes per character
	dc.w 1			;  1 pixel between characters
	dc.w 1			;  1 pixel between lines
	dc.w 6			;  6 pixels to base-line
	dc.w 3			;  3 pixels per space
	dc.l Normal_font
	dc.l Normal_kerning_table
	dc.l Normal_conversion_table
	dc.l Normal_width_table
	dc.l Insert_8w
.Fat:	dc.w 8			;  8 pixels high
	dc.w 16			; 16 bytes per character
	dc.w 2			;  2 pixels between characters
	dc.w 1			;  1 pixel between lines
	dc.w 6			;  6 pixels to base-line
	dc.w 4			;  4 pixels per space
	dc.l Normal_font
	dc.l Normal_kerning_table
	dc.l Normal_conversion_table
	dc.l Normal_width_table
	dc.l Insert_fat_8w
.High:	dc.w 16			; 16 pixels high
	dc.w 16			; 16 bytes per character
	dc.w 1			;  1 pixel between characters
	dc.w 1			;  1 pixel between lines
	dc.w 13			; 13 pixels to base-line
	dc.w 4			;  4 pixels per space
	dc.l Normal_font
	dc.l Normal_kerning_table
	dc.l Normal_conversion_table
	dc.l Normal_width_table
	dc.l Insert_high_8w
.Fat_high:	dc.w 16			; 16 pixels high
	dc.w 16			; 16 bytes per character
	dc.w 2			;  2 pixels between characters
	dc.w 1			;  1 pixel between lines
	dc.w 13			; 13 pixels to base-line
	dc.w 4			;  4 pixels per space
	dc.l Normal_font
	dc.l Normal_kerning_table
	dc.l Normal_conversion_table
	dc.l Normal_width_table
	dc.l Insert_fat_high_8w

Techno_Font:
	dc.l .Normal,.Fat,.High,.Fat_high

.Normal:	dc.w 8			;  8 pixels high
	dc.w 16			; 16 bytes per character
	dc.w 1			;  1 pixel between characters
	dc.w 1			;  1 pixel between lines
	dc.w 6			;  6 pixels to base-line
	dc.w 3			;  3 pixels per space
	dc.l Techno_font
	dc.l Techno_kerning_table
	dc.l Normal_conversion_table
	dc.l Techno_width_table
	dc.l Insert_8w
.Fat:	dc.w 8			;  8 pixels high
	dc.w 16			; 16 bytes per character
	dc.w 2			;  2 pixels between characters
	dc.w 1			;  1 pixel between lines
	dc.w 6			;  6 pixels to base-line
	dc.w 4			;  4 pixels per space
	dc.l Techno_font
	dc.l Techno_kerning_table
	dc.l Normal_conversion_table
	dc.l Techno_width_table
	dc.l Insert_fat_8w
.High:	dc.w 16			; 16 pixels high
	dc.w 16			; 16 bytes per character
	dc.w 1			;  1 pixel between characters
	dc.w 1			;  1 pixel between lines
	dc.w 13			; 13 pixels to base-line
	dc.w 4			;  4 pixels per space
	dc.l Techno_font
	dc.l Techno_kerning_table
	dc.l Normal_conversion_table
	dc.l Techno_width_table
	dc.l Insert_high_8w
.Fat_high:	dc.w 16			; 16 pixels high
	dc.w 16			; 16 bytes per character
	dc.w 2			;  2 pixels between characters
	dc.w 1			;  1 pixel between lines
	dc.w 13			; 13 pixels to base-line
	dc.w 4			;  4 pixels per space
	dc.l Techno_font
	dc.l Techno_kerning_table
	dc.l Normal_conversion_table
	dc.l Techno_width_table
	dc.l Insert_fat_high_8w

Normal_kerning_table:
	dc.w 0			; Not converted

	dc.b "FaFcFdFeFgFmFnFoFpFqFrFsFuFvFwFxFyFz"
	dc.b "FäFöFüF.F:F,F;F'F+F-F=F<"
	dc.b "F0F1F2F3F4F5F6F7F8F9"

	dc.b "LaLcLeLgLjLoLqLtLuLvLy"

;	dc.b "PaPcPdPePgPmPnPoPpPqPrPsPuPvPwPxPyPz"
;	dc.b "PäPö"
	dc.b "P.P:P,P;P'P+P-P=P<"
;	dc.b "P0P1P2P3P4P5P6P7P8P9"

	dc.b "TaTcTdTeTgTmTnToTpTqTrTsTuTvTwTxTyTz"
	dc.b "TäTöTüT.T:T,T;T'T+T-T=T<"
	dc.b "T0T1T2T3T4T5T6T7T8T9"

	dc.b "YaYcYdYeYgYmYnYoYpYqYrYsYuYvYwYxYyYz"
	dc.b "YäYöYüY.Y:Y,Y;Y'Y+Y-Y=Y<"
	dc.b "Y0Y1Y2Y3Y4Y5Y6Y7Y8Y9"

	dc.b "ajbjcjdjejfjgjhjijkjljmjnj"
	dc.b "ojpjrjsjtjujvjwjxjyjzjAjBj"
	dc.b "CjDjEjFjGjHjIjKjLjMjNjOjPj"
	dc.b "QjRjSjTjUjVjWjXjYjZj0j1j2j"
	dc.b "3j4j5j6j7j8j9jäjÄjöjÖjüjÜj"
	dc.b "ßj.j:j'j""j?j!j/j(j)j+j-j=j"
	dc.b ">j<j"

	dc.b "lalcldlelflllolqltlulvlwly"
	dc.b "l'l""l?l+l-l<"
	dc.b "l0l1l2l3l4l5l6l7l8l9"

	dc.b 0,0

Techno_kerning_table:
	dc.w 0			; Not converted

	dc.b "a1a'a""a?"
	dc.b "l1l'l""l?"
	dc.b "r.r,"
	dc.b "v.v,"

	dc.b "FaFcFdFeFgFmFnFoFpFqFrFsFuFvFwFxFyFz"
	dc.b "FäFöFüF.F:F,F;F'F+F-F=F<"
	dc.b "F0F1F2F3F4F5F6F7F8F9"

	dc.b "TaTcTdTeTgTmTnToTpTqTrTsTuTvTwTxTyTz"
	dc.b "TäTöTüT.T:T,T;T'T+T-T=T<"
	dc.b "T0T1T2T3T4T5T6T7T8T9"

	dc.b 0,0

Normal_conversion_table:
	dc.b -1,77,75,81,-1,82,84,74		; spc !"#$%&'
	dc.b 79,80,83,85,72,86,70,78		; ()*+,-./
	dc.b 62,53,54,55,56,57,58,59,60,61	; 0123456789
	dc.b 71,73,89,87,88,76,-1		; :;<=>?@
	dc.b 27,28,29,30,31,32,33,34,35,36	; ABCDEFGHIJ
	dc.b 37,38,39,40,41,42,43,44,45,46	; KLMNOPQRST
	dc.b 47,48,49,50,51,52		; UVWXYZ
	dc.b -1,78,-1,-1,-1,-1		; [\]^_`
	dc.b 1,2,3,4,5,6,7,8,9,10		; abcdefghij
	dc.b 11,12,13,14,15,16,17,18,19,20	; klmnopqrst
	dc.b 21,22,23,24,25,26		; uvwxyz
	dcb.b 6,-1
	dc.b 68				; Ü ATARI
	dc.b -1				; È ATARI
	dc.b -1				; À ATARI
	dc.b 63				; ä ATARI
	dc.b -1				; À ATARI
	dc.b -1
	dc.b -1				; Ç ATARI
	dc.b -1				; È ATARI
	dc.b -1
	dc.b -1				; È ATARI
	dcb.b 3,-1
	dc.b 64				; Ä ATARI
	dc.b -1
	dc.b -1				; È ATARI
	dcb.b 3,-1
	dc.b 65				; ö ATARI
	dc.b -1
	dc.b -1				; Û ATARI
	dcb.b 2,-1
	dc.b 66				; Ö ATARI
	dc.b 67				; Ü ATARI
	dc.b -1				; Ç ATARI
	dcb.b 2,-1
	dc.b 69				; ß ATARI
	dc.b -1
	dc.b -1				; À ATARI
	dcb.b 21,-1
	dc.b -1				; À ATARI
	dcb.b 9,-1
	dc.b -1				; À AMIGA
	dc.b -1				; À AMIGA
	dcb.b 2,-1
	dc.b 64				; Ä AMIGA
	dcb.b 2,-1
	dc.b -1				; Ç AMIGA
	dc.b -1				; È AMIGA
	dc.b -1				; È AMIGA
	dc.b -1				; È AMIGA
	dcb.b 9,-1
	dc.b -1				; Ô AMIGA
	dc.b -1
	dc.b 66				; Ö AMIGA
	dcb.b 4,-1
	dc.b -1				; Û AMIGA
	dc.b 68				; Ü AMIGA
	dcb.b 2,-1
	dc.b 69				; ß AMIGA
	dc.b -1				; À AMIGA
	dc.b -1				; À AMIGA
	dc.b -1				; À AMIGA
	dc.b -1
	dc.b 63				; ä AMIGA
	dcb.b 2,-1
	dc.b -1				; Ç AMIGA
	dc.b -1				; È AMIGA
	dc.b -1				; È AMIGA
	dc.b -1				; È AMIGA
	dcb.b 9,-1
	dc.b -1				; Ô AMIGA
	dc.b -1
	dc.b 65				; ö AMIGA
	dcb.b 4,-1
	dc.b -1				; Û AMIGA
	dc.b 67				; ü AMIGA
	dcb.b 3,-1

Normal_width_table:
	dc.b 5
	dc.b 4,4,4,4,4,2,4,4,1,2
	dc.b 4,2,7,4,4,4,4,3,4,2
	dc.b 4,4,7,4,4,3
	dc.b 4,4,4,4,4,4,4,4,1,4
	dc.b 5,4,5,4,4,4,4,4,4,5
	dc.b 4,5,5,5,5,5
	dc.b 2,4,4,5,4,4,4,4,4,4
	dc.b 4,4,4,4,4,4,4,1,1,1
	dc.b 1,1,3,4,1,4
	dc.b 2,2,5,4,5,5,3,3,3,3
	dc.b 3,5,5,5
Techno_width_table:
	dc.b 5
	dc.b 5,4,4,4,4,2,4,4,1,1
	dc.b 4,2,7,4,4,4,4,4,4,2
	dc.b 4,4,7,4,4,4
	dc.b 4,4,4,4,4,4,4,4,1,4
	dc.b 5,4,5,5,4,4,5,4,4,5
	dc.b 4,5,5,5,5,5
	dc.b 2,4,4,4,4,4,4,4,4,4
	dc.b 5,4,4,4,4,4,4,2,2,2
	dc.b 2,1,3,4,1,4
	dc.b 2,2,5,4,5,5,3,3,3,3
	dc.b 3,5,5,5
	even


	SECTION	Fast_BSS,bss
PA_Sp:	ds.l 1			; PA stack
PAStack_start:        
	ds.l Max_PA
PAStack_end:

PA_width:	ds.w 1			; In pixels
PA_height:	ds.w 1

Font_Sp:	ds.l 1			; Font stack
FontStack_start:
	ds.l Max_Fonts
FontStack_end:
