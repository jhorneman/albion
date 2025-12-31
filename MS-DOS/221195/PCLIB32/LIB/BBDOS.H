#ifndef BBDOS_H
#define BBDOS_H

/************
 * NAME     :  BBDOS.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/

/* Includes */
#include <BBDEF.h>



/* Defines */


/* Maximum number of files to handle */
#define BBDOS_MAXFILES			50

/* Maximale L„nge eines Filenamens */
#define BB_MAX_FILENAME_LENGTH	90

/* Size of IO Buffer */
#define DOS_IOBUFFER_SIZE		4096

/* Defines for BBDOS_FileHandleStruct.status */
#define BBDOSFILESTAT_FREE		0				/* Free entry */
#define BBDOSFILESTAT_OPEN		( 1 << 0 )		/* Used entry (file is opened) */

/* File modes (used for DSA_Open) */
#define BBDOSFILESTAT_READ		( 1 << 1 )		/* Open file with read permission */
#define BBDOSFILESTAT_WRITE		( 1 << 2 )		/* Open file with write permission, delete old file */
#define BBDOSFILESTAT_READWRITE		( 1 << 3 )		/* Open file with write permission, delete old file */
#define BBDOSFILESTAT_APPEND	( 1 << 4 )		/* Open file with write permission, append on end of old fileÊ*/

/* Defines for seek mode */

#define BBDOS_SEEK_CURRENTPOS	0	/* Seek from current position */
#define BBDOS_SEEK_START		1	/* Seek from start of file */
#define BBDOS_SEEK_EOF			2	/* Seek from end of file */
#define BBDOS_SEEKMODES			3	/* Number of seek modes */

/* Typedefs */


/* Structures */


/* BBDOS internal file handling structure */

struct BBDOS_FileHandleStruct
{
	/* System independent elements */

	UNSHORT	status;		/* Status flag set */

	/* System dependent elements */

	SISHORT	refNum;		/* File reference number*/
};

/* Prototypes */

/* Global functions */

extern BOOLEAN DOS_Init( void );
extern void DOS_Exit( void );

extern SISHORT DOS_Open( UNCHAR * filenameptr, UNSHORT filemode );
extern BOOLEAN DOS_Close( SISHORT fh );

extern SILONG DOS_Read( SISHORT fh, UNBYTE * bufptr, UNLONG bytestoread );
extern SILONG DOS_Write( SISHORT fh, UNBYTE * bufptr, UNLONG bytestowrite );
extern BOOLEAN DOS_Seek( SISHORT fh, UNSHORT seekmode, SILONG offset );

extern UNBYTE * DOS_ReadFile( UNCHAR * filenameptr, UNBYTE * filebufferptr, UNBYTE * ioptr );
extern BOOLEAN DOS_WriteFile( UNCHAR * filenameptr, UNBYTE * filebufferptr, UNLONG filelength );
extern BOOLEAN DOS_Delete( UNCHAR * filenameptr );
extern SILONG DOS_GetFileLength( UNCHAR * filenameptr );

extern SILONG DOS_GetSeekPosition( SISHORT fh );

extern BOOLEAN DOS_Exist(UNCHAR *filenameptr);
extern UNCHAR *DOS_GetCurrentDir( void );
extern BOOLEAN DOS_SetCurrentDir(UNCHAR *path);
extern BOOLEAN DOS_Eof(SISHORT fh);

#endif
