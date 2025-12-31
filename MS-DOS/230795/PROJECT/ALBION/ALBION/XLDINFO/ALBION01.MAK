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

!else

# Paths for non-Juries
Pdir = C:\ALBION
Pdir2 = U:\PROJECT\ALBION\ALBION
Ddir = U:\PROJECT\ALBION\CENTRAL.DAT
Idir = $(Pdir2)\XLDINFO

!endif


Compiler = $(Idir)\COMPILE.EXE
Makefile = $(Idir)\ALBION01.MAK


# Define files
XLD_files = $(Pdir)\XLDLIBS\ICONDAT0.XLD &
	$(Pdir)\XLDLIBS\ICONGFX0.XLD &
	$(Pdir)\XLDLIBS\BLKLIST0.XLD

Extra_files = $(Compiler) $(Makefile)


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\ICONDAT0.XLD : $(Ddir)\ICONDATA\ICONLIST.0?? $(Extra_files)
	$(Compiler) $(Idir)\ICONDATA 0

$(Pdir)\XLDLIBS\ICONGFX0.XLD : $(Ddir)\ICON_GFX\JURIE_GR.0?? $(Extra_files)
	$(Compiler) $(Idir)\ICON_GFX 0

$(Pdir)\XLDLIBS\BLKLIST0.XLD : $(Ddir)\ICONDATA\CUTBLOCK.0?? $(Extra_files)
	$(Compiler) $(Idir)\BLOKLIST 0

