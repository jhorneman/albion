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
Makefile = U:\PROJECT\ALBION\SWAP\ALBION06.MAK

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
XLD_files = $(Pdir)\XLDLIBS\LABDATA0.XLD &
	$(Pdir)\XLDLIBS\LABDATA1.XLD &
	$(Pdir)\XLDLIBS\LABDATA2.XLD &
	$(Pdir)\XLDLIBS\3DWALLS0.XLD &
	$(Pdir)\XLDLIBS\3DWALLS1.XLD &
	$(Pdir)\XLDLIBS\3DOBJEC0.XLD &
	$(Pdir)\XLDLIBS\3DOBJEC1.XLD &
	$(Pdir)\XLDLIBS\3DOBJEC2.XLD &
	$(Pdir)\XLDLIBS\3DOBJEC3.XLD


# This is the main "target"

ALL : $(XLD_files)


# These are the XLD-files coming from the server

$(Pdir)\XLDLIBS\LABDATA0.XLD : $(Ddir)\LAB_DATA\LAB__INF.0?? $(Extra_files)
	$(Compiler) LAB_DATA\LAB__INF.0?? $^. 0

$(Pdir)\XLDLIBS\LABDATA1.XLD : $(Ddir)\LAB_DATA\LAB__INF.1?? $(Extra_files)
	$(Compiler) LAB_DATA\LAB__INF.1?? $^. 1

$(Pdir)\XLDLIBS\LABDATA2.XLD : $(Ddir)\LAB_DATA\LAB__INF.2?? $(Extra_files)
	$(Compiler) LAB_DATA\LAB__INF.2?? $^. 2

$(Pdir)\XLDLIBS\3DWALLS0.XLD : $(Ddir)\3DWALLS\WAL0??.BPX $(Extra_files)
	$(Compiler) 3DWALLS\WAL0??.BPX $^. 0

$(Pdir)\XLDLIBS\3DWALLS1.XLD : $(Ddir)\3DWALLS\WAL1??.BPX $(Extra_files)
	$(Compiler) 3DWALLS\WAL1??.BPX $^. 1

$(Pdir)\XLDLIBS\3DOBJEC0.XLD : $(Ddir)\3DOBJECT\OBJ0??.BPX $(Extra_files)
	$(Compiler) 3DOBJECT\OBJ0??.BPX $^. 0

$(Pdir)\XLDLIBS\3DOBJEC1.XLD : $(Ddir)\3DOBJECT\OBJ1??.BPX $(Extra_files)
	$(Compiler) 3DOBJECT\OBJ1??.BPX $^. 1

$(Pdir)\XLDLIBS\3DOBJEC2.XLD : $(Ddir)\3DOBJECT\OBJ2??.BPX $(Extra_files)
	$(Compiler) 3DOBJECT\OBJ2??.BPX $^. 2

$(Pdir)\XLDLIBS\3DOBJEC3.XLD : $(Ddir)\3DOBJECT\OBJ3??.BPX $(Extra_files)
	$(Compiler) 3DOBJECT\OBJ3??.BPX $^. 3

