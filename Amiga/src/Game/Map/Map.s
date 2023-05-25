; Map routines
; Written by J.Horneman (In Tune With The Universe)
; Start : 24-3-1994

	SECTION	Program,code
;*****************************************************************************
; [ Initialize map ]
; All registers are	restored
;*****************************************************************************
Init_map:
	movem.l	d0/d1/d7/a0,-(sp)
; ---------- Load map -----------------------------
	move.w	Map_nr,d0
	moveq.l	#Map_data_file,d1
	jsr	Load_subfile
	move.b	d0,Mapdata_handle		; Store handle
; ---------- Determine map type -------------------
	jsr	Claim_pointer		; Get map info
	move.l	d0,a0
	move.b	Map_type(a0),d0
	move.w	Map_special(a0),d1
	Free	Mapdata_handle
	cmp.b	#1,d0			; 2D or 3D map ?
	seq	_2D_or_3D_map
	lsr.w	#Map_type,d1		; Get map type
	and.w	#$0003,d1
	move.b	d1,Current_map_type
	LOCAL
; ---------- Initialize ---------------------------

;	sf	Bumped			; Reset flags
;	sf	Combat_req
;	sf	Big_brother_flag
;	clr.w	Current_map_music
;	move.w	#-1,Current_NPC		; !

	Get	Mapdata_handle,a0	
	moveq.l	#0,d0			; Store map's width
	move.b	Map_width(a0),d0
	move.w	d0,Width_of_map
	moveq.l	#0,d1			; Store map's height
	move.b	Map_height(a0),d1
	move.w	d1,Height_of_map
	Free	Mapdata_handle
	mulu.w	d0,d1			; Calculate map size
	move.w	d1,Size_of_map
	jsr	Calculate_map_data_offsets
; ---------- Start map ----------------------------
	tst.b	_2D_or_3D_map		; 2D or 3D map ?
	bne.s	.3D
	lea.l	Map2D_Mod,a0
	bra.s	.Do
.3D:	lea.l	Map3D_Mod,a0
.Do:	jsr	Push_Module
	movem.l	(sp)+,d0/d1/d7/a0
	rts

;*****************************************************************************
; [ Exit map ]
; All registers are	restored
;*****************************************************************************
Exit_map:
;	st	Time_lock
	jsr	Exit_display		; Leave map
	jmp	Pop_Module

;*****************************************************************************
; [ Calculate offsets to map data ]
; All registers are restored
;*****************************************************************************
Calculate_map_data_offsets:
	movem.l	d0/d1/d7/a0/a1,-(sp)
	Get	Mapdata_handle,a0
; ---------- Skip map layers ----------------------
	move.l	#Map_data,d0		; Start at underlay	layer
	moveq.l	#0,d1
	move.w	Size_of_map,d1
	add.l	d1,d0			; 3 bytes/square
	add.l	d1,d1
	add.l	d1,d0
; ---------- Skip event entries -------------------
	addq.l	#2,d0			; Skip number
	move.l	d0,Event_entry_offset	; Store offset
	lea.l	0(a0,d0.l),a1		; Skip event entries
	moveq.l	#0,d1
	move.w	Height_of_map,d7		; +1 !!!
.Loop1:	move.w	(a1)+,d1
	addq.l	#2,d0
	add.l	d1,d0
	add.l	d1,a1
	dbra	d7,.Loop1
; ---------- Skip event data ----------------------
	move.w	0(a0,d0.l),d1		; Get number of events
	addq.l	#2,d0			; Skip number
	move.l	d0,Event_data_offset	; Store offset
	mulu.w	#Event_data_size,d1		; Skip event data
	add.l	d1,d0
; ---------- Skip NPC path data -------------------
	move.l	d0,NPC_path_base_offset	; Store offset
	move.l	a0,a1			; Skip NPC path data
	lea.l	NPC_data(a1),a1
	moveq.l	#NPCs_per_map-1,d7
.Loop2:	tst.b	NPC_char_nr(a1)		; Anyone there ?
	beq.s	.Next2
	move.w	NPC_status_bits(a1),d1	; Get movement type
	lsr.w	#NPC_movement_type,d1
	and.w	#$0003,d1
	cmp.w	#Path_movement,d1		; Path ?
	beq.s	.Path
	addq.l	#2,d0			; No
	bra.s	.Next2
.Path:	add.l	#Max_steps*2,d0		; Yes
.Next2:	lea.l	NPC_data_size(a1),a1	; Next NPC
	dbra	d7,.Loop2
; ---------- Skip Goto-point data -----------------
	move.w	0(a0,d0.l),d1		; Get number of goto points
	move.w	d1,Nr_goto_points
	addq.l	#2,d0			; Skip number
	move.l	d0,Goto_point_offset	; Store
	mulu.w	#Goto_pnt_data_size,d1	; Skip goto point data
	add.l	d1,d0
; ---------- Skip event automap data --------------
	tst.b	_2D_or_3D_map		; 3D map ?
	beq.s	.Exit
	move.l	d0,Event_automap_offset	; Store
.Exit:	Free	Mapdata_handle
	movem.l	(sp)+,d0/d1/d7/a0/a1
	rts

;*****************************************************************************
; [ Load map texts ]
; All registers are restored
;*****************************************************************************
Load_map_texts:
	movem.l	d0/d1,-(sp)
	move.w	Map_nr,d0			; Load map text file
	moveq.l	#Map_text_file,d1
	jsr	Load_subfile
	move.b	d0,Maptext_handle
	movem.l	(sp)+,d0/d1
	rts

;*****************************************************************************
; [ Load map palette ]
; All registers are restored
;*****************************************************************************
Load_map_palette:
	movem.l	d0/d1/d6/d7/a0-a2,-(sp)
	Get	Mapdata_handle,a0		; Get palette number
	moveq.l	#0,d0
	move.b	Palette_nr(a0),d0
	Free	Mapdata_handle
	moveq.l	#Palette_file,d1		; Load palette
	jsr	Load_subfile
	move.b	d0,d7
	jsr	Claim_pointer		; Copy palette
	move.l	d0,a0
	lea.l	Map_palette,a1
	lea.l	Backup_palette,a2
	move.w	#192*3-1,d6
.Loop:	move.b	(a0),(a1)+
	move.b	(a0)+,(a2)+
	dbra	d6,.Loop
	move.b	d7,d0			; Destroy palette
	jsr	Free_pointer
	jsr	Free_memory
	lea.l	Map_palette,a0		; Set palette
	moveq.l	#0,d0
	move.w	#192,d1
	jsr	Set_palette_part
	movem.l	(sp)+,d0/d1/d6/d7/a0-a2
	rts
