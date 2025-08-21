@echo off
cls
echo ****************************************************************************
echo * SKY Operating System V2.0j                                               *
echo ****************************************************************************
echo * SKYOS Compile-Link-Build Utiltity. (CLB V1.0)
echo * Starting process....
echo ****************************************************************************
echo.

del c:\sky\object\kernel.sys

call compile.bat
cd object
call link.bat
cd..


if not exist c:\sky\object\kernel.sys goto ende

rem call c:\sky\setup\setup.exe 2 3 2

rem cd c:\sky\vgasetup
call setup\setup

Generating link map...
call \sky\tools\excinfo\excinfo

echo.
echo.

goto ende

:ende
echo Compiling and/or Linking failed
pause

