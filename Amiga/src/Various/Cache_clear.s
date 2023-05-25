	movem.l	d0/d1/a0/a1,-(sp)
	lea.l	Line_buffer,a0		; Clear caches
	move.l	#Line_buffer_size,d0
	move.l	#CACRF_ClearD,d1
	kickEXEC	CacheClearE
	movem.l	(sp)+,d0/d1/a0/a1
