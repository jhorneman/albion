; Screen management
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	XDEF	Update_screen
	XDEF	Copy_screen
	XDEF	Clear_screen
	XDEF	Switch_screens
	XDEF	Work_screen
	XDEF	On_screen
	XDEF	Off_screen
	XDEF	Screen_flag

	SECTION	Program,code
;***************************************************************************
; [ Initialize the screens ]
; All registers are restored
;***************************************************************************
Init_screens:
	movem.l	d0-d3/d6/d7/a0/a1,-(sp)
; ---------- Find best mode ID --------------------
	lea.l	BestModeID_tag_items,a0	; Find
	kickGFX	BestModeIDA
	cmp.l	#-1,d0			; Found ?
	bne.s	.Ok
	move.l	#MODE_ID_ERROR,d0		; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,My_mode_ID		; Store
	LOCAL
	lea.l	Open_screen_tag_items,a0	; Insert mode ID
	move.l	d0,d1
	move.l	#SA_DisplayID,d0
	jsr	Set_tag_data
; ---------- Prepare overscan ---------------------
	move.l	d1,a0			; Query overscan
	lea.l	Overscan,a1
	move.l	#OSCAN_MAX,d0
	kickINTU	QueryOverscan
	lea.l	Open_screen_tag_items,a0	; Insert left & top of
	lea.l	Overscan,a1		;  screen
	move.l	#SA_Left,d0
	moveq.l	#0,d1
	move.w	ra_MaxX(a1),d1
	sub.w	ra_MinX(a1),d1
	sub.w	#Screen_width,d1
	asr.w	#1,d1
	jsr	Set_tag_data
	move.l	#SA_Top,d0
	move.w	ra_MaxY(a1),d1
	sub.w	ra_MinY(a1),d1
	sub.w	#Screen_height,d1
	asr.w	#1,d1
	jsr	Set_tag_data
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
	jmp	Fatal_error
.Ok1:	move.l	d0,Bitmap_1		; Yes
	move.l	d0,a0			; Is it interleaved ?
	move.l	bm_Planes+4(a0),d0
	sub.l	bm_Planes(a0),d0
	cmp.l	#Bytes_per_plane,d0
	beq.s	.Ok2
	move.l	#INTERLEAVE_ERROR,d0	; No -> Exit
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
	jmp	Fatal_error
.Ok1:	move.l	d0,Bitmap_2		; Yes
	move.l	d0,a0			; Is it interleaved ?
	move.l	bm_Planes+4(a0),d0
	sub.l	bm_Planes(a0),d0
	cmp.l	#Bytes_per_plane,d0
	beq.s	.Ok2
	move.l	#INTERLEAVE_ERROR,d0	; No -> Exit
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
	move.l	My_window,a0		; Set dummy Intuition pointer
	lea.l	Dummy_sprite,a1
	moveq.l	#1,d0
	moveq.l	#16,d1
	moveq.l	#0,d2
	moveq.l	#0,d3
	kickINTU	SetPointer
	LOCAL
; ---------- Make copper list ---------------------
	move.l	#ucl_data_size,d0		; Make UCopList
	move.l	#MEMF_PUBLIC|MEMF_CLEAR,d1
	kickEXEC	AllocMem
	move.l	d0,My_UCopList
	move.l	d0,a6
	move.l	d0,a0			; Initialize
	moveq.l	#5,d0
	kickGFX	UCopperListInit
	move.l	a6,a1			; Wait for start of screen
	moveq.l	#0,d0
	moveq.l	#0,d1
	kickGFX	CWait
	move.l	a6,a1
	kickGFX	CBump
	move.l	a6,a1			; Cause copper interrupt
	move.w	#intreq,d0
	move.w	#$8010,d1
	kickGFX	CMove
	move.l	a6,a1
	kickGFX	CBump
	move.l	a6,a1			; Wait for end of screen
	move.w	#Screen_height,d0
	moveq.l	#0,d1
	kickGFX	CWait
	move.l	a6,a1
	kickGFX	CBump
	move.l	a6,a1			; Cause copper interrupt
	move.w	#intreq,d0
	move.w	#$8010,d1
	kickGFX	CMove
	move.l	a6,a1
	kickGFX	CBump
	move.l	a6,a1			; End copper list
	move.w	#10000,d0
	move.w	#255,d1
	kickGFX	CWait
	kickEXEC	Forbid			; Insert in viewport
	move.l	My_screen,a0
	lea.l	sc_ViewPort(a0),a0
	move.l	My_UCopList,vp_UCopIns(a0)
	kickEXEC	Permit
	kickINTU	RethinkDisplay		; Yo-ho
	LOCAL
; ---------- Initialize double-buffering ----------
	move.l	My_screen,a0		; Allocate screen buffer
	move.l	Bitmap_1,a1
	moveq.l	#0,d0
	kickINTU	AllocScreenBuffer
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#ALLOC_SCREEN_BUFFER_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,My_screen_buffer1
	LOCAL
	move.l	My_screen,a0		; Allocate screen buffer
	move.l	Bitmap_2,a1
	moveq.l	#0,d0
	kickINTU	AllocScreenBuffer
	tst.l	d0			; Success ?
	bne.s	.Ok
	move.l	#ALLOC_SCREEN_BUFFER_ERROR,d0	; No -> Exit
	jmp	Fatal_error
.Ok:	move.l	d0,My_screen_buffer2
; ---------- Set screen pointers ------------------
	move.l	Bitmap_1,a0		; Get first screen pointer
	move.l	bm_Planes(a0),On_screen
	move.l	Bitmap_2,a0		; Get second screen pointer
	move.l	bm_Planes(a0),Off_screen
	move.l	Off_screen,Work_screen
	movem.l	(sp)+,d0-d3/d6/d7/a0/a1
	rts

;***************************************************************************
; [ Free the screens ]
; All registers are restored
;***************************************************************************
Exit_screens:
	movem.l	d0/d1/a0/a1,-(sp)
	Wait_4_blitter
	move.l	My_screen_buffer1,d0	; Screen buffer made ?
	beq.s	.No
	move.l	My_screen,a0		; Yes -> Close it
	move.l	d0,a1
	kickINTU	FreeScreenBuffer
	clr.l	My_screen_buffer1
.No:	LOCAL
	move.l	My_screen_buffer2,d0	; Screen buffer made ?
	beq.s	.No
	move.l	My_screen,a0		; Yes -> Close it
	move.l	d0,a1
	kickINTU	FreeScreenBuffer
	clr.l	My_screen_buffer2
.No:	LOCAL
	move.l	My_screen,d0		; Screen open ?
	beq.s	.No
	move.l	d0,a0			; Yes -> Remove copper list
	lea.l	sc_ViewPort(a0),a0
	kickGFX	FreeVPortCopLists
	kickINTU	RemakeDisplay
.No:	LOCAL
	move.l	My_window,d0		; Window open ?
	beq.s	.No
	move.l	d0,a0			; Yes -> Restore Intuition pointer
	kickINTU	ClearPointer
	move.l	My_window,a0		; Close it
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
	movem.l	d0/d1/d7/a0-a2,-(sp)
	jsr	Draw_HDOBs		; Draw
	jsr	My_vsync			; Wait
	Wait_4_blitter
	tst.b	Screen_flag		; Select new screen buffer
	beq.s	.Two
	move.l	My_screen_buffer1,a1
	bra.s	.Do
.Two:	move.l	My_screen_buffer2,a1
.Do:	move.l	My_screen,a0		; Switch
	kickINTU	ChangeScreenBuffer
	move.l	Top_counter,d0
.Wait:	cmp.l	Top_counter,d0
	beq.s	.Wait
	move.l	Off_screen,a0		; Switch on- & off-screen
	move.l	On_screen,Off_screen
	move.l	a0,On_screen
	move.l	Off_screen,Work_screen	; Set work screen
	not.b	Screen_flag		; Toggle
	jsr	Erase_HDOBs		; Erase
	jsr	Copy_screen		: Copy
	jsr	Handle_busy_pointer
	jsr	ScrQ_handler		; Do screen queue 
	movem.l	(sp)+,d0/d1/d7/a0-a2
	rts

;***************************************************************************
; [ Switch screens ]
; All registers are restored
;***************************************************************************
Switch_screens:
	movem.l	d0/d1/d7/a0-a2,-(sp)
	jsr	Draw_HDOBs		; Draw
	jsr	My_vsync			; Wait
	Wait_4_blitter
	tst.b	Screen_flag		; Select new screen buffer
	beq.s	.Two
	move.l	My_screen_buffer1,a1
	bra.s	.Do
.Two:	move.l	My_screen_buffer2,a1
.Do:	move.l	My_screen,a0		; Switch
	kickINTU	ChangeScreenBuffer
	move.l	Top_counter,d0
.Wait:	cmp.l	Top_counter,d0
	beq.s	.Wait
	move.l	Off_screen,a0		; Switch on- & off-screen
	move.l	On_screen,Off_screen
	move.l	a0,On_screen
	move.l	Off_screen,Work_screen	; Set work screen
	not.b	Screen_flag		; Toggle
	jsr	Erase_HDOBs		; Erase
	jsr	Handle_busy_pointer
	jsr	ScrQ_handler		; Do screen queue 
	movem.l	(sp)+,d0/d1/d7/a0-a2
	rts

;***************************************************************************
; [ Copy the on- to the off-screen ]
; All registers are restored
; Notes :
;   - Always call [ My_vsync ] before calling this routine.
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
BestModeID_tag_items:
	dc.l BIDTAG_DIPFMustHave,DIPF_IS_PAL|DIPF_IS_DBUFFER|DIPF_IS_SPRITES|DIPF_IS_SPRITES_ATT|DIPF_IS_SPRITES_CHNG_RES|DIPF_IS_SPRITES_CHNG_BASE
	dc.l BIDTAG_DIPFMustNotHave,SPECIAL_FLAGS|DIPF_IS_LACE|DIPF_IS_PROGBEAM
	dc.l BIDTAG_NominalWidth,320		; !!
	dc.l BIDTAG_NominalHeight,200	; !!
	dc.l BIDTAG_Depth,Screen_depth
	dc.l BIDTAG_RedBits,8
	dc.l BIDTAG_BlueBits,8
	dc.l BIDTAG_GreenBits,8
	dc.l TAG_DONE,0

Open_screen_tag_items:
	dc.l SA_Left,0			; Will be inserted
	dc.l SA_Top,0			; Will be inserted
	dc.l SA_Width,Screen_width
	dc.l SA_Height,Screen_height
	dc.l SA_Depth,Screen_depth
	dc.l SA_BitMap,0			; Will be inserted
	dc.l SA_ShowTitle,FALSE
	dc.l SA_Quiet,TRUE
	dc.l SA_DisplayID,0		; Will be inserted
	dc.l SA_Type,CUSTOMSCREEN|CUSTOMBITMAP
	dc.l SA_DClip,Overscan
	dc.l SA_ErrorCode,Extended_error_code
	dc.l SA_Draggable,FALSE
	dc.l SA_Exclusive,TRUE
	dc.l SA_Interleaved,TRUE
	dc.l SA_VideoControl,Open_screen_video_control_tag_items
	dc.l TAG_DONE,0

Open_screen_video_control_tag_items:
	dc.l VTAG_SPRITERESN_SET,SPRITERESN_140NS
	dc.l VTAG_BORDERSPRITE_SET,TRUE
	dc.l TAG_DONE,0

Open_window_tag_items:
	dc.l WA_Left,0
	dc.l WA_Top,0
	dc.l WA_Width,Screen_width
	dc.l WA_Height,Screen_height
	dc.l WA_Flags,WFLG_ACTIVATE|WFLG_BACKDROP|WFLG_BORDERLESS|WFLG_RMBTRAP
	dc.l WA_IDCMP,0
	dc.l WA_CustomScreen,0		; Will be inserted
	dc.l WA_AutoAdjust,FALSE
	dc.l WA_ReportMouse,TRUE
	dc.l TAG_DONE,0


	SECTION	Chip_BSS,bss_c
Dummy_sprite:
	ds.l 3


	SECTION	Fast_BSS,bss
Screen_flag:	ds.b 1
	even
On_screen:	ds.l 1
Off_screen:	ds.l 1
Work_screen:	ds.l 1

My_screen:	ds.l 1
My_window:	ds.l 1
My_UCopList:	ds.l 1
My_screen_buffer1:	ds.l 1
My_screen_buffer2:	ds.l 1
My_mode_ID:	ds.l 1
Bitmap_1:	ds.l 1
Bitmap_2:	ds.l 1
Overscan:	ds.b ra_data_size
