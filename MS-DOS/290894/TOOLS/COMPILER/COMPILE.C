/************
 * NAME     : COMPILE.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 22-7-1994
 * PROJECT  : XLD Library compiler
 * NOTES    :
 * SEE ALSO : XLOAD.C
 ************/

/* includes */

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <XLOAD.H>
#include <dos.h>
#include <stdio.h>
#include <fcntl.h>

/* defines */

#define PATH_LENGTH	(100)
#define INFO_LINES	(6)
#define BUFFER_SIZE	(1000000)

/* global variables */

int Library, Subfile;

BOOLEAN Packed, Encrypted;

UNLONG Lengths[SUBFILES_MAX];
UNLONG Zero = 0;

UNSHORT Nr_subfiles,Max_subfiles,Highest_subfile;

UNCHAR *Subfiles[SUBFILES_MAX];
UNCHAR Subfilenames[SUBFILES_MAX*PATH_LENGTH];

UNCHAR Source_path[PATH_LENGTH],Library_path[PATH_LENGTH];
UNCHAR Source_fname[15];

UNCHAR XLD_ID[4] = {'X','L','D','0'};
UNCHAR Format = 'I';

UNBYTE Library_types[4] = {
	XLD_NORMAL,
	XLD_PACKED,
	XLD_ENCRYPTED,
	XLD_PACKED_ENCRYPTED
};

UNBYTE *Buffer;

UNBYTE *Output_address;
UNLONG Output_length, Total_unpacked;

UNSHORT Nr_index;

/* prototypes */

void BBMAIN(int argc, char **argv);
BOOLEAN Read_info_file(UNCHAR *Path);
void Get_subfile_data(UNCHAR *Path);
void Encrypt_block(UNSHORT seed, UNSHORT *ptr, UNLONG length);
void Load_subfile(UNSHORT i);

/* yo-ho */

void
BBMAIN(int argc, char **argv)
{
	UNSHORT i;

	/* Exit if number of parameters is wrong */
	if (argc != 2)
	{
		printf("\n");
		printf("Syntax : Compile {infofilename}.\n");
		printf("Written by J.Horneman.\n");
		printf("Start : 22-7-1994.\n");
		printf("\n");
		printf("This tool will compile an XLD-library using instructions from the\n");
		printf("infofile, which should have the following format :\n");
		printf("\n");
		printf("PACKED          (if you want the subfiles to be packed)\n");
		printf("ENCRYPTED       (if you want the subfiles to be encrypted)\n");
		printf("[Source_path]   (where the subfiles are, just the path!)\n");
		printf("[Source_file]   (which must contain the ???-wildcard)\n");
		printf("[Library_path]  (i.e. the full path of the target library)\n");
		printf("[Max_subfiles]  (optionally, the number of subfiles in the library)\n");
		printf("\n");
		return;
	}

	/* Try to read info-file */
	if (!Read_info_file(argv[1]))
		return;

	/* Feedback */
	printf("\n");
	printf("************************************************************\n");
	printf("Compiling XLD-library.\n");
	printf("Source : %s%s.\n",Source_path,Source_fname);
	printf("Target : %s.\n",Library_path);
	if (Packed)
		printf("Packed / ");
	else
		printf("Unpacked / ");
	if (Encrypted)
		printf("encrypted.\n");
	else
		printf("not encrypted.\n");
	printf("************************************************************\n");
	printf("\n");

	/* Read paths and lengths of subfiles */
	Get_subfile_data(Source_path);
	if (!Nr_subfiles)
	{
		printf("No subfiles were found.");
		return;
	}

	/* Implement maximum number of subfiles */
	if (Max_subfiles)
	{
		Highest_subfile = Max_subfiles;
		if (Nr_subfiles > Max_subfiles)
			Nr_subfiles = Max_subfiles;
	}

	/* Try to get work memory */
	Buffer = BASEMEM_Alloc(BUFFER_SIZE, BASEMEM_Status_Flat);
	if (!Buffer)
	{
		printf("Not enough memory!\n");
		return;
	}

	/* Try to open library */
	Library = open(Library_path,O_WRONLY|O_CREAT|O_BINARY);
	if (!Library)
	{
		printf("Library couldn't be opened.\n");
		return;
	}

	/* Write library header */
	write(Library,&XLD_ID[0],4);
	write(Library,&Format,1);

	if (Packed)
	{
		if (Encrypted)
			write(Library,&Library_types[XLD_PACKED],1);
		else
			write(Library,&Library_types[XLD_PACKED_ENCRYPTED],1);

		write(Library,&Highest_subfile,2);

		/* Write dummy subfile lengths */
		for (i=1;i<=Highest_subfile;i++)
			write(Library,&Zero,4);
	}
	else
	{
		if (Encrypted)
			write(Library,&Library_types[XLD_ENCRYPTED],1);
		else
			write(Library,&Library_types[XLD_NORMAL],1);

		write(Library,&Highest_subfile,2);

		/* Write subfile lengths */
		for (i=1;i<=Highest_subfile;i++)
			write(Library,&Lengths[i],4);
	}

	/* Compile library */
	for (i=1;i<=Highest_subfile;i++)
	{
		if (Subfiles[i])
		{
			/* Load subfile */
			Load_subfile(i);

			if (!Packed)
			{
				if (Encrypted)
					Encrypt_block(i+1, (UNSHORT *) Buffer, Lengths[i]);

				Output_address = Buffer;
				Output_length = Lengths[i];
				Total_unpacked += Lengths[i];
			}

			/* Write subfile to Omnifile */
			write(Library,Output_address,Output_length);
		}
	}

	/* Write subfile lengths (if packing) */
	if (Packed)
	{
		lseek(Library,sizeof(struct XLD_Library),SEEK_SET);

		/* Write subfile lengths */
		for (i=1;i<=Highest_subfile;i++)
			Write(Library,&Lengths[i],4);
	}

	/* Close library */
	close(Library);

	/* Status report */
	if (Nr_subfiles == 1)
		printf("1 file was processed, ");
	else
		printf("%u files were processed, ",Nr_subfiles);

	if (Highest_subfile == 1)
		printf("1 was generated.\n");
	else
		printf("%u were generated.\n",Highest_subfile);

	printf("Library length : %u.\n",Total_unpacked+(Highest_subfile*4)+
	 sizeof(struct XLD_Library));
}

/************************************************************************
 * Read and interpret info-file
 ************************************************************************/
BOOLEAN
Read_info_file(UNCHAR *Path)
{
	FILE *File;
	UNCHAR Info[INFO_LINES][200], *p;
	UNSHORT i = 0;

	/* Open info-file */
	File = fopen(Path,"r");
	if (!File)
	{
		printf("Info-file %s couldn't be opened.\n",Path);
		return(FALSE);
	}

	/* Yes -> Read strings */
	while (fgets(Info[i],200,File))
	{
		/* Until EOF or INFO_LINES strings have been read */
		i++;
		if (i==INFO_LINES)
			break;
	}
	/* Close file */
	fclose(File);

	/* Exit if less than INFO_LINES strings were read */
	if (i!=INFO_LINES)
	{
		printf("Illegal info-file %s.\n",Path);
		return(FALSE);
	}

	/* Check packed flag */
	if (strncmp(Info[0],"PACKED",6)==0) Packed = TRUE;

	/* Check encrypted flag */
	if (strncmp(Info[1],"ENCRYPTED",9)==0) Encrypted = TRUE;

	/* Get paths and filename */
	strcpy(Source_path,Info[2]);
	strcpy(Source_fname,Info[3]);
	strcpy(Library_path,Info[4]);

	/* Remove EOLs */
	Source_path[strlen(Source_path)-1] = 0;
	Source_fname[strlen(Source_fname)-1] = 0;
	Library_path[strlen(Library_path)-1] = 0;

	/* Find number index */
	p = (UNCHAR *) strstr(Source_fname,"???");
	if (!p)
		{
		printf("Source-filename does not contain ???.\n");
		return(FALSE);
		}
	Nr_index = p-Source_fname;

	/* Get maximum number of subfiles */
	if (Info[5]!="")
		Max_subfiles = atoi(Info[5]);

	return(TRUE);
}

/************************************************************************
 * Get paths and lengths of all subfiles
 ************************************************************************/
void
Get_subfile_data(UNCHAR *Source_path)
{
	struct find_t Fileinfo;
	unsigned rc;
	UNCHAR *Fname, *Ptr, Nr[4];
	UNCHAR Path[PATH_LENGTH];
	UNSHORT i,l;

	/* Build complete path */
	strcpy(Path,Source_path);
	strcat(Path,Source_fname);

	/* Clear arrays */
	for (i=0;i<SUBFILES_MAX;i++)
	{
		Subfiles[i]=NULL;
		Lengths[i]=0;
	}

	/* Reset variables */
	Nr_subfiles = 0;
	Highest_subfile = 0;
	Ptr = Subfilenames;

	/* Find all files */
	rc = _dos_findfirst(Path,_A_NORMAL,&Fileinfo);
	while (!rc)
	{
		/* Get path of current file */
		Fname = (UNCHAR *) Fileinfo.name;

		/* Extract subfile number from path */
		Nr[0]=Fname[Nr_index];
		Nr[1]=Fname[Nr_index+1];
		Nr[2]=Fname[Nr_index+2];
		Nr[3]=0;
		i = atoi(Nr);

		/* Is zero ? */
		if (i)
		{
			/* No -> Too large ? */
			if (i>SUBFILES_MAX)
				/* Yes -> Tell off */
				printf("Subfile number %d too large!\n",i);
			else
			{
				/* No -> Increase maximum subfile number if necessary */
				if (i>Highest_subfile)
					Highest_subfile = i;

				/* Store length and path of subfile */
				Lengths[i] = Fileinfo.size;
				Subfiles[i] = Ptr;

				// printf(">>>file %u : %s, %u bytes.\n",i,Fname,Lengths[i]);

				/* Copy path */
				strcpy(Ptr,Source_path);
				Ptr += strlen(Source_path);
				strcpy(Ptr,Fname);
				Ptr += strlen(Fname)+1;

				/* Increase total number of subfiles */
				Nr_subfiles++;
			}
		}
		/* Find next file */
		rc = _dos_findnext(&Fileinfo);
	}
}

/************************************************************************
 * Encrypt block
 ************************************************************************/
void
Encrypt_block(UNSHORT seed, UNSHORT *ptr, UNLONG length)
{
	UNSHORT v;
	UNLONG j;

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
void
Load_subfile(UNSHORT i)
{
	int fh;
	UNLONG Length;

	fh = open(Subfiles[i],O_RDONLY|O_BINARY);
	Length = filelength(fh);
	read(fh,Buffer,Length);
	close(fh);
}

