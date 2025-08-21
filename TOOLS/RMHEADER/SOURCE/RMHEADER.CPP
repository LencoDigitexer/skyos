/***************************************************************************/
/* SKY Systems Extra Utitiliy                                              */
/***************************************************************************/
/* Project Members: þ Christian Resl 12.11.1979                            */
/*                    Username: Chris                                      */
/*                  þ Robert Szeleney 04.06.1980                           */
/*                    Username: Bertl                                      */
/***************************************************************************/
/***************************************************************************/
/* File:        RMHEADER.CPP (RMHEADER.EXE)                                */
/* Parameter:   <Sourcefile> <Destinationfile>                             */
/* Description: Copies the Sourcefile to the Destinationfile without the   */
/*              first 512 bytes (EXE-Header).                              */
/***************************************************************************/
/* Created:     08.05.1997                                                 */
/* Last Update: 01.06.1997                                                 */
/***************************************************************************/

#include <STDIO.H>
#include <CONIO.H>
#include <FCNTL.H>
#include <SYS\STAT.H>
#include <IO.H>

int main(int argc,char *argv[])
{
 int i,FileHandle1,FileHandle2;
 char Buffer[512];

 if (argc<3)
 {
  printf("\nUsage: RMHEADER <Sourcefile> <Destinationfile>\n");
  return 0;
 }
 _fmode=O_BINARY;
 if ((FileHandle1 = open(argv[1],O_RDONLY)) == -1)
 {
  printf("\nSourcefile not found.\n");
  return 1;
 }
 if ((FileHandle2 = creat(argv[2],S_IWRITE)) == -1)
 {
  printf("\nError creating Destinationfile. Existing?\n");
  return 1;
 }
 lseek(FileHandle1,512,SEEK_SET);
 while(!eof(FileHandle1))
 {
  i=read(FileHandle1,Buffer,512);
  write(FileHandle2,Buffer,i);
 }
 close(FileHandle1);
 close(FileHandle2);
 return 0;
}
