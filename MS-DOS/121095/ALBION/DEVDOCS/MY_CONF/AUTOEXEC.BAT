@echo off
SET PCTOOLS=D:\TOOLS\PCT\DATA
PROMPT $P$G
 rem call C:\AVGA2\UTILITY\AVGA2CFG.COM
C:\DOS\SUBST E: D:\E
C:\DOS\SUBST F: D:\F
if "%bootcon%" == "B" goto B
if "%bootcon%" == "C" goto C
if "%bootcon%" == "D" goto D
if "%bootcon%" == "E" goto E

:A
rem WINDOWS
 rem C:\QEMM\LOADHI /R:3 C:\DUBLDISK\DUBLDISK
 rem C:\DUBLDISK\DRVOFF.EXE
 rem C:\QEMM\LOADHI /R:1 C:\DOS\SHARE.EXE /L:500
@C:\WINDOWS\AD_WRAP.COM
PATH C:\WINDOWS;C:\BAT;C:\DOS;C:\;C:\DUBLDISK;C:\PROAUDIO;C:\TOOLS
SET TEMP=D:\TMP
SET NORTON=C:\TOOLS
C:\WINDOWS\SMARTDRV.EXE /X 2048 2048 /Q
C:\QEMM\LOADHI /R:1 KEYB GR,,C:\DOS\KEYBOARD.SYS
C:\QEMM\LOADHI /R:1 C:\DOS\MOUSE.COM
C:\QEMM\LOADHI /R:2 /RES=4112 /SQT=FA00-FAFF DOSKEY /BUFSIZE=128 /HISTORY /OVERSTRIKE
 rem e:\windows\AD-DOS.COM
C:\QEMM\LOADHI /R:1 C:\CDROM\MSCDEX /D:MSCD000 /L:G
goto EXIT

:B
rem COMMANCHE
 rem C:\DUBLDISK\DUBLDISK
 rem C:\DUBLDISK\DRVOFF
PATH C:\dubldisk;C:\DOS;C:\TOOLS;C:\PROAUDIO;C:\BAT;
KEYB GR,,C:\DOS\KEYBOARD.SYS
C:\DOS\MOUSE.COM
e:
CD \COMANCHE
C
C:\WINDOWS\MSCDEX.EXE /S /D:MSCD000 /L:G
goto EXIT

:C
rem STRIKE COMMANDER
PATH C:\DOS;C:\TOOLS;C:\PROAUDIO;C:\BAT;
KEYB GR,,C:\DOS\KEYBOARD.SYS
C:\DOS\MOUSE.COM >NUL
C:\WINDOWS\SMARTDRV.EXE 2048 /Q
D:
cd \games\strike
call sc
goto EXIT

:D
rem COMPRESS
PATH C:\dubldisk;C:\DOS;C:\TOOLS;
C:
KEYB GR,,C:\DOS\KEYBOARD.SYS

rem attrib -r -h -s dubldisk.*
attrib -r -a -h *.org
attrib -r -h    bootcon.sys
attrib -r -h    boot.tav
attrib -a -h -s 386spart.par

C:\DOS\MOUSE.COM
call COMPRESS

rem attrib +r +h +s dubldisk.*
attrib +r +a +h *.org
attrib +r +h    bootcon.sys
attrib +r +h    boot.tav
attrib +a +h +s 386spart.par
goto EXIT

:E
rem MAXIMALER DOS-SPEICHER
PATH C:\BAT;C:\DOS;C:\WINDOWS;C:\;C:\DUBLDISK;C:\PROAUDIO;C:\TOOLS
SET TEMP=D:\TMP

C:\QEMM\LOADHI /R:1 KEYB GR,,C:\DOS\KEYBOARD.SYS
C:\QEMM\LOADHI /R:1 C:\DOS\MOUSE.COM
C:\QEMM\LOADHI /R:1 C:\CDROM\MSCDEX /D:MSCD000 /L:G
goto EXIT

:EXIT
mem
