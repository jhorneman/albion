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
Makefile = U:\PROJECT\ALBION\SWAP\ALBION08.MAK

Extra_files = $(Compiler) $(Makefile)


!ifeq User Jurie

# Paths for Jurie
Pdir = G:\ALBION


# Set environment variables for compiler (only during make)
.BEFORE
	set XLD_SRC_PATH=$(Ddir)
	set XLD_LIB_PATH=$(Pdir)\XLDLIBS

# Define files
XLD_files = $(Pdir)\XLDLIBS\ENGLISH\MAPTEXT1.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\MAPTEXT2.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\MAPTEXT3.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT0.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT1.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT2.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT3.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT9.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\WORDLIS0.XLD


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\ENGLISH\MAPTEXT1.XLD : $(Ddir)\ENGLISH\MAPTEXTS\MAP_01??.PTX $(Extra_files)
	$(Compiler) ENGLISH\MAPTEXTS\MAP_01??.PTX ENGLISH\$^. 1

$(Pdir)\XLDLIBS\ENGLISH\MAPTEXT2.XLD : $(Ddir)\ENGLISH\MAPTEXTS\MAP_02??.PTX $(Extra_files)
	$(Compiler) ENGLISH\MAPTEXTS\MAP_02??.PTX ENGLISH\$^. 2

$(Pdir)\XLDLIBS\ENGLISH\MAPTEXT3.XLD : $(Ddir)\ENGLISH\MAPTEXTS\MAP_03??.PTX $(Extra_files)
	$(Compiler) ENGLISH\MAPTEXTS\MAP_03??.PTX ENGLISH\$^. 3

$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT0.XLD : $(Ddir)\ENGLISH\EVENTTXT\EVT_00??.CMP $(Extra_files)
	$(Compiler) ENGLISH\EVENTTXT\EVT_00??.CMP ENGLISH\$^. 0

$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT1.XLD : $(Ddir)\ENGLISH\EVENTTXT\EVT_01??.CMP $(Extra_files)
	$(Compiler) ENGLISH\EVENTTXT\EVT_01??.CMP ENGLISH\$^. 1

$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT2.XLD : $(Ddir)\ENGLISH\EVENTTXT\EVT_02??.CMP $(Extra_files)
	$(Compiler) ENGLISH\EVENTTXT\EVT_02??.CMP ENGLISH\$^. 2

$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT3.XLD : $(Ddir)\ENGLISH\EVENTTXT\EVT_03??.CMP $(Extra_files)
	$(Compiler) ENGLISH\EVENTTXT\EVT_03??.CMP ENGLISH\$^. 3

$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT9.XLD : $(Ddir)\ENGLISH\EVENTTXT\EVT_09??.CMP $(Extra_files)
	$(Compiler) ENGLISH\EVENTTXT\EVT_09??.CMP ENGLISH\$^. 9

$(Pdir)\XLDLIBS\ENGLISH\WORDLIS0.XLD : $(Ddir)\ENGLISH\WORDLIST.0?? $(Extra_files)
	$(Compiler) ENGLISH\WORDLIST.0?? ENGLISH\$^. 0

!endif

