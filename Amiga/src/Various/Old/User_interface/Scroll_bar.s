; Scroll bar handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 25-1-1994

	MODULE	DDT_Scroll_bar

	OUTPUT	DDT:Objects/User_interface/Scroll_bar.o


	XREF	Plot_pixel
	XREF	Draw_hline
	XREF	Draw_vline
	XREF	Draw_box
	XREF	Draw_deep_box
	XREF	Mouse_Y
	XREF	Hide_HDOBs
	XREF	Show_HDOBs
	XREF	Update_screen
	XREF	Switch_screens
	XREF	Mouse_off
	XREF	Mouse_on
	XREF	Push_MA
	XREF	Pop_MA
	XREF	Button_state
	XREF	Wait_4_unclick
	XREF	Offset_X
	XREF	Offset_Y

	XDEF	Set_scroll_bar
	XDEF	Set_scroll_bar2
	XDEF	Get_scroll_bar
	XDEF	Add_scroll_bar
	XDEF	Remove_scroll_bar
	XDEF	Remove_all_scroll_bars
	XDEF	Scroll_bar_pressed


	incdir	DDT:
	include	Constants/Global.i
	include	Constants/Core.i
	include	Constants/Hull.i
	include	Constants/User_interface.i


	SECTION	Program,code
;***************************************************************************	
; [ Set scroll bar position ]
;   IN : d0 - Scroll bar Y position (.w)
;        a0 - Pointer to scroll bar data (.l)
; All registers are restored
;***************************************************************************
Set_scroll_bar:
	jmp	Update_slider

;***************************************************************************	
; [ Set scroll bar position 2 ]
;   IN : d0 - Scroll bar result (.w)
;        a0 - Pointer to scroll bar data (.l)
; All registers are restored
;***************************************************************************
Set_scroll_bar2:
	movem.l	d0/a0,-(sp)
	ext.l	d0			; Calculate row
	divu.w	Units_width(a0),d0
	mulu.w	Row_height(a0),d0		; Calculate Y
	cmp.w	Scroll_bar_max_height(a0),d0	; Too high ?
	bmi.s	.Ok
	move.w	Scroll_bar_max_height(a0),d0	; Yes -> clip
.Ok:	jsr	Update_slider		; Update
	movem.l	(sp)+,d0/a0
	rts

;***************************************************************************	
; [ Get scroll bar position ]
;   IN : a0 - Pointer to scroll bar data (.l)
;  OUT : d0 - Scroll bar Y-position (.w)
; All registers are restored
;***************************************************************************
Get_scroll_bar:
	move.w	Slider_Y(a0),d0
	rts

;***************************************************************************
; [ Add a scroll bar ]
;   IN : a0 - Pointer to scroll bar data (.l)
; All registers are restored
;***************************************************************************
Add_scroll_bar:
	movem.l	d7/a1,-(sp)
	lea.l	Scroll_bar_list,a1		; Search free entry
	moveq.l	#Max_scroll_bars-1,d7
.Loop:	tst.l	(a1)+			; Free ?
	bne.s	.Next
	move.l	a0,-4(a1)			; Yes -> Insert
	jsr	Init_scroll_bar
	bra.s	.Exit
.Next:	dbra	d7,.Loop			; Next
	; NO FREE ENTRY !
.Exit:	movem.l	(sp)+,d7/a1
	rts

;***************************************************************************
; [ Remove a scroll bar ]
;   IN : a0 - Pointer to scroll bar data (.l)
; All registers are restored
;***************************************************************************
Remove_scroll_bar:
	movem.l	d7/a1,-(sp)
	lea.l	Scroll_bar_list,a1		; Search scroll bar
	moveq.l	#Max_scroll_bars-1,d7
.Loop:	cmp.l	(a1)+,a0			; Is this the one ?
	bne.s	.Next
	clr.l	-4(a1)			; Yes -> Remove
	bra.s	.Exit
.Next:	dbra	d7,.Loop			; Next
	; NOT FOUND !
.Exit:	movem.l	(sp)+,d7/a1
	rts

;***************************************************************************
; [ Remove all scroll bars ]
; All registers are restored
;***************************************************************************
Remove_all_scroll_bars:
	movem.l	d7/a1,-(sp)
	lea.l	Scroll_bar_list,a1
	moveq.l	#Max_scroll_bars-1,d7
.Loop:	clr.l	(a1)+			; Remove
	dbra	d7,.Loop			; Next
	movem.l	(sp)+,d7/a1
	rts

;***************************************************************************	
; [ Find a scroll bar ]
;   IN : d0 - Object ID (.l)
;  OUT : a0 - Pointer to scroll bar data / 0 (.l)
; Changed registers : a0
;***************************************************************************
Find_scroll_bar:
	movem.l	d7/a1,-(sp)
	lea.l	Scroll_bar_list,a1		; Search scroll bar
	moveq.l	#Max_scroll_bars-1,d7
.Loop:	tst.l	(a1)			; Anything there ?
	beq.s	.Next
	move.l	(a1),a0			; Yes -> Examine
	cmp.l	Scroll_bar_object_ID(a0),d0	; Is this the one ?
	beq.s	.Exit
.Next:	addq.l	#4,a1			; Next
	dbra	d7,.Loop
	sub.l	a0,a0			; Not found
.Exit:	movem.l	(sp)+,d7/a1
	rts

;***************************************************************************	
; [ Initialize scroll bar ]
;   IN : a0 - Pointer to scroll bar data (.l)
; All registers are restored
;***************************************************************************
Init_scroll_bar:
	movem.l	d0/d1/a1/a2,-(sp)
; ---------- Scroll at all ? ----------------------
	moveq.l	#0,d0			; Total units less or same
	move.w	Total_units(a0),d0		;  as visible units ?
	move.w	Units_width(a0),d1
	mulu.w	Units_height(a0),d1
	cmp.w	d0,d1
	bgt.s	.Yes
	move.w	Scroll_bar_height(a0),d0	; Yes -> Set data
	move.w	d0,Slider_height(a0)
	clr.w	Scroll_bar_max_height(a0)
	move.w	#1,Row_height(a0)
	bra.s	.Go_on
; ---------- Prepare scroll bar variables ---------
.Yes:	cmp.w	#1,Units_width(a0)		; More than 1 unit per row ?
	beq.s	.One
	divu.w	Units_width(a0),d0		; Yes
.One:	move.w	d0,d1
	moveq.l	#0,d0			; Calculate row height
	move.w	Scroll_bar_height(a0),d0
	divu.w	d1,d0
	move.w	d0,Row_height(a0)
	moveq.l	#0,d0			; Calculate maximum height
	move.w	Total_units(a0),d0
	divu.w	Units_width(a0),d0
	sub.w	Units_height(a0),d0
	mulu.w	Row_height(a0),d0
	move.w	d0,Scroll_bar_max_height(a0)
	sub.w	Scroll_bar_height(a0),d0	; Calculate slider height
	neg.w	d0
	move.w	d0,Slider_height(a0)
; ---------- Prepare object layer -----------------
.Go_on:	move.l	Scroll_bar_object_layer(a0),a1	; Get address
	move.w	Scroll_bar_X(a0),d0		; Insert X-coordinates
	sub.w	Offset_X,d0
	move.w	d0,Obj_X1(a1)
	addq.w	#Scroll_bar_width-1,d0
	move.w	d0,Obj_X2(a1)
	move.w	Scroll_bar_Y(a0),d0		; Insert Y-coordinates
	sub.w	Offset_Y,d0
	move.w	d0,Obj_Y1(a1)
	add.w	Scroll_bar_height(a0),d0
	subq.w	#1,d0
	move.w	d0,Obj_Y2(a1)
; ---------- Display scroll bar -------------------
	sf	Slider_selected		; Clear flag
	move.w	Slider_Y(a0),d0		; Display
	jsr	Update_slider
	movem.l	(sp)+,d0/d1/a1/a2
	rts

;***************************************************************************	
; [ Scroll bar was pressed ] Mouse Event
;   IN : d0 - Object ID (.l)
; No registers are restored
;***************************************************************************
Scroll_bar_pressed:
	and.l	#$00ffffff,d0		; Search
	jsr	Find_scroll_bar
	cmp.l	#0,a0			; Found ?
	beq	.Exit
; ---------- Determine action ---------------------
	move.w	Mouse_Y,d0		; Still in bar area ?
	move.w	Scroll_bar_Y(a0),d1
	cmp.w	d1,d0
	bmi	.Exit
	move.w	d1,d2
	add.w	Scroll_bar_height(a0),d1
	cmp.w	d1,d0
	bpl	.Exit
	add.w	Slider_Y(a0),d2		; Above slider ?
	cmp.w	d2,d0
	bpl	.Not_above
	jsr	Scroll_bar_page_up		; Yes -> Page up
	bra	.Exit
.Not_above:
	add.w	Slider_height(a0),d2	; Below slider ?
	cmp.w	d2,d0
	bmi.s	.Drag
	jsr	Scroll_bar_page_down	; Yes -> Page down
	bra	.Exit
; ---------- Drag slider --------------------------
.Drag:	st	Slider_selected		; Selected !
	jsr	Hide_HDOBs
	jsr	Update_screen
	lea.l	Scroll_bar_MA,a1		; Create MA
	move.w	X1(a0),d0
	move.w	d0,MA_X1(a1)
	addq.w	#Scroll_bar_width-1,d0
	move.w	d0,MA_X2(a1)
	move.w	Y1(a0),d0
	move.w	d0,MA_Y1(a1)
	add.w	Scroll_bar_max_height(a0),d0
	move.w	d0,MA_Y2(a1)
	exg.l	a0,a1			; Set mouse
	jsr	Push_MA
	move.l	a1,a0
	move.w	Slider_Y(a0),d0
	add.w	Scroll_bar_Y(a0),d0
	move.w	d0,Mouse_Y
	move.w	Slider_Y(a0),d0		; Get Y-coordinate
.Again:	ext.l	d0			; Update needed ?
	divu.w	Row_height(a0),d0
	cmp.w	Current_row(a0),d0
	beq.s	.No
	move.w	Slider_Y(a0),d0		; Yes -> Update
	jsr	Update_slider
	jsr	Update_screen
	bra.s	.Go_on
.No:	jsr	Draw_scroll_bar		; Show bar
	jsr	Switch_screens
.Go_on:	move.b	Button_state,d0		; Still pressed ?
	btst	#Left_pressed,d0
	beq.s	.Done
	move.w	Mouse_Y,d0		; Get position
	sub.w	Scroll_bar_Y(a0),d0
	move.w	d0,Slider_Y(a0)		; Store
	bra.s	.Again
.Done:	Pop	MA			; Restore mouse
	jsr	Mouse_on
	sf	Slider_selected		; Deselect
	jsr	Update_slider
	jsr	Update_screen
	jsr	Show_HDOBs
.Exit:	rts

;***************************************************************************	
; [ Move the slider up one page ]
;   IN : a0 - Pointer to scroll bar data (.l)
; No registers are restored
;***************************************************************************
Scroll_bar_page_up:
	jsr	Light_slider		; Select
	move.w	Units_height(a0),d0		; Half-page up
	lsr.w	#1,d0
	bne.s	.Notzero
	moveq.l	#1,d0
.Notzero:	neg.w	d0
	add.w	Current_row(a0),d0
	bpl.s	.Ok
	moveq.l	#0,d0
.Ok:	mulu.w	Row_height(a0),d0
	jsr	Wait_4_unclick		; Wait
	jsr	Update_slider		; Deselect
	jmp	Update_screen

;***************************************************************************	
; [ Move the slider down one page ]
;   IN : a0 - Pointer to scroll bar data (.l)
; No registers are restored
;***************************************************************************
Scroll_bar_page_down:
	jsr	Light_slider		; Select
	move.w	Units_height(a0),d0		; Half-page down
	lsr.w	#1,d0
	bne.s	.Notzero
	moveq.l	#1,d0
.Notzero:	add.w	Current_row(a0),d0
	mulu.w	Row_height(a0),d0
	cmp.w	Scroll_bar_max_height(a0),d0
	bmi.s	.Ok
	move.w	Scroll_bar_max_height(a0),d0
.Ok:	jsr	Wait_4_unclick		; Wait
	jsr	Update_slider		; Deselect
	jmp	Update_screen

;***************************************************************************	
; [ Light slider ]
;   IN : a0 - Pointer to scroll bar data (.l)
; All registers are restored
;***************************************************************************	
Light_slider:
	st	Slider_selected
	jsr	Draw_scroll_bar
	jsr	Update_screen
	sf	Slider_selected
	rts

;***************************************************************************	
; [ Update slider ]
;   IN : d0 - New Y-coordinate (.w)
;        a0 - Pointer to scroll bar data (.l)
; All registers are restored
;***************************************************************************	
Update_slider:
	movem.l	d0/d1/a0,-(sp)
	cmp.w	Scroll_bar_max_height(a0),d0	; Too far down ?
	bmi.s	.Ok
	move.w	Scroll_bar_max_height(a0),d0	; Yes
.Ok:	move.w	d0,Slider_Y(a0)		; Set variables
	ext.l	d0
	divu.w	Row_height(a0),d0
	move.w	d0,Current_row(a0)
	mulu.w	Units_width(a0),d0
	move.w	d0,Scroll_bar_result(a0)
	jsr	Draw_scroll_bar		; Re-draw scroll bar
	move.l	Scroll_bar_draw_units(a0),d0	; Update
	beq.s	.None
	move.l	d0,a0
	jsr	(a0)
.None:	movem.l	(sp)+,d0/d1/a0
	rts

;***************************************************************************	
; [ Draw scroll bar ]
;   IN : a0 - Pointer to scroll bar data (.l)
; All registers are restored
;***************************************************************************
Draw_scroll_bar:
	movem.l	d0-d4/a0/a1,-(sp)
	tst.b	Slider_selected		; Slider selected ?
	bne.s	.Yes
	lea.l	Unselected_slider_colours,a1
	bra.s	.Do
.Yes:	lea.l	Selected_slider_colours,a1
; ---------- Draw scroll bar ----------------------
.Do:	move.w	Scroll_bar_X(a0),d0		; Get coordinates
	move.w	Scroll_bar_Y(a0),d1
	subq.w	#1,d0			; Surrounding box
	subq.w	#1,d1
	moveq.l	#Scroll_bar_width+2,d2
	move.w	Scroll_bar_height(a0),d3
	addq.w	#2,d3
	jsr	Draw_deep_box
	addq.w	#1,d0			; Top line
	addq.w	#1,d1
	move.w	d0,d2
	addq.w	#Scroll_bar_width-1,d2
	moveq.l	#Scroll_bar_shadow_colour,d4
	jsr	Draw_hline
	move.w	d1,d3			; Left line
	addq.w	#1,d1
	add.w	Scroll_bar_height(a0),d3
	subq.w	#1,d3
	jsr	Draw_vline
; ---------- Draw slider --------------------------
	move.w	Scroll_bar_X(a0),d0		; Get coordinates
	move.w	Scroll_bar_Y(a0),d1
	add.w	Slider_Y(a0),d1
	move.w	(a1)+,d4			; Top-left pixel
	jsr	Plot_pixel
	addq.w	#1,d0 			; Top line
	move.w	d0,d2
	addq.w	#Scroll_bar_width-3,d2
	move.w	(a1)+,d4
	jsr	Draw_hline
	move.w	d2,d0			; Top-right pixel
	addq.w	#1,d0
	move.w	(a1)+,d4
	jsr	Plot_pixel
	subq.w	#Scroll_bar_width-1,d0	; Left line
	addq.w	#1,d1
	move.w	d1,d3
	add.w	Slider_height(a0),d3
	subq.w	#3,d3
	move.w	(a1)+,d4
	jsr	Draw_vline
	addq.w	#1,d0			; Main box
	move.w	(a1)+,d4
	jsr	Draw_box
	addq.w	#Scroll_bar_width-2,d0	; Right line
	addq.w	#1,d0
	move.w	(a1)+,d4
	jsr	Draw_hline
	subq.w	#Scroll_bar_width-1,d0	; Bottom-left pixel
	add.w	Slider_height(a0),d1
	subq.w	#2,d1
	move.w	(a1)+,d4
	jsr	Plot_pixel
	addq.w	#1,d0 			; Bottom line
	move.w	(a1)+,d4
	jsr	Draw_hline
	addq.w	#Scroll_bar_width-2,d0	; Bottom-right pixel
	move.w	(a1)+,d4
	jsr	Plot_pixel
	move.w	Scroll_bar_Y(a0),d3		; Slider at the bottom ?
	add.w	Scroll_bar_height(a0),d3
	subq.w	#1,d3
	cmp.w	d1,d3
	bmi.s	.Exit
	subq.w	#Scroll_bar_width-1,d0	; No -> Draw shadow
	addq.w	#1,d1
	move.w	d0,d2
	addq.w	#Scroll_bar_width-1,d2
	moveq.l	#Scroll_bar_shadow_colour,d4
	jsr	Draw_hline
.Exit:	movem.l	(sp)+,d0-d4/a0/a1
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data

; Top-left corner
; Top edge
; Top-right corner
; Left edge
; Main area
; Right edge
; Bottom-left corner
; Bottom edge
; Bottom-right corner

Unselected_slider_colours:

Selected_slider_colours:



	SECTION	Fast_BSS,bss
Scroll_bar_list:	ds.l Max_scroll_bars
Scroll_bar_MA:	ds.w 4			; MA used for dragging
Slider_selected:	ds.b 1
	even
