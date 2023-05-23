; File handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	SECTION	Program,code
;***************************************************************************
; [ Initialize the file handling ]
; All registers are restored
;***************************************************************************
Init_files:
	movem.l	d0/d1/a0/a1,-(sp)
; ---------- Set current directory ----------------
	kickDOS	GetProgramDir		; Get home directory
	move.l	d0,d1			; Is now current directory
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
.Error:	movem.l	(sp)+,d0/d1/a0/a1
	rts


	SECTION	Fast_BSS,bss
Old_WindowPtr1:	ds.l 1
Old_WindowPtr2:	ds.l 1
