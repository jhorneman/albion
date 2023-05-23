; OS contants & macros
; Written by J.Horneman (In Tune With The Universe)
; Start : 27-1-1994

	OPT	GENSYM

	incdir	DDT:
	include	Constants/Global.i


Exec_base	EQU	4

;*****************************************************************************
; These are OS constants

; Device commands
CMD_NONSTD	EQU 9

; Interrupt types
INTB_COPER	EQU 4
INTB_VERTB	EQU 5

; Memory allocation types
MEMF_ANY	EQU 0
MEMF_PUBLIC	EQU 1
MEMF_CHIP	EQU 1<<1
MEMF_FAST	EQU 1<<2
MEMF_CLEAR	EQU 1<<16
MEMF_LARGEST	EQU 1<<17

; Lock modes
ACCESS_WRITE	EQU -1
ACCESS_READ	EQU -2

; Open modes
MODE_OLDFILE	EQU 1005
MODE_NEWFILE	EQU 1006
MODE_READWRITE	EQU 1004

; Seek modes
OFFSET_BEGINNING	EQU -1
OFFSET_CURRENT	EQU 0
OFFSET_END	EQU 1

; Node types
NT_UNKNOWN		EQU 0
NT_TASK		EQU 1		; Exec task
NT_INTERRUPT	EQU 2
NT_DEVICE	EQU 3
NT_MSGPORT	EQU 4
NT_MESSAGE	EQU 5			; Indicates message currently pending
NT_FREEMSG	EQU 6
NT_REPLYMSG	EQU 7		; Message has been replied
NT_RESOURCE	EQU 8
NT_LIBRARY	EQU 9
NT_MEMORY	EQU 10
NT_SOFTINT	EQU 11			; Internal flag used by SoftInits
NT_FONT		EQU 12
NT_PROCESS	EQU 13			; AmigaDOS Process
NT_SEMAPHORE	EQU 14
NT_SIGNALSEM	EQU 15		; Signal semaphores
NT_BOOTNODE	EQU 16
NT_KICKMEM	EQU 17
NT_GRAPHICS	EQU 18
NT_DEATHMESSAGE	EQU 19

; ie_Class values
IECLASS_RAWKEY	EQU 1
IECLASS_RAWMOUSE	EQU 2
IECLASS_TIMER	EQU 6
IECLASS_DISKREMOVED	EQU 15
IECLASS_DISKINSERTED	EQU 16

; ie_Qualifier values
IEQUALIFIERB_LSHIFT	EQU 0
IEQUALIFIERB_RSHIFT	EQU 1
IEQUALIFIERB_CAPSLOCK	EQU 2
IEQUALIFIERB_CONTROL	EQU 3
IEQUALIFIERB_LALT	EQU 4
IEQUALIFIERB_RALT	EQU 5
IEQUALIFIERB_LCOMMAND	EQU 6
IEQUALIFIERB_RCOMMAND	EQU 7
IEQUALIFIERB_NUMERICPAD	EQU 8
IEQUALIFIERB_REPEAT	EQU 9
IEQUALIFIERB_INTERRUPT	EQU 10
IEQUALIFIERB_MULTIBROADCAST	EQU 11
IEQUALIFIERB_MIDBUTTON	EQU 12
IEQUALIFIERB_RBUTTON	EQU 13
IEQUALIFIERB_LEFTBUTTON	EQU 14
IEQUALIFIERB_RELATIVEMOUSE	EQU 15

ie_button_mask	EQU (1<<IEQUALIFIERB_RBUTTON)|(1<<IEQUALIFIERB_LEFTBUTTON)

; Console device commands
CD_ASKKEYMAP	EQU (CMD_NONSTD+0)
CD_SETKEYMAP	EQU (CMD_NONSTD+1)
CD_ASKDEFAULTKEYMAP	EQU (CMD_NONSTD+2)
CD_SETDEFAULTKEYMAP	EQU (CMD_NONSTD+3)
RawKeyConvert	EQU -48

; Input device commands
IND_ADDHANDLER	EQU (CMD_NONSTD+0)
IND_REMHANDLER	EQU (CMD_NONSTD+1)
PeekQualifier	EQU -42

; For AllocBitMap
BMF_CLEAR	EQU 1
BMF_DISPLAYABLE	EQU 2
BMF_INTERLEAVED	EQU 4
BMF_STANDARD	EQU 8
BMF_MINPLANES	EQU 16

; For Console Unit
CONU_LIBRARY	EQU -1

; Screen flags
CUSTOMSCREEN	EQU $000F
CUSTOMBITMAP	EQU $0040

; Task flags
TB_PROCTIME	EQU 0
TB_ETASK	EQU 3
TB_STACKCHK	EQU 4
TB_EXCEPT	EQU 5
TB_SWITCH	EQU 6
TB_LAUNCH	EQU 7

; Task states
TS_INVALID	EQU 0
TS_ADDED	EQU 1
TS_RUN	EQU 2
TS_READY	EQU 3
TS_WAIT	EQU 4
TS_EXCEPT	EQU 5
TS_REMOVED	EQU 6

; AttnFlags bits
AFB_68010	EQU 0			; Also set for 68020
AFB_68020	EQU 1			; Also set for 68030
AFB_68030	EQU 2			; Also set for 68040
AFB_68040	EQU 3

; Overscan types
OSCAN_TEXT	EQU 1			; Entirely visible
OSCAN_STANDARD	EQU 2		; Just past edges
OSCAN_MAX	EQU 3			; As much as possible
OSCAN_VIDEO	EQU 4		; Even more than is possible

; Cache manipulation bits
CACRF_ClearI	EQU 1<<3
CACRF_ClearD	EQU 1<<11

; IECLASS_RAWMOUSE codes
IECODE_UP_PREFIX	EQU $80
IECODE_LBUTTON	EQU $68
IECODE_RBUTTON	EQU $69
IECODE_MBUTTON	EQU $6A

; RAWMOUSE codes and qualifiers (Console OR IDCMP)
SELECTUP	EQU (IECODE_LBUTTON+IECODE_UP_PREFIX)
SELECTDOWN	EQU (IECODE_LBUTTON)
MENUUP	EQU (IECODE_RBUTTON+IECODE_UP_PREFIX)
MENUDOWN	EQU (IECODE_RBUTTON)
MIDDLEUP	EQU (IECODE_MBUTTON+IECODE_UP_PREFIX)
MIDDLEDOWN	EQU (IECODE_MBUTTON)

; Parameters for GfxNew
VIEW_EXTRA_TYPE EQU 1
VIEWPORT_EXTRA_TYPE	EQU 2
SPECIAL_MONITOR_TYPE	EQU 3
MONITOR_SPEC_TYPE	EQU 4

; Flags for AllocScreenBuffer()
SB_SCREEN_BITMAP	EQU 1
SB_COPY_BITMAP	EQU 2

; Various
EXTEND_VSTRUCT	EQU $1000

; Types of requesters known to ASL, used as arguments to AllocAslRequest()
ASL_FileRequest	EQU 0
ASL_FontRequest	EQU 1
ASL_ScreenModeRequest	EQU 2

; Flag bits for the ASLFR_Flags1 tag
FRB_FILTERFUNC	EQU 7
FRB_INTUIFUNC	EQU 6
FRB_DOSAVEMODE	EQU 5
FRB_PRIVATEIDCMP	EQU 4
FRB_DOMULTISELECT	EQU 3
FRB_DOPATTERNS	EQU 0

; Flag bits for the ASLFR_Flags2 tag
FRB_DRAWERSONLY	EQU 0
FRB_FILTERDRAWERS	EQU 1
FRB_REJECTICONS	EQU 2

;*****************************************************************************
; These are window flags

WFLG_SIZEGADGET	EQU $0001
WFLG_DRAGBAR	EQU $0002
WFLG_DEPTHGADGET	EQU $0004
WFLG_CLOSEGADGET	EQU $0008

WFLG_SIZEBRIGHT	EQU $0010
WFLG_SIZEBBOTTOM	EQU $0020

; Combinations of the WFLG_REFRESHBITS select the refresh type
WFLG_REFRESHBITS	EQU $00C0
WFLG_SMART_REFRESH	EQU $0000
WFLG_SIMPLE_REFRESH	EQU $0040
WFLG_SUPER_BITMAP	EQU $0080
WFLG_OTHER_REFRESH	EQU $00C0

WFLG_BACKDROP	EQU $0100

WFLG_REPORTMOUSE	EQU $0200

WFLG_GIMMEZEROZERO	EQU $0400

WFLG_BORDERLESS	EQU $0800

WFLG_ACTIVATE	EQU $1000

WFLG_RMBTRAP	EQU $00010000
WFLG_NOCAREREFRESH	EQU $00020000

WFLG_NW_EXTENDED	EQU $00040000

WFLG_NEWLOOKMENUS	EQU $00200000

; These flags are set only by Intuition.  YOU MAY NOT SET THEM YOURSELF!
WFLG_WINDOWACTIVE	EQU $2000
WFLG_INREQUEST	EQU $4000
WFLG_MENUSTATE	EQU $8000

WFLG_WINDOWREFRESH	EQU $01000000
WFLG_WBENCHWINDOW	EQU $02000000
WFLG_WINDOWTICKED	EQU $04000000

; V36 and higher flags to be set only by Intuition:
WFLG_VISITOR	EQU $08000000
WFLG_ZOOMED	EQU $10000000
WFLG_HASZOOM	EQU $20000000

;*****************************************************************************
; This is the Node structure
	rsreset
ln_Succ:	rs.l 1
ln_Pred:	rs.l 1
ln_Type:	rs.b 1
ln_Pri:	rs.b 1
ln_Name:	rs.l 1
ln_data_size:	rs.b 0

;*****************************************************************************
; This is the MinNode structure
	rsreset
mln_Succ:	rs.l 1
mln_Pred:	rs.l 1
mln_data_size:	rs.b 0

;*****************************************************************************
; This is the List structure
	rsreset
lh_Head:	rs.l 1
lh_Tail:	rs.l 1
lh_TailPred:	rs.l 1
lh_Type:	rs.b 1
l_pad:	rs.b 1
lh_data_size:	rs.b 0

;*****************************************************************************
; This is the MinList structure
	rsreset
mlh_Head:	rs.l 1
mlh_Tail:	rs.l 1
mlh_TailPred:	rs.l 1
mlh_data_size:	rs.b 0

;*****************************************************************************
; This is the Message port structure
	rsreset
	rs.b ln_data_size
mp_Flags:	rs.b 1
mp_Sigbit:	rs.b 1
mp_Sigtask:	rs.l 1
mp_MsgList:	rs.b lh_data_size
mp_data_size:	rs.b 0

PF_ACTION	EQU 3			; Mask
PA_SIGNAL	EQU 0

;*****************************************************************************
; This is the Message structure
	rsreset
	rs.b ln_data_size
mn_ReplyPort:	rs.l 1		; message reply port
mn_Length:	rs.w 1			; total message length, in bytes
	; (include the size of the Message structure in the length)
mn_data_size:	rs.b 0

;*****************************************************************************
; This is the SemaphoreRequest structure
	rsreset
sr_Link:	rs.b mln_data_size
sr_Waiter:	rs.l 1
sr_data_size:	rs.b 0

;*****************************************************************************
; This is the Signal Semaphore structure
	rsreset
sis_Link:	rs.b ln_data_size
sis_NestCount:	rs.w 1
sis_WaitQueue:	rs.b mlh_data_size
sis_MultipleLink:	rs.b sr_data_size
sis_Owner:	rs.l 1
sis_QueueCount:	rs.w 1
sis_data_size:	rs.b 0

;*****************************************************************************
; This is the IOStdReq structure
	rsreset
io_Message:	rs.b mn_data_size
io_Device:	rs.l 1				; device node pointer
io_Unit:	rs.l 1				; unit (driver private)
io_Command:	rs.w 1			; device command
io_Flags:	rs.b 1
io_Error:	rs.b 1				; error or warning num
io_Actual:	rs.l 1				; actual number of bytes transferred
io_Length:	rs.l 1				; requested number bytes transferred
io_Data:	rs.l 1				; points to data area
io_Offset:	rs.l 1				; offset for block structured devices
iostdreq_data_size:	rs.b 0

;*****************************************************************************
; This is the Library structure
	rsreset
lib_Node:	rs.b ln_data_size
lib_Flags:	rs.b 1
lib_pad:	rs.b 1
lib_NegSize:	rs.w 1
lib_PosSize:	rs.w 1
lib_Version:	rs.w 1
lib_Revision:	rs.w 1
lib_IdString:	rs.l 1
lib_Sum:	rs.l 1
lib_OpenCnt:	rs.w 1
lib_data_size:	rs.b 0

;*****************************************************************************
; This is the Task structure
	rsreset
tc_Node:	rs.b ln_data_size
tc_Flags:	rs.b 1
tc_State:	rs.b 1
tc_IDNestCnt:	rs.b 1		; intr disabled nesting
tc_TDNestCnt:	rs.b 1		; task disabled nesting
tc_SigAlloc:	rs.l 1		; sigs allocated
tc_SigWait:	rs.l 1		; sigs we are waiting for
tc_SigRecvd:	rs.l 1		; sigs we have received
tc_SigExcept:	rs.l 1		; sigs we will take excepts for
tc_TrapAlloc:	rs.w 1		; traps allocated
tc_TrapAble:	rs.w 1		; traps enabled
tc_ExceptData:	rs.l 1		; points to except data
tc_ExceptCode:	rs.l 1		; points to except code
tc_TrapData:	rs.l 1		; points to trap data
tc_TrapCode:	rs.l 1		; points to trap code
tc_SPReg:	rs.l 1			; stack pointer	   
tc_SPLower:	rs.l 1		; stack lower bound   
tc_SPUpper:	rs.l 1		; stack upper bound + 2*/
tc_Switch:	rs.l 1			; task losing CPU	 
tc_Launch:	rs.l 1			; task getting CPU 
tc_MemEntry:	rs.b lh_data_size	; Allocated memory. Freed by RemTask()
tc_UserData:	rs.l 1		; For use by the task; no restrictions!
tc_data_size:	rs.b 0

;*****************************************************************************
; This is the Process structure
	rsreset
pr_Task:	rs.b tc_data_size
pr_MsgPort:	rs.b mp_data_size	; This is BPTR address from DOS functions
pr_Pad:	rs.w 1			; Remaining variables on 4 byte boundaries
pr_SegList:	rs.l 1		; Array of seg lists used by this process
pr_StackSize:	rs.l 1		; Size of process stack in bytes
pr_GlobVec:	rs.l 1		; Global vector for this process (BCPL)
pr_TaskNum:	rs.l 1		; CLI task number of zero if not a CLI
pr_StackBase:	rs.l 1		; Ptr to high memory end of process stack
pr_Result2:	rs.l 1		; Value of secondary result from last call
pr_CurrentDir:	rs.l 1		; Lock associated with current directory
pr_CIS:		rs.l 1		; Current CLI Input Stream
pr_COS:		rs.l 1		; Current CLI Output Stream
pr_ConsoleTask:	rs.l 1		; Console handler process for current window
pr_FileSystemTask:	rs.l 1		; File handler process for current drive
pr_CLI:		rs.l 1		; pointer to CommandLineInterface
pr_ReturnAddr:	rs.l 1		; pointer to previous stack frame
pr_PktWait:	rs.l 1		; Function to be called when awaiting msg
pr_WindowPtr:	rs.l 1		; Window pointer for errors

;*****************************************************************************
; This is the ExtendedNode structure
	rsreset
xln_Succ:	rs.l 1
xln_Pred:	rs.l 1
xln_Type:	rs.b 1
xln_Pri:	rs.b 1
xln_Name:	rs.l 1
xln_Subsystem:	rs.b 1
xln_Subtype:	rs.b 1
xln_Library:	rs.l 1
xln_Init:	rs.l 1
xln_data_size:	rs.b 0

;*****************************************************************************
; This is the Rectangle structure
	rsreset
ra_MinX:	rs.w 1
ra_MinY:	rs.w 1
ra_MaxX:	rs.w 1
ra_MaxY:	rs.w 1
ra_data_size:	rs.b 0

;*****************************************************************************
; This is the Point structure
	rsreset
pt_x:	rs.w 1
pt_y:	rs.w 1
pt_data_size:	rs.b 0

;*****************************************************************************
; This is the QueryHeader structure
	rsreset
qh_StructID:	rs.l 1		; datachunk type identifier
qh_DisplayID:	rs.l 1		; copy of display record key
qh_SkipID:	rs.l 1			; TAG_SKIP -- see tagitems.h
qh_Length:	rs.l 1			; length of data in double-longwords
qh_data_size:	rs.b 0

;*****************************************************************************
; This is the DimensionInfo structure
	rsreset
	rs.b qh_data_size
dim_MaxDepth:	rs.w 1		; log2( max number of colors
dim_MinRasterWidth:	rs.w 1		; minimum width in pixels
dim_MinRasterHeight:	rs.w 1	; minimum height in pixels
dim_MaxRasterWidth:	rs.w 1		; maximum width in pixels
dim_MaxRasterHeight:	rs.w 1	; maximum height in pixels
dim_Nominal:	rs.b ra_data_size	; "standard" dimensions
dim_MaxOScan:	rs.b ra_data_size	; fixed, hardware dependent
dim_VideoOScan:	rs.b ra_data_size	; fixed, hardware dependent
dim_TxtOScan:	rs.b ra_data_size	; editable via preferences
dim_StdOScan:	rs.b ra_data_size	; editable via preferences
dim_pad:	rs.w 7
dim_reserved:	rs.w 4		; terminator
dim_data_size:	rs.b 0

;*****************************************************************************
; This is the ColorMap structure
	rsreset
cm_Flags:	rs.b 1
cm_Type:	rs.b 1
cm_Count:	rs.w 1
cm_ColorTable:	rs.l 1
cm_vpe:	rs.l 1
cm_LowColorBits:	rs.l 1
cm_TransparencyPlane:	rs.b 1
cm_SpriteResolution:	rs.b 1
cm_SpriteResDefault:	rs.b 1
cm_AuxFlags:	rs.b 1
cm_vp:	rs.l 1
cm_NormalDisplayInfo:	rs.l 1
cm_CoerceDisplayInfo:	rs.l 1
cm_batch_items:	rs.l 1
cm_VPModeID:	rs.l 1
cm_PalExtra:	rs.l 1
cm_SpriteBase_Even:	rs.w 1
cm_SpriteBase_Odd:	rs.w 1
cm_Bp_0_base:	rs.w 1
cm_Bp_1_base:	rs.w 1
cm_data_size:	rs.b 0

;*****************************************************************************
; This is the ViewPort structure
	rsreset
vp_Next:	rs.l 1
vp_ColorMap:	rs.l 1		; table of colors for this viewport
				; if this is nil, MakeVPort assumes
				; default values
vp_DspIns:	rs.l 1			; user by MakeView()
vp_SprIns:	rs.l 1			; used by sprite stuff
vp_ClrIns:	rs.l 1			; used by sprite stuff
vp_UCopIns:	rs.l 1		; User copper list
vp_DWidth:	rs.w 1
vp_DHeight:	rs.w 1
vp_DxOffset:	rs.w 1
vp_DyOffset:	rs.w 1
vp_Modes:	rs.w 1
vp_SpritePriorities:	rs.b 1	; used by makevp
vp_ExtendedModes:	rs.b 1
vp_RasInfo:	rs.l 1
vp_data_size:	rs.b 0

;*****************************************************************************
; This is the View structure
	rsreset
v_ViewPort:	rs.l 1
v_LOFCprList:	rs.l 1
v_SHFCprList:	rs.l 1
v_DyOffset:	rs.w 1
v_DxOffset:	rs.w 1
v_Modes:	rs.w 1
v_data_size:	rs.b 0

;*****************************************************************************
; This is the ViewExtra structure
	rsreset
	rs.b xln_data_size
ve_View:	rs.l 1
ve_Monitor:	rs.l 1
ve_TopLine:	rs.w 1
ve_data_size:	rs.b 0

;*****************************************************************************
; This is the ViewPortExtra structure
	rsreset
	rs.b xln_data_size
vpe_ViewPort:	rs.l 1
vpe_DisplayClip:	rs.b ra_data_size
vpe_VecTable:	rs.l 1
vpe_DriverData:	rs.w 4
vpe_Flags:	rs.w 1
vpe_Origin:	rs.b pt_data_size*2
vpe_cop1ptr:	rs.l 1
vpe_cop2ptr:	rs.l 1
vpe_data_size:	rs.b 0

;*****************************************************************************
; This is the BitMap structure
	rsreset
bm_BytesPerRow:	rs.w 1
bm_Rows:	rs.w 1
bm_Flags:	rs.b 1
bm_Depth:	rs.b 1
bm_Pad:	rs.w 1
bm_Planes:	rs.l 8
bm_data_size:	rs.b 0

;*****************************************************************************
; This is the RastPort structure
	rsreset
rp_Layer:	rs.l 1
rp_BitMap:	rs.l 1
rp_AreaPtrn:	rs.l 1
rp_TmpRas:	rs.l 1
rp_AreaInfo:	rs.l 1
rp_GelsInfo:	rs.l 1
rp_Mask:	rs.b 1
rp_FgPen:	rs.b 1
rp_BgPen:	rs.b 1
rp_AOLPen:	rs.b 1
rp_DrawMode:	rs.b 1
rp_AreaPtSz:	rs.b 1
rp_linpatcnt:	rs.b 1
rp_Dummy:	rs.b 1
rp_Flags:	rs.w 1
rp_LinePtrn:	rs.w 1
rp_cp_x:	rs.w 1
rp_cp_y:	rs.w 1
rp_minterms:	rs.w 4
rp_PenWidth:	rs.w 1
rp_PenHeight:	rs.w 1
rp_Font:	rs.l 1
rp_AlgoStyle:	rs.b 1
rp_TxFlags:	rs.b 1
rp_TxHeight:	rs.w 1
rp_TxWidth:	rs.w 1
rp_TxBaseline:	rs.w 1
rp_TxSpacing:	rs.w 1
rp_RP_User:	rs.l 1
rp_longreserved:	rs.w 4
rp_wordreserved:	rs.w 7
rp_reserved:	rs.w 4
rp_data_size:	rs.b 0

;*****************************************************************************
; This is the RasInfo structure
	rsreset
ri_Next:	rs.l 1	    		; used for dualpf
ri_BitMap:	rs.l 1
ri_RxOffset:	rs.w 1
ri_RyOffset:	rs.w 1
ri_data_size:	rs.b 0

;*****************************************************************************
; This is the LayerInfo structure
	rsreset
li_top_layer:	rs.l 1
li_check_lp:	rs.l 1		; !! Private !!
li_obs:	rs.l 1
li_FreeClipRects:	rs.l 1		; !! Private !!
li_PrivateReserve1:	rs.l 1		; !! Private !!
li_PrivateReserve2:	rs.l 1		; !! Private !!
li_Lock:	rs.b sis_data_size		; !! Private !!
li_gs_Head:	rs.b mlh_data_size	; !! Private !!
li_PrivateReserve3:	rs.w 1		; !! Private !!
li_PrivateReserve4:	rs.l 1		; !! Private !!
li_Flags:	rs.w 1
li_fatten_count:	rs.b 1		; !! Private !!
li_LockLayersCount:	rs.b 1		; !! Private !!
li_PrivateReserve5:	rs.w 1		; !! Private !!
li_BlankHook:	rs.l 1		; !! Private !!
li_LayerInfo_extra:	rs.l 1		; !! Private !!
li_data_size:	rs.b 0

;*****************************************************************************
; This is the Screen structure
	rsreset
sc_NextScreen:	rs.l 1		; linked list of screens
sc_FirstWindow:	rs.l 1		; linked list Screen's Windows
sc_LeftEdge:	rs.w 1		; parameters of the screen
sc_TopEdge:	rs.w 1		; parameters of the screen
sc_Width:	rs.w 1			; null-terminated Title text
sc_Height:	rs.w 1			; for Windows without ScreenTitle
sc_MouseY:	rs.w 1			; position relative to upper-left
sc_MouseX:	rs.w 1			; position relative to upper-left
sc_Flags:	rs.w 1			; see definitions below
sc_Title:	rs.l 1
sc_DefaultTitle:	rs.l 1
	; Bar sizes for this Screen and all Window's in this Screen
sc_BarHeight:	rs.b 1
sc_BarVBorder:	rs.b 1
sc_BarHBorder:	rs.b 1
sc_MenuVBorder:	rs.b 1
sc_MenuHBorder:	rs.b 1
sc_WBorTop:	rs.b 1
sc_WBorLeft:	rs.b 1
sc_WBorRight:	rs.b 1
sc_WBorBottom:	rs.b 1
	rseven
sc_Font:	rs.l 1			; this screen's default font
sc_ViewPort:	rs.b vp_data_size	; describing the Screen's display
sc_RastPort:	rs.b rp_data_size
sc_BitMap:	rs.b bm_data_size
sc_LayerInfo:	rs.b li_data_size
sc_FirstGadget:	rs.l 1
sc_DetailPen:	rs.b 1
sc_BlockPen:	rs.b 1
sc_SaveColor0:	rs.w 1
sc_BarLayer:	rs.l 1
sc_ExtData:	rs.l 1
sc_UserData:	rs.l 1
; Ends here but size should not be used

;*****************************************************************************
; This is the Window structure
	rsreset
wd_NextWindow:	rs.l 1		; for the linked list of a Screen

wd_LeftEdge:	rs.w 1		; screen dimensions
wd_TopEdge:	rs.w 1		; screen dimensions
wd_Width:	rs.w 1			; screen dimensions
wd_Height:	rs.w 1			; screen dimensions

wd_MouseY:	rs.w 1			; relative top top-left corner 
wd_MouseX:	rs.w 1			; relative top top-left corner 

wd_MinWidth:	rs.w 1		; minimum sizes
wd_MinHeight:	rs.w 1		; minimum sizes
wd_MaxWidth:	rs.w 1		; maximum sizes
wd_MaxHeight:	rs.w 1		; maximum sizes

wd_Flags:	rs.l 1			; see below for definitions

wd_MenuStrip:	rs.l 1		; first in a list of menu headers

wd_Title:	rs.l 1			; title text for the Window

wd_FirstRequest:	rs.l 1		; first in linked list of active Requesters 
wd_DMRequest:	rs.l 1		; the double-menu Requester 
wd_ReqCount:	rs.w 1		; number of Requesters blocking this Window
wd_WScreen:	rs.l 1		; this Window's Screen
wd_RPort:	rs.l 1			; this Window's very own RastPort

; the border variables describe the window border.  If you specify
; WFLG_GIMMEZEROZERO when you open the window, then the upper-left of the
; ClipRect for this window will be upper-left of the BitMap (with correct
; offsets when in SuperBitMap mode; you MUST select WFLG_GIMMEZEROZERO
; when using SuperBitMap).  If you don't specify ZeroZero, then you save
; memory (no allocation of RastPort, Layer, ClipRect and associated
; Bitmaps), but you also must offset all your writes by BorderTop,
; BorderLeft and do your own mini-clipping to prevent writing over the
; system gadgets
wd_BorderLeft:	rs.b 1
wd_BorderTop:	rs.b 1
wd_BorderRight:	rs.b 1
wd_BorderBottom:	rs.b 1
wd_BorderRPort:	rs.l 1

; You supply a linked-list of gadget that you want for your Window.
; This list DOES NOT include system Gadgets.  You get the standard
; window system Gadgets by setting flag-bits in the variable Flags (see
; the bit definitions below)
wd_FirstGadget:	rs.l 1

; these are for opening/closing the windows
wd_Parent:	rs.l 1
wd_Descendant:	rs.l 1

; sprite data information for your own Pointer
; set these AFTER you Open the Window by calling SetPointer()
wd_Pointer:	rs.l 1
wd_PtrHeight:	rs.b 1
wd_PtrWidth:	rs.b 1
wd_XOffset:	rs.b 1
wd_YOffset:	rs.b 1

; the IDCMP Flags and User's and Intuition's Message Ports
wd_IDCMPFlags:	rs.l 1
wd_UserPort:	rs.l 1
wd_WindowPort:	rs.l 1
wd_MessageKey:	rs.l 1

wd_DetailPen:	rs.b 1
wd_BlockPen:	rs.b 1

; the CheckMark is a pointer to the imagery that will be used when
; rendering MenuItems of this Window that want to be checkmarked
; if this is equal to NULL, you'll get the default imagery
wd_CheckMark:	rs.l 1

; if non-null, Screen title when Window is active 
wd_ScreenTitle:	rs.l 1

; These variables have the mouse coordinates relative to the 
; inner-Window of WFLG_GIMMEZEROZERO Windows.  This is compared with the
; MouseX and MouseY variables, which contain the mouse coordinates
; relative to the upper-left corner of the Window, WFLG_GIMMEZEROZERO
; notwithstanding
wd_GZZMouseX:	rs.w 1
wd_GZZMouseY:	rs.w 1

; these variables contain the width and height of the inner-Window of
; WFLG_GIMMEZEROZERO Windows
wd_GZZWidth:	rs.w 1
wd_GZZHeight:	rs.w 1

wd_ExtData:	rs.l 1

; general-purpose pointer to User data extension 
wd_UserData:	rs.l 1
wd_WLayer:	rs.l 1		; stash of Window.RPort->Layer

; NEW 1.2: need to keep track of the font that OpenWindow opened,
; in case user SetFont's into RastPort
wd_IFont:	rs.l 1

; (V36) another flag word (the Flags field is used up).
; At present, all flag values are system private.
; Until further notice, you may not change nor use this field.
wd_MoreFlags:	rs.l 1

; ----- subsequent fields are INTUITION PRIVATE ---

;*****************************************************************************
; This is the ScreenBuffer structure
	rsreset
sb_BitMap:	rs.l 1			; BitMap of this buffer
sb_DBufInfo:	rs.l 1		; DBufInfo for this buffer
sb_data_size:	rs.b 0

;*****************************************************************************
; This is the Exec Base structure
	rsreset
	rs.b lib_data_size
SoftVer:	rs.w 1			; Kickstart release number (obs.)
LowMemChkSum:	rs.w 1		; Checksum of 68000 trap vectors
ChkBase:	rs.l 1			; System base pointer complement
ColdCapture:	rs.l 1		; Coldstart soft capture vector
CoolCapture:	rs.l 1		; Coolstart soft capture vector
WarmCapture:	rs.l 1		; Warmstart soft capture vector
SysStkUpper:	rs.l 1		; System stack base   (upper bound)
SysStkLower:	rs.l 1		; Top of system stack (lower bound)
MaxLocMem:	rs.l 1			; Top of chip memory
DebugEntry:	rs.l 1		; Global debugger entry point
DebugData:	rs.l 1			; Global debugger data segment
AlertData:	rs.l 1			; Alert data segment
MaxExtMem:	rs.l 1			; Top of extended mem, or null if none
ChkSum:	rs.w 1			; For all of the above (minus 2)

; Interrupt Related
IntVector:	rs.l 16*3

; Dynamic System Variables

ThisTask:	rs.l 1			; Pointer to current task (readable)

IdleCount:	rs.l 1			; Idle counter
DispCount:	rs.l 1			; Dispatch counter
Quantum:	rs.w 1			; Time slice quantum
Elapsed:	rs.w 1			; Current quantum ticks
SysFlags:	rs.w 1			; Misc internal system flags
IDNestCnt:	rs.b 1			; Interrupt disable nesting count
TDNestCnt:	rs.b 1			; Task disable nesting count
AttnFlags:	rs.w 1			; Special attention flags (readable)

AttnResched:	rs.w 1		; Rescheduling attention
ResModules:	rs.l 1		; Resident module array pointer
TaskTrapCode:	rs.l 1
TaskExceptCode:	rs.l 1
TaskExitCode:	rs.l 1
TaskSigAlloc:	rs.l 1
TaskTrapAlloc:	rs.l 1

; System Lists (private!)

MemList:	rs.b lh_data_size
ResourceList:	rs.b lh_data_size
DeviceList:	rs.b lh_data_size
IntrList:	rs.b lh_data_size
LibList:	rs.b lh_data_size
PortList:	rs.b lh_data_size
TaskReady:	rs.b lh_data_size
TaskWait:	rs.b lh_data_size
SoftIntList:	rs.b 5*(lh_data_size+2)

; Other Globals

LastAlert:	rs.l 4

; These next two variables are provided to allow system developers to have
; a rough idea of the period of two externally controlled signals --
; the time between vertical blank interrupts and the external line rate
; (which is counted by CIA A's "time of day" clock).  In general these
; values will be 50 or 60, and may or may not track each other.  These
; values replace the obsolete AFB_PAL and AFB_50HZ flags.

VBlankFrequency:	rs.b 1		; Readable
PowerSupplyFrequency:	rs.b 1	; Readable
	; ...

;*****************************************************************************
; This is the Graphics Base structure
	rsreset
	rs.b lib_data_size
gb_ActiView:	rs.l 1		; struct *View
gb_copinit:	rs.l 1		; struct *copinit
				; Pointer to copper start up list
	; ...

;*****************************************************************************
; This is the TimeVal structure
	rsreset
tv_secs:	rs.l 1
tv_micro:	rs.l 1
tv_data_size:	rs.b 0

;*****************************************************************************
; This is the Input Event structure
	rsreset
ie_NextEvent:	rs.l 1		; Pointer to next event
ie_Class:	rs.b 1			; Input event class
ie_SubClass:	rs.b 1		; Optional subclass of the class
ie_Code:	rs.w 1			; Input event code
ie_Qualifier:	rs.w 1		; Qualifiers in effect for the event
ie_X:	rs.w 1			; Event position
ie_Y:	rs.w 1
ie_TimeStamp:	rs.b tv_data_size	; System tick at the event
ie_data_size:	rs.b 0

;*****************************************************************************
; This is the File Info Block structure
	rsreset
fib_DiskKey:	rs.l 1
fib_DirEntryType:	rs.l 1
fib_FileName:	rs.b 108
fib_Protection:	rs.l 1
fib_EntryType:	rs.l 1
fib_Size:	rs.l 1
fib_NumBlocks:	rs.l 1
ds_Days:	rs.l 1				; Date-stamp structure
ds_Minute:	rs.l 1
ds_Tick:	rs.l 1
fib_Comment:	rs.b 80
fib_Reserved:	rs.b 36
fib_data_size:	rs.b 0

;*****************************************************************************
; This is the Info-data structure
	rsreset
id_NumSoftErrors:	rs.l 1
id_UnitNumber:	rs.l 1
id_DiskState:	rs.l 1
id_NumBlocks:	rs.l 1
id_NumBlocksUsed:	rs.l 1
id_BytesPerBlock:	rs.l 1
id_DiskType:	rs.l 1
id_VolumeNode:	rs.l 1
id_InUse:	rs.l 1
id_data_size:

;*****************************************************************************
; This is the File handle structure
	rsreset
fh_Link:	rs.l 1
fh_Port:	rs.l 1
fh_Type:	rs.l 1
fh_Buf:	rs.l 1
fh_Pos:	rs.l 1
fh_End:	rs.l 1
fh_Func:	rs.l 1
fh_Func2:	rs.l 1
fh_Func3:	rs.l 1
fh_Args:	rs.l 1
fh_Arg2:	rs.l 1
fh_data_size:	rs.b 0

;*****************************************************************************
; This is the IntuiText structure
	rsreset
it_FrontPen:	rs.b 1		; the pen numbers for the rendering
it_BackPen:	rs.b 1
it_DrawMode:	rs.b 1		; the mode for rendering the text
	rseven
it_LeftEdge:	rs.w 1		; relative start location for the text
it_TopEdge:	rs.w 1		; relative start location for the text
it_ITextFont:	rs.l 1		; if NULL, you accept the default
it_IText:	rs.l 1			; pointer to null-terminated text
it_NextText:	rs.l 1		; pointer to another IntuiText to render
it_data_size:	rs.b 0

;*****************************************************************************
; This is the UCopList structure
	rsreset
ucl_Next:	rs.l 1
ucl_FirstCopList:	rs.l 1
ucl_CopList:	rs.l 1
ucl_data_size:	rs.b 0

;*****************************************************************************
; This is the DBufInfo structure
	rsreset
dbi_Link1:	rs.l 1
dbi_Count1:	rs.l 1
dbi_SafeMessage:	rs.b mn_data_size	; Replied to when safe to write to
				;  old bitmap
dbi_UserData1:	rs.l 1		; First user data

dbi_Link2:	rs.l 1
dbi_Count2:	rs.l 1
dbi_DispMessage:	rs.b mn_data_size	; Replied to when new bitmap has
				;  been displayed at least once
dbi_UserData2:	rs.l 1		; Second user data
dbi_MatchLong:	rs.l 1
dbi_CopPtr1:	rs.l 1
dbi_CopPtr2:	rs.l 1
dbi_CopPtr3:	rs.l 1
dbi_BeamPos1:	rs.w 1
dbi_BeamPos2:	rs.w 1
dbi_data_size:	rs.b 0

;*****************************************************************************
; This is the IntuiMessage structure
	rsreset
im_ExecMessage:	rs.b mn_data_size

; the Class bits correspond directly with the IDCMP Flags, except for the
; special bit IDCMP_LONELYMESSAGE (defined below)
im_Class:	rs.l 1

; the Code field is for special values like MENU number 
im_Code:	rs.w 1

; the Qualifier field is a copy of the current InputEvent's Qualifier
im_Qualifier:	rs.w 1

; IAddress contains particular addresses for Intuition functions, like
; the pointer to the Gadget or the Screen
im_IAddress:	rs.l 1

; when getting mouse movement reports, any event you get will have the
; the mouse coordinates in these variables.  the coordinates are relative
; to the upper-left corner of your Window (WFLG_GIMMEZEROZERO
; notwithstanding)
; If the DELTAMOVE IDCMP flag is set, these values will be deltas from
; the last reported position.
im_MouseX:	rs.w 1
im_MouseY:	rs.w 1

; the time values are copies of the current system clock time.  Micros
; are in units of microseconds, Seconds in seconds.
im_Seconds:	rs.l 1
im_Micros:	rs.l 1

; the IDCMPWindow variable will always have the address of the Window of
; this IDCMP
im_IDCMPWindow:	rs.l 1

; system-use variable
im_SpecialLink:	rs.l 1

im_data_size:	rs.b 0

;*****************************************************************************
; This is the SimpleSprite structure
	rsreset
ss_posctldata:	rs.l 1
ss_height:	rs.w 1
ss_x:	rs.w 1
ss_y:	rs.w 1
ss_num:	rs.w 1
ss_data_size:	rs.b 0
; ExtSprite data follows

;*****************************************************************************
; This is the FileRequester structure
	rsreset
fr_Reserved0:	rs.b 4
fr_File:	rs.l 1		; Contents of File gadget on exit
fr_Drawer:	rs.l 1		; Contents of Drawer gadget on exit
fr_Reserved1:	rs.b 10
fr_LeftEdge:	rs.w 1	; Coordinates of requester on exit
fr_TopEdge:	rs.w 1
fr_Width:	rs.w 1
fr_Height:	rs.w 1
fr_Reserved2:	rs.b 2
fr_NumArgs:	rs.l 1	; Number of files selected
fr_ArgList:	rs.l 1	; List of files selected
fr_UserData:	rs.l 1	; You can store your own data here
fr_Reserved3:	rs.b 8
fr_Pattern:	rs.l 1	; Contents of Pattern gadget on exit
fr_data_size:	rs.b 0

;*****************************************************************************
; These are OS macros
	
kickEXEC	macro
	move.l	a6,-(sp)
	move.l	Exec_base,a6
	jsr	_LVO\1(a6)
	move.l	(sp)+,a6
	endm

kickDOS	macro
	move.l	a6,-(sp)
	move.l	DOS_base,a6
	jsr	_LVO\1(a6)
	move.l	(sp)+,a6
	endm
	
kickGFX	macro
	move.l	a6,-(sp)
	move.l	Graphics_base,a6
	jsr	_LVO\1(a6)
	move.l	(sp)+,a6
	endm

kickINTU	macro
	move.l	a6,-(sp)
	move.l	Intuition_base,a6
	jsr	_LVO\1(a6)
	move.l	(sp)+,a6
	endm

kickASL	macro
	move.l	a6,-(sp)
	move.l	Asl_base,a6
	jsr	_LVO\1(a6)
	move.l	(sp)+,a6
	endm

kickCON	macro
	move.l	a6,-(sp)
	move.l	Console_base,a6
	jsr	\1(a6)
	move.l	(sp)+,a6
	endm

kickINP	macro
	move.l	a6,-(sp)
	move.l	Input_base,a6
	jsr	\1(a6)
	move.l	(sp)+,a6
	endm

Wait_4_blitter	macro
	kickGFX	WaitBlit			; Wait for blitter
	endm

;*****************************************************************************
; These are other OS constants

	include	OS_error_codes.i
	include	OS_tags.i

; These are OS library offsets

	include	Exec_lib.i
	include	Dos_lib.i
	include	Graphics_lib.i
	include	Intuition_lib.i
	include	Asl_lib.i
