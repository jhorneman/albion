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
Makefile = $(Pdir)\XLDINFO\ALBION05.MAK

!else

# Paths for non-Juries
Pdir = C:\ALBION
Pdir2 = U:\PROJECT\ALBION\ALBION
Ddir = U:\PROJECT\ALBION\CENTRAL.DAT
Compiler = $(Pdir2)\XLDINFO\COMPILE.EXE
Makefile = $(Pdir2)\XLDINFO\ALBION05.MAK

!endif

# Define files
XLD_files = $(Pdir)\XLDLIBS\PRTCHAR0.XLD &
	$(Pdir)\XLDLIBS\NPCCHAR0.XLD &
	$(Pdir)\XLDLIBS\MONCHAR0.XLD &
	$(Pdir)\XLDLIBS\MONGRP0.XLD &
	$(Pdir)\XLDLIBS\EVNTSET0.XLD &
	$(Pdir)\XLDLIBS\EVNTTXT0.XLD &
	$(Pdir)\XLDLIBS\CHESTDT0.XLD &
	$(Pdir)\XLDLIBS\MERCHDT0.XLD

Extra_files = $(Compiler) $(Makefile)

# This is the main "target"

ALL : $(XLD_files)

# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\PRTCHAR0.XLD : $(Ddir)\PARTCHAR\PRT__DAT.??? $(Extra_files)
	$(Compiler) PARTCHAR

$(Pdir)\XLDLIBS\NPCCHAR0.XLD : $(Ddir)\NPC_CHAR\NPC__DAT.??? $(Extra_files)
	$(Compiler) NPCCHAR

$(Pdir)\XLDLIBS\MONCHAR0.XLD : $(Ddir)\MON_CHAR\MON__DAT.??? $(Extra_files)
	$(Compiler) MONCHAR

$(Pdir)\XLDLIBS\MONGRP0.XLD : $(Ddir)\MONGROUP\MONGROUP.??? $(Extra_files)
	$(Compiler) MONGROUP

$(Pdir)\XLDLIBS\EVNTSET0.XLD : $(Ddir)\EVENTSET\EVENT.??? $(Extra_files)
	$(Compiler) EVENTSET

$(Pdir)\XLDLIBS\EVNTTXT0.XLD : $(Ddir)\EVENTTXT\EVENTTXT.??? $(Extra_files)
	$(Compiler) EVENTTXT

$(Pdir)\XLDLIBS\CHESTDT0.XLD : $(Ddir)\CHESTDAT\CHESTDAT.??? $(Extra_files)
	$(Compiler) CHESTDAT

$(Pdir)\XLDLIBS\MERCHDT0.XLD : $(Ddir)\MERCHDAT\WARESDAT.??? $(Extra_files)
	$(Compiler) MERCHDAT

