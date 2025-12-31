; ************
; * NAME     : BBHARD.INC
; * AUTOR    : Viper, BlueByte
; * START    : 25.05.94 14:00
; * PROJECT  : Poject32/Macintosh
; * NOTES    : MacroDefinitionen fÅr MASM
; * SEE ALSO :
; * VERSION  : 1.0
; ************/

; Defines fÅr MaximaleWerte Bildschirm

MAXSCREENWIDTH_PCLIB EQU 802
MAXSCREENHEIGHT_PCLIB EQU 602

; CRTC: Cathode-Ray-Tube-Controller
				;
CRTC_HT   	EQU 00H 	; HorizontalTotal.
CRTC_HDE   	EQU 01H 	; HorizontalDisplayEnd.
CRTC_HSHB   	EQU 02H 	; StartHorizontalBlank
CRTC_HEHB   	EQU 03H 	; EndHorizontalBlank
CRTC_HSHR   	EQU 04H 	; StartHorizontalRetrace
CRTC_HEHR   	EQU 05H 	; EndHorizontalRetrace
CRTC_VT   	EQU 06H 	; VerticalTotal
;CRTC_O   	EQU 07H 	; Overflow
CRTC_PRS   	EQU 08H 	; PresetRowScan
CRTC_MSL   	EQU 09H 	; MaxScanLine
CRTC_CS   	EQU 0AH 	; CursorStart
CRTC_CE   	EQU 0BH 	; CursorEnd
CRTC_SAH   	EQU 0CH 	; StartAddressHigh.
CRTC_SAL   	EQU 0DH 	; StartAddressLow.
CRTC_CLH   	EQU 0EH 	; CursorLocationHigh
CRTC_CLL   	EQU 0FH 	; CursorLocationLow
CRTC_VRS   	EQU 10H 	; VerticalRetraceStart.
CRTC_VRL   	EQU 11H 	; VerticalRetraceLow
CRTC_VDE   	EQU 12H 	; VerticalDisplayEnd.
CRTC_O   	EQU 13H 	; Offset
CRTC_UL   	EQU 14H 	; UnderlineLocation
CRTC_SVB   	EQU 15H 	; StartVerticalBlank
CRTC_EVB   	EQU 16H 	; EndVerticalBlank
CRTC_MODE   	EQU 17H 	; Mode
CRTC_LC   	EQU 18H 	; LineCompare

; TS: Timing Sequencer

TS_IND   	EQU 3C4H 	; TS-Index.
TS_DAT   	EQU 3C5H 	; TS-Data.

TS_R  	 	EQU 00H 	; Reset.
TS_CM  	 	EQU 01H 	; Clocking Mode.
TS_MM  	 	EQU 02H 	; MapMask.
TS_CMS   	EQU 03H 	; CharacterMapSelect
TS_MMO   	EQU 04H 	; MemoryMOde.
TS_SCR   	EQU 06H 	; StateControl.
TS_AMR   	EQU 07H 	; AuxiliaryMode.

; GDC: Graphics-Controller Registers
				;
GCR_SS   	EQU 3CDH 	; GDC SegmentSelect.
				;
GCR_IND   	EQU 3CEH 	; GDC-Index.
GCR_DAT   	EQU 3CFH 	; GDC-Data.
				;
GCR_SR   	EQU 00H 	; Set/Reset.
GCR_ESR   	EQU 01H 	; EnableSet/Reset.
GCR_CC   	EQU 02H 	; ColorCompare.
GCR_DR   	EQU 03H 	; DataRotate
GCR_RMS   	EQU 04H 	; ReadMapSelect.
GCR_M  	 	EQU 05H 	; Mode.
GCR_MISC   	EQU 06H 	; Miscellaneous.
GCR_CDC   	EQU 07H 	; ColorDontCare.
GCR_BM   	EQU 08H 	; BitMask.

; MISC: Miscellaneous Output Register

MISC_IND   	EQU 03C2H 	; Write Misc
MISC_DAT   	EQU 03CCH 	; Read Misc

