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
Makefile = U:\PROJECT\ALBION\SWAP\ALBION03.MAK

Extra_files = $(Compiler) $(Makefile)


!ifeq User Jurie

# Paths for Jurie
Pdir = G:\ALBION

!else

# Paths for non-Juries
Pdir = C:\ALBION
Pdir2 = U:\PROJECT\ALBION\ALBION

!endif


# Set environment variables for compiler (only during make)
.BEFORE
	set XLD_SRC_PATH=$(Ddir)
	set XLD_LIB_PATH=$(Pdir)\XLDLIBS

# Define files
XLD_files = $(Pdir)\XLDLIBS\MAPDATA1.XLD &
	$(Pdir)\XLDLIBS\MAPDATA2.XLD &
	$(Pdir)\XLDLIBS\MAPDATA3.XLD &
	$(Pdir)\XLDLIBS\INITIAL\AUTOMAP1.XLD &
	$(Pdir)\XLDLIBS\INITIAL\AUTOMAP2.XLD &
	$(Pdir)\XLDLIBS\INITIAL\AUTOMAP3.XLD


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\MAPDATA1.XLD : $(Ddir)\MAP_DATA\MAP_01??.DAT $(Extra_files)
	$(Compiler) MAP_DATA\MAP_01??.DAT $^. 1

$(Pdir)\XLDLIBS\MAPDATA2.XLD : $(Ddir)\MAP_DATA\MAP_02??.DAT $(Extra_files)
	$(Compiler) MAP_DATA\MAP_02??.DAT $^. 2

$(Pdir)\XLDLIBS\MAPDATA3.XLD : $(Ddir)\MAP_DATA\MAP_03??.DAT $(Extra_files)
	$(Compiler) MAP_DATA\MAP_03??.DAT $^. 3

$(Pdir)\XLDLIBS\INITIAL\AUTOMAP1.XLD : $(Ddir)\AUTOMAPS\AUTO01??.MAP $(Extra_files)
	$(Compiler) AUTOMAPS\AUTO01??.MAP INITIAL\$^. 1

$(Pdir)\XLDLIBS\INITIAL\AUTOMAP2.XLD : $(Ddir)\AUTOMAPS\AUTO02??.MAP $(Extra_files)
	$(Compiler) AUTOMAPS\AUTO02??.MAP INITIAL\$^. 2

$(Pdir)\XLDLIBS\INITIAL\AUTOMAP3.XLD : $(Ddir)\AUTOMAPS\AUTO03??.MAP $(Extra_files)
	$(Compiler) AUTOMAPS\AUTO03??.MAP INITIAL\$^. 3

