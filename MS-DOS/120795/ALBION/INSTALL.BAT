@echo off

REM Tell the user what is going on

echo This is a preliminary version of Albion
echo Copyright Blue Byte Software GmbH 1995
echo ----------------------------------------
echo Use this batch to install Albion on your C-partition. Albion requires
echo 30 Mb free disk space.
echo ----------------------------------------

REM Ask if user wants to install Albion

choice /c:YN "Do you want to install Albion in C:\ALBION "

REM Check if choice reported an error
if ERRORLEVEL 255 goto END

REM Check if user selected NO
if ERRORLEVEL 2 goto END

REM Check if user selected YES
if ERRORLEVEL 1 goto INSTALL

REM User aborted
goto END

:INSTALL

REM Tell user what is going on

echo Installing Albion... please wait.
echo ----------------------------------------

REM Create Albion directory
md C:\ALBION

REM Copy Albion
xcopy *.* C:\ALBION /e /y

REM Make Albion writable
attrib -r /s

REM Tell user everything worked out fine

echo ----------------------------------------
echo Albion was installed succesfully. Type
echo SETUP to setup your sound hardware, then
echo GO to play Albion.

:END

