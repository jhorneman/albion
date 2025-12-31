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
Compiler = $(Pdir)\XLDINFO\COMPILE.EXE
Makefile = $(Pdir)\XLDINFO\ALBION01.MAK

!else

# Paths for non-Juries
Pdir = C:\ALBION
Pdir2 = U:\PROJECT\ALBION\ALBION
Ddir = U:\PROJECT\ALBION\CENTRAL.DAT
Compiler = $(Pdir2)\XLDINFO\COMPILE.EXE
Makefile = $(Pdir2)\XLDINFO\ALBION01.MAK

!endif

# Define files
XLD_files = $(Pdir)\XLDLIBS\ICONDAT0.XLD &
	$(Pdir)\XLDLIBS\ICONGFX0.XLD &
	$(Pdir)\XLDLIBS\BLKLIST0.XLD

Extra_files = $(Compiler) $(Makefile)

# This is the main "target"

ALL : $(XLD_files)

# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\ICONDAT0.XLD : $(Ddir)\ICONDATA\ICONLIST.??? $(Extra_files)
	$(Compiler) ICONDATA

$(Pdir)\XLDLIBS\ICONGFX0.XLD : $(Ddir)\ICON_GFX\JURIE_GR.??? $(Extra_files)
	$(Compiler) ICON_GFX

$(Pdir)\XLDLIBS\BLKLIST0.XLD : $(Ddir)\ICONDATA\CUTBLOCK.??? $(Extra_files)
	$(Compiler) BLOKLIST

