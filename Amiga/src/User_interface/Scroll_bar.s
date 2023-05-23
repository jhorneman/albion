; OOUI Scroll bar handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 25-1-1994

	XDEF	Scroll_bar_class

	SECTION	Program,code
;***************************************************************************	
; [ Initialize method for scroll bar objects ]
;   IN : a0 - Pointer to scroll bar object (.l)
;        a1 - Pointer to scroll bar OID (.l)
; All registers are restored
;***************************************************************************
Init_scroll_bar:
	movem.l	d0/d1,-(sp)
; ---------- Initialize object --------------------
	move.w	OID_Total_units(a1),Total_units(a0)	; Copy
	move.w	OID_Units_width(a1),Units_width(a0)
	move.w	OID_Units_height(a1),Units_height(a0)
	move.w	OID_Scroll_bar_height(a1),Scroll_bar_height(a0)
	move.w	OID_X(a1),d0		; Set rectangle
	move.w	OID_Y(a1),d1
	add.w	X1(a0),d0
	add.w	Y1(a0),d1
	move.w	d0,X1(a0)
	move.w	d1,Y1(a0)
	addq.w	#Scroll_bar_width,d0
	add.w	Scroll_bar_height(a0),d1
	subq.w	#1,d1
	move.w	d0,X2(a0)
	move.w	d1,Y2(a0)
; ---------- Scroll at all ? ----------------------
	moveq.l	#0,d0			; Total units less or same
	move.w	Total_units(a0),d0		;  as visible units ?
	move.w	Units_width(a0),d1
	mulu.w	Units_height(a0),d1
	cmp.w	d0,d1
	bmi.s	.No
	move.w	Scroll_bar_height(a0),d0	; Yes -> Set data
	move.w	d0,Slider_height(a0)
	clr.w	Scroll_bar_max_height(a0)
	move.w	#1,Row_height(a0)
	bra.s	.Exit
; ---------- Prepare scroll bar variables ---------
.No:	cmp.w	#1,Units_width(a0)		; More than 1 unit per row ?
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
.Exit:	movem.l	(sp)+,d0/d1
	rts

;***************************************************************************	
; [ Draw AND Update method for scroll bar objects ]
;   IN : a0 - Pointer to scroll bar object (.l)
; All registers are restored
;***************************************************************************
Draw_scroll_bar:
	movem.l	d0/d1/d7/a0,-(sp)
	sf	Slider_selected
	jsr	Do_draw_scroll_bar		; Re-draw scroll bar
	move.w	Object_self(a0),d0		; Draw all elements
	move.w	#Draw_method,d1
	move.w 	Scroll_bar_result(a0),d7
	jsr	Execute_brother_methods
	movem.l	(sp)+,d0/d1/d7/a0
	rts

;***************************************************************************	
; [ Set method for scroll bar objects ]
;   IN : d2 - Scroll bar result (.w)
;        a0 - Pointer to scroll bar object (.l)
; All registers are restored
;***************************************************************************
Set_scroll_bar:
	movem.l	d0/a0,-(sp)
	move.w	d2,d0
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
; [ Get method for scroll bar objects ]
;   IN : a0 - Pointer to scroll bar object (.l)
;  OUT : d0 - Scroll bar Y-position (.w)
; All registers are restored
;***************************************************************************
Get_scroll_bar:
	move.w	Scroll_bar_result(a0),d0
	rts

;***************************************************************************	
; [ Scroll bar clicked mouse event ]
;   IN : a0 - Pointer to scroll bar object (.l)
; No registers are restored
;***************************************************************************
Scroll_bar_clicked:
; ---------- Determine action ---------------------
	move.w	Mouse_Y,d0		; Still in bar area ?
	move.w	Y1(a0),d1
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
	move.w	d0,X1(a1)
	addq.w	#Scroll_bar_width-1,d0
	move.w	d0,X2(a1)
	move.w	Y1(a0),d0
	move.w	d0,Y1(a1)
	add.w	Scroll_bar_max_height(a0),d0
	move.w	d0,Y2(a1)
	jsr	Mouse_off			; Off
	move.w	Slider_Y(a0),d0		; Get Y-coordinate
	cmp.w	Scroll_bar_max_height(a0),d0	; Too far down ?
	bmi.s	.Ok
	move.w	Scroll_bar_max_height(a0),d0	; Yes
.Ok:	add.w	Y1(a0),d0
	move.w	Mouse_Y,d7		; Save offset
	sub.w	d0,d7
	move.l	d7,-(sp)
	move.w	d0,Mouse_Y		; Set mouse Y
	exg.l	a0,a1			; Set mouse area
	jsr	Push_MA
	move.l	a1,a0
	move.w	Slider_Y(a0),d0		; Get Y-coordinate
.Again:	ext.l	d0			; Update needed ?
	divu.w	Row_height(a0),d0
	cmp.w	Current_row(a0),d0
	beq.s	.No
	move.w	Slider_Y(a0),d0		; Yes -> Update
	jsr	Update_slider
	jsr	Update_screen
	bra.s	.Go_on
.No:	jsr	Do_draw_scroll_bar		; Show bar
	jsr	Switch_screens
.Go_on:	move.b	Button_state,d0		; Still pressed ?
	btst	#Left_pressed,d0
	beq.s	.Done
	move.w	Mouse_Y,d0		; Get position
	sub.w	Y1(a0),d0
	move.w	d0,Slider_Y(a0)		; Store
	bra.s	.Again
.Done:	Pop	MA			; Restore mouse
	move.l	(sp)+,d0
	add.w	d0,Mouse_Y
	jsr	Mouse_on
	sf	Slider_selected		; Deselect
	move.w	Slider_Y(a0),d0
	jsr	Update_slider
	jsr	Update_screen
	jsr	Show_HDOBs
.Exit:	rts

;***************************************************************************	
; [ Move the slider up one page ]
;   IN : a0 - Pointer to scroll bar object (.l)
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
;   IN : a0 - Pointer to scroll bar object (.l)
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
;   IN : a0 - Pointer to scroll bar object (.l)
; All registers are restored
;***************************************************************************	
Light_slider:
	st	Slider_selected
	jsr	Do_draw_scroll_bar
	jsr	Update_screen
	sf	Slider_selected
	rts

;***************************************************************************	
; [ Update slider ]
;   IN : d0 - New Y-coordinate (.w)
;        a0 - Pointer to scroll bar object (.l)
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
	jsr	Do_draw_scroll_bar		; Re-draw scroll bar
	move.w	Object_self(a0),d0		; Draw all elements
	move.w	#Draw_method,d1
	move.w 	Scroll_bar_result(a0),d7
	jsr	Execute_brother_methods
.None:	movem.l	(sp)+,d0/d1/a0
	rts

;***************************************************************************	
; [ Draw scroll bar ]
;   IN : a0 - Pointer to scroll bar object (.l)
; All registers are restored
;***************************************************************************
Do_draw_scroll_bar:
	movem.l	d0-d4/a1,-(sp)
	tst.b	Slider_selected		; Slider selected ?
	bne.s	.Yes
	lea.l	Unselected_slider_colours,a1
	bra.s	.Do
.Yes:	lea.l	Selected_slider_colours,a1
; ---------- Draw scroll bar ----------------------
.Do:	move.w	X1(a0),d0			; Get coordinates
	move.w	Y1(a0),d1
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
	move.w	#Darkest,d4
	jsr	Draw_hline
	move.w	d1,d3			; Left line
	addq.w	#1,d1
	add.w	Scroll_bar_height(a0),d3
	subq.w	#1,d3
	jsr	Draw_vline
; ---------- Draw slider --------------------------
	move.w	X1(a0),d0			; Get coordinates
	move.w	Y1(a0),d1
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
	move.w	(a1)+,d4
	jsr	Draw_vline
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
	move.w	Slider_Y(a0),d2		; Slider at the bottom ?
	cmp.w	Scroll_bar_max_height(a0),d2
	bpl.s	.Exit
	subq.w	#Scroll_bar_width-1,d0	; No -> Draw shadow
	addq.w	#1,d1
	move.w	d0,d2
	addq.w	#Scroll_bar_width-1,d2
	move.w	#Darkest,d4
	jsr	Draw_hline
.Exit:	movem.l	(sp)+,d0-d4/a1
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Scroll_bar_class:
	dc.l 0
	dc.w Scroll_bar_object_size
	Method Init,Init_scroll_bar
	Method Draw,Draw_scroll_bar
	Method Update,Draw_scroll_bar
	Method Get,Get_scroll_bar
	Method Set,Set_scroll_bar
	Method Mev,.Mev
	dc.w -1

.Mev:	dc.w $0202
	dc.l Scroll_bar_clicked
	dc.w -1

Unselected_slider_colours:
	dc.w Brightest,Brighter,Normal
	dc.w Brighter,Normal,Dark
	dc.w Normal,Dark,Darker

Selected_slider_colours:
	dc.w White,Gold,Gold+2
	dc.w Gold,Gold+1,Gold+3
	dc.w Gold+2,Gold+3,Gold+3


	SECTION	Fast_BSS,bss
Scroll_bar_MA:	ds.w 4			; MA used for dragging
Slider_selected:	ds.b 1
	even
