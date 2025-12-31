; ************
; * NAME     : SYS_ASS.asm
; * AUTOR    : Viper, BlueByte
; * START    : 25.05.94 14:00
; * PROJECT  : Poject32/Macintosh
; * NOTES    :
; * SEE ALSO :
; * VERSION  : 1.0
; ************/

	.386p

	ASSUME NOTHING

; extern Functions
	PUBLIC	C SYS_ASS_MouseBob_ ; Palette aktivieren

; Include Files
	INCLUDE ..\..\LIB\ASMINC\BBMACRO.INC
	INCLUDE ..\..\LIB\ASMINC\BBHARD.INC

SYS	SEGMENT PARA USE32 'CODE'	;

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : FASTREPD mback:DWORD,mbackold:DWORD,mgto:DWORD,mtrans:WORD,msize:DWORD
;	*							mback(ESI) = QuellHintergrund
;	*							mbackold(EDI) = ZielHintergrund
;	*							mgto(EBX) = GTO
; *							mtrans(dl) = durchsichtige Farbe
; *							msize(ecx) = Anzahl zu kopierende Bytes
; * FUNCTION  : Simuliert REP MOVSD
; * FILE      : SYS_ASS.C
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     : ben”tigt Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

SYS_ASS_MouseBob_ PROC C mback:DWORD,mbackold:DWORD,mgto:DWORD,mtrans:WORD,msize:DWORD

copyinmousebob:
	mov	al,[esi] ; QuellByte
	mov	ah,[ebx] ; GTO Byte
	cmp	ah,dl    ; Maskenfarbe
	je	copynomousebob
	mov	[esi],ah ; Wenn nicht MaskenFarbe GTOByte schreiben
copynomousebob:
	mov	[edi],al ; In ZielHintergrund schreiben
	inc esi
	inc edi
	inc ebx
	dec ecx
	jg	 SHORT copyinmousebob

	ret 	 	 	;
SYS_ASS_MouseBob_ ENDP

SYS  	ENDS 				;

END

