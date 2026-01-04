# Makefile for Albion headers, objects and libraries
# PC CD-ROM version

# Ignore return values and do not check if target was made
.IGNORE

# Default user is Jurie
!ifndef User
User = Jurie
!endif


!ifeq User Jurie

# Paths for Jurie
Sdir = G:\ALBION
Sdir2 = G:\PCLIB32
Tdir = U:\PROJECT\ALBION\SWAP
Bdir = Q:\PROGRAM\RELAYER\RAINER

Back_files = $(Sdir)\SRC\COMMAGIC\COMMAG0.C &
	$(Sdir)\SRC\COMMAGIC\COMMAG1.C &
	$(Sdir)\SRC\COMMAGIC\COMMAG2.C &
	$(Sdir)\SRC\COMMAGIC\COMMAG3.C &
	$(Sdir)\SRC\COMMAGIC\COMMAG4.C &
	$(Sdir)\SRC\COMMAGIC\COMMSUB.C &
	$(Sdir)\LIB\COMMAGIC.H &
	$(Sdir)\LIB\COMMSUB.H

!else

# Paths for non-Juries
Sdir = U:\PROJECT\ALBION\SWAP
Sdir2 = U:\PROJECT\ALBION\SWAP
Tdir = D:\ALBION
Bdir = Q:\PROGRAM\RELAYER\RAINER

Back_files = $(Bdir)\SRC\COMMAGIC\COMMAG0.C &
	$(Bdir)\SRC\COMMAGIC\COMMAG1.C &
	$(Bdir)\SRC\COMMAGIC\COMMAG2.C &
	$(Bdir)\SRC\COMMAGIC\COMMAG3.C &
	$(Bdir)\SRC\COMMAGIC\COMMAG4.C &
	$(Bdir)\SRC\COMMAGIC\COMMSUB.C &
	$(Bdir)\LIB\COMMAGIC.H &
	$(Bdir)\LIB\COMMSUB.H

!endif


# Define header files
Header_files = $(Tdir)\LIB\2D_COLL.H &
	$(Tdir)\LIB\2D_DISPL.H &
	$(Tdir)\LIB\2D_MAP.H &
	$(Tdir)\LIB\2D_PATH.H &
	$(Tdir)\LIB\3DCOMMON.H &
	$(Tdir)\LIB\3DM.H &
	$(Tdir)\LIB\3D_MAP.H &
	$(Tdir)\LIB\3D_PREP.H &
	$(Tdir)\LIB\3D_MOVE.H &
	$(Tdir)\LIB\3D_SEL.H &
	$(Tdir)\LIB\ALBION.H &
	$(Tdir)\LIB\ANIMATE.H &
	$(Tdir)\LIB\APRES.H &
	$(Tdir)\LIB\AUTOMAP.H &
	$(Tdir)\LIB\BATFORM.H &
	$(Tdir)\LIB\BOOLREQ.H &
	$(Tdir)\LIB\BUTTONS.H &
	$(Tdir)\LIB\CAMP.H &
	$(Tdir)\LIB\CHEST.H &
	$(Tdir)\LIB\COLOURS.H &
	$(Tdir)\LIB\COMACTS.H &
	$(Tdir)\LIB\COMBAT.H &
	$(Tdir)\LIB\COMBVAR.H &
	$(Tdir)\LIB\COMMAGIC.H &
	$(Tdir)\LIB\COMMSUB.H &
	$(Tdir)\LIB\COMOBS.H &
	$(Tdir)\LIB\COMSHOW.H &
	$(Tdir)\LIB\CONTROL.H &
	$(Tdir)\LIB\DIAGNOST.H &
	$(Tdir)\LIB\DIALOGUE.H &
	$(Tdir)\LIB\DOOR.H &
	$(Tdir)\LIB\EVELOGIC.H &
	$(Tdir)\LIB\EVENTS.H &
	$(Tdir)\LIB\EXAMITEM.H &
	$(Tdir)\LIB\FASTFLC.H &
	$(Tdir)\LIB\FINDCOL.H &
	$(Tdir)\LIB\FLC.H &
	$(Tdir)\LIB\FLCVAR.H &
	$(Tdir)\LIB\FONT.H &
	$(Tdir)\LIB\GAME.H &
	$(Tdir)\LIB\GAMETEXT.H &
	$(Tdir)\LIB\GAMETIME.H &
	$(Tdir)\LIB\GAMEVAR.H &
	$(Tdir)\LIB\GFXFUNC.H &
	$(Tdir)\LIB\GOLDFOOD.H &
	$(Tdir)\LIB\GRAPHICS.H &
	$(Tdir)\LIB\HDOB.H &
	$(Tdir)\LIB\ICCHANGE.H &
	$(Tdir)\LIB\INPUT.H &
	$(Tdir)\LIB\INPUTNR.H &
	$(Tdir)\LIB\INVENTO.H &
	$(Tdir)\LIB\INVITEMS.H &
	$(Tdir)\LIB\INVLEFT.H &
	$(Tdir)\LIB\ITEMLIST.H &
	$(Tdir)\LIB\ITEMSEL.H &
	$(Tdir)\LIB\ITMLOGIC.H &
	$(Tdir)\LIB\MAGIC.H &
	$(Tdir)\LIB\MAIN.H &
	$(Tdir)\LIB\MAINMENU.H &
	$(Tdir)\LIB\MAP.H &
	$(Tdir)\LIB\MAP_PUM.H &
	$(Tdir)\LIB\MEMBRSEL.H &
	$(Tdir)\LIB\MERCHANT.H &
	$(Tdir)\LIB\MONDISP.H &
	$(Tdir)\LIB\MONLOGIC.H &
	$(Tdir)\LIB\MUSIC.H &
	$(Tdir)\LIB\NPCS.H &
	$(Tdir)\LIB\NPCSOUND.H &
	$(Tdir)\LIB\PLACES.H &
	$(Tdir)\LIB\POPUP.H &
	$(Tdir)\LIB\PRTLOGIC.H &
	$(Tdir)\LIB\QUERMOD.H &
	$(Tdir)\LIB\SAVELOAD.H &
	$(Tdir)\LIB\SCRIPT.H &
	$(Tdir)\LIB\SCROLBAR.H &
	$(Tdir)\LIB\SCROLL.H &
	$(Tdir)\LIB\SELWIN.H &
	$(Tdir)\LIB\SORT.H &
	$(Tdir)\LIB\SPECITEM.H &
	$(Tdir)\LIB\SPELLS.H &
	$(Tdir)\LIB\STATAREA.H &
	$(Tdir)\LIB\TACTICAL.H &
	$(Tdir)\LIB\TEXT.H &
	$(Tdir)\LIB\TEXTWIN.H &
	$(Tdir)\LIB\USERFACE.H &
	$(Tdir)\LIB\VERSION.H &
	$(Tdir)\LIB\XFTYPES.H &
	$(Tdir)\LIB\XLOAD.H &
	$(Tdir)\LIB\ZOOMFUNC.H &
	$(Tdir)\LIB\BBBASMEM.H &
	$(Tdir)\LIB\BBDEF.H &
	$(Tdir)\LIB\BBDOS.H &
	$(Tdir)\LIB\BBDSA.H &
	$(Tdir)\LIB\BBERROR.H &
	$(Tdir)\LIB\BBEVENT.H &
	$(Tdir)\LIB\BBEXTRDF.H &
	$(Tdir)\LIB\BBLBM.H &
	$(Tdir)\LIB\BBMEM.H &
	$(Tdir)\LIB\BBMISC.H &
	$(Tdir)\LIB\BBMTH.H &
	$(Tdir)\LIB\BBOPM.H &
	$(Tdir)\LIB\BBSYSTEM.H &
	$(Tdir)\LIB\PCBASE.H &
	$(Tdir)\LIB\PCLIB32.H


# Define object files
Object_files = $(Tdir)\LIB\2D_COLL.OBJ &
	$(Tdir)\LIB\2D_DISPL.OBJ &
	$(Tdir)\LIB\2D_MAP.OBJ &
	$(Tdir)\LIB\2D_PATH.OBJ &
	$(Tdir)\LIB\3D_MAP.OBJ &
	$(Tdir)\LIB\3D_PREP.OBJ &
	$(Tdir)\LIB\3D_MOVE.OBJ &
	$(Tdir)\LIB\3D_SEL.OBJ &
	$(Tdir)\LIB\ALBION.OBJ &
	$(Tdir)\LIB\ANIMATE.OBJ &
	$(Tdir)\LIB\APRES.OBJ &
	$(Tdir)\LIB\AUTOMAP.OBJ &
	$(Tdir)\LIB\BATFORM.OBJ &
	$(Tdir)\LIB\BOOLREQ.OBJ &
	$(Tdir)\LIB\BUTTONS.OBJ &
	$(Tdir)\LIB\CAMP.OBJ &
	$(Tdir)\LIB\CHEST.OBJ &
	$(Tdir)\LIB\COLOURS.OBJ &
	$(Tdir)\LIB\COMACTS.OBJ &
	$(Tdir)\LIB\COMBAT.OBJ &
	$(Tdir)\LIB\COMMAGIC.OBJ &
	$(Tdir)\LIB\COMOBS.OBJ &
	$(Tdir)\LIB\COMSHOW.OBJ &
	$(Tdir)\LIB\CONTROL.OBJ &
	$(Tdir)\LIB\DIAGNOST.OBJ &
	$(Tdir)\LIB\DIALOGUE.OBJ &
	$(Tdir)\LIB\DOOR.OBJ &
	$(Tdir)\LIB\EVELOGIC.OBJ &
	$(Tdir)\LIB\EVENTS.OBJ &
	$(Tdir)\LIB\EXAMITEM.OBJ &
	$(Tdir)\LIB\FASTFLC.OBJ &
	$(Tdir)\LIB\FINDCOL.OBJ &
	$(Tdir)\LIB\FLC.OBJ &
	$(Tdir)\LIB\FLCVAR.OBJ &
	$(Tdir)\LIB\FONT.OBJ &
	$(Tdir)\LIB\GAME.OBJ &
	$(Tdir)\LIB\GAMETEXT.OBJ &
	$(Tdir)\LIB\GAMETIME.OBJ &
	$(Tdir)\LIB\GFXFUNC.OBJ &
	$(Tdir)\LIB\GOLDFOOD.OBJ &
	$(Tdir)\LIB\GRAPHICS.OBJ &
	$(Tdir)\LIB\HDOB.OBJ &
	$(Tdir)\LIB\ICCHANGE.OBJ &
	$(Tdir)\LIB\INPUT.OBJ &
	$(Tdir)\LIB\INPUTNR.OBJ &
	$(Tdir)\LIB\INVENTO.OBJ &
	$(Tdir)\LIB\INVITEMS.OBJ &
	$(Tdir)\LIB\INVLEFT.OBJ &
	$(Tdir)\LIB\ITEMLIST.OBJ &
	$(Tdir)\LIB\ITEMSEL.OBJ &
	$(Tdir)\LIB\ITMLOGIC.OBJ &
	$(Tdir)\LIB\MAGIC.OBJ &
	$(Tdir)\LIB\MAIN.OBJ &
	$(Tdir)\LIB\MAINMENU.OBJ &
	$(Tdir)\LIB\MAP.OBJ &
	$(Tdir)\LIB\MAP_PUM.OBJ &
	$(Tdir)\LIB\MEMBRSEL.OBJ &
	$(Tdir)\LIB\MERCHANT.OBJ &
	$(Tdir)\LIB\MONLOGIC.OBJ &
	$(Tdir)\LIB\MUSIC.OBJ &
	$(Tdir)\LIB\NPCS.OBJ &
	$(Tdir)\LIB\NPCSOUND.OBJ &
	$(Tdir)\LIB\PLACES.OBJ &
	$(Tdir)\LIB\POPUP.OBJ &
	$(Tdir)\LIB\PRTLOGIC.OBJ &
	$(Tdir)\LIB\QUERMOD.OBJ &
	$(Tdir)\LIB\SAVELOAD.OBJ &
	$(Tdir)\LIB\SCRIPT.OBJ &
	$(Tdir)\LIB\SCROLBAR.OBJ &
	$(Tdir)\LIB\SCROLL.OBJ &
	$(Tdir)\LIB\SELWIN.OBJ &
	$(Tdir)\LIB\SORT.OBJ &
	$(Tdir)\LIB\SPECITEM.OBJ &
	$(Tdir)\LIB\SPELLS.OBJ &
	$(Tdir)\LIB\STATAREA.OBJ &
	$(Tdir)\LIB\TACTICAL.OBJ &
	$(Tdir)\LIB\TEXTWIN.OBJ &
	$(Tdir)\LIB\USERFACE.OBJ &
	$(Tdir)\LIB\VERSION.OBJ &
	$(Tdir)\LIB\XFTYPES.OBJ &
	$(Tdir)\LIB\XLOAD.OBJ &
	$(Tdir)\LIB\DLLLOAD.OBJ

Library_files = $(Tdir)\LIB\TEXT.LIB &
	$(Tdir)\LIB\ZOOMFUNC.LIB &
	$(Tdir)\LIB\3DM.LIB &
	$(Tdir)\LIB\AIL.LIB &
	$(Tdir)\LIB\BBMEM.LIB &
	$(Tdir)\LIB\PCBASE.LIB


Other_files = $(Tdir)\SRC\MAKEFILE


# This is the main "target"

ALL : $(Header_files) $(Object_files) $(Library_files) $(Other_files) $(Back_files)


# Copy the header files : */

$(Tdir)\LIB\2D_COLL.H : $(Sdir)\LIB\2D_COLL.H
	copy $(Sdir)\LIB\2D_COLL.H $(Tdir)\LIB\2D_COLL.H

$(Tdir)\LIB\2D_DISPL.H : $(Sdir)\LIB\2D_DISPL.H
	copy $(Sdir)\LIB\2D_DISPL.H $(Tdir)\LIB\2D_DISPL.H

$(Tdir)\LIB\2D_MAP.H : $(Sdir)\LIB\2D_MAP.H
	copy $(Sdir)\LIB\2D_MAP.H $(Tdir)\LIB\2D_MAP.H

$(Tdir)\LIB\2D_PATH.H : $(Sdir)\LIB\2D_PATH.H
	copy $(Sdir)\LIB\2D_PATH.H $(Tdir)\LIB\2D_PATH.H

$(Tdir)\LIB\3DCOMMON.H : $(Sdir)\LIB\3DCOMMON.H
	copy $(Sdir)\LIB\3DCOMMON.H $(Tdir)\LIB\3DCOMMON.H

$(Tdir)\LIB\3DM.H : $(Sdir)\LIB\3DM.H
	copy $(Sdir)\LIB\3DM.H $(Tdir)\LIB\3DM.H

$(Tdir)\LIB\3D_MAP.H : $(Sdir)\LIB\3D_MAP.H
	copy $(Sdir)\LIB\3D_MAP.H $(Tdir)\LIB\3D_MAP.H

$(Tdir)\LIB\3D_PREP.H : $(Sdir)\LIB\3D_PREP.H
	copy $(Sdir)\LIB\3D_PREP.H $(Tdir)\LIB\3D_PREP.H

$(Tdir)\LIB\3D_MOVE.H : $(Sdir)\LIB\3D_MOVE.H
	copy $(Sdir)\LIB\3D_MOVE.H $(Tdir)\LIB\3D_MOVE.H

$(Tdir)\LIB\3D_SEL.H : $(Sdir)\LIB\3D_SEL.H
	copy $(Sdir)\LIB\3D_SEL.H $(Tdir)\LIB\3D_SEL.H

$(Tdir)\LIB\ALBION.H : $(Sdir)\LIB\ALBION.H
	copy $(Sdir)\LIB\ALBION.H $(Tdir)\LIB\ALBION.H

$(Tdir)\LIB\ANIMATE.H : $(Sdir)\LIB\ANIMATE.H
	copy $(Sdir)\LIB\ANIMATE.H $(Tdir)\LIB\ANIMATE.H

$(Tdir)\LIB\APRES.H : $(Sdir)\LIB\APRES.H
	copy $(Sdir)\LIB\APRES.H $(Tdir)\LIB\APRES.H

$(Tdir)\LIB\AUTOMAP.H : $(Sdir)\LIB\AUTOMAP.H
	copy $(Sdir)\LIB\AUTOMAP.H $(Tdir)\LIB\AUTOMAP.H

$(Tdir)\LIB\BATFORM.H : $(Sdir)\LIB\BATFORM.H
	copy $(Sdir)\LIB\BATFORM.H $(Tdir)\LIB\BATFORM.H

$(Tdir)\LIB\BOOLREQ.H : $(Sdir)\LIB\BOOLREQ.H
	copy $(Sdir)\LIB\BOOLREQ.H $(Tdir)\LIB\BOOLREQ.H

$(Tdir)\LIB\BUTTONS.H : $(Sdir)\LIB\BUTTONS.H
	copy $(Sdir)\LIB\BUTTONS.H $(Tdir)\LIB\BUTTONS.H

$(Tdir)\LIB\CAMP.H : $(Sdir)\LIB\CAMP.H
	copy $(Sdir)\LIB\CAMP.H $(Tdir)\LIB\CAMP.H

$(Tdir)\LIB\CHEST.H : $(Sdir)\LIB\CHEST.H
	copy $(Sdir)\LIB\CHEST.H $(Tdir)\LIB\CHEST.H

$(Tdir)\LIB\COLOURS.H : $(Sdir)\LIB\COLOURS.H
	copy $(Sdir)\LIB\COLOURS.H $(Tdir)\LIB\COLOURS.H

$(Tdir)\LIB\COMACTS.H : $(Sdir)\LIB\COMACTS.H
	copy $(Sdir)\LIB\COMACTS.H $(Tdir)\LIB\COMACTS.H

$(Tdir)\LIB\COMBAT.H : $(Sdir)\LIB\COMBAT.H
	copy $(Sdir)\LIB\COMBAT.H $(Tdir)\LIB\COMBAT.H

$(Tdir)\LIB\COMBVAR.H : $(Sdir)\LIB\COMBVAR.H
	copy $(Sdir)\LIB\COMBVAR.H $(Tdir)\LIB\COMBVAR.H

$(Tdir)\LIB\COMMAGIC.H : $(Sdir)\LIB\COMMAGIC.H
	copy $(Sdir)\LIB\COMMAGIC.H $(Tdir)\LIB\COMMAGIC.H

$(Tdir)\LIB\COMMSUB.H : $(Sdir)\LIB\COMMSUB.H
	copy $(Sdir)\LIB\COMMSUB.H $(Tdir)\LIB\COMMSUB.H

$(Tdir)\LIB\COMOBS.H : $(Sdir)\LIB\COMOBS.H
	copy $(Sdir)\LIB\COMOBS.H $(Tdir)\LIB\COMOBS.H

$(Tdir)\LIB\COMSHOW.H : $(Sdir)\LIB\COMSHOW.H
	copy $(Sdir)\LIB\COMSHOW.H $(Tdir)\LIB\COMSHOW.H

$(Tdir)\LIB\CONTROL.H : $(Sdir)\LIB\CONTROL.H
	copy $(Sdir)\LIB\CONTROL.H $(Tdir)\LIB\CONTROL.H

$(Tdir)\LIB\DIAGNOST.H : $(Sdir)\LIB\DIAGNOST.H
	copy $(Sdir)\LIB\DIAGNOST.H $(Tdir)\LIB\DIAGNOST.H

$(Tdir)\LIB\DIALOGUE.H : $(Sdir)\LIB\DIALOGUE.H
	copy $(Sdir)\LIB\DIALOGUE.H $(Tdir)\LIB\DIALOGUE.H

$(Tdir)\LIB\DOOR.H : $(Sdir)\LIB\DOOR.H
	copy $(Sdir)\LIB\DOOR.H $(Tdir)\LIB\DOOR.H

$(Tdir)\LIB\EVELOGIC.H : $(Sdir)\LIB\EVELOGIC.H
	copy $(Sdir)\LIB\EVELOGIC.H $(Tdir)\LIB\EVELOGIC.H

$(Tdir)\LIB\EVENTS.H : $(Sdir)\LIB\EVENTS.H
	copy $(Sdir)\LIB\EVENTS.H $(Tdir)\LIB\EVENTS.H

$(Tdir)\LIB\EXAMITEM.H : $(Sdir)\LIB\EXAMITEM.H
	copy $(Sdir)\LIB\EXAMITEM.H $(Tdir)\LIB\EXAMITEM.H

$(Tdir)\LIB\FASTFLC.H : $(Sdir)\LIB\FASTFLC.H
	copy $(Sdir)\LIB\FASTFLC.H $(Tdir)\LIB\FASTFLC.H

$(Tdir)\LIB\FINDCOL.H : $(Sdir)\LIB\FINDCOL.H
	copy $(Sdir)\LIB\FINDCOL.H $(Tdir)\LIB\FINDCOL.H

$(Tdir)\LIB\FLC.H : $(Sdir)\LIB\FLC.H
	copy $(Sdir)\LIB\FLC.H $(Tdir)\LIB\FLC.H

$(Tdir)\LIB\FLCVAR.H : $(Sdir)\LIB\FLCVAR.H
	copy $(Sdir)\LIB\FLCVAR.H $(Tdir)\LIB\FLCVAR.H

$(Tdir)\LIB\FONT.H : $(Sdir)\LIB\FONT.H
	copy $(Sdir)\LIB\FONT.H $(Tdir)\LIB\FONT.H

$(Tdir)\LIB\GAME.H : $(Sdir)\LIB\GAME.H
	copy $(Sdir)\LIB\GAME.H $(Tdir)\LIB\GAME.H

$(Tdir)\LIB\GAMETEXT.H : $(Sdir)\LIB\GAMETEXT.H
	copy $(Sdir)\LIB\GAMETEXT.H $(Tdir)\LIB\GAMETEXT.H

$(Tdir)\LIB\GAMETIME.H : $(Sdir)\LIB\GAMETIME.H
	copy $(Sdir)\LIB\GAMETIME.H $(Tdir)\LIB\GAMETIME.H

$(Tdir)\LIB\GAMEVAR.H : $(Sdir)\LIB\GAMEVAR.H
	copy $(Sdir)\LIB\GAMEVAR.H $(Tdir)\LIB\GAMEVAR.H

$(Tdir)\LIB\GFXFUNC.H : $(Sdir)\LIB\GFXFUNC.H
	copy $(Sdir)\LIB\GFXFUNC.H $(Tdir)\LIB\GFXFUNC.H

$(Tdir)\LIB\GOLDFOOD.H : $(Sdir)\LIB\GOLDFOOD.H
	copy $(Sdir)\LIB\GOLDFOOD.H $(Tdir)\LIB\GOLDFOOD.H

$(Tdir)\LIB\GRAPHICS.H : $(Sdir)\LIB\GRAPHICS.H
	copy $(Sdir)\LIB\GRAPHICS.H $(Tdir)\LIB\GRAPHICS.H

$(Tdir)\LIB\HDOB.H : $(Sdir)\LIB\HDOB.H
	copy $(Sdir)\LIB\HDOB.H $(Tdir)\LIB\HDOB.H

$(Tdir)\LIB\ICCHANGE.H : $(Sdir)\LIB\ICCHANGE.H
	copy $(Sdir)\LIB\ICCHANGE.H $(Tdir)\LIB\ICCHANGE.H

$(Tdir)\LIB\INPUT.H : $(Sdir)\LIB\INPUT.H
	copy $(Sdir)\LIB\INPUT.H $(Tdir)\LIB\INPUT.H

$(Tdir)\LIB\INPUTNR.H : $(Sdir)\LIB\INPUTNR.H
	copy $(Sdir)\LIB\INPUTNR.H $(Tdir)\LIB\INPUTNR.H

$(Tdir)\LIB\INVENTO.H : $(Sdir)\LIB\INVENTO.H
	copy $(Sdir)\LIB\INVENTO.H $(Tdir)\LIB\INVENTO.H

$(Tdir)\LIB\INVITEMS.H : $(Sdir)\LIB\INVITEMS.H
	copy $(Sdir)\LIB\INVITEMS.H $(Tdir)\LIB\INVITEMS.H

$(Tdir)\LIB\INVLEFT.H : $(Sdir)\LIB\INVLEFT.H
	copy $(Sdir)\LIB\INVLEFT.H $(Tdir)\LIB\INVLEFT.H

$(Tdir)\LIB\ITEMLIST.H : $(Sdir)\LIB\ITEMLIST.H
	copy $(Sdir)\LIB\ITEMLIST.H $(Tdir)\LIB\ITEMLIST.H

$(Tdir)\LIB\ITEMSEL.H : $(Sdir)\LIB\ITEMSEL.H
	copy $(Sdir)\LIB\ITEMSEL.H $(Tdir)\LIB\ITEMSEL.H

$(Tdir)\LIB\ITMLOGIC.H : $(Sdir)\LIB\ITMLOGIC.H
	copy $(Sdir)\LIB\ITMLOGIC.H $(Tdir)\LIB\ITMLOGIC.H

$(Tdir)\LIB\MAGIC.H : $(Sdir)\LIB\MAGIC.H
	copy $(Sdir)\LIB\MAGIC.H $(Tdir)\LIB\MAGIC.H

$(Tdir)\LIB\MAIN.H : $(Sdir)\LIB\MAIN.H
	copy $(Sdir)\LIB\MAIN.H $(Tdir)\LIB\MAIN.H

$(Tdir)\LIB\MAINMENU.H : $(Sdir)\LIB\MAINMENU.H
	copy $(Sdir)\LIB\MAINMENU.H $(Tdir)\LIB\MAINMENU.H

$(Tdir)\LIB\MAP.H : $(Sdir)\LIB\MAP.H
	copy $(Sdir)\LIB\MAP.H $(Tdir)\LIB\MAP.H

$(Tdir)\LIB\MAP_PUM.H : $(Sdir)\LIB\MAP_PUM.H
	copy $(Sdir)\LIB\MAP_PUM.H $(Tdir)\LIB\MAP_PUM.H

$(Tdir)\LIB\MEMBRSEL.H : $(Sdir)\LIB\MEMBRSEL.H
	copy $(Sdir)\LIB\MEMBRSEL.H $(Tdir)\LIB\MEMBRSEL.H

$(Tdir)\LIB\MERCHANT.H : $(Sdir)\LIB\MERCHANT.H
	copy $(Sdir)\LIB\MERCHANT.H $(Tdir)\LIB\MERCHANT.H

$(Tdir)\LIB\MONDISP.H : $(Sdir)\LIB\MONDISP.H
	copy $(Sdir)\LIB\MONDISP.H $(Tdir)\LIB\MONDISP.H

$(Tdir)\LIB\MONLOGIC.H : $(Sdir)\LIB\MONLOGIC.H
	copy $(Sdir)\LIB\MONLOGIC.H $(Tdir)\LIB\MONLOGIC.H

$(Tdir)\LIB\MUSIC.H : $(Sdir)\LIB\MUSIC.H
	copy $(Sdir)\LIB\MUSIC.H $(Tdir)\LIB\MUSIC.H

$(Tdir)\LIB\NPCS.H : $(Sdir)\LIB\NPCS.H
	copy $(Sdir)\LIB\NPCS.H $(Tdir)\LIB\NPCS.H

$(Tdir)\LIB\NPCSOUND.H : $(Sdir)\LIB\NPCSOUND.H
	copy $(Sdir)\LIB\NPCSOUND.H $(Tdir)\LIB\NPCSOUND.H

$(Tdir)\LIB\PLACES.H : $(Sdir)\LIB\PLACES.H
	copy $(Sdir)\LIB\PLACES.H $(Tdir)\LIB\PLACES.H

$(Tdir)\LIB\POPUP.H : $(Sdir)\LIB\POPUP.H
	copy $(Sdir)\LIB\POPUP.H $(Tdir)\LIB\POPUP.H

$(Tdir)\LIB\PRTLOGIC.H : $(Sdir)\LIB\PRTLOGIC.H
	copy $(Sdir)\LIB\PRTLOGIC.H $(Tdir)\LIB\PRTLOGIC.H

$(Tdir)\LIB\QUERMOD.H : $(Sdir)\LIB\QUERMOD.H
	copy $(Sdir)\LIB\QUERMOD.H $(Tdir)\LIB\QUERMOD.H

$(Tdir)\LIB\SAVELOAD.H : $(Sdir)\LIB\SAVELOAD.H
	copy $(Sdir)\LIB\SAVELOAD.H $(Tdir)\LIB\SAVELOAD.H

$(Tdir)\LIB\SCRIPT.H : $(Sdir)\LIB\SCRIPT.H
	copy $(Sdir)\LIB\SCRIPT.H $(Tdir)\LIB\SCRIPT.H

$(Tdir)\LIB\SCROLBAR.H : $(Sdir)\LIB\SCROLBAR.H
	copy $(Sdir)\LIB\SCROLBAR.H $(Tdir)\LIB\SCROLBAR.H

$(Tdir)\LIB\SCROLL.H : $(Sdir)\LIB\SCROLL.H
	copy $(Sdir)\LIB\SCROLL.H $(Tdir)\LIB\SCROLL.H

$(Tdir)\LIB\SELWIN.H : $(Sdir)\LIB\SELWIN.H
	copy $(Sdir)\LIB\SELWIN.H $(Tdir)\LIB\SELWIN.H

$(Tdir)\LIB\SORT.H : $(Sdir)\LIB\SORT.H
	copy $(Sdir)\LIB\SORT.H $(Tdir)\LIB\SORT.H

$(Tdir)\LIB\SPECITEM.H : $(Sdir)\LIB\SPECITEM.H
	copy $(Sdir)\LIB\SPECITEM.H $(Tdir)\LIB\SPECITEM.H

$(Tdir)\LIB\SPELLS.H : $(Sdir)\LIB\SPELLS.H
	copy $(Sdir)\LIB\SPELLS.H $(Tdir)\LIB\SPELLS.H

$(Tdir)\LIB\STATAREA.H : $(Sdir)\LIB\STATAREA.H
	copy $(Sdir)\LIB\STATAREA.H $(Tdir)\LIB\STATAREA.H

$(Tdir)\LIB\TACTICAL.H : $(Sdir)\LIB\TACTICAL.H
	copy $(Sdir)\LIB\TACTICAL.H $(Tdir)\LIB\TACTICAL.H

$(Tdir)\LIB\TEXT.H : $(Sdir)\LIB\TEXT.H
	copy $(Sdir)\LIB\TEXT.H $(Tdir)\LIB\TEXT.H

$(Tdir)\LIB\TEXTWIN.H : $(Sdir)\LIB\TEXTWIN.H
	copy $(Sdir)\LIB\TEXTWIN.H $(Tdir)\LIB\TEXTWIN.H

$(Tdir)\LIB\USERFACE.H : $(Sdir)\LIB\USERFACE.H
	copy $(Sdir)\LIB\USERFACE.H $(Tdir)\LIB\USERFACE.H

$(Tdir)\LIB\VERSION.H : $(Sdir)\LIB\VERSION.H
	copy $(Sdir)\LIB\VERSION.H $(Tdir)\LIB\VERSION.H

$(Tdir)\LIB\XFTYPES.H : $(Sdir)\LIB\XFTYPES.H
	copy $(Sdir)\LIB\XFTYPES.H $(Tdir)\LIB\XFTYPES.H

$(Tdir)\LIB\XLOAD.H : $(Sdir)\LIB\XLOAD.H
	copy $(Sdir)\LIB\XLOAD.H $(Tdir)\LIB\XLOAD.H

$(Tdir)\LIB\ZOOMFUNC.H : $(Sdir)\LIB\ZOOMFUNC.H
	copy $(Sdir)\LIB\ZOOMFUNC.H $(Tdir)\LIB\ZOOMFUNC.H


# Copy the PCLIB32 files :

$(Tdir)\LIB\BBBASMEM.H : $(Sdir2)\LIB\BBBASMEM.H
	copy $(Sdir2)\LIB\BBBASMEM.H $(Tdir)\LIB\BBBASMEM.H

$(Tdir)\LIB\BBDEF.H : $(Sdir2)\LIB\BBDEF.H
	copy $(Sdir2)\LIB\BBDEF.H $(Tdir)\LIB\BBDEF.H

$(Tdir)\LIB\BBDOS.H : $(Sdir2)\LIB\BBDOS.H
	copy $(Sdir2)\LIB\BBDOS.H $(Tdir)\LIB\BBDOS.H

$(Tdir)\LIB\BBDSA.H : $(Sdir2)\LIB\BBDSA.H
	copy $(Sdir2)\LIB\BBDSA.H $(Tdir)\LIB\BBDSA.H

$(Tdir)\LIB\BBERROR.H : $(Sdir2)\LIB\BBERROR.H
	copy $(Sdir2)\LIB\BBERROR.H $(Tdir)\LIB\BBERROR.H

$(Tdir)\LIB\BBEVENT.H : $(Sdir2)\LIB\BBEVENT.H
	copy $(Sdir2)\LIB\BBEVENT.H $(Tdir)\LIB\BBEVENT.H

$(Tdir)\LIB\BBEXTRDF.H : $(Sdir2)\LIB\BBEXTRDF.H
	copy $(Sdir2)\LIB\BBEXTRDF.H $(Tdir)\LIB\BBEXTRDF.H

$(Tdir)\LIB\BBLBM.H : $(Sdir2)\LIB\BBLBM.H
	copy $(Sdir2)\LIB\BBLBM.H $(Tdir)\LIB\BBLBM.H

$(Tdir)\LIB\BBMEM.H : $(Sdir2)\LIB\BBMEM.H
	copy $(Sdir2)\LIB\BBMEM.H $(Tdir)\LIB\BBMEM.H

$(Tdir)\LIB\BBMISC.H : $(Sdir2)\LIB\BBMISC.H
	copy $(Sdir2)\LIB\BBMISC.H $(Tdir)\LIB\BBMISC.H

$(Tdir)\LIB\BBMTH.H : $(Sdir2)\LIB\BBMTH.H
	copy $(Sdir2)\LIB\BBMTH.H $(Tdir)\LIB\BBMTH.H

$(Tdir)\LIB\BBOPM.H : $(Sdir2)\LIB\BBOPM.H
	copy $(Sdir2)\LIB\BBOPM.H $(Tdir)\LIB\BBOPM.H

$(Tdir)\LIB\BBSYSTEM.H : $(Sdir2)\LIB\BBSYSTEM.H
	copy $(Sdir2)\LIB\BBSYSTEM.H $(Tdir)\LIB\BBSYSTEM.H

$(Tdir)\LIB\PCBASE.H : $(Sdir2)\LIB\PCBASE.H
	copy $(Sdir2)\LIB\PCBASE.H $(Tdir)\LIB\PCBASE.H

$(Tdir)\LIB\PCLIB32.H : $(Sdir2)\LIB\PCLIB32.H
	copy $(Sdir2)\LIB\PCLIB32.H $(Tdir)\LIB\PCLIB32.H


# Copy the object files : */

$(Tdir)\LIB\2D_COLL.OBJ : $(Sdir)\LIB\2D_COLL.OBJ
	copy $(Sdir)\LIB\2D_COLL.OBJ $(Tdir)\LIB\2D_COLL.OBJ

$(Tdir)\LIB\2D_DISPL.OBJ : $(Sdir)\LIB\2D_DISPL.OBJ
	copy $(Sdir)\LIB\2D_DISPL.OBJ $(Tdir)\LIB\2D_DISPL.OBJ

$(Tdir)\LIB\2D_MAP.OBJ : $(Sdir)\LIB\2D_MAP.OBJ
	copy $(Sdir)\LIB\2D_MAP.OBJ $(Tdir)\LIB\2D_MAP.OBJ

$(Tdir)\LIB\2D_PATH.OBJ : $(Sdir)\LIB\2D_PATH.OBJ
	copy $(Sdir)\LIB\2D_PATH.OBJ $(Tdir)\LIB\2D_PATH.OBJ

$(Tdir)\LIB\3D_MAP.OBJ : $(Sdir)\LIB\3D_MAP.OBJ
	copy $(Sdir)\LIB\3D_MAP.OBJ $(Tdir)\LIB\3D_MAP.OBJ

$(Tdir)\LIB\3D_PREP.OBJ : $(Sdir)\LIB\3D_PREP.OBJ
	copy $(Sdir)\LIB\3D_PREP.OBJ $(Tdir)\LIB\3D_PREP.OBJ

$(Tdir)\LIB\3D_MOVE.OBJ : $(Sdir)\LIB\3D_MOVE.OBJ
	copy $(Sdir)\LIB\3D_MOVE.OBJ $(Tdir)\LIB\3D_MOVE.OBJ

$(Tdir)\LIB\3D_SEL.OBJ : $(Sdir)\LIB\3D_SEL.OBJ
	copy $(Sdir)\LIB\3D_SEL.OBJ $(Tdir)\LIB\3D_SEL.OBJ

$(Tdir)\LIB\ALBION.OBJ : $(Sdir)\LIB\ALBION.OBJ
	copy $(Sdir)\LIB\ALBION.OBJ $(Tdir)\LIB\ALBION.OBJ

$(Tdir)\LIB\ANIMATE.OBJ : $(Sdir)\LIB\ANIMATE.OBJ
	copy $(Sdir)\LIB\ANIMATE.OBJ $(Tdir)\LIB\ANIMATE.OBJ

$(Tdir)\LIB\APRES.OBJ : $(Sdir)\LIB\APRES.OBJ
	copy $(Sdir)\LIB\APRES.OBJ $(Tdir)\LIB\APRES.OBJ

$(Tdir)\LIB\AUTOMAP.OBJ : $(Sdir)\LIB\AUTOMAP.OBJ
	copy $(Sdir)\LIB\AUTOMAP.OBJ $(Tdir)\LIB\AUTOMAP.OBJ

$(Tdir)\LIB\BATFORM.OBJ : $(Sdir)\LIB\BATFORM.OBJ
	copy $(Sdir)\LIB\BATFORM.OBJ $(Tdir)\LIB\BATFORM.OBJ

$(Tdir)\LIB\BOOLREQ.OBJ : $(Sdir)\LIB\BOOLREQ.OBJ
	copy $(Sdir)\LIB\BOOLREQ.OBJ $(Tdir)\LIB\BOOLREQ.OBJ

$(Tdir)\LIB\BUTTONS.OBJ : $(Sdir)\LIB\BUTTONS.OBJ
	copy $(Sdir)\LIB\BUTTONS.OBJ $(Tdir)\LIB\BUTTONS.OBJ

$(Tdir)\LIB\CAMP.OBJ : $(Sdir)\LIB\CAMP.OBJ
	copy $(Sdir)\LIB\CAMP.OBJ $(Tdir)\LIB\CAMP.OBJ

$(Tdir)\LIB\CHEST.OBJ : $(Sdir)\LIB\CHEST.OBJ
	copy $(Sdir)\LIB\CHEST.OBJ $(Tdir)\LIB\CHEST.OBJ

$(Tdir)\LIB\COLOURS.OBJ : $(Sdir)\LIB\COLOURS.OBJ
	copy $(Sdir)\LIB\COLOURS.OBJ $(Tdir)\LIB\COLOURS.OBJ

$(Tdir)\LIB\COMACTS.OBJ : $(Sdir)\LIB\COMACTS.OBJ
	copy $(Sdir)\LIB\COMACTS.OBJ $(Tdir)\LIB\COMACTS.OBJ

$(Tdir)\LIB\COMBAT.OBJ : $(Sdir)\LIB\COMBAT.OBJ
	copy $(Sdir)\LIB\COMBAT.OBJ $(Tdir)\LIB\COMBAT.OBJ

$(Tdir)\LIB\COMMAGIC.OBJ : $(Sdir)\LIB\COMMAGIC.OBJ
	copy $(Sdir)\LIB\COMMAGIC.OBJ $(Tdir)\LIB\COMMAGIC.OBJ

$(Tdir)\LIB\COMOBS.OBJ : $(Sdir)\LIB\COMOBS.OBJ
	copy $(Sdir)\LIB\COMOBS.OBJ $(Tdir)\LIB\COMOBS.OBJ

$(Tdir)\LIB\COMSHOW.OBJ : $(Sdir)\LIB\COMSHOW.OBJ
	copy $(Sdir)\LIB\COMSHOW.OBJ $(Tdir)\LIB\COMSHOW.OBJ

$(Tdir)\LIB\CONTROL.OBJ : $(Sdir)\LIB\CONTROL.OBJ
	copy $(Sdir)\LIB\CONTROL.OBJ $(Tdir)\LIB\CONTROL.OBJ

$(Tdir)\LIB\DIAGNOST.OBJ : $(Sdir)\LIB\DIAGNOST.OBJ
	copy $(Sdir)\LIB\DIAGNOST.OBJ $(Tdir)\LIB\DIAGNOST.OBJ

$(Tdir)\LIB\DIALOGUE.OBJ : $(Sdir)\LIB\DIALOGUE.OBJ
	copy $(Sdir)\LIB\DIALOGUE.OBJ $(Tdir)\LIB\DIALOGUE.OBJ

$(Tdir)\LIB\DOOR.OBJ : $(Sdir)\LIB\DOOR.OBJ
	copy $(Sdir)\LIB\DOOR.OBJ $(Tdir)\LIB\DOOR.OBJ

$(Tdir)\LIB\EVELOGIC.OBJ : $(Sdir)\LIB\EVELOGIC.OBJ
	copy $(Sdir)\LIB\EVELOGIC.OBJ $(Tdir)\LIB\EVELOGIC.OBJ

$(Tdir)\LIB\EVENTS.OBJ : $(Sdir)\LIB\EVENTS.OBJ
	copy $(Sdir)\LIB\EVENTS.OBJ $(Tdir)\LIB\EVENTS.OBJ

$(Tdir)\LIB\EXAMITEM.OBJ : $(Sdir)\LIB\EXAMITEM.OBJ
	copy $(Sdir)\LIB\EXAMITEM.OBJ $(Tdir)\LIB\EXAMITEM.OBJ

$(Tdir)\LIB\FASTFLC.OBJ : $(Sdir)\LIB\FASTFLC.OBJ
	copy $(Sdir)\LIB\FASTFLC.OBJ $(Tdir)\LIB\FASTFLC.OBJ

$(Tdir)\LIB\FINDCOL.OBJ : $(Sdir)\LIB\FINDCOL.OBJ
	copy $(Sdir)\LIB\FINDCOL.OBJ $(Tdir)\LIB\FINDCOL.OBJ

$(Tdir)\LIB\FLC.OBJ : $(Sdir)\LIB\FLC.OBJ
	copy $(Sdir)\LIB\FLC.OBJ $(Tdir)\LIB\FLC.OBJ

$(Tdir)\LIB\FLCVAR.OBJ : $(Sdir)\LIB\FLCVAR.OBJ
	copy $(Sdir)\LIB\FLCVAR.OBJ $(Tdir)\LIB\FLCVAR.OBJ

$(Tdir)\LIB\FONT.OBJ : $(Sdir)\LIB\FONT.OBJ
	copy $(Sdir)\LIB\FONT.OBJ $(Tdir)\LIB\FONT.OBJ

$(Tdir)\LIB\GAME.OBJ : $(Sdir)\LIB\GAME.OBJ
	copy $(Sdir)\LIB\GAME.OBJ $(Tdir)\LIB\GAME.OBJ

$(Tdir)\LIB\GAMETEXT.OBJ : $(Sdir)\LIB\GAMETEXT.OBJ
	copy $(Sdir)\LIB\GAMETEXT.OBJ $(Tdir)\LIB\GAMETEXT.OBJ

$(Tdir)\LIB\GAMETIME.OBJ : $(Sdir)\LIB\GAMETIME.OBJ
	copy $(Sdir)\LIB\GAMETIME.OBJ $(Tdir)\LIB\GAMETIME.OBJ

$(Tdir)\LIB\GFXFUNC.OBJ : $(Sdir)\LIB\GFXFUNC.OBJ
	copy $(Sdir)\LIB\GFXFUNC.OBJ $(Tdir)\LIB\GFXFUNC.OBJ

$(Tdir)\LIB\GOLDFOOD.OBJ : $(Sdir)\LIB\GOLDFOOD.OBJ
	copy $(Sdir)\LIB\GOLDFOOD.OBJ $(Tdir)\LIB\GOLDFOOD.OBJ

$(Tdir)\LIB\GRAPHICS.OBJ : $(Sdir)\LIB\GRAPHICS.OBJ
	copy $(Sdir)\LIB\GRAPHICS.OBJ $(Tdir)\LIB\GRAPHICS.OBJ

$(Tdir)\LIB\HDOB.OBJ : $(Sdir)\LIB\HDOB.OBJ
	copy $(Sdir)\LIB\HDOB.OBJ $(Tdir)\LIB\HDOB.OBJ

$(Tdir)\LIB\ICCHANGE.OBJ : $(Sdir)\LIB\ICCHANGE.OBJ
	copy $(Sdir)\LIB\ICCHANGE.OBJ $(Tdir)\LIB\ICCHANGE.OBJ

$(Tdir)\LIB\INPUT.OBJ : $(Sdir)\LIB\INPUT.OBJ
	copy $(Sdir)\LIB\INPUT.OBJ $(Tdir)\LIB\INPUT.OBJ

$(Tdir)\LIB\INPUTNR.OBJ : $(Sdir)\LIB\INPUTNR.OBJ
	copy $(Sdir)\LIB\INPUTNR.OBJ $(Tdir)\LIB\INPUTNR.OBJ

$(Tdir)\LIB\INVENTO.OBJ : $(Sdir)\LIB\INVENTO.OBJ
	copy $(Sdir)\LIB\INVENTO.OBJ $(Tdir)\LIB\INVENTO.OBJ

$(Tdir)\LIB\INVITEMS.OBJ : $(Sdir)\LIB\INVITEMS.OBJ
	copy $(Sdir)\LIB\INVITEMS.OBJ $(Tdir)\LIB\INVITEMS.OBJ

$(Tdir)\LIB\INVLEFT.OBJ : $(Sdir)\LIB\INVLEFT.OBJ
	copy $(Sdir)\LIB\INVLEFT.OBJ $(Tdir)\LIB\INVLEFT.OBJ

$(Tdir)\LIB\ITEMLIST.OBJ : $(Sdir)\LIB\ITEMLIST.OBJ
	copy $(Sdir)\LIB\ITEMLIST.OBJ $(Tdir)\LIB\ITEMLIST.OBJ

$(Tdir)\LIB\ITEMSEL.OBJ : $(Sdir)\LIB\ITEMSEL.OBJ
	copy $(Sdir)\LIB\ITEMSEL.OBJ $(Tdir)\LIB\ITEMSEL.OBJ

$(Tdir)\LIB\ITMLOGIC.OBJ : $(Sdir)\LIB\ITMLOGIC.OBJ
	copy $(Sdir)\LIB\ITMLOGIC.OBJ $(Tdir)\LIB\ITMLOGIC.OBJ

$(Tdir)\LIB\MAGIC.OBJ : $(Sdir)\LIB\MAGIC.OBJ
	copy $(Sdir)\LIB\MAGIC.OBJ $(Tdir)\LIB\MAGIC.OBJ

$(Tdir)\LIB\MAIN.OBJ : $(Sdir)\LIB\MAIN.OBJ
	copy $(Sdir)\LIB\MAIN.OBJ $(Tdir)\LIB\MAIN.OBJ

$(Tdir)\LIB\MAINMENU.OBJ : $(Sdir)\LIB\MAINMENU.OBJ
	copy $(Sdir)\LIB\MAINMENU.OBJ $(Tdir)\LIB\MAINMENU.OBJ

$(Tdir)\LIB\MAP.OBJ : $(Sdir)\LIB\MAP.OBJ
	copy $(Sdir)\LIB\MAP.OBJ $(Tdir)\LIB\MAP.OBJ

$(Tdir)\LIB\MAP_PUM.OBJ : $(Sdir)\LIB\MAP_PUM.OBJ
	copy $(Sdir)\LIB\MAP_PUM.OBJ $(Tdir)\LIB\MAP_PUM.OBJ

$(Tdir)\LIB\MEMBRSEL.OBJ : $(Sdir)\LIB\MEMBRSEL.OBJ
	copy $(Sdir)\LIB\MEMBRSEL.OBJ $(Tdir)\LIB\MEMBRSEL.OBJ

$(Tdir)\LIB\MERCHANT.OBJ : $(Sdir)\LIB\MERCHANT.OBJ
	copy $(Sdir)\LIB\MERCHANT.OBJ $(Tdir)\LIB\MERCHANT.OBJ

$(Tdir)\LIB\MONLOGIC.OBJ : $(Sdir)\LIB\MONLOGIC.OBJ
	copy $(Sdir)\LIB\MONLOGIC.OBJ $(Tdir)\LIB\MONLOGIC.OBJ

$(Tdir)\LIB\MUSIC.OBJ : $(Sdir)\LIB\MUSIC.OBJ
	copy $(Sdir)\LIB\MUSIC.OBJ $(Tdir)\LIB\MUSIC.OBJ

$(Tdir)\LIB\NPCS.OBJ : $(Sdir)\LIB\NPCS.OBJ
	copy $(Sdir)\LIB\NPCS.OBJ $(Tdir)\LIB\NPCS.OBJ

$(Tdir)\LIB\NPCSOUND.OBJ : $(Sdir)\LIB\NPCSOUND.OBJ
	copy $(Sdir)\LIB\NPCSOUND.OBJ $(Tdir)\LIB\NPCSOUND.OBJ

$(Tdir)\LIB\PLACES.OBJ : $(Sdir)\LIB\PLACES.OBJ
	copy $(Sdir)\LIB\PLACES.OBJ $(Tdir)\LIB\PLACES.OBJ

$(Tdir)\LIB\POPUP.OBJ : $(Sdir)\LIB\POPUP.OBJ
	copy $(Sdir)\LIB\POPUP.OBJ $(Tdir)\LIB\POPUP.OBJ

$(Tdir)\LIB\PRTLOGIC.OBJ : $(Sdir)\LIB\PRTLOGIC.OBJ
	copy $(Sdir)\LIB\PRTLOGIC.OBJ $(Tdir)\LIB\PRTLOGIC.OBJ

$(Tdir)\LIB\QUERMOD.OBJ : $(Sdir)\LIB\QUERMOD.OBJ
	copy $(Sdir)\LIB\QUERMOD.OBJ $(Tdir)\LIB\QUERMOD.OBJ

$(Tdir)\LIB\SAVELOAD.OBJ : $(Sdir)\LIB\SAVELOAD.OBJ
	copy $(Sdir)\LIB\SAVELOAD.OBJ $(Tdir)\LIB\SAVELOAD.OBJ

$(Tdir)\LIB\SCRIPT.OBJ : $(Sdir)\LIB\SCRIPT.OBJ
	copy $(Sdir)\LIB\SCRIPT.OBJ $(Tdir)\LIB\SCRIPT.OBJ

$(Tdir)\LIB\SCROLBAR.OBJ : $(Sdir)\LIB\SCROLBAR.OBJ
	copy $(Sdir)\LIB\SCROLBAR.OBJ $(Tdir)\LIB\SCROLBAR.OBJ

$(Tdir)\LIB\SCROLL.OBJ : $(Sdir)\LIB\SCROLL.OBJ
	copy $(Sdir)\LIB\SCROLL.OBJ $(Tdir)\LIB\SCROLL.OBJ

$(Tdir)\LIB\SELWIN.OBJ : $(Sdir)\LIB\SELWIN.OBJ
	copy $(Sdir)\LIB\SELWIN.OBJ $(Tdir)\LIB\SELWIN.OBJ

$(Tdir)\LIB\SORT.OBJ : $(Sdir)\LIB\SORT.OBJ
	copy $(Sdir)\LIB\SORT.OBJ $(Tdir)\LIB\SORT.OBJ

$(Tdir)\LIB\SPECITEM.OBJ : $(Sdir)\LIB\SPECITEM.OBJ
	copy $(Sdir)\LIB\SPECITEM.OBJ $(Tdir)\LIB\SPECITEM.OBJ

$(Tdir)\LIB\SPELLS.OBJ : $(Sdir)\LIB\SPELLS.OBJ
	copy $(Sdir)\LIB\SPELLS.OBJ $(Tdir)\LIB\SPELLS.OBJ

$(Tdir)\LIB\STATAREA.OBJ : $(Sdir)\LIB\STATAREA.OBJ
	copy $(Sdir)\LIB\STATAREA.OBJ $(Tdir)\LIB\STATAREA.OBJ

$(Tdir)\LIB\TACTICAL.OBJ : $(Sdir)\LIB\TACTICAL.OBJ
	copy $(Sdir)\LIB\TACTICAL.OBJ $(Tdir)\LIB\TACTICAL.OBJ

$(Tdir)\LIB\TEXTWIN.OBJ : $(Sdir)\LIB\TEXTWIN.OBJ
	copy $(Sdir)\LIB\TEXTWIN.OBJ $(Tdir)\LIB\TEXTWIN.OBJ

$(Tdir)\LIB\USERFACE.OBJ : $(Sdir)\LIB\USERFACE.OBJ
	copy $(Sdir)\LIB\USERFACE.OBJ $(Tdir)\LIB\USERFACE.OBJ

$(Tdir)\LIB\VERSION.OBJ : $(Sdir)\LIB\VERSION.OBJ
	copy $(Sdir)\LIB\VERSION.OBJ $(Tdir)\LIB\VERSION.OBJ

$(Tdir)\LIB\XFTYPES.OBJ : $(Sdir)\LIB\XFTYPES.OBJ
	copy $(Sdir)\LIB\XFTYPES.OBJ $(Tdir)\LIB\XFTYPES.OBJ

$(Tdir)\LIB\XLOAD.OBJ : $(Sdir)\LIB\XLOAD.OBJ
	copy $(Sdir)\LIB\XLOAD.OBJ $(Tdir)\LIB\XLOAD.OBJ


# Copy the library files : */

$(Tdir)\LIB\TEXT.LIB : $(Sdir)\LIB\TEXT.LIB
	copy $(Sdir)\LIB\TEXT.LIB $(Tdir)\LIB\TEXT.LIB

$(Tdir)\LIB\ZOOMFUNC.LIB : $(Sdir)\LIB\ZOOMFUNC.LIB
	copy $(Sdir)\LIB\ZOOMFUNC.LIB $(Tdir)\LIB\ZOOMFUNC.LIB

$(Tdir)\LIB\3DM.LIB : $(Sdir)\LIB\3DM.LIB
	copy $(Sdir)\LIB\3DM.LIB $(Tdir)\LIB\3DM.LIB


# Copy the other files : */

$(Tdir)\SRC\MAKEFILE : $(Sdir)\SRC\MAKEFILE
	copy $(Sdir)\SRC\MAKEFILE $(Tdir)\SRC\MAKEFILE


!ifeq User Jurie

# Copy special files : */

$(Tdir)\LIB\DLLLOAD.OBJ : G:\AIL\AIL_303\DLLLOAD.OBJ
	copy G:\AIL\AIL_303\DLLLOAD.OBJ $(Tdir)\LIB\DLLLOAD.OBJ

$(Tdir)\LIB\BBMEM.LIB : G:\PCLIB32\LIB\BBMEM.LIB
	copy G:\PCLIB32\LIB\BBMEM.LIB $(Tdir)\LIB\BBMEM.LIB

$(Tdir)\LIB\AIL.LIB : G:\AIL\AIL_303\AIL.LIB
	copy G:\AIL\AIL_303\AIL.LIB $(Tdir)\LIB\AIL.LIB

$(Tdir)\LIB\PCBASE.LIB : G:\PCLIB32\LIB\PCBASE.LIB
	copy G:\PCLIB32\LIB\PCBASE.LIB $(Tdir)\LIB\PCBASE.LIB


# Copy files back : */

$(Sdir)\SRC\COMMAGIC\COMMAG0.C : $(Bdir)\SRC\COMMAGIC\COMMAG0.C
	copy $(Bdir)\SRC\COMMAGIC\COMMAG0.C $(Sdir)\SRC\COMMAGIC\COMMAG0.C

$(Sdir)\SRC\COMMAGIC\COMMAG1.C : $(Bdir)\SRC\COMMAGIC\COMMAG1.C
	copy $(Bdir)\SRC\COMMAGIC\COMMAG1.C $(Sdir)\SRC\COMMAGIC\COMMAG1.C

$(Sdir)\SRC\COMMAGIC\COMMAG2.C : $(Bdir)\SRC\COMMAGIC\COMMAG2.C
	copy $(Bdir)\SRC\COMMAGIC\COMMAG2.C $(Sdir)\SRC\COMMAGIC\COMMAG2.C

$(Sdir)\SRC\COMMAGIC\COMMAG3.C : $(Bdir)\SRC\COMMAGIC\COMMAG3.C
	copy $(Bdir)\SRC\COMMAGIC\COMMAG3.C $(Sdir)\SRC\COMMAGIC\COMMAG3.C

$(Sdir)\SRC\COMMAGIC\COMMAG4.C : $(Bdir)\SRC\COMMAGIC\COMMAG4.C
	copy $(Bdir)\SRC\COMMAGIC\COMMAG4.C $(Sdir)\SRC\COMMAGIC\COMMAG4.C

$(Sdir)\SRC\COMMAGIC\COMMSUB.C : $(Bdir)\SRC\COMMAGIC\COMMSUB.C
	copy $(Bdir)\SRC\COMMAGIC\COMMSUB.C $(Sdir)\SRC\COMMAGIC\COMMSUB.C

$(Sdir)\LIB\COMMAGIC.H : $(Bdir)\LIB\COMMAGIC.H
	copy $(Bdir)\LIB\COMMAGIC.H $(Sdir)\LIB\COMMAGIC.H

$(Sdir)\LIB\COMMSUB.H : $(Bdir)\LIB\COMMSUB.H
	copy $(Bdir)\LIB\COMMSUB.H $(Sdir)\LIB\COMMSUB.H

!else


# Copy special files : */

$(Tdir)\LIB\DLLLOAD.OBJ : $(Sdir)\LIB\DLLLOAD.OBJ
	copy $(Sdir)\LIB\DLLLOAD.OBJ $(Tdir)\LIB\DLLLOAD.OBJ

$(Tdir)\LIB\BBMEM.LIB : $(Sdir)\LIB\BBMEM.LIB
	copy $(Sdir)\LIB\BBMEM.LIB $(Tdir)\LIB\BBMEM.LIB

$(Tdir)\LIB\AIL.LIB : $(Sdir)\LIB\AIL.LIB
	copy $(Sdir)\LIB\AIL.LIB $(Tdir)\LIB\AIL.LIB

$(Tdir)\LIB\PCBASE.LIB : $(Sdir)\LIB\PCBASE.LIB
	copy $(Sdir)\LIB\PCBASE.LIB $(Tdir)\LIB\PCBASE.LIB


# Copy files back : */

$(Bdir)\SRC\COMMAGIC\COMMAG0.C : $(Tdir)\SRC\COMMAGIC\COMMAG0.C
	copy $(Tdir)\SRC\COMMAGIC\COMMAG0.C $(Bdir)\SRC\COMMAGIC\COMMAG0.C

$(Bdir)\SRC\COMMAGIC\COMMAG1.C : $(Tdir)\SRC\COMMAGIC\COMMAG1.C
	copy $(Tdir)\SRC\COMMAGIC\COMMAG1.C $(Bdir)\SRC\COMMAGIC\COMMAG1.C

$(Bdir)\SRC\COMMAGIC\COMMAG2.C : $(Tdir)\SRC\COMMAGIC\COMMAG2.C
	copy $(Tdir)\SRC\COMMAGIC\COMMAG2.C $(Bdir)\SRC\COMMAGIC\COMMAG2.C

$(Bdir)\SRC\COMMAGIC\COMMAG3.C : $(Tdir)\SRC\COMMAGIC\COMMAG3.C
	copy $(Tdir)\SRC\COMMAGIC\COMMAG3.C $(Bdir)\SRC\COMMAGIC\COMMAG3.C

$(Bdir)\SRC\COMMAGIC\COMMAG4.C : $(Tdir)\SRC\COMMAGIC\COMMAG4.C
	copy $(Tdir)\SRC\COMMAGIC\COMMAG4.C $(Bdir)\SRC\COMMAGIC\COMMAG4.C

$(Bdir)\SRC\COMMAGIC\COMMSUB.C : $(Tdir)\SRC\COMMAGIC\COMMSUB.C
	copy $(Tdir)\SRC\COMMAGIC\COMMSUB.C $(Bdir)\SRC\COMMAGIC\COMMSUB.C

$(Bdir)\LIB\COMMAGIC.H : $(Tdir)\LIB\COMMAGIC.H
	copy $(Tdir)\LIB\COMMAGIC.H $(Bdir)\LIB\COMMAGIC.H

$(Bdir)\LIB\COMMSUB.H : $(Tdir)\LIB\COMMSUB.H
	copy $(Tdir)\LIB\COMMSUB.H $(Bdir)\LIB\COMMSUB.H

!endif

