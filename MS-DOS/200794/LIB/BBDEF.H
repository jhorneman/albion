#ifndef BBDEF_H
#define BBDEF_H

/************
 * NAME     :  BBDEF.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/


/* Defines */

#define OR  ||
#define AND &&
#define NOT !
#define EOR ^
#define XOR ^


#ifndef NULL
#define NULL 0L
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif



/* Typedefs */

typedef unsigned char	UNCHAR;		/* 8 bit integer */
typedef UNCHAR *		UNCHAR_P;
typedef signed char		SIBYTE;
typedef SIBYTE *		SIBYTE_P;
typedef unsigned char	UNBYTE;
typedef UNBYTE *		UNBYTE_P;

typedef signed short	SISHORT;	/* 16 bit integer */
typedef SISHORT *		SISHORT_P;
typedef unsigned short	UNSHORT;
typedef UNSHORT *		UNSHORT_P;

typedef signed int		SILONG;		/* 32 bit integer */
typedef SILONG *		SILONG_P;

typedef unsigned int	UNLONG;
typedef UNLONG *		UNLONG_P;

typedef float			SIFLOAT;	/* 32 bit floating point */
typedef SIFLOAT *		SIFLOAT_P;

typedef double			SIDOUBLE;	/* 64 bit floating point */
typedef SIDOUBLE *		SIDOUBLE_P;



/* Structures */


/* Retangle */

struct BBRECT
{
	SISHORT	left;		/* Left */
	SISHORT	top;		/* Top */
	SISHORT	width;		/* Width */
	SISHORT	height;		/* Height */
};



/* Point */

struct BBPOINT
{
	SISHORT x;			/* X Pos */
	SISHORT y;			/* Y Pos */
};




#endif

