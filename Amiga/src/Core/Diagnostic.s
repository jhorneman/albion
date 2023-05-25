; Diagnostic routines
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 9-10-1992

	XDEF	Init_diagnostic_screen
	XDEF	Exit_diagnostic_screen
	XDEF	DI_Print
	XDEF	DI_CR
	XDEF	DI_DecB
	XDEF	DI_DecW
	XDEF	DI_DecL
	XDEF	DI_Print_text

	SECTION	Program,code
	ifne	Cheat
;*****************************************************************************
; [ Display memory status ]
;*****************************************************************************
Memory_status:
	jsr	Init_diagnostic_screen
	lea.l	Headline,a1		; Print headline
	jsr	DI_Print
	jsr	DI_CR
	jsr	DI_CR
	lea.l	Totals,a5			; Clear totals
	clr.l	(a5)
	clr.l	4(a5)
	clr.l	8(a5)
	clr.l	12(a5)
	lea.l	Memory_areas,a0		; Print area infos
	moveq.l	#0,d1
	move.w	Number_of_areas,d7
	bra	.Entry
.Loop:	move.w	d1,d0			; Area number
	jsr	DI_DecB
	lea.l	Chip_text,a1		; Memory type
	tst.b	Memory_type(a0)
	beq.s	.Chip
	lea.l	Fast_text,a1
.Chip:	jsr	DI_Print
	move.l	Area_size(a0),d0		; Area size
	move.l	d0,d2
	add.l	d0,(a5)
	jsr	DI_DecL
	move.l	a0,-(sp)			; Largest free block
	jsr	Find_LFB
	move.l	Block_size(a0),d0
	jsr	DI_DecL
	move.l	(sp)+,a0
	jsr	Calculate_TFM		; Total free memory
	sub.l	d0,d2
	add.l	d0,4(a5)
	jsr	DI_DecL
	move.b	#-1,d0			; Extra memory gained
	jsr	Calculate_EMG
	sub.l	d0,d2
	add.l	d0,8(a5)
	jsr	DI_DecL
	move.l	d2,d0			; Used memory
	add.l	d0,12(a5)
	jsr	DI_DecL
	jsr	DI_CR
	addq.w	#1,d1
	lea.l	Area_data_size(a0),a0
.Entry:	dbra	d7,.Loop
; ---------- Print area totals --------------------
	jsr	DI_CR
	lea.l	Total_text,a1
	jsr	DI_Print
	move.l	(a5),d0			; Area size
	jsr	DI_DecL
	add.w	#7+1,Print_X		; Skip LFB
	move.l	4(a5),d0			; Total free memory
	jsr	DI_DecL
	move.l	8(a5),d0			; Extra memory gained
	jsr	DI_DecL
	move.l	12(a5),d0			; Used memory
	jsr	DI_DecL
	jsr	DI_CR
; ---------- Exit ---------------------------------
	jmp	Exit_diagnostic_screen

;*****************************************************************************
; [ Display memory blocks ]
;*****************************************************************************
Display_memory_blocks:
	jsr	Init_diagnostic_screen
	lea.l	Memory_areas,a0		; Search all areas
	lea.l	Memory_type_texts,a2
	move.w	Number_of_areas,d7
	bra	.Entry
.Loop:	move.l	a0,a3			; Search area
	lea.l	Chip_text2,a4		; Memory type
	tst.b	Memory_type(a0)
	beq.s	.Again
	lea.l	Fast_text2,a4
.Again:	move.l	Block_next(a3),d1		; End of chain ?
	beq.s	.Next
	move.l	d1,a3
	tst.l	Identifier(a3)		; Free ?
	beq.s	.Again
	lea.l	Space,a1			; Print Allocated status
	btst	#Allocated,Block_flags(a3)
	bne.s	.Yes1
	lea.l	Not_allocated_text,a1
.Yes1:	jsr	DI_Print
	lea.l	Invalid_text,a1		; Print Invalid status
	btst	#Invalid,Block_flags(a3)
	bne.s	.Yes2
	lea.l	Space,a1
.Yes2:	jsr	DI_Print
	move.l	a4,a1			; Print memory type
	jsr	DI_Print
	moveq.l	#0,d0			; Get type string
	move.b	Block_file_type(a3),d0
	lsl.w	#2,d0
	move.l	0(a2,d0.w),a1
	jsr	DI_Print			; Print it
	move.w	Block_subfile_nr(a3),d0	; Print subfile number
	jsr	DI_DecW
	move.l	Block_size(a3),d0		; Print block size
	jsr	DI_DecL
	jsr	DI_CR
	bra.s	.Again
.Next:	lea.l	Area_data_size(a0),a0	; Next area
.Entry:	dbra	d7,.Loop
	jmp	Exit_diagnostic_screen

;*****************************************************************************
; [ Display memory claims ]
;*****************************************************************************
Display_claims:
	jsr	Init_diagnostic_screen
	lea.l	Headline2,a1		; Print headline
	jsr	DI_Print
	jsr	DI_CR
	jsr	DI_CR
	lea.l	Memory_areas,a0		; Search all areas
	lea.l	Memory_type_texts,a2
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	move.l	a0,a3			; Search area
.Again:	move.l	Block_next(a3),d1		; End of chain ?
	beq.s	.Next
	move.l	d1,a3
	tst.b	Block_claim(a3)		; Claimed ?
	beq.s	.Again
	lea.l	Space,a1			; Print Allocated status
	btst	#Allocated,Block_flags(a3)
	bne.s	.Yes
	lea.l	Not_allocated_text,a1
.Yes:	jsr	DI_Print
	moveq.l	#0,d0			; Get type string
	move.b	Block_file_type(a3),d0
	lsl.w	#2,d0
	move.l	0(a2,d0.w),a1
	jsr	DI_Print			; Print it
	move.w	Block_subfile_nr(a3),d0	; Print subfile number
	jsr	DI_DecW
	move.l	Block_size(a3),d0		; Print block size
	jsr	DI_DecL
	jsr	DI_CR
	bra.s	.Again
.Next:	lea.l	Area_data_size(a0),a0	; Next area
.Entry:	dbra	d7,.Loop
	jmp	Exit_diagnostic_screen

;*****************************************************************************
; [ Display memory handles ]
;*****************************************************************************
Display_handles:
	jsr	Init_diagnostic_screen
	lea.l	Headline7,a1		; Print headline
	jsr	DI_Print
	jsr	DI_CR
	jsr	DI_CR
	lea.l	Memory_areas,a0		; Search all areas
	lea.l	Memory_type_texts,a2
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	move.l	a0,a3			; Search area
.Again:	move.l	Block_next(a3),d1		; End of chain ?
	beq.s	.Next
	move.l	d1,a3
	tst.b	Block_handle(a3)		; Has a handle ?
	beq.s	.Again
	lea.l	Space,a1			; Print Allocated status
	btst	#Allocated,Block_flags(a3)
	bne.s	.Yes
	lea.l	Not_allocated_text,a1
.Yes:	jsr	DI_Print
	moveq.l	#0,d0			; Get type string
	move.b	Block_file_type(a3),d0
	lsl.w	#2,d0
	move.l	0(a2,d0.w),a1
	jsr	DI_Print			; Print it
	move.w	Block_subfile_nr(a3),d0	; Print subfile number
	jsr	DI_DecW
	move.l	Block_size(a3),d0		; Print block size
	jsr	DI_DecL
	jsr	DI_CR
	bra.s	.Again
.Next:	lea.l	Area_data_size(a0),a0	; Next area
.Entry:	dbra	d7,.Loop
	jmp	Exit_diagnostic_screen

;*****************************************************************************
; [ Print claimed block ]
;   IN : a0 - Pointer to memory block structure (.l)
; All registers are restored
;*****************************************************************************
Print_claimed_block:
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	a0,a3			; Save
	jsr	Init_diagnostic_screen
	lea.l	Headline4,a1		; Print headline
	jsr	DI_Print
	jsr	DI_CR
	jsr	DI_CR
	lea.l	Space,a1			; Print Allocated status
	btst	#Allocated,Block_flags(a3)
	bne.s	.Yes
	lea.l	Not_allocated_text,a1
.Yes:	jsr	DI_Print
	moveq.l	#0,d0			; Get type string
	move.b	Block_file_type(a3),d0
	lea.l	Memory_type_texts,a2
	lsl.w	#2,d0
	move.l	0(a2,d0.w),a1
	jsr	DI_Print			; Print it
	move.w	Block_subfile_nr(a3),d0	; Print subfile number
	jsr	DI_DecW
	move.l	Block_size(a3),d0		; Print block size
	jsr	DI_DecL
	jsr	DI_CR
	jsr	Exit_diagnostic_screen
	movem.l	(sp)+,d0-d7/a0-a6
	rts

;*****************************************************************************
; [ Toggle update time display ]
; All registers are restored
;*****************************************************************************
Toggle_update_time:
	not.b	Show_update_time		; Toggle
	tst.b	Show_update_time		; On ?
	beq	.Exit
	moveq.l	#5,d0
	jsr	Delay
.Exit:	rts
	endc

;*****************************************************************************
; [ Initialize diagnostic screen ]
; All registers are restored
;*****************************************************************************
Init_diagnostic_screen:
	move.l	a0,-(sp)
	move.l	On_screen,a0		; Clear screen
	jsr	Clear_screen
	move.l	a0,Work_screen
	jsr	Cursor_home		; Cursor home
	Push	PA,Default_PA
	Push	CA,Default_CA
	move.w	#White,Ink_colour
	move.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Exit diagnostic screen ]
; All registers are restored
;*****************************************************************************
Exit_diagnostic_screen:
	Pop	PA
	Pop	CA
	jsr	Wait_4_click		; Wait for user
	jmp	Update_screen		; "Restore" screen

;***************************************************************************	
; [ Print text ]
;   IN : a1 - Pointer to text (.l)
; All registers are restored
;***************************************************************************	
DI_Print_text:
	movem.l	d0/a0/a1,-(sp)
.Again:	cmp.b	#-1,(a1)			; End of text ?
	beq.s	.Done
	jsr	DI_Print			; No -> Print
	jsr	DI_CR			; Next line
	move.l	a1,a0			; Skip string
	jsr	Strlen
	lea.l	1(a1,d0.w),a1
	bra.s	.Again
.Done:	movem.l	(sp)+,d0/a0/a1
	rts

;***************************************************************************	
; [ Print string ]
;   IN : a1 - Pointer to string (.l)
; All registers are restored
;***************************************************************************	
DI_Print:
	movem.l	d0/d1/d7/a0,-(sp)
	move.l	a1,a0
	jsr	Strlen
	move.w	d0,d7
	move.w	Print_X,d0
	move.w	Print_Y,d1
	mulu.w	#Char_width+1,d0
	mulu.w	#Char_height+2,d1
	jsr	Put_text_line
;	addq.w	#1,d7
	add.w	d7,Print_X
	movem.l	(sp)+,d0/d1/d7/a0
	rts

;***************************************************************************	
; [ Print decimal byte ]
;   IN : d0 - Value (.b)
; All registers are restored
;***************************************************************************	
DI_DecB:
	movem.l	d0/d1/d6/d7/a0,-(sp)
	and.w	#$00ff,d0
	lea.l	Number,a0
	moveq.l	#" ",d6
	moveq.l	#3,d7
	jsr	DecR_convert
	lea.l	Number,a0
	move.w	Print_X,d0
	move.w	Print_Y,d1
	mulu.w	#Char_width+1,d0
	mulu.w	#Char_height+2,d1
	jsr	Put_text_line
	add.w	#3+1,Print_X
	movem.l	(sp)+,d0/d1/d6/d7/a0
	rts

;***************************************************************************	
; [ Print decimal word ]
;   IN : d0 - Value (.w)
; All registers are restored
;***************************************************************************	
DI_DecW:
	movem.l	d0/d1/d6/d7/a0,-(sp)
	and.l	#$0000ffff,d0
	lea.l	Number,a0
	moveq.l	#" ",d6
	moveq.l	#5,d7
	jsr	DecR_convert
	lea.l	Number,a0
	move.w	Print_X,d0
	move.w	Print_Y,d1
	mulu.w	#Char_width+1,d0
	mulu.w	#Char_height+2,d1
	jsr	Put_text_line
	add.w	#5+1,Print_X
	movem.l	(sp)+,d0/d1/d6/d7/a0
	rts

;***************************************************************************	
; [ Print decimal longword ]
;   IN : d0 - Value (.l)
; All registers are restored
;***************************************************************************	
DI_DecL:
	movem.l	d0/d1/d6/d7/a0,-(sp)
	lea.l	Number,a0
	moveq.l	#" ",d6
	moveq.l	#7,d7
	jsr	DecR_convert
	lea.l	Number,a0
	move.w	Print_X,d0
	move.w	Print_Y,d1
	mulu.w	#Char_width+1,d0
	mulu.w	#Char_height+2,d1
	jsr	Put_text_line
	add.w	#7+1,Print_X
	movem.l	(sp)+,d0/d1/d6/d7/a0
	rts

;***************************************************************************	
; [ Carriage return ]
; All registers are restored
;***************************************************************************	
DI_CR:
	move.l	a0,-(sp)
	clr.w	Print_X			; Back to start of line
	addq.w	#1,Print_Y		; Next line
	cmp.w	#28,Print_Y		; Screen full ?
	bmi.s	.Exit
	clr.w	Print_Y			; Back to top of screen
	jsr	Wait_4_click		; Wait for user
	move.l	Work_screen,a0		; Clear screen
	jsr	Clear_screen
.Exit:	move.l	(sp)+,a0
	rts

;***************************************************************************	
; [ Cursor home ]
; All registers are restored
;***************************************************************************	
Cursor_home:
	clr.w	Print_X			; Cursor home
	clr.w	Print_Y
	rts

;***************************************************************************	
; The DATA & BSS segments
;***************************************************************************	
	SECTION	Fast_DATA,data
Memory_type_texts:
	dc.l .Type00,.Type01,.Type02,.Type03
	dc.l .Type04,.Type05,.Type06,.Type07
	dc.l .Type08,.Type09,.Type10,.Type11
	dc.l .Type12,.Type13,.Type14,.Type15
	dc.l .Type16,.Type17,.Type18,.Type19
	dc.l .Type20,.Type21,.Type22,.Type23
	dc.l .Type24,.Type25,.Type26,.Type27
	dc.l .Type28,.Type29,.Type30,.Type31
	dc.l .Type32,.Type33,.Type34,.Type35
	dc.l .Type36,.Type37,.Type38,.Type39
	dc.l .Type40,.Type41,.Type42,.Type43
	dc.l .Type44

.Type00:	dc.b "Memory block         ",0


	ifne Cheat
Diagnostics_list1:
	dc.b "q"
	even
	dc.l Memory_status
	dc.b "w"
	even
	dc.l Grab_screen
	dc.b "e"
	even
	dc.l Display_memory_blocks
	dc.b "r"
	even
	dc.l Display_claims
	dc.b "g"
	even
	dc.l Toggle_update_time
	dc.b "h"
	even
	dc.l Display_handles
	dc.b "l"
	even
	dc.l Display_stacks
	dc.b " "
	even
	dc.l Exit_program
	dc.w 0

Headline:	dc.b " NR TYPE    SIZE     LFB     TFM     EMG    USED",0
Headline2:	dc.b "CLAIMED MEMORY BLOCKS :",0
Headline3:	dc.b "OUT OF MEMORY !",0
Headline4:	dc.b "CLAIMED BLOCK DURING GARBAGE COLLECTION :",0
Headline6:	dc.b "FINGERPRINT :",0
Headline7:	dc.b "MEMORY HANDLES :",0

Text1:	dc.b "REQUIRED MEMORY SIZE : ",0
Text2:	dc.b "REQUIRED MEMORY TYPE : ",0
Chip_text:	dc.b "CHIP ",0
Fast_text:	dc.b "FAST ",0
Dontcare_text:	dc.b "DONT CARE",0
Total_text:	dc.b "   TOTAL ",0
Space:	dc.b " ",0
Not_allocated_text	dc.b "X",0
Invalid_text:	dc.b "I",0
Chip_text2:	dc.b "C ",0
Fast_text2:	dc.b "F ",0
	even
	else
Diagnostics_list1:
	ifne Fingerprint
	dc.b "1"
	even
	dc.l Print_second_fingerprint
	else
	dc.b "1"
	even
	dc.l Pretty_colours
	endc
	dc.w 0
	endc

	SECTION	Fast_BSS,bss
Print_X:	ds.w 1
Print_Y:	ds.w 1

	ifne	Cheat
Totals:	ds.l 4
Grabbed_handle:	ds.b 1
	even
Grabbed_filename_ptr:	ds.l 1
	endc

	ifne Fingerprint
String:	ds.b Fingerprint_size
	ds.b 1
	even
	endc
