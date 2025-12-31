/************
 * NAME     : SHOWMEM.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 13-11-1995
 * PROJECT  : Show memory
 * NOTES    :
 * SEE ALSO : BASEMEM.C
 ************/

/*
 ** Pragmas ****************************************************************
 */

#pragma off (unreferenced);

/*
 ** Includes ***************************************************************
 */

#include <stdio.h>
#include <dos.h>
#include <string.h>
#include <i86.h>
#include <time.h>
#include <sys\timeb.h>

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/* DPMI host interrupt */
#define DPMI_INT	0x31

/*
 ** Structure definitions **************************************************
 */

/* DPMI memory info */
struct meminfo {
    unsigned LargestBlockAvail;
    unsigned MaxUnlockedPage;
    unsigned LargestLockablePage;
    unsigned LinAddrSpace;
    unsigned NumFreePagesAvail;
    unsigned NumPhysicalPagesFree;
    unsigned TotalPhysicalPages;
    unsigned FreeLinAddrSpace;
    unsigned SizeOfPageFile;
    unsigned Reserved[3];
};

/*
 ** Prototypes *************************************************************
 */

void main(int argc, char** argv);

void BASEMEM_Init(void);

void BASEMEM_Report(FILE *Output_file);

void BASEMEM_FillMemByte(UNBYTE * ptr, UNLONG size, UNBYTE fillbyte);

/*
 ** Global variables *******************************************************
 */

/* Program start time buffer */
static struct timeb Time_buffer;

/* DPMI version information */
static UNSHORT DPMI_version_nr;
static UNSHORT DPMI_subversion_nr;
static UNSHORT DPMI_flags;
static UNSHORT DPMI_CPU_type;

static UNLONG DPMI_page_size;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : main
 * FUNCTION  : Main function of SHOWMEM
 * FILE      : SHOWMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.11.95 13:27
 * LAST      : 13.11.95 13:27
 * INPUTS    : int argc - Number of arguments.
 *             char **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
main(int argc, char** argv)
{
	FILE *Memory_log_file;

	/* Get the time */
	ftime(&Time_buffer);

	/* Initialize BASEMEM */
	BASEMEM_Init();

	/* Open memory log */
	Memory_log_file = fopen("MEMORY.LOG", "at");

	/* Print BASEMEM report */
	BASEMEM_Report(Memory_log_file);

	/* Close memory log */
	fclose(Memory_log_file);

	printf("\n");

	/* Print BASEMEM report */
	BASEMEM_Report(NULL);

	/* Inform the user */
	printf("The current DPMI memory state has been stored in MEMORY.LOG.\n");
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BASEMEM_Init
 * FUNCTION  : Initialize base memory.
 * FILE      : SHOWMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 11:10
 * LAST      : 28.12.94 11:10
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
BASEMEM_Init(void)
{
	union REGS regs;
	struct SREGS seg_regs;

	/* Try to get DPMI version information from DPMI host */
	BASEMEM_FillMemByte((UNBYTE *) &seg_regs, sizeof(seg_regs), 0);
	regs.w.ax = 0x0400;

	int386x(DPMI_INT, &regs, &regs, &seg_regs);

	/* Get DPMI version */
	DPMI_version_nr		= (UNSHORT) regs.h.ah;
	DPMI_subversion_nr	= (UNSHORT) regs.h.al;

	/* Get DPMI flags */
	DPMI_flags = (UNSHORT) regs.w.bx;

	/* Get processor type */
	DPMI_CPU_type = (UNSHORT) regs.h.cl;

	/* Try to get page size from DPMI host */
	BASEMEM_FillMemByte((UNBYTE *) &seg_regs, sizeof(seg_regs), 0);
	regs.w.ax = 0x0604;

	int386x(DPMI_INT, &regs, &regs, &seg_regs);

	/* Success ? */
	if (!regs.x.cflag)
	{
		/* Yes -> Get page size */
		DPMI_page_size = (regs.w.bx << 16) + regs.w.cx;
	}
	else
	{
		/* No -> Set page size to default value */
		DPMI_page_size = 4096;
	}
}

/* #FUNCTION END# */

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BASEMEM_Report
 * FUNCTION  : Print a memory report.
 * FILE      : SHOWMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.09.95 20:34
 * LAST      : 06.11.95 11:42
 * INPUTS    : FILE *Output_file - Output file (NULL = stdout).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBBASMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
BASEMEM_Report(FILE *Output_file)
{
	union REGS regs;
	struct SREGS seg_regs;
	struct meminfo MemInfo;

	/* Output to stdout if argument is NULL */
	if (Output_file == NULL)
		Output_file = stdout;

	/* Print header */
	fprintf
	(
		Output_file,
		"**** Memory log ****\n"
	);

	/* Print the time */
	fprintf
	(
		Output_file,
		"Executed on %s\n", ctime(&Time_buffer.time)
	);

	/* Print general DPMI data */
	fprintf
	(
		Output_file,
		"DPMI version : %u.%u\n",
		DPMI_version_nr,
		DPMI_subversion_nr
	);
	fprintf
	(
		Output_file,
		"CPU type     : %u\n",
		DPMI_CPU_type
	);
	fprintf
	(
		Output_file,
		"Flags        : %#x\n\n",
		DPMI_flags
	);

	/* Ask DPMI host for DPMI information */
	BASEMEM_FillMemByte((UNBYTE *) &seg_regs, sizeof(seg_regs), 0);
	regs.w.ax = 0x0500;
	seg_regs.es = FP_SEG(&MemInfo);
	regs.x.edi = FP_OFF(&MemInfo);

	int386x(DPMI_INT, &regs, &regs, &seg_regs);

	/* Print this information (from DPMI 0.9 specification) */
   fprintf
	(
		Output_file,
		"Largest available free block in bytes  : %lu\n",
		MemInfo.LargestBlockAvail
	);

	if (MemInfo.MaxUnlockedPage != 0xFFFFFFFF)
	{
		fprintf
		(
			Output_file,
			"Maximum unlocked page allocation       : %lu\n",
			MemInfo.MaxUnlockedPage
		);
	}

   if (MemInfo.LargestLockablePage != 0xFFFFFFFF)
	{
		fprintf
		(
			Output_file,
			"Maximum locked page allocation         : %lu\n",
			MemInfo.LargestLockablePage
		);
	}

   if (MemInfo.LinAddrSpace != 0xFFFFFFFF)
	{
		fprintf
		(
			Output_file,
			"Linear addr space size in pages        : %lu\n",
			MemInfo.LinAddrSpace
		);
	}

   if (MemInfo.NumFreePagesAvail != 0xFFFFFFFF)
	{
		fprintf
		(
			Output_file,
			"Total number of unlocked pages         : %lu\n",
			MemInfo.NumFreePagesAvail
		);
	}

   if (MemInfo.NumPhysicalPagesFree != 0xFFFFFFFF)
	{
		fprintf
		(
			Output_file,
			"Number of free pages                   : %lu\n",
			MemInfo.NumPhysicalPagesFree
		);
	}

	if (MemInfo.TotalPhysicalPages != 0xFFFFFFFF)
	{
		fprintf
		(
			Output_file,
			"Total number of physical pages         : %lu\n",
			MemInfo.TotalPhysicalPages
		);
	}

   if (MemInfo.FreeLinAddrSpace != 0xFFFFFFFF)
	{
		fprintf
		(
			Output_file,
			"Free linear address space in pages     : %lu\n",
			MemInfo.FreeLinAddrSpace
		);
	}

   if (MemInfo.SizeOfPageFile != 0xFFFFFFFF)
	{
		fprintf
		(
			Output_file,
			"Size of paging file/partition in pages : %lu\n",
			MemInfo.SizeOfPageFile
		);
	}

	fprintf(Output_file, "\n");
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BASEMEM_FillMemByte
 * FUNCTION  : Fills a buffer with a byte value.
 * FILE      : SHOWMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.06.94 14:00
 * LAST      : 06.11.95 11:11
 * INPUTS    : UNBYTE *ptr - Pointer to buffer to fill.
 *             UNLONG size - Size of buffer to fill.
 *             UNBYTE fillbyte - Fill value.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBBASMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
BASEMEM_FillMemByte(UNBYTE * ptr, UNLONG size, UNBYTE fillbyte)
{
	/* Legal parameters ? */
	if (ptr != NULL)
	{
		/* Yes -> Fill memory */
		memset
		(
			ptr,
			fillbyte,
			size
		);
	}
}

/* #FUNCTION END# */

