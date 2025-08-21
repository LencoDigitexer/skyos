@echo off
cls

echo Assembling Utility (c) 1999 by Szeleney Robert
echo This file will assemble the startup code...
echo Starting nasm...
nasm -f bin -o object\startup.o source\kernel\startup.asm
