//лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
//лл                                                                        лл
//лл  VP32.C                                                                лл
//лл                                                                        лл
//лл  Creative Voice File (.VOC) performance utility                        лл
//лл                                                                        лл
//лл  V1.00 of 15-Nov-92: Derived from VP16.C v1.00                         лл
//лл  V1.01 of  1-May-93: Zortech C++ v3.1 compatibility added              лл
//лл  V1.02 of 18-Nov-93: MetaWare / Phar Lap compatibility added           лл
//лл                                                                        лл
//лл  Project: IBM Audio Interface Library for 32-bit DOS                   лл
//лл   Author: John Miles                                                   лл
//лл                                                                        лл
//лл  C source compatible with Watcom C386 v9.0 or later                    лл
//лл                           Zortech C++ v3.1 or later                    лл
//лл                           MetaWare High C/C++ v3.1 or later            лл
//лл                                                                        лл
//лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
//лл                                                                        лл
//лл  Copyright (C) 1991-1993 Miles Design, Inc.                            лл
//лл                                                                        лл
//лл  Miles Design, Inc.                                                    лл
//лл  6702 Cat Creek Trail                                                  лл
//лл  Austin, TX 78731                                                      лл
//лл  (512) 345-2642 / FAX (512) 338-9630 / BBS (512) 454-9990              лл
//лл                                                                        лл
//лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <malloc.h>
#include <io.h>
#include <conio.h>
#include <ctype.h>

#include <AIL32.H>
#include <AILDLL.H>

const char VERSION[] = "1.02";

#ifndef _WIN32
#define _filelength filelength
#define _kbhit kbhit
#define _getch getch
#define _fileno fileno
#endif

/*****************************************************************/
void main(int argc, char *argv[])
{
   HDRIVER hdriver;
   char *drvr;
#ifdef __DOS__
   char *dll;
#endif
   ail_drvr_desc *desc;
#ifdef __DOS__
   union REGS inregs,outregs;
#endif
   unsigned int VOCseg;
   char *VOCbuf;

#ifdef __HIGHC__
   unsigned char *buffer;
   FARPTR VOCfar;
   ULONG i;
#endif

   setbuf(stdout,NULL);

   printf("\nVP32 version %s                   Copyright (C) 1991, 1992 Miles Design, Inc.\n",VERSION);
   printf("-------------------------------------------------------------------------------\n\n");

   if (argc != 3)
      {
      printf("This program plays a Creative Voice File (.VOC) through any Audio Interface\n");
      printf("Library digital sound driver.\n\n");
      printf("Usage: VP32 filename.voc driver.dll\n");
      exit(1);
      }

   //
   // Allocate a buffer for the .VOC file in real-mode (lower 1MB) memory
   //

#ifdef __DOS__
#ifdef DPMI          // Rational Systems DOS/4GW

   inregs.x.eax = 0x100;
   inregs.x.ebx = ((FILE_size(argv[1])+16) / 16);

   int386(0x31,&inregs,&outregs);

   if (outregs.x.cflag)
      {
      printf("Insufficient memory to load %s.\n",argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

   VOCseg = outregs.x.eax << 16;
   VOCbuf = (char *) (outregs.x.eax * 16);
#endif
#else
   VOCseg = 0;
   VOCbuf = (char*)malloc(FILE_size(argv[1]));
#endif

   //
   // Read the entire .VOC file into memory
   //
   // Under MetaWare/Phar Lap, we will buffer it down into lower (far) memory
   // since FILE_read() requires a near pointer
   //

   VOCbuf = (char*)FILE_read(argv[1],VOCbuf);

   if (VOCbuf == NULL)
      {
      printf("Couldn't load %s.\n",argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }


   //
   // Load driver file
   //

#ifdef __DOS__
   dll = FILE_read(argv[2],NULL);
   if (dll==NULL)
      {
      printf("Could not load driver '%s'.\n",argv[2]);
      exit(1);
      }

   drvr=DLL_load(dll,DLLMEM_ALLOC | DLLSRC_MEM,NULL);
   if (drvr==NULL)     
      {
      printf("Invalid DLL image.\n");
      exit(1);
      }

   free(dll);
#else
   drvr = (char*)DLL_load(argv[2], DLLMEM_ALLOC | DLLSRC_FILE, NULL);
   if (drvr == NULL)
   {
       printf("Invalid DLL image.\n");
       exit(1);
   }
#endif

   //
   // Initialize API before calling any Library functions
   //

   AIL_startup();

   hdriver = AIL_register_driver(drvr);
   if (hdriver==-1)
      {
      printf("Driver %s not compatible with linked API version.\n",
         argv[2]);
      AIL_shutdown(NULL);
      exit(1);
      }

   //
   // Get driver type and factory default I/O parameters; exit if
   // driver is not capable of interpreting PCM sound data
   //

   desc = AIL_describe_driver(hdriver);

   if (desc->drvr_type != DSP_DRVR && desc->drvr_type != XMIDI_AND_DSP_DRVR)
      {
      printf("%s is not a digital sound driver.\n",argv[2]);
      AIL_shutdown(NULL);
      exit(1);
      }

   if (!AIL_detect_device(hdriver,desc->def_IO,desc->def_IRQ,
      desc->def_DMA,desc->def_DRQ))
         {
         printf("Sound hardware not found.\n");
         AIL_shutdown(NULL);
         exit(1);
         }

   AIL_init_driver(hdriver,desc->def_IO,desc->def_IRQ,
      desc->def_DMA,desc->def_DRQ);

   //
   // Format .VOC file data and begin playback
   //

   AIL_format_VOC_file(hdriver,(void FAR *) VOCbuf,-1);

   AIL_play_VOC_file(hdriver,(void FAR *) VOCbuf,VOCseg,-1);

   AIL_start_digital_playback(hdriver);

   printf("Press any key to stop ... \n");

   while (AIL_VOC_playback_status(hdriver) != DAC_DONE)
      {
      if (_kbhit())
         {
         (void)_getch();
         break;
         }
      }

   printf("VP32 stopped.\n");
   AIL_shutdown(NULL);
}
