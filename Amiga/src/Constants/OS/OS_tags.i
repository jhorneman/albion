; OS contants : tags
; Written by J.Horneman (In Tune With The Universe)
; Start : 27-1-1994


; This is the TagItem structure
	rsreset
ti_Tag:	rs.l 1		; Identifies the type of the data
ti_Data:	rs.l 1		; Type-specific data
ti_data_size:	rs.b 0


; Control tags
TAG_DONE	equ 0		; Terminates array of TagItems. ti_Data unused
TAG_END	equ 0		; Synonym for TAG_DONE
TAG_IGNORE	equ 1		; Ignore this item, not end of array
TAG_MORE	equ 2		; ti_Data is pointer to another array of TagItems
			; Note that this tag terminates the current array
TAG_SKIP	equ 3		; Skip this and the next ti_Data items


; Differentiates user tags from control tags
TAG_USER   equ $80000000


; Tags for OpenScreenTagList
	rsset TAG_USER+33
SA_Left:	rs.b 1
SA_Top:	rs.b 1
SA_Width:	rs.b 1
SA_Height:	rs.b 1
SA_Depth:	rs.b 1
SA_DetailPen:	rs.b 1
SA_BlockPen:	rs.b 1
SA_Title:	rs.b 1
SA_Colors:	rs.b 1
SA_ErrorCode:	rs.b 1
SA_Font:	rs.b 1
SA_SysFont:	rs.b 1
SA_Type:	rs.b 1
SA_BitMap:	rs.b 1
SA_PubName:	rs.b 1
SA_PubSig:	rs.b 1
SA_PubTask:	rs.b 1
SA_DisplayID:	rs.b 1
SA_DClip:	rs.b 1
SA_Overscan:	rs.b 1
SA_Obsolete1:	rs.b 1
SA_ShowTitle:	rs.b 1		; Booleans
SA_Behind:	rs.b 1
SA_Quiet:	rs.b 1
SA_AutoScroll:	rs.b 1
SA_Pens:	rs.b 1
SA_FullPalette:	rs.b 1
SA_ColorMapEntries:	rs.b 1
SA_Parent:	rs.b 1
SA_Draggable:	rs.b 1
SA_Exclusive:	rs.b 1
SA_SharePens:	rs.b 1
SA_BackFill:	rs.b 1
SA_Interleaved:	rs.b 1
SA_Colors32:	rs.b 1
SA_VideoControl:	rs.b 1
SA_FrontChild:	rs.b 1
SA_BackChild:	rs.b 1
SA_LikeWorkbench:	rs.b 1
SA_Reserved:	rs.b 1
SA_MinimizeISG:	rs.b 1


; Tags for OpenWindowTagList
	rsset TAG_USER+100
WA_Left:	rs.b 1
WA_Top:	rs.b 1
WA_Width:	rs.b 1
WA_Height:	rs.b 1
WA_DetailPen:	rs.b 1
WA_BlockPen:	rs.b 1
WA_IDCMP:	rs.b 1
WA_Flags:	rs.b 1
WA_Gadgets:	rs.b 1
WA_Checkmark:	rs.b 1
WA_Title:	rs.b 1
WA_ScreenTitle:	rs.b 1
WA_CustomScreen:	rs.b 1
WA_SuperBitMap:	rs.b 1
WA_MinWidth:	rs.b 1
WA_MinHeight:	rs.b 1
WA_MaxWidth:	rs.b 1
WA_MaxHeight:	rs.b 1
WA_InnerWidth:	rs.b 1
WA_InnerHeight:	rs.b 1
WA_PubScreenName:	rs.b 1
WA_PubScreen:	rs.b 1
WA_PubScreenFallBack:	rs.b 1
WA_WindowName:	rs.b 1
WA_Colors:	rs.b 1
WA_Zoom:	rs.b 1
WA_MouseQueue:	rs.b 1
WA_BackFill:	rs.b 1
WA_RptQueue:	rs.b 1
WA_SizeGadget:	rs.b 1
WA_DragBar:	rs.b 1
WA_DepthGadget:	rs.b 1
WA_CloseGadget:	rs.b 1
WA_Backdrop:	rs.b 1
WA_ReportMouse:	rs.b 1
WA_NoCareRefresh:	rs.b 1
WA_Borderless:	rs.b 1
WA_Activate:	rs.b 1
WA_RMBTrap:	rs.b 1
WA_WBenchWindow:	rs.b 1
WA_SimpleRefresh:	rs.b 1
WA_SmartRefresh:	rs.b 1
WA_SizeBRight:	rs.b 1
WA_SizeBBottom:	rs.b 1
WA_AutoAdjust:	rs.b 1
WA_GimmeZeroZero:	rs.b 1
WA_MenuHelp:	rs.b 1
WA_NewLookMenus:	rs.b 1
WA_AmigaKey:	rs.b 1
WA_NotifyDepth:	rs.b 1
WA_Obsolete:	rs.b 1
WA_Pointer:	rs.b 1
WA_BusyPointer:	rs.b 1
WA_PointerDelay:	rs.b 1
WA_TabletMessages:	rs.b 1
WA_HelpGroup:	rs.b 1
WA_HelpGroupWindow:	rs.b 1


; Tags for AllocSpriteData
SPRITEA_Width	equ $81000000
SPRITEA_XReplication	equ $81000002
SPRITEA_YReplication	equ $81000004
SPRITEA_OutputHeight	equ $81000006
SPRITEA_Attached	equ $81000008
	; MUST pass in OutputHeight if using this tag
SPRITEA_OldDataFormat	equ $8100000a


; Tags for GetExtSprite
GSTAG_SPRITE_NUM	equ $82000020
GSTAG_ATTACHED	equ $82000022
GSTAG_SOFTSPRITE	equ $82000024


; Tags valid for either GetExtSprite or ChangeExtSprite
	; Request "NTSC-Like" height if possible.
GSTAG_SCANDOUBLED	equ $83000000

; Tags for VideoControl
VTAG_END_CM	equ $00000000
VTAG_CHROMAKEY_CLR	equ $80000000
VTAG_CHROMAKEY_SET	equ $80000001
VTAG_BITPLANEKEY_CLR	equ $80000002
VTAG_BITPLANEKEY_SET	equ $80000003
VTAG_BORDERBLANK_CLR	equ $80000004
VTAG_BORDERBLANK_SET	equ $80000005
VTAG_BORDERNOTRANS_CLR	equ $80000006
VTAG_BORDERNOTRANS_SET	equ $80000007
VTAG_CHROMA_PEN_CLR	equ $80000008
VTAG_CHROMA_PEN_SET	equ $80000009
VTAG_CHROMA_PLANE_SET	equ $8000000A
VTAG_ATTACH_CM_SET		equ $8000000B
VTAG_NEXTBUF_CM	equ $8000000C
VTAG_BATCH_CM_CLR	equ $8000000D
VTAG_BATCH_CM_SET	equ $8000000E
VTAG_NORMAL_DISP_GET	equ $8000000F
VTAG_NORMAL_DISP_SET	equ $80000010
VTAG_COERCE_DISP_GET	equ $80000011
VTAG_COERCE_DISP_SET	equ $80000012
VTAG_VIEWPORTEXTRA_GET	equ $80000013
VTAG_VIEWPORTEXTRA_SET	equ $80000014
VTAG_CHROMAKEY_GET	equ $80000015
VTAG_BITPLANEKEY_GET	equ $80000016
VTAG_BORDERBLANK_GET	equ $80000017
VTAG_BORDERNOTRANS_GET	equ $80000018
VTAG_CHROMA_PEN_GET	equ $80000019
VTAG_CHROMA_PLANE_GET	equ $8000001A
VTAG_ATTACH_CM_GET	equ $8000001B
VTAG_BATCH_CM_GET	equ $8000001C
VTAG_BATCH_ITEMS_GET	equ $8000001D
VTAG_BATCH_ITEMS_SET	equ $8000001E
VTAG_BATCH_ITEMS_ADD	equ $8000001F
VTAG_VPMODEID_GET	equ $80000020
VTAG_VPMODEID_SET	equ $80000021
VTAG_VPMODEID_CLR	equ $80000022
VTAG_USERCLIP_GET	equ $80000023
VTAG_USERCLIP_SET	equ $80000024
VTAG_USERCLIP_CLR	equ $80000025

; the following tags are V39 specific. They will be ignored by earlier versions
VTAG_PF1_BASE_GET	equ $80000026
VTAG_PF2_BASE_GET	equ $80000027
VTAG_SPEVEN_BASE_GET	equ $80000028
VTAG_SPODD_BASE_GET	equ $80000029
VTAG_PF1_BASE_SET	equ $8000002a
VTAG_PF2_BASE_SET	equ $8000002b
VTAG_SPEVEN_BASE_SET	equ $8000002c
VTAG_SPODD_BASE_SET	equ $8000002d
VTAG_BORDERSPRITE_GET	equ $8000002e
VTAG_BORDERSPRITE_SET	equ $8000002f
VTAG_BORDERSPRITE_CLR	equ $80000030
VTAG_SPRITERESN_SET	equ $80000031
VTAG_SPRITERESN_GET	equ $80000032
VTAG_PF1_TO_SPRITEPRI_SET	equ $80000033
VTAG_PF1_TO_SPRITEPRI_GET	equ $80000034
VTAG_PF2_TO_SPRITEPRI_SET	equ $80000035
VTAG_PF2_TO_SPRITEPRI_GET	equ $80000036
VTAG_IMMEDIATE	equ $80000037
VTAG_FULLPALETTE_SET	equ $80000038
VTAG_FULLPALETTE_GET	equ $80000039
VTAG_FULLPALETTE_CLR	equ $8000003A
VTAG_DEFSPRITERESN_SET	equ $8000003B
VTAG_DEFSPRITERESN_GET	equ $8000003C


; These are values for VTAG_SPRITERESN_SET and VTAG_SPRITERESN_GET
SPRITERESN_ECS	equ 0	; 140ns, except in 35ns viewport, where it is 70ns.
SPRITERESN_140NS	equ 1
SPRITERESN_70NS	equ 2
SPRITERESN_35NS	equ 3
SPRITERESN_DEFAULT	equ -1


; Datachunk type identifiers
DTAG_DISP	equ  $80000000
DTAG_DIMS	equ  $80001000
DTAG_MNTR	equ  $80002000
DTAG_NAME	equ  $80003000
DTAG_VEC	equ  $80004000	; internal use only


; DIPF flags

DIPF_IS_LACE	EQU $00000001
DIPF_IS_DUALPF	EQU $00000002
DIPF_IS_PF2PRI	EQU $00000004
DIPF_IS_HAM	EQU $00000008
DIPF_IS_ECS	EQU $00000010
	; note: ECS modes (SHIRES, VGA, and PRODUCTIVITY) do not support
	; attached sprites.
DIPF_IS_AA	EQU $00010000
	; AA modes - may only be available if machine has correct memory
	; type to support required bandwidth - check availability. (V39)

DIPF_IS_PAL	EQU $00000020
DIPF_IS_SPRITES	EQU $00000040
DIPF_IS_GENLOCK	EQU $00000080

DIPF_IS_WB	EQU $00000100
DIPF_IS_DRAGGABLE	EQU $00000200
DIPF_IS_PANELLED	EQU $00000400
DIPF_IS_BEAMSYNC	EQU $00000800

DIPF_IS_EXTRAHALFBRITE	EQU $00001000

; The following DIPF_IS_... flags are new for V39

DIPF_IS_SPRITES_ATT	EQU $00002000
	; Supports attached sprites
DIPF_IS_SPRITES_CHNG_RES	EQU $00004000
	; Supports variable sprite resolution
DIPF_IS_SPRITES_BORDER	EQU $00008000
	; Sprite can be displayed in the border
DIPF_IS_SCANDBL	EQU $00020000
	; Scan doubled
DIPF_IS_SPRITES_CHNG_BASE	EQU $00040000
	; Can change the sprite base colour
DIPF_IS_SPRITES_CHNG_PRI	EQU $00080000
	; Can change the sprite priority with respect to the playfield(s).
DIPF_IS_DBUFFER	EQU $00100000
	; Can support double buffering
DIPF_IS_PROGBEAM	EQU $00200000
	; Is a programmed beam-sync mode
DIPF_IS_FOREIGN	EQU $80000000
	; This mode is not native to the Amiga

; Tags for BestModeID

SPECIAL_FLAGS	EQU (DIPF_IS_DUALPF|DIPF_IS_PF2PRI|DIPF_IS_HAM|DIPF_IS_EXTRAHALFBRITE)

BIDTAG_DIPFMustHave	EQU $80000001
	; Mask of the DIPF_ flags the ModeID must have
	; Default - NULL

BIDTAG_DIPFMustNotHave	EQU $80000002
	; Mask of the DIPF_ flags the ModeID must not have
	; Default - SPECIAL_FLAGS

BIDTAG_ViewPort	EQU $80000003
	; ViewPort for which a ModeID is sought.
	; Default - NULL

BIDTAG_NominalWidth	EQU $80000004
BIDTAG_NominalHeight	EQU $80000005
	; Together make the aspect ratio and override the
	;  vp->Width/Height.
	; Default - SourceID NominalDimensionInfo,
	;  or vp->DWidth/Height, or (640 * 200),
	;  in that preferred order.

BIDTAG_DesiredWidth	EQU $80000006
BIDTAG_DesiredHeight	EQU $80000007
	; Nominal Width and Height of the returned ModeID.
	; Default - same as Nominal

BIDTAG_Depth	EQU $80000008
	; ModeID must support this depth.
	; Default - vp->RasInfo->BitMap->Depth or 1

BIDTAG_MonitorID	EQU $80000009
	; ModeID must use this monitor.
	; Default - use best monitor available

BIDTAG_SourceID	EQU $8000000a
	; Instead of a ViewPort.
	; Default - VPModeID(vp) if BIDTAG_ViewPort is
	;  specified, else leave the DIPFMustHave and
	;  DIPFMustNotHave values untouched.

BIDTAG_RedBits	EQU $8000000b
BIDTAG_BlueBits	EQU $8000000c
BIDTAG_GreenBits	EQU $8000000d
	; Match up from the database
	; Default - 4

BIDTAG_GfxPrivate	EQU $8000000e	; Private
