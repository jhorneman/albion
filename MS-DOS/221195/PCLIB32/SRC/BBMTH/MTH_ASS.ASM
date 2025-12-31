; ************
; * NAME     : MTH_ASS.asm
; * AUTOR    : R.Reber, BlueByte
; * START    : 26.05.95 12:00
; * PROJECT  : Poject32
; * NOTES    :
; * SEE ALSO :
; * VERSION  : 1.0
; ************/

	.386p

	ASSUME DS:MTH_DATA

	.MODEL FLAT
	.CODE

; extern Functions
	PUBLIC	C MTH_ASS_SQRT_ 					; Wurzel aus 32 Bit Zahl ziehen
	PUBLIC	C MTH_ASS_SQRT64_ 				; Wurzel aus 64 Bit Zahl ziehen
	PUBLIC	C MTH_ASS_Random_ 				; Wurzel aus 32 Bit Zahl ziehen
	PUBLIC	C MTH_ASS_VektorLen_ 			; Wurzel aus 32 Bit Zahl ziehen
	PUBLIC	C MTH_ASS_ArcTangens_ 			; Wurzel aus 32 Bit Zahl ziehen

; externe Variablen
	extrn	_MTH_randomseed:DWORD ; RandomSeed Wert
	extrn	_MTH_TANGENSDATA:DWORD ; RandomSeed Wert

; Include Files
	INCLUDE ..\..\LIB\ASMINC\BBMACRO.INC
	INCLUDE ..\..\LIB\ASMINC\BBHARD.INC

; interne Macros


MTH	SEGMENT PARA USE32 'CODE'	;

; ****************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : MTH_ASS_SQRT
; * FUNCTION  : initialisiert Palette
; *
; * FILE      : MTH_ASS.ASM
; * AUTHOR    : Christian Jungen, BlueByte
; * FIRST     : 26.04.94 17:22
; * LAST      :24.07.94 20:57:18
; * INPUTS    : EAX = 32 Bit Zahl
; * RESULT    : EAX = 32 Bit Wurzel aus 32 Bit Zahl
; * BUGS      : -
; * NOTES     : -
; * SEE ALSO  : -
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

MTH_ASS_SQRT_ PROC C

	xor	ecx,ecx			; Ergebnis=0
	mov	ebx,010000000h		; Bitfeld=2^32
sqr_iteration:
	mov	edx,ecx
	add	edx,ebx			; XQuadrat=Ergebnis+Bitfeld
	sar	ecx,1			; SHR(Ergebnis,1)

	cmp	eax,edx			; XQuadrat<=x?
	jl		SHORT sqr_XQuadrat_is_greater
	sub	eax,edx			; x=x-XQuadrat
	add	ecx,ebx			; Ergebnis=Ergebnis+Bitfeld
sqr_XQuadrat_is_greater:

	sar	ebx,2			; SHR(Bitfeld,2)
	jne	SHORT sqr_iteration

	mov	eax,ecx			; Ergebnis nach eax kopiern

	ret

MTH_ASS_SQRT_ ENDP

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : SQR64
; * FUNCTION  : Berechnet die Wurzel aus einer 64 Bit Zahl
; * FILE      : MTH_ASS.ASM
; * AUTHOR    : R.Reber
; * FIRST     : 27.07.94 14:00
; * LAST      :01.08.94 17:42:55
; * INPUTS    : UNLONG (EAX): Low	dword NUMBER
; *             UNLONG (EDX): High	dword NUMBER
; * RESULT    : UNSHORT (EAX): RESULT:
; * BUGS      :
; * NOTES     : Routine wird nur innerhalb MTH_ASS verwendet von veklen
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

MTH_ASS_SQRT64_  PROC C USES EBP

	sub 	ESI,ESI  ; clear Result
	sub 	EBX,EBX  ; HiWi-Var
	mov 	ECX,32  ; Number of Loops=Precision
@@L01:   		 ;*SQR-LOOP
	mov 	EBP,ESI  ; save Result for a while
					 ;
	shl 	EAX,1  ; Shift two Bits to BX
	rcl 	EDX,1  ;
	adc 	EBX,EBX  ; first Bit
	shl 	EAX,1  ;
	rcl 	EDX,1  ;
	adc 	EBX,EBX  ; second Bit
					 ;
	add 	ESI,ESI  ; Calc 4*Result+1
	add 	ESI,ESI  ;
	inc 	ESI 	 ;
					 ;
	sub 	EBX,ESI  ; now: Compare Result with n.Item
	jnb 	@@M01  ; if no carry, jump
	add 	EBX,ESI  ; restaurate Sub
@@M01:   		 ;*short for ready
	cmc 	 		 ; Complement Carry-Flag
	mov 	ESI,EBP  ; load Result back
	rcl 	ESI,1  ; Shift Carry from down under into Result
	dec	ecx
	jg		@@L01  ; do the LOOP for CX-Times
	mov 	EAX,ESI  ; Result to DX

	ret
MTH_ASS_SQRT64_	 ENDP

;******************************************************************************
;* #FUNCTION HEADER BEGIN#
;* NAME      : Random
;* FUNCTION  :
;*
;* INPUTS    :
;* RESULT    : ZufallsZahl von 0-65535
;*
;* FILE      : MTH_ASS.ASM
;* AUTHOR    : R.Reber
;* FIRST     : 24.08.94 11:41:19
;* LAST      : 24.08.94 12:20:24
;* BUGS      :
;* NOTES     : Der StartWert wird bei InitRandom festgelegt
;*					Routine verÑndert Register EAX,EBX,EDX, sollte sich dies
;*					Ñndern muss die Routine Explode umgeschrieben werden
;* SEE ALSO  : Explode
;* VERSION   : 0.1
;* #FUNCTION HEADER END#

MTH_ASS_Random_ PROC

	mov 	EAX,_MTH_randomseed
	imul 	EAX,1103515245
	add 	EAX,14271
	mov 	_MTH_randomseed,EAX

	mov 	EBX,65536
	cdq
	idiv EBX

	mov 	EAX,EDX
	and 	EAX,65535

	ret
MTH_ASS_Random_ ENDP

;******************************************************************************
;* #FUNCTION HEADER BEGIN#
;* NAME      : GetVektorLen_
;* FUNCTION  : berechnet LÑnge eines Vektors
;*
;* INPUTS    : XV(EAX) = Vektor X
;*             YV(EBX) = Vektor Y
;*					ZV(ECX) = Vektor Z
;*
;* RESULT    : EAX = LÑnge des Vektors
;*
;* FILE      : MTH_ASS.ASM
;* AUTHOR    : R.Reber
;* FIRST     : 10.08.94 17:18:43
;* LAST      : 24.08.94 11:41:19
;* BUGS      :
;* NOTES     : Routine funktioniert mit komplettem 32 BitBereich
;* SEE ALSO  :
;* VERSION   : 0.1
;* #FUNCTION HEADER END#

MTH_ASS_VektorLen_ PROC

	imul EAX
	mov 	ESI,EAX
	mov 	EDI,EDX
	mov 	EAX,EBX
	imul EAX
	mov 	EBX,EAX
	mov	EAX,ECX
	mov 	ECX,EDX
	imul EAX
	add 	EAX,EBX  ; Add low dword,
	adc 	EDX,ECX  ; Add high dword,
	add 	EAX,ESI  ; Add low dword,
	adc 	EDX,EDI  ; Add high dword,
	call	MTH_ASS_SQRT64_  ; Wurzel um Entfernung von Object zum Betrachterstandpunkt zu berechnen

	ret
MTH_ASS_VektorLen_ ENDP



;******************************************************************************
;* #FUNCTION HEADER BEGIN#
;* NAME      : MTH_ArcTangens_
;* FUNCTION  : berechnet LÑnge eines Vektors
;*
;* INPUTS    : XV(EAX) = Vektor X
;*             YV(EBX) = Vektor Y
;*
;* RESULT    : AX = Winkel(0-65535) des Vektors
;*
;* FILE      : MTH_ASS.ASM
;* AUTHOR    : R.Reber
;* FIRST     : 10.08.94 17:18:43
;* LAST      : 24.08.94 11:41:19
;* BUGS      :
;* NOTES     : Routine funktioniert mit komplettem 32 BitBereich
;*					benîtigt 4 KB Grosse Tangens Tabelle
;*             Beispiel
;*					x=0;y=-10 -> 0
;*					x=0;y=10 -> 32768
;*					x=10;y=0 -> 16384
;* SEE ALSO  :
;* VERSION   : 0.1
;* #FUNCTION HEADER END#

MTH_ASS_ArcTangens_ PROC USES EBP

	lea	EDI,_MTH_TANGENSDATA ; Addresse der Tangenstabelle

	mov	EBP,EAX
	or 	EAX,EAX
	jns 	WPD2P
	neg 	EAX
WPD2P:

	mov 	ESI,EBX
	or 	EBX,EBX
	jns 	WPD3P
	neg 	EBX
WPD3P:

	mov 	EDX,EAX
	shr 	EDX,18
	shl 	EAX,14  ;*16384

	or 	EBX,EBX
	je 	NOZEROY2 ; Division durch 0
	cmp 	EDX,EBX
	jae 	NOZEROY2	 ; Division durch 0 wenn Hiwort des Divisors grîsser als Divident
	div 	EBX
NOZEROY2:

	mov 	ECX,2047
	mov 	EDX,2047

	REPT 8
	shr 	EDX,1
	mov 	EBX,EDX
	shl 	EBX,2
	cmp 	EDX,[EDI+EBX]
	jbe 	@F
	sub 	ECX,EDX
	add 	EDI,EBX
@@:
	ENDM

SEARCHTAN2:
	cmp 	EAX,[EDI]
	jle 	POSTAN2
	add 	EDI,4
	dec 	ECX
	jns 	SEARCHTAN2
	xor 	ECX,ECX
POSTAN2:
	sal 	ECX,3

	or		EBP,EBP
	js 	TANLINKS
	or		ESI,ESI
	js 	TANOBEN
	add	ECX,16384
	jmp 	ENDTAN
TANOBEN:
	xor	ECX,16383
	jmp 	ENDTAN

TANLINKS:
	or		ESI,ESI
	js 	TANOBENLINKS
	xor	ECX,16383
	add	ECX,32768
	jmp 	ENDTAN
TANOBENLINKS:
	add	ECX,32768+16384
ENDTAN:

	mov	EAX,ECX

	ret

MTH_ASS_ArcTangens_ ENDP

MTH  	ENDS


;******************************************************************************
;******************************************************************************
; Variablen Segment fÅr MTH
MTH_DATA SEGMENT PARA USE32 'DATA'	;

DUMMY	DD ?

MTH_DATA ENDS

END
