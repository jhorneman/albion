
	OUTPUT	RAM:DDT_MCP

	OPT AMIGA

NL	EQU 10
CLRHOME	EQU $0c


	incdir	DDT:Constants/
	include	Global.i
	include	OS/OS.i


	SECTION	Program,code
Start:
	moveq.l	#39,d0			; Open DOS library
	lea.l	DOS_library_name,a1
	kickEXEC	OpenLibrary
	tst.l	d0			; Success ?
	bne.s	.Ok1
	move.l	#DOS_LIBRARY_ERROR,d0	; No -> Exit
	bra.s	.Error
.Ok1:	move.l	d0,DOS_base
	move.l	#Program_filename,d1	; Load program
	kickDOS	LoadSeg
	tst.l	d0			; Errors ?
	bne.s	.Ok2
	moveq.l	#-1,d0			; Yes
	bra.s	.Done
.Ok2:	move.l	d0,d1			; No -> Get program address
	lsl.l	#2,d1
	addq.l	#4,d1
	move.l	d1,a1
	move.l	a5,a0			; Get parameter
	move.l	d0,-(sp)			; Execute
	jsr	(a1)
	move.l	d0,d7
	move.l	(sp)+,d1			; Remove program
	kickDOS	UnLoadSeg
	move.l	d7,d0			; Output
.Done:	tst.l	d0			; Errors ?
	beq	.Exit
	addq.l	#1,d0			; Yes
	neg.l	d0
.Error:	lea.l	Error_texts,a0
.Again:	move.l	(a0)+,d1
	bmi.s	.Found
	cmp.l	d0,d1
	beq.s	.Found
.Skip:	tst.b	(a0)+
	bne.s	.Skip
	move.l	a0,d1
	btst	#0,d1
	beq.s	.Again
	addq.l	#1,a0
	bra.s	.Again
.Found:	move.l	a0,a5			; Protect
	move.l	#Console_window_name,d1	; Open console window
	move.l	#MODE_OLDFILE,d2
	kickDOS	Open
	tst.l	d0
	beq	.Exit
	move.l	d0,Console_window_handle
	lea.l	Start_text,a0		; Print start text
	jsr	Print_1_text
	move.l	a5,a0			; Print error text
	jsr	Print_1_text
	lea.l	End_text,a0		; Print end text
	jsr	Print_1_text
	jsr	Wait_4_key		; Wait
	move.l	Console_window_handle,d1	; Close window
	kickDOS	Close
.Exit:	move.l	DOS_base,d0		; DOS library open ?
	beq.s	.No
	move.l	d0,a1			; Yes -> Close
	kickEXEC	CloseLibrary
	clr.l	DOS_base
.No:	rts

;***************************************************************************
; [ Wait for a key in the console window ]
;   IN : { Console_window_handle} - Handle of console window (.l)
; Changed registers : d0,d1,d2,d3,a0,a1
;***************************************************************************
Wait_4_key:
	move.l	Console_window_handle,d1	; Wait for a key
	move.l	#Read_buffer,d2
	moveq.l	#1,d3
	kickDOS	Read
	rts

;***************************************************************************
; [ Print a single text in the error window ]
;   IN : a0 - Pointer to text (.l)
; All registers are restored
;***************************************************************************
Print_1_text:
	movem.l	d0-d3/a0/a1,-(sp)
	jsr	Strlen			; Get text length
	tst.l	d0			; Zero ?
	beq.s	.Exit
	move.l	Console_window_handle,d1	; Print
	move.l	a0,d2
	move.l	d0,d3
	kickDOS	Write
.Exit:	movem.l	(sp)+,d0-d3/a0/a1
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
; The DATA & BSS segments
;***************************************************************************
	SECTION	Fast_DATA,data
DOS_library_name:	dc.b "dos.library",0
Program_filename:	dc.b  "DDT_test",0
Console_window_name:	dc.b "CON:0/20/640/200/ DDT output",0

Start_text:
	dc.b "An error occurred :",NL,NL,"  ",0
End_text:
	dc.b NL,NL
	dc.b "Press return to exit.",NL,0
	even

Error_texts:
	dc.l 0
	dc.b "Program not found.",0
	even
	dc.l KICKSTART_ERROR
	dc.b "Wrong Kickstart version.",0
	even
	dc.l PROCESSOR_ERROR
	dc.b "Processor type too low.",0
	even

	dc.l DOS_LIBRARY_ERROR
	dc.b "DOS library couldn't be opened.",0
	even
	dc.l GRAPHICS_LIBRARY_ERROR
	dc.b "Graphics library couldn't be opened.",0
	even
	dc.l INTUITION_LIBRARY_ERROR
	dc.b "Intuition library couldn't be opened.",0
	even
	dc.l ASL_LIBRARY_ERROR
	dc.b "ASL library couldn't be opened.",0
	even

	dc.l ALLOC_SPRITE_ERROR
	dc.b "Sprite couldn't be allocated.",0
	even
	dc.l GET_SPRITE_ERROR
	dc.b "Get sprite failed.",0
	even

	dc.l MODE_ID_ERROR
	dc.b "No suitable screen mode available.",0
	even
	dc.l ALLOC_BITMAP_ERROR
	dc.b "Bitmap couldn't be allocated.",0
	even
	dc.l INTERLEAVE_ERROR
	dc.b "Bitmap isn't interleaved.",0
	even
	dc.l OPEN_SCREEN_ERROR
	dc.b "Screen couldn't be opened.",0
	even
	dc.l OPEN_WINDOW_ERROR
	dc.b "Window couldn't be opened.",0
	even
	dc.l ALLOC_SCREEN_BUFFER_ERROR
	dc.b "Screen-buffer couldn't be allocated.",0
	even

	dc.l CREATE_PORT_ERROR
	dc.b "Port couldn't be created.",0
	even
	dc.l CREATE_IOREQ_ERROR
	dc.b "I/O-requester couldn't be created.",0
	even
	dc.l OPEN_DEVICE_ERROR
	dc.b "Device couldn't be opened.",0
	even
	dc.l ADD_INPUT_HANDLER_ERROR
	dc.b "Input handler couldn't be added.",0
	even

	dc.l NOT_ENOUGH_CHIP_MEMORY
	dc.b "Not enough CHIP memory.",0
	even
	dc.l NOT_ENOUGH_FAST_MEMORY
	dc.b "Not enough FAST memory.",0
	even
	dc.l NOT_ENOUGH_BLOCKS
	dc.b "Not enough memory blocks.",0
	even
	dc.l NOT_ENOUGH_HANDLES
	dc.b "Not enough memory handles.",0
	even
	dc.l HANDLES_CORRUPTED
	dc.b "Memory handles are corrupted.",0
	even
	dc.l AREA_CORRUPTED
	dc.b "Memory area is corrupted.",0
	even
	dc.l NOT_ENOUGH_FILES
	dc.b "Not enough files.",0
	even

	dc.l DATA_DIR_NOT_FOUND
	dc.b "Data-directory wasn't found.",0
	even
	dc.l ILLEGAL_FILE_TYPE
	dc.b "Illegal file type.",0
	even
	dc.l INCOMPLETE_READ
	dc.b "Incomplete read.",0
	even
	dc.l INCOMPLETE_WRITE
	dc.b "Incomplete write.",0
	even
	dc.l BAD_OMNIFILE
	dc.b "Bad Omnifile.",0
	even
	dc.l SUBFILE_NOT_LOADED
	dc.b "Subfile couldn't be loaded.",0
	even
	dc.l DISK_ERROR
	dc.b "Disk error.",0
	even

	dc.l TEXT_TOO_LONG_ERROR
	dc.b "Text too long.",0
	even

	dc.l -1
	dc.b "UNKNOWN ERROR",0
	even


	SECTION	Fast_BSS,bss
DOS_base:	ds.l 1
Console_window_handle:	ds.l 1
Read_buffer:	ds.l 1
