; Screen management
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	MODULE	DDT_Screen

	OUTPUT	DDT:Objects/Core/Screen.o


	XREF	Create_port
	XREF	Destroy_port
	XREF	Graphics_base
	XREF	Intuition_base
	XREF	Kickstart_version
	XREF	Set_tag_data
	XREF	Fatal_error
	XREF	Extended_error_code
	XREF	Draw_HDOBs
	XREF	Erase_HDOBs
	XREF	ScrQ_handler

	XDEF	Init_screens
	XDEF	Exit_screens
	XDEF	Update_screen
	XDEF	Copy_screen
	XDEF	Clear_screen
	XDEF	Switch_screens
	XDEF	Work_screen
	XDEF	On_screen
	XDEF	Off_screen
	XDEF	Screen_flag
	XDEF	My_screen
	XDEF	My_window


	incdir	DDT:
	include	Constants/Global.i
	include	Constants/OS/OS.i
	include	Constants/Core.i
	include	Constants/Hardware_registers.i


	SECTION	Program,code
;***************************************************************************
; [ Initialize the screens ]
; All registers are restored
;***************************************************************************
Init_screens:
	movem.l	d0-d3/a0/a1,-(sp)
; ---------- Allocate bitmaps for screens ---------
	move.l	#Screen_width,d0		; Allocate first bitmap
	move.l	#Screen_height,d1
	move.l	#Screen_depth,d2
	move.l	#BMF_CLEAR|BMF_DISPLAYABLE|BMF_INTERLEAVED,d3
	sub.l	a0,a0
	kickGFX	AllocBitMap
	tst.l	d0			; Success ?
	bne.s	.Ok1
	move.l	#ALLOC_BITMAP_ERROR,d0	; No -> Exit
	move.l	Extended_error_code,d1
	jmp	Fatal_error
.Ok1:	move.l	d0,Bitmap_1		; Yes
	move.l	d0,a0			; Is it interleaved ?
	move.l	bm_Planes+4(a0),d0
	sub.l	bm_Planes(a0),d0
	cmp.l	#Bytes_per_plane,d0
	beq.s	.Ok2
	move.l	#ALLOC_BITMAP_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok2:	LOCAL
	move.l	#Screen_width,d0		; Allocate second bitmap
	move.l	#Screen_height,d1
	move.l	#Screen_depth,d2
	move.l	#BMF_CLEAR|BMF_DISPLAYABLE|BMF_INTERLEAVED,d3
	kickGFX	AllocBitMap
	tst.l	d0			; Success ?
	bne.s	.Ok1
	move.l	#ALLOC_BITMAP_ERROR,d0	; No -> Exit
	move.l	Extended_error_code,d1
	jmp	Fatal_error
.Ok1:	move.l	d0,Bitmap_2		; Yes
	move.l	d0,a0			; Is it interleaved ?
	move.l	bm_Planes+4(a0),d0
	sub.l	bm_Planes(a0),d0
	cmp.l	#Bytes_per_plane,d0
	beq.s	.Ok2
	move.l	#ALLOC_BITMAP_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok2:	LOCAL
; ---------- Open screen & window -----------------
	lea.l	Open_screen_tag_items,a0	; Insert bitmap pointer
	move.l	#SA_BitMap,d0
	move.l	Bitmap_1,d1
	jsr	Set_tag_data
	sub.l	a0,a0			; Open screen
	lea.l	Open_screen_tag_items,a1
	kickINTU	OpenScreenTagList
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#OPEN_SCREEN_ERROR,d0	; No -> Exit
	move.l	Extended_error_code,d1
	jmp	Fatal_error
.Ok:	move.l	d0,My_screen

	move.l	My_screen,a0		; Get first screen pointer
	lea.l	sc_RastPort(a0),a0
	move.l	rp_BitMap(a0),a0
	move.l	a0,Bitmap_1

	LOCAL
	lea.l	Open_window_tag_items,a0	; Insert screen pointer
	move.l	#WA_CustomScreen,d0
	move.l	My_screen,d1
	jsr	Set_tag_data
	sub.l	a0,a0			; Open window
	lea.l	Open_window_tag_items,a1
	kickINTU	OpenWindowTagList
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#OPEN_WINDOW_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,My_window
	LOCAL
; ---------- Initialize double-buffering ----------
	move.l	My_screen,a0		; Allocate double-buffering info
	lea.l	sc_ViewPort(a0),a0
	kickGFX	AllocDBufInfo
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#CREATE_DBI_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,My_DBI
	LOCAL
	jsr	Create_port		; Create ports
	move.l	d0,DBI_port_1
	jsr	Create_port
	move.l	d0,DBI_port_2
	move.l	My_DBI,a0			; Initialize DBI-info
	move.l	DBI_port_1,dbi_SafeMessage+mn_ReplyPort(a0)
	move.l	DBI_port_2,dbi_DispMessage+mn_ReplyPort(a0)
	st	DBI_initialized		; Success
; ---------- Set screen pointers ------------------
	move.l	Bitmap_1,a0
	move.l	bm_Planes(a0),On_screen
	move.l	Bitmap_2,a0
	move.l	bm_Planes(a0),Off_screen
	move.l	Off_screen,Work_screen
	movem.l	(sp)+,d0-d3/a0/a1
	rts

;***************************************************************************
; [ Free the screens ]
; All registers are restored
;***************************************************************************
Exit_screens:
	movem.l	d0/d1/a0/a1,-(sp)
	Wait_4_blitter
; ---------- Destroy DBI --------------------------
	tst.b	DBI_initialized		; DBI initialized ?
	beq.s	.No
	jsr	Wait_4_change		; Yes -> Clean messages up
	jsr	Wait_4_write
	sf	DBI_initialized
.No:	LOCAL
	move.l	My_DBI,d0			; DBI allocated ?
	beq.s	.No
	move.l	d0,a1			; Yes -> Free it
	kickGFX	FreeDBufInfo
	clr.l	My_DBI
.No:	LOCAL
	move.l	DBI_port_1,a0		; Destroy ports
	jsr	Destroy_port
	clr.l	DBI_port_1
	move.l	DBI_port_2,a0
	jsr	Destroy_port
	clr.l	DBI_port_2
	LOCAL
; ---------- Close window & screen ----------------
	move.l	My_window,d0		; Window open ?
	beq.s	.No
	move.l	d0,a0			; Yes -> Close it
	kickINTU	CloseWindow
	clr.l	My_window
.No:	LOCAL
	move.l	My_screen,d0		; Screen open ?
	beq.s	.No
	move.l	d0,a0			; Yes -> Close it
	kickINTU	CloseScreen
	clr.l	My_screen
.No:	LOCAL
	move.l	Bitmap_1,d0		; Bitmap allocated ?
	beq.s	.No
	move.l	d0,a0			; Yes -> Free it
	kickGFX	FreeBitMap
	clr.l	Bitmap_1
.No:	LOCAL
	move.l	Bitmap_2,d0		; Bitmap allocated ?
	beq.s	.No
	move.l	d0,a0			; Yes -> Free it
	kickGFX	FreeBitMap
	clr.l	Bitmap_2
.No:	LOCAL
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************
; [ Clear the screen ]
;   IN : a0 - Screen base (.l)
; All registers are restored
;***************************************************************************
Clear_screen:
	movem.l	d0/d7/a0,-(sp)
	Wait_4_blitter
	moveq.l	#0,d0
	move.w	#Screen_size/32-1,d7
.Loop:
	rept 8
	move.l	d0,(a0)+
	endr
	dbra	d7,.Loop
	movem.l	(sp)+,d0/d7/a0
	rts

;***************************************************************************
; [ Update screen ]
; All registers are restored
;***************************************************************************
Update_screen:
	movem.l	d0/d1/a0-a2,-(sp)
	jsr	Wait_4_change		; Wait until it's safe
	st	Safe_to_change
	jsr	Draw_HDOBs		; Draw
	Wait_4_blitter			; Wait for blitter
	tst.b	Screen_flag		; Get new bitmap
	bne.s	.Two
	move.l	Bitmap_2,a1
	bra.s	.Do
.Two:	move.l	Bitmap_1,a1
.Do:	move.l	My_screen,a0		; Change
	lea.l	sc_ViewPort(a0),a0
	move.l	My_DBI,a2
	kickGFX	ChangeVPBitMap
	sf	Safe_to_change		; No longer safe
	sf	Safe_to_write
	jsr	Wait_4_write		; Wait until it's safe
	st	Safe_to_write
	move.l	Off_screen,a0		; Switch on- & off-screen
	move.l	On_screen,Off_screen
	move.l	a0,On_screen
	move.l	Off_screen,Work_screen	; Set work screen
	not.b	Screen_flag		; Toggle
	jsr	Erase_HDOBs		; Erase
	jsr	Copy_screen		: Copy
	jsr	ScrQ_handler		; Do screen queue 
	movem.l	(sp)+,d0/d1/a0-a2
	rts

;***************************************************************************
; [ Switch screens ]
; All registers are restored
;***************************************************************************
Switch_screens:
	movem.l	d0/d1/a0-a2,-(sp)
	jsr	Wait_4_change		; Wait until it's safe
	st	Safe_to_change
	jsr	Draw_HDOBs		; Draw
	Wait_4_blitter			; Wait for blitter
	tst.b	Screen_flag		; Get new bitmap
	bne.s	.Two
	move.l	Bitmap_2,a1
	bra.s	.Do
.Two:	move.l	Bitmap_1,a1
.Do:	move.l	My_screen,a0		; Change
	lea.l	sc_ViewPort(a0),a0
	move.l	My_DBI,a2
	kickGFX	ChangeVPBitMap
	sf	Safe_to_change		; No longer safe
	sf	Safe_to_write
	jsr	Wait_4_write		; Wait until it's safe
	st	Safe_to_write
	move.l	Off_screen,a0		; Switch on- & off-screen
	move.l	On_screen,Off_screen
	move.l	a0,On_screen
	move.l	Off_screen,Work_screen	; Set work screen
	not.b	Screen_flag		; Toggle
	jsr	Erase_HDOBs		; Erase
	jsr	ScrQ_handler		; Do screen queue 
	movem.l	(sp)+,d0/d1/a0-a2
	rts

;***************************************************************************
; [ Copy the on- to the off-screen ]
; All registers are restored
;***************************************************************************
Copy_screen:
	movem.l	d7/a0/a1,-(sp)
	Wait_4_blitter
	move.l	On_screen,a0		; Copy
	move.l	Off_screen,a1
	move.w	#Screen_size/32-1,d7
.Loop:
	rept 8
	move.l	(a0)+,(a1)+
	endr
	dbra	d7,.Loop
	not.b	Screen_flag		; Adjust HDOB's
	jsr	Erase_HDOBs
	not.b	Screen_flag
	movem.l	(sp)+,d7/a0/a1
	rts

;*****************************************************************************
; [ Wait until it's safe to write to the screen bitmap ]
; All registers are restored
;*****************************************************************************
Wait_4_write:
	movem.l	d0/d1/a0/a1,-(sp)
	tst.b	Safe_to_write		; Is it safe to write ?
	bne.s	.Exit
.Again:	move.l	DBI_port_1,a0		; No -> Any message ?
	kickEXEC	GetMsg
	tst.l	d0
	bne.s	.Done
	move.l	DBI_port_1,a0		; No -> Wait
	kickEXEC	WaitPort
	bra.s	.Again
.Done:
;	move.l	d0,a1			; Reply to message
;	kickEXEC	ReplyMsg
.Exit:	movem.l	(sp)+,d0/d1/a0/a1
	rts

;*****************************************************************************
; [ Wait until it's safe to change the screen bitmap ]
; All registers are restored
;*****************************************************************************
Wait_4_change:
	movem.l	d0/d1/a0/a1,-(sp)
	tst.b	Safe_to_change		; Is it safe to change ?
	bne.s	.Exit
.Again:	move.l	DBI_port_2,a0		; No -> Any message ?
	kickEXEC	GetMsg
	tst.l	d0
	bne.s	.Done
	move.l	DBI_port_2,a0		; No -> Wait
	kickEXEC	WaitPort
	bra.s	.Again
.Done:
;	move.l	d0,a1			; Reply to message
;	kickEXEC	ReplyMsg
.Exit:	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************	
; The DATA & BSS segments
;***************************************************************************
	SECTION	Fast_DATA,data
Safe_to_write:	dc.b -1
Safe_to_change:	dc.b -1
	even
Open_screen_tag_items:
	dc.l SA_Left,0
	dc.l SA_Top,0
	dc.l SA_Width,Screen_width
	dc.l SA_Height,Screen_height
	dc.l SA_Depth,Screen_depth
	dc.l SA_BitMap,0
	dc.l SA_ShowTitle,FALSE
	dc.l SA_Quiet,TRUE
	dc.l SA_Type,CUSTOMSCREEN|CUSTOMBITMAP
;	dc.l SA_DisplayID,DEFAULT_MONITOR_ID|LORES_KEY
	dc.l SA_ErrorCode,Extended_error_code
	dc.l SA_Draggable,FALSE
	dc.l SA_Exclusive,TRUE
	dc.l SA_Interleaved,TRUE
	dc.l SA_VideoControl,Open_screen_video_control_tag_items
	dc.l TAG_DONE,0

Open_screen_video_control_tag_items:
	dc.l VTAG_SPODD_BASE_SET,Mptr_colour_base
	dc.l VTAG_SPEVEN_BASE_SET,Mptr_colour_base
	dc.l VTAG_SPRITERESN_SET,SPRITERESN_ECS
	dc.l TAG_DONE,0

Open_window_tag_items:
	dc.l WA_Left,0
	dc.l WA_Top,0
	dc.l WA_Width,Screen_width
	dc.l WA_Height,Screen_height
	dc.l WA_Flags,WFLG_ACTIVATE|WFLG_BACKDROP|WFLG_BORDERLESS|WFLG_RMBTRAP
	dc.l WA_IDCMP,0
	dc.l WA_CustomScreen,0
	dc.l WA_AutoAdjust,FALSE
	dc.l WA_ReportMouse,TRUE
	dc.l TAG_DONE,0


	SECTION	Fast_BSS,bss
DBI_initialized:	ds.b 1
Screen_flag:	ds.b 1
	even

On_screen:	ds.l 1
Off_screen:	ds.l 1
Work_screen:	ds.l 1

My_screen:	ds.l 1
My_window:	ds.l 1
Bitmap_1:	ds.l 1
Bitmap_2:	ds.l 1

My_DBI:	ds.l 1
DBI_port_1:	ds.l 1
DBI_port_2:	ds.l 1
