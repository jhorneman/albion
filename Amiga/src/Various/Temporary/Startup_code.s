; Colour palette management
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	MODULE	DDT_Startup

	OUTPUT	Objects/Core/Startup.o

	OPT	O1+,O2+,O4+,O5+,O10+,O11+,OW-
	OPT	O7+,EVEN,USER,INCONCE
	OPT	P=68020


	XDEF	Init_computer
	XDEF	Exit_computer
	XDEF	Set_tag_data
	XDEF	Create_port
	XDEF	Destroy_port
	XDEF	Create_IO_request
	XDEF	Destroy_IO_request
	XDEF	Forbid_it
	XDEF	Permit_it

	XDEF	DDT_name
	XDEF	Kickstart_version
	XDEF	Processor_type
	XDEF	VBLs_per_second
	XDEF	VBL_counter
	XDEF	My_task_ptr
	XDEF	DOS_base
	XDEF	Graphics_base
	XDEF	Intuition_base
	XDEF	Extended_error_code


	include	Constants/Global.i
	include	Constants/OS/OS.i
	include	Constants/Core.i


	SECTION	Program,code
;***************************************************************************
; [ Initialize the computer state ]
; All registers are restored
;***************************************************************************
Init_computer:
	movem.l	d0-d7/a0-a6,-(sp)
	jsr	Get_computer_state
	jsr	Open_libraries
	sub.l	a1,a1			; Find own task
	kickEXEC	FindTask
	move.l	d0,My_task_ptr
	moveq.l	#VERB,d0			; Install VBL interrupt handler
	lea.l	My_VBL_interrupt,a1
	kickEXEC	AddIntServer
	jsr	Init_screens		; Initialize Core
	jsr	Init_files
	jsr	Init_keyboard
	jsr	Init_graphics
	movem.l	(sp)+,d0-d7/a0-a6
	rts

;***************************************************************************
; [ Restore the computer state ]
; All registers are restored
;***************************************************************************
Exit_computer:
	movem.l	d0-d7/a0-a6,-(sp)
; Exit Core
	moveq.l	#VERB,d0			; Remove VBL interrupt handler
	lea.l	My_VBL_interrupt,a1
	kickEXEC	RemIntServer
	jsr	Close_libraries
	movem.l	(sp)+,d0-d7/a0-a6
	rts

;*****************************************************************************
; [ Determine computer state ]
; All registers are restored
;*****************************************************************************
Get_computer_state:
	movem.l	d0/d1/a0,-(sp)
	move.l	Exec_base,a0
; ---------- Get Kickstart version ----------------
	move.w	lib_Version(a0),d0
	cmp.w	#39,d0			; Too low ?
	bpl.s	.Ok
	move.l	#KICKSTART_ERROR,d0		; Yes -> Exit
	jmp	Fatal_error
.Ok:	move.w	d0,Kickstart_version
	LOCAL
; ---------- Get VBLs per second ------------------
	moveq.l	#0,d0
	move.b	VBlankFrequency(a0),d0
	move.w	d0,VBLs_per_second
; ---------- Get processor type -------------------
	move.w	AttnFlags(a0),d0
	btst	#AFB_68040,d0		; 68040 ?
	beq.s	.Not_40
	moveq.l	#40,d1			; Yes
	bra.s	.Done
.Not_40:	btst	#AFB_68030,d0		; 68030 ?
	beq.s	.Not_30
	moveq.l	#30,d1			; Yes
	bra.s	.Done
.Not_30:	btst	#AFB_68020,d0		; 68020 ?
	beq.s	.Not_20
	moveq.l	#20,d1			; Yes
	bra.s	.Done
.Not_20:	btst	#AFB_68010,d0		; 68010 ?
	beq.s	.Not_10
	moveq.l	#10,d1			; Yes
	bra.s	.Done
.Not_10:	moveq.l	#0,d1			; 68000
.Done:	cmp.w	#20,d0			; Too low ?
	bpl.s	.Ok
	move.l	#PROCESSOR_ERROR,d0		; Yes -> Exit
	jmp	Fatal_error
.Ok:	move.w	d1,Processor_type
	movem.l	(sp)+,d0/d1/a0
	rts

;*****************************************************************************
; [ Open libraries ]
; All registers are restored
;*****************************************************************************
Open_libraries:
	movem.l	d0/d1/a0/a1,-(sp)
	moveq.l	#39,d0			; Open Kick 3.0 DOS library
	lea.l	DOS_library_name,a1
	kickEXEC	OpenLibrary
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#DOS_LIBRARY_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,DOS_base
	LOCAL
	moveq.l	#39,d0			; Open Kick 3.0 Graphics library
	lea.l	Graphics_library_name,a1
	kickEXEC	OpenLibrary
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#GRAPHICS_LIBRARY_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,Graphics_base
	LOCAL
	moveq.l	#33,d0			; Open Kick 3.0 Intuition library
	lea.l	Intuition_library_name,a1
	kickEXEC	OpenLibrary
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#INTUITION_LIBRARY_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,Intuition_base
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;*****************************************************************************
; [ Close libraries ]
; All registers are restored
;*****************************************************************************
Close_libraries:
	movem.l	d0/d1/a0/a1,-(sp)
	move.l	DOS_base,d0		; DOS library open ?
	beq.s	.No
	move.l	d0,a1			; Yes -> Close
	kickEXEC	CloseLibrary
	clr.l	DOS_base
.No:	LOCAL
	move.l	Graphics_base,d0		; Graphics library open ?
	beq.s	.No
	move.l	d0,a1			; Yes -> Close
	kickEXEC	CloseLibrary
	clr.l	Graphics_base
.No:	LOCAL
	move.l	Intuition_base,d0		; Intuition library open ?
	beq.s	.No
	move.l	d0,a1			; Yes -> Close
	kickEXEC	CloseLibrary
	clr.l	Intuition_base
.No:	LOCAL
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************
; [ VBL interrupt handler ]
; All registers are restored
;***************************************************************************
My_VBL_handler:
	movem.l	d0-d7/a0-a6,-(sp)
	addq.w	#1,VBL_counter		; Count

;	jsr	Read_mouse		; Handle mouse
;	jsr	Draw_mouse

;	jsr	Random			; Randomize
;	jsr	VblQ_handler		; Do VBL queue

;	tst.b	Music_flag		; Music on ?
;	beq.s	.Exit
;	music	Music_introut		; Yes
;.Exit:

	movem.l	(sp)+,d0-d7/a0-a6
	rts

;*****************************************************************************
; [ Set data of a tag in a tag list ]
;   IN : d0 - Tag type (.l)
;        d1 - Tag data (.l)
;        a0 - Pointer to tag list (.l)
; All registers are restored
; Notes :
;   - No funny tag list tricks (like TAG_MORE, TAG_SKIP, etc.) are allowed.
;*****************************************************************************
Set_tag_data:
	movem.l	d2/a0,-(sp)
.Again:	move.l	ti_Tag(a0),d2		; Get tag type
	cmp.l	#TAG_DONE,d2		; End of tag list ?
	beq.s	.Exit
	cmp.l	d0,d2			; No -> Is this the tag ?
	bne.s	.Next
	move.l	d1,ti_Data(a0)		; Yes -> Set data
	bra.s	.Exit
.Next:	addq.l	#ti_data_size,a0		; No -> Next tag
	bra.s	.Again
.Exit:	movem.l	(sp)+,d2/a0
	rts

;*****************************************************************************
; [ Create a message port ]
;  OUT : d0 - Pointer to a message port (.l)
; Changed registers : d0
;*****************************************************************************
Create_port:
	movem.l	d1/a0/a1,-(sp)
	kickEXEC	CreateMsgPort		; Create a message port
	tst.l	d0			; Success ?
	bne.s	.Exit
	move.l	#CREATE_PORT_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Exit:	movem.l	(sp)+,d1/a0/a1
	rts

;*****************************************************************************
; [ Destroy a message port ]
;  OUT : a0 - Pointer to a message port (.l)
; All registers are restored
;*****************************************************************************
Destroy_port:
	movem.l	d0/d1/a0/a1,-(sp)
	cmp.l	#0,a0
	beq.s	.Exit
	kickEXEC	DeleteMsgPort
.Exit:	movem.l	(sp)+,d0/d1/a0/a1
	rts

;*****************************************************************************
; [ Create an I/O request ]
;   IN : d0 - Pointer to a message port (.l)
;  OUT : d0 - Pointer to an I/O request (.l)
; Changed registers : d0
;*****************************************************************************
Create_IO_request:
	movem.l	d1/a0/a1,-(sp)
	move.l	d0,a0			; Create I/O request
	move.l	#iostdreq_data_size,d0
	kickEXEC	CreateIORequest
	tst.l	d0			; Success ?
	bne.s	.Exit
	move.l	#CREATE_IOREQ_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Exit:	movem.l	(sp)+,d1/a0/a1
	rts

;*****************************************************************************
; [ Destroy an I/O request
;  OUT : a0 - Pointer to an I/O request (.l)
; All registers are restored
;*****************************************************************************
Destroy_IO_request:
	movem.l	d0/d1/a0/a1,-(sp)
	cmp.l	#0,a0
	beq.s	.Exit
	kickEXEC	DeleteIORequest
.Exit:	movem.l	(sp)+,d0/d1/a0/a1
	rts

;*****************************************************************************
; [ Forbid multitasking ]
; All registers are	restored
;*****************************************************************************
Forbid_it:
	movem.l	d0/d1/a0,-(sp)
	kickEXEC	Forbid
	movem.l	(sp)+,d0/d1/a0
	rts

;*****************************************************************************
; [ Permit multitasking ]
; All registers are	restored
;*****************************************************************************
Permit_it:
	movem.l	d0/d1/a0,-(sp)
	kickEXEC	Permit
	movem.l	(sp)+,d0/d1/a0
	rts


; !!! Extended error code in Extended_error_code
;     Usually for OS error codes

Fatal_error:
;	move.l	d0,Return_value
	jmp	Exit_program

Exit_program:
	rts




	SECTION	Fast_DATA,data
My_VBL_interrupt:
	dc.l 0,0
	dc.b NT_INTERRUPT
	dc.b 127				; Priority = maximum
	dc.l DDT_name
	dc.l 0
	dc.l My_VBL_handler

DDT_name:	dc.b "DDT",0

DOS_library_name:	dc.b "dos.library",0
Graphics_library_name:	dc.b "graphics.library",0
Intuition_library_name:	dc.b "intuition.library",0
	even



	SECTION	Fast_BSS,bss
Kickstart_version:	ds.w 1
Processor_type:	ds.w 1
VBLs_per_second:	ds.w 1
VBL_counter:	ds.w 1
My_task_ptr:	ds.l 1

DOS_base:	ds.l 1
Graphics_base:	ds.l 1
Intuition_base:	ds.l 1

Extended_error_code:	ds.l 1
