@echo off
echo Linking utility (c) 1998 - 1999 by Szeleney Robert
echo Linking Object files...
echo.

del archives\*.a
del archives\*.o

echo Phase1: Building kernel archive...
copy kernel.o archives
ar rcs archives\archive.a *.o

echo Phase8: Linking Kernel to binary format. Offset = 0x10000...
cd archives
ld -Map m -o kernel.sys -oformat binary -T l.scr kernel.o archive.a
copy kernel.sys ..
cd ..

del *.


