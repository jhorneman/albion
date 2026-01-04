# Makefile for Albion data

# Erase targets on error
.ERASE

# Do not output commands
.SILENT

# Default user is Jurie
!ifndef User
User = Jurie
!endif

# Define paths
Ddir = U:\PROJECT\ALBION\CENTRAL.DAT

# Define compiler and makefile
Compiler = U:\PROJECT\ALBION\SWAP\COMPILE.EXE
Makefile = U:\PROJECT\ALBION\SWAP\ALBION02.MAK

Extra_files = $(Compiler) $(Makefile)


!ifeq User Jurie

# Paths for Jurie
Pdir = G:\ALBION
Ddir = U:\PROJECT\ALBION\CENTRAL.DAT

# Define files for Jurie
XLD_files = $(Pdir)\XLDLIBS\PICTURE0.XLD &
	$(Pdir)\XLDLIBS\FLICS0.XLD &
	$(Pdir)\XLDLIBS\SCRIPT0.XLD &
	$(Pdir)\XLDLIBS\SCRIPT2.XLD &
	$(Pdir)\XLDLIBS\ITEMLIST.DAT &
	$(Pdir)\XLDLIBS\ITEMNAME.DAT &
	$(Pdir)\XLDLIBS\ITEMGFX &
	$(Pdir)\XLDLIBS\SPELLDAT.DAT

!else

# Paths for non-Juries
Pdir = C:\ALBION
Pdir2 = U:\PROJECT\ALBION\ALBION
Ddir = U:\PROJECT\ALBION\CENTRAL.DAT


# Define files for non-Juries
XLD_files = $(Pdir)\XLDLIBS\PICTURE0.XLD &
	$(Pdir)\XLDLIBS\FLICS0.XLD &
	$(Pdir)\XLDLIBS\SCRIPT0.XLD &
	$(Pdir)\XLDLIBS\SCRIPT2.XLD &
	$(Pdir)\XLDLIBS\PALETTE.000 &
	$(Pdir)\XLDLIBS\SLAB &
	$(Pdir)\XLDLIBS\AUTOGFX0.XLD &
	$(Pdir)\XLDLIBS\TACTICO0.XLD &
	$(Pdir)\ALBION.EXE &
	$(Pdir)\XLDLIBS\ITEMLIST.DAT &
	$(Pdir)\XLDLIBS\ITEMNAME.DAT &
	$(Pdir)\XLDLIBS\ITEMGFX &
	$(Pdir)\XLDLIBS\SPELLDAT.DAT

!endif


# Set environment variables for compiler (only during make)
.BEFORE
	set XLD_SRC_PATH=$(Ddir)
	set XLD_LIB_PATH=$(Pdir)\XLDLIBS


# This is the main "target"

ALL : $(XLD_files)


# Compile XLD-files

$(Pdir)\XLDLIBS\PICTURE0.XLD : $(Ddir)\PICS\PICT0??.LBM $(Extra_files)
	$(Compiler) PICS\PICT0??.LBM $^. 0

$(Pdir)\XLDLIBS\FLICS0.XLD : $(Ddir)\FLICS\FLIC0??.FLC $(Extra_files)
	$(Compiler) FLICS\FLIC0??.FLC $^. 0

$(Pdir)\XLDLIBS\SCRIPT0.XLD : $(Ddir)\SCRIPTS\SCRPT0??.TXT $(Extra_files)
	$(Compiler) SCRIPTS\SCRPT0??.TXT $^. 0

$(Pdir)\XLDLIBS\SCRIPT2.XLD : $(Ddir)\SCRIPTS\SCRPT2??.TXT $(Extra_files)
	$(Compiler) SCRIPTS\SCRPT2??.TXT $^. 2


# Copy extra files from the server

$(Pdir)\XLDLIBS\ITEMLIST.DAT : $(Ddir)\ITEMLIST.DAT
	copy $(Ddir)\ITEMLIST.DAT $(Pdir)\XLDLIBS\ITEMLIST.DAT

$(Pdir)\XLDLIBS\ITEMNAME.DAT : $(Ddir)\ITEMNAME.DAT
	copy $(Ddir)\ITEMNAME.DAT $(Pdir)\XLDLIBS\ITEMNAME.DAT

$(Pdir)\XLDLIBS\ITEMGFX : $(Ddir)\ITEM_GFX.JUR
	copy $(Ddir)\ITEM_GFX.JUR $(Pdir)\XLDLIBS\ITEMGFX

$(Pdir)\XLDLIBS\SPELLDAT.DAT : $(Ddir)\SPELLDAT.DAT
	copy $(Ddir)\SPELLDAT.DAT $(Pdir)\XLDLIBS\SPELLDAT.DAT


!ifneq User Jurie

# Copy extra files for non-Juries

$(Pdir)\XLDLIBS\PALETTE.000 : $(Pdir2)\XLDLIBS\PALETTE.000
	copy $(Pdir2)\XLDLIBS\PALETTE.000 $(Pdir)\XLDLIBS\PALETTE.000

$(Pdir)\XLDLIBS\SLAB : $(Pdir2)\XLDLIBS\SLAB
	copy $(Pdir2)\XLDLIBS\SLAB $(Pdir)\XLDLIBS\SLAB

$(Pdir)\XLDLIBS\AUTOGFX0.XLD : $(Pdir2)\XLDLIBS\AUTOGFX0.XLD
	copy $(Pdir2)\XLDLIBS\AUTOGFX0.XLD $(Pdir)\XLDLIBS\AUTOGFX0.XLD

$(Pdir)\XLDLIBS\TACTICO0.XLD : $(Pdir2)\XLDLIBS\TACTICO0.XLD
	copy $(Pdir2)\XLDLIBS\TACTICO0.XLD $(Pdir)\XLDLIBS\TACTICO0.XLD

$(Pdir)\ALBION.EXE : $(Pdir2)\ALBION.EXE
	copy $(Pdir2)\ALBION.EXE $(Pdir)\ALBION.EXE

!endif

