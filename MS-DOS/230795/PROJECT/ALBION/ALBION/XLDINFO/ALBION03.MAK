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
Makefile = $(Idir)\ALBION03.MAK
Language = GERMAN


# Define files
XLD_files = $(Pdir)\XLDLIBS\MAPDATA0.XLD &
	$(Pdir)\XLDLIBS\MAPDATA1.XLD &
	$(Pdir)\XLDLIBS\MAPDATA2.XLD &
	$(Pdir)\XLDLIBS\MAPDATA3.XLD &
	$(Pdir)\XLDLIBS\$(Language)\MAPTEXT0.XLD &
	$(Pdir)\XLDLIBS\$(Language)\MAPTEXT1.XLD &
	$(Pdir)\XLDLIBS\$(Language)\MAPTEXT2.XLD &
	$(Pdir)\XLDLIBS\$(Language)\MAPTEXT3.XLD &
	$(Pdir)\XLDLIBS\INITIAL\AUTOMAP0.XLD &
	$(Pdir)\XLDLIBS\INITIAL\AUTOMAP1.XLD &
	$(Pdir)\XLDLIBS\INITIAL\AUTOMAP2.XLD &
	$(Pdir)\XLDLIBS\INITIAL\AUTOMAP3.XLD

Extra_files = $(Compiler) $(Makefile)


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\MAPDATA0.XLD : $(Ddir)\MAP_DATA\MAP_00??.DAT $(Extra_files)
	$(Compiler) $(Idir)\MAP_DATA 0

$(Pdir)\XLDLIBS\MAPDATA1.XLD : $(Ddir)\MAP_DATA\MAP_01??.DAT $(Extra_files)
	$(Compiler) $(Idir)\MAP_DATA 1

$(Pdir)\XLDLIBS\MAPDATA2.XLD : $(Ddir)\MAP_DATA\MAP_02??.DAT $(Extra_files)
	$(Compiler) $(Idir)\MAP_DATA 2

$(Pdir)\XLDLIBS\MAPDATA3.XLD : $(Ddir)\MAP_DATA\MAP_03??.DAT $(Extra_files)
	$(Compiler) $(Idir)\MAP_DATA 3

$(Pdir)\XLDLIBS\$(Language)\MAPTEXT0.XLD : $(Ddir)\MAPTEXTS\MAP_00??.PTX $(Extra_files)
	$(Compiler) $(Idir)\MAPTEXTS 0

$(Pdir)\XLDLIBS\$(Language)\MAPTEXT1.XLD : $(Ddir)\MAPTEXTS\MAP_01??.PTX $(Extra_files)
	$(Compiler) $(Idir)\MAPTEXTS 1

$(Pdir)\XLDLIBS\$(Language)\MAPTEXT2.XLD : $(Ddir)\MAPTEXTS\MAP_02??.PTX $(Extra_files)
	$(Compiler) $(Idir)\MAPTEXTS 2

$(Pdir)\XLDLIBS\$(Language)\MAPTEXT3.XLD : $(Ddir)\MAPTEXTS\MAP_03??.PTX $(Extra_files)
	$(Compiler) $(Idir)\MAPTEXTS 3

$(Pdir)\XLDLIBS\INITIAL\AUTOMAP0.XLD : $(Ddir)\AUTOMAPS\AUTO00??.MAP $(Extra_files)
	$(Compiler) $(Idir)\AUTOMAPS 0

$(Pdir)\XLDLIBS\INITIAL\AUTOMAP1.XLD : $(Ddir)\AUTOMAPS\AUTO01??.MAP $(Extra_files)
	$(Compiler) $(Idir)\AUTOMAPS 1

$(Pdir)\XLDLIBS\INITIAL\AUTOMAP2.XLD : $(Ddir)\AUTOMAPS\AUTO02??.MAP $(Extra_files)
	$(Compiler) $(Idir)\AUTOMAPS 2

$(Pdir)\XLDLIBS\INITIAL\AUTOMAP3.XLD : $(Ddir)\AUTOMAPS\AUTO03??.MAP $(Extra_files)
	$(Compiler) $(Idir)\AUTOMAPS 3

