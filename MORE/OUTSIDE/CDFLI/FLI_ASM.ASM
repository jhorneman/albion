	.MODEL HUGE
	.386


GLOBAL _SKIP3EMSPAGES: PROC

	.CODE

	INCLUDE INC\LIBMAC.INC

; ****************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : _DOCHUNK12
; * FUNCTION  :
; * FILE      : FLI_ASM.ASM
; * AUTHOR    : RN, BLUEBYTE
; * FIRST     : 07.07.94 10:28
; * LAST      : 08.07.94 10:28
; * INPUTS    : UNBYTE far *: Cursor in dem Animationsfile
; * RESULT    : UNBYTE far *:    "    "   "  "
; * BUGS      : Wenn mehr als 256 Zeilen zu bearbeiten sind!!!
; *		(linestogo wird in bh-Register gehalten.)
; * NOTES     : Registerbelegung:
; *
; *             ds:si  =  fliworkptr	/* ANIMWORKPTR */
; *		es:di  =  vw		/* work pointer for video ram */
; *		dx     =  vb		/* pointer to base line in video ram */
; *		bl     =  packets	/* packets in line */
; *		bh     =  linestogo	/* lines to go in chunk */
; *		cx     =  count		/* local var */
; *
; * SEE ALSO  : FLI_ASM.C (= gleiche Routine in C )
; * VERSION   : 1.0
; * #FUNCTION HEADER END#
; ****************************************************************************

PUBLIC _DOCHUNK12;
PUBLIC _CLEARVGA;

_DOCHUNK12  	PROC

		ARG ANIMWORKPTR: DWORD; (UNBYTE far *)

	       	procinit

	        ;/* set fli work pointer */
	        ;fliworkptr = ANIMWORKPTR; (== ds:[si])
	        lds  si, ANIMWORKPTR

		mov  ax,0a000h
		mov  es,ax

	        ;/* get video ram base pointer */
	        ;vb = ( ( UNBYTE far * ) 0xa0000000 ) + *( ( UNSHORT * ) fliworkptr ) * 320;
		;vb == dx

	        mov  ax, 320
		mov  bx, word ptr ds:[si]
	        mul  bx
		mov  dx, ax

	        ;/* get lines to go */
	        ;linestogo = *( ( UNSHORT * ) (fliworkptr+2) );

		mov  bh, ds:[si+2]

	        ;fliworkptr += 4;
	        add  si, 4

	        ;/* any line to go ? */
	        ;if ( !linestogo )
	        ;  return;

	        cmp  bh,0
	        je   _DOCHUNKexit

		cld

	        ;/* yes ! */
	        ;/* work on all lines */
_DOCHUNKlp0:    ;do{
	        ;  /* get number of packets */
	        ;  packets = ( *fliworkptr++ );  (== bl)

	        mov  bl, byte ptr ds:[si]
		inc  si

	        ;  /* any packet for this line ? */
	        ;  if ( packets ){

	        cmp  bl,0
	        je   _DOCHUNKgo2

	        ;  /* yes ! */
	        ;  /* set video work pointer */
	        ;  vw = vb; ( vw == di )

		mov di, dx

	        ;  /* startup skip */
	        ;  vw += ( UNSHORT ) ( *fliworkptr++ );

		mov al,byte ptr ds:[si]
		mov ah,0
		add di,ax
		inc si

	        ;    /* for on all packets */
_DOCHUNKlp1:    ;    do{
	        ;      /* load counter */
	        ;      count = *fliworkptr++;

		lodsb    		; count ist in al
		cbw			; vorzeichenrichtig erweitern -> ax
		mov cx,ax		; L„nge ist nun in cx

	        ;      /* counter positive ? */
	        ;      if ( count >= 0 ){

	        test cx,08000h
	        jne  _DOCHUNKelse

	        ;	 /* yes: copy string */
 	        ;	 memcpy( vw, fliworkptr, count );

		shr cx,1
		jnb _DOCHUNKmovs1
		movsb

_DOCHUNKmovs1:  shr cx,1
		jnb _DOCHUNKmovs2
		movsw

_DOCHUNKmovs2:  rep movsd

		jmp  _DOCHUNKifend

_DOCHUNKelse:   ;      }else{
		;	 color = *fliworkptr++; (== al)

	        lodsb
		mov ah,al
		shl eax,8
		mov al,ah
		shl eax,8
		mov al,ah

	        ;	 setmem( vw, -count, color );

		neg cx

		shr cx,1
		jnb _DOCHUNKstos1
		stosb

_DOCHUNKstos1:	shr cx,1
		jnb _DOCHUNKstos2
		stosw

_DOCHUNKstos2:	rep stosd

_DOCHUNKifend:  ;      }
	        ;      /* add skip byte */
		;    if ( packets )

		cmp bl,1
		je _DOCHUNKgo1

		;      vw += ( UNSHORT ) ( *fliworkptr++ );

		mov al, byte ptr ds:[si]
		mov ah, 0
		add di, ax
	        inc si

_DOCHUNKgo1:    ;    }while ( packets );

	        ;/* dec number of packets */

		dec bl
	        jne _DOCHUNKlp1

_DOCHUNKgo2:    ;   /* next line in video ram */
	        ;   vb += 320;

		add dx, 320

	        ;   /* setup ems and anim pointer */
	        ;   /* EMS offset > 48K ? */
	        ;     if ( FP_OFF( fliworkptr ) >= 0xc000 ){

	        cmp  si,0c000h
	        jb   _DOCHUNKgo0

	        ;       /* yes ! */
	        ;       /* skip 3 EMS pages */
	        ;       SKIP3EMSPAGES();

		push bx
		push dx
		push es
		call _SKIP3EMSPAGES C
		pop  es
		pop  dx
		pop  bx

	        ;       /* recalc pointer */
	        ;       fliworkptr -= 0xc000;

		sub  si,0c000h

	        ;     }
_DOCHUNKgo0:    ;   }while ( linestogo );

	        dec bh
	        jne _DOCHUNKlp0


_DOCHUNKexit:   ;/* return pointer */
	        ;return( fliworkptr );

	        mov  ax,ds
	        mov  dx,si
	       	procexit

_DOCHUNK12  	ENDP


; ****************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : CLEARVGA
; * FUNCTION  : L”scht den Vga-Screen
; * FILE      : FLI_ASM.ASM
; * AUTHOR    : RN, BLUEBYTE
; * FIRST     : 08.07.94 12:32
; * LAST      : 08.07.94 10:28
; * INPUTS    : -
; * RESULT    : -
; * BUGS      : -
; * NOTES     : -
; * SEE ALSO  : -
; * VERSION   : 1.0
; * #FUNCTION HEADER END#
; ****************************************************************************

_CLEARVGA	PROC

		procinit

		mov ax,0a000h
		mov es,ax
		mov di,0

		mov eax,0

		cld
		mov cx,64000 / 4
		rep stosd

		procexit

_CLEARVGA	ENDP
END












































