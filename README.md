How to compile/install SkyOS
=======================================================================

NOTE: This manual is base on the build 05.02.2000 and later.

To compile SkyOS you need this tools:
- DOS 6.0/6.2
- NASM (must be installed in c:\nasm)
- DJGPP (must be installed in c:\djgpp)
- RHIDE

Compile/Link/Build/Install/Start
======================================================================

1. Set the path environment variable to c:\nasm;c:\djgpp\bin;c:\sky

2. Extract the source archive into c:\sky

3. Start RHIDE from the c:\sky directory

4. Compile SkyOS (F9)

5. Close RHIDE and type build.bat
   This batch file will compile some files and links the kernel.

   After linking is finished the setup programm in c:\sky\setup\setup
   is executed.

6. Select a harddisk or floppy to install SkyOS.

7. If you selected a harddisk select a partition.
   Note: Only primary partitions are supported.

8. Select blocksize. Must be 4 in this version.

9. Confirm with yes.

10. After setup has finished a mapfile is generated in c:\mapfile.sys

11. If you install SkyOS in a partition install the SkyOS bootmanager.
    Type:
    cd \sky\setup
    setup /b

    Confirm with 'y'.
    reboot

12. If you have installed SkyOS on a disk reboot.

13. If you have installed the SkyOS bootmanager configure it.
    Note: Only primary partitions on the first HDD can be booted.

Think that's all. If you have any questions write a e-mail to me.
bertlman@gmx.at






