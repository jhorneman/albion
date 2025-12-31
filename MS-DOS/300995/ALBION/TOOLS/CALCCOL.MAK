# Makefile for Albion data

# Ignore return values and do not check if target was made
.IGNORE

# Do not output commands
.SILENT


# Paths
Sdir = G:\ALBION\DATA\PALETTES
Tdir = G:\ALBION\DATA\TRANSTAB


# Define colour calculator and makefile
Calccol = G:\ALBION\TOOLS\CALCCOL.EXE
Makefile = G:\ALBION\TOOLS\CALCCOL.MAK


# Define files
Files = $(Tdir)\TRANSTAB.023 &
	$(Tdir)\TRANSTAB.024 &
	$(Tdir)\TRANSTAB.027 &
	$(Tdir)\TRANSTAB.032 &
	$(Tdir)\TRANSTAB.033 &
	$(Tdir)\TRANSTAB.034 &
	$(Tdir)\TRANSTAB.035 &
	$(Tdir)\TRANSTAB.036 &
	$(Tdir)\TRANSTAB.037 &
	$(Tdir)\TRANSTAB.038 &
	$(Tdir)\TRANSTAB.039 &
	$(Tdir)\TRANSTAB.040 &
	$(Tdir)\TRANSTAB.041 &
	$(Tdir)\TRANSTAB.042 &
	$(Tdir)\TRANSTAB.043 &
	$(Tdir)\TRANSTAB.044

Extra_files = $(Calccol) $(Makefile)


# This is the main "target"

ALL : $(Files)


# These are the transparency tables

$(Tdir)\TRANSTAB.023 : $(Sdir)\023.PAL $(Extra_files)
	$(Calccol) $(Sdir)\023.PAL $(Tdir)\TRANSTAB.023

$(Tdir)\TRANSTAB.024 : $(Sdir)\024.PAL $(Extra_files)
	$(Calccol) $(Sdir)\024.PAL $(Tdir)\TRANSTAB.024

$(Tdir)\TRANSTAB.027 : $(Sdir)\027.PAL $(Extra_files)
	$(Calccol) $(Sdir)\027.PAL $(Tdir)\TRANSTAB.027

$(Tdir)\TRANSTAB.032 : $(Sdir)\032.PAL $(Extra_files)
	$(Calccol) $(Sdir)\032.PAL $(Tdir)\TRANSTAB.032

$(Tdir)\TRANSTAB.033 : $(Sdir)\033.PAL $(Extra_files)
	$(Calccol) $(Sdir)\033.PAL $(Tdir)\TRANSTAB.033

$(Tdir)\TRANSTAB.034 : $(Sdir)\034.PAL $(Extra_files)
	$(Calccol) $(Sdir)\034.PAL $(Tdir)\TRANSTAB.034

$(Tdir)\TRANSTAB.035 : $(Sdir)\035.PAL $(Extra_files)
	$(Calccol) $(Sdir)\035.PAL $(Tdir)\TRANSTAB.035

$(Tdir)\TRANSTAB.036 : $(Sdir)\036.PAL $(Extra_files)
	$(Calccol) $(Sdir)\036.PAL $(Tdir)\TRANSTAB.036

$(Tdir)\TRANSTAB.037 : $(Sdir)\037.PAL $(Extra_files)
	$(Calccol) $(Sdir)\037.PAL $(Tdir)\TRANSTAB.037

$(Tdir)\TRANSTAB.038 : $(Sdir)\038.PAL $(Extra_files)
	$(Calccol) $(Sdir)\038.PAL $(Tdir)\TRANSTAB.038

$(Tdir)\TRANSTAB.039 : $(Sdir)\039.PAL $(Extra_files)
	$(Calccol) $(Sdir)\039.PAL $(Tdir)\TRANSTAB.039

$(Tdir)\TRANSTAB.040 : $(Sdir)\040.PAL $(Extra_files)
	$(Calccol) $(Sdir)\040.PAL $(Tdir)\TRANSTAB.040

$(Tdir)\TRANSTAB.041 : $(Sdir)\041.PAL $(Extra_files)
	$(Calccol) $(Sdir)\041.PAL $(Tdir)\TRANSTAB.041

$(Tdir)\TRANSTAB.042 : $(Sdir)\042.PAL $(Extra_files)
	$(Calccol) $(Sdir)\042.PAL $(Tdir)\TRANSTAB.042

$(Tdir)\TRANSTAB.043 : $(Sdir)\043.PAL $(Extra_files)
	$(Calccol) $(Sdir)\043.PAL $(Tdir)\TRANSTAB.043

$(Tdir)\TRANSTAB.044 : $(Sdir)\044.PAL $(Extra_files)
	$(Calccol) $(Sdir)\044.PAL $(Tdir)\TRANSTAB.044

