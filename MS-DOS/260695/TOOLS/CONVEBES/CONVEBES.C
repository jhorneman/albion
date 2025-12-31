/************
 * NAME     : CONVEBES.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 8-2-1995
 * PROJECT  : CONVert LBM-picture to EBEs tool
 * NOTES    : - Contains LBM-code by Maverick.
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBOPM.H>
#include <BBDSA.H>
#include <BBMISC.H>
#include <BBDOS.H>
#include <BBERROR.h>
#include <dos.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys\stat.h>

/* structure definitions */

struct LBM
{
	UNBYTE * lbmptr;	/* pointer to buffer of LBM file */
	SILONG lbmsize;		/* size of LBM file */

	UNBYTE * bmhdptr;	/* pointer to BMHD chunk */
	UNBYTE * bodyptr;	/* pointer to BODY chunk */
	UNBYTE * cmapptr;	/* pointer to CMAP chunk */
};

/* global variables */

struct LBM mylbm;
UNSHORT	iffhoehe, iffbreite;
UNBYTE *out;

/* prototypes */

void main(int argc, char** argv);
UNBYTE *FindChunk( struct LBM * lbmptr, UNCHAR chunk[4] );
void Main_error_output_function(UNCHAR * buffer);

/* yo-ho */

void
main(int argc, char** argv)
{
	/* Exit if number of parameters is wrong */
	if (argc != 3)
	{
		printf("\n");
		printf("Syntax : ConvEBES {.LBM filename} {output filename}.\n");
		printf("Written by J.Horneman.\n");
		printf("Start : 27-7-1994.\n");
		printf("\n");
		printf("This tool will convert an .LBM-picture to raw EBEs.\n");
		printf("\n");
		return;
	}

	ERROR_Init(&Main_error_output_function);

	BASEMEM_Init();

	printf("\nSource : %s.\n",argv[1]);
	printf("Target : %s.\n",argv[2]);

	/* load LBM file */

	{
		int fh;

		fh = open(argv[1], O_RDONLY|O_BINARY);
		mylbm.lbmsize = filelength(fh);
		mylbm.lbmptr = BASEMEM_Alloc(mylbm.lbmsize, BASEMEM_Status_Flat);
		read(fh,mylbm.lbmptr,mylbm.lbmsize);
		close(fh);
	}

	/* search BMHD chunk */

	if( ( mylbm.bmhdptr = FindChunk( &mylbm, "BMHD" ) ) == NULL )
		return;

	/* search BODY chunk */

	if( ( mylbm.bodyptr = FindChunk( &mylbm, "BODY" ) ) == NULL )
		return;

	Convert();

	/* save raw data */
	{
		int fh;

		/* Open file */
		fh = open(argv[2],(UNSHORT) O_WRONLY|O_CREAT|O_BINARY,
		 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if (fh != -1)
		{
			UNSHORT i, j, k;
			UNBYTE *Ptr, *Ptr2, *Ptr3;

			/* Write ebes */
			Ptr = out;
			for (i=0;i<iffbreite/8;i++)
			{
				Ptr2 = Ptr;
				for (j=0;j<iffhoehe/8;j++)
				{
					Ptr3 = Ptr2;
					for (k=0;k<8;k++)
					{
						Write(fh, Ptr3, 8);
						Ptr3 += iffbreite;
					}
					Ptr2 += iffbreite * 8;
				}
				Ptr += 8;
			}

			/* Close file */
			close(fh);
		}
	}

	/* free memory of LBM */

	BASEMEM_Free(out);

	BASEMEM_Free( mylbm.lbmptr );

	BASEMEM_Exit();

	ERROR_PrintAllErrors(BBERROR_PAE_NORMAL);
}

void
Convert(void)
{
	/* locals */

	UNSHORT	x, y, i, j;
	UNBYTE 	color;
	UNBYTE	cbyte;

	UNBYTE * wbodyptr;

	UNSHORT iffpacked;

	UNBYTE * bmhdptr;
	UNBYTE *out_ptr;

	/* get pointer to BMHD chunk */

	bmhdptr = mylbm.bmhdptr;

	/* get LBM parameter */

	iffbreite = abs( ( ( ( UNSHORT ) *( bmhdptr +  8 ) ) << 8 ) + * ( bmhdptr +  9 ) );
	iffhoehe  = abs( ( ( ( UNSHORT ) *( bmhdptr + 10 ) ) << 8 ) + * ( bmhdptr + 11 ) );
	iffpacked = *( bmhdptr + 18 );

	/* ALLOCATE WORK BUFFER */
	out = BASEMEM_Alloc((UNLONG) iffbreite * iffhoehe, BASEMEM_Status_Flat);
	out_ptr = out;

	/* set work body */

	wbodyptr = mylbm.bodyptr + 8L;

	/* display lbm file */

	if ( iffpacked == 0 )
	{
		/* non packed lbm file */

		for( y = 0 ; y < iffhoehe ; y++ )
		{
			for( x = 0 ; x < iffbreite ; x++ )
			{
				color = *wbodyptr++;

				*out_ptr++ = color;
			}
		}
	}
	else
	{
		/* packed lbm */

		/* init out pos */

		x = 0;
		y = 0;

		/* depack it */

		do
		{
			/* get todo byte */

			cbyte = *wbodyptr++;


			/* work on byte */

			if( cbyte < 0x80 )
			{
				/* copy bytes */

				/* number of bytes to copy */

				cbyte++;
				j = cbyte;

				/* copy it */

				for ( i=0; i<j; i++ )
				{
					color = *wbodyptr++;
					*out_ptr++ = color;
					x++;
				}

			}
			else if ( cbyte > 0x80 )
			{
				/* fill with one byte */

				/* number of bytes to fill up */

				cbyte = ~cbyte + 2;
				j = cbyte;

				/* get byte to fill with */

				color = *wbodyptr++;

				/* fill it */

				for ( i=0; i<j; i++ )
				{
					*out_ptr++ = color;
					x++;
				}
			}


			/* line done ? */

			if ( x == iffbreite )
			{
				/* yes: start next line */

				x = 0;
				y++;
			}

		}
		while ( y < iffhoehe );
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FindChunk
 * FUNCTION  : Search a chunk in LBM file memory.
 * FILE      : LBM.C
 * AUTHOR    : MAVERICK
 * FIRST     : 20.07.94
 * LAST      :
 * INPUTS    : struct LBM * lbmptr: pointer to LBM structure.
 *             UNCHAR chunk[4]: chunk to look for.
 * RESULT    : UNBYTE * : pointer to chunk or NULL if chunk not found.
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
FindChunk( struct LBM * lbmptr, UNCHAR chunk[4] )
{
	/* locals */

	UNBYTE * wptr;
	SILONG wsize;
	UNLONG os;
	UNSHORT i, count;


	/* set work pointer */

	wptr = lbmptr->lbmptr;

	/* get size of lbm */

	wsize = lbmptr->lbmsize;


	/* search chunk */

	for ( os=0 ; os<wsize ; os++ )
	{

		count = 0;

		for ( i=0 ; i<4 ; i++ )
			if ( ( *( wptr + os +i ) ) == chunk[i] )
				count++;

		if ( count == 4 )
			return( lbmptr->lbmptr + os );
	}


	/* chunk not found */

	return ( NULL );
}

/* #FUNCTION END# */

void
Main_error_output_function(UNCHAR * buffer)
{
	printf("%s",(char *) buffer);
}

