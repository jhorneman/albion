; ************
; * NAME     : POLYTEX.asm
; * AUTOR    : Viper, BlueByte
; * START    : 25.05.94 14:00
; * PROJECT  : BB3DLIB
; * NOTES    :
; * SEE ALSO :
; * VERSION  : 1.0
; ************/

; ******************************************************************************
; * #FUNCTION HEADER BEGIN#
; * NAME      : _DRAWTEXTPOLY(SILONG *area,UNSHORT *textarea,UNBYTE *teximage,UNBYTE *transtable,SILONG anz,UNLONG typ);
; * FUNCTION  : Zeichnet Texture Polygon
; *
; *							SILONG *area = Zeiger auf PolygonKoordinaten (Bildschirmkoordinaaten)
; *                     	Format:
; *								(SILONG)X0,(SILONG)Y0,
; *								(SILONG)X1,(SILONG)Y1,
; *								(SILONG)X2,(SILONG)Y3,
; *								...
; *
; *							UNSHORT *textarea = Zeiger auf TextureKoordinaten
; *								Werte gehen von 0-65535 (Jedes Pixel kann
; *								256-fach unterteilt werden)
; *                     	Format:
; *								(UNSHORT)X0,(UNSHORT)Y0,
; *								(UNSHORT)X1,(UNSHORT)Y1,
; *								(UNSHORT)X2,(UNSHORT)Y2,
; *								...
; *
; *							UNBYTE *teximage = Zeiger auf TexturBild
; *								im Format 256*256 Pixel muss an einer 64 KB Grenze
; *								im Speicher aligned sein.
; *
; *							UNBYTE *transtable = Zeiger auf 64K oder 256 Byte Umrechungstabelle
; *
; *							SILONG anz = Anzahl Eckpunkte die in area und Textarea stehen
; *
; *							UNLONG typ = verschiedene Flags wie z.B. transparent, Shilouette	usw...
; *
; * FILE      : POLY.ASM
; * AUTHOR    : viper
; * FIRST     : 27.07.94 14:00
; * LAST      :20.10.94 10:11:11
; * INPUTS    :
; * RESULT    : None.
; * BUGS      :
; * NOTES     :
; * SEE ALSO  :
; * VERSION   : 1.0
; * #FUNCTION HEADER END#

