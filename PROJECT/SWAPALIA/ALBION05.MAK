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
Makefile = U:\PROJECT\ALBION\SWAP\ALBION05.MAK

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
XLD_files = $(Pdir)\XLDLIBS\INITIAL\PRTCHAR0.XLD &
	$(Pdir)\XLDLIBS\INITIAL\PRTCHAR1.XLD &
	$(Pdir)\XLDLIBS\INITIAL\PRTCHAR2.XLD &
	$(Pdir)\XLDLIBS\INITIAL\NPCCHAR0.XLD &
	$(Pdir)\XLDLIBS\INITIAL\NPCCHAR1.XLD &
	$(Pdir)\XLDLIBS\INITIAL\NPCCHAR2.XLD &
	$(Pdir)\XLDLIBS\INITIAL\CHESTDT0.XLD &
	$(Pdir)\XLDLIBS\INITIAL\CHESTDT1.XLD &
	$(Pdir)\XLDLIBS\INITIAL\CHESTDT2.XLD &
	$(Pdir)\XLDLIBS\INITIAL\CHESTDT5.XLD &
	$(Pdir)\XLDLIBS\INITIAL\MERCHDT0.XLD &
	$(Pdir)\XLDLIBS\INITIAL\MERCHDT1.XLD &
	$(Pdir)\XLDLIBS\INITIAL\MERCHDT2.XLD


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\INITIAL\PRTCHAR0.XLD : $(Ddir)\PARTCHAR\PRT__DAT.0?? $(Extra_files)
	$(Compiler) PARTCHAR\PRT__DAT.0?? INITIAL\$^. 0

$(Pdir)\XLDLIBS\INITIAL\PRTCHAR1.XLD : $(Ddir)\PARTCHAR\PRT__DAT.1?? $(Extra_files)
	$(Compiler) PARTCHAR\PRT__DAT.1?? INITIAL\$^. 1

$(Pdir)\XLDLIBS\INITIAL\PRTCHAR2.XLD : $(Ddir)\PARTCHAR\PRT__DAT.2?? $(Extra_files)
	$(Compiler) PARTCHAR\PRT__DAT.2?? INITIAL\$^. 2

$(Pdir)\XLDLIBS\INITIAL\NPCCHAR0.XLD : $(Ddir)\NPC_CHAR\NPC__DAT.0?? $(Extra_files)
	$(Compiler) NPC_CHAR\NPC__DAT.0?? INITIAL\$^. 0

$(Pdir)\XLDLIBS\INITIAL\NPCCHAR1.XLD : $(Ddir)\NPC_CHAR\NPC__DAT.1?? $(Extra_files)
	$(Compiler) NPC_CHAR\NPC__DAT.1?? INITIAL\$^. 1

$(Pdir)\XLDLIBS\INITIAL\NPCCHAR2.XLD : $(Ddir)\NPC_CHAR\NPC__DAT.2?? $(Extra_files)
	$(Compiler) NPC_CHAR\NPC__DAT.2?? INITIAL\$^. 2

$(Pdir)\XLDLIBS\INITIAL\CHESTDT0.XLD : $(Ddir)\CHESTDAT\CHESTDAT.0?? $(Extra_files)
	$(Compiler) CHESTDAT\CHESTDAT.0?? INITIAL\$^. 0

$(Pdir)\XLDLIBS\INITIAL\CHESTDT1.XLD : $(Ddir)\CHESTDAT\CHESTDAT.1?? $(Extra_files)
	$(Compiler) CHESTDAT\CHESTDAT.1?? INITIAL\$^. 1

$(Pdir)\XLDLIBS\INITIAL\CHESTDT2.XLD : $(Ddir)\CHESTDAT\CHESTDAT.2?? $(Extra_files)
	$(Compiler) CHESTDAT\CHESTDAT.2?? INITIAL\$^. 2

$(Pdir)\XLDLIBS\INITIAL\CHESTDT5.XLD : $(Ddir)\CHESTDAT\CHESTDAT.5?? $(Extra_files)
	$(Compiler) CHESTDAT\CHESTDAT.5?? INITIAL\$^. 5

$(Pdir)\XLDLIBS\INITIAL\MERCHDT0.XLD : $(Ddir)\MERCHDAT\WARESDAT.0?? $(Extra_files)
	$(Compiler) MERCHDAT\WARESDAT.0?? INITIAL\$^. 0

$(Pdir)\XLDLIBS\INITIAL\MERCHDT1.XLD : $(Ddir)\MERCHDAT\WARESDAT.1?? $(Extra_files)
	$(Compiler) MERCHDAT\WARESDAT.1?? INITIAL\$^. 1

$(Pdir)\XLDLIBS\INITIAL\MERCHDT2.XLD : $(Ddir)\MERCHDAT\WARESDAT.2?? $(Extra_files)
	$(Compiler) MERCHDAT\WARESDAT.2?? INITIAL\$^. 2

