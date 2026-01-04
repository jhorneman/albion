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
Makefile = U:\PROJECT\ALBION\SWAP\ALBION04.MAK

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
XLD_files = $(Pdir)\XLDLIBS\3DOVERL0.XLD &
	$(Pdir)\XLDLIBS\3DOVERL1.XLD &
	$(Pdir)\XLDLIBS\3DOVERL2.XLD &
	$(Pdir)\XLDLIBS\3DFLOOR0.XLD &
	$(Pdir)\XLDLIBS\3DFLOOR1.XLD &
	$(Pdir)\XLDLIBS\3DFLOOR2.XLD


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\3DOVERL0.XLD : $(Ddir)\3DOVERL\OVE0??.BPX $(Extra_files)
	$(Compiler) 3DOVERL\OVE0??.BPX $^. 0

$(Pdir)\XLDLIBS\3DOVERL1.XLD : $(Ddir)\3DOVERL\OVE1??.BPX $(Extra_files)
	$(Compiler) 3DOVERL\OVE1??.BPX $^. 1

$(Pdir)\XLDLIBS\3DOVERL2.XLD : $(Ddir)\3DOVERL\OVE2??.BPX $(Extra_files)
	$(Compiler) 3DOVERL\OVE2??.BPX $^. 2

$(Pdir)\XLDLIBS\3DFLOOR0.XLD : $(Ddir)\3DFLOORS\FLO0??.BPX $(Extra_files)
	$(Compiler) 3DFLOORS\FLO0??.BPX $^. 0

$(Pdir)\XLDLIBS\3DFLOOR1.XLD : $(Ddir)\3DFLOORS\FLO1??.BPX $(Extra_files)
	$(Compiler) 3DFLOORS\FLO1??.BPX $^. 1

$(Pdir)\XLDLIBS\3DFLOOR2.XLD : $(Ddir)\3DFLOORS\FLO2??.BPX $(Extra_files)
	$(Compiler) 3DFLOORS\FLO2??.BPX $^. 2

