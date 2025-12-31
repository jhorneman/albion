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
Makefile = $(Idir)\ALBION04.MAK


# Define files
XLD_files = $(Pdir)\XLDLIBS\3DOVERL0.XLD &
	$(Pdir)\XLDLIBS\3DOVERL1.XLD &
	$(Pdir)\XLDLIBS\3DOVERL2.XLD &
	$(Pdir)\XLDLIBS\3DFLOOR0.XLD &
	$(Pdir)\XLDLIBS\3DFLOOR1.XLD &
	$(Pdir)\XLDLIBS\3DFLOOR2.XLD

Extra_files = $(Compiler) $(Makefile)


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\3DOVERL0.XLD : $(Ddir)\3DOVERL\OVE0??.BPX $(Extra_files)
	$(Compiler) $(Idir)\3DOVERL 0

$(Pdir)\XLDLIBS\3DOVERL1.XLD : $(Ddir)\3DOVERL\OVE1??.BPX $(Extra_files)
	$(Compiler) $(Idir)\3DOVERL 1

$(Pdir)\XLDLIBS\3DOVERL2.XLD : $(Ddir)\3DOVERL\OVE2??.BPX $(Extra_files)
	$(Compiler) $(Idir)\3DOVERL 2

$(Pdir)\XLDLIBS\3DFLOOR0.XLD : $(Ddir)\3DFLOORS\FLO0??.BPX $(Extra_files)
	$(Compiler) $(Idir)\3DFLOORS 0

$(Pdir)\XLDLIBS\3DFLOOR1.XLD : $(Ddir)\3DFLOORS\FLO1??.BPX $(Extra_files)
	$(Compiler) $(Idir)\3DFLOORS 1

$(Pdir)\XLDLIBS\3DFLOOR2.XLD : $(Ddir)\3DFLOORS\FLO2??.BPX $(Extra_files)
	$(Compiler) $(Idir)\3DFLOORS 2

