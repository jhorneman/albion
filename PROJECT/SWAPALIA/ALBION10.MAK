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
Makefile = U:\PROJECT\ALBION\SWAP\ALBION10.MAK

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
XLD_files = $(Pdir)\XLDLIBS\FONTS0.XLD &
	$(Pdir)\XLDLIBS\PALETTE0.XLD &
	$(Pdir)\XLDLIBS\PARTGR0.XLD &
	$(Pdir)\XLDLIBS\PARTKL0.XLD &
	$(Pdir)\XLDLIBS\MONGFX0.XLD &
	$(Pdir)\XLDLIBS\3DBCKGR0.XLD &
	$(Pdir)\XLDLIBS\COMBACK0.XLD &
	$(Pdir)\XLDLIBS\COMGFX0.XLD &
	$(Pdir)\XLDLIBS\FBODPIX0.XLD &
	$(Pdir)\XLDLIBS\SONGS0.XLD &
	$(Pdir)\XLDLIBS\WAVELIB0.XLD &
	$(Pdir)\XLDLIBS\AUTOGFX0.XLD &
	$(Pdir)\XLDLIBS\TRANSTB0.XLD


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\FONTS0.XLD : $(Ddir)\FONTS\FONT.0?? $(Extra_files)
	$(Compiler) FONTS\FONT.0?? $^. 0

$(Pdir)\XLDLIBS\PALETTE0.XLD : $(Ddir)\PALETTES\0??.PAL $(Extra_files)
	$(Compiler) PALETTES\0??.PAL $^. 0

$(Pdir)\XLDLIBS\PARTGR0.XLD : $(Ddir)\PARTGR\PARTGR.0?? $(Extra_files)
	$(Compiler) PARTGR\PARTGR.0?? $^. 0

$(Pdir)\XLDLIBS\PARTKL0.XLD : $(Ddir)\PARTKL\PARTKL.0?? $(Extra_files)
	$(Compiler) PARTKL\PARTKL.0?? $^. 0

$(Pdir)\XLDLIBS\MONGFX0.XLD : $(Ddir)\MONGFX\MONGFX.0?? $(Extra_files)
	$(Compiler) MONGFX\MONGFX.0?? $^. 0

$(Pdir)\XLDLIBS\3DBCKGR0.XLD : $(Ddir)\3DBACKGR\3DBACKGR.0?? $(Extra_files)
	$(Compiler) 3DBACKGR\3DBACKGR.0?? $^. 0

$(Pdir)\XLDLIBS\COMBACK0.XLD : $(Ddir)\COMBACK\COMBACK.0?? $(Extra_files)
	$(Compiler) COMBACK\COMBACK.0?? $^. 0

$(Pdir)\XLDLIBS\COMGFX0.XLD : $(Ddir)\COMGFX\COMGFX.0?? $(Extra_files)
	$(Compiler) COMGFX\COMGFX.0?? $^. 0

$(Pdir)\XLDLIBS\FBODPIX0.XLD : $(Ddir)\FBODPICS\FBODPIC.0?? $(Extra_files)
	$(Compiler) FBODPICS\FBODPIC.0?? $^. 0

$(Pdir)\XLDLIBS\SONGS0.XLD : $(Ddir)\SONGS\SONG0??.XMI $(Extra_files)
	$(Compiler) SONGS\SONG0??.XMI $^. 0

$(Pdir)\XLDLIBS\WAVELIB0.XLD : $(Ddir)\WAVELIBS\WAVE0??.WVL $(Extra_files)
	$(Compiler) WAVELIBS\WAVE0??.WVL $^. 0

$(Pdir)\XLDLIBS\AUTOGFX0.XLD : $(Ddir)\AUTOMGFX\AUTOMGFX.0?? $(Extra_files)
	$(Compiler) AUTOMGFX\AUTOMGFX.0?? $^. 0

$(Pdir)\XLDLIBS\TRANSTB0.XLD : $(Ddir)\TRANSTAB\TRANSTAB.0?? $(Extra_files)
	$(Compiler) TRANSTAB\TRANSTAB.0?? $^. 0

