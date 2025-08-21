@echo off
echo Generating Bootcode...
c:\bc31\bin\tasm bootcode
c:\bc31\bin\tlink bootcode

c:\sky\tools\rmheader\rmheader bootcode.exe bootcode.dat

copy bootcode.dat c:\sky\system
del bootcode.exe
del bootcode.dat
del bootcode.obj
del bootcode.map

