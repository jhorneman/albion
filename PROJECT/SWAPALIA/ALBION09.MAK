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
Makefile = U:\PROJECT\ALBION\SWAP\ALBION09.MAK

Extra_files = $(Compiler) $(Makefile)


!ifeq User Jurie

# Paths for Jurie
Pdir = G:\ALBION


# Set environment variables for compiler (only during make)
.BEFORE
	set XLD_SRC_PATH=$(Ddir)
	set XLD_LIB_PATH=$(Pdir)\XLDLIBS


# Define files
XLD_files = $(Pdir)\XLDLIBS\FRENCH\MAPTEXT1.XLD &
	$(Pdir)\XLDLIBS\FRENCH\MAPTEXT2.XLD &
	$(Pdir)\XLDLIBS\FRENCH\MAPTEXT3.XLD &
	$(Pdir)\XLDLIBS\FRENCH\EVNTTXT0.XLD &
	$(Pdir)\XLDLIBS\FRENCH\EVNTTXT1.XLD &
	$(Pdir)\XLDLIBS\FRENCH\EVNTTXT2.XLD &
	$(Pdir)\XLDLIBS\FRENCH\EVNTTXT3.XLD &
	$(Pdir)\XLDLIBS\FRENCH\EVNTTXT9.XLD &
	$(Pdir)\XLDLIBS\FRENCH\WORDLIS0.XLD


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\FRENCH\MAPTEXT1.XLD : $(Ddir)\FRENCH\MAPTEXTS\MAP_01??.PTX $(Extra_files)
	$(Compiler) FRENCH\MAPTEXTS\MAP_01??.PTX FRENCH\$^. 1

$(Pdir)\XLDLIBS\FRENCH\MAPTEXT2.XLD : $(Ddir)\FRENCH\MAPTEXTS\MAP_02??.PTX $(Extra_files)
	$(Compiler) FRENCH\MAPTEXTS\MAP_02??.PTX FRENCH\$^. 2

$(Pdir)\XLDLIBS\FRENCH\MAPTEXT3.XLD : $(Ddir)\FRENCH\MAPTEXTS\MAP_03??.PTX $(Extra_files)
	$(Compiler) FRENCH\MAPTEXTS\MAP_03??.PTX FRENCH\$^. 3

$(Pdir)\XLDLIBS\FRENCH\EVNTTXT0.XLD : $(Ddir)\FRENCH\EVENTTXT\EVT_00??.CMP $(Extra_files)
	$(Compiler) FRENCH\EVENTTXT\EVT_00??.CMP FRENCH\$^. 0

$(Pdir)\XLDLIBS\FRENCH\EVNTTXT1.XLD : $(Ddir)\FRENCH\EVENTTXT\EVT_01??.CMP $(Extra_files)
	$(Compiler) FRENCH\EVENTTXT\EVT_01??.CMP FRENCH\$^. 1

$(Pdir)\XLDLIBS\FRENCH\EVNTTXT2.XLD : $(Ddir)\FRENCH\EVENTTXT\EVT_02??.CMP $(Extra_files)
	$(Compiler) FRENCH\EVENTTXT\EVT_02??.CMP FRENCH\$^. 2

$(Pdir)\XLDLIBS\FRENCH\EVNTTXT3.XLD : $(Ddir)\FRENCH\EVENTTXT\EVT_03??.CMP $(Extra_files)
	$(Compiler) FRENCH\EVENTTXT\EVT_03??.CMP FRENCH\$^. 3

$(Pdir)\XLDLIBS\FRENCH\EVNTTXT9.XLD : $(Ddir)\FRENCH\EVENTTXT\EVT_09??.CMP $(Extra_files)
	$(Compiler) FRENCH\EVENTTXT\EVT_09??.CMP FRENCH\$^. 9

$(Pdir)\XLDLIBS\FRENCH\WORDLIS0.XLD : $(Ddir)\FRENCH\WORDLIST.0?? $(Extra_files)
	$(Compiler) FRENCH\WORDLIST.0?? FRENCH\$^. 0

!endif

