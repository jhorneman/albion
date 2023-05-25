; Screen management
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	MODULE	DDT_Screen

	OUTPUT	DDT:Objects/Core/Screen.o

	OPT	O1+,O2+,O4+,O5+,O10+,O11+,OW-
	OPT	O7+,EVEN,USER,INCONCE
	OPT	P=68020
	OPT	DEBUG


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
	XDEF	Update_copper_lists
	XDEF	Update_screen
	XDEF	Copy_screen
	XDEF	Clear_screen
	XDEF	Switch_screens
	XDEF	Work_screen
	XDEF	On_screen
	XDEF	Off_screen
	XDEF	Screen_flag
	XDEF	My_viewport


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
; ---------- Initialize View ----------------------
	move.l	Graphics_base,a0		; Save view
	move.l	gb_ActiView(a0),Old_view
	lea.l	My_view,a1		; Initialize view
	kickGFX	InitView
; ---------- Initialize View Extra ----------------
	move.l	#VIEW_EXTRA_TYPE,d0		; Create view extra
	kickGFX	GfxNew
	move.l	d0,My_view_extra
	lea.l	My_view,a0		; Link to view
	move.l	d0,a1
	kickGFX	GfxAssociate
	lea.l	My_view,a0
	move.w	v_Modes(a0),d0
	or.w	#EXTEND_VSTRUCT,d0
	move.w	d0,v_Modes(a0)
	sub.l	a1,a1			; Open monitor
	move.l	#DEFAULT_MONITOR_ID|LORES_KEY,d0
	kickGFX	OpenMonitor
	move.l	d0,My_monspec
	move.l	My_view_extra,a0		; Insert in view extra
	move.l	d0,ve_Monitor(a0)
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
	move.l	#Screen_width,d0		; Allocate second bitmap
	move.l	#Screen_height,d1
	move.l	#Screen_depth,d2
	move.l	#BMF_CLEAR|BMF_DISPLAYABLE|BMF_INTERLEAVED,d3
	kickGFX	AllocBitMap
	move.l	d0,Bitmap_2
	move.l	d0,a0			; Set pointers
	move.l	bm_Planes(a0),Off_screen
	move.l	bm_Planes(a0),Work_screen
; ---------- Initialize ViewPort ------------------
	lea.l	My_viewport,a0
	kickGFX	InitVPort
	lea.l	My_viewport,a0
	move.l	#My_rasinfo,vp_RasInfo(a0)
	move.w	#Screen_width,vp_DWidth(a0)
	move.w	#Screen_height,vp_DHeight(a0)
	lea.l	My_view,a1		; Insert viewport in view
	move.l	a0,v_ViewPort(a1)
; ---------- Initialize ViewPort Extra ------------
	move.l	#VIEWPORT_EXTRA_TYPE,d0	; Create viewport extra
	kickGFX	GfxNew
	move.l	d0,My_viewport_extra
	lea.l	Video_control_tag_items,a0	; Insert in tag list
	move.l	d0,d1
	move.l	#VTAG_VIEWPORTEXTRA_SET,d0
	jsr	Set_tag_data
	sub.l	a0,a0			; Get DisplayClip
	lea.l	My_dim,a1
	move.l	#dim_data_size,d0
	move.l	#DTAG_DIMS,d1
	move.l	#DEFAULT_MONITOR_ID|LORES_KEY,d2
	kickGFX	GetDisplayInfoData
	lea.l	My_dim+dim_Nominal,a0	; Copy to viewport extra
	move.l	My_viewport_extra,a1
	lea.l	vpe_DisplayClip(a1),a1
	moveq.l	#ra_data_size/2-1,d0
.Loop:	move.w	(a0)+,(a1)+
	dbra	d0,.Loop
	move.l	#DEFAULT_MONITOR_ID|LORES_KEY,d0	; Find display info
	kickGFX	FindDisplayInfo
	lea.l	Video_control_tag_items,a0	; Insert in tag list
	move.l	d0,d1
	move.l	#VTAG_NORMAL_DISP_SET,d0
	jsr	Set_tag_data
; ---------- Initialize ColorMap ------------------
	moveq.l	#1,d0			; Get colormap
	kickGFX	GetColorMap
	move.l	d0,My_colormap
	lea.l	Video_control_tag_items,a0	; Insert in tag list
	move.l	d0,d1
	move.l	#VTAG_ATTACH_CM_SET,d0
	jsr	Set_tag_data
	move.l	My_colormap,a0		; Attach colormap
	lea.l	Video_control_tag_items,a1
	kickGFX	VideoControl
; ---------- Initialize double-buffering ----------
	jsr	Update_copper_lists		; Get copper lists
	movem.l	(sp)+,d0-d3/a0/a1
	rts

;***************************************************************************
; [ Free the screens ]
; All registers are restored
;***************************************************************************
Exit_screens:
	movem.l	d0/d1/a0/a1,-(sp)
	Wait_4_blitter
	move.l	Old_view,a1		; Restore view
	kickGFX	LoadView
	kickGFX	WaitTOF
	move.l	Copper_1,a0		; Free copper lists
	kickGFX	FreeCprList
	move.l	Copper_2,a0
	kickGFX	FreeCprList
	lea.l	My_viewport,a0
	kickGFX	FreeVPortCopLists
	move.l	My_viewport_extra,a0	; Free viewport extra
	kickGFX	GfxFree
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
	move.l	My_monspec,a0		; Close monitor
	kickGFX	CloseMonitor
	move.l	My_view_extra,a0		; Destroy view extra
	kickGFX	GfxFree
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************
; [ Update copper lists ]
; All registers are restored
;***************************************************************************
Update_copper_lists:
	movem.l	d0/d1/a0-a2,-(sp)
	lea.l	My_rasinfo,a0		; First bitmap
	move.l	Bitmap_1,ri_BitMap(a0)
	lea.l	My_view,a0
	lea.l	My_viewport,a1
	kickGFX	MakeVPort
	lea.l	My_view,a1
	kickGFX	MrgCop
	lea.l	My_view,a0
	move.l	v_LOFCprList(a0),Copper_1
	clr.l	v_LOFCprList(a0)
	lea.l	My_rasinfo,a0		; Second bitmap
	move.l	Bitmap_2,ri_BitMap(a0)
	lea.l	My_view,a0
	lea.l	My_viewport,a1
	kickGFX	MakeVPort
	lea.l	My_view,a1
	kickGFX	MrgCop
	move.l	My_view+v_LOFCprList,Copper_2
	movem.l	(sp)+,d0/d1/a0-a2
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
	jsr	Draw_HDOBs		; Draw
	Wait_4_blitter			; Wait for blitter
	kickGFX	WaitTOF			; Wait for top of frame
	tst.b	Screen_flag		; Select screen
	bne.s	.Two
	move.l	Copper_1,a0
	bra.s	.Do
.Two:	move.l	Copper_2,a0
.Do:	lea.l	My_view,a1		; Insert in View
	move.l	a0,v_LOFCprList(a1)
	kickGFX	LoadView			; Activate
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
	jsr	Draw_HDOBs		; Draw
	Wait_4_blitter			; Wait for blitter
	kickGFX	WaitTOF			; Wait for top of frame
	tst.b	Screen_flag		; Select screen
	bne.s	.Two
	move.l	Copper_1,a0
	bra.s	.Do
.Two:	move.l	Copper_2,a0
.Do:	lea.l	My_view,a1		; Insert in View
	move.l	a0,v_LOFCprList(a1)
	kickGFX	LoadView			; Activate
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

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************
	SECTION	Fast_DATA,data
Video_control_tag_items:
	dc.l VTAG_SPODD_BASE_SET,Mptr_colour_base
	dc.l VTAG_SPEVEN_BASE_SET,Mptr_colour_base
	dc.l VTAG_SPRITERESN_SET,SPRITERESN_ECS
	dc.l VTAG_ATTACH_CM_SET,0
	dc.l VTAG_VIEWPORTEXTRA_SET,0
	dc.l VTAG_NORMAL_DISP_SET,0
	dc.l TAG_DONE,0


	SECTION	Fast_BSS,bss
Screen_flag:	ds.b 1
	even
Old_view:	ds.l 1

On_screen:	ds.l 1
Off_screen:	ds.l 1
Work_screen:	ds.l 1

My_colormap:	ds.l 1
My_viewport_extra:	ds.l 1
My_viewport:	ds.b vp_data_size
My_view_extra:	ds.l 1
My_view:	ds.b v_data_size
My_rasinfo:	ds.l ri_data_size
My_monspec:	ds.l 1
My_dim:	ds.b dim_data_size

Bitmap_1:	ds.l 1
Bitmap_2:	ds.l 1
Copper_1:	ds.l 1
Copper_2:	ds.l 1
