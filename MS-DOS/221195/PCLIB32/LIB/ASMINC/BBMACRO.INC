; ************
; * NAME     : BBMACRO.INC
; * AUTOR    : Viper, BlueByte
; * START    : 25.05.94 14:00
; * PROJECT  : Poject32/Macintosh
; * NOTES    : MacroDefinitionen fÅr MASM
; * SEE ALSO :
; * VERSION  : 1.0
; ************/

;--------------------------------------
;SAFEWAIT Wait for three Jumps

SAFEWAIT   	MACRO 	;
	jmp 	@F 	 	;
@@:  	 	 	;
	jmp 	@F 	 	;
@@:  	 	 	;
	jmp 	@F 	 	;
@@:  	 	 	;
			ENDM  	;



; Copy mem to mem over ax
movm	MACRO adra,adrb
	mov	ax,adrb
	mov	adra,ax
ENDM

; Copy mem to mem over eax
movme	MACRO adra,adrb
	mov	eax,adrb
	mov	adra,eax
ENDM

; add mem to mem over ax
addm	MACRO adra,adrb
	mov	ax,adrb
	add	adra,ax
ENDM

; add mem to mem over eax
addme	MACRO adra,adrb
	mov	eax,adrb
	add	adra,eax
ENDM

; sub mem to mem over ax
subm	MACRO adra,adrb
	mov	ax,adrb
	sub	adra,ax
ENDM

; sub mem to mem over eax
subme	MACRO adra,adrb
	mov	eax,adrb
	sub	adra,eax
ENDM

; compare mem to mem over eax
cmpme	MACRO adra,adrb
	mov	eax,adrb
	cmp	adra,eax
ENDM

; lea Adr
leame MACRO A,B
	lea 	EAX,B
	mov 	A,EAX
ENDM

xchgm MACRO A,B
	mov 	AX,B
	xchg 	A,AX
	mov 	B,AX
ENDM

xchgme MACRO A,B
	mov 	EAX,B
	xchg 	A,EAX
	mov 	B,EAX
ENDM

clr MACRO A
	sub 	A,A
ENDM

tst MACRO A
	or		A,A
ENDM

; OUT Byte to port
out_b		MACRO VAL1,VAL2 	;
	ifidni <VAL1>,<DX>  	;
	else 	 	 	;
	mov 	DX,VAL1  	;
	endif  	 	;
	IFIDNI <VAL2>,<AL>  	;
	out 	DX,AL  	;
	else 	 	 	;
	mov 	AL,VAL2  	;
	out 	DX,AL  	;
	endif  	 	;
ENDM

; OUT word to port
out_w	MACRO VAL1,VAL2
	ifidni <VAL1>,<DX>  	;
	else 	 	 	;
	mov 	DX,VAL1  	;
	endif  	 	;
	IFIDNI <VAL2>,<AX>  	;
	out 	DX,AX  	;
	else 	 	 	;
	mov 	AX,VAL2  	;
	out 	DX,AX  	;
	endif  	 	;
ENDM

; IN Byte to port
IN_B MACRO VAL1 	;
	ifidni <VAL1>,<DX>  	;
	else 	 	 	;
	mov 	DX,VAL1  	;
	endif  	 	;
	in 	AL,DX  	;
ENDM	;

; alle Register Retten
PUSHALL MACRO
	push	eax
	push	ebx
	push	ecx
	push	edx
	push	esi
	push	edi
	push	ebp
ENDM

; alle Register zurÅckholen
POPALL MACRO
	pop	ebp
	pop	edi
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	pop	eax
ENDM

; Ersatz fÅr Call Routine wird verwendet wenn eine interne Assemblerfunktion
; von einer anderen aufgerufen wird
ACALL MACRO VAL 	;
	push	EBP
	lea	EBP,VAL
	push	@F
	jmp	EBP  	;
@@:
	pop	EBP
ENDM	;

; Hintergrundfarbe aufblitzen lassen
FLASH MACRO a,b,c
	push	ax
	push	dx

	cli 	 		 ;
	sub 	AX,AX  ; clear AX
	mov 	DX,03C8H  ; DX with ColorPort
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out 	DX,AL  ; Start with ColorRegister 0

	mov		al,a
	safewait  	 ; like BIOS, Flush Cache, wait a little
	mov 	DX,03C9H  ; DX with ColorPort
	out 	DX,AL  ; Set Color -> RED

	mov		al,b
	safewait  	 ; like BIOS, Flush Cache, wait a little
	mov 	DX,03C9H  ; DX with ColorPort
	out 	DX,AL  ; Set Color -> RED

	mov		al,c
	safewait  	 ; like BIOS, Flush Cache, wait a little
	mov 	DX,03C9H  ; DX with ColorPort
	out 	DX,AL  ; Set Color -> RED

	sti
	pop	dx
	pop	ax
ENDM

