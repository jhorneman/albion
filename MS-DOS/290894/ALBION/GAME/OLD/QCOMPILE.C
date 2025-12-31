/****************************************************************************
 QUICK Amberfile compiler for Amberstar 2 files
 Written by J.Horneman (In Tune With The Universe)
 Start : 20-1-1993
 Syntax : QCompile {amberfile info filename}
****************************************************************************/

/*** Includes ***/

#include <new_functions.h>

/*** Constants ***/

#define Path_length 100
#define max_subfiles 600
#define Buffer_size 500000
#define BSS_size 26124+1000

/*** Prototypes ***/

void Cleanup();
ULONG LOF();
BOOL Read_info_file();
void Swap();
BOOL A_gt_B();
void Encrypt_block();
void Load_subfile();

/*** Structure definitions ***/

struct MyAnchor {
	struct AChain	*ap_Base;
	struct AChain	*ap_Last;
	LONG	ap_BreakBits;
	LONG	ap_FoundBreak;
	BYTE	ap_Flags;
	BYTE	ap_Reserved;
	WORD	ap_Strlen;
	struct	FileInfoBlock ap_Info;		/* Examine result */
	UBYTE	ap_Buf[Path_length];
};

/*** Variable declarations ***/

struct FileHandle *amberfile,*subfile;

ULONG zero = 0,l;
ULONG lengths[max_subfiles],pack_header,max;
ULONG unpacked_length,packed_length,rate;
ULONG total_packed,total_unpacked;

UWORD count,i;

UBYTE *buffer,*pack_bss,*unpacked_address,*packed_address;

BOOL packed=FALSE,encrypted=FALSE,pack_success;

char *subfiles[max_subfiles];
char filenames[max_subfiles*Path_length];
char sourcefile_path[Path_length],amberfile_path[Path_length];

char Pack_ID[] = {(char) 1,'L','O','B'};
char Packed_header[] = {'A','M','P','C'};
char Unpacked_header[] = {'A','M','B','R'};
char Encrypted_header[] = {'A','M','N','C'};
char Packcrypted_header[] = {'A','M','N','P'};

/*** Main program ***/

main(argc,argv)
int argc;
char *argv[];
{
/* Print info if number of arguments is wrong */
if (argc != 2)
	{
	printf("Syntax : QCompile {Amberfile info filename}.\n");
	printf("Written by J.Horneman (In Tune With The Universe).\n");
	printf("Start : 20-1-1993.\n\n");
	printf("This tool will quickly compile all source files and\n");
	printf("save them as an Amberfile.\n\n");
	Cleanup("");
	}

/*** Read info-file ***/
if (Read_info_file(argv[1]) == FALSE) Cleanup();

printf("\nCompiling...\n");
printf("Destination Amberfile : %s\n",amberfile_path);
printf("     Source directory : %s\n",sourcefile_path);

if (packed)
	printf("Packed / ");
else
	printf("Unpacked / ");
if (encrypted)
	printf("encrypted\n\n");
else
	printf("not encrypted\n\n");

/*** Read file paths and lengths ***/
count = Get_file_infos(sourcefile_path,filenames);
if (count == 0)
	Cleanup("No subfiles were found.");

/*** Sort subfiles ***/
Shell_sort(Swap,A_gt_B,count);

/*** Try to get work memory ***/
if (!(buffer = (UBYTE *) AllocMem(Buffer_size,(ULONG) 0)))
	Cleanup("Not enough memory");
unpacked_address = buffer;

if (packed)
	if (!(pack_bss = (UBYTE *) AllocMem(BSS_size,MEMF_CLEAR)))
		Cleanup("Not enough memory");

/*** Try to open Amberfile ***/
if (!(amberfile = (struct FileHandle *) Open(amberfile_path,MODE_NEWFILE)))
	Cleanup("Amberfile couldn't be opened.");

/*** Write Amberfile header ***/
if (packed)
	{
	/*** Write header ***/
	if (encrypted)
		Write(amberfile,(UBYTE *) Packcrypted_header,4);
	else
		Write(amberfile,(UBYTE *) Packed_header,4);

	Write(amberfile,&count,2);

	/*** Write dummy subfile lengths ***/
	for (i=0;i<count;i++)
		Write(amberfile,&zero,4);
	}
else
	{
	/*** Write header ***/
	if (encrypted)
		Write(amberfile,(UBYTE *) Encrypted_header,4);
	else
		Write(amberfile,(UBYTE *) Unpacked_header,4);

	Write(amberfile,&count,2);
	/*** Write subfile lengths ***/
	for (i=0;i<count;i++)
		Write(amberfile,&lengths[i],4);
	}

/*** Compile amberfile ***/
for (i=0;i<count;i++)
	{
	/*** Load subfile ***/
	Load_subfile();

	if (packed)
		{
		/*** Already packed ? ***/
		if (!(strncmp((char *) unpacked_address,Pack_ID,4)))
			{
			printf("File %s has already been packed.\n",subfiles[i]);
			packed_address = buffer;
			packed_length = lengths[i];
			unpacked_length = lengths[i];
			}
		else
			{
			unpacked_length = lengths[i];

			/*** Empty file ? ***/
			if (!(unpacked_length))
				{
				printf("File %s is empty.\n",subfiles[i]);
				packed_address = buffer;
				packed_length = unpacked_length;
				}
			else
				{
				/*** Pack file ***/
				Pack();

				if (packed_length+4 >= unpacked_length)
					{
					printf("File %s couldn't be packed.\n",subfiles[i]);

					/*** RE-load subfile ***/
					Load_subfile();

					if (encrypted)
						Encrypt_block(i+1,(UWORD *) buffer,lengths[i]);

					packed_address = buffer;
					lengths[i] = unpacked_length+4;
					packed_length = unpacked_length;
					pack_success = FALSE;
					}
				else
					{
					lengths[i] = packed_length+4;
					total_packed = total_packed + 4;

					if (encrypted)
						Encrypt_block(i+1,(UWORD *) (packed_address+4),packed_length-4);
					pack_success = TRUE;
					}
				}
			}
		total_packed = total_packed + packed_length;
		total_unpacked = total_unpacked + unpacked_length;
		}
	else
		{
		if (encrypted)
			Encrypt_block(i+1,(UWORD *) buffer,lengths[i]);

		packed_address = buffer;
		packed_length = lengths[i];
		total_unpacked = total_unpacked + lengths[i];
		}

	/*** Write subfile to Amberfile ***/
	if (packed && packed_length)
		if (pack_success)
			{
			if (Write(amberfile,&Pack_ID,4) != 4)
				{
				printf("File %s could not be written.\n",subfiles[i]);
				Cleanup();
				}
			}
		else
			{
			if (Write(amberfile,&zero,4) != 4)
				{
				printf("File %s could not be written.\n",subfiles[i]);
				Cleanup();
				}
			}
	if (Write(amberfile,packed_address,packed_length) != packed_length)
		{
		printf("File %s could not be written.\n",subfiles[i]);
		Cleanup();
		}
	}

/*** Write subfile lengths (if packing) ***/
if (packed)
	{
	Seek(amberfile,6,OFFSET_BEGINNING);

	/*** Write subfile lengths ***/
	for (i=0;i<count;i++)
		Write(amberfile,&lengths[i],4);
	}

/*** Close amberfile ***/
Close(amberfile);
amberfile = NULL;

/*** Status report ***/
if (count == 1)
	printf("1 file was processed.\n");
else
	printf("%u files were processed.\n",count);

if (packed)
	{
	rate = (100*total_packed)/total_unpacked;
	printf("Amberfile length : %u.\n",total_packed+(count*4)+6);
	printf("Pack rate : %d%%.\n",rate);
	}
else
	printf("Amberfile length : %u.\n",total_unpacked+(count*4)+6);

/*** Exit ***/
Cleanup("");
}

/************************************************************************
 * Clean up & exit							*
 ************************************************************************/
void Cleanup(text)
char *text;
{
	/*** A last message ***/
	if (strlen(text) != 0) printf("%s\n",text);

	/*** Close opened files ***/
	if (amberfile) Close(amberfile);
	if (subfile) Close(subfile);

	/*** Free allocated memory ***/
	if (buffer) FreeMem(buffer,Buffer_size);
	if (pack_bss) FreeMem(pack_bss,BSS_size);

	/*** Exit ***/
        exit(0);
}

/************************************************************************
 * Compare two subfiles							*
 ************************************************************************/
BOOL A_gt_B(a,b)
int a,b;
	{
	int i,la,lb,maxl;
	char *stra,*strb;

	a--;
	b--;

	stra = subfiles[a];
	strb = subfiles[b];

	la = strlen(stra);
	lb = strlen(strb);
	maxl = (la>lb) ? la : lb;

	for (i=0;i<maxl;i++)
		{
		if (stra[i]>strb[i]) return(TRUE);
		if (stra[i]<strb[i]) return(FALSE);
		}

	if (la=maxl)
		return(FALSE);
	else
		return(TRUE);
	}

/************************************************************************
 * Swap two subfiles							*
 ************************************************************************/
void Swap(a,b)
int a,b;
	{
	ULONG tempsize;
	char *tempstring;

	a--;
	b--;

	tempstring = subfiles[b];
	tempsize = lengths[b];
	subfiles[b]=subfiles[a];
	lengths[b]=lengths[a];
	subfiles[a]=tempstring;
	lengths[a]=tempsize;
	}

/************************************************************************
 * Pack a file								*
 ************************************************************************/
Pack()
	{
	#asm
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	_unpacked_length,d0
	addq.l	#1,d0
	and.l	#$fffffffe,d0
	move.l	_unpacked_address,a0
	move.l	_pack_bss,a4

	move.l	a0,a3
	move.l	d0,d3

	move.l	a3,a0
	move.l	d3,d0
	move.l	a0,a1
	add.l	d0,a0
	move.l	a0,a2
	move.l	a1,a3
	move.l	d0,d7
Loop:	move.b	(a3)+,(a2)+
	subq.l	#1,d7
	bne.s	Loop

;a0 = daten-compare
;a1 = daten-original  (wird ueberschrieben)
;a4 = zeiger auf buffer (26124 bytes gross)
;d0 = laenge

l01fd:	bra.s	l0202
l01fe:	dc.w 0,0
l01ff:	dc.w 0,0
l0200:	dc.w 0,0
l0201:	dc.w 0,0
l0202:	lea	l0200(pc),a2
	move.l	a1,(a2)
	lea	l01fe(pc),a2
	move.l	a0,(a2)
	lea	l01ff(pc),a2
	move.l	d0,(a2)
	move.l	d0,(a1)
	move.b	#6,(a1)
	addq.l	#8,a1
;	lea	l0409(pc),a4	;***
	movea.l a4,a5
	adda.l	#$2204,a5
	movea.l a5,a6
	adda.l	#$2204,a6
	bsr	l020e
	lea	l0227(pc),a3
	lea	l0227(pc),a2
	move.b	#0,(a2)+
	moveq	#-$80,d2
	moveq	#0,d1
	bsr	l0210
l0203:	moveq	#0,d7
	move.w	-2(a3),d7
	cmp.l	d7,d0
	bgt.s	l0204
	move.w	d0,-2(a3)
l0204:	cmpi.w	#2,-2(a3)
	bgt.s	l0205
	move.w	#1,-2(a3)
	or.b	d2,(a3)
	move.b	(a0),(a2)+
	bra.s	l0206
l0205:	move.w	-2(a3),d6
	subq.l	#3,d6
	andi.w	#$0f,d6
	move.w	-4(a3),d7
	ror.w	#8,d7
	lsl.b	#4,d7
	or.b	d7,d6
	rol.w	#8,d7
	move.b	d6,(a2)+
	move.b	d7,(a2)+
;	cmpa.l	l015f,a0	;***
;	blt.s	l0206
;	bsr	l0159		;***
l0206:	lsr.b	#1,d2
	bne.s	l0208
l0207:	move.b	(a3)+,(a1)+
	cmpa.l	a3,a2
	bne.s	l0207
	lea	l0227(pc),a3
	lea	l0227(pc),a2
	move.b	#0,(a2)+
	moveq	#-$80,d2
l0208:	moveq	#0,d7
	move.w	-2(a3),d7
	sub.l	d7,d0
	beq.s	l020a
	subq.w	#1,d7
l0209:	addq.l	#1,a0
	addq.l	#1,d1
	andi.w	#$0fff,d1
	bsr	l021d
	bsr	l0210
	dbra	d7,l0209
	bra	l0203
l020a:	cmp.b	#-$80,d2
	beq.s	l020c
l020b:	move.b	(a3)+,(a1)+
	cmpa.l	a3,a2
	bne.s	l020b
l020c:	move.l	a1,d1
	btst	#0,d1
	beq.s	l020d
	move.b	#0,(a1)+
l020d:	lea	l0201(pc),a3
	suba.l	l0200(pc),a1
	move.l	a1,0(a3)
	movea.l l0200(pc),a3
	suba.l	#8,a1
	move.l	a1,4(a3)
	
;	rts
	bra	Exit

l020e:	move.w	#$2204,d7
l020f:	move.w	#$2000,0(a4,d7.w)
	move.w	#$2000,0(a5,d7.w)
	move.w	#$2000,0(a6,d7.w)
	subq.l	#2,d7
	bpl.s	l020f
	rts
l0210:	movem.l d0-a2,-(sp)
	lsl.w	#1,d1
	moveq	#0,d2
	move.b	(a0),d2
	addi.w	#$1001,d2
	lsl.w	#1,d2
	move.w	#$2000,0(a4,d1.w)
	move.w	#$2000,0(a5,d1.w)
	move.w	#0,-2(a3)
	moveq	#1,d3
l0211:	tst.w	d3
	bmi.s	l0213
	cmpi.w	#$2000,0(a5,d2.w)
	beq.s	l0212
	move.w	0(a5,d2.w),d2
	bra.s	l0215
l0212:	move.w	d1,0(a5,d2.w)
	move.w	d2,0(a6,d1.w)
	bra	l021c
l0213:	cmpi.w	#$2000,0(a4,d2.w)
	beq.s	l0214
	move.w	0(a4,d2.w),d2
	bra.s	l0215
l0214:	move.w	d1,0(a4,d2.w)
	move.w	d2,0(a6,d1.w)
	bra	l021c
l0215:	move.l	d1,d0
	sub.w	d2,d0
	asr.w	#1,d0
	andi.w	#$0fff,d0
	movea.l a0,a2
	suba.l	d0,a2
	addq.l	#1,a2
	movea.l a0,a1
	addq.l	#1,a1
	moveq	#-1,d3
	move.w	#$10,d4
l0216:	cmpm.b	(a1)+,(a2)+
	dbne	d4,l0216
	bhi.s	l0217
	moveq	#1,d3
l0217:	move.w	#$11,d5
	sub.w	d4,d5
	cmp.w	-2(a3),d5
	blt.s	l0211
	bgt.s	l0218
	cmp.w	-4(a3),d0
	bge.s	l0211
l0218:	move.w	d0,-4(a3)
	move.w	d5,-2(a3)
	cmp.w	#$12,d5
	blt.s	l0211
	move.w	0(a6,d2.w),0(a6,d1.w)
	move.w	0(a4,d2.w),0(a4,d1.w)
	move.w	0(a5,d2.w),0(a5,d1.w)
	move.w	0(a4,d2.w),d0
	move.w	d1,0(a6,d0.w)
	move.w	0(a5,d2.w),d0
	move.w	d1,0(a6,d0.w)
	move.w	0(a6,d2.w),d0
	cmp.w	0(a5,d0.w),d2
	beq.s	l0219
	cmp.w	0(a4,d0.w),d2
	beq.s	l021a
	illegal
l0219:	move.w	d1,0(a5,d0.w)
	bra.s	l021b
l021a:	move.w	d1,0(a4,d0.w)
l021b:	move.w	#$2000,0(a6,d2.w)
l021c:	movem.l (sp)+,d0-a2
	rts
l021d:	movem.l d0-d2,-(sp)
	lsl.w	#1,d1
	cmpi.w	#$2000,0(a6,d1.w)
	beq	l0226
	cmpi.w	#$2000,0(a5,d1.w)
	bne.s	l021e
	move.w	0(a4,d1.w),d2
	bra.s	l0222
l021e:	cmpi.w	#$2000,0(a4,d1.w)
	bne.s	l021f
	move.w	0(a5,d1.w),d2
	bra.s	l0222
l021f:	move.w	0(a4,d1.w),d2
	cmpi.w	#$2000,0(a5,d2.w)
	beq.s	l0221
l0220:	move.w	0(a5,d2.w),d2
	cmpi.w	#$2000,0(a5,d2.w)
	bne.s	l0220
	move.w	0(a6,d2.w),d0
	move.w	0(a4,d2.w),0(a5,d0.w)
	move.w	0(a4,d2.w),d0
	move.w	0(a6,d2.w),0(a6,d0.w)
	move.w	0(a4,d1.w),0(a4,d2.w)
	move.w	0(a4,d1.w),d0
	move.w	d2,0(a6,d0.w)
l0221:	move.w	0(a5,d1.w),0(a5,d2.w)
	move.w	0(a5,d1.w),d0
	move.w	d2,0(a6,d0.w)
l0222:	move.w	0(a6,d1.w),0(a6,d2.w)
	move.w	0(a6,d1.w),d0
	cmp.w	0(a5,d0.w),d1
	beq.s	l0223
	cmp.w	0(a4,d0.w),d1
	beq	l0224
	illegal
l0223:	move.w	d2,0(a5,d0.w)
	bra.s	l0225
l0224:	move.w	d2,0(a4,d0.w)
l0225:	move.w	#$2000,0(a6,d1.w)
l0226:	movem.l (sp)+,d0-d2
	rts
	dc.w 0,0
l0227:	dc.w 0,0,0,0,0,0,0,0,0

Exit:	move.l	l0200(pc),a0			;adresse
	move.l	l0201(pc),d0			;laenge
	move.l	d0,_packed_length
	move.l	a0,_packed_address
	movem.l	(a7)+,d0-d7/a0-a6
	#endasm
	}

/************************************************************************
 * Get file length							*
 ************************************************************************/
ULONG LOF(path)
char *path;
	{
	struct FileLock *lock;
	struct FileInfoBlock *my_fib;
	ULONG length;

	if (!(my_fib = (struct FileInfoBlock *) AllocMem((ULONG) sizeof(struct FileInfoBlock),0L)))
		length = 0;
	else
		{
		if (lock = Lock(path,(ULONG) ACCESS_READ))
			{
			if (Examine(lock,my_fib))
				length = (ULONG) my_fib->fib_Size;
			else
				length = 0;
			}
		else
			length = 0;

		if (lock) UnLock(lock);
		FreeMem(my_fib,(ULONG) sizeof(struct FileInfoBlock));
		}
	return (length);
	}

/************************************************************************
 * Read info-file
 ************************************************************************/
BOOL Read_info_file(infofilename)
char *infofilename;
	{
	struct FileHandle *infofile;
	int i;
	char *Error,info[4][80];

	/*** Open file ***/
	if (!(infofile = Open(infofilename,(ULONG) MODE_OLDFILE)))
		printf("Info-file couldn't be opened.\n");
	else
		{
		/*** Read up to 4 lines ***/
		do
			{
			Error = (char *) FGets(infofile,(UBYTE *) info[i],(ULONG) 80);
			i++;
			} while (Error != NULL && i<4);

		/*** Close file ***/
		Close(infofile);
		}

	if (i < 4)
		{
		printf("Not a legal info-file.\n");
		return (FALSE);
		}
	else
		{
		/*** Check packed flag ***/
/*		if (strncmp(info[0],"PACKED",6)==0) packed = TRUE; */

		/*** Check encrypted flag ***/
/*		if (strncmp(info[1],"ENCRYPTED",6)==0) encrypted = TRUE; */

		/*** Get paths ***/
		strcpy(sourcefile_path,info[2]);
		strcpy(amberfile_path,info[3]);

		amberfile_path[strlen(amberfile_path)-1] = 0;
		sourcefile_path[strlen(sourcefile_path)-1] = 0;

		return (TRUE);
		}
	}

/************************************************************************
 * Generic Shell-sort
 ************************************************************************/
Shell_sort(swap,compare,count)
void swap();
BOOL compare();
int count;
	{
	int inc,l,ps;

	inc=count;
	while (inc>1)
		{
		inc=inc/2;
		for(l=1;l<=count-inc;l++)
			{
			if ((*compare)(l,l+inc))
				{
				(*swap)(l,l+inc);
				ps=l-inc;
				while (ps>0)
					{
					if ((*compare)(ps,ps+inc))
						{
						(*swap)(ps,ps+inc);
						ps=ps-inc;
						}
					else
						ps=0;
					}
				}
			}
		}
	}

/************************************************************************
 * Get file infos from search pattern
 ************************************************************************/
int Get_file_infos(search_string,ptr)
char *search_string,*ptr;
	{
	struct MyAnchor *my_anchor;
	char *path;
	BOOL error;
	int count;

	/*** Set up anchor ***/
	if (!(my_anchor = (struct MyAnchor *) AllocMem(sizeof(struct MyAnchor),MEMF_CLEAR)))
		{
		printf("Out of memory.\n");
		return(0);
		}
	my_anchor->ap_Strlen = (WORD) Path_length;

	/*** Search all files ***/
	count = 0;
	error = MatchFirst(search_string,my_anchor);
	while (!error)
		{
		/*** Get file length & path ***/
		lengths[count] = ((struct FileInfoBlock *) (&my_anchor->ap_Info))->fib_Size;
		subfiles[count] = ptr;
		path = (char *) (&my_anchor->ap_Buf);
		strcpy(ptr,path);
		ptr += strlen(path)+1;
		count++;

		error = MatchNext(my_anchor);	/* Next file */
		}

	MatchEnd(my_anchor);			/* Destroy anchor */
	FreeMem(my_anchor,sizeof(struct MyAnchor));

	return(count);				/* Return number of files */
}

/************************************************************************
 * Encrypt block
 ************************************************************************/
void Encrypt_block(seed,ptr,length)
UWORD seed,*ptr;
ULONG length;
	{
	UWORD v;
	ULONG j;

	for (j=0;j<=length/2;j++)
		{
		v = *ptr;
		v = v^seed;
		*(ptr++) = v;
		seed = seed*17+87;
		}
	}

/************************************************************************
 * Load subfile
 ************************************************************************/
void Load_subfile()
	{
	/*** Try to open subfile ***/
	if (!(subfile = (struct FileHandle *) Open(subfiles[i],MODE_OLDFILE)))
		{
		printf("File %s couldn't be opened.\n",subfiles[i]);
		Cleanup("");
		}

	/*** Read subfile ***/
	if (Read(subfile,unpacked_address,lengths[i]) != lengths[i])
		{
		printf("File %s could not be read.\n",subfiles[i]);
		Cleanup();
		}

	/*** Close subfile ***/
	Close(subfile);
	subfile = NULL;
	}
