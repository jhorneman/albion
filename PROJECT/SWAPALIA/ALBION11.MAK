# Makefile for Albion DEMO data

# Erase targets on error
.ERASE

# Do not output commands
.SILENT

# Default user is Jurie
!ifndef User
User = Jurie
!endif

# Define paths
Ddir = U:\PROJECT\ALBION\DEMO.DAT

# Define compiler and makefile
Compiler = U:\PROJECT\ALBION\SWAP\COMPILE.EXE
Makefile = U:\PROJECT\ALBION\SWAP\ALBION11.MAK

Extra_files = $(Compiler) $(Makefile)


!ifeq User Jurie

# Paths for Jurie
Pdir = U:\PROJECT\ALBION\ALBIDEMO


# Set environment variables for compiler (only during make)
.BEFORE
	set XLD_SRC_PATH=$(Ddir)
	set XLD_LIB_PATH=$(Pdir)\XLDLIBS


# Define files
XLD_files = $(Pdir)\XLDLIBS\MAPDATA1.XLD &
	$(Pdir)\XLDLIBS\MAPDATA2.XLD &
	$(Pdir)\XLDLIBS\MAPDATA3.XLD &
	$(Pdir)\XLDLIBS\FLICS0.XLD &
	$(Pdir)\XLDLIBS\GERMAN\MAPTEXT1.XLD &
	$(Pdir)\XLDLIBS\GERMAN\MAPTEXT2.XLD &
	$(Pdir)\XLDLIBS\GERMAN\MAPTEXT3.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT0.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT1.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT2.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT3.XLD &
	$(Pdir)\XLDLIBS\GERMAN\EVNTTXT9.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\MAPTEXT1.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\MAPTEXT2.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\MAPTEXT3.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT0.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT1.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT2.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT3.XLD &
	$(Pdir)\XLDLIBS\ENGLISH\EVNTTXT9.XLD &
	$(Pdir)\XLDLIBS\ICONDAT0.XLD &
	$(Pdir)\XLDLIBS\ICONGFX0.XLD &
	$(Pdir)\XLDLIBS\BLKLIST0.XLD &
	$(Pdir)\XLDLIBS\PICTURE0.XLD


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\MAPDATA1.XLD : $(Ddir)\MAP_DATA\MAP_01??.DAT $(Extra_files)
	$(Compiler) MAP_DATA\MAP_01??.DAT $^. 1

$(Pdir)\XLDLIBS\MAPDATA2.XLD : $(Ddir)\MAP_DATA\MAP_02??.DAT $(Extra_files)
	$(Compiler) MAP_DATA\MAP_02??.DAT $^. 2

$(Pdir)\XLDLIBS\MAPDATA3.XLD : $(Ddir)\MAP_DATA\MAP_03??.DAT $(Extra_files)
	$(Compiler) MAP_DATA\MAP_03??.DAT $^. 3

$(Pdir)\XLDLIBS\FLICS0.XLD : $(Ddir)\FLICS\FLIC0??.FLC $(Extra_files)
	$(Compiler) FLICS\FLIC0??.FLC $^. 0

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

$(Pdir)\XLDLIBS\ICONDAT0.XLD : $(Ddir)\ICONDATA\ICONLIST.0?? $(Extra_files)
	$(Compiler) ICONDATA\ICONLIST.0?? $^. 0

$(Pdir)\XLDLIBS\ICONGFX0.XLD : $(Ddir)\ICON_GFX\JURIE_GR.0?? $(Extra_files)
	$(Compiler) ICON_GFX\JURIE_GR.0?? $^. 0

$(Pdir)\XLDLIBS\BLKLIST0.XLD : $(Ddir)\ICONDATA\CUTBLOCK.0?? $(Extra_files)
	$(Compiler) ICONDATA\CUTBLOCK.0?? $^. 0

$(Pdir)\XLDLIBS\PICTURE0.XLD : $(Ddir)\PICS\PICT0??.LBM $(Extra_files)
	$(Compiler) PICS\PICT0??.LBM $^. 0

!endif

