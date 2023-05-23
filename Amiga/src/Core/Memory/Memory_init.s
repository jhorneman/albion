; Memory Manager IV (Son Of Chip-mem)
; Initialization
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 26-1-1994

	SECTION	Program,code
;*****************************************************************************
; [ Reset all memory areas ]
; All registers are restored
;*****************************************************************************
Reset_memory:
	movem.l	d7/a0-a2,-(sp)
	move.w	#Check_frequency,Check_counter	; Reset
	lea.l	Memory_areas,a2		; All areas
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	movea.l	a2,a1			; Reset area
.Again:	tst.l	Block_next(a1)		; Last in	chain ?
	beq.s	.Done
	movea.l	Block_next(a1),a0		; Delete block
	jsr	Delete_entry
	bra.s	.Again
.Done:	jsr	Find_free_entry		; Create first block
	move.l	Area_start(a1),Block_start(a0)
	move.l	Area_size(a1),Block_size(a0)
	move.l	a2,a1
	jsr	Add_entry			; Add block to list
	lea.l	Area_data_size(a2),a2	; Next area
.Entry:	dbra	d7,.Loop
	LOCAL
	lea.l	File_infos,a0		; Clear file infos
	moveq.l	#Max_files-1,d7
.Loop:	clr.b	File_type(a0)
	lea.l	File_data_size(a0),a0
	dbra	d7,.Loop
	jsr	Clear_all_handles		; Clear handles
	movem.l	(sp)+,d7/a0-a2
	rts

;*****************************************************************************
; [ Initialize memory ]
; All registers are restored
; NOTE :
;  - This routine assumes the memory area list is cleared.
;  - FAST memory is grabbed first so it will be used first as well.
;*****************************************************************************
Init_memory:
	movem.l	d0-d7/a0-a2,-(sp)
	lea.l	Memory_areas,a2
	moveq.l	#0,d7
	move.l	#OS_FAST,d3		; Allocate all FAST
	moveq.l	#FAST_area,d5
	move.l	#MEMF_FAST,d6
	jsr	.Grab_everything
	move.l	d4,Total_FAST_memory
	move.l	#OS_CHIP,d3		; Allocate all CHIP
	moveq.l	#CHIP_area,d5
	move.l	#MEMF_CHIP,d6
	jsr	.Grab_everything
	move.l	d4,Total_CHIP_memory
	cmp.l	#Minimum_CHIP,d4		; Enough CHIP ?
	bpl.s	.Enough
	move.l	#NOT_ENOUGH_CHIP_MEMORY,d0	; No!
	jmp	Fatal_error
.Enough:	move.l	Total_FAST_memory,d0	; Enough FAST ?
	cmp.l	#Minimum_FAST,d0
	bpl.s	.Ok
	move.l	Total_CHIP_memory,d0	; No -> Maybe enough CHIP ?
	cmp.l	#Minimum_CHIP+Minimum_FAST,d0
	bpl.s	.Ok
	move.l	#NOT_ENOUGH_FAST_MEMORY,d0	; No!
	jmp	Fatal_error
.Ok:	move.w	d7,Number_of_areas		; Store number of areas
	movem.l	(sp)+,d0-d7/a0-a2
	rts

; [ Grab all memory ]
;   IN : d3 - Memory for OS (.l)
;        d5 - My memory type (.b) 
;        d6 - OS memory type (.l)
;        d7 - Area counter (.b)
;        a2 - Pointer to areas (.l)
;  OUT : d4 - Total memory grabbed (.l)
;        d7 - Area counter
;        a2 - Pointer to areas (.l)
; Changed registers : d4,d7,a2
.Grab_everything:
	movem.l	d0-d3/d5/d6/a0/a1,-(sp)
	moveq.l	#0,d4
.Again1:	move.l	d6,d1			; Find largest block
	or.l	#MEMF_LARGEST,d1
	kickEXEC	AvailMem
	cmp.l	#Small_fish,d0		; Too small ?
	bmi.s	.Done
	move.l	d0,d2			; Save size
	kickEXEC	AllocMem			; Get it
	tst.l	d0			; Error ? -> try again
	beq.s	.Again1
	move.l	d0,Area_start(a2)		; Initialize area
	move.l	d2,Area_size(a2)
	move.b	d5,Memory_type(a2)
	add.l	d2,d4			; Add up
	lea.l	Area_data_size(a2),a2	; Next area
	addq.w	#1,d7		
	cmp.w	#Max_areas,d7
	bmi.s	.Again1
.Done:

;	move.l	d4,d3			; Use this to test
;	cmp.b	#CHIP_area,d5		; the minimum memory
;	bne.s	.Fast			; configuration
;	sub.l	#Minimum_CHIP,d3
;	bra.s	.Continue
;.Fast:	sub.l	#Minimum_FAST,d3
;.Continue:

	move.l	d6,d1			; Find remaining memory
	kickEXEC	AvailMem
	sub.l	d0,d3			; Enough left for OS ?
	bmi	.End
.Again2:	lea.l	-Area_data_size(a2),a2	; Next area
	cmp.b	Memory_type(a2),d5
	bne	.End
	move.l	Area_size(a2),d0		; Split or whole ?
	cmp.l	d0,d3
	bmi.s	.Split
	sub.l	d0,d3			; Return area
	sub.l	d0,d4
	move.l	Area_size(a2),d0		; Free area memory
	move.l	Area_start(a2),a1
	kickEXEC	FreeMem
	subq.w	#1,d7			; One area less
	beq	.End
	bra.s	.Again2
.Split:	sub.l	d0,d4			; Return area
	movem.l	d0/d1/a0/a1,-(sp)
	move.l	Area_size(a2),d0		; Free area memory
	move.l	Area_start(a2),a1
	kickEXEC	FreeMem
	movem.l	(sp)+,d0/d1/a0/a1
	subq.w	#1,d7			; One area less
	sub.l	d3,d0			; Get proper size
	cmp.l	#Small_fish,d0		; Too small ?
	bmi.s	.End
	move.l	d0,d2			; Save size
	move.l	d6,d1			; Get area again
	kickEXEC	AllocMem
	tst.l	d0
	beq.s	.End
	move.l	d0,Area_start(a2)		; Initialize area
	move.l	d2,Area_size(a2)
	move.b	d5,Memory_type(a2)
	add.l	d2,d4			; Add up
	lea.l	Area_data_size(a2),a2	; Next area
	addq.w	#1,d7		
.End:	movem.l	(sp)+,d0-d3/d5/d6/a0/a1
	rts

;*****************************************************************************
; [ Restore memory ]
; All registers are restored
;*****************************************************************************
Exit_memory:
	movem.l	d0/d1/d7/a0-a2,-(sp)
	lea.l	Memory_areas,a2		; All areas
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	move.l	Area_size(a2),d0		; Free area memory
	move.l	Area_start(a2),a1
	kickEXEC	FreeMem
	lea.l	Area_data_size(a2),a2	; Next area
.Entry:	dbra	d7,.Loop
	movem.l	(sp)+,d0/d1/d7/a0-a2
	rts

;***************************************************************************	
; The DATA & BSS segments
;***************************************************************************	
	SECTION	Fast_BSS,bss
Total_CHIP_memory:	ds.l 1
Total_FAST_memory:	ds.l 1
