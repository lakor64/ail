//����������������������������������������������������������������������������
//��                                                                        ��
//��  MIX32.C                                                               ��
//��                                                                        ��
//��  XMIDI multiple-sequence sound effects demo                            ��
//��                                                                        ��
//��  V1.00 of  5-Aug-92: 32-bit conversion of MIXDEMO.C (John Lemberger)   ��
//��  V1.01 of  1-May-93: Zortech C++ v3.1 compatibility added              ��
//��                                                                        ��
//��  Project: IBM Audio Interface Library for 32-bit DPMI (AIL/32)         ��
//��   Author: John Miles                                                   ��
//��                                                                        ��
//��  C source compatible with Watcom C386 v9.0 or later                    ��
//��  C source compatible with Zortech C++ v3.1 or later                    ��
//��                                                                        ��
//����������������������������������������������������������������������������
//��                                                                        ��
//��  Copyright (C) 1991-1993 Miles Design, Inc.                            ��
//��                                                                        ��
//��  Miles Design, Inc.                                                    ��
//��  6702 Cat Creek Trail                                                  ��
//��  Austin, TX 78731                                                      ��
//��  (512) 345-2642 / FAX (512) 338-9630 / BBS (512) 454-9990              ��
//��                                                                        ��
//����������������������������������������������������������������������������

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <malloc.h>
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#include <string.h>
#include <conio.h>

#include <AIL32.H>
#include <AILDLL.H>

const char VERSION[] = "1.01";

char seq_fn[] = "DEMO.XMI";       // name of XMIDI sequence file

#ifndef _WIN32
#define _getch getch
#endif

/***************************************************************/

#ifdef __DOS__

//
// Standard C routine to determine available memory in 32-bit 
// protected mode
//

#ifdef DPMI                       // Rational Systems DOS/4GW

struct meminfo {
    unsigned int LargestBlockAvail;
   unsigned int MaxUnlockedPage;
   unsigned int LargestLockablePage;
   unsigned int LinAddrSpace;
   unsigned int NumFreePagesAvail;
   unsigned int NumPhysicalPagesFree;
   unsigned int TotalPhysicalPages;
   unsigned int FreeLinAddrSpace;
   unsigned int SizeOfPageFile;
   unsigned int Reserved[3];
}
MemInfo;

unsigned int mem_avail(void)
{
   union REGS regs;
   struct SREGS sregs;

   memset(&sregs,0,sizeof(struct SREGS));
   regs.x.eax = 0x00000500;
   sregs.es = FP_SEG(&MemInfo);
   regs.x.edi = FP_OFF(&MemInfo);

   int386x(0x31,&regs,&regs,&sregs);
   return MemInfo.LargestBlockAvail;
}
#endif
#endif

/***************************************************************/

//
// Standard C routine for Global Timbre Library access
//

void *load_global_timbre(FILE *GTL, unsigned short bank, unsigned short patch)
{
   unsigned short *timb_ptr;
   static unsigned short len;

   static struct                  // GTL file header entry structure
   {
      signed char patch;
      signed char bank;
      unsigned long offset;
   }
   GTL_hdr;

   if (GTL==NULL) return NULL;    // if no GTL, return failure

   rewind(GTL);                   // else rewind to GTL header

   do                             // search file for requested timbre
      {
      fread(&GTL_hdr,sizeof(GTL_hdr),1,GTL);

      if (GTL_hdr.bank == -1) 
         return NULL;             // timbre not found, return NULL
      }
   while ((GTL_hdr.bank != bank) ||
          (GTL_hdr.patch != patch));       

   fseek(GTL,GTL_hdr.offset,SEEK_SET);    
   fread(&len,2,1,GTL);           // timbre found, read its length

   timb_ptr = (unsigned short*)malloc(len);        // allocate memory for timbre ..
   if (!timb_ptr)
       return NULL;
   *timb_ptr = len;         
                                  // and load it
   fread((timb_ptr+1),len-2,1,GTL);       
                           
   if (ferror(GTL))               // return NULL if any errors
      return NULL;                // occurred
   else
      return timb_ptr;            // else return pointer to timbre
}
   
/***************************************************************/
void main(int argc, char *argv[])
{
   HDRIVER hdriver;
   HSEQUENCE hseq[8];
   char *state[8];
   char GTL_filename[32];
   unsigned char *buffer;
   unsigned long state_size;
   ail_drvr_desc *desc;
   char *drvr;
   char *timb;
#ifdef __DOS__
   char *dll;
#endif
   FILE *GTL;
   unsigned i,ch,tc_size;
   char *tc_addr;
   unsigned short treq;

   setbuf(stdout,NULL);

   printf("\nMIX32 version %s                  Copyright (C) 1991, 1992 Miles Design, Inc.\n",VERSION);
   printf("-------------------------------------------------------------------------------\n\n");

#ifdef __DOS__
   printf("Largest available memory block (32-bit flat model): %lu bytes\n\n",mem_avail());
#endif

   if (argc != 2)
      {
      printf("This program demonstrates the Audio Interface Library's ");
      printf("multiple-sequence\nExtended MIDI playback features, and also ");
      printf("displays the amount of free\nmemory available in protected mode.\n\n");
      printf("Usage: MIX32 driver_filename\n");
      exit(1);
      }

   //
   // Load driver file
   //

#ifdef __DOS__
   dll = (char*)FILE_read(argv[1],NULL);
   if (dll==NULL)
      {
      printf("Could not load driver '%s'.\n",argv[2]);
      exit(1);
      }

   drvr=(char*)DLL_load(dll,DLLMEM_ALLOC | DLLSRC_MEM,NULL);
   if (drvr==NULL)     
      {
      printf("Invalid DLL image.\n");
      exit(1);
      }

   free(dll);
#else
   drvr = (char*)DLL_load(argv[1], DLLMEM_ALLOC | DLLSRC_FILE, NULL);
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

   //
   // Register the driver with the API
   //

   hdriver = AIL_register_driver(drvr);
   if (hdriver==-1)
      {
      printf("Driver %s not compatible with linked API version.\n",
         argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

   //
   // Get driver type and factory default I/O parameters; exit if
   // driver is not capable of interpreting MIDI files
   //

   desc = AIL_describe_driver(hdriver);

   if (desc->drvr_type != XMIDI_DRVR && desc->drvr_type != XMIDI_AND_DSP_DRVR)
      {
      printf("Driver %s not an XMIDI driver.\n",argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

   //
   // Verify presence of driver's sound hardware and prepare 
   // driver/hardware for use
   //

   if (!AIL_detect_device(hdriver,desc->def_IO,desc->def_IRQ,
      desc->def_DMA,desc->def_DRQ))
         {
         printf("Sound hardware not found.\n");
         AIL_shutdown(NULL);
         exit(1);
         }

   AIL_init_driver(hdriver,desc->def_IO,desc->def_IRQ,
      desc->def_DMA,desc->def_DRQ);

   state_size = AIL_state_table_size(hdriver);

   //
   // Load XMIDI data file
   //

   buffer = (unsigned char*)FILE_read(seq_fn,NULL);
   if (buffer == NULL)
      {
      printf("Can't load XMIDI file %s.\n",seq_fn);
      AIL_shutdown(NULL);
      exit(1);
      }

   //
   // Get name of Global Timbre Library file by appending suffix 
   // supplied by XMIDI driver to GTL filename prefix "SAMPLE."
   //

   strcpy(GTL_filename,"SAMPLE.");
   strcat(GTL_filename,desc->data_suffix);

   //
   // Set up local timbre cache; open Global Timbre Library file
   //

   tc_size = AIL_default_timbre_cache_size(hdriver);
   if (tc_size)
      {
      tc_addr = (char*)malloc((unsigned long) tc_size);
      AIL_define_timbre_cache(hdriver,tc_addr,tc_size);
      }

   GTL = fopen(GTL_filename,"rb");

   //
   // Register all sequences in XMIDI file (up to 8 allowed), loading
   // new timbres as needed
   //

   for (i=0;i<8;i++)
      {
      state[i] = (char*)malloc(state_size);
      if ((hseq[i] = AIL_register_sequence(hdriver,buffer,i,state[i],
         NULL)) == -1)
         {
         free(state[i]);
         break;
         }

      while ((treq=AIL_timbre_request(hdriver,hseq[i])) != 0xffff)
         {
         timb = (char*)load_global_timbre(GTL,treq/256,treq%256);
         if (timb != NULL)
            {
            AIL_install_timbre(hdriver,treq/256,treq%256,timb);
            free(timb);
            }
         else
            {
            printf("Timbre bank %u, patch %u not found ",
               treq/256,treq%256);
            printf("in Global Timbre Library %s\n",GTL_filename);
            AIL_shutdown(NULL);
            exit(1);
            }
         }
      }

   if (GTL != NULL) fclose(GTL);         
   printf("Sequences 0-%u registered.\n\n",i-1);

   //
   // Show menu, start sequences at user's request
   //

   printf("Options: [1] to start main sequence (BACKGND.MID)\n");
   printf("         [2] to start sequence #1   (SHANTY.MID)\n");
   printf("         [3] to start sequence #2   (CHORAL.MID)\n");
   printf("       [ESC] to quit\n\n");

   while ((ch=_getch()) != 27)
      {
      switch (ch)
         {
         case '1':
         case '2':
         case '3':
            i = ch - '1';
            AIL_start_sequence(hdriver,hseq[i]);
            break;
         }
      }

   //
   // Shut down API and all installed drivers; write signoff message
   // to any front-panel displays
   //

   printf("MIX32 stopped.\n");
   AIL_shutdown("MIX32 stopped.");
}
