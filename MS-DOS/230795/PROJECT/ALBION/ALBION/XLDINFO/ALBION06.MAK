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
Makefile = $(Idir)\ALBION06.MAK


# Define files
XLD_files = $(Pdir)\XLDLIBS\LABDATA0.XLD &
	$(Pdir)\XLDLIBS\LABDATA1.XLD &
	$(Pdir)\XLDLIBS\LABDATA2.XLD &
	$(Pdir)\XLDLIBS\3DWALLS0.XLD &
	$(Pdir)\XLDLIBS\3DWALLS1.XLD &
	$(Pdir)\XLDLIBS\3DOBJEC0.XLD &
	$(Pdir)\XLDLIBS\3DOBJEC1.XLD &
	$(Pdir)\XLDLIBS\3DOBJEC2.XLD

Extra_files = $(Compiler) $(Makefile)


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\LABDATA0.XLD : $(Ddir)\LAB_DATA\LAB__INF.0?? $(Extra_files)
	$(Compiler) $(Idir)\LAB_DATA 0

$(Pdir)\XLDLIBS\LABDATA1.XLD : $(Ddir)\LAB_DATA\LAB__INF.1?? $(Extra_files)
	$(Compiler) $(Idir)\LAB_DATA 1

$(Pdir)\XLDLIBS\LABDATA2.XLD : $(Ddir)\LAB_DATA\LAB__INF.2?? $(Extra_files)
	$(Compiler) $(Idir)\LAB_DATA 2

$(Pdir)\XLDLIBS\3DWALLS0.XLD : $(Ddir)\3DWALLS\WAL0??.BPX $(Extra_files)
	$(Compiler) $(Idir)\3DWALLS 0

$(Pdir)\XLDLIBS\3DWALLS1.XLD : $(Ddir)\3DWALLS\WAL1??.BPX $(Extra_files)
	$(Compiler) $(Idir)\3DWALLS 1

$(Pdir)\XLDLIBS\3DOBJEC0.XLD : $(Ddir)\3DOBJECT\OBJ0??.BPX $(Extra_files)
	$(Compiler) $(Idir)\3DOBJECT 0

$(Pdir)\XLDLIBS\3DOBJEC1.XLD : $(Ddir)\3DOBJECT\OBJ1??.BPX $(Extra_files)
	$(Compiler) $(Idir)\3DOBJECT 1

$(Pdir)\XLDLIBS\3DOBJEC2.XLD : $(Ddir)\3DOBJECT\OBJ2??.BPX $(Extra_files)
	$(Compiler) $(Idir)\3DOBJECT 2

