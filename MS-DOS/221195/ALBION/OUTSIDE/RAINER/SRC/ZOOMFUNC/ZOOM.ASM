; ************
; * NAME     : ZOOM.asm
; * AUTOR    : R.Reber, BlueByte
; * START    : 25.05.94 14:00
; * PROJECT  : ALBION
; * NOTES    :
; * SEE ALSO :
; * VERSION  : 1.0
; ************/

	.386p

	ASSUME DS:ZOOM_DATA

	.MODEL FLAT
	.CODE

; Public Functions
	PUBLIC	C ZOOM_BLOCK_ 						; horizontale Linie

; Public Variablen
	PUBLIC	_ZOOMFUNC_DESTCLIPX0 ; Clipping Variablen fÅr ZoomBob
	PUBLIC	_ZOOMFUNC_DESTCLIPY0
	PUBLIC	_ZOOMFUNC_DESTCLIPX1
	PUBLIC	_ZOOMFUNC_DESTCLIPY1

; Include Files
	INCLUDE g:\PCLIB32\LIB\ASMINC\BBMACRO.INC
	INCLUDE g:\PCLIB32\LIB\ASMINC\BBHARD.INC

; Zoommodes
ZOOMMODE_NORMAL=0
ZOOMMODE_SILHOUETTE=1
ZOOMMODE_RECOLOUR_BACK=2
ZOOMMODE_RECOLOUR=3
ZOOMMODE_TRANSPARENT=4

ZOOM_CODE SEGMENT PARA USE32 'CODE'	;

;ZOOMBOB
;******************************************************************************
;* #FUNCTION HEADER BEGIN#
;* NAME      : ZOOM_BLOCK
;* FUNCTION  : Stellt OPM mit wahlweiser Breite und Hîhe dar (interne Funktion)
;*					Farbe 0 wird durchsichtig dargestellt
;*
;* INPUTS    : OPM(EDI)= Zeiger auf OPM
;*					Xpos(EAX) = X Position Bildschirm
;*					Ypos(EBX) = Y Position Bildschirm
;*					sizex(ECX) = neue Grîsse in X
;*					sizey(EDX) = neue Grîsse in Y
;* INPUTS    : sourceptr = Zeiger auf QUELLOPM
;*					destptr = Zeiger auf Zielopm
;*					sourcenextypos = Breite QuellHauptopm
;*					sourcewidth = Breite QuellBereich
;*					sourceheight = Hîhe QuellBereich
;*					destnextypos = Breite ZielHauptopm
;*					destwidth = neue Breite ZielBereich
;*					destheight = neue Hîhe ZielBereich
;*					destx = xpos ZielBereich
;*					desty = ypos ZielBereich
;*					color/colortable/transparenttable = Zeichenfarbe (nur wenn shilouetteMode),UmfÑrbTabelle,TransparentTabelle
;*					zoommode = ZOOMMODE_NORMAL = Graphik mit Farbe 0 transparent zeichnen
;*                        ZOOMMODE_SILHOUETTE = GraphikUmriss mit color zeichnen
;*                        ZOOMMODE_RECOLOUR = Graphik mittels Umrechnungstabelle zeichnen
;*                        ZOOMMODE_TRANSPARENT = Graphik mittels Urechnungstabelle transparent zeichnen
;*
;* zusÑtzlich mÅssen folgende Globale Variablen initialisiert sein !
;*					destcx0 = Clipx0 Zielbereich
;*					destcy0 = Clipy0 Zielbereich
;*					destcx1 = Clipx1 Zielbereich
;*					destcy1 = Clipy1 Zielbereich
;* RESULT    :
;*
;* FILE      : SCREEN.ASM
;* AUTHOR    : VIPER
;* FIRST     : 30.08.94 13:35:58
;* LAST      : 20.03.95 16:49:06
;* BUGS      :
;* NOTES     : Das QuellOPM darf kein virtuelles OPM sein !!!
;* SEE ALSO  :
;* VERSION   : 0.1
;* #FUNCTION HEADER END#

ZOOM_BLOCK_ PROC C USES EBP sourceptr:DWORD,destptr:DWORD,sourcenextypos:DWORD,sourcewidth:DWORD,sourceheight:DWORD,destnextypos:DWORD,destwidth:DWORD,destheight:DWORD,destxpos:DWORD,destypos:DWORD,color:DWORD,zoommode:DWORD

	push	es
	push	ds
	pop	es

; ZielBob zentrieren
	movme	ZOOM_DWIDTH,DESTWIDTH
	movme	ZOOM_DHEIGHT,DESTHEIGHT

;	Breite / Hîhe des OPMS
	mov 	EAX,SOURCEWIDTH  ; Breite
	sal	EAX,16
	mov	ZOOM_SWIDTH,EAX
	mov	ZOOM_SBAKWIDTH,EAX

	mov 	EAX,SOURCEHEIGHT  ; Hîhe des OPM
	sal	EAX,16
	mov	ZOOM_SHEIGHT,EAX
	mov	ZOOM_SBAKHEIGHT,EAX

; Clipping
	movme	ZOOM_SXOFFSET,ZOOM_SBAKWIDTH ; Offset auf QuellBitmap
	mov	ZOOM_SYOFFSET,0

; links
	mov	EAX,DESTXPOS
	sub 	EAX,_ZOOMFUNC_DESTCLIPX0
	jns	CLIPZOMLEFT_BLOCK
	neg	EAX
	sub	ZOOM_DWIDTH,EAX ; Ziel Breite - X offset
	test	ZOOM_DWIDTH,-1 ; Breite <=0 dann Object ausserhalb des Screens
	jle	WRONGZOOMSIZE_BLOCK
	imul	ZOOM_SBAKWIDTH ; * Breite Quell
	idiv	DESTWIDTH ; / Breite Ziel
	sub	ZOOM_SWIDTH,EAX ; Quell Breite - skalierter Offset
	rol	EAX,16
	mov	ZOOM_SXOFFSET,EAX ; X Offset der auf QuellBitmap aufaddiert wird
	movme	DESTXPOS,_ZOOMFUNC_DESTCLIPX0
CLIPZOMLEFT_BLOCK:

; rechts
	mov	EAX,DESTXPOS
	add	EAX,ZOOM_DWIDTH
	dec	EAX
	sub 	EAX,_ZOOMFUNC_DESTCLIPX1
	jle	CLIPZOMRIGHT_BLOCK
	sub	ZOOM_DWIDTH,EAX ; Ziel Breite - X offset
	test	ZOOM_DWIDTH,-1 ; Breite <=0 dann Object ausserhalb des Screens
	jle	WRONGZOOMSIZE_BLOCK
	imul	ZOOM_SBAKWIDTH ; * Breite Quell
	idiv	DESTWIDTH ; / Breite Ziel
	sub	ZOOM_SWIDTH,EAX ; Quell Breite - skalierter Offset
CLIPZOMRIGHT_BLOCK:

; oben
	mov	EAX,DESTYPOS
	sub	EAX,_ZOOMFUNC_DESTCLIPY0
	jns	CLIPZOMTOP_BLOCK
	neg	EAX
	sub	ZOOM_DHEIGHT,EAX ; Ziel Breite - X offset
	test	ZOOM_DHEIGHT,-1 ; Breite <=0 dann Object ausserhalb des Screens
	jle	WRONGZOOMSIZE_BLOCK
	imul	ZOOM_SBAKHEIGHT ; * Breite Quell
	idiv	DESTHEIGHT ; / Breite Ziel
	sub	ZOOM_SHEIGHT,EAX ; Quell Breite - skalierter Offset
	rol	EAX,16 ; Hi mit LoWort vertauschen
	mov	ZOOM_SYOFFSET,EAX ; X Offset der auf QuellBitmap aufaddiert wird
	movme	DESTYPOS,_ZOOMFUNC_DESTCLIPY0
CLIPZOMTOP_BLOCK:

; unten
	mov	EAX,DESTYPOS
	add	EAX,ZOOM_DHEIGHT
	dec	EAX
	sub	EAX,_ZOOMFUNC_DESTCLIPY1
	jle	CLIPZOMBOTTOM_BLOCK
	sub	ZOOM_DHEIGHT,EAX ; Ziel Breite - X offset
	test	ZOOM_DHEIGHT,-1 ; Breite <=0 dann Object ausserhalb des Screens
	jle	WRONGZOOMSIZE_BLOCK
	imul	ZOOM_SBAKHEIGHT ; * Breite Quell
	idiv	DESTHEIGHT ; / Breite Ziel
	sub	ZOOM_SHEIGHT,EAX ; Quell Breite - skalierter Offset
CLIPZOMBOTTOM_BLOCK:

; an rechte Grenze setzen
	mov 	EAX,SOURCEPTR  ; Bitmap des OPMs
	mov	ZOOM_SBITMAP,EAX ; an rechte Grenze setzen
	mov	EDI,DESTPTR ; AusgabeBildschirm

	mov	EAX,DESTYPOS ; Position am Bildschirm
	imul 	EAX,DESTNEXTYPOS
	add	EAX,DESTXPOS
	add	EDI,EAX

	mov	EAX,ZOOM_SWIDTH
	mov	EBX,ZOOM_DWIDTH ; neue Breite
	cdq
	idiv EBX
	rol 	EAX,16  ; Hi mit LoWort vertauschen
	mov	ZOOM_MX,EAX ; Steigung fÅr X

	mov	EAX,ZOOM_SHEIGHT
 	mov	EBX,ZOOM_DHEIGHT ; neue Hîhe
	cdq
	idiv EBX
	rol 	EAX,16  ; Hi mit LoWort vertauschen
	mov	ZOOM_MY,EAX ; Steigung fÅr Y

	movme	SOURCEWIDTHII,SOURCENEXTYPOS
	mov	EAX,DESTNEXTYPOS
	sub	EAX,ZOOM_DWIDTH
	mov	DESTWIDTHII,EAX

	push	EBP

	CLD ; Richtung stosb vorwÑrts

	cmp	zoommode,ZOOMMODE_SILHOUETTE
	je		shilouettenmode

	cmp	zoommode,ZOOMMODE_RECOLOUR_BACK
	je		recolorbackmode

	cmp	zoommode,ZOOMMODE_RECOLOUR
	je		recolormode

	cmp	zoommode,ZOOMMODE_TRANSPARENT
	je		transparentmode

;**************************************
; normaler Zoommode
;**************************************

; gezoomtes OPM darstellen
	mov	EBX,ZOOM_MX
	mov	EBP,ZOOM_SYOFFSET ; Clip Y Offset
	mov	EAX,ZOOM_DHEIGHT
	mov	ZOOM_LOPVAR,EAX
YZOMDRAW_NORMAL:
	mov	ESI,ZOOM_SBITMAP ; StartAddresse Screen
	movzx	EDX,BP ; LoWort = Ganzzahl
	imul	EDX,SOURCEWIDTHII ; mit PixelBreite multiplizieren
	add	ESI,EDX ; nÑchste Zeile Quellopm

	mov	EDX,ZOOM_SXOFFSET ; Clip X Offset
	mov	ECX,ZOOM_DWIDTH
	xor	EAX,EAX
ZOOMLOOP_NORMAL:

 	add	EDX,EBX ; X addieren
	adc	EDX,0
	mov	AX,DX ; LoWort = Ganzzahl

	mov	al,[ESI+EAX] ; Pixel lesem
	or		al,al
	je 	@F
	stosb
	dec	ECX
	jg		ZOOMLOOP_NORMAL
	jmp	ENDZOOMLOP_NORMAL
@@:
	inc	edi
	dec	ECX
	jg		ZOOMLOOP_NORMAL
ENDZOOMLOP_NORMAL:

	add	EBP,ZOOM_MY ; Y addieren
	adc	EBP,0
	add	EDI,DESTWIDTHII ; Modulo nÑchste Zeile Bildschirm
	dec 	ZOOM_LOPVAR
	jg 	YZOMDRAW_NORMAL

	pop	EBP
WRONGZOOMSIZE_BLOCK:
	pop	es

	ret
;**************************************
; Shilhouetten Zoommode
;**************************************
shilouettenmode:

	mov	EBX,ZOOM_MX
	movme ZOOM_LOPVAR,ZOOM_DHEIGHT
	mov 	EAX,color
YZOMDRAW_SHILHOUETTE:
	mov	ESI,ZOOM_SBITMAP ; StartAddresse Screen
	movzx	EDX,WORD PTR ZOOM_SYOFFSET ; LoWort = Ganzzahl
	imul	EDX,SOURCEWIDTHII ; mit PixelBreite multiplizieren
	add	ESI,EDX ; nÑchste Zeile Quellopm

	mov	EDX,ZOOM_SXOFFSET ; Clip X Offset
	mov	ECX,ZOOM_DWIDTH
	xor	EBP,EBP
ZOOMLOOP_SHILHOUETTE:

 	add	EDX,EBX ; X addieren
	adc	EDX,0
	mov	BP,DX ; LoWort = Ganzzahl

	test	BYTE PTR [EBP+ESI],-1 ; Pixel lesem
	je 	@F
	stosb
	dec	ECX
	jg		ZOOMLOOP_SHILHOUETTE
	jmp	ENDZOOMLOP_SHILHOUETTE
@@:
	inc	edi
	dec	ECX
	jg		ZOOMLOOP_SHILHOUETTE
ENDZOOMLOP_SHILHOUETTE:

	mov	ECX,ZOOM_MY
	add	ZOOM_SYOFFSET,ECX ; Y addieren
	adc	ZOOM_SYOFFSET,0
	add	EDI,DESTWIDTHII ; Modulo nÑchste Zeile Bildschirm
	dec 	ZOOM_LOPVAR
	jg 	YZOMDRAW_SHILHOUETTE

	pop	EBP
	pop	es

	ret

;**************************************
; RecolorBackground Zoommode
;**************************************
recolorbackmode:
	mov	EBX,ZOOM_MX
	movme ZOOM_LOPVAR,ZOOM_DHEIGHT
	mov	EBP,color
YZOMDRAW_RECOLOR_BACK:
	mov	ESI,ZOOM_SBITMAP ; StartAddresse Screen
	movzx	EDX,WORD PTR ZOOM_SYOFFSET ; LoWort = Ganzzahl
	imul	EDX,SOURCEWIDTHII ; mit PixelBreite multiplizieren
	add	ESI,EDX ; nÑchste Zeile Quellopm

	mov	EDX,ZOOM_SXOFFSET ; Clip X Offset
	mov	ECX,ZOOM_DWIDTH
	xor	EAX,EAX
ZOOMLOOP_RECOLOR_BACK:

 	add	EDX,EBX ; X addieren
	adc	EDX,0
	mov	AX,DX ; LoWort = Ganzzahl

	test	BYTE PTR [ESI+EAX],-1 ; Pixel testen
	je 	@F
	movzx	AX,BYTE PTR [EDI] ; Pixel aus Hintergrund lesen
	mov	AL,[EBP+EAX] ; Pixel aus UmrechenTabelle
	stosb
	dec	ECX
	jg		ZOOMLOOP_RECOLOR_BACK
	jmp	ENDZOOMLOP_RECOLOR_BACK
@@:
	inc	edi
	dec	ECX
	jg		ZOOMLOOP_RECOLOR_BACK
ENDZOOMLOP_RECOLOR_BACK:

	mov	ECX,ZOOM_MY
	add	ZOOM_SYOFFSET,ECX ; Y addieren
	adc	ZOOM_SYOFFSET,0
	add	EDI,DESTWIDTHII ; Modulo nÑchste Zeile Bildschirm
	dec 	ZOOM_LOPVAR
	jg 	YZOMDRAW_RECOLOR_BACK

	pop	EBP
	pop	es

	ret
;**************************************
; Recolor Zoommode
;**************************************
recolormode:
	mov	EBX,ZOOM_MX
	movme ZOOM_LOPVAR,ZOOM_DHEIGHT
	mov	EBP,color
YZOMDRAW_RECOLOR:
	mov	ESI,ZOOM_SBITMAP ; StartAddresse Screen
	movzx	EDX,WORD PTR ZOOM_SYOFFSET ; LoWort = Ganzzahl
	imul	EDX,SOURCEWIDTHII ; mit PixelBreite multiplizieren
	add	ESI,EDX ; nÑchste Zeile Quellopm

	mov	EDX,ZOOM_SXOFFSET ; Clip X Offset
	mov	ECX,ZOOM_DWIDTH
	xor	EAX,EAX
ZOOMLOOP_RECOLOR:

 	add	EDX,EBX ; X addieren
	adc	EDX,0
	mov	AX,DX ; LoWort = Ganzzahl

	movzx	AX,BYTE PTR [ESI+EAX] ; Pixel lesem
	or		AL,AL
	je 	@F
	mov	AL,[EBP+EAX] ; Pixel aus UmrechenTabelle
	stosb
	dec	ECX
	jg		ZOOMLOOP_RECOLOR
	jmp	ENDZOOMLOP_RECOLOR
@@:
	inc	edi
	dec	ECX
	jg		ZOOMLOOP_RECOLOR
ENDZOOMLOP_RECOLOR:

	mov	ECX,ZOOM_MY
	add	ZOOM_SYOFFSET,ECX ; Y addieren
	adc	ZOOM_SYOFFSET,0
	add	EDI,DESTWIDTHII ; Modulo nÑchste Zeile Bildschirm
	dec 	ZOOM_LOPVAR
	jg 	YZOMDRAW_RECOLOR

	pop	EBP
	pop	es

	ret
;**************************************
; Transparent Zoommode
;**************************************
transparentmode:
	mov	EBX,ZOOM_MX
	movme ZOOM_LOPVAR,ZOOM_DHEIGHT
	mov	EBP,color
YZOMDRAW_TRANSPARENT:
	mov	ESI,ZOOM_SBITMAP ; StartAddresse Screen
	movzx	EDX,WORD PTR ZOOM_SYOFFSET ; LoWort = Ganzzahl
	imul	EDX,SOURCEWIDTHII ; mit PixelBreite multiplizieren
	add	ESI,EDX ; nÑchste Zeile Quellopm

	mov	EDX,ZOOM_SXOFFSET ; Clip X Offset
	mov	ECX,ZOOM_DWIDTH
	xor	EAX,EAX
ZOOMLOOP_TRANSPARENT:

 	add	EDX,EBX ; X addieren
	adc	EDX,0
	mov	AX,DX ; LoWort = Ganzzahl

	mov	AL,BYTE PTR [ESI+EAX] ; VordergrundPixel lesem
	or		AL,AL
	je 	@F
	mov	AH,[EDI] ; HintergrundPixel lesen
	mov	AL,[EBP+EAX] ; Pixel aus TransparenzUmrechenTabelle
	stosb
	dec	ECX
	jg		ZOOMLOOP_TRANSPARENT
	jmp	ENDZOOMLOP_TRANSPARENT
@@:
	inc	edi
	dec	ECX
	jg		ZOOMLOOP_TRANSPARENT
ENDZOOMLOP_TRANSPARENT:

	mov	ECX,ZOOM_MY
	add	ZOOM_SYOFFSET,ECX ; Y addieren
	adc	ZOOM_SYOFFSET,0
	add	EDI,DESTWIDTHII ; Modulo nÑchste Zeile Bildschirm
	dec 	ZOOM_LOPVAR
	jg 	YZOMDRAW_TRANSPARENT

	pop	EBP
	pop	es

	ret



ZOOM_BLOCK_ ENDP

ZOOM_CODE  	ENDS 				;

ZOOM_DATA	SEGMENT PARA USE32 'DATA'	;

; Lokale Variablen ZoomBobFunktion
ZOOM_DWIDTH DD ?
ZOOM_DHEIGHT DD ?
ZOOM_SBITMAP DD ?
ZOOM_SWIDTH DD ?
ZOOM_SHEIGHT DD ?
ZOOM_SBAKWIDTH DD ?
ZOOM_SBAKHEIGHT DD ?
ZOOM_SXOFFSET DD ?
ZOOM_SYOFFSET DD ?
ZOOM_MX DD ?
ZOOM_MY DD ?
ZOOM_DWIDTHMODULO DD ?
SOURCEWIDTHII DD ?
DESTWIDTHII DD ?
_ZOOMFUNC_DESTCLIPX0 DD ?
_ZOOMFUNC_DESTCLIPY0 DD ?
_ZOOMFUNC_DESTCLIPX1 DD ?
_ZOOMFUNC_DESTCLIPY1 DD ?
ZOOM_LOPVAR DD ?

ZOOM_DATA	ENDS

END


