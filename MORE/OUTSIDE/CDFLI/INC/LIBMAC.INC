;--------------------------------------
; PROCINIT - prepare Proc, save Regs
;--------------------------------------
PROCINIT MACRO ARGCOUNT
		push 	BP
		mov 	BP,SP
		IFNB 	<ARGCOUNT>
			sub 	SP,ARGCOUNT
		ENDIF
		push 	SI DI DS ES
		mov   ax,@DATA
		mov   ds,ax
ENDM
;--------------------------------------
; PROCEXIT - prepare Proc for leaving
;--------------------------------------
PROCEXIT MACRO ARGCOUNT
		pop 	ES DS DI SI
		ifnb 	<ARGCOUNT>
			add 	SP,ARGCOUNT
		ENDIF
		pop 	BP
		ret
ENDM PROCEXIT
;--------------------------------------
; MOVB - direct Memory-Transfer Byte
;--------------------------------------
MOVB MACRO DST,SRC
		mov al,byte ptr [SRC]
		mov byte ptr [DST],al
ENDM MOVB
;--------------------------------------
; MOVW - direct Memory-Transfer Word
;--------------------------------------
MOVW MACRO DST,SRC
		mov ax,word ptr [SRC]
		mov word ptr [DST],ax
ENDM MOVW
MOVD MACRO DST,SRC
		mov eax,dword ptr [SRC]
		mov dword ptr [DST],eax
ENDM MOVD
;--------------------------------------
; FILLEAX - fills eax with Value Byte
;--------------------------------------
FILLEAX MACRO VALUE
		mov al,VALUE
		mov ah,al
		shl eax,16
		mov al,VALUE
		mov ah,al
ENDM FILLEAX


