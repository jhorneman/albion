@echo off
echo.
echo Put-Map (C) 1995 M. Pukropski Blue Byte Software
echo Input-Parameter ist Map-Nummer (immer 4stellig!)
echo.
@if not "%1"=="" goto ANFANG
echo Es wurde kein Input-Parameter Åbergeben!
echo Beispiele: put_map 1001   put_map 0290   put_map 0050
echo.
goto ENDE
:ANFANG
@if exist U:\PROJECT\ALBION\MAPFLAG.FLG goto AUSSTIEG
@copy C:\AUTOEXEC.BAT U:\PROJECT\ALBION\MAPFLAG.FLG
@xcopy D:\DDT_.ORD\DDT__MAP.ORD\MAP_%1.DAT U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD
@xcopy D:\DDT_.ORD\DDT_TEXT.ORD\MAP__TXT.ORD\MAP_%1.TXT U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD
@xcopy E:\DDTM.ORD\CEV_%1.MSK U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD
@xcopy E:\DDTM.ORD\MAP_%1.MSK U:\PROJECT\ALBION\EDITOR.DAT\MAP_TUBE.ORD
echo.
echo Sollen die ausgelagerten Map-Files auf der lokalen Platte
choice /c:jn "gelîscht werden " 
if errorlevel=2 goto NICHTLOESCHEN
if errorlevel=1 goto LOESCHEN
goto ENDE
:LOESCHEN
echo Lîsche lokale Files...
@del E:\DDTM.ORD\MAP_%1.MSK
@del E:\DDTM.ORD\CEV_%1.MSK
@del D:\DDT_.ORD\DDT_TEXT.ORD\MAP__TXT.ORD\MAP_%1.TXT
@del D:\DDT_.ORD\DDT__MAP.ORD\MAP_%1.DAT
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


