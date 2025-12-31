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
Makefile = $(Idir)\ALBION05.MAK
Language = GERMAN


# Define files
XLD_files = $(Pdir)\XLDLIBS\INITIAL\PRTCHAR0.XLD &
	$(Pdir)\XLDLIBS\INITIAL\PRTCHAR1.XLD &
	$(Pdir)\XLDLIBS\INITIAL\PRTCHAR2.XLD &
	$(Pdir)\XLDLIBS\INITIAL\NPCCHAR0.XLD &
	$(Pdir)\XLDLIBS\INITIAL\NPCCHAR1.XLD &
	$(Pdir)\XLDLIBS\INITIAL\NPCCHAR2.XLD &
	$(Pdir)\XLDLIBS\MONCHAR0.XLD &
	$(Pdir)\XLDLIBS\MONCHAR1.XLD &
	$(Pdir)\XLDLIBS\MONCHAR2.XLD &
	$(Pdir)\XLDLIBS\MONGRP0.XLD &
	$(Pdir)\XLDLIBS\MONGRP1.XLD &
	$(Pdir)\XLDLIBS\MONGRP2.XLD &
	$(Pdir)\XLDLIBS\EVNTSET0.XLD &
	$(Pdir)\XLDLIBS\EVNTSET1.XLD &
	$(Pdir)\XLDLIBS\EVNTSET2.XLD &
	$(Pdir)\XLDLIBS\EVNTSET9.XLD &
	$(Pdir)\XLDLIBS\$(Language)\EVNTTXT0.XLD &
	$(Pdir)\XLDLIBS\$(Language)\EVNTTXT1.XLD &
	$(Pdir)\XLDLIBS\$(Language)\EVNTTXT2.XLD &
	$(Pdir)\XLDLIBS\$(Language)\EVNTTXT9.XLD &
	$(Pdir)\XLDLIBS\INITIAL\CHESTDT0.XLD &
	$(Pdir)\XLDLIBS\INITIAL\CHESTDT1.XLD &
	$(Pdir)\XLDLIBS\INITIAL\CHESTDT2.XLD &
	$(Pdir)\XLDLIBS\INITIAL\MERCHDT0.XLD &
	$(Pdir)\XLDLIBS\INITIAL\MERCHDT1.XLD &
	$(Pdir)\XLDLIBS\INITIAL\MERCHDT2.XLD &
	$(Pdir)\XLDLIBS\$(Language)\WORDLIS0.XLD

Extra_files = $(Compiler) $(Makefile)


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\INITIAL\PRTCHAR0.XLD : $(Ddir)\PARTCHAR\PRT__DAT.0?? $(Extra_files)
	$(Compiler) $(Idir)\PARTCHAR 0

$(Pdir)\XLDLIBS\INITIAL\PRTCHAR1.XLD : $(Ddir)\PARTCHAR\PRT__DAT.1?? $(Extra_files)
	$(Compiler) $(Idir)\PARTCHAR 1

$(Pdir)\XLDLIBS\INITIAL\PRTCHAR2.XLD : $(Ddir)\PARTCHAR\PRT__DAT.2?? $(Extra_files)
	$(Compiler) $(Idir)\PARTCHAR 2

$(Pdir)\XLDLIBS\INITIAL\NPCCHAR0.XLD : $(Ddir)\NPC_CHAR\NPC__DAT.0?? $(Extra_files)
	$(Compiler) $(Idir)\NPCCHAR 0

$(Pdir)\XLDLIBS\INITIAL\NPCCHAR1.XLD : $(Ddir)\NPC_CHAR\NPC__DAT.1?? $(Extra_files)
	$(Compiler) $(Idir)\NPCCHAR 1

$(Pdir)\XLDLIBS\INITIAL\NPCCHAR2.XLD : $(Ddir)\NPC_CHAR\NPC__DAT.2?? $(Extra_files)
	$(Compiler) $(Idir)\NPCCHAR 2

$(Pdir)\XLDLIBS\MONCHAR0.XLD : $(Ddir)\MON_CHAR\MON__DAT.0?? $(Extra_files)
	$(Compiler) $(Idir)\MONCHAR 0

$(Pdir)\XLDLIBS\MONCHAR1.XLD : $(Ddir)\MON_CHAR\MON__DAT.1?? $(Extra_files)
	$(Compiler) $(Idir)\MONCHAR 1

$(Pdir)\XLDLIBS\MONCHAR2.XLD : $(Ddir)\MON_CHAR\MON__DAT.2?? $(Extra_files)
	$(Compiler) $(Idir)\MONCHAR 2

$(Pdir)\XLDLIBS\MONGRP0.XLD : $(Ddir)\MONGROUP\MONGROUP.0?? $(Extra_files)
	$(Compiler) $(Idir)\MONGROUP 0

$(Pdir)\XLDLIBS\MONGRP1.XLD : $(Ddir)\MONGROUP\MONGROUP.1?? $(Extra_files)
	$(Compiler) $(Idir)\MONGROUP 1

$(Pdir)\XLDLIBS\MONGRP2.XLD : $(Ddir)\MONGROUP\MONGROUP.2?? $(Extra_files)
	$(Compiler) $(Idir)\MONGROUP 2

$(Pdir)\XLDLIBS\EVNTSET0.XLD : $(Ddir)\EVENTSET\EVENT.0?? $(Extra_files)
	$(Compiler) $(Idir)\EVENTSET 0

$(Pdir)\XLDLIBS\EVNTSET1.XLD : $(Ddir)\EVENTSET\EVENT.1?? $(Extra_files)
	$(Compiler) $(Idir)\EVENTSET 1

$(Pdir)\XLDLIBS\EVNTSET2.XLD : $(Ddir)\EVENTSET\EVENT.2?? $(Extra_files)
	$(Compiler) $(Idir)\EVENTSET 2

$(Pdir)\XLDLIBS\EVNTSET9.XLD : $(Ddir)\EVENTSET\EVENT.9?? $(Extra_files)
	$(Compiler) $(Idir)\EVENTSET 9

$(Pdir)\XLDLIBS\$(Language)\EVNTTXT0.XLD : $(Ddir)\EVENTTXT\EVT_00??.CMP $(Extra_files)
	$(Compiler) $(Idir)\EVENTTXT 0

$(Pdir)\XLDLIBS\$(Language)\EVNTTXT1.XLD : $(Ddir)\EVENTTXT\EVT_01??.CMP $(Extra_files)
	$(Compiler) $(Idir)\EVENTTXT 1

$(Pdir)\XLDLIBS\$(Language)\EVNTTXT2.XLD : $(Ddir)\EVENTTXT\EVT_02??.CMP $(Extra_files)
	$(Compiler) $(Idir)\EVENTTXT 2

$(Pdir)\XLDLIBS\$(Language)\EVNTTXT9.XLD : $(Ddir)\EVENTTXT\EVT_09??.CMP $(Extra_files)
	$(Compiler) $(Idir)\EVENTTXT 9

$(Pdir)\XLDLIBS\INITIAL\CHESTDT0.XLD : $(Ddir)\CHESTDAT\CHESTDAT.0?? $(Extra_files)
	$(Compiler) $(Idir)\CHESTDAT 0

$(Pdir)\XLDLIBS\INITIAL\CHESTDT1.XLD : $(Ddir)\CHESTDAT\CHESTDAT.1?? $(Extra_files)
	$(Compiler) $(Idir)\CHESTDAT 1

$(Pdir)\XLDLIBS\INITIAL\CHESTDT2.XLD : $(Ddir)\CHESTDAT\CHESTDAT.2?? $(Extra_files)
	$(Compiler) $(Idir)\CHESTDAT 2

$(Pdir)\XLDLIBS\INITIAL\MERCHDT0.XLD : $(Ddir)\MERCHDAT\WARESDAT.0?? $(Extra_files)
	$(Compiler) $(Idir)\MERCHDAT 0

$(Pdir)\XLDLIBS\INITIAL\MERCHDT1.XLD : $(Ddir)\MERCHDAT\WARESDAT.1?? $(Extra_files)
	$(Compiler) $(Idir)\MERCHDAT 1

$(Pdir)\XLDLIBS\INITIAL\MERCHDT2.XLD : $(Ddir)\MERCHDAT\WARESDAT.2?? $(Extra_files)
	$(Compiler) $(Idir)\MERCHDAT 2

$(Pdir)\XLDLIBS\$(Language)\WORDLIS0.XLD : $(Ddir)\WORDLIST.0?? $(Extra_files)
	$(Compiler) $(Idir)\WORDLIST 0

