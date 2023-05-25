; Startup code
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	XDEF	Start
	XDEF	Exit_program
	XDEF	Fatal_error
	XDEF	Clear_cache
	XDEF	My_vsync
	XDEF	Dummy
	XDEF	File_requester

	XDEF	DDT_name
	XDEF	Kickstart_version
	XDEF	Processor_type
	XDEF	VBLs_per_second
	XDEF	VBL_counter
	XDEF	DOS_base
	XDEF	Graphics_base
	XDEF	Intuition_base
	XDEF	Extended_error_code

	SECTION	Program,code
	jmp	Start
DDT_name:
Version_string1:
	dc.b "DDT v0.01",0		; Version info
Version_string2:
	dc.b "4-2-1994 / Deutsch",0
	even

;*****************************************************************************
; [ DDT ]
;   IN : a0 - Pointer to interface structure (.l)
;  OUT : d0 - Return value (.l)
; No registers are restored
;*****************************************************************************
Start:
	move.l	a0,Interface_ptr		; Store pointer
	move.l	(sp)+,Return_address	; Store return address
	move.l	sp,Stack_base
	jsr	Init_computer		; Initialize computer
	jsr	Init_memory		; Initialize memory

Restart:	move.l	Stack_base,sp		; Reset stack
	jsr	Reset_memory		; Reset memory
	jsr	Reset_Font_stack		; Reset stacks
	jsr	Reset_PA_stack
	jsr	Reset_MA_stack
	jsr	Reset_Mptr_stack
	jsr	Reset_mouse
	jsr	Reset_CA_stack
	jsr	Reset_root_stack
	jsr	Reset_module_stack
	jsr	Reset_keyboard		; Reset input buffers
	jsr	Reset_mouse_buffer
	jsr	Clear_all_HDOBs		; Reset HDOBs

	move.l	Off_screen,a0		; Clear screens
	jsr	Clear_screen
	jsr	Update_screen
;	jsr	Fade_in_all		; Must be here !!!

	jsr	Init_program		; Initialize program

Main_loop:	jsr	Update_display		; Main loop
	jsr	Handle_input
	jsr	Switch_screens
	bra.s	Main_loop

Exit_program:
	move.l	Stack_base,sp		; Reset stack
	jsr	Exit_memory		; Restore
	jsr	Exit_computer
	move.l	Return_value,d0		; Return
	move.l	Return_address,a0
	jmp	(a0)

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
;	move.l	d0,a1			; Increase priority
;	moveq.l	#16,d0
;	kickEXEC	SetTaskPri
	moveq.l	#INTB_COPER,d0		; Install Copper interrupt
	lea.l	My_Copper_interrupt,a1
	kickEXEC	AddIntServer
	st	Vbl_installed
	jsr	Init_mouse		; Initialize Core
	jsr	Init_screens
	jsr	Init_input
	jsr	Init_files
.Exit:	movem.l	(sp)+,d0-d7/a0-a6
	rts

;***************************************************************************
; [ Restore the computer state ]
; All registers are restored
;***************************************************************************
Exit_computer:
	movem.l	d0-d7/a0-a6,-(sp)
	Wait_4_blitter
	jsr	End_graphics_ops
	jsr	Exit_files		; Exit Core
	jsr	Exit_input
	jsr	Exit_screens
	jsr	Exit_mouse
	tst.b	Vbl_installed		; Remove Copper interrupt
	beq	.Skip
	moveq.l	#INTB_COPER,d0
	lea.l	My_Copper_interrupt,a1
	kickEXEC	RemIntServer
.Skip:	jsr	Close_libraries
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
.Done:	cmp.w	#20,d1			; Too low ?
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
	moveq.l	#39,d0			; Open DOS library
	lea.l	DOS_library_name,a1
	kickEXEC	OpenLibrary
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#DOS_LIBRARY_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,DOS_base
	LOCAL
	moveq.l	#39,d0			; Open Graphics library
	lea.l	Graphics_library_name,a1
	kickEXEC	OpenLibrary
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#GRAPHICS_LIBRARY_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,Graphics_base
	LOCAL
	moveq.l	#33,d0			; Open Intuition library
	lea.l	Intuition_library_name,a1
	kickEXEC	OpenLibrary
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#INTUITION_LIBRARY_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,Intuition_base
	LOCAL
	moveq.l	#33,d0			; Open Asl library
	lea.l	Asl_library_name,a1
	kickEXEC	OpenLibrary
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#ASL_LIBRARY_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,Asl_base
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
	move.l	Asl_base,d0		; Asl library open ?
	beq.s	.No
	move.l	d0,a1			; Yes -> Close
	kickEXEC	CloseLibrary
	clr.l	Asl_base
.No:	LOCAL
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************
; [ Copper interrupt handler ]
; All registers are restored
; Notes :
;   - There are TWO copper interrupts per frame !
;***************************************************************************
My_Copper_handler:
	not.b	Interrupt_toggle
	beq.s	.Two
	addq.l	#1,Top_counter
	bra.s	.Exit
.Two:	movem.l	d0-d7/a0-a6,-(sp)
	addq.l	#1,VBL_counter		; Count
	jsr	Update_mouse		; Update mouse
	jsr	Random			; Randomize
	jsr	VblQ_handler		; Do VBL queue
	movem.l	(sp)+,d0-d7/a0-a6
.Exit:	rts

;*****************************************************************************
; [ Set data of a tag in a tag list ]
;   IN : d0 - Tag type (.l)
;        d1 - Tag data (.l)
;        a0 - Pointer to tag list (.l)
; All registers are restored
; Notes :
;   - No funny tag list tricks (like TAG_MORE, TAG_SKIP, etc.) are handled.
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
; [ Handle fatal error ]
;   IN : d0 - Error code (.l)
; No registers are restored
;*****************************************************************************
Fatal_error:
	neg.l	d0
	subq.l	#1,d0
	move.l	d0,Return_value
	jmp	Exit_program

;*****************************************************************************
; [ Clear all CPU caches ]
; All registers are restored
;*****************************************************************************
Clear_cache:
	movem.l	d0/d1/a0/a1,-(sp)
	kickEXEC	CacheClearU
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************
; [ Wait for the vertical blank ]
; All registers are restored
;***************************************************************************
My_vsync:
	move.l	d0,-(sp)
	move.l	VBL_counter,d0
.Wait:	cmp.l	VBL_counter,d0
	beq.s	.Wait
	move.l	(sp)+,d0
	rts	

;*****************************************************************************
; [ Dummy routine ]
; All registers are restored
;*****************************************************************************
Dummy:
	rts

;*****************************************************************************
; [ Filerequester ]
;   IN : a0 - Pointer to title text (.l)
;        a1 - Pointer to file pattern (.l)
;  OUT : a0 - Pointer to filename / 0 (No file) (.l)
; Changed registers : a0
;*****************************************************************************
File_requester:
	movem.l	d0/d1/d7/a1/a2,-(sp)
	moveq.l	#0,d7			; Default is no file
	move.l	a0,d1			; Insert title text
	lea.l	File_requester_tags,a0
	move.l	#ASLFR_TitleText,d0
	jsr	Set_tag_data
	move.l	#ASLFR_InitialPattern,d0	; Insert file pattern
	move.l	a1,d1
	jsr	Set_tag_data
	move.l	#ASL_FileRequest,d0		; Allocate requester
	kickASL	AllocAslRequest
	move.l	d0,My_requester
	move.l	d0,a0			; Do
	sub.l	a1,a1
	kickASL	AslRequest
	tst.l	d0			; Cancelled ?
	beq.s	.Exit
	move.l	My_requester,a2		; No
	lea.l	Full_name,a0		; Make full filename
	move.l	fr_Drawer(a2),a1
	jsr	Strcpy
	move.l	#Full_name,d1
	move.l	fr_File(a2),d2
	move.l	#100,d3
	kickDOS	AddPart
	tst.l	d0			; Success ?
	beq.s	.Exit
	move.l	#Full_name,d7		; Yes
.Exit:	move.l	My_requester,a0		; Free requester
	kickASL	FreeAslRequest
	move.l	d7,a0			; Output
	movem.l	(sp)+,d0/d1/d7/a1/a2
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************
	SECTION	Fast_DATA,data
Empty_tags:
	dc.l TAG_DONE
File_requester_tags:
	dc.l ASLFR_TitleText,0		; Will be inserted
	dc.l ASLFR_InitialLeftEdge,20
	dc.l ASLFR_InitialTopEdge,20
	dc.l ASLFR_InitialHeight,300
	dc.l ASLFR_InitialWidth,160
	dc.l ASLFR_InitialPattern,0		; Will be inserted
	dc.l ASLFR_PositiveText,.Ok
	dc.l ASLFR_NegativeText,.Cancel
	dc.l ASLFR_PrivateIDCMP,TRUE
	dc.l TAG_DONE

.Ok:	dc.b "Ok",0
.Cancel:	dc.b "Cancel",0
	even

My_Copper_interrupt:
	dc.l 0,0
	dc.b NT_INTERRUPT
	dc.b 0				; Priority = maximum
	dc.l DDT_name
	dc.l 0
	dc.l My_Copper_handler

DOS_library_name:	dc.b "dos.library",0
Graphics_library_name:	dc.b "graphics.library",0
Intuition_library_name:	dc.b "intuition.library",0
Asl_library_name:	dc.b "asl.library",0
	even


	SECTION	Fast_BSS,bss
Vbl_installed:	ds.b 1
Blitter_or_CPU:	ds.b 1
Interrupt_toggle:	ds.b 1
	even
Stack_base:	ds.l 1
Interface_ptr:	ds.l 1
Return_address:	ds.l 1
Return_value:	ds.l 1
Extended_error_code:	ds.l 2
Kickstart_version:	ds.w 1
Processor_type:	ds.w 1
VBLs_per_second:	ds.w 1
VBL_counter:	ds.l 1
Top_counter:	ds.l 1
My_task_ptr:	ds.l 1

My_requester:	ds.l 1
Full_name:	ds.b 100
	even

DOS_base:	ds.l 1
Graphics_base:	ds.l 1
Intuition_base:	ds.l 1
Asl_base:	ds.l 1
