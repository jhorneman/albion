; Screen management
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	MODULE	DDT_Screen

	OUTPUT	DDT:Objects/Core/Screen.o

	OPT	O1+,O2+,O4+,O5+,O10+,O11+,OW-
	OPT	O7+,EVEN,USER,INCONCE
	OPT	P=68020
	OPT	DEBUG


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
	move.l	d0,Bitmap_1
	move.l	d0,a0			; Set pointer
	move.l	bm_Planes(a0),On_screen
	move.l	#Screen_size,d0		; Get second bitmap
	move.l	#MEMF_FAST,d1
	kickEXEC	AllocMem			; Get it
	move.l	d0,Screen_2
	move.l	d0,Off_screen		; Set pointers
	move.l	d0,Work_screen
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
	movem.l	(sp)+,d0-d3/a0/a1
	rts

;***************************************************************************
; [ Free the screens ]
; All registers are restored
;***************************************************************************
Exit_screens:
	movem.l	d0/d1/a0/a1,-(sp)
	Wait_4_blitter
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
	move.l	Screen_2,d0		; Bitmap allocated ?
	beq.s	.No
	move.l	d0,a1			; Yes -> Free it
	move.l	#Screen_size,d0
	kickEXEC	FreeMem
	clr.l	Screen_2
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
; [ Switch screens ]
; All registers are restored
;***************************************************************************
Copy_screen:
Update_screen:
Switch_screens:
	movem.l	d0/d1/d7/a0/a1,-(sp)
;	move.l	My_screen,a0		; Wait for end of screen
;	lea.l	sc_ViewPort(a0),a0
;	kickGFX	WaitBOVP

	kickGFX	WaitTOF

	XX	$0f0

	move.l	Off_screen,a0		; Copy
	move.l	On_screen,a1
	move.w	#Screen_size/32-1,d7
.Loop:
	rept 8
	move.l	(a0)+,(a1)+
	endr
	dbra	d7,.Loop

	XX	$f00

	movem.l	(sp)+,d0/d1/d7/a0/a1
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************
	SECTION	Fast_DATA,data
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
	dc.l SA_DisplayID,PAL_MONITOR_ID|LORES_KEY
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
Screen_flag:	ds.b 1
	even

On_screen:	ds.l 1
Off_screen:	ds.l 1
Work_screen:	ds.l 1

My_screen:	ds.l 1
My_window:	ds.l 1
Bitmap_1:	ds.l 1
Screen_2:	ds.l 1
