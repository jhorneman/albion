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
Makefile = U:\PROJECT\ALBION\SWAP\ALBION07.MAK

Extra_files = $(Compiler) $(Makefile)


!ifeq User Jurie

# Paths for Jurie
Pdir = G:\ALBION

# Define files for Jurie
XLD_files = $(Pdir)\XLDLIBS\GERMAN\MAPTEXT1.XLD &
	$(Pdir)\XLDLIBS\GERMAN\MAPTEXT2.XLD &
	$(Pdir)\XLDLIBS\GERMAN\MAPTEXT3.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT0.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT1.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT2.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT3.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT9.XLD &
	$(Pdir)\XLDLIBS\GERMAN\WORDLIS0.XLD

!else

# Paths for non-Juries
Pdir = C:\ALBION
Pdir2 = U:\PROJECT\ALBION\ALBION

# Define files for non-Juries
XLD_files = $(Pdir)\XLDLIBS\GERMAN\MAPTEXT1.XLD &
	$(Pdir)\XLDLIBS\GERMAN\MAPTEXT2.XLD &
	$(Pdir)\XLDLIBS\GERMAN\MAPTEXT3.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT0.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT1.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT2.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT3.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT9.XLD &
	$(Pdir)\XLDLIBS\GERMAN\WORDLIS0.XLD &
	$(Pdir)\XLDLIBS\GERMAN\SYSTEXTS

!endif


# Set environment variables for compiler (only during make)
.BEFORE
	set XLD_SRC_PATH=$(Ddir)
	set XLD_LIB_PATH=$(Pdir)\XLDLIBS


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\GERMAN\MAPTEXT1.XLD : $(Ddir)\GERMAN\MAPTEXTS\MAP_01??.PTX $(Extra_files)
	$(Compiler) GERMAN\MAPTEXTS\MAP_01??.PTX GERMAN\$^. 1

$(Pdir)\XLDLIBS\GERMAN\MAPTEXT2.XLD : $(Ddir)\GERMAN\MAPTEXTS\MAP_02??.PTX $(Extra_files)
	$(Compiler) GERMAN\MAPTEXTS\MAP_02??.PTX GERMAN\$^. 2

$(Pdir)\XLDLIBS\GERMAN\MAPTEXT3.XLD : $(Ddir)\GERMAN\MAPTEXTS\MAP_03??.PTX $(Extra_files)
	$(Compiler) GERMAN\MAPTEXTS\MAP_03??.PTX GERMAN\$^. 3

$(Pdir)\XLDLIBS\GERMAN\EVNTTXT0.XLD : $(Ddir)\GERMAN\EVENTTXT\EVT_00??.CMP $(Extra_files)
	$(Compiler) GERMAN\EVENTTXT\EVT_00??.CMP GERMAN\$^. 0

$(Pdir)\XLDLIBS\GERMAN\EVNTTXT1.XLD : $(Ddir)\GERMAN\EVENTTXT\EVT_01??.CMP $(Extra_files)
	$(Compiler) GERMAN\EVENTTXT\EVT_01??.CMP GERMAN\$^. 1

$(Pdir)\XLDLIBS\GERMAN\EVNTTXT2.XLD : $(Ddir)\GERMAN\EVENTTXT\EVT_02??.CMP $(Extra_files)
	$(Compiler) GERMAN\EVENTTXT\EVT_02??.CMP GERMAN\$^. 2

$(Pdir)\XLDLIBS\GERMAN\EVNTTXT3.XLD : $(Ddir)\GERMAN\EVENTTXT\EVT_03??.CMP $(Extra_files)
	$(Compiler) GERMAN\EVENTTXT\EVT_03??.CMP GERMAN\$^. 3

$(Pdir)\XLDLIBS\GERMAN\EVNTTXT9.XLD : $(Ddir)\GERMAN\EVENTTXT\EVT_09??.CMP $(Extra_files)
	$(Compiler) GERMAN\EVENTTXT\EVT_09??.CMP GERMAN\$^. 9

$(Pdir)\XLDLIBS\GERMAN\WORDLIS0.XLD : $(Ddir)\GERMAN\WORDLIST.0?? $(Extra_files)
	$(Compiler) GERMAN\WORDLIST.0?? GERMAN\$^. 0

!ifneq User Jurie

# Copy for non-Juries

$(Pdir)\XLDLIBS\GERMAN\SYSTEXTS : $(Pdir2)\XLDLIBS\GERMAN\SYSTEXTS
	copy $(Pdir2)\XLDLIBS\GERMAN\SYSTEXTS $(Pdir)\XLDLIBS\GERMAN\SYSTEXTS

!endif

