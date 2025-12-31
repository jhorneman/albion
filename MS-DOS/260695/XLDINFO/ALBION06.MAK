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
Makefile = $(Pdir)\XLDINFO\ALBION06.MAK

!else

# Paths for non-Juries
Pdir = C:\ALBION
Pdir2 = U:\PROJECT\ALBION\ALBION
Ddir = U:\PROJECT\ALBION\CENTRAL.DAT
Compiler = $(Pdir2)\XLDINFO\COMPILE.EXE
Makefile = $(Pdir2)\XLDINFO\ALBION06.MAK

!endif

# Define files
XLD_files = $(Pdir)\XLDLIBS\LABDATA0.XLD &
	$(Pdir)\XLDLIBS\3DWALLS0.XLD &
	$(Pdir)\XLDLIBS\3DWALLS1.XLD &
	$(Pdir)\XLDLIBS\3DOBJEC0.XLD &
	$(Pdir)\XLDLIBS\3DOBJEC1.XLD &
	$(Pdir)\XLDLIBS\3DOBJEC2.XLD

Extra_files = $(Compiler) $(Makefile)

# This is the main "target"

ALL : $(XLD_files)

# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\LABDATA0.XLD : $(Ddir)\LAB_DATA\LAB__INF.??? $(Extra_files)
	$(Compiler) LAB_DATA

$(Pdir)\XLDLIBS\3DWALLS0.XLD : $(Ddir)\3DWALLS\WAL0??.BPX $(Extra_files)
	$(Compiler) 3DWALLS 0

$(Pdir)\XLDLIBS\3DWALLS1.XLD : $(Ddir)\3DWALLS\WAL1??.BPX $(Extra_files)
	$(Compiler) 3DWALLS 1

$(Pdir)\XLDLIBS\3DOBJEC0.XLD : $(Ddir)\3DOBJECT\OBJ0??.BPX $(Extra_files)
	$(Compiler) 3DOBJECT 0

$(Pdir)\XLDLIBS\3DOBJEC1.XLD : $(Ddir)\3DOBJECT\OBJ1??.BPX $(Extra_files)
	$(Compiler) 3DOBJECT 1

$(Pdir)\XLDLIBS\3DOBJEC2.XLD : $(Ddir)\3DOBJECT\OBJ2??.BPX $(Extra_files)
	$(Compiler) 3DOBJECT 2

