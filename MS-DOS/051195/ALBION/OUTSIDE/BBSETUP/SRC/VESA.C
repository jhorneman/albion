//------------------------------------------------------------------------------
//VESA.C : Stellt Routinen zur Abfrage der VESA-Eigenschaften zur Verfgung
//------------------------------------------------------------------------------
#include "setup.h"

#define D32RealSeg(P)   ((((unsigned long) (P)) >> 4) & 0xFFFF)
#define D32RealOff(P)   (((unsigned long) (P)) & 0xF)

//VESA-Struktur:
struct VgaInfoBlock
{
	UNCHAR   VESA_Signature[4];    //Sollte immer 'VESA' sein
	SISHORT  VESA_Version;
	UNCHAR  *OEM_String;
	SILONG   Capabilities;
	SISHORT *SupportList;
	SISHORT  MemoryBlocks;         //Anzahl der 64k Bl”cke auf der Karte
	UNCHAR   Dummy[236];
} *MyVesaInfo = NULL;

struct VesaModeInfoBlock
{
	SISHORT Attributes;
	UNCHAR  Dummy[255];
} *MyVesaModeInfo = NULL;

//Siehe WATCOM - User's Guide - How do I simulate a Real-Mode interrupt with DOS4GW ?
static struct rminfo
{
	SILONG  EDI;
	SILONG  ESI;
	SILONG  EBP;
	SILONG  reserved_by_system;
	SILONG  EBX;
	SILONG  EDX;
	SILONG  ECX;
	SILONG  EAX;
	SISHORT flags;
	SISHORT ES,DS,FS,GS,IP,CS,SP,SS;
} RMI;

//------------------------------------------------------------------------------
//Speicher im ersten Megabyte anlegen:
//------------------------------------------------------------------------------
void *D32DosMemAlloc(long size)
{
	union REGS Regs;

	Regs.x.eax = 0x0100;           //DPMI allocate DOS memory
	// Shift right by 4 changed to divide by 16 - JH
	// Cast to UNLONG added - JH
	Regs.x.ebx = ((UNLONG) size + 15) / 16; //Number of paragraphs requested
	int386(0x31,&Regs, &Regs);

	if (Regs.x.cflag) InternalError(1049);
	return((void *)((Regs.x.eax & 0xFFFF) << 4));
}

//------------------------------------------------------------------------------
//Gibt TRUE zurck, falls ein VESA-Treiber installiert ist.
//------------------------------------------------------------------------------
BOOL VESA_IsAvailable (void)
{
	union  REGS Regs;
	struct SREGS SRegs;

	if (!MyVesaInfo)
	{
		//Allocate plan DOS Mem for a plain dos interrupt:
		MyVesaInfo=D32DosMemAlloc (sizeof (struct VgaInfoBlock));
	}

	//Preset the IRQ-Parameters
	memset(&RMI, 0, sizeof(RMI));

	RMI.EAX=0x4f00; //Subfunction 00: Get VESA Information

	RMI.ES  = D32RealSeg (MyVesaInfo);
	RMI.EDI = D32RealOff (MyVesaInfo);

	//Alle VESA-Calls laufen ber int 10h, AH=4fh
	//Fr Dokumentation siehe in PCGPE: VESASP12.TX

   //Use DMPI call 300h to issue the DOS interrupt
	SRegs.es=SRegs.ds=SRegs.fs=SRegs.gs=0;
   Regs.w.ax = 0x0300;
   Regs.h.bl = 0x10;    //Call IRQ 10h
   Regs.h.bh = 0;
   Regs.w.cx = 0;
   SRegs.es = FP_SEG(&RMI);
   Regs.x.edi = FP_OFF(&RMI);
   int386x (0x31, &Regs, &Regs, &SRegs);

	if ((RMI.EAX&255)!=0x4f) return (FALSE); 			//Function not supportet
	if (((RMI.EAX/256)&255)==0x01) return (FALSE);	//Function call failed

	if (memcmp (MyVesaInfo->VESA_Signature, "VESA", 4)!=0)
	{
		//Function didn't return a good block. Strange.
		return (FALSE);
	}

	return (TRUE);
}

//------------------------------------------------------------------------------
//šberprft, ob der Videomodus verwendbar ist. TRUE=Ja
//------------------------------------------------------------------------------
BOOL VESA_SupportsMode (short Videomode, short *SupportList)
{
	union  REGS Regs;
	struct SREGS SRegs;

	if (!VESA_IsInSupportList (Videomode, SupportList)) return (FALSE);

	if (!MyVesaModeInfo)
	{
		//Allocate plan DOS Mem for a plain dos interrupt:
		MyVesaModeInfo=D32DosMemAlloc (sizeof (struct VesaModeInfoBlock));
	}

	//Preset the IRQ-Parameters
	memset(&RMI, 0, sizeof(RMI));

	RMI.EAX=0x4f01; //Subfunction 01: Get VESA Mode-Information
	RMI.ECX=Videomode;

	RMI.ES  = D32RealSeg (MyVesaModeInfo);
	RMI.EDI = D32RealOff (MyVesaModeInfo);

	//Alle VESA-Calls laufen ber int 10h, AH=4fh
	//Fr Dokumentation siehe in PCGPE: VESASP12.TX

   //Use DMPI call 300h to issue the DOS interrupt
	SRegs.es=SRegs.ds=SRegs.fs=SRegs.gs=0;
   Regs.w.ax = 0x0300;
   Regs.h.bl = 0x10;    //Call IRQ 10h
   Regs.h.bh = 0;
   Regs.w.cx = 0;
   SRegs.es = FP_SEG(&RMI);
   Regs.x.edi = FP_OFF(&RMI);
   int386x (0x31, &Regs, &Regs, &SRegs);

	if ((RMI.EAX&255)!=0x4f) return (FALSE); 			//Function not supportet
	if (((RMI.EAX/256)&255)==0x01) return (FALSE);	//Function call failed

	if ((MyVesaModeInfo->Attributes&3)==2)
	{
		//Bit1=1 -> BIOS stellt Informationen zur Verfgungn
		//Bit0=0 -> Mode not supported by Hardware
		return (FALSE);
	}

	return (TRUE);
}

//------------------------------------------------------------------------------
//Schaut, ob ein Modus in der VESA-Supportlist ist.
//------------------------------------------------------------------------------
BOOL VESA_IsInSupportList (short Videomode, short *SupportList)
{
	while (1)
	{
		if (*SupportList==Videomode) return (TRUE);
		if (*SupportList==-1) return (FALSE);
		SupportList++;
	}
}

//------------------------------------------------------------------------------
//šberfllt die VESA-INI-Datei mit Informationen
//------------------------------------------------------------------------------
BOOL VESA_UpdateIni (void)
{
	union  REGS Regs;
	struct SREGS SRegs;
	UNCHAR Buffer  [80];

	if (!MyVesaInfo)
	{
		//Allocate plan DOS Mem for a plain dos interrupt:
		MyVesaInfo=D32DosMemAlloc (sizeof (struct VgaInfoBlock));
	}

	//Preset the IRQ-Parameters
	memset(&RMI, 0, sizeof(RMI));

	RMI.EAX=0x4f00; //Subfunction 00: Get VESA Information

	RMI.ES  = D32RealSeg (MyVesaInfo);
	RMI.EDI = D32RealOff (MyVesaInfo);

	//Alle VESA-Calls laufen ber int 10h, AH=4fh
	//Fr Dokumentation siehe in PCGPE: VESASP12.TX

   //Use DMPI call 300h to issue the DOS interrupt
	SRegs.es=SRegs.ds=SRegs.fs=SRegs.gs=0;
   Regs.w.ax = 0x0300;
   Regs.h.bl = 0x10;    //Call IRQ 10h
   Regs.h.bh = 0;
   Regs.w.cx = 0;
   SRegs.es = FP_SEG(&RMI);
   Regs.x.edi = FP_OFF(&RMI);
   int386x (0x31, &Regs, &Regs, &SRegs);

	if ((RMI.EAX&255)!=0x4f) return (FALSE); 			//Function not supportet
	if (((RMI.EAX/256)&255)==0x01) return (FALSE);	//Function call failed

	if (memcmp (MyVesaInfo->VESA_Signature, "VESA", 4)!=0)
	{
		//Function didn't return a good block. Strange.
		return (FALSE);
	}

	//Convert a real-mode adress to a protected mode adress:
	MyVesaInfo->OEM_String =
		(char*)(
					(((unsigned long)(MyVesaInfo->OEM_String))&0x0000ffff) +
					((((unsigned long)(MyVesaInfo->OEM_String))&0xf0000000)>>12)
				 );

	//Convert a real-mode adress to a protected mode adress:
	MyVesaInfo->SupportList =
		(short*)(
					(((unsigned long)(MyVesaInfo->SupportList))&0x0000ffff) +
					((((unsigned long)(MyVesaInfo->SupportList))&0xf0000000)>>12)
				 );

	//Es ist Vorraussetzung, daá bereits eine INI-Datei existiert. Sonst tut
	//diese Routine keinen Hammerschlag. Sie auch "setup.sam".
   if (IfExistsFile (IniFilename))
	{
		//16.2Mio Farben:
		if (VESA_SupportsMode (0x118, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_1024x768x16.2M", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_1024x768x16.2M", "N");

		if (VESA_SupportsMode (0x115, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_800x600x16.2M", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_800x600x16.2M", "N");

		if (VESA_SupportsMode (0x112, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_640x480x16.2M", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_640x480x16.2M", "N");

		//64000 Farben:
		if (VESA_SupportsMode (0x117, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_1024x768x64k", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_1024x768x64k", "N");

		if (VESA_SupportsMode (0x114, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_800x600x64k", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_800x600x64k", "N");

		if (VESA_SupportsMode (0x111, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_640x480x64k", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_640x480x64k", "N");

		//32000 Farben:
		if (VESA_SupportsMode (0x116, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_1024x768x32k", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_1024x768x32k", "N");

		if (VESA_SupportsMode (0x113, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_800x600x32k", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_800x600x32k", "N");

		if (VESA_SupportsMode (0x110, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_640x480x32k", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_640x480x32k", "N");

		//256 Farben:
		if (VESA_SupportsMode (0x105, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_1024x768x256", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_1024x768x256", "N");

		if (VESA_SupportsMode (0x103, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_800x600x256", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_800x600x256", "N");

		if (VESA_SupportsMode (0x101, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_640x480x256", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_640x480x256", "N");

		if (VESA_SupportsMode (0x100, MyVesaInfo->SupportList))
            IniSet (IniFilename, "VESA", "MODE_640x400x256", "Y");
      else  IniSet (IniFilename, "VESA", "MODE_640x400x256", "N");

		sprintf (Buffer, "%li", MyVesaInfo->MemoryBlocks*64*1024);
      IniSet (IniFilename, "VESA", "MEMORY", Buffer);

		sprintf (Buffer, "%li", (SILONG)(MyVesaInfo->VESA_Version&255));
      IniSet (IniFilename, "VESA", "VESA_VERSION_SUBNUMBER", Buffer);

		sprintf (Buffer, "%li", (SILONG)(MyVesaInfo->VESA_Version/256));
      IniSet (IniFilename, "VESA", "VESA_VERSION_NUMBER", Buffer);

      IniSet (IniFilename, "VESA", "OEM", MyVesaInfo->OEM_String);
	}

	return (TRUE);
}
