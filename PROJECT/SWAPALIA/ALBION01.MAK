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
Makefile = U:\PROJECT\ALBION\SWAP\ALBION01.MAK

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
XLD_files = $(Pdir)\XLDLIBS\ICONDAT0.XLD &
	$(Pdir)\XLDLIBS\ICONGFX0.XLD &
	$(Pdir)\XLDLIBS\BLKLIST0.XLD &
	$(Pdir)\XLDLIBS\MONCHAR0.XLD &
	$(Pdir)\XLDLIBS\MONGRP0.XLD &
	$(Pdir)\XLDLIBS\MONGRP1.XLD &
	$(Pdir)\XLDLIBS\MONGRP2.XLD &
	$(Pdir)\XLDLIBS\SAMPLES0.XLD &
	$(Pdir)\XLDLIBS\SAMPLES1.XLD &
	$(Pdir)\XLDLIBS\SAMPLES2.XLD &
	$(Pdir)\XLDLIBS\EVNTSET0.XLD &
	$(Pdir)\XLDLIBS\EVNTSET1.XLD &
	$(Pdir)\XLDLIBS\EVNTSET2.XLD &
	$(Pdir)\XLDLIBS\EVNTSET3.XLD &
	$(Pdir)\XLDLIBS\EVNTSET9.XLD


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\ICONDAT0.XLD : $(Ddir)\ICONDATA\ICONLIST.0?? $(Extra_files)
	$(Compiler) ICONDATA\ICONLIST.0?? $^. 0

$(Pdir)\XLDLIBS\ICONGFX0.XLD : $(Ddir)\ICON_GFX\JURIE_GR.0?? $(Extra_files)
	$(Compiler) ICON_GFX\JURIE_GR.0?? $^. 0

$(Pdir)\XLDLIBS\BLKLIST0.XLD : $(Ddir)\ICONDATA\CUTBLOCK.0?? $(Extra_files)
	$(Compiler) ICONDATA\CUTBLOCK.0?? $^. 0

$(Pdir)\XLDLIBS\MONCHAR0.XLD : $(Ddir)\MON_CHAR\MON__DAT.0?? $(Extra_files)
	$(Compiler) MON_CHAR\MON__DAT.0?? $^. 0

$(Pdir)\XLDLIBS\MONGRP0.XLD : $(Ddir)\MONGROUP\MONGROUP.0?? $(Extra_files)
	$(Compiler) MONGROUP\MONGROUP.0?? $^. 0

$(Pdir)\XLDLIBS\MONGRP1.XLD : $(Ddir)\MONGROUP\MONGROUP.1?? $(Extra_files)
	$(Compiler) MONGROUP\MONGROUP.1?? $^. 1

$(Pdir)\XLDLIBS\MONGRP2.XLD : $(Ddir)\MONGROUP\MONGROUP.2?? $(Extra_files)
	$(Compiler) MONGROUP\MONGROUP.2?? $^. 2

$(Pdir)\XLDLIBS\SAMPLES0.XLD : $(Ddir)\SAMPLES\SAMPLE.0?? $(Extra_files)
	$(Compiler) SAMPLES\SAMPLE.0?? $^. 0

$(Pdir)\XLDLIBS\SAMPLES1.XLD : $(Ddir)\SAMPLES\SAMPLE.1?? $(Extra_files)
	$(Compiler) SAMPLES\SAMPLE.1?? $^. 1

$(Pdir)\XLDLIBS\SAMPLES2.XLD : $(Ddir)\SAMPLES\SAMPLE.2?? $(Extra_files)
	$(Compiler) SAMPLES\SAMPLE.2?? $^. 2

$(Pdir)\XLDLIBS\EVNTSET0.XLD : $(Ddir)\EVENTSET\EVENT.0?? $(Extra_files)
	$(Compiler) EVENTSET\EVENT.0?? $^. 0

$(Pdir)\XLDLIBS\EVNTSET1.XLD : $(Ddir)\EVENTSET\EVENT.1?? $(Extra_files)
	$(Compiler) EVENTSET\EVENT.1?? $^. 1

$(Pdir)\XLDLIBS\EVNTSET2.XLD : $(Ddir)\EVENTSET\EVENT.2?? $(Extra_files)
	$(Compiler) EVENTSET\EVENT.2?? $^. 2

$(Pdir)\XLDLIBS\EVNTSET3.XLD : $(Ddir)\EVENTSET\EVENT.3?? $(Extra_files)
	$(Compiler) EVENTSET\EVENT.3?? $^. 3

$(Pdir)\XLDLIBS\EVNTSET9.XLD : $(Ddir)\EVENTSET\EVENT.9?? $(Extra_files)
	$(Compiler) EVENTSET\EVENT.9?? $^. 9

