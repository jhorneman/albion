# Makefile for Albion data

# Ignore return values and do not check if target was made
.IGNORE

# Default user is Jurie
!ifndef User
User = Jurie
!endif


!ifeq User Jurie

# Paths for Jurie
Pdir = G:\ALBION
Ddir = U:\PROJECT\ALBION\CENTRAL.DAT
Idir = $(Pdir)\XLDINFO

# Define files for Jurie
XLD_files = $(Pdir)\XLDLIBS\PALETTE0.XLD &
	$(Pdir)\XLDLIBS\PARTGR0.XLD &
	$(Pdir)\XLDLIBS\PARTKL0.XLD &
	$(Pdir)\XLDLIBS\NPCGR0.XLD &
	$(Pdir)\XLDLIBS\NPCGR1.XLD &
	$(Pdir)\XLDLIBS\NPCKL0.XLD &
	$(Pdir)\XLDLIBS\MONGFX0.XLD &
	$(Pdir)\XLDLIBS\3DBCKGR0.XLD &
	$(Pdir)\XLDLIBS\COMBACK0.XLD &
	$(Pdir)\XLDLIBS\COMGFX0.XLD &
	$(Pdir)\XLDLIBS\SMLPORT0.XLD &
	$(Pdir)\XLDLIBS\FBODPIX0.XLD &
	$(Pdir)\XLDLIBS\SONGS0.XLD &
	$(Pdir)\XLDLIBS\SAMPLES0.XLD &
	$(Pdir)\XLDLIBS\SAMPLES1.XLD &
	$(Pdir)\XLDLIBS\WAVELIB0.XLD &
	$(Pdir)\XLDLIBS\ITEMLIST.DAT &
	$(Pdir)\XLDLIBS\ITEMNAME.DAT &
	$(Pdir)\XLDLIBS\ITEMGFX &
	$(Pdir)\XLDLIBS\SPELLDAT.DAT &
	$(Pdir)\XLDLIBS\TACTICO0.XLD

!else

# Paths for non-Juries
Pdir = C:\ALBION
Pdir2 = U:\PROJECT\ALBION\ALBION
Ddir = U:\PROJECT\ALBION\CENTRAL.DAT
Idir = $(Pdir2)\XLDINFO

# Define files for non-Juries
XLD_files = $(Pdir)\XLDLIBS\PALETTE0.XLD &
	$(Pdir)\XLDLIBS\PARTGR0.XLD &
	$(Pdir)\XLDLIBS\PARTKL0.XLD &
	$(Pdir)\XLDLIBS\NPCGR0.XLD &
	$(Pdir)\XLDLIBS\NPCGR1.XLD &
	$(Pdir)\XLDLIBS\NPCKL0.XLD &
	$(Pdir)\XLDLIBS\MONGFX0.XLD &
	$(Pdir)\XLDLIBS\3DBCKGR0.XLD &
	$(Pdir)\XLDLIBS\COMBACK0.XLD &
	$(Pdir)\XLDLIBS\COMGFX0.XLD &
	$(Pdir)\XLDLIBS\SMLPORT0.XLD &
	$(Pdir)\XLDLIBS\FBODPIX0.XLD &
	$(Pdir)\XLDLIBS\SONGS0.XLD &
	$(Pdir)\XLDLIBS\SAMPLES0.XLD &
	$(Pdir)\XLDLIBS\SAMPLES1.XLD &
	$(Pdir)\XLDLIBS\WAVELIB0.XLD &
	$(Pdir)\XLDLIBS\PALETTE.000 &
	$(Pdir)\XLDLIBS\SLAB &
	$(Pdir)\XLDLIBS\FONTS0.XLD &
	$(Pdir)\XLDLIBS\SYSTEXT.GER &
	$(Pdir)\XLDLIBS\SYSTEXT.ENG &
	$(Pdir)\XLDLIBS\SYSTEXT.FRE &
	$(Pdir)\XLDLIBS\ITEMLIST.DAT &
	$(Pdir)\XLDLIBS\ITEMNAME.DAT &
	$(Pdir)\XLDLIBS\SPELLDAT.DAT &
	$(Pdir)\XLDLIBS\ITEMGFX &
	$(Pdir)\XLDLIBS\AUTOMGFX &
	$(Pdir)\XLDLIBS\TACTICO0.XLD &
	$(Pdir)\ALBION.EXE

!endif


Compiler = $(Idir)\COMPILE.EXE
Makefile = $(Idir)\ALBION02.MAK


Extra_files = $(Compiler) $(Makefile)


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from Jurie

!ifeq User Jurie

# Compile for Jurie

$(Pdir)\XLDLIBS\PALETTE0.XLD : $(Pdir)\DATA\PALETTES\0??.PAL $(Extra_files)
	$(Compiler) $(Idir)\PALETTES 0

$(Pdir)\XLDLIBS\PARTGR0.XLD : $(Pdir)\DATA\PARTGR\PARTGR.0?? $(Extra_files)
	$(Compiler) $(Idir)\PARTGR 0

$(Pdir)\XLDLIBS\PARTKL0.XLD : $(Pdir)\DATA\PARTKL\PARTKL.0?? $(Extra_files)
	$(Compiler) $(Idir)\PARTKL 0

$(Pdir)\XLDLIBS\NPCGR0.XLD : $(Pdir)\DATA\NPCGR\NPC2DG.0?? $(Extra_files)
	$(Compiler) $(Idir)\NPCGR 0

$(Pdir)\XLDLIBS\NPCGR1.XLD : $(Pdir)\DATA\NPCGR\NPC2DG.1?? $(Extra_files)
	$(Compiler) $(Idir)\NPCGR 1

$(Pdir)\XLDLIBS\NPCKL0.XLD : $(Pdir)\DATA\NPCKL\NPC2DK.0?? $(Extra_files)
	$(Compiler) $(Idir)\NPCKL 0

$(Pdir)\XLDLIBS\MONGFX0.XLD : $(Pdir)\DATA\MONGFX\MONGFX.0?? $(Extra_files)
	$(Compiler) $(Idir)\MONGFX 0

$(Pdir)\XLDLIBS\3DBCKGR0.XLD : $(Pdir)\DATA\3DBACKGR\3DBACKGR.0?? $(Extra_files)
	$(Compiler) $(Idir)\3DBACKGR 0

$(Pdir)\XLDLIBS\COMBACK0.XLD : $(Pdir)\DATA\COMBACK\COMBACK.0?? $(Extra_files)
	$(Compiler) $(Idir)\COMBACK 0

$(Pdir)\XLDLIBS\COMGFX0.XLD : $(Pdir)\DATA\COMGFX\COMGFX.0?? $(Extra_files)
	$(Compiler) $(Idir)\COMGFX 0

$(Pdir)\XLDLIBS\SMLPORT0.XLD : $(Pdir)\DATA\PORTRAIT\PORTRAIT.0?? $(Extra_files)
	$(Compiler) $(Idir)\SMALPORT 0

$(Pdir)\XLDLIBS\FBODPIX0.XLD : $(Pdir)\DATA\FBODPICS\FBODPIC.0?? $(Extra_files)
	$(Compiler) $(Idir)\FBODPICS 0

$(Pdir)\XLDLIBS\SONGS0.XLD : $(Pdir)\MUSIC\SONGS\SONG0??.XMI $(Extra_files)
	$(Compiler) $(Idir)\SONGS 0

$(Pdir)\XLDLIBS\SAMPLES0.XLD : $(Ddir)\SAMPLES\SAMPLE.0?? $(Extra_files)
	$(Compiler) $(Idir)\SAMPLES 0

$(Pdir)\XLDLIBS\SAMPLES1.XLD : $(Ddir)\SAMPLES\SAMPLE.1?? $(Extra_files)
	$(Compiler) $(Idir)\SAMPLES 1

$(Pdir)\XLDLIBS\WAVELIB0.XLD : $(Pdir)\MUSIC\WAVELIBS\WAVE0??.WVL $(Extra_files)
	$(Compiler) $(Idir)\WAVELIBS 0

$(Pdir)\XLDLIBS\TACTICO0.XLD : $(Pdir)\DATA\TACTICON\TACTICON.0?? $(Extra_files)
	$(Compiler) $(Idir)\TACTICON 0

!else

# Copy for non-Juries

$(Pdir)\XLDLIBS\PALETTE0.XLD : $(Pdir2)\XLDLIBS\PALETTE0.XLD
	copy $(Pdir2)\XLDLIBS\PALETTE0.XLD $(Pdir)\XLDLIBS\PALETTE0.XLD

$(Pdir)\XLDLIBS\PARTGR0.XLD : $(Pdir2)\XLDLIBS\PARTGR0.XLD
	copy $(Pdir2)\XLDLIBS\PARTGR0.XLD $(Pdir)\XLDLIBS\PARTGR0.XLD

$(Pdir)\XLDLIBS\PARTKL0.XLD : $(Pdir2)\XLDLIBS\PARTKL0.XLD
	copy $(Pdir2)\XLDLIBS\PARTKL0.XLD $(Pdir)\XLDLIBS\PARTKL0.XLD

$(Pdir)\XLDLIBS\NPCGR0.XLD : $(Pdir2)\XLDLIBS\NPCGR0.XLD
	copy $(Pdir2)\XLDLIBS\NPCGR0.XLD $(Pdir)\XLDLIBS\NPCGR0.XLD

$(Pdir)\XLDLIBS\NPCGR1.XLD : $(Pdir2)\XLDLIBS\NPCGR1.XLD
	copy $(Pdir2)\XLDLIBS\NPCGR1.XLD $(Pdir)\XLDLIBS\NPCGR1.XLD

$(Pdir)\XLDLIBS\NPCKL0.XLD : $(Pdir2)\XLDLIBS\NPCKL0.XLD
	copy $(Pdir2)\XLDLIBS\NPCKL0.XLD $(Pdir)\XLDLIBS\NPCKL0.XLD

$(Pdir)\XLDLIBS\MONGFX0.XLD : $(Pdir2)\XLDLIBS\MONGFX0.XLD
	copy $(Pdir2)\XLDLIBS\MONGFX0.XLD $(Pdir)\XLDLIBS\MONGFX0.XLD

$(Pdir)\XLDLIBS\3DBCKGR0.XLD : $(Pdir2)\XLDLIBS\3DBCKGR0.XLD
	copy $(Pdir2)\XLDLIBS\3DBCKGR0.XLD $(Pdir)\XLDLIBS\3DBCKGR0.XLD

$(Pdir)\XLDLIBS\COMBACK0.XLD : $(Pdir2)\XLDLIBS\COMBACK0.XLD
	copy $(Pdir2)\XLDLIBS\COMBACK0.XLD $(Pdir)\XLDLIBS\COMBACK0.XLD

$(Pdir)\XLDLIBS\COMGFX0.XLD : $(Pdir2)\XLDLIBS\COMGFX0.XLD
	copy $(Pdir2)\XLDLIBS\COMGFX0.XLD $(Pdir)\XLDLIBS\COMGFX0.XLD

$(Pdir)\XLDLIBS\SMLPORT0.XLD : $(Pdir2)\XLDLIBS\SMLPORT0.XLD
	copy $(Pdir2)\XLDLIBS\SMLPORT0.XLD $(Pdir)\XLDLIBS\SMLPORT0.XLD

$(Pdir)\XLDLIBS\FBODPIX0.XLD : $(Pdir2)\XLDLIBS\FBODPIX0.XLD
	copy $(Pdir2)\XLDLIBS\FBODPIX0.XLD $(Pdir)\XLDLIBS\FBODPIX0.XLD

$(Pdir)\XLDLIBS\SONGS0.XLD : $(Pdir2)\XLDLIBS\SONGS0.XLD
	copy $(Pdir2)\XLDLIBS\SONGS0.XLD $(Pdir)\XLDLIBS\SONGS0.XLD

$(Pdir)\XLDLIBS\SAMPLES0.XLD : $(Ddir)\SAMPLES\SAMPLE.0?? $(Extra_files)
	$(Compiler) $(Idir)\SAMPLES 0

$(Pdir)\XLDLIBS\SAMPLES1.XLD : $(Ddir)\SAMPLES\SAMPLE.1?? $(Extra_files)
	$(Compiler) $(Idir)\SAMPLES 1

$(Pdir)\XLDLIBS\WAVELIB0.XLD : $(Pdir2)\XLDLIBS\WAVELIB0.XLD
	copy $(Pdir2)\XLDLIBS\WAVELIB0.XLD $(Pdir)\XLDLIBS\WAVELIB0.XLD

# Extra files for non-Juries

$(Pdir)\XLDLIBS\PALETTE.000 : $(Pdir2)\XLDLIBS\PALETTE.000
	copy $(Pdir2)\XLDLIBS\PALETTE.000 $(Pdir)\XLDLIBS\PALETTE.000

$(Pdir)\XLDLIBS\SLAB : $(Pdir2)\XLDLIBS\SLAB
	copy $(Pdir2)\XLDLIBS\SLAB $(Pdir)\XLDLIBS\SLAB

$(Pdir)\XLDLIBS\FONTS0.XLD : $(Pdir2)\XLDLIBS\FONTS0.XLD
	copy $(Pdir2)\XLDLIBS\FONTS0.XLD $(Pdir)\XLDLIBS\FONTS0.XLD

$(Pdir)\XLDLIBS\SYSTEXT.GER : $(Pdir2)\XLDLIBS\SYSTEXT.GER
	copy $(Pdir2)\XLDLIBS\SYSTEXT.GER $(Pdir)\XLDLIBS\SYSTEXT.GER

$(Pdir)\XLDLIBS\SYSTEXT.ENG : $(Pdir2)\XLDLIBS\SYSTEXT.ENG
	copy $(Pdir2)\XLDLIBS\SYSTEXT.ENG $(Pdir)\XLDLIBS\SYSTEXT.ENG

$(Pdir)\XLDLIBS\SYSTEXT.FRE : $(Pdir2)\XLDLIBS\SYSTEXT.FRE
	copy $(Pdir2)\XLDLIBS\SYSTEXT.FRE $(Pdir)\XLDLIBS\SYSTEXT.FRE

$(Pdir)\XLDLIBS\AUTOMGFX : $(Pdir2)\XLDLIBS\AUTOMGFX
	copy $(Pdir2)\XLDLIBS\AUTOMGFX $(Pdir)\XLDLIBS\AUTOMGFX

$(Pdir)\XLDLIBS\TACTICO0.XLD : $(Pdir2)\XLDLIBS\TACTICO0.XLD
	copy $(Pdir2)\XLDLIBS\TACTICO0.XLD $(Pdir)\XLDLIBS\TACTICO0.XLD

$(Pdir)\ALBION.EXE : $(Pdir2)\ALBION.EXE
	copy $(Pdir2)\ALBION.EXE $(Pdir)\ALBION.EXE

!endif

# Extra files coming from the server

$(Pdir)\XLDLIBS\ITEMLIST.DAT : $(Ddir)\ITEMLIST.DAT
	copy $(Ddir)\ITEMLIST.DAT $(Pdir)\XLDLIBS\ITEMLIST.DAT

$(Pdir)\XLDLIBS\ITEMNAME.DAT : $(Ddir)\ITEMNAME.DAT
	copy $(Ddir)\ITEMNAME.DAT $(Pdir)\XLDLIBS\ITEMNAME.DAT

$(Pdir)\XLDLIBS\ITEMGFX : $(Ddir)\ITEM_GFX.JUR
	copy $(Ddir)\ITEM_GFX.JUR $(Pdir)\XLDLIBS\ITEMGFX

$(Pdir)\XLDLIBS\SPELLDAT.DAT : $(Ddir)\SPELLDAT.DAT
	copy $(Ddir)\SPELLDAT.DAT $(Pdir)\XLDLIBS\SPELLDAT.DAT

