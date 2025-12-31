; ************
; * NAME     : DSA_ASS.asm
; * AUTOR    : Viper, BlueByte
; * START    : 25.05.94 14:00
; * PROJECT  : Poject32/Macintosh
; * NOTES    :
; * SEE ALSO :
; * VERSION  : 1.0
; ************/

	.386p

	ASSUME DS:FNT_DATA

	.MODEL FLAT
	.CODE

; Public Functions
	PUBLIC	C OPM_ASS_init_font_ 					; gefllter Block
	PUBLIC	C OPM_ASS_draw_text_ 						; vertikale Linie
	PUBLIC	C OPM_ASS_draw_char_ 						; vertikale Linie

; externe Variablen
;	extrn	_lastpage:WORD ; letze eingeschaltete Page

; Include Files
	INCLUDE ..\..\LIB\ASMINC\BBMACRO.INC
	INCLUDE ..\..\LIB\ASMINC\BBHARD.INC

tst	MACRO	arg
	or	arg,arg
	ENDM
clr	MACRO	arg
	sub	arg,arg
	ENDM


FNT_CODE	SEGMENT PARA USE32 'CODE'	;

;*******************************************************************************
; AES_init_font PROC
;
;  Initialisiert Screen fr Textausgabe
;
; Parameter:
;  void AES_init(int x1,int y1,int x2,int y2,char *opm_bitmap_ptr,int screen_modulo)
;*******************************************************************************
OPM_ASS_init_font_ PROC C X1:DWORD,Y1:DWORD,X2:DWORD,Y2:DWORD,OPM_BITMAP_PTR:DWORD,SCREEN_MODULO:DWORD
;------------------------------------------------
	mov	eax,OPM_BITMAP_PTR
	mov	opm_ptr,eax
	mov	eax,X1
	mov	x_left_clip,eax
	mov	eax,Y1
	mov	y_up_clip,eax
	mov	eax,X2
	mov	x_right_clip,eax
	mov	eax,Y2
	mov	y_down_clip,eax
	mov	eax,SCREEN_MODULO
	mov	screen_width,eax
;------------------------------------------------
	ret
OPM_ASS_init_font_ ENDP


;*******************************************************************************
; draw_text PROC
;
; Parameter:
;  void draw_text(int x_pos,int y_pos,int color,int font_size,char *text_ptr)
;
;  Schreibt Strin an X,Y-Position mit angegebener Farbe und Fonth”he
;*******************************************************************************
OPM_ASS_draw_text_ PROC C X_POS:DWORD,Y_POS:DWORD,TEXT_COLOR:DWORD,FONT_SIZE:DWORD,STRING_PTR:DWORD
	mov	eax,FONT_SIZE
	mov	esi,font_ptrs[eax*4]			; Zeiger auf Font holen

	movzx	eax,WORD PTR [esi]			; Fontbreite laden
	mov	char_width,eax
	movzx	ebx,WORD PTR 2[esi]			; Fonth”he laden
	mov	char_height,ebx
	add	esi,2*2					; Zeiger auf IMG
	add	eax,7
	sar	eax,3
	imul	ebx					; (Breite+7)/8*H”he=Bytes
	mov	char_bytes,eax

	mov	ft_ptr,esi

	mov	edi,STRING_PTR				; Zeiger auf String
drwtext_nxt_char:
	xor	eax,eax
	add	al,[edi]				; Zeichen holen
	je	drwtext_exit
	inc	edi

	imul	char_bytes
	add	eax,ft_ptr					; Offset in der Font

	push	EDI

	push	eax					; Font-Zeiger
	push	char_height  				; dy
	push	char_width				; dx
	push	Y_POS					; Y
	push	X_POS					; X
	push	TEXT_COLOR
	call	OPM_ASS_draw_char_
	add	esp,6*4

	pop	EDI

	mov	eax,char_width
	add	X_POS,eax
	jmp	drwtext_nxt_char
drwtext_exit:
	ret

font_ptrs	LABEL DWORD
		DD font_6_bitmap			; 0
		DD font_6_bitmap			; 1
		DD font_6_bitmap			; 2
		DD font_6_bitmap			; 3
		DD font_6_bitmap			; 4
		DD font_6_bitmap			; 5
		DD font_6_bitmap			; 6
		DD font_6_bitmap			; 7
		DD font_8_bitmap			; 8
		DD font_6_bitmap			; 9
		DD font_6_bitmap			; 10
		DD font_6_bitmap			; 11
		DD font_6_bitmap			; 12
		DD font_6_bitmap			; 13
		DD font_6_bitmap			; 14
		DD font_6_bitmap			; 15
		DD font_16_bitmap			; 16

OPM_ASS_draw_text_ ENDP



;*******************************************************************************
; draw_char PROC
;
;  Zeichnet 1-Plane Grafikobjekt inklusive Clipping auf OPM
;  Format des Grafikobjekts: pro X-Zeile muss
;
; Parameter:
;  void draw_char(int draw_color,int x_pos,int y_pos,int char_width,int char_height,char *char_ptr)
;*******************************************************************************
OPM_ASS_draw_char_ PROC C USES EBP DRAW_COLOR:DWORD,X_POS:DWORD,Y_POS:DWORD,CHAR_WIDTH:DWORD,CHAR_HEIGHT:DWORD,CHAR_PTR:DWORD
;------------------------------------------------
	clr	dl				; kein Clipping default

	mov	eax,DRAW_COLOR			; ah=Zeichenfarbe
	mov	ah,al
	mov	esi,CHAR_PTR

	mov	edi,opm_ptr
	add	edi,X_POS 			; Zeiger auf Screen
	mov	ebx,Y_POS
	imul	ebx,screen_width
	add	edi,ebx

	mov	ebx,CHAR_HEIGHT			; H”he setzten
	mov	y_line,ebx
	mov	ebx,CHAR_WIDTH			; Breite setzten
	mov	x_line,ebx
	mov	x_char_width,ebx

	mov	ebx,Y_POS			; Clipping oben
	sub	ebx,y_up_clip
	jge	drwchr_no_up_clip
	neg	ebx
	sub	y_line,ebx			; H”he vermindern
	jle	drwchr_exit				; => Exit
	mov	ecx,ebx
	imul	ecx,CHAR_WIDTH			; Breite setzten
	add	esi,ebx
	imul	ebx,screen_width
	add	edi,ebx
	mov	dl,1				; Flag fr Clipping setzten
drwchr_no_up_clip:

	mov	ebx,Y_POS			; Clipping unten
	add	ebx,CHAR_HEIGHT
	sub	ebx,y_down_clip
	jle	drwchr_no_down_clip
	sub	y_line,ebx			; H”he vermindern
	jle	drwchr_exit				; => Exit
	mov	dl,1				; Flag fr Clipping setzten
drwchr_no_down_clip:

	mov	ebx,X_POS 			; Clipping rechts
	add	ebx,CHAR_WIDTH			; Breite laden
	sub	ebx,x_right_clip
	jle	drwchr_no_right_clip
	dec	ebx
	sub	x_line,ebx			; Zeichenbreite
	jle	drwchr_exit
	mov	dl,1				; Flag fr Clipping setzten
drwchr_no_right_clip:


	mov	bits_to_leave_out,0		; Defaulwerte setzten
	mov	bytes_to_leave_out,0
	mov	ebx,x_left_clip			; Clipping links
	sub	ebx,X_POS
	jle	drwchr_no_left_clip
	sub	x_line,ebx			; Zeichenbreite
	jle	drwchr_exit
	add	edi,ebx				; +Offset
	mov	ecx,ebx				; Bit die ausgelassen werden mssen
	and	ecx,7
	mov	bits_to_leave_out,ecx
	mov	ecx,ebx				; Bytes die ausgelassen werden mssen
	sar	ecx,3
	mov	bytes_to_leave_out,ecx
	mov	dl,1				; Flag fr Clipping setzten
drwchr_no_left_clip:

	cmp	dl,1				; Clipping an?
	je	drwchr_bit_with_clipping


;------------------------------------------------
; blitten ohne Clipping
	mov	ebx,CHAR_WIDTH
	add	ebx,7				; aufrunden
	sar	ebx,3
	mov	x_char_bytes_width,ebx

	mov	ebx,x_line			; Breite in Bits und Bytes umrechnen
	mov	ecx,ebx
	sar	ebx,3
	mov	x_line_bytes,ebx
	and	ecx,7
	mov	x_line_bits,ecx

	mov	ebp,edi				; Screenpointer setzten
drwchr_nxt_y_nc:
	mov	edi,ebp				; Screenpointer kopieren
	push	esi				; Zeiger auf Char-Daten merken

; ganze 8er Gruppen
	mov	ecx,x_line_bytes		; X-Bytes-Z„hler laden
	tst	ecx
	je	drwchr_no_8er_pack			; = 8 Pixel aufs mal knpfen?
drwchr_nxt_8er_pack:
	mov	al,[esi]			; 8 Bits aufeinmal lesen
	inc	esi

CHAR_PLOT MACRO _OFFSET
	LOCAL skip_it
	add	al,al
	jnc	SHORT skip_it
	mov	_OFFSET[edi],ah			; Zeichenfarbe
skip_it:
	ENDM

_OFFSET	= 0
	REPT 8
	CHAR_PLOT _OFFSET
_OFFSET	= _OFFSET +1
	ENDM
	add	edi,8				; Screenpointer updaten
	dec	ecx
	jne	drwchr_nxt_8er_pack
drwchr_no_8er_pack:

; die restlichen Bits
	mov	ecx,x_line_bits			; X-Bits-Z„hler laden
	tst	ecx
	je	drwchr_no_1er_pack			; Anzahl Pixel knpfen?
	mov	al,[esi]			; 8 Bits aufeinmal lesen
	inc	esi
	mov	ecx,x_line_bits			; X-Z„hler laden
drwchr_nxt_x_1er:
	add	al,al
	jnc	drwchr_skip_1er
	mov	[edi],ah			; Zeichenfarbe
drwchr_skip_1er:
	inc	edi
	dec	ecx				; X_Counter
	jne	drwchr_nxt_x_1er
drwchr_no_1er_pack:

	add	ebp,screen_width
	pop	esi
	add	esi,x_char_bytes_width

	dec	y_line
	jne	drwchr_nxt_y_nc

	jmp	drwchr_exit				; => Ende

;------------------------------------------------
drwchr_bit_with_clipping:
	mov	ebx,CHAR_WIDTH
	add	ebx,7				; aufrunden
	sar	ebx,3
	mov	x_char_bytes_width,ebx

	mov	ebp,edi
drwchr_nxt_y:mov	edi,ebp				; Screenpointer kopieren

	push	esi				; Zeiger auf Char-Daten merken
	add	esi,bytes_to_leave_out		; soviele Bytes auslassen
	mov	al,[esi]			; 8 Bits aufeinmal lesen
	inc	esi
	mov	edx,8				; 8 Bits neugeladen

	mov	ecx,bits_to_leave_out		; soviele Bits links auslassen
	sal	al,cl
	sub	edx,ecx				; Anzahl geladener Bit vermindern

	mov	ecx,x_line			; X-Z„hler laden
drwchr_nxt_x:add	al,al
	jnc	drwchr_skip
	mov	[edi],ah			; Zeichenfarbe
drwchr_skip:	inc	edi
	dec	edx				; Anzahl geladener Bits vermindern
	jge	drwchr_no_new_load
	mov	al,[esi]			; 8 Bits aufeinmal lesen
	inc	esi
	mov	edx,8				; 8 Bits neugeladen
drwchr_no_new_load:
	dec	ecx				; X_Counter
	jne	drwchr_nxt_x

	add	ebp,screen_width
	pop	esi
	add	esi,x_char_bytes_width

	dec	y_line
	jne	drwchr_nxt_y
drwchr_exit:
;------------------------------------------------
	ret

OPM_ASS_draw_char_ ENDP

FNT_CODE  	ENDS 				;


;*********************************
;******** Daten ******************
;*********************************

FNT_DATA	SEGMENT PARA USE32 'DATA'	;

	ALIGN 4
x_line_bits		DD ?
x_line_bytes		DD ?
x_line			DD ?
y_line			DD ?
x_char_width		DD ?
x_char_bytes_width	DD ?
bits_to_leave_out	DD ?
bytes_to_leave_out	DD ?
char_width	DD ?
char_height	DD ?
char_bytes	DD ?
opm_ptr		DD ?
x_left_clip	DD ?
x_right_clip	DD ?
y_up_clip	DD ?
y_down_clip	DD ?
screen_width	DD ?
ft_ptr DD ?

font_6_bitmap LABEL WORD
	DW 6	; Fontbreite
	DW 6	; Fonth”he
	DB	00h,00h,00h,00h,00h,00h,020h,070h,0d8h,020h,020h,020h,020h,020h,020h,0d8h
	DB	070h,020h,010h,018h,0ech,018h,010h,00h,020h,060h,0dch,060h,020h,00h,070h,0a8h
	DB	0d8h,0a8h,070h,00h,0f8h,0f0h,0e0h,0c8h,098h,00h,0d8h,0a8h,070h,0a8h,0d8h,00h
	DB	0ch,018h,0b0h,0e0h,040h,00h,078h,084h,0a4h,0b4h,084h,078h,030h,078h,078h,0fch
	DB	00h,030h,010h,018h,010h,070h,0f0h,060h,0e0h,080h,0dch,090h,018h,010h,0e0h,080h
	DB	0b8h,0e8h,038h,02ch,014h,014h,014h,034h,074h,064h,040h,040h,040h,060h,070h,030h
	DB	0f8h,0c8h,0c8h,0d8h,0d8h,0f8h,060h,020h,020h,070h,070h,070h,078h,08h,0f8h,0c0h
	DB	0c0h,0f8h,0f0h,010h,0f8h,018h,018h,0f8h,0c0h,0c0h,0c0h,0d8h,0f8h,018h,0f0h,080h
	DB	0f8h,018h,018h,0f8h,0c0h,0c0h,0f8h,098h,0d8h,0f8h,0f8h,08h,038h,030h,030h,030h
	DB	070h,050h,070h,0d8h,0d8h,0f8h,0f8h,0d8h,0c8h,0f8h,018h,018h,00h,070h,08h,0f8h
	DB	070h,00h,0e0h,080h,0dch,090h,0f0h,01ch,01ch,03ch,060h,040h,070h,058h,0e0h,0d0h
	DB	018h,08h,078h,028h,044h,02ch,028h,014h,0ech,0c0h,08h,0d0h,050h,0a0h,0c0h,00h
	DB	00h,00h,00h,00h,00h,00h,030h,030h,030h,030h,00h,030h,0d8h,0d8h,090h,00h
	DB	00h,00h,050h,0f8h,050h,0f8h,050h,00h,078h,0a0h,070h,028h,0f0h,020h,0c8h,0d0h
	DB	020h,058h,098h,00h,060h,0d0h,060h,0e8h,0d0h,068h,030h,030h,060h,00h,00h,00h
	DB	030h,060h,060h,060h,030h,00h,060h,030h,030h,030h,060h,00h,048h,030h,078h,030h
	DB	048h,00h,020h,020h,0f8h,020h,020h,00h,00h,00h,00h,030h,030h,060h,00h,00h
	DB	078h,00h,00h,00h,00h,00h,00h,030h,030h,00h,018h,030h,060h,0c0h,080h,00h
	DB	070h,098h,0a8h,0c8h,070h,00h,010h,030h,010h,010h,010h,00h,0f0h,08h,070h,080h
	DB	0f8h,00h,0f0h,08h,070h,08h,0f0h,00h,030h,050h,090h,0f8h,010h,00h,0f0h,080h
	DB	0f0h,08h,0f0h,00h,070h,080h,0f0h,088h,070h,00h,0f8h,08h,010h,020h,020h,00h
	DB	070h,088h,070h,088h,070h,00h,070h,088h,078h,08h,070h,00h,030h,030h,00h,030h
	DB	030h,00h,030h,030h,00h,030h,010h,020h,018h,030h,060h,030h,018h,00h,00h,078h
	DB	00h,078h,00h,00h,060h,030h,018h,030h,060h,00h,070h,098h,030h,030h,00h,030h
	DB	070h,088h,0b8h,0b0h,080h,078h,070h,088h,0f8h,088h,088h,00h,0f0h,088h,0f0h,088h
	DB	0f0h,00h,078h,080h,080h,080h,078h,00h,0f0h,088h,088h,088h,0f0h,00h,0f8h,080h
	DB	0f0h,080h,0f8h,00h,0f8h,080h,0f0h,080h,080h,00h,078h,080h,098h,088h,078h,00h
	DB	088h,088h,0f8h,088h,088h,00h,070h,020h,020h,020h,070h,00h,08h,08h,08h,088h
	DB	070h,00h,048h,050h,060h,050h,048h,00h,040h,040h,040h,040h,078h,00h,088h,0d8h
	DB	0a8h,088h,088h,00h,088h,0c8h,0a8h,098h,088h,00h,070h,088h,088h,088h,070h,00h
	DB	0f0h,088h,0f0h,080h,080h,00h,070h,088h,088h,088h,070h,018h,0f0h,088h,0f0h,0a0h
	DB	098h,00h,078h,080h,070h,08h,0f0h,00h,0f8h,020h,020h,020h,020h,00h,088h,088h
	DB	088h,088h,078h,00h,088h,088h,088h,050h,020h,00h,088h,088h,0a8h,0d8h,088h,00h
	DB	088h,050h,020h,050h,088h,00h,088h,088h,050h,020h,020h,00h,0f8h,010h,020h,040h
	DB	0f8h,00h,078h,060h,060h,060h,078h,00h,0c0h,060h,030h,018h,08h,00h,078h,018h
	DB	018h,018h,078h,00h,020h,070h,0d8h,00h,00h,00h,00h,00h,00h,00h,00h,0f8h
	DB	060h,060h,030h,00h,00h,00h,00h,070h,08h,0f8h,078h,00h,080h,0f0h,088h,088h
	DB	0f0h,00h,00h,070h,080h,080h,070h,00h,08h,078h,088h,088h,078h,00h,00h,070h
	DB	0f8h,080h,070h,00h,018h,020h,078h,020h,020h,00h,00h,078h,088h,078h,08h,0f0h
	DB	080h,0b0h,0c8h,088h,088h,00h,00h,060h,020h,020h,070h,00h,010h,010h,010h,010h
	DB	010h,0e0h,080h,090h,0e0h,090h,088h,00h,060h,020h,020h,020h,070h,00h,00h,050h
	DB	0f8h,0a8h,088h,00h,00h,0f0h,088h,088h,088h,00h,00h,070h,088h,088h,070h,00h
	DB	00h,0f0h,088h,088h,0f0h,080h,00h,078h,088h,088h,078h,08h,00h,070h,048h,040h
	DB	040h,00h,00h,038h,060h,018h,070h,00h,020h,070h,020h,020h,010h,00h,00h,088h
	DB	088h,088h,078h,00h,00h,088h,088h,050h,020h,00h,00h,088h,0a8h,0a8h,0d8h,00h
	DB	00h,048h,030h,030h,048h,00h,00h,088h,088h,078h,08h,0f0h,00h,078h,010h,020h
	DB	078h,00h,038h,030h,060h,030h,038h,00h,030h,030h,030h,030h,030h,030h,070h,030h
	DB	018h,030h,070h,00h,040h,0e8h,0b8h,010h,00h,00h,00h,020h,070h,0c8h,0f8h,00h
	DB	078h,080h,080h,080h,078h,0c0h,050h,00h,088h,088h,088h,078h,010h,020h,070h,0f8h
	DB	080h,070h,020h,050h,070h,08h,0f8h,078h,050h,00h,070h,08h,0f8h,078h,040h,020h
	DB	070h,08h,0f8h,078h,020h,00h,070h,08h,0f8h,078h,00h,078h,080h,080h,078h,0e0h
	DB	020h,050h,070h,0f8h,080h,070h,050h,00h,070h,0f8h,080h,070h,040h,020h,070h,0f8h
	DB	080h,070h,050h,00h,060h,020h,020h,070h,020h,050h,020h,020h,020h,070h,040h,020h
	DB	060h,020h,020h,070h,050h,00h,070h,088h,0f8h,088h,020h,00h,070h,088h,0f8h,088h
	DB	020h,0f8h,080h,0f0h,080h,0f8h,00h,0f0h,038h,0e0h,078h,00h,078h,0a0h,0f8h,0a0h
	DB	0b8h,00h,020h,050h,070h,088h,088h,070h,050h,00h,070h,088h,088h,070h,040h,020h
	DB	070h,088h,088h,070h,020h,050h,088h,088h,088h,078h,040h,020h,088h,088h,088h,078h
	DB	050h,00h,088h,078h,08h,0f0h,050h,00h,070h,088h,088h,070h,050h,00h,088h,088h
	DB	088h,078h,010h,038h,060h,038h,010h,00h,018h,020h,070h,020h,078h,00h,088h,0d8h
	DB	070h,020h,070h,020h,0f0h,0d8h,0f0h,0d8h,0f0h,0c0h,018h,020h,078h,020h,020h,0c0h
	DB	010h,020h,070h,08h,0f8h,078h,010h,020h,060h,020h,020h,070h,010h,020h,070h,088h
	DB	088h,070h,010h,020h,088h,088h,088h,078h,0f0h,00h,0f0h,088h,088h,088h,078h,00h
	DB	048h,068h,058h,048h,070h,08h,0f8h,078h,00h,0f8h,070h,088h,088h,070h,00h,0f8h
	DB	060h,00h,060h,060h,0c8h,070h,00h,00h,078h,040h,040h,00h,00h,00h,078h,08h
	DB	08h,00h,0c0h,0c0h,0d8h,02ch,018h,03ch,0c0h,0c0h,0cch,014h,03ch,04h,030h,00h
	DB	030h,030h,030h,030h,034h,06ch,0d8h,06ch,034h,00h,0b0h,0d8h,06ch,0d8h,0b0h,00h
	DB	068h,0b0h,070h,08h,0f8h,078h,068h,0b0h,070h,088h,088h,070h,034h,048h,058h,068h
	DB	048h,0b0h,08h,030h,058h,068h,030h,040h,00h,070h,0b8h,0a0h,078h,00h,078h,0b0h
	DB	0b8h,0b0h,078h,00h,040h,020h,070h,088h,0f8h,088h,068h,0b0h,070h,088h,0f8h,088h
	DB	068h,0b0h,070h,088h,088h,070h,050h,00h,00h,00h,00h,00h,010h,020h,00h,00h
	DB	00h,00h,020h,070h,020h,020h,020h,00h,068h,0e8h,0e8h,068h,028h,028h,078h,0cch
	DB	0b4h,0bch,0c4h,078h,078h,08ch,0b4h,08ch,0a4h,078h,0f4h,05ch,054h,00h,00h,00h
	DB	048h,00h,048h,048h,048h,010h,0e8h,048h,048h,0e8h,08h,030h,098h,0d8h,070h,0d8h
	DB	0c8h,00h,0f0h,030h,030h,030h,0f8h,00h,078h,08h,018h,038h,068h,00h,0fch,018h
	DB	018h,018h,018h,00h,0f0h,018h,0d8h,0d8h,0d8h,00h,070h,030h,030h,030h,030h,00h
	DB	078h,030h,018h,018h,018h,00h,0fch,06ch,06ch,06ch,06ch,00h,0c0h,0d8h,0d8h,0d8h
	DB	0f8h,00h,078h,018h,018h,00h,00h,00h,0f0h,018h,018h,018h,0f0h,00h,0c0h,0f8h
	DB	018h,018h,078h,00h,0d8h,07ch,064h,064h,06ch,00h,038h,018h,018h,018h,078h,00h
	DB	0f8h,0d8h,0d8h,0d8h,070h,00h,06ch,06ch,06ch,06ch,0fch,00h,0f8h,098h,0d8h,018h
	DB	0f8h,00h,0d8h,0d8h,070h,038h,0f8h,00h,0f8h,018h,0d8h,0d0h,0c0h,0c0h,070h,018h
	DB	018h,018h,018h,00h,0d4h,0d4h,0f4h,0c4h,0fch,00h,078h,058h,058h,058h,0d8h,00h
	DB	038h,018h,018h,018h,018h,00h,0f8h,018h,018h,018h,018h,00h,00h,0fch,06ch,06ch
	DB	07ch,00h,0f8h,098h,0d8h,018h,018h,00h,0d8h,0d8h,070h,030h,030h,00h,030h,028h
	DB	070h,070h,0a0h,060h,00h,020h,070h,0d8h,088h,00h,00h,058h,0b4h,0b4h,068h,00h
	DB	00h,068h,0d0h,0d0h,068h,00h,070h,0d8h,0f0h,0d8h,0f0h,0c0h,0fch,06ch,064h,060h
	DB	060h,060h,00h,0f8h,050h,050h,050h,00h,0f8h,060h,030h,060h,0c0h,0f8h,038h,070h
	DB	0d8h,0d8h,070h,00h,00h,068h,068h,068h,07ch,0c0h,00h,0f8h,030h,030h,030h,020h
	DB	070h,070h,0a8h,0a8h,070h,070h,070h,0d8h,0f8h,0d8h,070h,00h,070h,0d8h,0d8h,050h
	DB	0d8h,00h,030h,068h,070h,0d8h,0d8h,070h,018h,020h,070h,0a8h,070h,0c0h,020h,070h
	DB	0a8h,0a8h,070h,020h,038h,040h,078h,040h,038h,00h,030h,048h,048h,048h,048h,00h
	DB	078h,00h,078h,00h,078h,00h,020h,0f8h,020h,00h,0f8h,00h,060h,018h,060h,00h
	DB	078h,00h,018h,060h,018h,00h,078h,00h,0ch,010h,030h,030h,030h,030h,030h,030h
	DB	030h,030h,020h,0c0h,020h,00h,0f8h,00h,020h,00h,068h,0b0h,00h,068h,0b0h,00h
	DB	030h,048h,030h,00h,00h,00h,030h,078h,030h,00h,00h,00h,00h,00h,030h,030h
	DB	00h,00h,00h,01ch,010h,0d0h,070h,020h,070h,068h,068h,068h,00h,00h,070h,018h
	DB	030h,078h,00h,00h,070h,030h,010h,070h,00h,00h,0f8h,00h,00h,00h,00h,00h

font_8_bitmap LABEL WORD
	DW 8	; Fontbreite
	DW 8	; Fonth”he
	DB	00h,00h,00h,00h,00h,00h,00h,00h,018h,03ch,066h,0c3h,0e7h,024h,024h,03ch
	DB	03ch,024h,024h,0e7h,0c3h,066h,03ch,018h,018h,01ch,0f6h,083h,083h,0f6h,01ch,018h
	DB	018h,038h,06fh,0c1h,0c1h,06fh,038h,018h,03ch,099h,0c3h,0e7h,0c3h,099h,03ch,00h
	DB	0ffh,0ffh,0feh,0fch,0f9h,0f3h,0e7h,00h,0e7h,0c3h,099h,03ch,099h,0c3h,0e7h,00h
	DB	01h,03h,06h,08ch,0d8h,070h,020h,00h,07eh,0c3h,0d3h,0d3h,0dbh,0c3h,0c3h,07eh
	DB	018h,03ch,03ch,03ch,07eh,010h,038h,010h,018h,01ch,016h,010h,010h,070h,0f0h,060h
	DB	0f0h,0c0h,0feh,0d8h,0deh,018h,018h,00h,0f0h,0c0h,0dfh,0dbh,0ffh,01eh,01bh,00h
	DB	05h,05h,05h,0dh,0dh,019h,079h,071h,0a0h,0a0h,0a0h,0b0h,0b0h,098h,09eh,08eh
	DB	07ch,0c6h,0c6h,00h,0c6h,0c6h,07ch,00h,06h,06h,06h,00h,06h,06h,06h,00h
	DB	07ch,06h,06h,07ch,0c0h,0c0h,07ch,00h,07ch,06h,06h,07ch,06h,06h,07ch,00h
	DB	0c6h,0c6h,0c6h,07ch,06h,06h,06h,00h,07ch,0c0h,0c0h,07ch,06h,06h,07ch,00h
	DB	07ch,0c0h,0c0h,07ch,0c6h,0c6h,07ch,00h,07ch,06h,06h,00h,06h,06h,06h,00h
	DB	07ch,0c6h,0c6h,07ch,0c6h,0c6h,07ch,00h,07ch,0c6h,0c6h,07ch,06h,06h,07ch,00h
	DB	00h,00h,03ch,06h,07eh,066h,03ch,00h,078h,060h,078h,060h,07eh,018h,01eh,00h
	DB	07h,0fh,01fh,018h,018h,010h,01eh,017h,0f0h,0f8h,0ech,04h,04h,04h,03ch,054h
	DB	011h,0bh,0dh,06h,07h,02eh,039h,038h,04h,028h,0d8h,028h,0d0h,010h,0e0h,00h
	DB	00h,00h,00h,00h,00h,00h,00h,00h,018h,018h,018h,018h,018h,00h,018h,00h
	DB	066h,066h,066h,00h,00h,00h,00h,00h,00h,06ch,0feh,06ch,06ch,0feh,06ch,00h
	DB	018h,03eh,060h,03ch,06h,07ch,018h,00h,00h,066h,06ch,018h,030h,066h,046h,00h
	DB	038h,06ch,038h,070h,0deh,0cch,076h,00h,018h,018h,018h,00h,00h,00h,00h,00h
	DB	0eh,01ch,018h,018h,018h,01ch,0eh,00h,070h,038h,018h,018h,018h,038h,070h,00h
	DB	00h,066h,03ch,0ffh,03ch,066h,00h,00h,00h,018h,018h,07eh,018h,018h,00h,00h
	DB	00h,00h,00h,00h,00h,030h,030h,060h,00h,00h,00h,07eh,00h,00h,00h,00h
	DB	00h,00h,00h,00h,00h,018h,018h,00h,02h,06h,0ch,018h,030h,060h,040h,00h
	DB	03ch,066h,06eh,076h,066h,066h,03ch,00h,018h,038h,018h,018h,018h,018h,07eh,00h
	DB	03ch,066h,06h,0ch,018h,030h,07eh,00h,07eh,0ch,018h,0ch,06h,066h,03ch,00h
	DB	0ch,01ch,03ch,06ch,07eh,0ch,0ch,00h,07eh,060h,07ch,06h,06h,066h,03ch,00h
	DB	03ch,060h,060h,07ch,066h,066h,03ch,00h,07eh,06h,0ch,018h,030h,030h,030h,00h
	DB	03ch,066h,066h,03ch,066h,066h,03ch,00h,03ch,066h,066h,03eh,06h,0ch,038h,00h
	DB	00h,018h,018h,00h,018h,018h,00h,00h,00h,018h,018h,00h,018h,018h,030h,00h
	DB	06h,0ch,018h,030h,018h,0ch,06h,00h,00h,00h,07eh,00h,00h,07eh,00h,00h
	DB	060h,030h,018h,0ch,018h,030h,060h,00h,03ch,066h,06h,0ch,018h,00h,018h,00h
	DB	03ch,066h,06eh,06ah,06eh,060h,03eh,00h,018h,03ch,066h,066h,07eh,066h,066h,00h
	DB	07ch,066h,066h,07ch,066h,066h,07ch,00h,03ch,066h,060h,060h,060h,066h,03ch,00h
	DB	078h,06ch,066h,066h,066h,06ch,078h,00h,07eh,060h,060h,07ch,060h,060h,07eh,00h
	DB	07eh,060h,060h,07ch,060h,060h,060h,00h,03eh,060h,060h,06eh,066h,066h,03eh,00h
	DB	066h,066h,066h,07eh,066h,066h,066h,00h,03ch,018h,018h,018h,018h,018h,03ch,00h
	DB	06h,06h,06h,06h,06h,066h,03ch,00h,066h,06ch,078h,070h,078h,06ch,066h,00h
	DB	060h,060h,060h,060h,060h,060h,07eh,00h,0c6h,0eeh,0feh,0d6h,0c6h,0c6h,0c6h,00h
	DB	066h,076h,07eh,07eh,06eh,066h,066h,00h,03ch,066h,066h,066h,066h,066h,03ch,00h
	DB	07ch,066h,066h,07ch,060h,060h,060h,00h,03ch,066h,066h,066h,076h,06ch,036h,00h
	DB	07ch,066h,066h,07ch,06ch,066h,066h,00h,03ch,066h,060h,03ch,06h,066h,03ch,00h
	DB	07eh,018h,018h,018h,018h,018h,018h,00h,066h,066h,066h,066h,066h,066h,03eh,00h
	DB	066h,066h,066h,066h,066h,03ch,018h,00h,0c6h,0c6h,0c6h,0d6h,0feh,0eeh,0c6h,00h
	DB	066h,066h,03ch,018h,03ch,066h,066h,00h,066h,066h,066h,03ch,018h,018h,018h,00h
	DB	07eh,06h,0ch,018h,030h,060h,07eh,00h,01eh,018h,018h,018h,018h,018h,01eh,00h
	DB	040h,060h,030h,018h,0ch,06h,02h,00h,078h,018h,018h,018h,018h,018h,078h,00h
	DB	010h,038h,06ch,0c6h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,0feh,00h
	DB	00h,0c0h,060h,030h,00h,00h,00h,00h,00h,00h,03ch,06h,03eh,066h,03eh,00h
	DB	060h,060h,07ch,066h,066h,066h,07ch,00h,00h,00h,03ch,060h,060h,060h,03ch,00h
	DB	06h,06h,03eh,066h,066h,066h,03eh,00h,00h,00h,03ch,066h,07eh,060h,03ch,00h
	DB	01ch,030h,07ch,030h,030h,030h,030h,00h,00h,00h,03eh,066h,066h,03eh,06h,07ch
	DB	060h,060h,07ch,066h,066h,066h,066h,00h,018h,00h,038h,018h,018h,018h,03ch,00h
	DB	018h,00h,018h,018h,018h,018h,018h,070h,060h,060h,066h,06ch,078h,06ch,066h,00h
	DB	038h,018h,018h,018h,018h,018h,03ch,00h,00h,00h,0ech,0feh,0d6h,0c6h,0c6h,00h
	DB	00h,00h,07ch,066h,066h,066h,066h,00h,00h,00h,03ch,066h,066h,066h,03ch,00h
	DB	00h,00h,07ch,066h,066h,066h,07ch,060h,00h,00h,03eh,066h,066h,066h,03eh,06h
	DB	00h,00h,07ch,066h,060h,060h,060h,00h,00h,00h,03eh,060h,03ch,06h,07ch,00h
	DB	00h,018h,07eh,018h,018h,018h,0eh,00h,00h,00h,066h,066h,066h,066h,03eh,00h
	DB	00h,00h,066h,066h,066h,03ch,018h,00h,00h,00h,0c6h,0c6h,0d6h,07ch,06ch,00h
	DB	00h,00h,066h,03ch,018h,03ch,066h,00h,00h,00h,066h,066h,066h,03eh,06h,07ch
	DB	00h,00h,07eh,0ch,018h,030h,07eh,00h,0eh,018h,018h,030h,018h,018h,0eh,00h
	DB	018h,018h,018h,018h,018h,018h,018h,018h,070h,018h,018h,0ch,018h,018h,070h,00h
	DB	00h,060h,0f2h,09eh,0ch,00h,00h,00h,00h,018h,018h,034h,034h,062h,07eh,00h
	DB	00h,03ch,066h,060h,066h,03ch,08h,038h,066h,00h,00h,066h,066h,066h,03eh,00h
	DB	0ch,018h,00h,03ch,07eh,060h,03ch,00h,018h,066h,00h,03ch,06h,07eh,03eh,00h
	DB	066h,00h,03ch,06h,03eh,066h,03eh,00h,030h,018h,00h,03ch,06h,07eh,03eh,00h
	DB	018h,018h,00h,03ch,06h,07eh,03eh,00h,00h,00h,03ch,060h,060h,03ch,08h,018h
	DB	018h,066h,00h,03ch,07eh,060h,03ch,00h,066h,00h,03ch,066h,07eh,060h,03ch,00h
	DB	030h,018h,00h,03ch,07eh,060h,03ch,00h,066h,00h,00h,038h,018h,018h,03ch,00h
	DB	018h,066h,00h,038h,018h,018h,03ch,00h,060h,030h,00h,038h,018h,018h,03ch,00h
	DB	066h,00h,018h,03ch,066h,07eh,066h,00h,018h,00h,018h,03ch,066h,07eh,066h,00h
	DB	0ch,018h,07eh,060h,07ch,060h,07eh,00h,00h,00h,07eh,01bh,07fh,0d8h,07eh,00h
	DB	03fh,078h,0d8h,0deh,0f8h,0d8h,0dfh,00h,018h,066h,00h,03ch,066h,066h,03ch,00h
	DB	066h,00h,00h,03ch,066h,066h,03ch,00h,030h,018h,00h,03ch,066h,066h,03ch,00h
	DB	018h,066h,00h,066h,066h,066h,03eh,00h,030h,018h,00h,066h,066h,066h,03eh,00h
	DB	066h,00h,066h,066h,066h,03eh,06h,07ch,066h,00h,03ch,066h,066h,066h,03ch,00h
	DB	066h,00h,066h,066h,066h,066h,03eh,00h,018h,018h,03ch,060h,060h,03ch,018h,018h
	DB	01ch,03ah,030h,07ch,030h,030h,07eh,00h,066h,066h,03ch,018h,03ch,018h,018h,00h
	DB	01ch,036h,066h,07ch,066h,066h,07ch,060h,01eh,030h,07ch,030h,030h,030h,060h,00h
	DB	0ch,018h,00h,03ch,06h,07eh,03eh,00h,0ch,018h,00h,038h,018h,018h,03ch,00h
	DB	0ch,018h,00h,03ch,066h,066h,03ch,00h,0ch,018h,00h,066h,066h,066h,03eh,00h
	DB	034h,058h,00h,07ch,066h,066h,066h,00h,034h,058h,00h,066h,076h,06eh,066h,00h
	DB	00h,03ch,06h,03eh,066h,03eh,00h,03ch,00h,03ch,066h,066h,066h,03ch,00h,03ch
	DB	00h,018h,00h,018h,030h,060h,066h,03ch,00h,00h,00h,03eh,030h,030h,030h,00h
	DB	00h,00h,00h,07ch,0ch,0ch,0ch,00h,0c6h,0cch,0d8h,036h,06bh,0c3h,086h,0fh
	DB	0c6h,0cch,0d8h,036h,06eh,0d6h,09fh,06h,00h,018h,00h,018h,018h,018h,018h,018h
	DB	01bh,036h,06ch,0d8h,06ch,036h,01bh,00h,0d8h,06ch,036h,01bh,036h,06ch,0d8h,00h
	DB	034h,058h,00h,03ch,06h,07eh,03eh,00h,034h,058h,00h,03ch,066h,066h,03ch,00h
	DB	02h,03ch,066h,06eh,076h,066h,03ch,040h,00h,02h,03ch,06eh,076h,066h,03ch,040h
	DB	00h,00h,07eh,0dbh,0dfh,0d8h,07eh,00h,07fh,0d8h,0d8h,0deh,0d8h,0d8h,07fh,00h
	DB	030h,018h,00h,018h,03ch,066h,07eh,066h,034h,058h,00h,018h,03ch,066h,07eh,066h
	DB	034h,058h,03ch,066h,066h,066h,066h,03ch,066h,00h,00h,00h,00h,00h,00h,00h
	DB	0ch,018h,030h,00h,00h,00h,00h,00h,00h,010h,038h,010h,010h,010h,00h,00h
	DB	07ah,0cah,0cah,0cah,07ah,0ah,0ah,0ah,07eh,0c3h,0bdh,0b1h,0b1h,0bdh,0c3h,07eh
	DB	07eh,0c3h,0bdh,0a5h,0b9h,0adh,0c3h,07eh,0f1h,05bh,05fh,055h,051h,00h,00h,00h
	DB	066h,00h,0e6h,066h,066h,0f6h,06h,01ch,0f6h,066h,066h,066h,066h,0f6h,06h,01ch
	DB	00h,066h,076h,03ch,06eh,066h,00h,00h,00h,07ch,0ch,0ch,0ch,07eh,00h,00h
	DB	00h,01eh,06h,0eh,01eh,036h,00h,00h,00h,07eh,0ch,0ch,0ch,0ch,00h,00h
	DB	00h,07ch,06h,066h,066h,066h,00h,00h,00h,01ch,0ch,0ch,0ch,0ch,00h,00h
	DB	00h,01eh,0ch,06h,06h,06h,00h,00h,00h,07eh,036h,036h,036h,036h,00h,00h
	DB	060h,06eh,066h,066h,066h,07eh,00h,00h,00h,03ch,0ch,0ch,00h,00h,00h,00h
	DB	00h,03eh,06h,06h,06h,03eh,00h,00h,060h,07eh,06h,06h,06h,0eh,00h,00h
	DB	00h,06ch,03eh,066h,066h,06eh,00h,00h,00h,01ch,0ch,0ch,0ch,03ch,00h,00h
	DB	00h,03eh,036h,036h,036h,01ch,00h,00h,00h,036h,036h,036h,036h,07eh,00h,00h
	DB	00h,07eh,066h,076h,06h,07eh,00h,00h,00h,066h,066h,03ch,0eh,07eh,00h,00h
	DB	00h,03eh,06h,036h,036h,034h,030h,00h,00h,078h,0ch,0ch,0ch,0ch,00h,00h
	DB	00h,0d6h,0d6h,0d6h,0d6h,0feh,00h,00h,00h,07ch,06ch,06ch,06ch,0ech,00h,00h
	DB	00h,01ch,0ch,0ch,0ch,0ch,0ch,00h,00h,03eh,06h,06h,06h,06h,06h,00h
	DB	00h,0feh,066h,066h,066h,07eh,00h,00h,00h,07eh,066h,076h,06h,06h,06h,00h
	DB	00h,036h,036h,01ch,0ch,0ch,0ch,00h,0eh,01bh,03ch,066h,066h,03ch,0d8h,070h
	DB	00h,010h,038h,06ch,0c6h,082h,00h,00h,066h,0f7h,099h,099h,0efh,066h,00h,00h
	DB	00h,00h,076h,0dch,0c8h,0dch,076h,00h,01ch,036h,066h,07ch,066h,066h,07ch,060h
	DB	00h,0feh,066h,062h,060h,060h,060h,0f8h,00h,00h,0feh,06ch,06ch,06ch,06ch,048h
	DB	0feh,066h,030h,018h,030h,066h,0feh,00h,00h,01eh,038h,06ch,06ch,06ch,038h,00h
	DB	00h,00h,06ch,06ch,06ch,06ch,07fh,0c0h,00h,00h,07eh,018h,018h,018h,018h,010h
	DB	03ch,018h,03ch,066h,066h,03ch,018h,03ch,00h,03ch,066h,07eh,066h,066h,03ch,00h
	DB	00h,03ch,066h,066h,066h,024h,066h,00h,01ch,036h,078h,0dch,0cch,0ech,078h,00h
	DB	0ch,018h,038h,054h,054h,038h,030h,060h,00h,010h,07ch,0d6h,0d6h,0d6h,07ch,010h
	DB	03eh,070h,060h,07eh,060h,070h,03eh,00h,03ch,066h,066h,066h,066h,066h,066h,00h
	DB	00h,07eh,00h,07eh,00h,07eh,00h,00h,018h,018h,07eh,018h,018h,00h,07eh,00h
	DB	030h,018h,0ch,018h,030h,00h,07eh,00h,0ch,018h,030h,018h,0ch,00h,07eh,00h
	DB	00h,0eh,01bh,01bh,018h,018h,018h,018h,018h,018h,018h,018h,0d8h,0d8h,070h,00h
	DB	018h,018h,00h,07eh,00h,018h,018h,00h,00h,032h,04ch,00h,032h,04ch,00h,00h
	DB	038h,06ch,038h,00h,00h,00h,00h,00h,038h,07ch,038h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,018h,018h,00h,00h,00h,00h,0fh,018h,0d8h,070h,030h,00h
	DB	038h,06ch,06ch,06ch,06ch,00h,00h,00h,038h,06ch,018h,030h,07ch,00h,00h,00h
	DB	078h,0ch,038h,0ch,078h,00h,00h,00h,00h,0feh,00h,00h,00h,00h,00h,00h

font_16_bitmap LABEL WORD
	DW 8	; Fontbreite
	DW 16	; Fonth”he
	DB	00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,018h,03ch,066h,0c3h,081h,0e7h,024h,024h,024h,03ch,00h,00h
	DB	00h,00h,00h,00h,03ch,024h,024h,024h,0e7h,081h,0c3h,066h,03ch,018h,00h,00h
	DB	00h,00h,00h,030h,038h,02ch,0e6h,083h,083h,0e6h,02ch,038h,030h,00h,00h,00h
	DB	00h,00h,00h,0ch,01ch,034h,067h,0c1h,0c1h,067h,034h,01ch,0ch,00h,00h,00h
	DB	00h,00h,00h,07ch,038h,0bah,092h,0d6h,0c6h,0d6h,092h,0bah,038h,07ch,00h,00h
	DB	00h,00h,00h,0feh,0feh,0feh,0fch,0fch,0f8h,0fah,0f2h,0f6h,0e6h,0eeh,00h,00h
	DB	00h,00h,00h,0eeh,0c6h,0d6h,092h,0bah,038h,0bah,092h,0d6h,0c6h,0eeh,00h,00h
	DB	00h,00h,00h,01h,01h,03h,03h,06h,06h,08ch,08ch,0d8h,058h,070h,030h,020h
	DB	00h,00h,00h,00h,03ch,066h,0c3h,091h,091h,09dh,081h,0c3h,066h,03ch,00h,00h
	DB	00h,00h,00h,00h,00h,018h,03ch,03ch,03ch,03ch,03ch,07eh,0ffh,010h,038h,010h
	DB	00h,00h,00h,08h,0eh,0fh,09h,08h,08h,078h,0f8h,070h,00h,00h,00h,00h
	DB	00h,00h,00h,078h,040h,070h,040h,040h,00h,01eh,010h,01ch,010h,010h,00h,00h
	DB	00h,00h,00h,038h,040h,040h,040h,038h,00h,01ch,012h,01ch,014h,012h,00h,00h
	DB	00h,00h,05h,05h,05h,05h,0dh,0dh,01dh,039h,079h,071h,071h,061h,041h,00h
	DB	00h,00h,0a0h,0a0h,0a0h,0a0h,0b0h,0b0h,0b8h,09ch,09eh,08eh,08eh,086h,082h,00h
	DB	00h,00h,07ch,0bah,0c6h,0c6h,0c6h,082h,00h,082h,0c6h,0c6h,0c6h,0bah,07ch,00h
	DB	00h,00h,00h,02h,06h,06h,06h,02h,00h,02h,06h,06h,06h,02h,00h,00h
	DB	00h,00h,07ch,03ah,06h,06h,06h,03ah,07ch,0b8h,0c0h,0c0h,0c0h,0b8h,07ch,00h
	DB	00h,00h,07ch,03ah,06h,06h,06h,03ah,07ch,03ah,06h,06h,06h,03ah,07ch,00h
	DB	00h,00h,00h,082h,0c6h,0c6h,0c6h,0bah,07ch,03ah,06h,06h,06h,02h,00h,00h
	DB	00h,00h,07ch,0b8h,0c0h,0c0h,0c0h,0b8h,07ch,03ah,06h,06h,06h,03ah,07ch,00h
	DB	00h,00h,07ch,0b8h,0c0h,0c0h,0c0h,0b8h,07ch,0bah,0c6h,0c6h,0c6h,0bah,07ch,00h
	DB	00h,00h,07ch,0bah,0c6h,0c6h,0c6h,082h,00h,02h,06h,06h,06h,02h,00h,00h
	DB	00h,00h,07ch,0bah,0c6h,0c6h,0c6h,0bah,07ch,0bah,0c6h,0c6h,0c6h,0bah,07ch,00h
	DB	00h,00h,07ch,0bah,0c6h,0c6h,0c6h,0bah,07ch,03ah,06h,06h,06h,03ah,07ch,00h
	DB	00h,00h,00h,00h,00h,00h,07ch,07eh,06h,06h,07eh,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,078h,040h,070h,040h,078h,00h,0eh,010h,0ch,02h,01ch,00h,00h
	DB	00h,00h,00h,00h,00h,00h,00h,00h,00h,07h,0fh,01fh,018h,018h,010h,01eh
	DB	00h,00h,00h,00h,00h,00h,00h,00h,00h,0f0h,0f8h,0ech,04h,04h,04h,03ch
	DB	00h,011h,0bh,0dh,06h,07h,02eh,039h,038h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,04h,028h,0d8h,028h,0d0h,010h,0e0h,00h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,018h,018h,018h,018h,018h,018h,018h,018h,00h,00h,018h,018h,00h
	DB	00h,00h,00h,066h,066h,066h,066h,066h,066h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,066h,066h,0ffh,0ffh,066h,066h,0ffh,0ffh,066h,066h,00h,00h,00h
	DB	00h,018h,018h,03eh,07eh,060h,060h,07ch,03eh,06h,06h,07eh,07ch,018h,018h,00h
	DB	00h,00h,00h,066h,066h,06ch,0ch,018h,018h,030h,036h,066h,066h,00h,00h,00h
	DB	00h,038h,07ch,06ch,06ch,038h,038h,070h,070h,0deh,0deh,0cch,0cch,0feh,076h,00h
	DB	00h,00h,00h,018h,018h,018h,018h,018h,018h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,06h,0ch,01ch,018h,018h,018h,018h,018h,018h,01ch,0ch,06h,00h
	DB	00h,00h,00h,060h,030h,038h,018h,018h,018h,018h,018h,018h,038h,030h,060h,00h
	DB	00h,00h,00h,066h,066h,03ch,03ch,0ffh,0ffh,03ch,03ch,066h,066h,00h,00h,00h
	DB	00h,00h,00h,00h,018h,018h,018h,07eh,07eh,018h,018h,018h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,018h,018h,018h,018h,030h
	DB	00h,00h,00h,00h,00h,00h,00h,07eh,07eh,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,018h,018h,018h,018h,00h
	DB	00h,00h,00h,06h,06h,06h,0ch,0ch,018h,018h,030h,030h,060h,060h,060h,00h
	DB	00h,00h,00h,03ch,07eh,066h,066h,066h,06eh,076h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,018h,018h,038h,038h,018h,018h,018h,018h,018h,018h,07eh,07eh,00h
	DB	00h,00h,00h,03ch,07eh,066h,066h,0ch,0ch,018h,018h,030h,030h,07eh,07eh,00h
	DB	00h,00h,00h,07eh,07eh,0ch,0ch,018h,018h,0ch,0ch,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,0ch,0ch,01ch,01ch,03ch,03ch,06ch,06ch,07eh,07eh,0ch,0ch,00h
	DB	00h,00h,00h,07eh,07eh,060h,060h,07ch,07eh,06h,06h,06h,066h,07eh,03ch,00h
	DB	00h,00h,00h,01ch,03ch,070h,060h,060h,07ch,07eh,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,07eh,07eh,06h,06h,0ch,0ch,018h,018h,030h,030h,030h,030h,00h
	DB	00h,00h,00h,03ch,07eh,066h,066h,03ch,03ch,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,03ch,07eh,066h,066h,07eh,03eh,06h,06h,06h,0eh,03ch,038h,00h
	DB	00h,00h,00h,00h,00h,018h,018h,018h,018h,00h,00h,018h,018h,018h,018h,00h
	DB	00h,00h,00h,00h,00h,018h,018h,018h,018h,00h,00h,018h,018h,018h,018h,030h
	DB	00h,00h,00h,00h,0eh,01ch,038h,070h,0e0h,070h,038h,01ch,0eh,00h,00h,00h
	DB	00h,00h,00h,00h,00h,07eh,07eh,00h,00h,07eh,07eh,00h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,0e0h,070h,038h,01ch,0eh,01ch,038h,070h,0e0h,00h,00h,00h
	DB	00h,00h,00h,03ch,07eh,066h,066h,0ch,0ch,018h,018h,018h,00h,018h,018h,00h
	DB	00h,00h,00h,038h,07ch,0e6h,0c2h,0dah,0d6h,0d6h,0dch,0c0h,0e2h,07eh,03ch,00h
	DB	00h,00h,00h,018h,03ch,07eh,066h,066h,066h,07eh,07eh,066h,066h,066h,066h,00h
	DB	00h,00h,00h,07ch,07eh,066h,066h,07eh,07ch,066h,066h,066h,066h,07eh,07ch,00h
	DB	00h,00h,00h,03ch,07eh,066h,066h,060h,060h,060h,060h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,078h,07ch,06eh,066h,066h,066h,066h,066h,066h,06eh,07ch,078h,00h
	DB	00h,00h,00h,07eh,07eh,060h,060h,07ch,07ch,060h,060h,060h,060h,07eh,07eh,00h
	DB	00h,00h,00h,07eh,07eh,060h,060h,07ch,07ch,060h,060h,060h,060h,060h,060h,00h
	DB	00h,00h,00h,03eh,07eh,060h,060h,06eh,06eh,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,066h,066h,066h,066h,07eh,07eh,066h,066h,066h,066h,066h,066h,00h
	DB	00h,00h,00h,07eh,07eh,018h,018h,018h,018h,018h,018h,018h,018h,07eh,07eh,00h
	DB	00h,00h,00h,06h,06h,06h,06h,06h,06h,06h,06h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,0cch,0cch,0d8h,0d8h,0f0h,0f0h,0d8h,0d8h,0cch,0cch,0c6h,0c6h,00h
	DB	00h,00h,00h,060h,060h,060h,060h,060h,060h,060h,060h,060h,060h,07eh,07eh,00h
	DB	00h,00h,00h,0c6h,0c6h,0eeh,0eeh,0feh,0d6h,0d6h,0c6h,0c6h,0c6h,0c6h,0c6h,00h
	DB	00h,00h,00h,066h,066h,066h,076h,076h,07eh,07eh,06eh,06eh,066h,066h,066h,00h
	DB	00h,00h,00h,03ch,07eh,066h,066h,066h,066h,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,07ch,07eh,066h,066h,066h,066h,07eh,07ch,060h,060h,060h,060h,00h
	DB	00h,00h,00h,03ch,07eh,066h,066h,066h,066h,066h,066h,066h,06ah,07ch,036h,00h
	DB	00h,00h,00h,0f8h,0fch,0cch,0cch,0cch,0fch,0f8h,0d8h,0cch,0cch,0c6h,0c6h,00h
	DB	00h,00h,00h,03eh,07eh,060h,060h,070h,038h,01ch,0eh,06h,06h,07eh,07ch,00h
	DB	00h,00h,00h,07eh,07eh,018h,018h,018h,018h,018h,018h,018h,018h,018h,018h,00h
	DB	00h,00h,00h,066h,066h,066h,066h,066h,066h,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,066h,066h,066h,066h,066h,066h,066h,066h,03ch,03ch,018h,018h,00h
	DB	00h,00h,00h,0c6h,0c6h,0c6h,0c6h,0c6h,0d6h,0d6h,0feh,0feh,0eeh,0c6h,082h,00h
	DB	00h,00h,00h,066h,066h,066h,03ch,03ch,018h,018h,03ch,03ch,066h,066h,066h,00h
	DB	00h,00h,00h,066h,066h,066h,066h,03ch,03ch,018h,018h,018h,018h,018h,018h,00h
	DB	00h,00h,00h,07eh,07eh,0ch,0ch,018h,018h,030h,030h,060h,060h,07eh,07eh,00h
	DB	00h,00h,00h,01eh,01eh,018h,018h,018h,018h,018h,018h,018h,018h,01eh,01eh,00h
	DB	00h,00h,00h,060h,060h,060h,030h,030h,018h,018h,0ch,0ch,06h,06h,06h,00h
	DB	00h,00h,00h,078h,078h,018h,018h,018h,018h,018h,018h,018h,018h,078h,078h,00h
	DB	00h,00h,010h,010h,038h,038h,06ch,06ch,0c6h,0c6h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,0feh,0feh,00h
	DB	00h,040h,060h,070h,038h,01ch,0ch,04h,00h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,00h,00h,03ch,03eh,06h,03eh,07eh,066h,066h,07eh,03eh,00h
	DB	00h,00h,00h,060h,060h,060h,07ch,07eh,066h,066h,066h,066h,066h,07eh,07ch,00h
	DB	00h,00h,00h,00h,00h,00h,03ch,07ch,060h,060h,060h,060h,060h,07eh,03eh,00h
	DB	00h,00h,00h,06h,06h,06h,03eh,07eh,066h,066h,066h,066h,066h,07eh,03eh,00h
	DB	00h,00h,00h,00h,00h,00h,03ch,07eh,066h,066h,07eh,060h,060h,07eh,03eh,00h
	DB	00h,00h,00h,0eh,01eh,018h,018h,07eh,07eh,018h,018h,018h,018h,018h,018h,00h
	DB	00h,00h,00h,00h,00h,00h,03eh,07eh,066h,066h,066h,066h,07eh,03eh,06h,07eh
	DB	00h,00h,00h,060h,060h,060h,07ch,07eh,066h,066h,066h,066h,066h,066h,066h,00h
	DB	00h,00h,00h,018h,018h,00h,038h,038h,018h,018h,018h,018h,018h,03ch,03ch,00h
	DB	00h,00h,00h,0ch,0ch,00h,0ch,0ch,0ch,0ch,0ch,0ch,0ch,0ch,0ch,07ch
	DB	00h,00h,00h,0c0h,0c0h,0c0h,0cch,0dch,0f8h,0f0h,0f8h,0d8h,0cch,0ceh,0c6h,00h
	DB	00h,00h,00h,038h,038h,018h,018h,018h,018h,018h,018h,018h,018h,03ch,03ch,00h
	DB	00h,00h,00h,00h,00h,00h,06ch,0feh,0feh,0d6h,0d6h,0d6h,0c6h,0c6h,0c6h,00h
	DB	00h,00h,00h,00h,00h,00h,03ch,07eh,066h,066h,066h,066h,066h,066h,066h,00h
	DB	00h,00h,00h,00h,00h,00h,03ch,07eh,066h,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,00h,00h,00h,07ch,07eh,066h,066h,066h,066h,066h,07eh,07ch,060h
	DB	00h,00h,00h,00h,00h,00h,03eh,07eh,066h,066h,066h,066h,066h,07eh,03eh,06h
	DB	00h,00h,00h,00h,00h,00h,07ch,07eh,066h,060h,060h,060h,060h,060h,060h,00h
	DB	00h,00h,00h,00h,00h,00h,03eh,07eh,060h,070h,03ch,0eh,06h,07eh,07ch,00h
	DB	00h,00h,00h,00h,018h,018h,07eh,07eh,018h,018h,018h,018h,018h,01eh,0eh,00h
	DB	00h,00h,00h,00h,00h,00h,066h,066h,066h,066h,066h,066h,066h,07eh,03eh,00h
	DB	00h,00h,00h,00h,00h,00h,066h,066h,066h,066h,066h,03ch,03ch,018h,018h,00h
	DB	00h,00h,00h,00h,00h,00h,0c6h,0c6h,0d6h,0d6h,0feh,0feh,0eeh,0c6h,082h,00h
	DB	00h,00h,00h,00h,00h,00h,066h,066h,03ch,03ch,018h,03ch,03ch,066h,066h,00h
	DB	00h,00h,00h,00h,00h,00h,066h,066h,066h,066h,066h,066h,07eh,03eh,06h,07eh
	DB	00h,00h,00h,00h,00h,00h,07eh,07eh,0ch,018h,018h,030h,030h,07eh,07eh,00h
	DB	00h,00h,0eh,018h,018h,018h,018h,038h,0f0h,0f0h,038h,018h,018h,018h,018h,0eh
	DB	00h,00h,018h,018h,018h,018h,018h,018h,018h,018h,018h,018h,018h,018h,018h,018h
	DB	00h,00h,0e0h,030h,030h,030h,030h,038h,01eh,01eh,038h,030h,030h,030h,030h,0e0h
	DB	00h,00h,00h,00h,00h,00h,062h,0f2h,0beh,09ch,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,00h,00h,018h,018h,03ch,024h,066h,042h,0c3h,0ffh,00h,00h
	DB	00h,00h,00h,03ch,07eh,066h,066h,060h,060h,060h,060h,066h,066h,07eh,03ch,0ch
	DB	00h,00h,00h,066h,066h,00h,066h,066h,066h,066h,066h,066h,066h,07eh,03eh,00h
	DB	00h,00h,06h,0ch,018h,00h,03ch,07eh,066h,066h,07eh,060h,060h,07eh,03eh,00h
	DB	00h,00h,018h,03ch,066h,00h,03ch,03eh,06h,03eh,07eh,066h,066h,07eh,03eh,00h
	DB	00h,00h,00h,066h,066h,00h,03ch,03eh,06h,03eh,07eh,066h,066h,07eh,03eh,00h
	DB	00h,00h,060h,030h,018h,00h,03ch,03eh,06h,03eh,07eh,066h,066h,07eh,03eh,00h
	DB	00h,00h,01ch,036h,01ch,00h,03ch,03eh,06h,03eh,07eh,066h,066h,07eh,03eh,00h
	DB	00h,00h,00h,00h,00h,00h,03ch,07ch,060h,060h,060h,060h,060h,07eh,03eh,0ch
	DB	00h,00h,018h,03ch,066h,00h,03ch,07eh,066h,066h,07eh,060h,060h,07eh,03eh,00h
	DB	00h,00h,00h,066h,066h,00h,03ch,07eh,066h,066h,07eh,060h,060h,07eh,03eh,00h
	DB	00h,00h,060h,030h,018h,00h,03ch,07eh,066h,066h,07eh,060h,060h,07eh,03eh,00h
	DB	00h,00h,00h,066h,066h,00h,038h,038h,018h,018h,018h,018h,018h,03ch,03ch,00h
	DB	00h,00h,018h,03ch,066h,00h,038h,038h,018h,018h,018h,018h,018h,03ch,03ch,00h
	DB	00h,00h,060h,030h,018h,00h,038h,038h,018h,018h,018h,018h,018h,03ch,03ch,00h
	DB	00h,00h,066h,066h,018h,03ch,07eh,066h,066h,07eh,07eh,066h,066h,066h,066h,00h
	DB	00h,03ch,066h,03ch,018h,03ch,07eh,066h,066h,07eh,07eh,066h,066h,066h,066h,00h
	DB	00h,06h,0ch,018h,07eh,07eh,060h,060h,07ch,07ch,060h,060h,060h,07eh,07eh,00h
	DB	00h,00h,00h,00h,00h,00h,076h,07fh,01bh,07bh,0ffh,0d8h,0d8h,0ffh,07fh,00h
	DB	00h,00h,03eh,07eh,0f8h,0d8h,0d8h,0deh,0deh,0f8h,0f8h,0d8h,0d8h,0deh,0deh,00h
	DB	00h,00h,018h,03ch,066h,00h,03ch,07eh,066h,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,066h,066h,00h,03ch,07eh,066h,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,060h,030h,018h,00h,03ch,07eh,066h,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,018h,03ch,066h,00h,066h,066h,066h,066h,066h,066h,066h,07eh,03eh,00h
	DB	00h,00h,060h,030h,018h,00h,066h,066h,066h,066h,066h,066h,066h,07eh,03eh,00h
	DB	00h,00h,00h,066h,066h,00h,066h,066h,066h,066h,066h,066h,07eh,03eh,06h,07eh
	DB	00h,00h,066h,066h,00h,03ch,07eh,066h,066h,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,066h,066h,00h,066h,066h,066h,066h,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,018h,018h,03ch,07eh,066h,060h,060h,066h,07eh,03ch,018h,018h,00h
	DB	00h,00h,0eh,01eh,038h,030h,030h,030h,030h,0feh,030h,030h,030h,07fh,0ffh,00h
	DB	00h,00h,00h,066h,066h,066h,066h,07eh,03ch,018h,07eh,018h,018h,018h,018h,00h
	DB	00h,00h,00h,018h,03ch,066h,066h,066h,07ch,066h,066h,066h,07ch,06ch,060h,040h
	DB	00h,00h,00h,0eh,01eh,018h,018h,018h,07eh,018h,018h,018h,018h,018h,070h,060h
	DB	00h,00h,06h,0ch,018h,00h,03ch,03eh,06h,03eh,07eh,066h,066h,07eh,03eh,00h
	DB	00h,00h,06h,0ch,018h,00h,038h,038h,018h,018h,018h,018h,018h,03ch,03ch,00h
	DB	00h,00h,06h,0ch,018h,00h,03ch,07eh,066h,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,06h,0ch,018h,00h,066h,066h,066h,066h,066h,066h,066h,07eh,03eh,00h
	DB	00h,00h,032h,07ah,04ch,00h,03ch,07eh,066h,066h,066h,066h,066h,066h,066h,00h
	DB	00h,00h,032h,07ah,04ch,00h,066h,066h,076h,07eh,07eh,06eh,066h,066h,066h,00h
	DB	00h,00h,00h,00h,00h,03ch,03eh,06h,03eh,07eh,066h,066h,07eh,03eh,00h,07eh
	DB	00h,00h,00h,00h,00h,03ch,07eh,066h,066h,066h,066h,066h,07eh,03ch,00h,07eh
	DB	00h,00h,00h,018h,018h,00h,018h,018h,018h,030h,030h,066h,066h,07eh,03ch,00h
	DB	00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,07eh,07eh,060h,060h,060h,00h
	DB	00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,07eh,07eh,06h,06h,06h,00h
	DB	00h,00h,060h,020h,020h,023h,026h,02ch,018h,030h,06eh,0d3h,06h,0ch,018h,01fh
	DB	00h,00h,060h,020h,020h,023h,026h,02ch,018h,032h,066h,0ceh,01ah,032h,03fh,02h
	DB	00h,00h,00h,00h,018h,018h,00h,018h,018h,018h,018h,018h,018h,018h,018h,018h
	DB	00h,00h,00h,00h,00h,00h,00h,01ah,036h,06ch,0d8h,06ch,036h,01ah,00h,00h
	DB	00h,00h,00h,00h,00h,00h,00h,0b0h,0d8h,06ch,036h,06ch,0d8h,0b0h,00h,00h
	DB	00h,00h,032h,07ah,04ch,00h,03ch,03eh,06h,03eh,07eh,066h,066h,07eh,03eh,00h
	DB	00h,00h,032h,07ah,04ch,00h,03ch,07eh,066h,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,01h,03dh,07eh,066h,066h,06eh,06eh,076h,076h,066h,066h,07eh,0bch,080h
	DB	00h,00h,00h,00h,00h,01h,03dh,07eh,066h,06eh,07eh,076h,066h,07eh,0bch,080h
	DB	00h,00h,00h,00h,00h,00h,07eh,0ffh,0dbh,0dbh,0dfh,0d8h,0d8h,0ffh,07fh,00h
	DB	00h,00h,00h,07eh,0feh,0d8h,0d8h,0deh,0deh,0d8h,0d8h,0d8h,0d8h,0feh,07eh,00h
	DB	00h,060h,030h,018h,00h,03ch,07eh,066h,066h,07eh,07eh,066h,066h,066h,066h,00h
	DB	00h,032h,07ah,04ch,00h,03ch,07eh,066h,066h,07eh,07eh,066h,066h,066h,066h,00h
	DB	00h,032h,07ah,04ch,00h,03ch,07eh,066h,066h,066h,066h,066h,066h,07eh,03ch,00h
	DB	00h,00h,066h,066h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,06h,0ch,018h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,010h,07ch,010h,010h,010h,010h,010h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,07ah,0cah,0cah,0cah,0cah,0cah,07ah,0ah,0ah,0ah,0ah,0ah,00h
	DB	00h,00h,00h,07ch,0c6h,082h,0bah,0a2h,0a2h,0a2h,0bah,082h,0c6h,07ch,00h,00h
	DB	00h,00h,00h,07ch,0c6h,082h,0bah,0aah,0b2h,0bah,0aah,082h,0c6h,07ch,00h,00h
	DB	00h,0f1h,05bh,05fh,055h,051h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,066h,066h,00h,0e6h,066h,066h,066h,066h,066h,066h,066h,0f6h,0eh,03ch
	DB	00h,0f6h,0f6h,066h,066h,066h,066h,066h,066h,066h,066h,066h,0f6h,0f6h,0eh,07ch
	DB	00h,00h,00h,066h,076h,076h,03eh,03ch,03ch,06eh,06eh,066h,062h,00h,00h,00h
	DB	00h,00h,00h,07ch,07ch,0ch,0ch,0ch,0ch,0ch,0ch,07eh,07eh,00h,00h,00h
	DB	00h,00h,00h,01eh,01eh,06h,06h,0eh,01eh,036h,036h,036h,036h,00h,00h,00h
	DB	00h,00h,00h,07eh,07eh,0ch,0ch,0ch,0ch,0ch,0ch,0ch,0ch,00h,00h,00h
	DB	00h,00h,00h,07eh,07eh,06h,066h,066h,066h,066h,066h,066h,066h,00h,00h,00h
	DB	00h,00h,00h,038h,038h,018h,018h,018h,018h,018h,018h,018h,018h,00h,00h,00h
	DB	00h,00h,00h,01eh,01eh,04h,0ch,0ch,0ch,0ch,0ch,0ch,04h,00h,00h,00h
	DB	00h,00h,00h,07eh,07eh,066h,066h,066h,066h,066h,066h,066h,066h,00h,00h,00h
	DB	00h,00h,00h,06eh,06eh,066h,066h,066h,066h,066h,066h,07eh,07eh,00h,00h,00h
	DB	00h,00h,00h,03ch,03ch,0ch,0ch,00h,00h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,03ch,03eh,06h,06h,06h,06h,06h,06h,03eh,03ch,00h,00h,00h
	DB	00h,060h,060h,07eh,07eh,06h,06h,06h,0eh,01ch,030h,030h,030h,00h,00h,00h
	DB	00h,00h,00h,06ch,06eh,03eh,036h,066h,066h,066h,06eh,06eh,06eh,00h,00h,00h
	DB	00h,00h,00h,01ch,01ch,0ch,0ch,0ch,0ch,0ch,0ch,07ch,07ch,00h,00h,00h
	DB	00h,00h,00h,0feh,0feh,0feh,0c6h,0c6h,0c6h,0c6h,0c6h,0feh,07ch,00h,00h,00h
	DB	00h,00h,00h,036h,036h,036h,036h,036h,036h,036h,036h,07eh,07eh,00h,00h,00h
	DB	00h,00h,00h,07eh,07eh,066h,066h,076h,076h,06h,07eh,07eh,07eh,00h,00h,00h
	DB	00h,00h,00h,06eh,06eh,066h,036h,03eh,018h,01ch,0eh,07eh,07eh,00h,00h,00h
	DB	00h,00h,00h,03eh,03eh,03eh,06h,06h,036h,036h,036h,036h,036h,030h,030h,030h
	DB	00h,00h,00h,07ch,07eh,06h,06h,06h,06h,06h,06h,06h,06h,00h,00h,00h
	DB	00h,00h,00h,0d6h,0d6h,0d6h,0d6h,0f6h,0f6h,0c6h,0c6h,0feh,0feh,00h,00h,00h
	DB	00h,00h,00h,07eh,07eh,066h,066h,066h,066h,066h,066h,0e6h,0e6h,00h,00h,00h
	DB	00h,00h,00h,038h,038h,018h,018h,018h,018h,018h,018h,018h,018h,018h,018h,00h
	DB	00h,00h,00h,07eh,07eh,06h,06h,06h,06h,06h,06h,06h,06h,06h,06h,00h
	DB	00h,00h,00h,07eh,03eh,036h,036h,036h,036h,036h,036h,03eh,03eh,00h,00h,00h
	DB	00h,00h,00h,07eh,07eh,066h,066h,076h,076h,06h,06h,06h,06h,06h,06h,00h
	DB	00h,00h,00h,06eh,06eh,066h,076h,03eh,0eh,06h,06h,06h,06h,06h,06h,00h
	DB	00h,00h,00h,01ch,036h,032h,018h,03ch,066h,066h,03ch,018h,04ch,06ch,038h,00h
	DB	00h,00h,00h,00h,00h,00h,010h,010h,038h,038h,06ch,06ch,0c6h,0c6h,00h,00h
	DB	00h,00h,00h,00h,00h,066h,0f7h,099h,099h,0efh,066h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,00h,062h,0f6h,0dch,088h,088h,0dch,0f6h,062h,00h,00h,00h
	DB	00h,00h,00h,018h,03ch,066h,066h,066h,07ch,066h,066h,066h,07ch,06ch,060h,040h
	DB	00h,00h,00h,0feh,07eh,062h,060h,060h,060h,060h,060h,060h,060h,060h,0f0h,00h
	DB	00h,00h,00h,00h,00h,02h,07eh,0fch,0a8h,028h,028h,028h,028h,00h,00h,00h
	DB	00h,00h,00h,0feh,0feh,060h,030h,018h,0ch,0ch,018h,030h,060h,0feh,0feh,00h
	DB	00h,00h,00h,00h,00h,01eh,038h,06ch,0c6h,0c6h,0c6h,0c6h,06ch,038h,00h,00h
	DB	00h,00h,00h,00h,00h,00h,066h,066h,066h,066h,066h,066h,07fh,05dh,0c0h,080h
	DB	00h,00h,00h,00h,02h,07eh,0fch,090h,030h,030h,030h,030h,020h,00h,00h,00h
	DB	00h,00h,07ch,010h,010h,07ch,0c6h,082h,082h,082h,082h,0c6h,07ch,010h,010h,07ch
	DB	00h,00h,00h,03ch,07eh,066h,042h,042h,07eh,042h,042h,066h,07eh,03ch,00h,00h
	DB	00h,00h,00h,038h,06ch,0c6h,0c6h,0c6h,0c6h,06ch,028h,0aah,0eeh,06ch,00h,00h
	DB	00h,00h,01eh,03eh,020h,030h,018h,03ch,066h,042h,042h,066h,07eh,03ch,00h,00h
	DB	00h,00h,0eh,01eh,010h,07ch,0d6h,092h,092h,092h,092h,0d6h,07ch,010h,0f0h,0e0h
	DB	00h,00h,00h,010h,010h,07ch,0d6h,092h,092h,092h,092h,0d6h,07ch,010h,010h,00h
	DB	00h,00h,00h,03eh,07eh,0e0h,0c0h,0c0h,0fch,0fch,0c0h,0c0h,0e0h,07eh,03eh,00h
	DB	00h,00h,00h,07ch,0feh,0c6h,0c6h,0c6h,0c6h,0c6h,0c6h,0c6h,0c6h,0c6h,0c6h,00h
	DB	00h,00h,00h,00h,00h,07eh,07eh,00h,07eh,07eh,00h,07eh,07eh,00h,00h,00h
	DB	00h,00h,00h,00h,018h,018h,018h,07eh,07eh,018h,018h,018h,00h,07eh,07eh,00h
	DB	00h,00h,00h,060h,070h,038h,01ch,0eh,01ch,038h,070h,060h,00h,07eh,07eh,00h
	DB	00h,00h,00h,06h,0eh,01ch,038h,070h,038h,01ch,0eh,06h,00h,07eh,07eh,00h
	DB	00h,00h,00h,0eh,019h,01bh,01bh,018h,018h,018h,018h,018h,018h,018h,018h,018h
	DB	00h,018h,018h,018h,018h,018h,018h,018h,018h,018h,018h,0d8h,0d8h,098h,070h,00h
	DB	00h,00h,00h,00h,018h,018h,00h,07eh,07eh,00h,018h,018h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,02h,03eh,07ch,040h,02h,03eh,07ch,040h,00h,00h,00h,00h
	DB	00h,00h,038h,06ch,044h,06ch,038h,00h,00h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,038h,07ch,07ch,07ch,038h,00h,00h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,018h,03ch,03ch,018h,00h,00h
	DB	00h,00h,00h,00h,00h,00h,01fh,010h,0d0h,0d0h,050h,070h,020h,00h,00h,00h
	DB	00h,00h,00h,030h,078h,048h,048h,048h,048h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,030h,048h,018h,030h,060h,078h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,00h,078h,018h,030h,018h,048h,030h,00h,00h,00h,00h,00h,00h,00h
	DB	00h,00h,0feh,0feh,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h


FNT_DATA	ENDS

END


