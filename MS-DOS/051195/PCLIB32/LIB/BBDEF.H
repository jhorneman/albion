#ifndef BBDEF_H
#define BBDEF_H

/************
 * NAME     :  BBDEF.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/

/* Defines */

/* Operating system and machine */

#define BB32_PCVESA
/*
#define BB32_POWERMAC
#define BB32_MAC
#define BB32_PCWIN32S
*/


/* Bytes in word order */

#ifdef BB32_POWERMAC
	#define BB32_BIGENDIAN
#endif

#ifdef BB32_MAC
	#define BB32_BIGENDIAN
#endif

#ifdef BB32_PCVESA
	#define BB32_LITTLEENDIAN
#endif

#ifdef BB32_PCWIN32S
	#define BB32_LITTLEENDIAN
#endif


/* Bool */

#define OR  ||
#define AND &&
#define NOT !
#define EOR ^
#define XOR ^

/* Begrenzt Wert auf minimal und maximalen */
#define LIMIT(val,min,max) ( (val) < (min) ? (min) : ((val) > (max) ? (max) : (val) ) )

/* Limit mit Wraparound (funktioniert nicht rekursiv) */
#define LIMITWRAP(val,min,max) ( (val) < (min) ? (val+max-min) : ((val) > (max) ? (val-max+min) : (val) ) )

/* Std. */

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

typedef SISHORT			BOOLEAN;	/* Boolean (TRUE or FALSE) */
typedef BOOLEAN *		BOOLEAN_P;



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