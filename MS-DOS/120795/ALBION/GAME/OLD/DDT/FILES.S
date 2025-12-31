; File handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	XDEF	Load_subfile	
	XDEF	Load_unique_subfile
	XDEF	Load_batch_of_subfiles
	XDEF	Load_batch_of_unique_subfiles
	XDEF	Load_file
	XDEF	Save_file
	XDEF	Save_encrypted_file
	XDEF	Save_subfile
	XDEF	Decompress
	XDEF	Current_filename
	XDEF	Batch

; NOTES :
;  - Files should be at least as long as [ File_header_size ] !

	SECTION	Program,code
;***************************************************************************
; [ Initialize the file handling ]
; All registers are restored
;***************************************************************************
Init_files:
	movem.l	d0/d1/a0/a1,-(sp)
; ---------- Set current directory ----------------
	kickDOS	GetProgramDir		; Get home directory
	move.l	d0,d1			; Make current directory
	kickDOS	CurrentDir
	move.l	d0,Old_current_dir
	move.l	#Data_directory,d1		; Get lock on data directory
	move.l	#ACCESS_READ,d2
	kickDOS	Lock
	tst.l	d0			; Error ?
	bne.s	.Ok
	move.l	#DATA_DIR_NOT_FOUND,d0	; Yes
	jmp	Fatal_error
.Ok:	move.l	d0,Data_dir_lock		; No
	move.l	d0,d1			; Make current directory
	kickDOS	CurrentDir
; ---------- Redirect error requesters ------------
	move.l	My_task_ptr,a0		; First in own task
	move.l	pr_WindowPtr(a0),Old_WindowPtr1
	move.l	#-1,pr_WindowPtr(a0)
	kickDOS	GetFileSysTask		; Get file system task
	tst.l	d0
	beq.s	.Error
	move.l	d0,a0			; Here too
	lea.l	-pr_MsgPort(a0),a0
	move.l	pr_WindowPtr(a0),Old_WindowPtr2
	move.l	My_window,pr_WindowPtr(a0)
.Error:	LOCAL
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************
; [ Restore the file handling ]
; All registers are restored
;***************************************************************************
Exit_files:
	movem.l	d0/d1/a0/a1,-(sp)
; ---------- Restore error requesters -------------
	move.l	My_task_ptr,a0		; First in own task
	move.l	Old_WindowPtr1,pr_WindowPtr(a0)
	kickDOS	GetFileSysTask		; Get file system task
	tst.l	d0
	beq.s	.Error
	move.l	d0,a0			; Here too
	lea.l	-pr_MsgPort(a0),a0
	move.l	Old_WindowPtr2,pr_WindowPtr(a0)
; ---------- Restore current directory ------------
.Error:	move.l	Old_current_dir,d1
	kickDOS	CurrentDir
	move.l	Data_dir_lock,d1
	kickDOS	UnLock
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;*****************************************************************************
; [ Disk access ON ]
; All registers are restored
;*****************************************************************************
Disk_access_ON:
	tst.b	Access_flag		; Accessing ?
	bne.s	.Exit
	st	Access_flag		; No -> ON
	tst.b	Graphics_ops
	beq.s	.Exit
	jsr	End_graphics_ops
	st	Reactivate_graphics_ops
.Exit:	rts

;*****************************************************************************
; [ Disk access OFF ]
; All registers are restored
;*****************************************************************************
Disk_access_OFF:
	tst.b	Access_flag		; Accessing ?
	beq.s	.Exit
	sf	Access_flag		; Yes -> OFF
	tst.b	Reactivate_graphics_ops
	beq.s	.Exit
	sf	Reactivate_graphics_ops
	jsr	Begin_graphics_ops
.Exit:	rts

;*****************************************************************************
; [ Open an Omnifile & read the header	]
;   IN : a5 - Pointer to local variables (.l)
; All registers are	restored
; NOTES :
;   - [ Omnifile_handle(a5) ] is the pointer to the file handle.
;   - [ Omnifile_header(a5) ] is the header of the Omnifile.
;   - [ Nr_of_subfiles(a5) ] is the number of subfiles.
;   - [ Omni_lengths_list ] contains the length list.
;   - The seek pointer will be on the start of the first subfile.
;*****************************************************************************
Open_Omnifile:
	movem.l	d0-d3/a0/a1,-(sp)
	clr.l	Omnifile_handle(a5)	; Clear handle
; --------- Open Omnifile ------------------------
	move.w	Omnifile_number(a5),d0	; Get pointer to filename
	jsr	Get_Omnifile_name
	move.l	a0,Current_filename		; Store
	move.l	a0,d1			; Open file
	move.l	#MODE_OLDFILE,d2
	kickDOS	Open
	tst.l	d0			; Error ?
	beq	.Error
	move.l	d0,Omnifile_handle(a5)	; Store handle
	move.l	d0,Current_file_handle
; --------- Read header ---------------------------
	move.l	Omnifile_handle(a5),d1	; Read header
	move.l	a5,d2
	add.l	#Omnifile_header,d2
	moveq.l	#6,d3
	kickDOS	Read
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; Entire header read ?
	beq.s	.Ok
.Read:	move.l	#INCOMPLETE_READ,d0		; No
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
; --------- Read subfile length list --------------
.Ok:	move.l	Omnifile_handle(a5),d1	; Read length list
	move.l	#Omni_lengths_list,d2
	moveq.l	#0,d3
	move.w	Nr_of_subfiles(a5),d3
	lsl.l	#2,d3
	kickDOS	Read
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; Entire list read ?
	bne	.Read
.Exit:	movem.l	(sp)+,d0-d3/a0/a1
	rts

; --------- Error recovery ------------------------
.Error:	jsr	Get_disk_error		; Get error code
	movem.l	(sp)+,d0-d3/a0/a1
	jsr	Close_Omnifile
	jsr	Disk_error		; Handle error
	bra	Open_Omnifile

	rsreset
Omnifile_header:	rs.l 1			; Positions are important !
Nr_of_subfiles:	rs.w 1
Omnifile_handle:	rs.l 1
Open_OMNI_LDS:	rs.b 0

;*****************************************************************************
; [ Close an Omnifile ]
;   IN : a5 - Pointer to local variables (.l)
; All registers are	restored
;*****************************************************************************
Close_Omnifile:
	jmp	Close_current_file

;*****************************************************************************
; [ Load a subfile from an Omnifile ]
;   IN : d0 - Subfile number {1...} (.w)
;        d1 - File type {1...} (.w)
;  OUT : d0 - Subfile memory handle (.b)
; Changed registers : d0
;*****************************************************************************
Load_subfile:
	movem.l	d1-d7/a0-a6,-(sp)
	lea.l	-Load_subfile_LDS(sp),sp	; Create local variables
	movea.l	sp,a5
	tst.w	d0			; Exit if zero
	beq	.Exit2
	clr.b	Subfile_handle(a5)		; Clear handle
	move.w	d0,Subfile_number(a5)	; Save input
	move.w	d1,Omnifile_number(a5)
	jsr	Reallocate_memory		; Already	present ?
	tst.b	d0
	bne	.Exit2
	jsr	Disk_access_ON		; No
	lea.l	Disk_Mptr,a0		; Change mouse pointer
	jsr	Push_busy_Mptr
	jsr	Open_Omnifile		; Open Omnifile
	move.w	Subfile_number(a5),d1	; Legal subfile number ?
	cmp.w	Nr_of_subfiles(a5),d1
	ble.s	.Ok
	jsr	Close_Omnifile		; No -> close
	bra	.Exit
.Ok:	lea.l	Omni_lengths_list,a0	; Calculate offset
	moveq.l	#0,d0
	move.w	Subfile_number(a5),d1
	subq.w	#1,d1
	bra.s	.Entry
.Loop:	add.l	(a0)+,d0
.Entry:	dbra	d1,.Loop
	move.l	d0,Subfile_offset(a5)	; Store
	move.l	(a0),Subfile_length(a5)	; Store subfile length
	jsr	Do_load_subfile		; Load
	jsr	Close_Omnifile		; Close
.Exit:	jsr	Pop_busy_Mptr		; Restore old mouse pointer
	jsr	Disk_access_OFF
	move.b	Subfile_handle(a5),d0	; Output
	bne.s	.Exit2			; Error ?
	move.l	#SUBFILE_NOT_LOADED,d0	; Yes
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Exit2:	lea.l	Load_subfile_LDS(sp),sp	; Destroy	local variables
	movem.l	(sp)+,d1-d7/a0-a6
	rts

	rsreset
	rs.b Open_OMNI_LDS
Omnifile_number:	rs.w 1
Subfile_number:	rs.w 1
Subfile_offset:	rs.l 1
Subfile_length:	rs.l 1
Subfile_handle:	rs.b 1
	rseven
Subfile_address:	rs.l 1
Load_subfile_LDS:   rs.b 0

;*****************************************************************************
; [ Load a UNIQUE (i.e. not re-allocated) subfile from an Omnifile ]
;   IN : d0 - Subfile number {1...} (.w)
;        d1 - File type {1...} (.w)
;  OUT : d0 - Subfile memory handle (.b)
; Changed registers : d0
;*****************************************************************************
Load_unique_subfile:
	movem.l	d1-d7/a0-a6,-(sp)
	lea.l	-Load_subfile_LDS(sp),sp	; Create local variables
	movea.l	sp,a5
	tst.w	d0			; Exit if zero
	beq	.Exit2
	clr.b	Subfile_handle(a5)		; Clear handle
	move.w	d0,Subfile_number(a5)	; Save input
	move.w	d1,Omnifile_number(a5)
	jsr	Duplicate_memory		; Can be duplicated ?
	tst.b	d0
	bne	.Exit2
	jsr	Disk_access_ON		; No
	lea.l	Disk_Mptr,a0		; Change mouse pointer
	jsr	Push_busy_Mptr
	jsr	Open_Omnifile		; Open Omnifile
	move.w	Subfile_number(a5),d1	; Legal subfile number ?
	cmp.w	Nr_of_subfiles(a5),d1
	ble.s	.Ok
	jsr	Close_Omnifile		; No -> close
	bra	.Exit
.Ok:	lea.l	Omni_lengths_list,a0	; Calculate offset
	moveq.l	#0,d0
	move.w	Subfile_number(a5),d1
	subq.w	#1,d1
	bra.s	.Entry
.Loop:	add.l	(a0)+,d0
.Entry:	dbra	d1,.Loop
	move.l	d0,Subfile_offset(a5)	; Store
	tst.l	(a0)			; Length = zero ?
	bne.s	.Full
	move.b	#-1,Subfile_handle(a5)	; Yes
	bra.s	.Done
.Full:	move.l	(a0),Subfile_length(a5)	; Store subfile length
	jsr	Do_load_subfile		; Load subfile
.Done:	jsr	Close_Omnifile		; Close
.Exit:	jsr	Pop_busy_Mptr		; Restore old mouse pointer
	jsr	Disk_access_OFF
	move.b	Subfile_handle(a5),d0	; Output
	bne.s	.Exit2			; Error ?
	move.l	#SUBFILE_NOT_LOADED,d0	; Yes
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Exit2:	lea.l	Load_subfile_LDS(sp),sp	; Destroy	local variables
	movem.l	(sp)+,d1-d7/a0-a6
	rts

;*****************************************************************************
; [ Load a batch of subfiles from an Omnifile ]
;   IN : d0 - Number of subfiles in batch (.w)
;        d1 - Omnifile number {1...} (.w)
;        a0 - Pointer to list of subfile numbers {1...} (words) (.l)
;        a1 - Pointer to destination list of memory handles (bytes) (.l)
; All registers are restored
; Notes :
;   - The batch is sorted but not destroyed.
;   - Subfiles with number 0 will result in handle 0.
;*****************************************************************************
Load_batch_of_subfiles:
	movem.l	d0-d7/a0-a6,-(sp)
	st	Batch_mode
; ---------- Check --------------------------------
	cmp.w	#1,d0			; Just one ?
	bne.s	.Not_1
	move.w	(a0),d0			; Yes -> Just load it
	jsr	Load_subfile
	move.b	d0,(a1)
	bra	.Exit3
; ---------- Do -----------------------------------
.Not_1:	lea.l	-Load_batch_LDS(sp),sp	; Create local variables
	movea.l	sp,a5
	move.w	d0,Subfiles_in_batch(a5)	; Save input
	move.w	d1,Omnifile_number(a5)
	move.l	a1,Subfile_handle_list(a5)
	tst.w	d0			; Load any files ?
	beq	.Exit2
	jsr	Prepare_batch_load		; Yes
; ---------- Try to re-allocate all subfiles ------
	move.l	a0,-(sp)
	lea.l	Desort_list,a3
	move.w	Omnifile_number(a5),d1
	move.w	Subfiles_in_batch(a5),d7
	move.w	d7,d6
	subq.w	#1,d7
.Loop5:	move.w	(a0),d0			; Get subfile number
	beq.s	.Zero
	jsr	Reallocate_memory		; Already	present ?
	tst.b	d0
	beq.s	.Next5
.Zero:	subq.w	#1,d6			; Yes -> Count down
	move.w	(a3),d2			; Desort & store handle
	move.b	d0,0(a1,d2.w)
	move.w	#-1,(a3)			; Mark
.Next5:	addq.l	#2,a0			; Next subfile
	addq.l	#2,a3
	dbra	d7,.Loop5
	move.l	(sp)+,a0
	tst.w	d6			; Any left ?
	beq	.Exit2
; ---------- Load all subfiles --------------------
	jsr	Disk_access_ON		; Yes
	move.l	a0,-(sp)			; Change mouse pointer
	lea.l	Disk_Mptr,a0
	jsr	Push_busy_Mptr
	move.l	(sp)+,a0
	jsr	Open_Omnifile		; Open Omnifile
	clr.l	Batch_offset(a5)		; Clear !
	clr.l	Subfile_length(a5)
	lea.l	Desort_list,a3
	moveq.l	#0,d6
	move.w	Subfiles_in_batch(a5),d7
	subq.w	#1,d7
.Loop3:	cmp.w	#-1,(a3)			; Already done ?
	bne.s	.Load
	addq.l	#2,a0			; Yes
	addq.l	#2,a3
	bra	.Next3b
.Load:	move.w	(a0)+,d0			; Get subfile number
	tst.w	d6			; First time ?
	beq.s	.First
	cmp.w	-4(a0),d0			; No -> Same as previous ?
	beq	.Store
.First:	move.w	d0,Subfile_number(a5)	; Store subfile number
	clr.b	Subfile_handle(a5)		; Clear handle
	tst.w	d0			; Legal subfile number ?
	ble	.Next3
	cmp.w	Nr_of_subfiles(a5),d0
	bgt	.Next3
	lea.l	Omni_lengths_list,a2	; Yes -> Calculate offset
	moveq.l	#0,d0
	move.w	Subfile_number(a5),d1
	subq.w	#1,d1
	bra.s	.Entry4
.Loop4:	add.l	(a2)+,d0
.Entry4:	dbra	d1,.Loop4
	move.l	d0,d1			; Relative to previous
	sub.l	Batch_offset(a5),d0
	move.l	d1,Batch_offset(a5)
	sub.l	Subfile_length(a5),d0
	move.l	d0,Subfile_offset(a5)	; Store
	move.l	(a2),Subfile_length(a5)	; Store subfile length
	jsr	Do_load_subfile		; Load subfile
.Next3:	tst.b	Subfile_handle(a5)		; Error ?
	bne.s	.Store
	move.l	#SUBFILE_NOT_LOADED,d0	; Yes
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Store:	move.w	(a3)+,d0			; Desort & store handle
	move.b	Subfile_handle(a5),0(a1,d0.w)
.Next3b:	addq.w	#1,d6			; Next subfile
	dbra	d7,.Loop3
	jsr	Close_Omnifile		; Close
.Exit:	jsr	Pop_busy_Mptr		; Restore old mouse pointer
	jsr	Disk_access_OFF
.Exit2:	lea.l	Load_batch_LDS(sp),sp	; Destroy	local variables
.Exit3:	sf	Batch_mode
	movem.l	(sp)+,d0-d7/a0-a6
	rts

	rsreset
	rs.b Load_subfile_LDS
Subfile_handle_list:	rs.l 1
Subfiles_in_batch:	rs.w 1
Batch_offset:	rs.l 1
Load_batch_LDS:     rs.b 0

;*****************************************************************************
; [ Load a batch of unique subfiles from an Omnifile ]
;   IN : d0 - Number of subfiles in batch (.w)
;        d1 - Omnifile number {1...} (.w)
;        a0 - Pointer to list of subfile numbers {1...} (words) (.l)
;        a1 - Pointer to destination list of memory handles (bytes) (.l)
; All registers are restored
; Notes :
;   - The batch is sorted but not destroyed.
;   - Subfiles with number 0 will result in handle 0.
;*****************************************************************************
Load_batch_of_unique_subfiles:
	movem.l	d0-d7/a0-a6,-(sp)
	st	Batch_mode
; ---------- Check --------------------------------
	cmp.w	#1,d0			; Just one ?
	bne.s	.Not_1
	move.w	(a0),d0			; Yes -> Just load it
	bclr	#15,d0
	bne.s	.Unique1
	jsr	Load_subfile
	bra.s	.Store1
.Unique1:	jsr	Load_unique_subfile
.Store1:	move.b	d0,(a1)
	bra	.Exit3
; ---------- Do -----------------------------------
.Not_1:	lea.l	-Load_batch_LDS(sp),sp	; Create local variables
	movea.l	sp,a5
	move.w	d0,Subfiles_in_batch(a5)	; Save input
	move.w	d1,Omnifile_number(a5)
	move.l	a1,Subfile_handle_list(a5)
	tst.w	d0			; Load any files ?
	beq	.Exit2
	jsr	Prepare_batch_load		; Yes
; ---------- Try to re-allocate all subfiles ------
	move.l	a0,-(sp)
	lea.l	Desort_list,a3
	move.w	Omnifile_number(a5),d1
	move.w	Subfiles_in_batch(a5),d7
	move.w	d7,d6
	subq.w	#1,d7
.Loop5:	move.w	(a0),d0			; Get subfile number
	beq.s	.Zero
	bclr	#15,d0			; Unique ?
	bne.s	.Unique2
	jsr	Reallocate_memory		; Already	present ?
	bra.s	.Skip
.Unique2:	jsr	Duplicate_memory		; Can be duplicated ?
.Skip:	tst.b	d0
	beq.s	.Next5
.Zero:	subq.w	#1,d6			; Yes -> Count down
	move.w	(a3),d2			; Desort & store handle
	move.b	d0,0(a1,d2.w)
	move.w	#-1,(a3)			; Mark
.Next5:	addq.l	#2,a0			; Next subfile
	addq.l	#2,a3
	dbra	d7,.Loop5
	move.l	(sp)+,a0
	tst.w	d6			; Any left ?
	beq	.Exit2
; ---------- Load all subfiles --------------------
	jsr	Disk_access_ON		; Yes
	move.l	a0,-(sp)			; Change mouse pointer
	lea.l	Disk_Mptr,a0
	jsr	Push_busy_Mptr
	move.l	(sp)+,a0
	jsr	Open_Omnifile		; Open Omnifile
	clr.l	Batch_offset(a5)		; Clear !
	clr.l	Subfile_length(a5)
	lea.l	Desort_list,a3
	moveq.l	#0,d6
	move.w	Subfiles_in_batch(a5),d7
	subq.w	#1,d7
.Loop3:	cmp.w	#-1,(a3)			; Already done ?
	bne.s	.Load
	addq.l	#2,a0			; Yes
	addq.l	#2,a3
	bra	.Next3b
.Load:	move.w	(a0)+,d0			; Get subfile number
	bclr	#15,d0			; Unique ?
	bne.s	.First
	tst.w	d6			; No -> First time ?
	beq.s	.First
	cmp.w	-4(a0),d0			; No -> Same as previous ?
	beq	.Store2
.First:	move.w	d0,Subfile_number(a5)	; Store subfile number
	clr.b	Subfile_handle(a5)		; Clear handle
	tst.w	d0			; Legal subfile number ?
	ble	.Next3
	cmp.w	Nr_of_subfiles(a5),d0
	bgt	.Next3
	lea.l	Omni_lengths_list,a2	; Yes -> Calculate offset
	moveq.l	#0,d0
	move.w	Subfile_number(a5),d1
	subq.w	#1,d1
	bra.s	.Entry4
.Loop4:	add.l	(a2)+,d0
.Entry4:	dbra	d1,.Loop4
	move.l	d0,d1			; Relative to previous
	sub.l	Batch_offset(a5),d0
	move.l	d1,Batch_offset(a5)
	sub.l	Subfile_length(a5),d0
	move.l	d0,Subfile_offset(a5)	; Store
	move.l	(a2),Subfile_length(a5)	; Store subfile length
	jsr	Do_load_subfile		; Load subfile
.Next3:	tst.b	Subfile_handle(a5)		; Error ?
	bne.s	.Store2
	move.l	#SUBFILE_NOT_LOADED,d0	; Yes
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Store2:	move.w	(a3)+,d0			; Desort & store handle
	move.b	Subfile_handle(a5),0(a1,d0.w)
.Next3b:	addq.w	#1,d6			; Next subfile
	dbra	d7,.Loop3
	jsr	Close_Omnifile		; Close
.Exit:	jsr	Pop_busy_Mptr		; Restore old mouse pointer
	jsr	Disk_access_OFF
.Exit2:	lea.l	Load_batch_LDS(sp),sp	; Destroy	local variables
.Exit3:	sf	Batch_mode
	movem.l	(sp)+,d0-d7/a0-a6
	rts

;*****************************************************************************
; [ Prepare batch load ]
;   IN : a0 - Pointer to list of subfile numbers {1...} (words) (.l)
; All registers are	restored
; NOTES :
;*****************************************************************************
Prepare_batch_load:
	movem.l	d0-d7/a0/a2,-(sp)
; ---------- Fill desort list ---------------------
	lea.l	Desort_list,a2
	moveq.l	#0,d1
	move.w	d0,d7
	subq.w	#1,d7
.Loop1:	move.w	d1,(a2)+
	addq.w	#1,d1
	dbra	d7,.Loop1
; ---------- Sort subfiles -------------------------
	subq.l	#2,a0			; Base 0 correction
	lea.l	Desort_list-2,a2
	move.w	d0,d5			; For I =	1 to Size-1
	add.w	d5,d5
	moveq.l	#2,d7
.Loop2:	move.w	2(a0,d7.w),d2		; Data(I)	> Data(I+1) ?
	cmp.w	0(a0,d7.w),d2
	bpl.s	.Next2
	move.w	0(a0,d7.w),d3		; Swap Data(I),Data(I+1)
	move.w	d2,0(a0,d7.w)
	move.w	d3,2(a0,d7.w)
	move.w	2(a2,d7.w),d2
	move.w	0(a2,d7.w),d3
	move.w	d2,0(a2,d7.w)
	move.w	d3,2(a2,d7.w)
	move.w	d7,d6			; P = I -	1
	subq.w	#2,d6
	beq.s	.Next2			; Exit if	P = 0
.Again:	move.w	2(a0,d6.w),d2		; Data(P)	> Data(P+1) ?
	cmp.w	0(a0,d6.w),d2
	bpl.s	.Next2
	move.w	0(a0,d6.w),d3		; Swap Data(P),Data(P+1)
	move.w	d2,0(a0,d6.w)
	move.w	d3,2(a0,d6.w)
	move.w	2(a2,d6.w),d2
	move.w	0(a2,d6.w),d3
	move.w	d2,0(a2,d6.w)
	move.w	d3,2(a2,d6.w)
	subq.w	#2,d6			; Decrease P
	bne.s	.Again			; Repeat if P > 0
.Next2:	addq.w	#2,d7			; Next I
	cmp.w	d5,d7
	bne.s	.Loop2
	movem.l	(sp)+,d0-d7/a0/a2
	rts

;*****************************************************************************
; [ Load an unpacked subfile from an Omnifile ]
;   IN : a5 - Pointer to local variables (.l)
; All registers are	restored
; NOTES :
;   - All	local variables must be set properly.
;   - Seek pointer must be on the start of the first subfile.
;*****************************************************************************
Load_unpacked_subfile:
	movem.l	d0-d3/d7/a0/a1/a4,-(sp)
	tst.l	Subfile_length(a5)		; Length = zero ?
	bne.s	.Full
	tst.b	Batch_mode		; Yes -> Batch mode ?
	beq.s	.No_batch
	move.l	Omnifile_handle(a5),d1	; Yes -> Seek
	move.l	Subfile_offset(a5),d2
	move.l	#OFFSET_CURRENT,d3
	kickDOS	Seek
	tst.l	d0			; Error ?
	bmi	.Error
.No_batch:	moveq.l	#0,d0			; Allocate file memory
	move.w	Omnifile_number(a5),d1
	move.w	Subfile_number(a5),d2
	jsr	File_allocate
	jsr	Clear_memory
	move.b	d0,Subfile_handle(a5)	; Store handle
	bra	.Exit
.Full:	move.l	Omnifile_handle(a5),d1	; Seek to start
	move.l	Subfile_offset(a5),d2
	move.l	#OFFSET_CURRENT,d3
	kickDOS	Seek
	tst.l	d0			; Error ?
	bmi	.Error
	move.l	Subfile_length(a5),d0	; Normal length
	move.w	Omnifile_number(a5),d1	; Allocate file memory
	move.w	Subfile_number(a5),d2
	jsr	File_allocate
	move.b	d0,Subfile_handle(a5)	; Store handle
	jsr	Claim_pointer		; Get actual address
	move.l	d0,Subfile_address(a5)
	move.l	Omnifile_handle(a5),d1	; Read subfile
	move.l	d0,d2
	move.l	Subfile_length(a5),d3
	kickDOS	Read
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; All bytes read ?
	beq	.Ok
	move.l	#INCOMPLETE_READ,d0		; No
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Ok:	move.b	Subfile_handle(a5),d0	; Free pointer
	jsr	Free_pointer
.Exit:	movem.l	(sp)+,d0-d3/d7/a0/a1/a4
	rts

; ---------- Error recovery -----------------------
.Error:	jsr	Get_disk_error		; Get error code
	move.b	Subfile_handle(a5),d0	; Free memory
	beq.s	.None			;  (if necessary)
	jsr	Free_pointer
	jsr	Free_memory
.None:	jsr	Disk_error		; Handle error
	move.l	Omnifile_handle(a5),d1	; Seek to start
	moveq.l	#0,d2
	move.w	Nr_of_subfiles(a5),d2
	lsl.w	#2,d2
	addq.l	#6,d2
	move.l	#OFFSET_BEGINNING,d3
	kickDOS	Seek
	movem.l	(sp)+,d0-d3/d7/a0/a1/a4
	bra	Load_unpacked_subfile	; Retry

;*****************************************************************************
; [ Load a packed subfile from an Omnifile ]
;   IN : a5 - Pointer to local variables (.l)
; All registers are	restored
; NOTES :
;   - All	local variables must be set properly.
;   - Seek pointer must be on the start of the first subfile.
;*****************************************************************************
Load_packed_subfile:
	movem.l	d0-d3/d7/a0/a1/a4,-(sp)
	tst.l	Subfile_length(a5)		; Length = zero ?
	bne.s	.Full
	tst.b	Batch_mode		; Yes -> Batch mode ?
	beq.s	.No_batch
	move.l	Omnifile_handle(a5),d1	; Yes -> Seek
	move.l	Subfile_offset(a5),d2
	move.l	#OFFSET_CURRENT,d3
	kickDOS	Seek
	tst.l	d0			; Error ?
	bmi	.Error
.No_batch:	moveq.l	#0,d0			; Allocate file memory
	move.w	Omnifile_number(a5),d1
	move.w	Subfile_number(a5),d2
	jsr	File_allocate
	jsr	Clear_memory
	move.b	d0,Subfile_handle(a5)	; Store handle
	bra	.Exit2
.Full:	move.w	Omnifile_number(a5),d0	; Get decompression factor
	jsr	Get_safety_factor
	move.l	Omnifile_handle(a5),d1	; Seek to start
	move.l	Subfile_offset(a5),d2
	move.l	#OFFSET_CURRENT,d3
	kickDOS	Seek
	tst.l	d0			; Error ?
	bmi	.Error
	lea.l	File_header,a4		; Read file header
	move.l	Omnifile_handle(a5),d1
	move.l	a4,d2
	moveq.l	#8,d3
	kickDOS	Read
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; All bytes read ?
	beq.s	.Ok
.Read:	move.l	#INCOMPLETE_READ,d0		; No
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Ok:	cmpi.l	#Packed_ID,(a4)		; Yes -> Packed ?
	beq	.Packed
	move.l	Subfile_length(a5),d0	; No -> Just load
	subq.l	#4,d0
	move.w	Omnifile_number(a5),d1	; Allocate file memory
	move.w	Subfile_number(a5),d2
	jsr	File_allocate
	move.b	d0,Subfile_handle(a5)	; Store handle
	jsr	Claim_pointer		; Get actual address
	move.l	d0,Subfile_address(a5)
	movea.l	d0,a0			; Copy file header
	addq.l	#4,a4
	move.l	(a4),(a0)+
	move.l	Omnifile_handle(a5),d1	; Read subfile
	move.l	a0,d2
	move.l	Subfile_length(a5),d3
	subq.l	#8,d3
	kickDOS	Read
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; All bytes read ?
	bne	.Read
	bra	.Exit			; Yes
.Packed:	move.l	4(a4),d0			; Packed -> Get unpacked length
	andi.l	#$00ffffff,d0
	move.l	d0,d7
	add.l	Safety_factor,d0		; Add safety factor
	move.w	Omnifile_number(a5),d1	; Allocate file memory
	move.w	Subfile_number(a5),d2
	jsr	File_allocate
	move.b	d0,Subfile_handle(a5)	; Store handle
	jsr	Claim_pointer		; Get actual address
	move.l	d0,Subfile_address(a5)
	movea.l	d0,a0			; Copy file header
	move.l	(a4)+,(a0)+
	move.l	(a4)+,(a0)+
	move.l	Omnifile_handle(a5),d1	; Read subfile
	move.l	a0,d2
	move.l	Subfile_length(a5),d3
	subq.l	#8,d3
	kickDOS	Read
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; All bytes read ?
	bne	.Read
	movea.l	Subfile_address(a5),a0	; Decompress subfile
	jsr	Decompress
	move.b	Subfile_handle(a5),d0	; Return unused memory
	move.l	d7,d1
	jsr	Shrink_memory
.Exit:	move.b	Subfile_handle(a5),d0	; Free pointer
	jsr	Free_pointer
.Exit2:	movem.l	(sp)+,d0-d3/d7/a0/a1/a4
	rts

.Error:	jsr	Get_disk_error		; Get error code
	move.b	Subfile_handle(a5),d0	; Free memory
	beq.s	.None			;  (if necessary)
	jsr	Free_pointer
	jsr	Free_memory
.None:	jsr	Disk_error		; Handle error
	move.l	Omnifile_handle(a5),d1	; Seek to start
	moveq.l	#0,d2
	move.w	Nr_of_subfiles(a5),d2
	lsl.w	#2,d2
	addq.l	#6,d2
	move.l	#OFFSET_BEGINNING,d3
	kickDOS	Seek
	movem.l	(sp)+,d0-d3/d7/a0/a1/a4
	bra	Load_packed_subfile		; Retry

;*****************************************************************************
; [ Load an unpacked and encrypted subfile from an Omnifile ]
;   IN : a5 - Pointer to local variables (.l)
; All registers are	restored
; NOTES :
;   - All	local variables must be set properly.
;   - Seek pointer must be on the start of the first subfile.
;*****************************************************************************
Load_unpacked_encrypted_subfile:
	jsr	Load_unpacked_subfile	; Load subfile
	movem.l	d0/d7/a0,-(sp)
	move.w	Subfile_number(a5),d0	; Decrypt file
	move.l	Subfile_length(a5),d7
	move.l	Subfile_address(a5),a0	; (is still valid)
	jsr	Crypt_block
	movem.l	(sp)+,d0/d7/a0
	rts

;*****************************************************************************
; [ Load a packed and encrypted subfile from an Omnifile ]
;   IN : a5 - Pointer to local variables (.l)
; All registers are	restored
; NOTES :
;   - All	local variables must be set properly.
;   - Seek pointer must be on the start of the first subfile.
;*****************************************************************************
Load_packed_encrypted_subfile:
	movem.l	d0-d3/d7/a0/a1/a4,-(sp)
	tst.l	Subfile_length(a5)		; Length = zero ?
	bne.s	.Full
	tst.b	Batch_mode		; Yes -> Batch mode ?
	beq.s	.No_batch
	move.l	Omnifile_handle(a5),d1	; Yes -> Seek
	move.l	Subfile_offset(a5),d2
	move.l	#OFFSET_CURRENT,d3
	kickDOS	Seek
	tst.l	d0			; Error ?
	bmi	.Error
.No_batch:	moveq.l	#0,d0			; Allocate file memory
	move.w	Omnifile_number(a5),d1
	move.w	Subfile_number(a5),d2
	jsr	File_allocate
	jsr	Clear_memory
	move.b	d0,Subfile_handle(a5)	; Store handle
	bra	.Exit2
.Full:	move.w	Omnifile_number(a5),d0	; Get decompression factor
	jsr	Get_safety_factor
	move.l	Omnifile_handle(a5),d1	; Seek to start
	move.l	Subfile_offset(a5),d2
	move.l	#OFFSET_CURRENT,d3
	kickDOS	Seek
	tst.l	d0			; Error ?
	bmi	.Error
	lea.l	File_header,a4		; Read file header
	move.l	Omnifile_handle(a5),d1
	move.l	a4,d2
	moveq.l	#8,d3
	kickDOS	Read
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; All bytes read ?
	beq.s	.Ok
.Read:	move.l	#INCOMPLETE_READ,d0		; No
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Ok:	cmpi.l	#Packed_ID,(a4)		; Yes -> Packed ?
	beq	.Packed
	move.l	Subfile_length(a5),d0	; No -> Just load
	subq.l	#4,d0
	move.w	Omnifile_number(a5),d1	; Allocate file memory
	move.w	Subfile_number(a5),d2
	jsr	File_allocate
	move.b	d0,Subfile_handle(a5)	; Store handle
	jsr	Claim_pointer		; Get actual address
	move.l	d0,Subfile_address(a5)
	movea.l	d0,a0			; Copy file header
	addq.l	#4,a4
	move.l	(a4),(a0)+
	move.l	Omnifile_handle(a5),d1	; Read subfile
	move.l	a0,d2
	move.l	Subfile_length(a5),d3
	subq.l	#8,d3
	kickDOS	Read
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; All bytes read ?
	bne	.Read
	move.l	d7,-(sp)			; Decrypt file
	move.w	Subfile_number(a5),d0
	move.l	d3,d7
	addq.l	#4,d7
	move.l	Subfile_address(a5),a0
	jsr	Crypt_block
	move.l	(sp)+,d7
	bra	.Exit			; Yes
.Packed:	move.l	4(a4),d0			; Get unpacked length
	andi.l	#$00ffffff,d0
	move.l	d0,d7
	add.l	Safety_factor,d0		; Add safety factor
	move.w	Omnifile_number(a5),d1	; Allocate file memory
	move.w	Subfile_number(a5),d2
	jsr	File_allocate
	move.b	d0,Subfile_handle(a5)	; Store handle
	jsr	Claim_pointer		; Get actual address
	move.l	d0,Subfile_address(a5)
	movea.l	d0,a0			; Copy file header
	move.l	(a4)+,(a0)+
	move.l	(a4)+,(a0)+
	move.l	Omnifile_handle(a5),d1	; Read subfile
	move.l	a0,d2
	move.l	Subfile_length(a5),d3
	subq.l	#8,d3
	kickDOS	Read
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; All bytes read ?
	bne	.Read
	move.l	d7,-(sp)			; Decrypt file
	move.w	Subfile_number(a5),d0
	move.l	d3,d7
	move.l	Subfile_address(a5),a0
	addq.l	#8,a0
	jsr	Crypt_block
	move.l	(sp)+,d7
	subq.l	#8,a0			; Decompress subfile
	jsr	Decompress
	move.b	Subfile_handle(a5),d0	; Return unused memory
	move.l	d7,d1
	jsr	Shrink_memory
.Exit:	move.b	Subfile_handle(a5),d0	; Free pointer
	jsr	Free_pointer
.Exit2:	movem.l	(sp)+,d0-d3/d7/a0/a1/a4
	rts

.Error:	jsr	Get_disk_error		; Get error code
	move.b	Subfile_handle(a5),d0	; Free memory
	beq.s	.None			;  (if necessary)
	jsr	Free_pointer
	jsr	Free_memory
.None:	jsr	Disk_error		; Handle error
	move.l	Omnifile_handle(a5),d1	; Seek to start
	moveq.l	#0,d2
	move.w	Nr_of_subfiles(a5),d2
	lsl.w	#2,d2
	addq.l	#6,d2
	move.l	#OFFSET_BEGINNING,d3
	kickDOS	Seek
	movem.l	(sp)+,d0-d3/d7/a0/a1/a4
	bra	Load_packed_encrypted_subfile	; Retry

;*****************************************************************************
; [ Load a file ]
;   IN : d0 - 0 / File type {1...} (.w)
;        a0 - Filename (when d0 = 0) (.l)
;  OUT : d0 - File memory handle (.b)
; Changed registers : d0
;*****************************************************************************
Load_file:
	movem.l	d1-d7/a0-a6,-(sp)
	jsr	Disk_access_ON
	lea.l	-Load_file_LDS(sp),sp	; Create local variables
	movea.l	sp,a5
	move.w	d0,LFile_type(a5)		; Store file type
	clr.l	LFile_handle(a5)		; Clear handles & flags
	clr.b	LFile_memory_handle(a5)
	clr.b	LFile_flags(a5)
	tst.w	d0			; Normal file type ?
	beq.s	.Abnormal
	move.w	d0,d1			; Already	present ?
	moveq.l	#0,d0
	jsr	Reallocate_memory
	tst.b	d0
	bne	.Exit2
	move.w	d1,d0			; No -> Get pointer to filename
.Abnormal:	jsr	Get_Omnifile_name
	move.l	a0,Current_filename		; Store
	move.l	a0,LFile_name(a5)
	move.w	LFile_type(a5),d0		; Get decompression factor
	jsr	Get_safety_factor
	lea.l	Disk_Mptr,a0		; Change mouse pointer
	jsr	Push_busy_Mptr
	move.l	LFile_name(a5),a0		; Get file length
	jsr	Get_file_length
	move.l	d0,LFile_length(a5)
	move.l	LFile_name(a5),d1		; Open file
	move.l	#MODE_OLDFILE,d2
	kickDOS	Open
	tst.l	d0			; Error ?
	beq	.Error
	move.l	d0,LFile_handle(a5)		; Store handle
	move.l	d0,Current_file_handle
	lea.l	File_header,a4
	move.l	d0,d1			; Read file header
	move.l	a4,d2
	moveq.l	#File_header_size,d3
	kickDOS	Read
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; All bytes read ?
	beq.s	.Ok
.Read:	move.l	#INCOMPLETE_READ,d0		; No
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Ok:	cmpi.w	#Crypt_ID,(a4)		; Encrypted file ?
	bne.s	.No_crypt
	addq.l	#2,a4			; Yes -> Skip crypt header
	move.w	(a4)+,d0			; Get seed
	eori.w	#Crypt_ID,d0
	moveq.l	#8,d7			; Decrypt rest of header
	move.l	a4,a0
	jsr	Crypt_block
	move.w	d0,LFile_crypt_seed(a5)
	bset	#Encrypted,LFile_flags(a5)
.No_crypt:	cmpi.l	#Packed_ID,(a4)		; Packed file ?
	beq	.Pack
	move.l	LFile_length(a5),d0		; No -> Actual file size
	btst	#Encrypted,LFile_flags(a5)	; Encrypted ?
	beq.s	.Go_on
	subq.l	#4,d0			; Yes -> Subtract crypt header
	bra.s	.Go_on
.Pack:	move.l	4(a4),d0			; Calculate unpacked length
	andi.l	#$00ffffff,d0
	move.l	d0,d6
	add.l	Safety_factor,d0
	bset	#Packed,LFile_flags(a5)
.Go_on:	move.w	LFile_type(a5),d1		; Allocate file memory
	moveq.l	#0,d2
	jsr	File_allocate
	move.b	d0,LFile_memory_handle(a5)	; Store handle
	jsr	Claim_pointer		; Get actual address
	move.l	d0,LFile_address(a5)
	movea.l	d0,a0			; Copy file header
	btst	#Encrypted,LFile_flags(a5)	; (tricky !)
	bne.s	.Yes
	move.l	(a4)+,(a0)+
.Yes:	move.l	(a4)+,(a0)+
	move.l	(a4)+,(a0)+
	move.l	LFile_handle(a5),d1		; Read remainder of file
	move.l	a0,d2
	move.l	LFile_length(a5),d3
	sub.l	#File_header_size,d3
	beq	.End			; Anything left ?
	kickDOS	Read
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; Everything read ?
	bne	.Read
	btst	#Encrypted,LFile_flags(a5)	; Encrypted ?
	beq.s	.No
	move.w	LFile_crypt_seed(a5),d0	; Decrypt rest of file
	move.l	LFile_length(a5),d7
	sub.l	#File_header_size,d7	; (header has been decrypted)
	move.l	LFile_address(a5),a0
	addq.l	#8,a0			; (crypt header is removed)
	jsr	Crypt_block
.No:	btst	#Packed,LFile_flags(a5)	; Packed ?
	beq.s	.End
	movea.l	LFile_address(a5),a0	; Decompress file
	jsr	Decompress
	move.b	LFile_memory_handle(a5),d0	; Return unused memory
	move.l	d6,d1
	jsr	Shrink_memory
.End:	jsr	Close_current_file		; Close file
	move.b	LFile_memory_handle(a5),d0	; Free pointer
	jsr	Free_pointer
.Exit:	Pop	Mptr			; Restore old mouse pointer
.Exit2:	lea.l	Load_file_LDS(sp),sp	; Destroy	local variables
	jsr	Disk_access_OFF
	movem.l	(sp)+,d1-d7/a0-a6
	rts

; --------- Error recovery ------------------------
.Error:	jsr	Get_disk_error		; Get error code
	jsr	Close_current_file		; Close file
	move.b	LFile_memory_handle(a5),d0	; Free memory
	beq.s	.Skip2			;  (if necessary)
	jsr	Free_pointer
	jsr	Free_memory
.Skip2:	move.w	LFile_type(a5),d0		; Restore d0
	Pop	Mptr			; Restore old mouse pointer
	lea.l	Load_file_LDS(sp),sp	; Destroy	local variables
	jsr	Disk_access_OFF
	movem.l	(sp)+,d1-d7/a0-a6
	jsr	Disk_error		; Handle error
	bra	Load_file			; Retry

	rsreset
LFile_type:	rs.w 1
LFile_length:	rs.l 1
LFile_name:	rs.l 1
LFile_handle:	rs.l 1
LFile_address:	rs.l 1
LFile_memory_handle:	rs.b 1
LFile_flags:	rs.b 1
LFile_crypt_seed:	rs.w 1
Load_file_LDS:	rs.b 0

;*****************************************************************************
; [ Save a file ]
;   IN : d0 - File length (.l)
;        d1 - 0 / File type {1...} (.w)
;        a0 - File address (.l)
;        a1 - Filename (when d1 = 0) (.l)
; All registers are restored
;*****************************************************************************
Save_file:
	movem.l	d0-d3/a0/a1/a5,-(sp)
	jsr	Disk_access_ON
	move.l	a0,-(sp)			; Change mouse pointer
	lea.l	Disk_Mptr,a0
	jsr	Push_busy_Mptr
	move.l	(sp)+,a0
	lea.l	-Load_file_LDS(sp),sp	; Create local variables
	movea.l	sp,a5
	move.l	d0,LFile_length(a5)		; Store input
	move.w	d1,LFile_type(a5)
	move.l	a0,LFile_address(a5)
	clr.l	LFile_handle(a5)		; Clear handle
	tst.w	d1			; Normal file type ?
	bne.s	.Yes
	moveq.l	#0,d0			; No -> Take filename
	move.l	a1,a0
	bra.s	.Go_on
.Yes:	move.w	d1,d0			; Yes
.Go_on:	jsr	Get_Omnifile_name		; Get pointer to filename
	move.l	a0,Current_filename		; Store
	move.l	a0,LFile_name(a5)
	move.l	a0,d1			; Open file
	move.l	#MODE_NEWFILE,d2
	kickDOS	Open
	tst.l	d0			; Error ?
	beq	.Error
	move.l	d0,LFile_handle(a5)		; Store handle
	move.l	d0,Current_file_handle
	move.l	d0,d1			; Write file
	move.l	LFile_address(a5),d2
	move.l	LFile_length(a5),d3
	kickDOS	Write
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; Everything written ?
	beq.s	.Ok
	move.l	#INCOMPLETE_WRITE,d0		; No
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Ok:	jsr	Close_current_file		; Close file
.Exit:	lea.l	Load_file_LDS(sp),sp	; Destroy	local variables
	Pop	Mptr			; Restore old mouse pointer
	jsr	Disk_access_OFF
	movem.l	(sp)+,d0-d3/a0/a1/a5
	rts

; --------- Error recovery ------------------------
.Error:	jsr	Get_disk_error		; Get error code
	jsr	Close_current_file		; Close file
	lea.l	Load_file_LDS(sp),sp	; Destroy	local variables
	Pop	Mptr			; Restore old mouse pointer
	jsr	Disk_access_OFF
	movem.l	(sp)+,d0-d3/a0/a1/a5
	jsr	Disk_error		; Handle error
	bra	Save_file			; Retry

;*****************************************************************************
; [ Encrypt and save a file ]
;   IN : d0 - File length (.l)
;        d1 - 0 / File type {1...} (.w)
;        a0 - File address (.l)
;        a1 - Filename (when d1 = 0) (.l)
; All registers are restored
;*****************************************************************************
Save_encrypted_file:
	movem.l	d0-d3/d7/a0/a1/a4/a5,-(sp)
	jsr	Disk_access_ON
	move.l	a0,-(sp)			; Change mouse pointer
	lea.l	Disk_Mptr,a0
	jsr	Push_busy_Mptr
	move.l	(sp)+,a0
	lea.l	-Load_file_LDS(sp),sp	; Create local variables
	movea.l	sp,a5
	move.l	d0,LFile_length(a5)		; Store input
	move.w	d1,LFile_type(a5)
	move.l	a0,LFile_address(a5)
	clr.l	LFile_handle(a5)		; Clear handle
	tst.w	d1			; Normal file type ?
	bne.s	.Yes
	moveq.l	#0,d0			; No -> Take filename
	move.l	a1,a0
	bra.s	.Go_on
.Yes:	move.w	d1,d0			; Yes
.Go_on:	jsr	Get_Omnifile_name		; Get pointer to filename
	move.l	a0,Current_filename		; Store
	move.l	a0,LFile_name(a5)
	move.l	a0,d1			; Open file
	move.l	#MODE_NEWFILE,d2
	kickDOS	Open
	tst.l	d0			; Error ?
	beq	.Error
	move.l	d0,LFile_handle(a5)		; Store handle
	move.l	d0,Current_file_handle
	lea.l	File_header,a4		; Make file header
	move.w	#Crypt_ID,(a4)
	jsr	Random			; Set seed
	move.w	d0,LFile_crypt_seed(a5)
	eori.w	#Crypt_ID,d0
	move.w	d0,2(a4)
	move.l	LFile_handle(a5),d1		; Write file header
	move.l	a4,d2
	moveq.l	#4,d3
	kickDOS	Write
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; Everything written ?
	beq.s	.Ok
.Write:	move.l	#INCOMPLETE_WRITE,d0	; No
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Ok:	move.w	LFile_crypt_seed(a5),d0	; Encrypt file
	move.l	LFile_length(a5),d7
	move.l	LFile_address(a5),a0
	jsr	Crypt_block
	bset	#Encrypted,LFile_flags(a5)
	move.l	LFile_handle(a5),d1		; Write remainder of file
	move.l	a0,d2
	move.l	d7,d3
	kickDOS	Write
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; Everything written ?
	bne	.Write
	jsr	Close_current_file		; Close file
	move.w	LFile_crypt_seed(a5),d0	; Decrypt file
	move.l	LFile_length(a5),d7
	move.l	LFile_address(a5),a0
	jsr	Crypt_block
.Exit:	lea.l	Load_file_LDS(sp),sp	; Destroy	local variables
	Pop	Mptr			; Restore old mouse pointer
	jsr	Disk_access_OFF
	movem.l	(sp)+,d0-d3/d7/a0/a1/a4/a5
	rts

; --------- Error recovery ------------------------
.Error:	jsr	Get_disk_error		; Get error code
	jsr	Close_current_file		; Close file
	lea.l	Load_file_LDS(sp),sp	; Destroy	local variables
	Pop	Mptr			; Restore old mouse pointer
	jsr	Disk_access_OFF
	movem.l	(sp)+,d0-d3/d7/a0/a1/a4/a5
	jsr	Disk_error		; Handle error
	bra	Save_encrypted_file		; Retry

;*****************************************************************************
; [ Save a subfile to an Omnifile ]
;   IN : d0 - Subfile number {1...} (.w)
;        d1 - File type {1...} (.w)
;        a0 - File address (.l)
; All registers are restored
;*****************************************************************************
Save_subfile:
	movem.l	d0-d7/a0/a5,-(sp)
	jsr	Disk_access_ON
	lea.l	-Load_subfile_LDS(sp),sp	; Create local variables
	movea.l	sp,a5
	move.w	d0,Subfile_number(a5)	; Save input
	move.w	d1,Omnifile_number(a5)
	move.l	a0,Subfile_address(a5)
	cmpi.w	#Max_file_type,d1		; Legal file type ?
	bpl	.Exit2
	tst.w	d0			; Legal subfile number ?
	ble	.Exit2
	cmp.w	#Max_subfiles,d0
	bgt	.Exit2
	lea.l	Disk_Mptr,a0		; Change mouse pointer
	jsr	Push_busy_Mptr
	jsr	Open_Omnifile		; Open Omnifile
	move.w	Subfile_number(a5),d1	; Legal subfile number ?
	cmp.w	Nr_of_subfiles(a5),d1
	ble.s	.Ok
.Error:	jsr	Close_Omnifile		; No -> close
	bra	.Exit
.Ok:	cmpi.l	#"OMNI",Omnifile_header(a5)	; Packed or encrypted ?
	bne.s	.Error
	lea.l	Omni_lengths_list,a0	; Calculate offset
	moveq.l	#0,d0
	move.w	Subfile_number(a5),d1
	subq.w	#1,d1
	bra.s	.Entry
.Loop:	add.l	(a0)+,d0
.Entry:	dbra	d1,.Loop
	move.l	d0,Subfile_offset(a5)	; Store
	tst.l	(a0)			; Length = zero ?
	beq.s	.Done
	move.l	(a0),Subfile_length(a5)	; Store subfile length
	jsr	Do_save_subfile
.Done:	jsr	Close_Omnifile		; Close
.Exit:	jsr	Pop_busy_Mptr		; Restore old mouse pointer
.Exit2:	lea.l	Load_subfile_LDS(sp),sp	; Destroy	local variables
	jsr	Disk_access_OFF
	movem.l	(sp)+,d0-d7/a0/a5
	rts

; [ Save actual subfile ]
;   IN : a5 - Pointer to local variables
; All registers are restored
Do_save_subfile:
	movem.l	d0-d3/a0/a1,-(sp)
	move.l	Omnifile_handle(a5),d1	; Seek to start
	move.l	Subfile_offset(a5),d2
	move.l	#OFFSET_CURRENT,d3
	kickDOS	Seek
	tst.l	d0			; Error ?
	bmi	.Error
	move.l	Omnifile_handle(a5),d1	; Write subfile
	move.l	Subfile_address(a5),d2
	move.l	Subfile_length(a5),d3
	kickDOS	Write
	cmp.l	#-1,d0			; Error ?
	beq	.Error
	cmp.l	d0,d3			; All bytes written ?
	beq.s	.Ok
	move.l	#INCOMPLETE_WRITE,d0	; No
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Ok:	movem.l	(sp)+,d0-d3/a0/a1
	rts

.Error:	jsr	Get_disk_error		; Get error code
	jsr	Disk_error		; Handle error
	move.l	Omnifile_handle(a5),d1	; Seek to start
	moveq.l	#0,d2
	move.w	Nr_of_subfiles(a5),d2
	lsl.w	#2,d2
	addq.l	#6,d2
	move.l	#OFFSET_BEGINNING,d3
	kickDOS	Seek
	movem.l	(sp)+,d0-d3/a0/a1
	bra	Do_save_subfile		; Retry

;*****************************************************************************
; [ Get disk error ]
; All registers are restored
;*****************************************************************************
Get_disk_error:
	movem.l	d0/d1/a0/a1,-(sp)
	kickDOS	IoErr			; Get error code
	move.w	d0,Current_error		; Store
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;*****************************************************************************
; [ Handle disk errors ]
; All registers are restored
;*****************************************************************************
Disk_error:
	movem.l	d0-d7/a0-a6,-(sp)
	jsr	Disk_access_OFF
	move.w	Current_error,d0
	jsr	Disk_error_handler
	jsr	Disk_access_ON
	movem.l	(sp)+,d0-d7/a0-a6
	rts

;*****************************************************************************
; [ Select subfile loader ]
;   IN : a5 - Pointer to local variables (.l)
; All registers are	restored
;*****************************************************************************
Do_load_subfile:
	movem.l	d7/a0,-(sp)
	lea.l	.Loaders,a0		; Search table
	move.l	Omnifile_header(a5),d0
	moveq.l	#4-1,d7
.Loop:	cmp.l	(a0),d0			; Is this the one ?
	bne.s	.Next
	move.l	4(a0),a0			; Yes -> Execute
	jsr	(a0)
	bra.s	.Exit
.Next:	addq.l	#8,a0			; No -> Next one
	dbra	d7,.Loop
	move.l	#BAD_OMNIFILE,d0		; Error
	move.l	Current_filename,Extended_error_code+4
	jmp	Fatal_error
.Exit:	movem.l	(sp)+,d7/a0
	rts

.Loaders:	dc.l "OMNI",Load_unpacked_subfile
	dc.l "OMPC",Load_packed_subfile
	dc.l "OMNC",Load_unpacked_encrypted_subfile
	dc.l "OMNP",Load_packed_encrypted_subfile

;*****************************************************************************
; [ Get file length ]
;   IN : a0 - Pointer to filename (.l)
;  OUT : d0 - File length / 0 = error (.l)
; Changed registers : d0
;*****************************************************************************
Get_file_length:
	movem.l	d1/d2/d6/d7/a0/a1/a5,-(sp)
	moveq.l	#0,d6			; Clear
	move.l	a0,d1			; Get lock on file
	move.l	#ACCESS_READ,d2
	kickDOS	Lock
	tst.l	d0			; Error ?
	beq	.Error
	move.l	d0,d6			; Save lock
	move.l	#Info_block,d0		; Examine file
	addq.l	#7,d0
	and.l	#$fffffff8,d0
	move.l	d0,a5
	move.l	d6,d1
	move.l	a5,d2
	kickDOS	Examine
	tst.l	d0			; Error ?
	beq	.Error
	move.l	fib_Size(a5),d7		; Get length
	move.l	d6,d1			; Unlock file
	kickDOS	UnLock
.Exit:	move.l	d7,d0			; Output
	movem.l	(sp)+,d1/d2/d6/d7/a0/a1/a5
	rts

.Error:	jsr	Get_disk_error		; Get error code
	tst.l	d6			; Locked ?
	beq.s	.Zero
	move.l	d6,d1			; Yes -> Unlock
	kickDOS	UnLock
.Zero:	jsr	Disk_error		; Handle error
	movem.l	(sp)+,d1/d2/d6/d7/a0/a1/a5
	bra	Get_file_length

;*****************************************************************************
; [ Get file type's decompression safety factor ]
;   IN : d0 - Omnifile type {0...} (.w)
; All registers are restored
;*****************************************************************************
Get_safety_factor:
	movem.l	d0/a0,-(sp)
	tst.w	d0			; Real file type ?
	bne.s	.Real
	moveq.l	#0,d0			; No -> Take default
	move.w	Default_safety_factor,d0
	bra.s	.Store
.Real:	jsr	Get_file_safety_factor	; Yes -> Get
.Store:	move.l	d0,Safety_factor		; Store
	movem.l	(sp)+,d0/a0
	rts

;*****************************************************************************
; [ Decompress a file ]
;   IN : a0 - Pointer to packed file (.l)
; All registers are restored
;*****************************************************************************
Decompress:   
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	Safety_factor,d0		; Do !
	jsr	.Do
	movem.l	(sp)+,d0-d7/a0-a6
	rts

.Do:	incdir DDT:Core/
	incbin Decompress.img
	even

;*****************************************************************************
; [ Close current file ]
; All registers are restored
;*****************************************************************************
Close_current_file:
	movem.l	d0/d1/a0/a1,-(sp)
	move.l	Current_file_handle,d1	; Anything open ?
	beq.s	.Exit
	tst.b	Access_flag		; Yes -> Accessing ?
	bne.s	.Yes
	jsr	Disk_access_ON		; No
	kickDOS	Close			; Close file
	clr.l	Current_file_handle
	jsr	Disk_access_OFF
	bra.s	.Exit
.Yes:	kickDOS	Close			; Yes -> Just close
	clr.l	Current_file_handle
.Exit:	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************	
	SECTION	Fast_DATA,data
Data_directory:
	dc.b "DDT:Omnifiles/",0
	even

	SECTION	Fast_BSS,bss
Old_current_dir:	ds.l 1
Data_dir_lock:	ds.l 1
Old_WindowPtr1:	ds.l 1
Old_WindowPtr2:	ds.l 1

Safety_factor:	ds.l 1
Omni_lengths_list:	ds.l Max_subfiles
Batch:	ds.w Max_batch
Desort_list:	ds.w Max_batch
Current_error:	ds.w 1			; From IoErr()
Current_filename:	ds.l 1
Current_file_handle:	ds.l 1

Info_block:	ds.b fib_data_size+8
File_header:	ds.b File_header_size
Access_flag:	ds.b 1
Batch_mode:	ds.b 1
Reactivate_graphics_ops:	ds.b 1
	even
