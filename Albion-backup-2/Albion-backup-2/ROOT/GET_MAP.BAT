@echo off
echo.
echo Get-Map (C) 1995 M. Pukropski Blue Byte Software
echo Input-Parameter ist Map-Nummer (immer 4stellig!)
echo.
@if not "%1"=="" goto ANFANG
echo Es wurde kein Input-Parameter Åbergeben!
echo Beispiele: get_map 1001   get_map 0290   get_map 0050
echo.
goto ENDE
:ANFANG
@if exist U:\PROJECT\ALBION\MAPFLAG.FLG goto AUSSTIEG
@copy C:\AUTOEXEC.BAT U:\PROJECT\ALBION\MAPFLAG.FLG
@xcopy U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD\MAP_%1.DAT D:\DDT_.ORD\DDT__MAP.ORD
@xcopy U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD\MAP_%1.TXT D:\DDT_.ORD\DDT_TEXT.ORD\MAP__TXT.ORD
@xcopy U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD\CEV_%1.MSK E:\DDTM.ORD 
@xcopy U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD\MAP_%1.MSK E:\DDTM.ORD
echo.
echo Sollen die kopierten Map-Files auf dem Netzwerk
choice /c:jn "gelîscht werden " 
if errorlevel=2 goto NICHTLOESCHEN
if errorlevel=1 goto LOESCHEN
goto ENDE
:LOESCHEN
echo Lîsche Netz-Files...
@del U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD\MAP_%1.MSK
@del U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD\CEV_%1.MSK
@del U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD\MAP_%1.TXT
@del U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD\MAP_%1.DAT
echo Ok.
:NICHTLOESCHEN
@del U:\PROJECT\ALBION\MAPFLAG.FLG     
@goto ENDE
:AUSSTIEG
echo.
echo Achtung!
echo Dieser Batch wird derzeit von einem anderen User benutzt.
echo Bitte zu einem spÑteren Zeitpunkt nochmal versuchen.
echo.
:ENDE


