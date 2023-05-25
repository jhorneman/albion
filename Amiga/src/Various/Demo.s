
	XREF	Exit_program
	XREF	Add_HDOB
	XREF	Remove_HDOB
	XREF	Mouse_X
	XREF	Mouse_Y
	XREF	Read_Mev
	XREF	Read_key
	XREF	Push_Root
	XREF	Switch_screens
	XREF	Update_screen
	XREF	VScroll_block
	XREF	Copy_screen
	XREF	Main_palette
	XREF	Set_full_palette
	XREF	Set_palette_part
	XREF	My_vsync
	XREF	Recolour_palette_part
	XREF	Wait_4_rightunclick
	XREF	Draw_hline
	XREF	Draw_vline
	XREF	Draw_box
	XREF	Plot_pixel
	XREF	Item_list_class
	XREF	BT_Button_class
	XREF	BS_Button_class
	XREF	TS_Button_class
	XREF	Add_object
	XREF	Execute_method
	XREF	Handle_object_interaction
	XREF	Button_state
	XREF	Claim_pointer
	XREF	Free_pointer
	XREF	Put_masked_block
	XREF	Put_unmasked_block
	XREF	Recolour_box
	XREF	On_screen
	XREF	Off_screen
	XREF	Work_screen
	XREF	BT_Switch_class
	XREF	BS_Switch_class
	XREF	TS_Switch_class
	XREF	Radio_class
	XREF	BT_Radio_button_class
	XREF	BS_Radio_button_class
	XREF	TS_Radio_button_class
	XREF	Pop_up_menu_class
	XREF	Wait_4_object
	XREF	Earth_class
	XREF	DBox_class
	XREF	Text_area_class
	XREF	Marmor_box
	XREF	Text_list_class

	XDEF	Init_program


	incdir	DDT:
	include	Constants/Global.i
	include	Constants/OS/OS.i
	include	Constants/Hardware_registers.i
	include	Constants/Core.i
	include	Constants/Hull.i
	include	Constants/User_interface.i


	SECTION	Program,code
Init_program:
	lea.l	Darkness,a0
	moveq.l	#0,d0
	moveq.l	#1,d1
	jsr	Set_palette_part

	lea.l	Main_palette,a0
	move.w	#192,d0
	moveq.l	#64,d1
	jsr	Set_palette_part

	lea.l	Test_pal,a0
	moveq.l	#0,d0
	move.w	#192,d1
	jsr	Set_palette_part

	lea.l	Main_palette,a0
	lea.l	Recolouring_table+192,a1
	lea.l	Gazonga,a2
	move.w	#192,d0
	moveq.l	#64,d1
	jsr	Recolour_palette_part

	lea.l	Test_pal,a0
	lea.l	Recolouring_table,a1
	lea.l	Gazonga,a2
	moveq.l	#0,d0
	move.w	#192,d1
	jsr	Recolour_palette_part

;	lea.l	HDOBje,a0
;	jsr	Add_HDOB

	moveq.l	#0,d0
	moveq.l	#0,d1
	move.w	#Screen_width-1,d2
	move.w	#Screen_height-1,d3
	jsr	Marmor_box

	moveq.l	#32,d0
	moveq.l	#0,d1
	moveq.l	#8,d5
	moveq.l	#13,d6
	move.w	#186,d7
	lea.l	Blobje,a0
	jsr	Put_unmasked_block

	lea.l	Earth_class,a0
	lea.l	Earth_OID,a1
	moveq.l	#-1,d0
	jsr	Add_object
	move.w	d0,GNARL

	lea.l	Item_list_class,a0
	lea.l	Item_list_OID,a1
	move.w	GNARL,d0
	jsr	Add_object

	moveq.l	#Draw_method,d1
	jsr	Execute_method

	lea.l	BT_Button_class,a0
	lea.l	Button_OID,a1
	move.w	GNARL,d0
	jsr	Add_object

	moveq.l	#Draw_method,d1
	jsr	Execute_method

	lea.l	BS_Button_class,a0
	lea.l	Button2_OID,a1
	move.w	GNARL,d0
	jsr	Add_object

	moveq.l	#Draw_method,d1
	jsr	Execute_method

	lea.l	TS_Button_class,a0
	lea.l	Button3_OID,a1
	move.w	GNARL,d0
	jsr	Add_object

	moveq.l	#Draw_method,d1
	jsr	Execute_method

	lea.l	BT_Switch_class,a0
	lea.l	Switch_OID,a1
	move.w	GNARL,d0
	jsr	Add_object

	moveq.l	#Draw_method,d1
	jsr	Execute_method

	lea.l	Radio_class,a0
	move.w	GNARL,d0
	jsr	Add_object
	move.w	d0,GNARL3
	move.w	d0,d7

	lea.l	BT_Radio_button_class,a0
	lea.l	Radio1_OID,a1
	move.w	d7,OID_Radio_handle(a1)
	move.w	d7,d0
	jsr	Add_object

	lea.l	BS_Radio_button_class,a0
	lea.l	Radio2_OID,a1
	move.w	d7,OID_Radio_handle(a1)
	move.w	d7,d0
	jsr	Add_object

	lea.l	BT_Radio_button_class,a0
	lea.l	Radio3_OID,a1
	move.w	d7,OID_Radio_handle(a1)
	move.w	d7,d0
	jsr	Add_object

	move.w	d7,d0
	moveq.l	#Set_method,d1
	moveq.l	#1,d2
	jsr	Execute_method

	move.w	d7,d0
	moveq.l	#Draw_method,d1
	jsr	Execute_method

;	lea.l	DBox_class,a0
;	lea.l	Text_border_OID,a1
;	moveq.l	#0,d0
;	jsr	Add_object
;
;	moveq.l	#Draw_method,d1
;	jsr	Execute_method

	lea.l	Text_area_class,a0
	lea.l	Text_OID,a1
	jsr	Add_object
	move.w	d0,GNARL2

	moveq.l	#Erase_method,d1
	jsr	Execute_method

	lea.l	Text_list_class,a0
	lea.l	Text_list_OID,a1
	move.w	GNARL,d0
	jsr	Add_object

	moveq.l	#Draw_method,d1
	jsr	Execute_method

	moveq.l	#15,d0
	moveq.l	#11,d1
	move.w	#16+162-1,d2
	move.w	#12+92-1,d3
	lea.l	Recolouring_table,a0
	jsr	Recolour_box

	jsr	Update_screen

.Wait:	move.w	Mouse_X,d0
	move.w	Mouse_Y,d1
	move.b	Button_state,d2
	jsr	Handle_object_interaction

	jsr	Switch_screens

	jsr	Read_key
	tst.l	d0
	beq.s	.Wait

	jmp	Exit_program

Wowie_zowie:
	move.w	GNARL3,d0			; Get radio status
	moveq.l	#Get_method,d1
	jsr	Execute_method
	lea.l	Texts,a1			; Get text
	move.l	-4(a1,d2.w*4),a1
	move.w	GNARL2,d0			; Print text
	moveq.l	#Print_method,d1
	jsr	Execute_method
	rts

Gazonga:
	mulu.w	#60,d0
	divu.w	#100,d0
	mulu.w	#60,d1
	divu.w	#100,d1
	mulu.w	#60,d2
	divu.w	#100,d2
	rts

Dumdum:
	rts

Yog_sototh:
	move.l	.Ptrs-4(pc,d0.w*4),a0
	rts

.Ptrs:	dc.l .T1,.T2,.T3,.T4,.T5
	dc.l .T6,.T7,.T8,.T9,.T10
	dc.l .T11,.T12,.T13,.T14,.T15
	dc.l .T16,.T17,.T18,.T19,.T20

.T1:	dc.b "One",0
.T2:	dc.b "Two",0
.T3:	dc.b "Three",0
.T4:	dc.b "Four",0
.T5:	dc.b "Five",0
.T6:	dc.b "Six",0
.T7:	dc.b "Seven",0
.T8:	dc.b "Eight",0
.T9:	dc.b "Nine",0
.T10:	dc.b "Ten",0
.T11:	dc.b "Eleven",0
.T12:	dc.b "Twelve",0
.T13:	dc.b "Thirteen",0
.T14:	dc.b "Fourteen",0
.T15:	dc.b "Fifteen",0
.T16:	dc.b "Sixteen",0
.T17:	dc.b "Seventeen",0
.T18:	dc.b "Eighteen",0
.T19:	dc.b "Nineteen",0
.T20:	dc.b "Twenty",0
	even


	SECTION	Chip_DATA,data_c
Mannetje:
	incbin	DDT:Graphics/Ready/Yo_ho
Blockje:
	incbin	DDT:Graphics/Ready/Test_button
	incbin	DDT:Graphics/Ready/Test_button2
Blobje:
	incbin	DDT:Graphics/Ready/Test_blob
Object_graphics:
	incbin	DDT:Graphics/Ready/Items/01
	incbin	DDT:Graphics/Ready/Items/02
	incbin	DDT:Graphics/Ready/Items/03
	incbin	DDT:Graphics/Ready/Items/04


	SECTION	Fast_DATA,data
Test_pal:
	incbin	DDT:Graphics/Ready/Test_192.pal

GNARL:	dc.w 0
GNARL2:	dc.w 0
GNARL3:	dc.w 0

Earth_OID:
	dc.w 0,0
	dc.w Screen_width,Screen_height

Item_list_OID:
	dc.w 20,110
	dc.w 6,3
	dc.w 60
	dc.b 0
	even
	dc.l Googa
	dc.l Dumdum,Dumdum
	dc.l PUM_pje

Button_OID:
	dc.w 200,1
	dc.w 50,16
	dc.l .Yahooga
	dc.l Wowie_zowie
	dc.w Gold+1,Gold+3

.Yahooga:	dc.b "Button",0
	even

Button2_OID:
	dc.w 200,21
	dc.w 40,40
	dc.l A_symbol
	dc.l Dumdum

Button3_OID:
	dc.w 250,21
	dc.w 40,40
	dc.l A_symbol
	dc.l Dumdum

A_symbol:	dc.w 2,29,192
	dc.b 6,0
	dc.l Blockje

Switch_OID:
	dc.w 260,1
	dc.w 50,16
	dc.l .Yahooga
	dc.l Dumdum
	dc.w Gold+1,Gold+3

.Yahooga:	dc.b "Switch",0
	even

Radio1_OID:
	dc.w 200,65
	dc.w 30,16
	dc.l .Yahooga
	dc.l Dumdum
	dc.w Gold+1,Gold+3
	dc.w 1,0

.Yahooga:	dc.b "One",0
	even

Radio2_OID:
	dc.w 232,65
	dc.w 40,40
	dc.l A_symbol
	dc.l Dumdum
	dc.l 0
	dc.w 2,0

Radio3_OID:
	dc.w 274,65
	dc.w 30,16
	dc.l .Yahooga
	dc.l Dumdum
	dc.w Gold+1,Gold+3
	dc.w 3,0

.Yahooga:	dc.b "Three",0
	even

Text_border_OID:
	dc.w 14,10
	dc.w 164,94

Text_OID:
	dc.w 14+2,10+2
	dc.w 160,90
	dc.w Turquoise,0,-1

Text_list_OID:
	dc.w 170,111,50
	dc.w 2,7,20,Red,Red+1
	dc.l Yog_sototh
	dc.l Dumdum,Dumdum

PUM_pje:	dc.w 7
	dc.l .Title
	dc.l Dumdum
	dc.w $800,0
	dc.l .Text1,Dumdum
	dc.w $800,0
	dc.l .Text2,Dumdum
	dc.w $200,0
	dc.l 0,0
	dc.w $400,0
	dc.l .Text3,Dumdum
	dc.w $400,0
	dc.l .Text4,Dumdum
	dc.w $400,0
	dc.l .Text5,Dumdum
	dc.w $400,0
	dc.l .Text6,Dumdum

.Title:	dc.b "{FAT }Torch{NORS}",0
.Text1:	dc.b "Drop",0
.Text2:	dc.b "Examine",0
.Text3:	dc.b "Learn spell",0
.Text4:	dc.b "Drink",0
.Text5:	dc.b "Activate",0
.Text6:	dc.b "Read",0
	even

HDOBje:	dc.w 16,16
	dc.w 6,96,0
	dc.b 8,0
	dc.l Mannetje
	dc.w $0800
	dcb.w 6,0

Texts:	dc.l Blah1,Blah2,Blah3

Blah1:	dc.l Text1,0
Blah2:	dc.l Text2,0
Blah3:	dc.l Text3,0

Text1:
	dc.b "{DFAU}"
	dc.b "{CAPI}T{UNCA}he color wheel and gradient slider are new gadget classes added to the"
	dc.b " boopsi facility in the V39 release of Intuition. They provide enhanced"
	dc.b " color selection gadgets for use with the expanded color capabilities of"
	dc.b " the new AA chip set and are also backward-compatible with ECS. This"
	dc.b " article explains how to use these two new gadget classes."
	dc.b "{VJMP020}",CR
	dc.b "{CAPI}T{UNCA}he color wheel and gradient slider are based on boopsi (boopsi is an"
	dc.b " acronym for Basic Object-Oriented Programming System for Intuition) so it"
	dc.b " helps to know a little about how boopsi works before trying to use them."
	dc.b " For information on boopsi, refer to chapter 12 and appendix B of the Amiga"
	dc.b " ROM Kernel Reference Manual: Libraries (ISBN 0-201-56774-1)."
	dc.b "{VJMP020}",CR
	dc.b "{CAPI}A{UNCA}nother method of specifying colors is in terms of hue, saturation and"
	dc.b " brightness, HSB. Hue is the dominant wavelength in the light we receive;"
	dc.b " we associate color names with hue. Saturation is a measure of the purity"
	dc.b " of the dominant wavelength, that is, the more dominant the wavelength, the"
	dc.b " more pure or saturated it is. Brightness is a measure of the luminance or"
	dc.b " brilliance of the wavelength."
	dc.b 0 

Text2:	dc.b "{DFAU}{TECF}"
	dc.b "{CAPI}I{UNCA}ntuition now has direct support for the AA display modes, through"
	dc.b " extensions to old mechanisms and through new tags. This includes the"
	dc.b " ability to select higher resolutions and to set colors in better than 4"
	dc.b " bits-per-gun. The graphics database and the ASL screen-mode requester"
	dc.b " are the definitive places to get information about what modes and"
	dc.b " depths are available or desired. You can specify higher depths using"
	dc.b " SA_Depth, and new display modes with SA_DisplayID. SA_Colors32"
	dc.b " supplants SA_Colors for specifying colors with a higher precision than"
	dc.b " 4 bits-per-gun. For full details, see the section on new screen"
	dc.b " features."
	dc.b "{VJMP020}",CR
	dc.b "{FAT }{CNTR}Mode Promotion{NORS}",CR,CR,"{JUST}"
	dc.b "The AA chipset provides some flicker-free display modes that are"
	dc.b " roughly equivalent to NTSC or PAL when output through a"
	dc.b " display-enhancer or flicker-fixing product. While the AA chipset"
	dc.b " provides these modes without the significant extra expense of a"
	dc.b " display-enhancer, some software help is required. Conversely, the"
	dc.b " display-enhancer lives on the video output, and is completely"
	dc.b " transparent to software."
	dc.b 0 

Text3:	dc.b "{DFAU}"
	dc.b "Localization is the process by which software dynamically adapts to"
	dc.b " different locales. A locale is made up of a set of attributes"
	dc.b " describing a language, a set of cultural and lingual conventions, a"
	dc.b " physical location, etc."

	dc.b " Without {HIGH}standardized{NORS} system support to help deal with localization"
	dc.b " issues, the task of localizing applications is significant. There"
	dc.b " needs to be several {INK 194}different versions{DFAU} of every application, each"
	dc.b " specially adapted to run in a particular language and country."

	dc.b " Given the importance of the international market to the Amiga, it is"
	dc.b " imperative that the operating system provide services to facilitate,"
	dc.b " and thus encourage, application software localization. This is where"
	dc.b " locale.library comes in."

	dc.b " The {INK 202}{NSHA}locale.library{DFAU} is an Amiga shared library offering services to let"
	dc.b " applications transparently adapt to any locale the user has chosen."
	dc.b " Functions are provided for formatted information display, text catalog"
	dc.b " management, character attribute acquisition, string sorting, and more."
	dc.b 0 


	SECTION	Fast_BSS,bss
Recolouring_table:	ds.b 256
	even
