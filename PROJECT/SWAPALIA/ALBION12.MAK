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
Makefile = U:\PROJECT\ALBION\SWAP\ALBION12.MAK

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
XLD_files = $(Pdir)\XLDLIBS\NPCGR0.XLD &
	$(Pdir)\XLDLIBS\NPCGR1.XLD &
	$(Pdir)\XLDLIBS\NPCKL0.XLD &
	$(Pdir)\XLDLIBS\SMLPORT0.XLD &
	$(Pdir)\XLDLIBS\SMLPORT1.XLD &
	$(Pdir)\XLDLIBS\TACTICO0.XLD


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\NPCGR0.XLD : $(Ddir)\NPCGR\NPC2DG.0?? $(Extra_files)
	$(Compiler) NPCGR\NPC2DG.0?? $^. 0

$(Pdir)\XLDLIBS\NPCGR1.XLD : $(Ddir)\NPCGR\NPC2DG.1?? $(Extra_files)
	$(Compiler) NPCGR\NPC2DG.1?? $^. 1

$(Pdir)\XLDLIBS\NPCKL0.XLD : $(Ddir)\NPCKL\NPC2DK.0?? $(Extra_files)
	$(Compiler) NPCKL\NPC2DK.0?? $^. 0

$(Pdir)\XLDLIBS\SMLPORT0.XLD : $(Ddir)\PORTRAIT\PORTRAIT.0?? $(Extra_files)
	$(Compiler) PORTRAIT\PORTRAIT.0?? $^. 0

$(Pdir)\XLDLIBS\SMLPORT1.XLD : $(Ddir)\PORTRAIT\PORTRAIT.1?? $(Extra_files)
	$(Compiler) PORTRAIT\PORTRAIT.1?? $^. 1

$(Pdir)\XLDLIBS\TACTICO0.XLD : $(Ddir)\TACTICON\TACTICON.0?? $(Extra_files)
	$(Compiler) TACTICON\TACTICON.0?? $^. 0

