; Memory Manager IV (Son Of Chip-mem)
; Memory allocation
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 26-1-1994


; Notes :
;   - Sadly, it is impossible to guarantee that no blocks are claimed when
;     memory is allocated. This is taken into account by [ Collect_garbage ].

	SECTION	Program,code
;*****************************************************************************
; [ Re-allocate memory ]
;   IN : d0 - Subfile number (.w)
;        d1 - File type (.b)
;  OUT : d0 - 0 -> No success (.b)
;             ? -> Memory handle (.b)
; Changed registers : d0
;*****************************************************************************
Reallocate_memory:
	movem.l	a0/a1,-(sp)
	jsr	Find_memory		; Find file
	move.l	a0,d0			; Found ?
	beq	.Exit
	jsr	Set_priority		; Yes -> (Re)set priority
	btst	#Allocated,Block_flags(a0)	; Already allocated ?
	beq.s	.No
	cmp.b	#-1,File_load_counter(a1)	; Yes -> Loaded too often ?
	beq.s	.Yes
	addq.b	#1,File_load_counter(a1)	; No -> Count up
.Yes:	move.b	Block_handle(a0),d0		; Get handle
	bra	.Exit
.No:	jsr	Do_reallocate		; No -> Reallocate
.Exit:	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Duplicate file memory, if found ]
;   IN : d0 - Subfile number (.w)
;        d1 - File type (.b)
;  OUT : d0 - 0 -> No success (.b)
;             Memory handle (.b)
; Changed registers : d0
;*****************************************************************************
Duplicate_memory:
	movem.l	d1/d2/a0-a2,-(sp)
	jsr	Find_memory		; Find file
	move.l	a0,d0			; Found ?
	beq.s	.Exit
	btst	#Allocated,Block_flags(a0)	; Already allocated ?
	beq.s	.No
	move.l	a0,a2
	move.l	Block_size(a2),d0		; Get file length
	move.l	d0,-(sp)			; Save
	moveq.l	#0,d1			; Allocate file memory
	move.b	File_type(a1),d1
	move.w	Subfile_nr(a1),d2
	jsr	File_allocate
	move.b	d0,d1			; Save
	jsr	Claim_pointer		; Get target address
	move.l	d0,a1
	move.l	Block_start(a2),a0		; Get source address
	move.l	(sp)+,d0			; Duplicate file
	jsr	Copy_memory
	move.b	d1,d0			; Output
	jsr	Free_pointer
	bra.s	.Exit
.No:	jsr	Set_priority		; (Re)set priority
	jsr	Do_reallocate		; Reallocate
.Exit:	movem.l	(sp)+,d1/d2/a0-a2
	rts

;*****************************************************************************
; [ Do actual re-allocating ]
;   IN : a0 - Pointer to memory block (.l)
;        a1 - Pointer to file info (.l)
;  OUT : d0 - Memory handle (.b)
; Changed registers : d0
;*****************************************************************************
Do_reallocate:
	movem.l	d1/d2/d7/a0-a2,-(sp)
	bset	#Allocated,Block_flags(a0)	; Allocate
	jsr	Create_memory_handle	; Create a handle
	move.l	a1,-(sp)			; Get area memory type
	jsr	Find_memory_area
	move.b	Memory_type(a1),d1
	move.l	(sp)+,a1
	cmp.b	#FAST_area,d1		; Is FAST area ?
	bne	.Exit
	jsr	Get_file_memory_type	; Yes -> but file is CHIP ?
	cmp.b	#CHIP,d1
	bne	.Exit
	move.b	d0,d7			; Yes
	move.l	a0,a2
	move.l	Block_size(a2),d0		; Allocate CHIP memory
	moveq.l	#0,d1
	move.b	File_type(a1),d1
	move.w	Subfile_nr(a1),d2
	jsr	File_allocate
	move.b	d0,d1			; Save
	jsr	Claim_pointer		; Get target address
	move.l	d0,a1
	move.b	d7,d0			; Find original block again (!)
	jsr	Find_entry
	move.l	a0,a2
	move.l	Block_start(a2),a0		; Get source address
	move.l	Block_size(a2),d0		; Copy file to CHIP
	jsr	Copy_memory
	move.l	a2,a0			; Kill original block
	jsr	Destroy_memory_handle
	clr.l	Block_info(a0)		; Erase info
	jsr	Merge_memory_block		; Try to merge
	move.b	d1,d0			; Output
	jsr	Free_pointer
.Exit:	movem.l	(sp)+,d1/d2/d7/a0-a2
	rts

;*****************************************************************************
; [ Find VALID file memory ]
;   IN : d0 - Subfile number (.w)
;        d1 - File type (.b)
;  OUT : a0 - Pointer to memory block (0 = not found) (.l)
;        a1 - Pointer to file info (.l)
; Changed registers : a0,a1
;*****************************************************************************
Find_memory:
	movem.l	d0-d2/d7/a2,-(sp)
	lea.l	File_infos,a1		; Search file infos
	moveq.l	#1,d2
	move.w	#Max_files-1,d7
.Loop1:	cmp.w	Subfile_nr(a1),d0		; The right file ?
	bne.s	.Next
	cmp.b	File_type(a1),d1
	beq.s	.Found
.Next:	addq.w	#1,d2			; Next file
	lea.l	File_data_size(a1),a1
	dbra	d7,.Loop1
	bra	.No_luck
.Found:	lea.l	Memory_areas,a2		; Search all areas
	move.w	Number_of_areas,d7
	bra.s	.Entry2
.Loop2:	move.l	a2,a0			; Search area
.Again:	move.l	Block_next(a0),d0		; End of chain ?
	beq.s	.Next2
	move.l	d0,a0			; No
	cmp.b	Block_file_index(a0),d2	; Is this the one ?
	bne.s	.Again
	btst	#Invalid,Block_flags(a0)	; Yes -> Invalid ?
	beq.s	.Exit
	bra.s	.Again
.Next2:	lea.l	Area_data_size(a2),a2	; Next area
.Entry2:	dbra	d7,.Loop2
.No_luck:	sub.l	a0,a0			; No success
.Exit:	movem.l	(sp)+,d0-d2/d7/a2
	rts

;*****************************************************************************
; [ Allocate a block of CHIP memory ]
;   IN : d0 - Size of memory block (.l)
;  OUT : d0 - Memory handle (.b)
; Changed registers : d0
;*****************************************************************************
Allocate_CHIP:
	move.l	d1,-(sp)
	move.b	#CHIP,d1
	jsr	Do_allocate
	move.l	(sp)+,d1
	rts

;*****************************************************************************
; [ Allocate a block of FAST memory ]
;   IN : d0 - Size of memory block (.l)
;  OUT : d0 - Memory handle (.b)
; Changed registers : d0
;*****************************************************************************
Allocate_FAST:
	move.l	d1,-(sp)
	move.b	#FAST,d1
	jsr	Do_allocate
	move.l	(sp)+,d1
	rts

;*****************************************************************************
; [ Allocate a block of DON'T CARE memory ]
;   IN : d0 - Size of memory block (.l)
;  OUT : d0 - Memory handle (.b)
; Changed registers : d0
;*****************************************************************************
Allocate_memory:
	move.l	d1,-(sp)
	move.b	#DONTCARE,d1
	jsr	Do_allocate
	move.l	(sp)+,d1
	rts

;*****************************************************************************
; [ Allocate memory for a file ]
;   IN : d0 - Size of file (.l)
;        d1 - File type / 0 (.b)
;        d2 - Subfile number (.w)
;  OUT : d0 - Memory handle (.b)
; Changed registers : d0
;*****************************************************************************
File_allocate:
	movem.l	d1/a0/a1,-(sp)
	move.l	d0,Original_length		; Store original length
	tst.b	d1			; Real file type ?
	bne.s	.Yes
	move.l	#-1,(sp)			; No
	move.b	Default_memory_type,d1	; Default memory type
	jsr	Do_allocate		; Allocate memory
	bra.s	.Go_on
.Yes:	move.l	d1,-(sp)			; Yes
	jsr	Get_file_memory_type	; Get memory type
	jsr	Do_allocate		; Allocate memory
.Go_on:	move.l	(sp)+,d1
	jsr	Find_entry		; Find entry
	move.l	d0,-(sp)
	jsr	Find_free_file_info_block	; Find free file info
	move.b	d0,Block_file_index(a0)	; Write index
	move.b	d1,File_type(a1)		; Write file type &
	move.w	d2,Subfile_nr(a1)		;  subfile number
	move.l	Original_length,d0		; Store length low byte
	move.b	d0,File_length_low(a1)
	tst.l	d0			; Should be empty ?
	bne.s	.Not_MT
	move.l	Block_start(a0),a1		; Yes -> Make sure
	clr.l	(a1)
.Not_MT:	jsr	Set_priority		; Set priority
	jsr	Age_memory
	move.l	(sp)+,d0
	movem.l	(sp)+,d1/a0/a1
	rts

;*****************************************************************************
; [ Allocate a memory block ]
;   IN : d0 - Size of memory block (.l)
;        d1 - Memory type (.b)
;  OUT : d0 - Memory handle (.b)
; Changed registers : d0
; Notes :
;   - All Allocate routines call this routine.
;   - The bottom 4 bits of the original length are stored.
;*****************************************************************************
Do_allocate:
	movem.l	d1/d7/a0/a1,-(sp)
	sf	Handles_invalid		; Clear
	sf	Restart_music
	addq.l	#3,d0			; Force longword boundary
	andi.b	#$fc,d0
	tst.l	d0			; No size ?
	bne.s	.Ok
	moveq.l	#4,d0			; Minimum size !
.Ok:	move.l	d0,d7
	lea.l	Passes,a1			; Try all	passes
.Again:	tst.l	(a1)			; Last pass ?
	beq.s	.End
	movea.l	(a1)+,a0			; No -> Try pass
	move.l	d7,d0
	jsr	(a0)
	tst.b	d0			; Success ?
	bne	.Exit
	bra	.Again
.End:	sf	Restart_music		; No
	jmp	Out_of_memory
.Exit:	tst.b	Handles_invalid		; Handles invalid ?
	beq.s	.Skip1
	sf	Handles_invalid		; Yes
	jsr	Validate_handles
.Skip1:	tst.b	Restart_music		; Restart music ?
	beq.s	.Skip2
	sf	Restart_music		; Yes
	move.l	d0,-(sp)
	st	Music_flag		; System flag
	moveq.l	#-1,d0			; Do
	jsr	Set_music
	move.l	(sp)+,d0
.Skip2:	movem.l	(sp)+,d1/d7/a0/a1
	rts

Passes:	dc.l Pass_0,Pass_1,Pass_2,Pass_9
	dc.l Pass_3,Pass_4,Pass_5,Pass_6
	dc.l Pass_7,Pass_8
	dc.l 0

; [ Allocate memory PASS 0 ]
;   IN : d1 - Memory type (.b)
;  OUT : d0 - 0 (.b)
;        d1 - FAST if it was CHIP before (.b)
; Changed registers : d0,d1,a1
; NOTE :
;  - If no FAST memory is available, but the memory block should be in
;    FAST memory, this pass changes the required memory type to CHIP.
Pass_0:
	moveq.l	#0,d0			; No way
	cmpi.b	#FAST,d1			; Should it be FAST ?
	bne.s	.Exit
	tst.l	Total_FAST_memory		; Yes -> Got any FAST ?
	bne.s	.Exit
	move.b	#CHIP,d1			; No -> Try CHIP
.Exit:	rts

; [ Allocate memory PASS 1 ]
;   IN : d0 - Size of memory block (.l)
;        d1 - Memory type (.b)
;  OUT : d0 - 0 - No success / Memory handle (.b)
; Changed registers : d0
; NOTE :
;  - This pass seeks the area of the right type with the largest LFB.
Pass_1:
	movem.l	d1-d7/a0-a6,-(sp)
	lea.l	Memory_areas,a1		; All areas
	moveq.l	#0,d2
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	move.b	Memory_type(a1),d3		; Right type ?
	btst	d3,d1
	beq.s	.Next
	move.l	a1,a0			; Find LFB
	jsr	Find_LFB
	cmp.l	#0,a0			; Any ?
	beq.s	.Next
	cmp.l	Block_size(a0),d2		; Larger ?
	bpl.s	.Next
	move.l	Block_size(a0),d2		; Set
	movea.l	a0,a2
.Next:	lea.l	Area_data_size(a1),a1	; Next area
.Entry:	dbra	d7,.Loop
	cmp.l	d0,d2			; Large enough ?
	bpl.s	.Go_on
	moveq.l	#0,d0			; No success
	bra	.Exit
.Go_on:	move.l	a2,a0			; Allocate
	jsr	Allocate_block
.Exit:	movem.l	(sp)+,d1-d7/a0-a6
	rts

; [ Allocate memory PASS 2 ]
;   IN : d0 - Size of memory block (.l)
;        d1 - Memory type (.b)
;  OUT : d0 - 0 - No success / Memory handle (.b)
; Changed registers : d0
; NOTE :
;  - This pass seeks the area of the right type with the most TFM and
;    collects garbage. After that it calls Pass 1.
Pass_2:
	movem.l	d1-d7/a0-a6,-(sp)
	Push	busy_Mptr,Memory_Mptr
	move.l	d0,d4
; ---------- Seek area with largest TFM -----------
	lea.l	Memory_areas,a0
	moveq.l	#0,d2
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	move.b	Memory_type(a0),d3		; Right type ?
	btst	d3,d1
	beq.s	.Next
	jsr	Calculate_TFM		; Calculate TFM
	cmp.l	d0,d2			; Larger ?
	bpl.s	.Next
	move.l	d0,d2			; Set
	movea.l	a0,a1
.Next:	lea.l	Area_data_size(a0),a0	; Next area
.Entry:	dbra	d7,.Loop
	cmp.l	d4,d2			; Large enough ?
	bpl.s	.Go_on
	moveq.l	#0,d0			; No success
	bra	.Exit
; ---------- Do it --------------------------------
.Go_on:	jsr	Collect_garbage		; Collect garbage
	move.l	d4,d0			; Try
	jsr	Pass_1
.Exit:	Pop	busy_Mptr
	movem.l	(sp)+,d1-d7/a0-a6
	rts

; [ Allocate memory PASS 9 ]
;   IN : d0 - Size of memory block (.l)
;        d1 - Memory type (.b)
;  OUT : d0 - 0 - No success / Memory handle (.b)
; Changed registers : d0
; NOTE :
;  - This pass tries to move unallocated CHIP blocks to FAST areas if CHIP
;    memory is required.
Pass_9:
	movem.l	d1-d7/a0-a6,-(sp)
	cmp.b	#CHIP,d1			; Chip ?
	beq.s	.Yes
.Out:	moveq.l	#0,d0			; No!
	bra	.Exit
.Yes:	tst.l	Total_FAST_memory		; Any FAST ?
	beq.s	.Out
	Push	busy_Mptr,Memory_Mptr
	move.l	d0,-(sp)			; Save
	move.w	d1,-(sp)
; ---------- Seek CHIP areas for unused CHIP blocks --
	lea.l	Memory_areas,a1
	move.w	Number_of_areas,d7
	bra	.Entry
.Loop:	cmp.b	#CHIP_area,Memory_type(a1)	; CHIP area ?
	bne	.Next
	move.l	a1,a3			; Yes -> Search area
	moveq.l	#0,d6
.Again:	move.l	Block_next(a3),d0		; End of chain ?
	beq	.Done
	move.l	d0,a3			; No
	tst.l	Block_info(a3)		; Free ?
	beq.s	.Again
	btst	#Allocated,Block_flags(a3)	; No -> Allocated ?
	bne.s	.Again
	moveq.l	#0,d0			; No -> File ?
	move.b	Block_file_index(a0),d0
	beq.s	.Again
	lea.l	File_infos,a0		; Yes -> Get file info
	subq.w	#1,d0
	mulu.w	#File_data_size,d0
	add.w	d0,a0
	move.b	File_type(a0),d1		; Get file type
	jsr	Get_file_memory_type	; Get memory type
	cmp.b	#CHIP,d1			; CHIP block ?
	bne.s	.Again
	move.b	Block_handle(a3),d5		; Yes -> Save handle
	move.l	Block_size(a3),d0		; Try to allocate
	move.b	#FAST,d1
	jsr	Pass_1
	tst.b	d0			; Success ?
	bne.s	.Do
	move.l	Block_size(a3),d0		; No -> Try pass 2 (!)
	jsr	Pass_2
	tst.b	d0			; Success ?
	beq.s	.Again
.Do:	jsr	Find_entry		; Yes -> Find entry
	bne.s	.Again
	jsr	Destroy_memory_handle	; Destroy handle
	add.l	Block_size(a3),d6		; Count up
	jsr	Move_memory_block		; Move it
	beq	.Again			; Success ?
	sub.l	Block_size(a0),d6		; No -> Count down
	clr.l	Block_info(a0)		; Merge new block
	jsr	Merge_memory_block
	bra	.Again
.Done:	tst.l	d6			; Any luck ?
	beq.s	.Next
	jsr	Collect_garbage		; Yes -> Collect garbage
.Next:	lea.l	Area_data_size(a1),a1	; Next area
.Entry:	dbra	d7,.Loop
	move.w	(sp)+,d1			; Restore
	move.l	(sp)+,d0
	Pop	busy_Mptr
	jsr	Pass_1			; Try
.Exit:	movem.l	(sp)+,d1-d7/a0-a6
	rts

; [ Allocate memory PASS 3 ]
;   IN : d0 - Size of memory block (.l)
;        d1 - Memory type (.b)
;  OUT : d0 - 0 - No success / Memory handle (.b)
; Changed registers : d0
; NOTE :
;  - This pass seeks the area of the right type with the most EMG, drowns
;    old memory blocks and collects garbage. After that it calls pass 1.
Pass_3:
	movem.l	d1-d7/a0-a6,-(sp)
	Push	busy_Mptr,Memory_Mptr
	move.l	d0,d4
	moveq.l	#0,d6			; Start priority = 0
; ---------- Seek area with largest EMG -----------
.Again:	lea.l	Memory_areas,a0
	moveq.l	#0,d2
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	move.b	Memory_type(a0),d3		; Right type ?
	btst	d3,d1
	beq.s	.Next
	jsr	Calculate_TFM		; Calculate TFM
	move.l	d0,d3
	move.b	d6,d0			; Calculate EMG
	jsr	Calculate_EMG
	add.l	d3,d0			; Add
	cmp.l	d0,d2			; Larger ?
	bpl.s	.Next
	move.l	d0,d2			; Set
	movea.l	a0,a1
.Next:	lea.l	Area_data_size(a0),a0	; Next area
.Entry:	dbra	d7,.Loop
	cmp.l	d4,d2			; Large enough ?
	bmi.s	.Up
	movea.l	a1,a0			; Sacrifice memory blocks
	move.b	d6,d0
	jsr	Drown_memory
	jsr	Collect_garbage		; Collect garbage
	jsr	Find_LFB			; Find largest block
	cmp.l	Block_size(a0),d4		; Large enough ?
	bls.s	.Go_on
.Up:	addq.b	#1,d6			; Increase priority
	bne.s	.Again
	moveq.l	#0,d0			; No success
	bra	.Exit
; ---------- Do it --------------------------------
.Go_on:	move.l	d4,d0			; Allocate
	jsr	Pass_1
.Exit:	Pop	busy_Mptr
	movem.l	(sp)+,d1-d7/a0-a6
	rts

; [ Allocate memory PASS 4 ]
;   IN : d1 - Memory type (.b)
;        a1 - Pointer to passes list (.l)
;  OUT : d0 - 0 (.b)
;        d1 - FAST if it was CHIP before (.b)
;        a1 - Reset to start of passes list (.l)
; Changed registers : d0,d1,a1
; NOTE :
;  - If the memory block should be in FAST memory, this pass changes the
;    required memory type to CHIP and goes through all the passes again.
Pass_4:
	moveq.l	#0,d0			; No way
	cmpi.b	#FAST,d1			; Should it be FAST ?
	bne.s	.Exit
	lea.l	Passes,a1			; Try all	passes again
	move.b	#CHIP,d1			; CHIP this time
.Exit:	rts

; [ Allocate memory PASS 5 ]
;   IN : d0 - Size of memory block (.l)
;        d1 - Memory type (.b)
;  OUT : d0 - 0 - No success / Memory handle (.b)
; Changed registers : d0
; NOTES :
;  - This pass tries to move FAST blocks in CHIP areas to FAST areas if CHIP
;    memory is required.
;  - Armageddon is called, so Pass 1 is sufficient to find free fast memory.
Pass_5:
	movem.l	d1-d7/a0-a6,-(sp)
	cmp.b	#CHIP,d1			; Chip ?
	beq.s	.Yes
.Out:	moveq.l	#0,d0			; No!
	bra	.Exit
.Yes:	tst.l	Total_FAST_memory		; Any FAST ?
	beq.s	.Out
	Push	busy_Mptr,Memory_Mptr
	move.l	d0,-(sp)			; Save
	move.w	d1,-(sp)
	jsr	Armageddon		; Kill! Kill! Kill!
; ---------- Seek CHIP areas for FAST blocks ------
	lea.l	Memory_areas,a1
	move.w	Number_of_areas,d7
	bra	.Entry
.Loop:	cmp.b	#CHIP_area,Memory_type(a1)	; CHIP area ?
	bne	.Next
	move.l	a1,a3			; Yes -> Search area
	moveq.l	#0,d6
.Again:	move.l	Block_next(a3),d0		; End of chain ?
	beq.s	.Done
	move.l	d0,a3			; No
	tst.l	Block_info(a3)		; Free ?
	beq.s	.Again
	tst.b	Block_claim(a3)		; No -> Claimed ?
	bne.s	.Again
	moveq.l	#0,d0			; No -> File ?
	move.b	Block_file_index(a0),d0
	beq.s	.Again
	lea.l	File_infos,a0		; Yes -> Get file info
	subq.w	#1,d0
	mulu.w	#File_data_size,d0
	add.w	d0,a0
	move.b	File_type(a0),d1		; Get file type
	jsr	Get_file_memory_type	; Get memory type
	cmp.b	#FAST,d1			; FAST block ?
	bne.s	.Again
	move.l	Block_size(a3),d0		; Yes -> Try to allocate
	jsr	Pass_1
	tst.b	d0			; Success ?
	beq.s	.Again
	jsr	Find_entry		; Yes -> Find entry
	bne.s	.Again
	jsr	Destroy_memory_handle	; Destroy handle
	add.l	Block_size(a3),d6		; Count up
	jsr	Move_memory_block		; Move it
	beq.s	.Again			; Success ?
	sub.l	Block_size(a0),d6		; No -> Count down
	clr.l	Block_info(a0)		; Merge new block
	jsr	Merge_memory_block
	bra.s	.Again
.Done:	tst.l	d6			; Any luck ?
	beq.s	.Next
	jsr	Collect_garbage		; Yes -> Collect garbage
.Next:	lea.l	Area_data_size(a1),a1	; Next area
.Entry:	dbra	d7,.Loop
	move.w	(sp)+,d1			; Restore
	move.l	(sp)+,d0
	Pop	busy_Mptr
	jsr	Pass_1			; Try
.Exit:	movem.l	(sp)+,d1-d7/a0-a6
	rts

; [ Allocate memory PASS 6 ]
;   IN : d0 - Size of memory block (.l)
;        d1 - Memory type (.b)
;  OUT : d0 - 0 - No success / Memory handle (.b)
; Changed registers : d0
; NOTES :
;  - This pass tries to move DONTCARE blocks in CHIP areas to FAST areas if CHIP
;    memory is required.
;  - Armageddon has been called in Pass 5, so Pass 1 is sufficient to find free
;    fast memory.
Pass_6:
	movem.l	d1-d7/a0-a6,-(sp)
	cmp.b	#CHIP,d1			; Chip ?
	beq.s	.Yes
.Out:	moveq.l	#0,d0			; No!
	bra	.Exit
.Yes:	tst.l	Total_FAST_memory		; Any FAST ?
	beq.s	.Out
	Push	busy_Mptr,Memory_Mptr
	move.l	d0,-(sp)			; Save
	move.w	d1,-(sp)
; ---------- Seek CHIP areas for DONTCARE blocks --
	lea.l	Memory_areas,a1
	move.w	Number_of_areas,d7
	bra	.Entry
.Loop:	cmp.b	#CHIP_area,Memory_type(a1)	; CHIP area ?
	bne	.Next
	move.l	a1,a3			; Yes -> Search area
	moveq.l	#0,d6
.Again:	move.l	Block_next(a3),d0		; End of chain ?
	beq.s	.Done
	move.l	d0,a3			; No
	tst.l	Block_info(a3)		; Free ?
	beq.s	.Again
	tst.b	Block_claim(a3)		; No -> Claimed ?
	bne.s	.Again
	moveq.l	#0,d0			; No -> File ?
	move.b	Block_file_index(a0),d0
	beq.s	.Again
	lea.l	File_infos,a0		; Yes -> Get file info
	subq.w	#1,d0
	mulu.w	#File_data_size,d0
	add.w	d0,a0
	move.b	File_type(a0),d1		; Get file type
	jsr	Get_file_memory_type	; Get memory type
	cmp.b	#DONTCARE,d1		; DONTCARE block ?
	bne.s	.Again
	move.l	Block_size(a3),d0		; Yes -> Try to allocate
	move.b	#FAST,d1
	jsr	Pass_1
	tst.b	d0			; Success ?
	beq.s	.Again
	jsr	Find_entry		; Yes -> Find entry
	bne.s	.Again
	jsr	Destroy_memory_handle	; Destroy handle
	add.l	Block_size(a0),d6		; Count up
	jsr	Move_memory_block		; Move it
	beq.s	.Again			; Success ?
	sub.l	Block_size(a0),d6		; No -> Count down
	clr.l	Block_info(a0)		; Merge new block
	jsr	Merge_memory_block
	bra.s	.Again
.Done:	tst.l	d6			; Any luck ?
	beq.s	.Next
	jsr	Collect_garbage		; Yes -> Collect garbage
.Next:	lea.l	Area_data_size(a1),a1	; Next area
.Entry:	dbra	d7,.Loop
	move.w	(sp)+,d1			; Restore
	move.l	(sp)+,d0
	Pop	busy_Mptr
	jsr	Pass_1			; Try
.Exit:	movem.l	(sp)+,d1-d7/a0-a6
	rts

; [ Allocate memory PASS 7 ]
;   IN : d1 - Memory type (.b)
;        a1 - Pointer to passes list (.l)
;  OUT : d0 - 0 (.b)
;        a1 - Reset to start of passes list (.l)
; Changed registers : d0,a1
; NOTE :
;  - If the program is in the combat screen, this pass dumps all the map
;    files and goes through all the passes again.
Pass_7:
	moveq.l	#0,d0			; No way
	tst.b	Reload_map_data		; Already dumped ?
	bne.s	.Exit
	jsr	Remove_map_data		; No
	tst.b	Reload_map_data		; Dumped now ?
	beq.s	.Exit
	lea.l	Passes,a1			; Yes -> Try again
.Exit:	rts

; [ Allocate memory PASS 8 ]
;   IN : d0 - Size of memory block (.l)
;        d1 - Memory type (.b)
;  OUT : d0 - 0 - No success / Memory handle (.b)
; Changed registers : d0
; NOTE :
;  - If CHIP memory is needed and music is activated, this pass deactivates
;    music and goes through all the passes again.
Pass_8:
	move.l	d2,-(sp)
	move.l	d0,d2			; Save
	moveq.l	#0,d0			; Default is no luck
	tst.l	Total_FAST_memory		; Got any FAST ?
	beq.s	.Do
	cmp.b	#CHIP,d1			; Yes -> CHIP required ?
	bne.s	.Exit
.Do:	tst.b	Music_flag		; Music on ?
	beq.s	.Exit
	jsr	Music_off			; Off!
	jsr	Removing_music
	sf	Restart_music		; No need
	jsr	Armageddon		; Kill! Kill! Kill!
	move.l	d2,d0			; Try again
	jsr	Pass_1
.Exit:	move.l	(sp)+,d2
	rts

;*****************************************************************************
; [ Drown EVERYTHING & collect garbage ]
; All registers are restored
;*****************************************************************************
Armageddon:
	movem.l	d0/d7/a0/a1,-(sp)
	lea.l	Memory_areas,a0
	move.b	#255,d0
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	jsr	Drown_memory
	move.l	a0,a1
	jsr	Collect_garbage
	lea.l	Area_data_size(a0),a0
.Entry:	dbra	d7,.Loop
	movem.l	(sp)+,d0/d7/a0/a1
	rts

;*****************************************************************************
; [ Allocate a memory block ]
;   IN : d0 - Required size (.l)
;        a0 - Pointer to memory entry (.l)
;  OUT : d0 - Memory handle (.b)
; Changed registers : d0
; NOTE :
;  - This routine is called by the allocation passes.
;*****************************************************************************
Allocate_block:
	movem.l	d1/a1,-(sp)
	clr.l	Block_info(a0)		; Clear info
	bset	#Allocated,Block_flags(a0)	; Allocate
	cmp.l	Block_size(a0),d0		; Right size ?
	beq.s	.Go_on
	jsr	Split_memory_block		; Split block &
	movea.l	Block_next(a0),a1		;  try to merge the new
	exg.l	a0,a1			;  block to the next one
	jsr	Merge_memory_block
	move.l	a1,a0
.Go_on:	jsr	Create_memory_handle	; Create handle
	movem.l	(sp)+,d1/a1
	rts

;*****************************************************************************
; [ Collect garbage	in a memory area ]
;   IN : a1 - Pointer to memory area (.l)
; All registers are restored
; NOTE :
;  - This routine is called by the allocation passes.
;  - Optimal garbage collection may be impossible because of claimed blocks.
;*****************************************************************************
Collect_garbage:
	movem.l	d0/a0/a3,-(sp)
	movea.l	a1,a3
.Again:	move.l	Block_next(a3),d0		; End of list ?
	beq.s	.End
	movea.l	d0,a3			; No -> Next block
	tst.l	Block_info(a3)		; Free block ?
	beq.s	.Free
	tst.b	Block_claim(a3)		; No -> Claimed ?
	bne.s	.Again
	jsr	Relocate_memory_block	; No -> Try to relocate
	bra.s	.Again
.Free:	move.l	a3,a0			; Try to merge free block
	jsr	Merge_memory_block
	move.l	a0,a3
	bra.s	.Again
.End:	movem.l	(sp)+,d0/a0/a3
	rts

;*****************************************************************************
; [ Relocate memory block ]
;   IN : a1 - Pointer to memory area (.l)
;        a3 - Pointer to source memory entry (.l)
;  OUT : a3 - Pointer to source memory entry (empty if successful) (.l)
; Changed registers : a3
; Notes :
;   - This routine finds it's own target memory in the same area, in front
;     of the source memory entry.
;   - This routine is only called by [Collect_garbage].
;   - This routine may assume that the source memory entry remains valid.
;*****************************************************************************
Relocate_memory_block:
	movem.l	d0/a0-a2,-(sp)
	moveq.l	#0,d0			; File ?
	move.b	Block_file_index(a3),d0
	beq.s	.No_file
	lea.l	File_infos,a2		; Yes -> Get file info
	subq.w	#1,d0
	mulu.w	#File_data_size,d0
	add.w	d0,a2
	moveq.l	#0,d0			; Get file type
	move.b	File_type(a2),d0
.No_file:	jsr	Get_file_relocator		; Get relocator
	cmp.l	#0,a2			; If any
	beq.s	.Exit
	jsr	Find_LFB_in_garbage		; Find a destination
	cmpa.l	#0,a0			; Any ?
	beq	.Exit
	st	Handles_invalid		; Flag
	move.l	Block_size(a3),d0		; Compare sizes
	cmp.l	Block_size(a0),d0
	beq.s	.Right			; Just right ?
	bmi.s	.Split			; No -> Too large ?
	move.l	Block_previous(a3),a0	; No -> Too small
	cmp.l	#0,a0			; Is there a previous block ?
	beq	.Exit
	tst.l	Block_info(a0)		; Yes -> Is it free ?
	bne	.Exit
	move.l	a0,-(sp)			; Yes -> Relocate
	move.l	Block_start(a0),a1
	move.l	Block_start(a3),a0
	jsr	(a2)
	move.l	(sp)+,a0
	move.l	Block_size(a0),Block_size(a3)	; Switch
	move.l	d0,Block_size(a0)
	add.l	Block_start(a0),d0
	move.l	d0,Block_start(a3)
	bra.s	.Done
.Split:	jsr	Split_memory_block		; Split block
.Right:	move.l	a0,-(sp)			; Relocate
	move.l	Block_start(a0),a1
	move.l	Block_start(a3),a0
	jsr	(a2)
	move.l	(sp)+,a0
.Done:	move.l	Block_info(a3),Block_info(a0)	; Copy information
	move.l	a3,a0			; Kill original
	clr.l	Block_info(a0)
	jsr	Merge_memory_block		; Try to merge
	move.l	a0,a3
.Exit:	movem.l	(sp)+,d0/a0-a2
	rts

;*****************************************************************************
; [ Find the Largest Free Block in a memory area ]
;   IN : a1 - Pointer to memory area (.l)
;        a3 - Pointer to last memory block (.l)
;  OUT : a0 - Pointer to memory entry (LFB) (.l)
; Changed registers : a0
; Notes :
;   - This routine is only called by [ Collect_garbage ].
;*****************************************************************************
Find_LFB_in_garbage:
	movem.l	d0/a1,-(sp)
	moveq.l	#0,d0			; Largest = 0
	sub.l	a0,a0
.Again:	tst.l	Block_next(a1)		; End of list ?
	beq.s	.Done
	movea.l	Block_next(a1),a1		; No
	cmpa.l	a3,a1			; Reached last block ?
	beq.s	.Done
	tst.l	Block_info(a1)		; Free ?
	bne.s	.Again
	cmp.l	Block_size(a1),d0		; Larger ?
	bpl.s	.Again
	move.l	Block_size(a1),d0		; Yes
	move.l	a1,a0
	bra.s	.Again
.Done:	movem.l	(sp)+,d0/a1
	rts

;*****************************************************************************
; [ Move memory block ]
;   IN : a0 - Pointer to target memory entry (.l)
;        a3 - Pointer to source memory entry (.l)
;  OUT : a3 - Pointer to source memory entry (empty if successful) (.l)
;        eq - Success
;        ne - No success
; Changed registers : a3
; Notes :
;   - Unlike [ Relocate_memory_block ], this routine doesn't find it's own
;     target memory.
;   - This routine is called by the allocation passes.
;   - This routine may assume that the source memory entry is valid.
;*****************************************************************************
Move_memory_block:
	movem.l	d0/d7/a0-a2,-(sp)
	moveq.l	#-1,d7			; Default is no luck
	moveq.l	#0,d0			; File ?
	move.b	Block_file_index(a3),d0
	beq.s	.No_file
	lea.l	File_infos,a2		; Yes -> Get file info
	subq.w	#1,d0
	mulu.w	#File_data_size,d0
	add.w	d0,a2
	moveq.l	#0,d0			; Get file type
	move.b	File_type(a2),d0
.No_file:	jsr	Get_file_relocator		; Get relocator
	cmp.l	#0,a2			; If any
	beq.s	.Exit
	st	Handles_invalid		; Flag
	move.l	a0,-(sp)			; Relocate
	move.l	Block_start(a0),a1
	move.l	Block_start(a3),a0
	move.l	Block_size(a3),d0
	jsr	(a2)
	move.l	(sp)+,a0
	move.l	Block_info(a3),Block_info(a0)	; Copy information
	move.l	a3,a0			; Kill original
	clr.l	Block_info(a0)
	jsr	Merge_memory_block		; Try to merge
	move.l	a0,a3
	moveq.l	#0,d7			; Yay!
.Exit:	tst.w	d7			; Any luck ?
	movem.l	(sp)+,d0/d7/a0-a2
	rts

;*****************************************************************************
; [ Find the Largest Free Block in a memory area ]
;   IN : a0 - Pointer to memory area (.l)
;  OUT : a0 - Pointer to memory entry (LFB) (.l)
; Changed registers : a0
;*****************************************************************************
Find_LFB:
	movem.l	d0/a1,-(sp)
	moveq.l	#0,d0			; Largest = 0
	sub.l	a1,a1
.Again:	tst.l	Block_next(a0)		; End of list ?
	beq.s	.Done
	movea.l	Block_next(a0),a0
	tst.l	Block_info(a0)		; Free ?
	bne.s	.Again
	cmp.l	Block_size(a0),d0		; Larger ?
	bpl.s	.Again
	move.l	Block_size(a0),d0		; Yes
	move.l	a0,a1
	bra.s	.Again
.Done:	move.l	a1,a0			; Store pointer to entry
	movem.l	(sp)+,d0/a1
	rts

;*****************************************************************************
; [ Calculate the Total Free Memory in a memory area ]
;   IN : a0 - Pointer to memory area (.l)
;  OUT : d0 - Total Free Memory (.l)
; Changed registers : d0
;*****************************************************************************
Calculate_TFM:
	movem.l	d1/a0,-(sp)
	moveq.l	#0,d0			; TFM = 0
.Again:	move.l	Block_next(a0),d1		; End of list ?
	beq.s	.Done
	movea.l	d1,a0
	tst.l	Block_info(a0)		; Free ?
	bne.s	.Again
	add.l	Block_size(a0),d0		; Yes
	bra.s	.Again
.Done:	movem.l	(sp)+,d1/a0
	rts

;*****************************************************************************
; [ Calculate the Extra Memory Gained in a memory area ]
;   IN : d0 - Maximum priority (.b)
;        a0 - Pointer to memory area (.l)
;  OUT : d0 - Extra Memory Gained (.l)
; Changed registers : d0
;*****************************************************************************
Calculate_EMG:
	movem.l	d1/d6/a0/a1,-(sp)
	move.b	d0,d6
	moveq.l	#0,d0			; EMG = 0
.Again:	move.l	Block_next(a0),d1		; End of list ?
	beq.s	.Exit
	movea.l	d1,a0			; No
	btst	#Allocated,Block_flags(a0)	; Allocated ?
	bne.s	.Again
	jsr	Find_file_info		; No -> Is it a file ?
	bne.s	.Again
	cmp.b	File_priority(a1),d6	; Priority low enough ?
	bcs.s	.Again
	add.l	Block_size(a0),d0		; Yes
	bra.s	.Again
.Exit:	movem.l	(sp)+,d1/d6/a0/a1
	rts

;*****************************************************************************
; [ Drown all memory blocks below sea level ]
;   IN : d0 - Sea level (= minimum priority) (.b)
;        a0 - Pointer to area (.l)
; All registers are restored
;*****************************************************************************
Drown_memory:
	movem.l	d1/a0/a1,-(sp)
.Again:	move.l	Block_next(a0),d1		; End of chain ?
	beq.s	.Exit
	move.l	d1,a0			; No
	btst	#Allocated,Block_flags(a0)	; Allocated ?
	bne.s	.Again
	jsr	Find_file_info		; No -> Is it a file ?
	bne.s	.Again
	cmp.b	File_priority(a1),d0	; Yes -> Below sea level ?
	bcs.s	.Again
	clr.l	Block_info(a0)		; Yes -> Clear information
	clr.b	File_type(a1)
	jsr	Merge_memory_block		; Try to merge
	bra.s	.Again
.Exit:	movem.l	(sp)+,d1/a0/a1
	rts

;*****************************************************************************
; [ Check if memory block can be merged with previous and/or next ]
;   IN : a0 - Pointer to memory entry (.l)
;  OUT : a0 - Pointer to memory entry (.l)
; Changed registers : a0
; NOTE :
;  - a0 points to the merged memory entry, or to the original entry.
;*****************************************************************************
Merge_memory_block:
	movem.l	d0/a1/a2,-(sp)
	cmp.l	#0,a0			; Real block ?
	beq	.Exit
	tst.l	Block_info(a0)		; Block occupied ?
	bne	.Exit
	tst.l	Block_previous(a0)		; Is it the area ?
	beq	.Exit
	movea.l	Block_previous(a0),a2	; Get previous
	tst.l	Block_previous(a2)		; Is THIS the area ?
	beq	.Try_next
	tst.l	Block_info(a2)		; Block occupied ?
	bne	.Try_next
	move.l	Block_size(a0),d0		; Merge with previous
	add.l	d0,Block_size(a2)
	jsr	Delete_entry		; Delete entry
	movea.l	a2,a0
.Try_next:
	tst.l	Block_next(a0)		; End of chain ?
	beq	.Exit
	movea.l	Block_next(a0),a2		; Get next
	tst.l	Block_info(a2)		; Block occupied ?
	bne	.Exit
	move.l	Block_size(a2),d0		; Merge with next
	add.l	d0,Block_size(a0)
	move.l	a2,a0			; Delete entry
	jsr	Delete_entry
.Exit:	movem.l	(sp)+,d0/a1/a2
	rts

;*****************************************************************************
; [ Split a memory block ]
;   IN : d0 - Size of first block (.l)
;        a0 - Pointer to memory block (.l)
; All registers are restored
; NOTES :
;   - The start & size of the first block will be set.
;   - The start & size of the second block will be set.
;   - The links will be set properly for both blocks.
;   - The flags, identifier, handle & claim of the second block will be
;     erased (i.e. the second block is a block of free memory).
;   - The area number of the second block will be set.
;*****************************************************************************
Split_memory_block:
	movem.l	d1/a0/a1,-(sp)
	addq.l	#1,d0			; Force word boundary
	andi.b	#$fe,d0
	cmp.l	Block_size(a0),d0		; Same size or larger ?
	bpl.s	.Exit
	movea.l	a0,a1			; Create a new entry
	jsr	Find_free_entry
	jsr	Add_entry
	move.l	Block_size(a1),d1		; Adjust size of new entry
	sub.l	d0,d1
	move.l	d1,Block_size(a0)
	move.l	Block_start(a1),d1		; Adjust start of new entry
	add.l	d0,d1
	move.l	d1,Block_start(a0)
	move.l	d0,Block_size(a1)		; Set size of old entry
.Exit:	movem.l	(sp)+,d1/a0/a1
	rts

;*****************************************************************************
; [ Set the priority of an entry ]
;   IN : a0 - Pointer to memory entry (.l)
; All registers are restored
;*****************************************************************************
Set_priority:
	movem.l	d1/a1,-(sp)
	jsr	Find_file_info		; Get file info
	bne.s	.Exit
	moveq.l	#0,d1			; Get file type
	move.b	File_type(a1),d1
	beq.s	.Exit
	jsr	Get_file_priority		; Get priority
	ori.b	#$80,d1			; Add base priority
	move.b	d1,File_priority(a1)	; Set
.Exit:	movem.l	(sp)+,d1/a1
	rts

;*****************************************************************************
; [ Find the memory area belonging to a memory block ]
;   IN : a0 - Pointer to memory block (.l)
;  OUT : a1 - Pointer to memory area descriptor (.l)
; All registers are restored
;*****************************************************************************
Find_memory_area:
	move.l	d0,-(sp)
	move.l	a0,a1
.Again:	move.l	Block_previous(a1),d0	; Is there a previous block ?
	beq.s	.Exit
	move.l	d0,a1			; Yes -> Go there
	bra.s	.Again
.Exit:	move.l	(sp)+,d0
	rts

;*****************************************************************************
; [ Validate all memory handles ]
; All registers are restored
;*****************************************************************************
Validate_handles:
	movem.l	d0/d7/a0-a2,-(sp)
	jsr	Clear_all_handles		; Clear all handles
	lea.l	Memory_areas,a1		; Search all areas
	lea.l	Memory_handles,a2
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	move.l	a1,a0			; Search area
.Again:	move.l	Block_next(a0),d0		; End of chain ?
	beq.s	.Next
	move.l	d0,a0
	moveq.l	#0,d0			; Get handle
	move.b	Block_handle(a0),d0
	beq.s	.Again
	lsl.w	#2,d0			; Insert
	move.l	a0,-4(a2,d0.w)
	bra.s	.Again
.Next:	lea.l	Area_data_size(a1),a1	; Next area
.Entry:	dbra	d7,.Loop
	movem.l	(sp)+,d0/d7/a0-a2
	rts

;*****************************************************************************
; [ Kill unclaimed memory ]
; All registers are restored
;*****************************************************************************
Kill_unclaimed_memory:
	movem.l	d0/d7/a0-a2,-(sp)
	lea.l	Memory_areas,a2		; Search all areas
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	move.l	a2,a0			; Search area
.Again:	move.l	Block_next(a0),d0		; End of chain ?
	beq.s	.Next
	move.l	d0,a0			; No
	tst.l	Block_info(a0)		; Block occupied ?
	beq.s	.Again
	tst.b	Block_claim(a0)		; Yes -> Claimed ?
	bne.s	.Again
	jsr	Find_file_info		; No -> Find file info
	bne.s	.No_file
	clr.b	File_type(a1)		; Clear file info
.No_file:	jsr	Destroy_memory_handle	; Destroy handle
	clr.l	Block_info(a0)		; Erase info
	jsr	Merge_memory_block		; Try to merge
	bra.s	.Again
.Next:	lea.l	Area_data_size(a2),a2	; Next area
.Entry:	dbra	d7,.Loop
.Exit:	movem.l	(sp)+,d0/d7/a0-a2
	rts

;***************************************************************************	
; The DATA & BSS segments
;***************************************************************************	
	SECTION	Fast_BSS,bss
Original_length:	ds.l 1
Restart_music:	ds.b 1
Handles_invalid:	ds.b 1
	even
