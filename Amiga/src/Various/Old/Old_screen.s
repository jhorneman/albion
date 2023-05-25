
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


	move.l	My_screen,d0		; Screen open ?
	beq.s	.No
	move.l	d0,a0			; Yes -> Close it
	kickINTU	CloseScreen
	clr.l	My_screen
.No:	LOCAL



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

;	dc.l WA_CustomScreen,0


My_screen:	ds.l 1
