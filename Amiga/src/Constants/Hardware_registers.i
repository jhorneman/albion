; Amiga hardware register offsets
; Original by Commodore Business Machines
; Adapted by J.Horneman

	OPT	GENSYM

XX	macro
	OPT	NOCHKIMM
	and.w	#$1dff,Custom+bplcon3
	move.w	#\1,Custom+colour00
	OPT	CHKIMM
	endm

; ******** Exception vectors ********

Level_1_vec	EQU	$64
Level_2_vec	EQU	$68
Level_3_vec	EQU	$6c
Level_4_vec	EQU	$70
Level_5_vec	EQU	$74
Level_6_vec	EQU	$78
Level_7_vec	EQU	$7c

; ******** Custom chip registers ********

Custom		EQU	$dff000		; Custom chip register base

bltddat	EQU $000			
dmaconr	EQU $002			; DMA control read
vposr	EQU $004 			
vhposr	EQU $006
dskdatr	EQU $008
joy0dat	EQU $00A			; Game-port 0 position
joy1dat	EQU $00C			; Game-port 1 position
clxdat	EQU $00E

adkconr     EQU   $010
pot0dat     EQU   $012
pot1dat     EQU   $014
potinp	  EQU   $016
serdatr     EQU   $018
dskbytr     EQU   $01A
intenar     EQU   $01C			; Interrupt enable read
intreqr     EQU   $01E			; Interrupt request read

dskpt	    EQU   $020
dsklen	    EQU   $024
dskdat	    EQU   $026
refptr	    EQU   $028
vposw	    EQU   $02A
vhposw	    EQU   $02C
copcon	    EQU   $02E			; Copper control
serdat	    EQU   $030
serper	    EQU   $032
potgo	    EQU   $034
joytest     EQU   $036
strequ	    EQU   $038
strvbl	    EQU   $03A
strhor	    EQU   $03C
strlong     EQU   $03E

bltcon0     EQU   $040			; Blitter control 0
bltcon1     EQU   $042			; Blitter control 1
bltafwm     EQU   $044			; First data word mask for A
bltalwm     EQU   $046			; Last data word mask for A
bltcpt	    EQU   $048			; Source C address
bltbpt	    EQU   $04C			; Source B address
bltapt	    EQU   $050			; Source A address
bltdpt	    EQU   $054			; Destination D address
bltsize     EQU   $058			; Start blitter & blit size

bltcon0l    EQU   $05B			; note: byte access only
bltsizv     EQU   $05C
bltsizh     EQU   $05E

bltcmod     EQU   $060			; Source C modulo value
bltbmod     EQU   $062 			; Source B module value
bltamod     EQU   $064			; Source A modulo value
bltdmod     EQU   $066			; Destination D modulo value

bltcdat     EQU   $070
bltbdat     EQU   $072
bltadat     EQU   $074

deniseid    EQU   $07C
dsksync     EQU   $07E

cop1lc	    EQU   $080			; Address of first copper list
cop2lc	    EQU   $084			; Address of second copper list
copjmp1     EQU   $088			; First copper list strobe register
copjmp2     EQU   $08A			; Second copper list strobe register
copins	    EQU   $08C		
diwstrt     EQU   $08E
diwstop     EQU   $090
ddfstrt     EQU   $092
ddfstop     EQU   $094
dmacon	    EQU   $096			; DMA control write
clxcon	    EQU   $098
intena	    EQU   $09A			; Interrupt enable write
intreq	    EQU   $09C			; Interrupt request write
adkcon	    EQU   $09E

aud	    EQU   $0A0
aud0	    EQU   $0A0
aud1	    EQU   $0B0
aud2	    EQU   $0C0
aud3	    EQU   $0D0

bpl1pt	    EQU   $0E0			; Bitplane addresses
bpl2pt	    EQU   $0E4
bpl3pt	    EQU   $0E8
bpl4pt	    EQU   $0EC
bpl5pt	    EQU   $0F0
bpl6pt	    EQU   $0F4

bplcon0     EQU   $100			; Bitplane control registers
bplcon1     EQU   $102			; (Scroll register)
bplcon2     EQU   $104
bplcon3     EQU   $106
bpl1mod     EQU   $108			; Odd bitplane modulo value
bpl2mod     EQU   $10A			; Even bitplane modulo value
bplhmod     EQU   $10C

bpldat	    EQU   $110

sprpt	    EQU   $120

spr	    EQU   $140

colour00    EQU   $180			; Colour registers
colour01    EQU   $182
colour02    EQU   $184
colour03    EQU   $186
colour04    EQU   $188
colour05    EQU   $18A
colour06    EQU   $18C
colour07    EQU   $18E
colour08    EQU   $190
colour09    EQU   $192
colour10    EQU   $194
colour11    EQU   $196
colour12    EQU   $198
colour13    EQU   $19A
colour14    EQU   $19C
colour15    EQU   $19E
colour16    EQU   $1A0
colour17    EQU   $1A2
colour18    EQU   $1A4
colour19    EQU   $1A6
colour20    EQU   $1A8
colour21    EQU   $1AA
colour22    EQU   $1AC
colour23    EQU   $1AE
colour24    EQU   $1B0
colour25    EQU   $1B2
colour26    EQU   $1B4
colour27    EQU   $1B6
colour28    EQU   $1B8
colour29    EQU   $1BA
colour30    EQU   $1BC
colour31    EQU   $1BE

htotal	    EQU   $1c0			; ECS registers
hsstop	    EQU   $1c2
hbstrt	    EQU   $1c4
hbstop	    EQU   $1c6
vtotal	    EQU   $1c8
vsstop	    EQU   $1ca
vbstrt	    EQU   $1cc
vbstop	    EQU   $1ce
sprhstrt    EQU   $1d0
sprhstop    EQU   $1d2
bplhstrt    EQU   $1d4
bplhstop    EQU   $1d6
hhposw	    EQU   $1d8
hhposr	    EQU   $1da
beamcon0    EQU   $1dc
hsstrt	    EQU   $1de
vsstrt	    EQU   $1e0
hcenter     EQU   $1e2
diwhigh     EQU   $1e4

; ******** CIA registers ********

CIA_A		EQU	$bfe000		; CIA-A register base
CIA_B		EQU	$BFD000		; CIA-B register base

pra	EQU	$001
prb	EQU	$101
ddra	EQU	$201
ddrb	EQU	$301
talo	EQU	$401
tahi	EQU	$501
tblo	EQU	$601
tbhi	EQU	$701
elsb	EQU	$801
e8_15	EQU	$901
emsb	EQU	$A01
*sp	EQU	$C01
icr	EQU	$D01
cra	EQU	$E01
crb	EQU	$F01

; ******** Audio channel data ********

ac_ptr	    EQU   $00	; ptr to start of waveform data
ac_len	    EQU   $04	; length of waveform in words
ac_per	    EQU   $06	; sample period
ac_vol	    EQU   $08	; volume
ac_dat	    EQU   $0A	; sample pair
ac_SIZEOF   EQU   $10

; ******** Hardware sprite data ********

sd_pos	    EQU   $00
sd_ctl	    EQU   $02
sd_dataa    EQU   $04
sd_datab    EQU   $06
sd_SIZEOF   EQU   $08
