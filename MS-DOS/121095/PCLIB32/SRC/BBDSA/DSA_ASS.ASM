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

	ASSUME NOTHING

; extern Functions
	PUBLIC	C DSA_ASS_ActivatePal_ 				; Palette aktivieren
	PUBLIC	C DSA_ASS_CopyMainOPMToScreen_ 	; gesamten Vesascreen kopieren
	PUBLIC	C DSA_ASS_CopyMainOPMToScreenONE_ ; gesamten Screen ohne Segmente kopieren
	PUBLIC	C DSA_ASS_CopyMainOPMToScreenCHN_ ; gesamten Chain4 Screen kopieren
	PUBLIC	C DSA_ASS_CopyOPMToScreen_			; Teil in Vesascreen kopieren
	PUBLIC	C DSA_ASS_CopyOPMToScreenONE_		; Teil in Screen one Segmente kopieren
	PUBLIC	C DSA_ASS_CopyOPMToScreenCHN_ 	; Teil in Chain4 Screen kopieren
	PUBLIC	C DSA_ASS_CopyScreenToOPM_			; Teil aus Vesascreen kopieren
	PUBLIC	C DSA_ASS_CopyScreenToOPMONE_		; Teil aus Vesascreen kopieren
	PUBLIC	C DSA_ASS_CopyScreenToOPMCHN_		; Teil aus Vesascreen kopieren
	PUBLIC	C DSA_ASS_ActivateCHAINFOUR_ 		; Register beschreiben die Chain 4 aktivieren
	PUBLIC	C DSA_ASS_SetVideoramCHN_ 			; auf VBL warten und Addresse Bildschirm setzen nur CHAIN4
	PUBLIC	C DSA_ASS_WaitVBL_ 	 				; auf VBL warten
	PUBLIC	C DSA_ASS_WaitEndVBL_ 				; auf Ende VBL warten
	PUBLIC	C DSA_ASS_WaitTOF_ 					; auf oberen Bildschirmrand warten
	PUBLIC	C DSA_ASS_WaitBOF_ 					; auf unteren Bildschirmrand warten
	PUBLIC	C DSA_ASS_SaveVesaPage_ 			; VesaSeite sichern
	PUBLIC	C DSA_ASS_RestoreVesaPage_ 		; Vesaseite restaurieren
	PUBLIC	C DSA_ASS_SaveCHAIN4Page_ 			; CHAIN4Seite sichern
	PUBLIC	C DSA_ASS_RestoreCHAIN4Page_ 		; CHAIN4seite restaurieren
	PUBLIC	C DSA_ASS_Flash_ 						; Farbe 0 Ñndern lassen

; externe Variablen
	extrn	_lastpage:WORD ; letze eingeschaltete Page
	extrn	_savepage:WORD ; gesicherte Page
	extrn _twowindows:WORD ; 2 Fenster notwendig
	extrn _chain4lastwriteselect:BYTE ; letzter eingeschalteter Schreibmodus CHAIN4
	extrn _chain4lastreadselect:BYTE ; letzter eingeschalteter Schreibmodus CHAIN4
	extrn _chain4savewriteselect:BYTE ; letzter eingeschalteter Schreibmodus CHAIN4
	extrn _chain4savereadselect:BYTE ; letzter eingeschalteter Schreibmodus CHAIN4
	extrn _chain4writeselectok:BYTE ; wiederherstellen notwendig ?
	extrn _chain4readselectok:BYTE ; wiederherstellen notwendig ?
	extrn _inMouseInterrupt:WORD ; Momentan von einer MausinterruptRoutine aufgerufen ?
	extrn	_screensize:DWORD ; ByteGrîsse des eingeschalteten Modus
	extrn	_screenwidth:DWORD ; Breite des eingeschalteten Modus
	extrn	_screenheight:DWORD ; Hîhe des eingeschalteten Modus
	extrn	_windowsize:DWORD ; Grîsse eines eingeblendeten Fensters
	extrn	_windowstep:DWORD ; Wert der dazu addiert wird um an nÑchstes Fenster zu kommen
	extrn	_intmouse:WORD ; Wert der dazu addiert wird um an nÑchstes Fenster zu kommen

; Include Files
	INCLUDE ..\..\LIB\ASMINC\BBMACRO.INC
	INCLUDE ..\..\LIB\ASMINC\BBHARD.INC

; interne Macros

; VesaFenster umschalten
; neue PageNr muss in dx stehen
; benutzt eax/ebx/edx
SELECTPAGE	MACRO
	LOCAL SAMEPAGE,NOTWOWINDOWS
	cmp 	DX,_LASTPAGE
	je 	SAMEPAGE
	mov 	_INTMOUSE,-1  ; verhindern dass innerhalb eines Interrupts ein Mausinterrupt aufgerufen wird ein
	mov 	_LASTPAGE,DX
	mov 	AX,4F05H  ; Funktion Vesa Set Window
	xor 	BX,BX  ; bh = Funktion Mem / bl = Set Window 0
	int 	10H
	test	_twowindows,-1
	je		NOTWOWINDOWS
	mov	DX,_LASTPAGE
	mov 	AX,4F05H
	mov	BX,1 ; bh = Funktion Mem / bl = Set Window 1
	int 	10H
NOTWOWINDOWS:
	mov 	_INTMOUSE,0  ; verhindern dass innerhalb eines Interrupts ein Mausinterrupt aufgerufen wird aus
SAMEPAGE:
ENDM

DSA	SEGMENT PARA USE32 'CODE'	;

; ****************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_ActivatePal PALETTE:DWORD
; * FUNCTION  : initialisiert Palette
; *							ESI = Zeiger auf Palette im Format
; *							ECX = Anzahl zu kopierende Farben
; *							Red1.B,Green1.B,Blue1.B,Alpha1.B,Red2.B,Green2.B,...
; *
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : RR, BlueByte
; * FIRST     : 26.04.94 17:22
; * LAST      :24.07.94 20:57:18
; * INPUTS    : -
; * RESULT    : 0 = ok / -1 = Error
; * BUGS      : -
; * NOTES     : First 4 Arguments will be in EAX-EDX other arguments 5- are pushed on stack
; * SEE ALSO  : -
; * VERSION   : 1.0
; * #FUNCTION HEADER END#
; ****************************************************************************

DSA_ASS_ActivatePal_ PROC C
	sub 	AX,AX  ; clear AX
	mov 	DX,03C8H  ; DX with ColorPort
	safewait  	 ; like BIOS, Flush Cache, wait a little
	cli 	 		 ;
	out 	DX,AL  ; Start with ColorRegister 0
PS_LOOP:   		 ; --- COLOR ---
	lodsb  		 ; load red Colorpart
	sub 	AH,AH  ; clear Highbyte
	shr 	AX,2  ; VGA-Color-Range 0-63

	safewait  	 ; like BIOS, Flush Cache, wait a little
	mov 	DX,03C9H  ; DX with ColorPort
	out 	DX,AL  ; Set Color -> RED

	lodsb  		 ; load green Colorpart
	sub 	AH,AH  ; clear Highbyte
	shr 	AX,2  ; VGA-Color-Range 0-63

	safewait  	 ; like BIOS, Flush Cache, wait a little
	mov 	DX,03C9H  ; DX with ColorPort
	out 	DX,AL  ; Set Color -> RED

	lodsb  		 ; load blue Colorpart
	sub 	AH,AH  ; clear Highbyte
	shr 	AX,2  ; VGA-Color-Range 0-63

	safewait  	 ; like BIOS, Flush Cache, wait a little
	mov 	DX,03C9H  ; DX with ColorPort
	out 	DX,AL  ; Set Color -> RED

	lodsb  		 ; load alpha

	loop PS_LOOP  ; next Color
	sti
	ret

DSA_ASS_ActivatePal_ ENDP


; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_CopyMainOPMToScreen
; * FUNCTION  : Copy base OPM of screen (GUI: window) onto physical display.
; *             Function checks if the OPMSTAT_CHANGED bit is set in the OPM status flag set.
; *             If the copyflag is set, the OPMSTAT_CHANGED flag is not checked.
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 20:57:28
; * INPUTS    : source: Zeiger auf zu kopierende Pixelmap
; *           :   dest: Zeiger auf ZielVideoram
; *
; * RESULT    : None.
; * BUGS      :
; * NOTES     : benîtigt Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_CopyMainOPMToScreen_ PROC C source:DWORD,dest:DWORD
	LOCAL lscreensize:DWORD

	mov 	EAX,_SCREENSIZE  ; lokale Variable fÅr screensize
	sub 	EAX,_WINDOWSIZE
	mov 	LSCREENSIZE,EAX  ; - 1 window fÅr Schleife

	xor 	DX,DX
	selectpage   ; Page 0 selectieren wenn noch nicht

	mov 	ESI,SOURCE  ; Quelladdresse setzen
	jmp 	INCOPYSCRLOOP
COPYSCRLOOP:
	mov 	AX,4F05H  ; Funktion Vesa Set Window
	xor 	BX,BX  ; bh = Funktion Mem / bl = Set Window
	mov 	DX,_LASTPAGE
	add 	DX,WORD PTR [_WINDOWSTEP]
	mov 	_LASTPAGE,DX
	int 	10H
	test	_twowindows,-1
	je		NOTWOWINDOWS2
	mov	DX,_LASTPAGE
	mov 	AX,4F05H
	mov	BX,1 ; bh = Funktion Mem / bl = Set Window 1
	int 	10H
NOTWOWINDOWS2:
INCOPYSCRLOOP:

	mov 	EDI,DEST
	mov 	ECX,_WINDOWSIZE
	shr 	ECX,2  ; / 4 teilen da langwortweise kopiert wird
	rep 	MOVSD  ; kopiere komplettes Fenster

	subme LSCREENSIZE,_WINDOWSIZE
	jns 	COPYSCRLOOP

	mov 	AX,4F05H  ; Funktion Vesa Set Window
	xor 	BX,BX  ; bh = Funktion Mem / bl = Set Window
	mov 	DX,_LASTPAGE
	add 	DX,WORD PTR [_WINDOWSTEP]
	mov 	_LASTPAGE,DX
	int 	10H
	test	_twowindows,-1
	je		NOTWOWINDOWS3
	mov	DX,_LASTPAGE
	mov 	AX,4F05H
	mov	BX,1 ; bh = Funktion Mem / bl = Set Window 1
	int 	10H
NOTWOWINDOWS3:

	mov 	EDI,DEST
	mov 	ECX,_WINDOWSIZE
	add 	ECX,LSCREENSIZE
	shr 	ECX,2
	rep 	MOVSD  ; kopiere komplettes Fenster

	ret
DSA_ASS_CopyMainOPMToScreen_ ENDP


; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_CopyMainOPMToScreenII
; * FUNCTION  : Copy base OPM of screen (GUI: window) onto physical display.
; *             Function checks if the OPMSTAT_CHANGED bit is set in the OPM status flag set.
; *             If the copyflag is set, the OPMSTAT_CHANGED flag is not checked.
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 20:57:49
; * INPUTS    : source(ESI): Zeiger auf zu kopierende Pixelmap
; *           : dest(EDI): Zeiger auf zu videoram
; *
; * RESULT    : None.
; * BUGS      :
; * NOTES     : spezielle Routine nur fÅr 320x200 benîtigt keinen Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

;DSA_ASS_CopyMainOPMToScreenONE_ PROC C USES ESI EDI ECX
DSA_ASS_CopyMainOPMToScreenONE_ PROC C
	mov 	ECX,_SCREENSIZE  ; Fenstergrîsse
	shr 	ECX,2  ; / 4 teilen da langwortweise kopiert wird
	rep 	MOVSD  ; kopiere komplettes Fenster
	ret
DSA_ASS_CopyMainOPMToScreenONE_ ENDP


; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_CopyMainOPMToScreenCHAINFOUR
; * FUNCTION  : Copy base OPM of screen (GUI: window) onto physical display.
; *             Function checks if the OPMSTAT_CHANGED bit is set in the OPM status flag set.
; *             If the copyflag is set, the OPMSTAT_CHANGED flag is not checked.
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 20:59:51
; * INPUTS    : source(EBX): Zeiger auf zu kopierende Pixelmap
; *           : dest(EDI): Zeiger auf zu kopierende Pixelmap
; *
; * RESULT    : None.
; * BUGS      :
; * NOTES     : spezielle Routine nur fÅr 360x240 benîtigt keinen Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_CopyMainOPMToScreenCHN_ PROC C source:DWORD,dest:DWORD
	LOCAL	plane:WORD

	cmp 	_SCREENWIDTH,320  ; 320 Breit dann 320x200 Screen kopieren
	je 	COPY320X200CHAIN4

; 360x240 Chain4 Bildschirm kopieren
	mov 	PLANE,2

; Sollte die Routine durch einen Interrupt unterbrochen werden wird jetzt die Page gerettet
	mov 	_CHAIN4WRITESELECTOK,-1

; Plane Schleife
	mov 	CX,4
COPYPLANES:
	push CX

; bei erstem Durchlauf zusÑtzlich TS_IND register setzen
	cmp 	CX,4
	jne 	SECONDPLANELOOP
	mov 	AX,TS_MM+0100H
	mov 	DX,TS_IND
	mov 	_CHAIN4LASTWRITESELECT,1
	out 	DX,AX  ; Planes einstellen auf AH = -1
	inc 	DX 	 ; Select Dat_Register
	jmp 	ENDSECONDPLANELOOP
SECONDPLANELOOP:
	mov 	AX,PLANE
	mov 	_CHAIN4LASTWRITESELECT,AL
	out 	DX,AL  ; Planes einstellen auf AH = -1
	shl 	PLANE,1
ENDSECONDPLANELOOP:

	mov 	EDI,DEST
	mov 	ESI,SOURCE
	inc 	SOURCE

LINES  	 		= 2 ; 2 Zeilen auf einmal kopieren

SOURCECH   		= 0
DESTCH   		= 0

; jeweils 2 Zeilen in einem Durchlauf kopieren
	mov 	ECX,240/LINES
COPYLINES:

					REPEAT (360*LINES)/16
	mov 	AH,12+SOURCECH[ESI]
	mov 	AL,8+SOURCECH[ESI]
	shl 	EAX,16
	mov 	AH,4+SOURCECH[ESI]
	mov 	AL,SOURCECH[ESI]
	mov 	DESTCH[EDI],EAX
SOURCECH   		= SOURCECH + 16
DESTCH   		= DESTCH + 4
					ENDM
	add 	ESI,(360*LINES)
	add 	EDI,(360*LINES)/4

	dec 	CX
	jg 	COPYLINES

	pop 	CX
	dec 	CX
	jg 	COPYPLANES

	mov 	_CHAIN4WRITESELECTOK,0

	ret

; 320x200 Chain4 Bildschirm kopieren
COPY320X200CHAIN4:

	mov 	PLANE,2

; Sollte die Routine durch einen Interrupt unterbrochen werden wird jetzt die Page gerettet
	mov 	_CHAIN4WRITESELECTOK,-1

; Plane Schleife
	mov 	CX,4
COPYPLANES320:
	push CX

; bei erstem Durchlauf zusÑtzlich TS_IND register setzen
	cmp 	CX,4
	jne 	SECONDPLANELOOP320
	mov 	AX,TS_MM+0100H
	mov 	DX,TS_IND
	mov 	_CHAIN4LASTWRITESELECT,1
	out 	DX,AX  ; Planes einstellen auf AH = -1
	inc 	DX 	 ; Select Dat_Register
	jmp 	ENDSECONDPLANELOOP320
SECONDPLANELOOP320:
	mov 	AX,PLANE
	mov 	_CHAIN4LASTWRITESELECT,AL
	out 	DX,AL  ; Planes einstellen auf AH = -1
	shl 	PLANE,1
ENDSECONDPLANELOOP320:

	mov 	EDI,DEST
	mov 	ESI,SOURCE
	inc 	SOURCE

LINES  	 		= 2 ; 2 Zeilen auf einmal kopieren

SOURCECH   		= 0
DESTCH   		= 0

; jeweils 2 Zeilen in einem Durchlauf kopieren
	mov 	ECX,200/LINES
COPYLINES320:

					REPEAT (320*LINES)/16
	mov 	AH,12+SOURCECH[ESI]
	mov 	AL,8+SOURCECH[ESI]
	shl 	EAX,16
	mov 	AH,4+SOURCECH[ESI]
	mov 	AL,SOURCECH[ESI]
	mov 	DESTCH[EDI],EAX
SOURCECH   		= SOURCECH + 16
DESTCH   		= DESTCH + 4
					ENDM
	add 	ESI,(320*LINES)
	add 	EDI,(320*LINES)/4

	dec 	CX
	jg 	COPYLINES320

	pop 	CX
	dec 	CX
	jg 	COPYPLANES320

	mov 	_CHAIN4WRITESELECTOK,0

	ret
DSA_ASS_CopyMainOPMToScreenCHN_ ENDP


; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_CopyOPMToScreen
; * FUNCTION  : Copy OPM of screen (GUI: window) onto physical display.
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:22:34
; * INPUTS    : source: Zeiger auf zu kopierende Pixelmap
; *							nextypos: Anzahl Bytes bis zur nÑchsten Zeile des OPMS
; *															(opm kann einzelnen im ram liegen (Breite des Opms)
; *															 oder im Hauptopm (Breite des Screens) )
; *							x: Position x im Zielbildschirm
; *							y: Position y im Zielbildschirm
; *							widthh: zu kopierende Breite
; *							height: zu kopierende Hoehe
; *
; * RESULT    : None.
; * BUGS      :
; * NOTES     : benîtigt Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_CopyOPMToScreen_ PROC C source:DWORD,destin:DWORD,nextypos:DWORD,x:DWORD,y:DWORD,widthh:DWORD,height:DWORD
	LOCAL modulo:DWORD,destmodulo:DWORD,DDWIDTH:DWORD,DBWIDTH:DWORD

; ds nach es kopieren wegen rep movs
	push ES
	mov 	AX,DS
	mov 	ES,AX

	mov 	EAX,Y
	mul 	_SCREENWIDTH  ; Ziel y koordinaate ausrechnen
	add 	EAX,X  ; x koordinaate dazu

	xor 	EDX,EDX
	div 	_WINDOWSIZE
	mov 	EDI,EDX  ; Rest = StartAddresse im Segment
	mul 	_WINDOWSTEP  ; nÑchstes Segment erreeicht ?

	push	EDI
	mov 	DX,AX
	selectpage   ; StartPage einschalten
	pop	EDI

	mov 	EBX,_WINDOWSIZE
	sub 	EBX,EDI
	add	EDI,DESTIN

	mov 	EAX,NEXTYPOS
	sub 	EAX,WIDTHH
	mov 	MODULO,EAX

	mov 	EAX,_SCREENWIDTH
	sub 	EAX,WIDTHH
	mov 	DESTMODULO,EAX

	mov 	EAX,WIDTHH
	shr 	EAX,2
	mov 	DDWIDTH,EAX

	mov 	EAX,WIDTHH
	and	EAX,3
	mov 	DBWIDTH,EAX

	mov 	ESI,SOURCE  ; Quelladdresse

	mov 	EDX,HEIGHT
COPYSCRLOOP2:

	sub	EBX,_SCREENWIDTH  ; Wurde Segmentgrenze Åberschritten
	js 	GOTONEXTSEGMENT

	mov 	ECX,DDWIDTH  ; zu kopierende Breite fÅr opm
	rep 	MOVSD  ; restliche Zeile mit Langworten kopieren
	mov 	ECX,DBWIDTH  ; zu kopierende Breite fÅr opm
	rep 	MOVSB  ; Restbytes kopieren

	add 	ESI,MODULO  ; nÑchste Zeile Quellbereich
	add	EDI,DESTMODULO  ; nÑchste Zeile Quellbereich
	dec	edx
	jg		COPYSCRLOOP2
	jmp 	ENDCOPYOPM
GOTONEXTSEGMENT:
	mov 	ECX,EBX
	add 	ECX,_SCREENWIDTH  ; rest im Segment ausrechnen

	cmp 	ECX,WIDTHH  ; Ist der Umbruch in der Mitte der Zeile
	jg 	NOTINLINE  ; Rest im Segment > Breite Screen ist ausserhalb des Fensters
; Segmentgrenze ist innerhalb einer Zeile

	mov 	EAX,ECX
	shr 	ECX,2
	rep 	MOVSD  ; restliche Zeile mit Langworten kopieren
	mov 	ECX,EAX
	and 	ECX,3
	rep 	MOVSB  ; Restliche Bytes in diesem Segment vollends kopieren

	push	edx
	push	ebx
	mov 	DX,_LASTPAGE
	add 	DX,WORD PTR [_WINDOWSTEP]  ; auf nÑchstes Segment addieren
	selectpage
	pop	ebx
	pop	edx

	mov 	EDI,DESTIN  ; Zielbereich auf videoram setzen

	mov 	EAX,EBX
	add 	EAX,_SCREENWIDTH
	mov 	ECX,WIDTHH
	sub 	ECX,EAX
	mov 	EAX,ECX
	shr 	ECX,2
	rep 	MOVSD  ; restliche Zeile mit Langworten kopieren
	mov 	ECX,EAX
	and 	ECX,3
	rep 	MOVSB  ; Restliche Bytes in diesem Segment vollends kopieren

	add	EBX,_WINDOWSIZE  ; (-)restsegment+windowsize = bytes in neuem Segment

	add 	ESI,MODULO  ; nÑchste Zeile Quellbereich
	add	EDI,DESTMODULO  ; nÑchste Zeile Quellbereich
	dec	edx
	jg		COPYSCRLOOP2
	jmp 	ENDCOPYOPM
NOTINLINE:

; Segmentgrenze ist ausserhalb einer Zeile

	mov 	ECX,DDWIDTH  ; zu kopierende Breite fÅr opm
	rep 	MOVSD  ; restliche Zeile mit Langworten kopieren
	mov 	ECX,DBWIDTH  ; zu kopierende Breite fÅr opm
	rep 	MOVSB  ; Restbytes kopieren

	push	edx
	push	ebx
	mov 	DX,_LASTPAGE
	add 	DX,WORD PTR [_WINDOWSTEP]  ; auf nÑchstes Segment addieren
	selectpage   ; nÑchste Page activieren
	pop	ebx
	pop	edx

	mov 	EDI,DESTIN
	sub 	EDI,EBX  ; Ziel auf Anfang nÑchster Zeile

	add	EBX,_WINDOWSIZE  ; (-)restsegment+windowsize = bytes in neuem Segment

	add 	ESI,MODULO  ; nÑchste Zeile Quellbereich
	dec	edx
	jg		COPYSCRLOOP2
ENDCOPYOPM:

	pop 	ES

	ret
DSA_ASS_CopyOPMToScreen_ ENDP


; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_CopyOPMToScreenONE
; * FUNCTION  : Copy OPM of screen (GUI: window) onto physical ONESEG display.
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:08:36
; * INPUTS    : source: Zeiger auf zu kopierende Pixelmap
; *							nextypos: Anzahl Bytes bis zur nÑchsten Zeile des OPMS
;	*															(opm kann einzelnen im ram liegen (Breite des Opms)
; *															 oder im Hauptopm (Breite des Screens) )
; *							x: Position x im Zielbildschirm
; *							y: Position y im Zielbildschirm
; *							widthh: zu kopierende Breite
; *							height: zu kopierende Hoehe
; *
; * RESULT    : None.
; * BUGS      :
; * NOTES     : spezielle Routine nur fÅr 320x200 benîtigt keinen Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_CopyOPMToScreenONE_ PROC C nextypos:DWORD,x:DWORD,y:DWORD,widthh:DWORD,height:DWORD
	LOCAL dest:DWORD,modulosrc:DWORD,modulodst:DWORD

; ds nach es kopieren wegen rep movs
	push ES
	mov 	AX,DS
	mov 	ES,AX

	mov 	EAX,Y
	mul 	_SCREENWIDTH  ; Ziel y koordinaate ausrechnen
	add 	EAX,X  ; x koordinaate dazu

	add 	EDI,EAX

	mov 	EAX,NEXTYPOS
	sub 	EAX,WIDTHH
	mov 	MODULOSRC,EAX

	mov 	EAX,_SCREENWIDTH
	sub 	EAX,WIDTHH
	mov 	MODULODST,EAX

	mov 	EAX,WIDTHH
	shr 	EAX,2
	mov 	EBX,WIDTHH
	and 	EBX,3

	mov 	EDX,HEIGHT
COPYSCRLOOP2II:

	mov 	ECX,EAX  ; zu kopierende Breite fÅr opm
	rep 	MOVSD  ; restliche Zeile mit Langworten kopieren
	mov 	ECX,EBX  ; zu kopierende Breite fÅr opm
	rep 	MOVSB  ; Restbytes kopieren

	add 	ESI,MODULOSRC  ; nÑchste Zeile Quellbereich
	add 	EDI,MODULODST  ; nÑchste Zeile Quellbereich

	dec 	EDX
	jg 	COPYSCRLOOP2II

	pop 	ES

	ret
DSA_ASS_CopyOPMToScreenONE_ ENDP


; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_CopyOPMToScreenCHN
; * FUNCTION  : Copy OPM of screen (GUI: window) onto physical CHAIN4 display.
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:09:28
; * INPUTS    : source: Zeiger auf zu kopierende Pixelmap
; *							nextypos: Anzahl Bytes bis zur nÑchsten Zeile des OPMS
;	*															(opm kann einzelnen im ram liegen (Breite des Opms)
; *															 oder im Hauptopm (Breite des Screens) )
; *							x: Position x im Zielbildschirm
; *							y: Position y im Zielbildschirm
; *							widthh: zu kopierende Breite
; *							height: zu kopierende Hoehe
; *
; * RESULT    : None.
; * BUGS      :
; * NOTES     : spezielle Routine nur fÅr CHAIN4 benîtigt keinen Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_CopyOPMToScreenCHN_ PROC C source:DWORD,dest:DWORD,nextypos:DWORD,x:DWORD,y:DWORD,widthh:DWORD,height:DWORD
	LOCAL plane:WORD,firstplane:WORD,vidwidth:DWORD,
				leftmask:WORD,rightmask:WORD,lop:WORD,
				planeesi:DWORD,planeedi:DWORD,
				widthcopy:DWORD,lwidthcopy:DWORD,rwidthcopy:DWORD,
				srcmodulo:DWORD,dstmodulo:DWORD

; ds nach es kopieren wegen rep movs
	push ES
	mov 	AX,DS
	mov 	ES,AX

	mov 	EAX,_SCREENWIDTH
	shr 	EAX,2
	mov 	VIDWIDTH,EAX  ; Breite einer Chain4 Zeile

	mov 	EBX,X
	add 	EBX,3
	shr 	EBX,2  ; Start X Koordinaate Videoram
	mov 	EAX,Y
	mul 	VIDWIDTH  ; Ziel koordinaate ausrechnen
	add 	EAX,EBX
	add 	EAX,DEST
	mov 	PLANEEDI,EAX  ; Startaddresse linke Seite Videoram % 4

	mov 	EAX,X
	add 	EAX,3
	and 	EAX,-4
	mov 	EDX,X
	sub 	EAX,X
	add 	EAX,SOURCE
	mov 	PLANEESI,EAX  ; Startadresse linke Seite Quellbereich % 4

; Tabelle fÅr linken Rand erstellen

	mov 	EAX,X
	and 	AX,3
	jne 	LRANDV0
	mov 	[LEFTMASK],0000Y
	jmp 	LRANDV3
LRANDV0:
	cmp 	AX,1
	jne 	LRANDV1
	mov 	[LEFTMASK],1110Y
	jmp 	LRANDV3
LRANDV1:
	cmp 	AX,2
	jne 	LRANDV2
	mov 	[LEFTMASK],1100Y
	jmp 	LRANDV3
LRANDV2:
	mov 	[LEFTMASK],1000Y
LRANDV3:

; Tabelle fÅr rechten Rand erstellen

	mov 	EAX,X
	add 	EAX,WIDTHH
	and 	AX,3
	jne 	RRANDV0
	mov 	[RIGHTMASK],0000Y
	jmp 	RRANDV3
RRANDV0:
	cmp 	AX,1
	jne 	RRANDV1
	mov 	[RIGHTMASK],0001Y
	jmp 	RRANDV3
RRANDV1:
	cmp 	AX,2
	jne 	RRANDV2
	mov 	[RIGHTMASK],0011Y
	jmp 	RRANDV3
RRANDV2:
	mov 	[RIGHTMASK],0111Y
RRANDV3:

	cmp 	WIDTHH,16
	jl 	COPYSMALLCHN

	mov 	EBX,X
	add 	EBX,15
	and 	EBX,-16  ; Start des Langwortbereichs

	mov 	EAX,X
	add 	EAX,3
	and 	EAX,-4  ; Start des Bytebereichs

	mov 	EDX,EBX
	sub 	EDX,EAX
	shr 	EDX,2
	mov 	LWIDTHCOPY,EDX  ; Breite des zu kopierenden Bytebereichs links

	mov 	EDX,X
	add 	EDX,WIDTHH
	and 	EDX,-16
	mov 	ECX,EDX
	sub 	EDX,EBX
	shr 	EDX,4
	mov 	WIDTHCOPY,EDX  ; Breite des zu kopierenden Langwortbereichs

	mov 	EAX,X
	add 	EAX,WIDTHH
	and 	EAX,-4  ; Start des Bytebereichs
	sub 	EAX,ECX  ; Bytes auf der rechten Seite
	shr 	EAX,2
	mov 	RWIDTHCOPY,EAX  ; Breite des zu kopierenden Bytebereichs rechts

	mov 	EAX,LWIDTHCOPY  ; Breite des zu kopierenden Bytebereichs links
	add 	EAX,RWIDTHCOPY
	mov 	EDX,WIDTHCOPY
	shl 	EDX,2
	add 	EAX,EDX
	mov 	EDX,VIDWIDTH
	sub 	EDX,EAX
	mov 	DSTMODULO,EDX  ; Zielmodulo
	shl 	EAX,2
	mov 	EDX,NEXTYPOS
	sub 	EDX,EAX
	mov 	SRCMODULO,EDX  ; Quellmodulo

; KopierRoutine

	mov 	LOP,0
	mov 	PLANE,2

; Sollte die Routine durch einen Interrupt unterbrochen werden wird jetzt die Page gerettet
; nur erlaubt wenn diese Routine nicht selbst durch einen Interrupt aufgerufen wird
	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINT1
	mov 	_CHAIN4WRITESELECTOK,-1
FROMMOUSEINT1:

	mov 	CX,4
COPYPLANES16:
	push CX

	cmp 	CX,4
	jne 	SECONDPLANELOOP16
	mov 	AX,TS_MM+0100H
	mov 	DX,TS_IND
	mov 	_CHAIN4LASTWRITESELECT,1
	out 	DX,AX  ; Planes einstellen auf AH = -1
	jmp 	ENDSECONDPLANELOOP16
SECONDPLANELOOP16:
	mov 	AX,PLANE
	mov 	DX,TS_DAT
	mov 	_CHAIN4LASTWRITESELECT,AL
	out 	DX,AL  ; Planes einstellen auf AH = -1
	shl 	PLANE,1
ENDSECONDPLANELOOP16:

; kopieren

	mov 	ESI,PLANEESI
	mov 	EDI,PLANEEDI

	add 	PLANEESI,1  ; Quellbereich + 1

	mov 	EDX,HEIGHT
COPYHEIGHT16:

; Rand linke Seite
	mov 	AX,LOP
	bt 	[LEFTMASK],AX
	jnc 	NOLRAND
	mov 	AL,-4[ESI]
	mov 	-1[EDI],AL
NOLRAND:

; linke Seite
	mov 	ECX,LWIDTHCOPY
	or 	ECX,ECX
	je 	NOCOPYLONGL4  ; wenn 0 nichts kopieren
COPYLONGL4:
	movsb
	add 	ESI,3
	loop COPYLONGL4
NOCOPYLONGL4:

; Mittelteil
	mov 	ECX,WIDTHCOPY
	or 	ECX,ECX
	je 	NOCOPYLONG16  ; wenn 0 nichts kopieren
COPYLONG16:
	mov 	AH,12[ESI]
	mov 	AL,8[ESI]
	shl 	EAX,16
	mov 	AH,4[ESI]
	mov 	AL,[ESI]
	mov 	[EDI],EAX
	add 	ESI,16
	add 	EDI,4
	loop COPYLONG16
NOCOPYLONG16:

; rechte Seite
	mov 	ECX,RWIDTHCOPY
	or 	ECX,ECX
	je 	NOCOPYLONGR4  ; wenn 0 nichts kopieren
COPYLONGR4:
	movsb
	add 	ESI,3
	loop COPYLONGR4
NOCOPYLONGR4:

; Rand rechte Seite
	mov 	AX,LOP
	bt 	[RIGHTMASK],AX
	jnc 	NORRAND
	mov 	AL,[ESI]
	mov 	[EDI],AL
NORRAND:

	add 	ESI,SRCMODULO  ; nÑchste Zeile Source
	add 	EDI,DSTMODULO  ; nÑchste Zeile dest

	dec 	EDX
	jg 	COPYHEIGHT16

	inc 	LOP

	pop 	CX
	dec 	CX
	jg 	COPYPLANES16

; Page wird nicht gerettet
	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINT1B
	mov 	_CHAIN4WRITESELECTOK,0
FROMMOUSEINT1B:

	pop 	ES

	ret
COPYSMALLCHN:
; Spezialroutine fÅr Bitmaps mit Breite < 16 Pixel

	mov 	EBX,X
	add 	EBX,3
	and 	EBX,-4  ; Start des bereichs

	mov 	EDX,X
	add 	EDX,WIDTHH
	and 	EDX,-4
	sub 	EDX,EBX
	js 	ONE4BYTE
	shr 	EDX,2
	mov 	WIDTHCOPY,EDX  ; Breite des zu kopierenden bereichs

	mov 	EAX,WIDTHCOPY
	mov 	EDX,VIDWIDTH
	sub 	EDX,EAX
	mov 	DSTMODULO,EDX  ; Zielmodulo
	shl 	EAX,2
	mov 	EDX,NEXTYPOS
	sub 	EDX,EAX
	mov 	SRCMODULO,EDX  ; Quellmodulo

; kopieren

	mov 	LOP,0
	mov 	PLANE,2

; Sollte die Routine durch einen Interrupt unterbrochen werden wird jetzt die Page gerettet
; nur erlaubt wenn diese Routine nicht selbst durch einen Interrupt aufgerufen wird
	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINT2
	mov 	_CHAIN4WRITESELECTOK,-1
FROMMOUSEINT2:

	mov 	CX,4
COPYPLANES16SMALL:
	push CX

	cmp 	CX,4
	jne 	SECONDPLANELOOP16SMALL
	mov 	AX,TS_MM+0100H
	mov 	DX,TS_IND
	mov 	_CHAIN4LASTWRITESELECT,1
	out 	DX,AX  ; Planes einstellen auf AH = -1
	jmp 	ENDSECONDPLANELOOP16SMALL
SECONDPLANELOOP16SMALL:
	mov 	AX,PLANE
	mov 	DX,TS_DAT
	mov 	_CHAIN4LASTWRITESELECT,AL
	out 	DX,AL  ; Planes einstellen auf AH = -1
	shl 	PLANE,1
ENDSECONDPLANELOOP16SMALL:

	mov 	ESI,PLANEESI
	mov 	EDI,PLANEEDI

	add 	PLANEESI,1  ; Quellbereich + 1

	mov 	EDX,HEIGHT
COPYHEIGHT16SMALL:

; Rand linke Seite
	mov 	AX,LOP
	bt 	[LEFTMASK],AX
	jnc 	NOLRANDSMALL
	mov 	AL,-4[ESI]
	mov 	-1[EDI],AL
NOLRANDSMALL:

; Mittelteil
	mov 	ECX,WIDTHCOPY
	or 	ECX,ECX
	je 	NOCOPYBYTE16SMALL  ; wenn 0 nichts kopieren
COPYBYTE16SMALL:
	movsb
	add 	ESI,3
	loop COPYBYTE16SMALL
NOCOPYBYTE16SMALL:

; Rand rechte Seite
	mov 	AX,LOP
	bt 	[RIGHTMASK],AX
	jnc 	NORRANDSMALL
	mov 	AL,[ESI]
	mov 	[EDI],AL
NORRANDSMALL:

	add 	ESI,SRCMODULO  ; nÑchste Zeile Source
	add 	EDI,DSTMODULO  ; nÑchste Zeile dest

	dec 	EDX
	jg 	COPYHEIGHT16SMALL

	inc 	LOP

	pop 	CX
	dec 	CX
	jg 	COPYPLANES16SMALL

; Page wird nicht gerettet
	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINT2B
	mov 	_CHAIN4WRITESELECTOK,0
FROMMOUSEINT2B:

	pop 	ES

	ret
ONE4BYTE:
; Spezialroutine wenn Opm in ein 4 Pixel bereich kopiert werden muss

	mov 	AX,[LEFTMASK]
	and 	AX,[RIGHTMASK]  ; herausfinden welche Bits
	mov 	[LEFTMASK],AX

; kopieren

	mov 	LOP,0
	mov 	PLANE,2

	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINT3
	mov 	_CHAIN4WRITESELECTOK,-1
FROMMOUSEINT3:

	mov 	CX,4
COPYPLANES4SMALL:
	push CX

	cmp 	CX,4
	jne 	SECONDPLANELOOP4SMALL
	mov 	AX,TS_MM+0100H
	mov 	DX,TS_IND
	mov 	_CHAIN4LASTWRITESELECT,1
	out 	DX,AX  ; Planes einstellen auf AH = -1
	jmp 	ENDSECONDPLANELOOP4SMALL
SECONDPLANELOOP4SMALL:
	mov 	AX,PLANE
	mov 	DX,TS_DAT
	mov 	_CHAIN4LASTWRITESELECT,AL
	out 	DX,AL  ; Planes einstellen auf AH = -1
	shl 	PLANE,1
ENDSECONDPLANELOOP4SMALL:

	mov 	ESI,PLANEESI
	mov 	EDI,PLANEEDI

	add 	PLANEESI,1  ; Quellbereich + 1

	mov 	EDX,HEIGHT
COPYHEIGHT4SMALL:

; Rand beide Seiten
	mov 	AX,LOP
	bt 	[LEFTMASK],AX
	jnc 	NORANDSMALL
	mov 	AL,-4[ESI]
	mov 	-1[EDI],AL
NORANDSMALL:

	add 	ESI,NEXTYPOS  ; nÑchste Zeile Source
	add 	EDI,VIDWIDTH  ; nÑchste Zeile dest

	dec 	EDX
	jg 	COPYHEIGHT4SMALL

	inc 	LOP

	pop 	CX
	dec 	CX
	jg 	COPYPLANES4SMALL

; Page wird nicht gerettet
	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINT3B
	mov 	_CHAIN4WRITESELECTOK,0
FROMMOUSEINT3B:

	pop 	ES

	ret
DSA_ASS_CopyOPMToScreenCHN_ ENDP


; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_CopyScreenToOPM
; * FUNCTION  : Copy Part of Physikal Display to OPM
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:10:43
; * INPUTS    : dest: Zeiger auf Videoram
; *           : source: Zeiger auf zu kopierende Pixelmap
; *							nextypos: Anzahl Bytes bis zur nÑchsten Zeile des OPMS
;	*															(opm kann einzelnen im ram liegen (Breite des Opms)
; *															 oder im Hauptopm (Breite des Screens) )
; *							x: Position x im Zielbildschirm
; *							y: Position y im Zielbildschirm
; *			 				widthh: zu kopierende Breite
; *							height: zu kopierende Hoehe
; *
; * RESULT    : None.
; * BUGS      :
; * NOTES     : benîtigt Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_CopyScreenToOPM_ PROC C source:DWORD,destin:DWORD,nextypos:DWORD,x:DWORD,y:DWORD,widthh:DWORD,height:DWORD
	LOCAL modulo:DWORD,destmodulo:DWORD,DDWIDTH:DWORD,DBWIDTH:DWORD

; ds nach es kopieren wegen rep movs
	push ES
	mov 	AX,DS
	mov 	ES,AX

	mov 	EAX,Y
	mul 	_SCREENWIDTH  ; Ziel y koordinaate ausrechnen
	add 	EAX,X  ; x koordinaate dazu

	xor 	EDX,EDX
	div 	_WINDOWSIZE
	mov 	ESI,EDX  ; Rest = StartAddresse im Segment
	mul 	_WINDOWSTEP  ; nÑchstes Segment erreeicht ?

	push	esi
	mov 	DX,AX
	selectpage   ; StartPage einschalten
	pop	esi

	mov 	EAX,_WINDOWSIZE
	sub 	EAX,ESI
	mov 	EBX,EAX
	add	ESI,SOURCE

	mov 	EAX,NEXTYPOS
	sub 	EAX,WIDTHH
	mov 	MODULO,EAX

	mov 	EAX,_SCREENWIDTH
	sub 	EAX,WIDTHH
	mov 	DESTMODULO,EAX

	mov 	EAX,WIDTHH
	shr 	EAX,2
	mov 	DDWIDTH,EAX

	mov 	EAX,WIDTHH
	and	EAX,3
	mov 	DBWIDTH,EAX

	mov 	EDI,DESTIN  ; Quelladdresse

	mov 	EDX,HEIGHT
SCRCOPYSCRLOOP2:

	sub	EBX,_SCREENWIDTH  ; Wurde Segmentgrenze Åberschritten
	js 	SCRGOTONEXTSEGMENT

	mov 	ECX,DDWIDTH  ; zu kopierende Breite fÅr opm
	rep 	MOVSD  ; restliche Zeile mit Langworten kopieren
	mov 	ECX,DBWIDTH  ; zu kopierende Breite fÅr opm
	rep 	MOVSB  ; Restbytes kopieren

	add 	ESI,DESTMODULO  ; nÑchste Zeile Quellbereich
	add 	EDI,MODULO  ; nÑchste Zeile Quellbereich
	dec 	EDX
	jg 	SCRCOPYSCRLOOP2
	jmp 	SCRENDCOPYOPM
SCRGOTONEXTSEGMENT:
	mov 	ECX,EBX
	add 	ECX,_SCREENWIDTH  ; rest im Segment ausrechnen

	cmp 	ECX,WIDTHH  ; Ist der Umbruch in der Mitte der Zeile
	jg 	SCRNOTINLINE  ; Rest im Segment > Breite Screen ist ausserhalb des Fensters
; Segmentgrenze ist innerhalb einer Zeile

	mov 	EAX,ECX
	shr 	ECX,2
	rep 	MOVSD  ; restliche Zeile mit Langworten kopieren
	mov 	ECX,EAX
	and 	ECX,3
	rep 	MOVSB  ; Restliche Bytes in diesem Segment vollends kopieren

	push	eDX
	push	EBX
	mov 	DX,_LASTPAGE
	add 	DX,WORD PTR [_WINDOWSTEP]  ; auf nÑchstes Segment addieren
	selectpage
	pop	EBX
	pop 	eDX

	mov 	ESI,SOURCE  ; Zielbereich auf videoram setzen

	mov 	EAX,EBX
	add 	EAX,_SCREENWIDTH
	mov 	ECX,WIDTHH
	sub 	ECX,EAX
	mov 	EAX,ECX
	shr 	ECX,2
	rep 	MOVSD  ; restliche Zeile mit Langworten kopieren
	mov 	ECX,EAX
	and 	ECX,3
	rep 	MOVSB  ; Restliche Bytes in diesem Segment vollends kopieren

	add	EBX,_WINDOWSIZE  ; (-)restsegment+windowsize = bytes in neuem Segment

	add 	ESI,DESTMODULO  ; nÑchste Zeile Quellbereich
	add 	EDI,MODULO  ; nÑchste Zeile Quellbereich
	dec 	EDX
	jg 	SCRCOPYSCRLOOP2
	jmp 	SCRENDCOPYOPM
SCRNOTINLINE:

; Segmentgrenze ist ausserhalb einer Zeile

	mov 	ECX,DDWIDTH  ; zu kopierende Breite fÅr opm
	rep 	MOVSD  ; restliche Zeile mit Langworten kopieren
	mov 	ECX,DBWIDTH  ; zu kopierende Breite fÅr opm
	rep 	MOVSB  ; Restbytes kopieren

	push	eDX
	push	EBX
	mov 	DX,_LASTPAGE
	add 	DX,WORD PTR [_WINDOWSTEP]  ; auf nÑchstes Segment addieren
	selectpage   ; nÑchste Page activieren
	pop	EBX
	pop	eDX

	mov 	ESI,SOURCE
	sub 	ESI,EBX  ; Ziel auf Anfang nÑchster Zeile

	add	EBX,_WINDOWSIZE  ; (-)restsegment+windowsize = bytes in neuem Segment

	add 	EDI,MODULO  ; nÑchste Zeile Quellbereich
	dec 	EDX
	jg 	SCRCOPYSCRLOOP2
SCRENDCOPYOPM:

	pop 	ES

	ret
DSA_ASS_CopyScreenToOPM_ ENDP


; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_CopyScreenToOPMONE
; * FUNCTION  : Copy Part of Physikal Display to OPM
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:14:30
; * INPUTS    : dest(edi): Zeiger auf Videoram
; * INPUTS    : source(esi): Zeiger auf zu kopierende Pixelmap
; *							nextypos: Anzahl Bytes bis zur nÑchsten Zeile des OPMS
;	*															(opm kann einzelnen im ram liegen (Breite des Opms)
; *															 oder im Hauptopm (Breite des Screens) )
; *							x: Position x im Zielbildschirm
; *							y: Position y im Zielbildschirm
; *							widthh: zu kopierende Breite
; *							height: zu kopierende Hoehe
; *
; * RESULT    : None.
; * BUGS      :
; * NOTES     : spezielle Routine nur fÅr 320x200 benîtigt keinen Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_CopyScreenToOPMONE_ PROC C nextypos:DWORD,x:DWORD,y:DWORD,widthh:DWORD,height:DWORD
	LOCAL dest:DWORD,modulosrc:DWORD,modulodst:DWORD

; ds nach es kopieren wegen rep movs
	push ES
	mov 	AX,DS
	mov 	ES,AX

	mov 	EAX,Y
	mul 	_SCREENWIDTH  ; Ziel y koordinaate ausrechnen
	add 	EAX,X  ; x koordinaate dazu

	add 	ESI,EAX

	mov 	EAX,NEXTYPOS
	sub 	EAX,WIDTHH
	mov 	MODULOSRC,EAX

	mov 	EAX,_SCREENWIDTH
	sub 	EAX,WIDTHH
	mov 	MODULODST,EAX

	mov 	EAX,WIDTHH
	shr 	EAX,2
	mov 	EBX,WIDTHH
	and 	EBX,3

	mov 	EDX,HEIGHT
SCRCOPYSCRLOOP2II:

	mov 	ECX,EAX  ; zu kopierende Breite fÅr opm
	rep 	MOVSD  ; restliche Zeile mit Langworten kopieren
	mov 	ECX,EBX  ; zu kopierende Breite fÅr opm
	rep 	MOVSB  ; Restbytes kopieren

	add 	EDI,MODULOSRC  ; nÑchste Zeile Quellbereich
	add 	ESI,MODULODST  ; nÑchste Zeile Quellbereich

	dec 	EDX
	jg 	SCRCOPYSCRLOOP2II

	pop 	ES

	ret
DSA_ASS_CopyScreenToOPMONE_ ENDP


; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_CopyScreenToOPMCHN
; * FUNCTION  : Copy Part of Physikal Display to OPM
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:16:19
; * INPUTS    : dest: Zeiger auf Videoram
; *           : source: Zeiger auf zu kopierende Pixelmap
; *							nextypos: Anzahl Bytes bis zur nÑchsten Zeile des OPMS
;	*															(opm kann einzelnen im ram liegen (Breite des Opms)
; *															 oder im Hauptopm (Breite des Screens) )
; *							x: Position x im Zielbildschirm
; *							y: Position y im Zielbildschirm
; *							widthh: zu kopierende Breite
; *							height: zu kopierende Hoehe
; *
; * RESULT    : None.
; * BUGS      :
; * NOTES     : spezielle Routine nur fÅr CHAIN4 benîtigt keinen Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_CopyScreenToOPMCHN_ PROC C source:DWORD,dest:DWORD,nextypos:DWORD,x:DWORD,y:DWORD,widthh:DWORD,height:DWORD
	LOCAL plane:WORD,firstplane:WORD,vidwidth:DWORD,
				leftmask:WORD,rightmask:WORD,lop:WORD,
				planeesi:DWORD,planeedi:DWORD,
				widthcopy:DWORD,lwidthcopy:DWORD,rwidthcopy:DWORD,
				srcmodulo:DWORD,dstmodulo:DWORD

; ds nach es kopieren wegen rep movs
	push ES
	mov 	AX,DS
	mov 	ES,AX

	mov 	EAX,_SCREENWIDTH
	shr 	EAX,2
	mov 	VIDWIDTH,EAX  ; Breite einer Chain4 Zeile

	mov 	EBX,X
	add 	EBX,3
	shr 	EBX,2  ; Start X Koordinaate Videoram
	mov 	EAX,Y
	mul 	VIDWIDTH  ; Ziel koordinaate ausrechnen
	add 	EAX,EBX
	add 	EAX,SOURCE
	mov 	PLANEESI,EAX  ; Startaddresse linke Seite Videoram % 4

	mov 	EAX,X
	add 	EAX,3
	and 	EAX,-4
	mov 	EDX,X
	sub 	EAX,X
	add 	EAX,DEST
	mov 	PLANEEDI,EAX  ; Startadresse linke Seite Quellbereich % 4

; Tabelle fÅr linken Rand erstellen

	mov 	EAX,X
	and 	AX,3
	jne 	SCRLRANDV0
	mov 	[LEFTMASK],0000Y
	jmp 	SCRLRANDV3
SCRLRANDV0:
	cmp 	AX,1
	jne 	SCRLRANDV1
	mov 	[LEFTMASK],1110Y
	jmp 	SCRLRANDV3
SCRLRANDV1:
	cmp 	AX,2
	jne 	SCRLRANDV2
	mov 	[LEFTMASK],1100Y
	jmp 	SCRLRANDV3
SCRLRANDV2:
	mov 	[LEFTMASK],1000Y
SCRLRANDV3:

; Tabelle fÅr rechten Rand erstellen

	mov 	EAX,X
	add 	EAX,WIDTHH
	and 	AX,3
	jne 	SCRRRANDV0
	mov 	[RIGHTMASK],0000Y
	jmp 	SCRRRANDV3
SCRRRANDV0:
	cmp 	AX,1
	jne 	SCRRRANDV1
	mov 	[RIGHTMASK],0001Y
	jmp 	SCRRRANDV3
SCRRRANDV1:
	cmp 	AX,2
	jne 	SCRRRANDV2
	mov 	[RIGHTMASK],0011Y
	jmp 	SCRRRANDV3
SCRRRANDV2:
	mov 	[RIGHTMASK],0111Y
SCRRRANDV3:

	cmp 	WIDTHH,16
	jl 	SCRCOPYSMALLCHN

	mov 	EBX,X
	add 	EBX,15
	and 	EBX,-16  ; Start des Langwortbereichs

	mov 	EAX,X
	add 	EAX,3
	and 	EAX,-4  ; Start des Bytebereichs

	mov 	EDX,EBX
	sub 	EDX,EAX
	shr 	EDX,2
	mov 	LWIDTHCOPY,EDX  ; Breite des zu kopierenden Bytebereichs links

	mov 	EDX,X
	add 	EDX,WIDTHH
	and 	EDX,-16
	mov 	ECX,EDX
	sub 	EDX,EBX
	shr 	EDX,4
	mov 	WIDTHCOPY,EDX  ; Breite des zu kopierenden Langwortbereichs

	mov 	EAX,X
	add 	EAX,WIDTHH
	and 	EAX,-4  ; Start des Bytebereichs
	sub 	EAX,ECX  ; Bytes auf der rechten Seite
	shr 	EAX,2
	mov 	RWIDTHCOPY,EAX  ; Breite des zu kopierenden Bytebereichs rechts

	mov 	EAX,LWIDTHCOPY  ; Breite des zu kopierenden Bytebereichs links
	add 	EAX,RWIDTHCOPY
	mov 	EDX,WIDTHCOPY
	shl 	EDX,2
	add 	EAX,EDX
	mov 	EDX,VIDWIDTH
	sub 	EDX,EAX
	mov 	SRCMODULO,EDX  ; Zielmodulo
	shl 	EAX,2
	mov 	EDX,NEXTYPOS
	sub 	EDX,EAX
	mov 	DSTMODULO,EDX  ; Quellmodulo

; KopierRoutine

	mov 	LOP,0
	mov 	PLANE,1

; Sollte die Routine durch einen Interrupt unterbrochen werden wird jetzt die Page gerettet
; nur erlaubt wenn diese Routine nicht selbst durch einen Interrupt aufgerufen wird
	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINTS1
	mov 	_CHAIN4READSELECTOK,-1
FROMMOUSEINTS1:

	mov 	CX,4
SCRCOPYPLANES16:
	push CX

	cmp 	CX,4
	jne 	SCRSECONDPLANELOOP16
	mov 	AX,GCR_RMS+0000H
	mov 	DX,GCR_IND
	mov 	_CHAIN4LASTREADSELECT,0
	out 	DX,AX
	jmp 	SCRENDSECONDPLANELOOP16
SCRSECONDPLANELOOP16:
	mov 	AX,PLANE
	mov 	DX,GCR_DAT
	mov 	_CHAIN4LASTREADSELECT,AL
	out 	DX,AL
	inc 	PLANE
SCRENDSECONDPLANELOOP16:

; kopieren

	mov 	ESI,PLANEESI
	mov 	EDI,PLANEEDI

	add 	PLANEEDI,1  ; Quellbereich + 1

	mov 	EDX,HEIGHT
SCRCOPYHEIGHT16:

; Rand linke Seite
	mov 	AX,LOP
	bt 	[LEFTMASK],AX
	jnc 	SCRNOLRAND
	mov 	AL,-1[ESI]
	mov 	-4[EDI],AL
SCRNOLRAND:

; linke Seite
	mov 	ECX,LWIDTHCOPY
	or 	ECX,ECX
	je 	SCRNOCOPYLONGL4  ; wenn 0 nichts kopieren
SCRCOPYLONGL4:
	movsb
	add 	EDI,3
	loop SCRCOPYLONGL4
SCRNOCOPYLONGL4:

; Mittelteil
	mov 	ECX,WIDTHCOPY
	or 	ECX,ECX
	je 	SCRNOCOPYLONG16  ; wenn 0 nichts kopieren
SCRCOPYLONG16:
	mov 	EAX,[ESI]
	mov 	4[EDI],AH
	mov 	[EDI],AL
	shr 	EAX,16
	mov 	12[EDI],AH
	mov 	8[EDI],AL
	add 	ESI,4
	add 	EDI,16
	loop SCRCOPYLONG16
SCRNOCOPYLONG16:

; rechte Seite
	mov 	ECX,RWIDTHCOPY
	or 	ECX,ECX
	je 	SCRNOCOPYLONGR4  ; wenn 0 nichts kopieren
SCRCOPYLONGR4:
	movsb
	add 	EDI,3
	loop SCRCOPYLONGR4
SCRNOCOPYLONGR4:

; Rand rechte Seite
	mov 	AX,LOP
	bt 	[RIGHTMASK],AX
	jnc 	SCRNORRAND
	mov 	AL,[ESI]
	mov 	[EDI],AL
SCRNORRAND:

	add 	ESI,SRCMODULO  ; nÑchste Zeile Source
	add 	EDI,DSTMODULO  ; nÑchste Zeile dest

	dec 	EDX
	jg 	SCRCOPYHEIGHT16

	inc 	LOP

	pop 	CX
	dec 	CX
	jg 	SCRCOPYPLANES16

; Page wird nicht gerettet
	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINTSB1
	mov 	_CHAIN4READSELECTOK,0
FROMMOUSEINTSB1:

	pop 	ES

	ret
SCRCOPYSMALLCHN:
; Spezialroutine fÅr Bitmaps mit Breite < 16 Pixel

	mov 	EBX,X
	add 	EBX,3
	and 	EBX,-4  ; Start des bereichs

	mov 	EDX,X
	add 	EDX,WIDTHH
	and 	EDX,-4
	sub 	EDX,EBX
	js 	ONE4BYTE
	shr 	EDX,2
	mov 	WIDTHCOPY,EDX  ; Breite des zu kopierenden bereichs

	mov 	EAX,WIDTHCOPY
	mov 	EDX,VIDWIDTH
	sub 	EDX,EAX
	mov 	SRCMODULO,EDX  ; Zielmodulo
	shl 	EAX,2
	mov 	EDX,NEXTYPOS
	sub 	EDX,EAX
	mov 	DSTMODULO,EDX  ; Quellmodulo

; kopieren

	mov 	LOP,0
	mov 	PLANE,1

; Sollte die Routine durch einen Interrupt unterbrochen werden wird jetzt die Page gerettet
; nur erlaubt wenn diese Routine nicht selbst durch einen Interrupt aufgerufen wird
	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINTS2
	mov 	_CHAIN4READSELECTOK,-1
FROMMOUSEINTS2:

	mov 	CX,4
SCRCOPYPLANES16SMALL:
	push CX

	cmp 	CX,4
	jne 	SCRSECONDPLANELOOP16SMALL
	mov 	AX,GCR_RMS+0000H
	mov 	DX,GCR_IND
	mov 	_CHAIN4LASTREADSELECT,0
	out 	DX,AX
	jmp 	SCRENDSECONDPLANELOOP16SMALL
SCRSECONDPLANELOOP16SMALL:
	mov 	AX,PLANE
	mov 	DX,GCR_DAT
	mov 	_CHAIN4LASTREADSELECT,AL
	out 	DX,AL
	inc 	PLANE
SCRENDSECONDPLANELOOP16SMALL:

	mov 	ESI,PLANEESI
	mov 	EDI,PLANEEDI

	add 	PLANEEDI,1  ; Zielbereich + 1

	mov 	EDX,HEIGHT
SCRCOPYHEIGHT16SMALL:

; Rand linke Seite
	mov 	AX,LOP
	bt 	[LEFTMASK],AX
	jnc 	SCRNOLRANDSMALL
	mov 	AL,-1[ESI]
	mov 	-4[EDI],AL
SCRNOLRANDSMALL:

; Mittelteil
	mov 	ECX,WIDTHCOPY
	or 	ECX,ECX
	je 	SCRNOCOPYBYTE16SMALL  ; wenn 0 nichts kopieren
SCRCOPYBYTE16SMALL:
	movsb
	add 	EDI,3
	loop SCRCOPYBYTE16SMALL
SCRNOCOPYBYTE16SMALL:

; Rand rechte Seite
	mov 	AX,LOP
	bt 	[RIGHTMASK],AX
	jnc 	SCRNORRANDSMALL
	mov 	AL,[ESI]
	mov 	[EDI],AL
SCRNORRANDSMALL:

	add 	ESI,SRCMODULO  ; nÑchste Zeile Source
	add 	EDI,DSTMODULO  ; nÑchste Zeile dest

	dec 	EDX
	jg 	SCRCOPYHEIGHT16SMALL

	inc 	LOP

	pop 	CX
	dec 	CX
	jg 	SCRCOPYPLANES16SMALL

; Page wird nicht gerettet
	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINTSB2
	mov 	_CHAIN4READSELECTOK,0
FROMMOUSEINTSB2:

	pop 	ES

	ret
SCRONE4BYTE:
; Spezialroutine wenn Opm in ein 4 Pixel bereich kopiert werden muss

	mov 	AX,[LEFTMASK]
	and 	AX,[RIGHTMASK]  ; herausfinden welche Bits
	mov 	[LEFTMASK],AX

; kopieren

	mov 	LOP,0
	mov 	PLANE,1

; Sollte die Routine durch einen Interrupt unterbrochen werden wird jetzt die Page gerettet
; nur erlaubt wenn diese Routine nicht selbst durch einen Interrupt aufgerufen wird
	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINTS3
	mov 	_CHAIN4READSELECTOK,-1
FROMMOUSEINTS3:

	mov 	CX,4
SCRCOPYPLANES4SMALL:
	push CX

	cmp 	CX,4
	jne 	SCRSECONDPLANELOOP4SMALL
	mov 	AX,GCR_RMS+0000H
	mov 	DX,GCR_IND
	mov 	_CHAIN4LASTREADSELECT,0
	out 	DX,AX
	jmp 	SCRENDSECONDPLANELOOP4SMALL
SCRSECONDPLANELOOP4SMALL:
	mov 	AX,PLANE
	mov 	DX,GCR_DAT
	mov 	_CHAIN4LASTREADSELECT,AL
	out 	DX,AL
	inc 	PLANE
SCRENDSECONDPLANELOOP4SMALL:

	mov 	ESI,PLANEESI
	mov 	EDI,PLANEEDI

	add 	PLANEEDI,1  ; Quellbereich + 1

	mov 	EDX,HEIGHT
SCRCOPYHEIGHT4SMALL:

; Rand beide Seiten
	mov 	AX,LOP
	bt 	[LEFTMASK],AX
	jnc 	SCRNORANDSMALL
	mov 	AL,-1[ESI]
	mov 	-4[EDI],AL
SCRNORANDSMALL:

	add 	EDI,NEXTYPOS  ; nÑchste Zeile Source
	add 	ESI,VIDWIDTH  ; nÑchste Zeile dest

	dec 	EDX
	jg 	SCRCOPYHEIGHT4SMALL

	inc 	LOP

	pop 	CX
	dec 	CX
	jg 	SCRCOPYPLANES4SMALL

; Page wird nicht gerettet
	test _INMOUSEINTERRUPT,-1
	jne 	FROMMOUSEINTSB3
	mov 	_CHAIN4READSELECTOK,0
FROMMOUSEINTSB3:

	pop 	ES

	ret
DSA_ASS_CopyScreenToOPMCHN_ ENDP

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_ActivateCHAINFOUR_
; * FUNCTION  : aktiviert Chain4 Mode schreibt dazu in Hardwareregister
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:16:38
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     : funktioniert nur bei 320x200 oder 360x240
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_ActivateCHAINFOUR_ PROC C widthh:DWORD,height:DWORD

	cmp 	WIDTHH,320  ; 320x200 ?
	je 	CHAIN4_320X200

	cmp 	WIDTHH,360  ; 360x240 ?
	jne 	FALSECHAIN4
	cmp 	HEIGHT,240
	jne 	FALSECHAIN4

; Register beschreiben um 360x240 zu aktivieren

	safewait  	 ; like BIOS, Flush Cache, wait a little
	out_b [CRTC_IND],CRTC_VRL  ; read Vertical Retrace Register
	in_b [CRTC_DAT]  ;
	and 	AL,7FH  ; disable ProtectionBit.
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out_b [CRTC_DAT],AL  ; write Vertical Retrace Register
					 ;
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out_b TS_IND,TS_MMO  ; Sequencer: MemoryModeRegister
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out_b TS_DAT,6  ; set PlaneMode & sequential Adressing
					 ;
	cli 	 		 ; While SynchReset switch Interrupts off
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out_b TS_IND,TS_R  ; Sequencer: SynchronousReset
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out_b TS_DAT,1  ; make Synchronous Reset
					 ;
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out_b MISC_IND,0E7H  ; write MiscOutputRegister
					 ;
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out_b TS_IND,TS_R  ; Sequencer: SynchronousReset
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out_b TS_DAT,3  ; make Synchronous Reset
	sti 	 		 ; enable Int's

; komplettes Videoram lîschen
	out_w TS_IND,TS_MM+0F00H  ; Select MapMask in Seq. all Four Planes
					 ;
	sub 	EAX,EAX  ;
	mov 	ECX,3FFFH  ; z.B. 360x240/4/2=10800 (Wordtransfer)
	mov 	EDI,0A0000H
	rep 	STOSD  ; fill RAM

; Register beschreiben um 360x240 zu aktivieren
	lea 	ESI,CRTC_36X24  ; lea table index.
	mov 	DX,[CRTC_IND]  ; now modify CRTC.
	sub 	AL,AL  ; clear register count.
	mov 	ECX,18H  ; loop 24 times.
@@INITCRTC:    ;
	mov 	AH,[ESI]  ; get value from table.
	cmp 	AH,0FFH  ; ? should this register be modified
	je 	@@L01  ; ! F: skip the out statement.
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out 	DX,AX  ; ! T: write value to CRTC.
@@L01:   		 ;
	inc 	ESI 	 ; prepare next table entry.
	inc 	AL 	 ; prepare next register.
	loop @@INITCRTC  ;
					 ;
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out_b [CRTC_IND],CRTC_VRL  ; read Vertical Retrace Register
	in_b [CRTC_DAT]  ;
	or 	AL,80H  ; enable ProtectionBit.
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out_b [CRTC_DAT],AL  ; write Vertical Retrace Register

	mov 	AX,-1
	ret
CHAIN4_320X200:
	cmp 	HEIGHT,200
	jne 	FALSECHAIN4

					 ; Chain 4-Addressierung ausschalten
	mov 	DX,03C4H  ; Sequenzer-Port
	mov 	AL,4H  ; Memory-Mode-Register
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out 	DX,AL  ; selektieren
	mov 	DX,3C5H  ; Memory-Mode-Register
	in 	AL,DX  ; einlesen
	and 	AL,0F7H  ; Bit 4 ausblenden (Chain 4-Bit)
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out 	DX,AL  ; schreiben

; komplettes Videoram lîschen
	out_w TS_IND,TS_MM+0F00H  ; Select MapMask in Seq. all Four Planes
					 ;
	sub 	EAX,EAX  ;
	mov 	ECX,3FFFH  ; z.B. 360x240/4/2=10800 (Wordtransfer)
	mov 	EDI,0A0000H
	rep 	STOSD  ; fill RAM

					 ; Zugriff auf CRTC-Register 0..7 erlauben
	mov 	DX,03D4H  ; DX=CRTC-Indexregister
	mov 	AL,11H  ; Index fÅr End-Vertical-Retrace
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out 	DX,AL  ; Register selektieren
	inc 	DX 	 ; DX=3c5h (CRTC-Datenregister)
	in 	AL,DX  ; einlesen
	and 	AL,07FH  ; Zugriff auf CRTC-Register 0..7 erlauben
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out 	DX,AL  ; schreiben
					 ; CRTC - Adressiermodus einstellen
	mov 	AL,017H  ; CRTC [17]
	mov 	AH,0C3H  ; Adressrotation
	mov 	DX,003D4H  ;
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out 	DX,AX  ;
					 ;
	mov 	AL,014H  ; CRTC [14]
	mov 	AH,0  ; Wortmodus einstellen
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out 	DX,AX  ;

	mov 	AX,-1
	ret
FALSECHAIN4:
	xor 	AX,AX
	ret
DSA_ASS_ActivateCHAINFOUR_ ENDP

; Variablen und Tabellen

CRTC_IND   	DW 03D4H 	; CRT-ControllerIndexReg (3B4h for Monochrome)
CRTC_DAT   	DW 03D5H 	; CRTC-DataReg (3B5h for Monochrome)

CRTC_36X24:    	; CRTC values for 360x240
			DB 06BH,059H,05AH,08EH,05EH,08AH,00DH,03EH ;
			DB 0FFH,0C0H,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH ;
			DB 0EAH,0ACH,0DFH,02DH,000H,0E7H,006H,0E3H ;


; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_SetVideoramCHAINFOUR_(screenadr)
; * FUNCTION  : Setzt AusgabeFenster auf entsprechende Addresse im Videoram
; *							au·erdem wird auf obere Bildschirmzeile gewartet
; *							screenadr(EBX) = zu setzende Addresse
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :11.08.94 12:47:42
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     : benîtigt Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_SetVideoramCHN_ PROC C

; wait on vbl
	mov 	DX,03DAH
@@02:  	 		 ;
	in 	AL,DX  ; load StatusByte
	test AL,8 	 ; in VBL?
	jne 	@@02 	 	; no

; set new adr of videoram
	mov 	DX,03D4H
	mov 	AX,BX  ;
	mov 	AL,CRTC_SAH  ; Grafikstartaddr angeben
	cli 	 		 ;
	out 	DX,AX  ;
	mov 	AX,BX  ;
	mov 	AH,AL  ; LowAddress to AH
	mov 	AL,CRTC_SAL  ;
	out 	DX,AX  ;
	sti 	 		 ;

; wait until vlb
	mov 	DX,03DAH
@@03:  	 		 ;
	in 	AL,DX  ; wait until VBL
	test AL,8 	 ;
	je 	@@03  ;

	ret
DSA_ASS_SetVideoramCHN_ ENDP

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_WaitVBL
; * FUNCTION  : wartet bis VBL beendet
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :11.08.94 12:47:35
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     : benîtigt Vesatreiber
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_WaitVBL_ PROC C

	mov 	DX,03DAH
@@AV01:   		 ;
	in 	AL,DX  ; wait until end of VBL
	test AL,8 	 ;
	jne 	@@AV01  ;

	ret 	 		 ;

DSA_ASS_WaitVBL_ ENDP

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_WaitEndVBL
; * FUNCTION  : wartet auf Anfang des VBL
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:17:06
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     :
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_WaitEndVBL_ PROC C

	mov 	DX,03DAH
@@BV01:   		 ;
	in 	AL,DX  ; wait until VBL
	test AL,8 	 ;
	je 	@@BV01  ;

	ret 	 		 ;

DSA_ASS_WaitEndVBL_ ENDP

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_WaitTOF
; * FUNCTION  : wartet auf obere Bildschirmzeile
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:16:59
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     :
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_WaitTOF_ PROC C

	mov 	DX,03DAH

	in 	AL,DX  ; load StatusByte
	test AL,8 	 ; in VBL?
	jne 	@@A01  ; YES
@@A02:   		 ;
	in 	AL,DX  ; wait of VBL
	test AL,8 	 ;
	je 	@@A02  ;
					 ;
@@A01:   		 ;
	in 	AL,DX  ; wait until end of VBL
	test AL,8 	 ;
	jne 	@@A01  ;

	ret 	 		 ;

DSA_ASS_WaitTOF_ ENDP

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_WaitBOF
; * FUNCTION  : wartet auf untere Bildschirmzeile
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:17:06
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     :
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_WaitBOF_ PROC C

	mov 	DX,03DAH

	in 	AL,DX  ; load StatusByte
	test AL,8 	 ; in VBL?
	je 	@@B01  ; no
@@B02:   		 ;
	in 	AL,DX  ; wait on end of VBL
	test AL,8 	 ;
	jne 	@@B02  ;
					 ;
@@B01:   		 ;
	in 	AL,DX  ; wait until VBL
	test AL,8 	 ;
	je 	@@B01  ;

	ret 	 		 ;

DSA_ASS_WaitBOF_ ENDP

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_SaveVesaPage
; * FUNCTION  : speichert alte VesaSeite in savepage
; *
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:17:11
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     : benîtigt Vesatreiber
; *							Funktion wird nur von externen C-Funktionen verwendet
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_SaveVesaPage_ PROC C
	mov 	AX,_LASTPAGE
	mov 	_SAVEPAGE,AX
	ret 	 		 ;
DSA_ASS_SaveVesaPage_ ENDP

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_RestoreVesaPage
; * FUNCTION  : setzt zuletzt gespeicherte VesaSeite von savepage
;	*
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:17:18
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     : benîtigt Vesatreiber
;	*							Funktion wird nur von externen C-Funktionen verwendet
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_RestoreVesaPage_ PROC C
	mov 	DX,_SAVEPAGE
	cmp 	_LASTPAGE,DX
	je 	NOOTHERVESAPAGE
	mov 	_LASTPAGE,DX
	mov 	AX,4F05H  ; Funktion Vesa Set Window
	xor 	BX,BX  ; bh = Funktion Mem / bl = Set Window
	int 	10H
	test	_twowindows,-1
	je		NOTWOWINDOWS4
	mov	DX,_LASTPAGE
	mov 	AX,4F05H
	mov	BX,1 ; bh = Funktion Mem / bl = Set Window 1
	int 	10H
NOTWOWINDOWS4:
NOOTHERVESAPAGE:
	ret 	 		 ;
DSA_ASS_RestoreVesaPage_ ENDP

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_SaveCHAIN4Page
; * FUNCTION  : speichert alte CHAIN4Seite in savepage
;	*
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:17:23
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     :	Funktion wird nur von externen C-Funktionen verwendet
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_SaveCHAIN4Page_ PROC C
	mov 	AL,_CHAIN4LASTWRITESELECT
	mov 	_CHAIN4SAVEWRITESELECT,AL
	mov 	AL,_CHAIN4LASTREADSELECT
	mov 	_CHAIN4SAVEREADSELECT,AL
	ret 	 		 ;
DSA_ASS_SaveCHAIN4Page_ ENDP

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_RestoreCHAIN4Page
; * FUNCTION  : setzt zuletzt gespeicherte CHAIN4Seite von savepage
;	*
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:17:30
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     :	Funktion wird nur von externen C-Funktionen verwendet
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_RestoreCHAIN4Page_ PROC C

	test _CHAIN4WRITESELECTOK,-1
	je 	NOCH4WR3
;	/* WriteSelect auf vorigen Wert */
	mov 	AL,TS_MM
	mov 	AH,_CHAIN4SAVEWRITESELECT
	mov 	DX,TS_IND
	cli
	out 	DX,AX
	sti
NOCH4WR3:

	test _CHAIN4READSELECTOK,-1
	je 	NOCH4WR4
;	/* ReadSelect auf vorigen Wert */
	mov 	AL,GCR_RMS
	mov 	AH,_CHAIN4SAVEREADSELECT
	mov 	DX,GCR_IND
	cli
	out 	DX,AX
	sti
NOCH4WR4:

	ret
DSA_ASS_RestoreCHAIN4Page_ ENDP

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : DSA_ASS_FLASH
; * FUNCTION  : Farbe 0 umschalten
; *
; * FILE      : DSA_ASS.ASM
; * AUTHOR    : viper
; * FIRST     : 25.05.94 14:00
; * LAST      :24.07.94 21:17:30
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     :	Funktion wird nur von externen C-Funktionen verwendet
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

DSA_ASS_FLASH_ PROC C
	cli 	 		 ;
	sub 	AX,AX  ; clear AX
	mov 	DX,03C8H  ; DX with ColorPort
	safewait  	 ; like BIOS, Flush Cache, wait a little
	out 	DX,AL  ; Start with ColorRegister 0

	mov		ax,CX
	safewait  	 ; like BIOS, Flush Cache, wait a little
	mov 	DX,03C9H  ; DX with ColorPort
	out 	DX,AL  ; Set Color -> RED

	mov		ax,SI
	safewait  	 ; like BIOS, Flush Cache, wait a little
	mov 	DX,03C9H  ; DX with ColorPort
	out 	DX,AL  ; Set Color -> RED

	mov		ax,DI
	safewait  	 ; like BIOS, Flush Cache, wait a little
	mov 	DX,03C9H  ; DX with ColorPort
	out 	DX,AL  ; Set Color -> RED

	sti
	ret
DSA_ASS_FLASH_ ENDP

DSA  	ENDS 				;

END



; Macro ersetzt rep movsb
;rep_b MACRO
;	LOCAL copylop,jmpcopylop
;	push ax
;	jmp jmpcopylop
;copylop:
;	mov	al,[ESI]
;	mov	[edi],al
;	inc esi
;	inc edi
;jmpcopylop:
;	dec cx
;	jns copylop
;	pop ax
;ENDM

