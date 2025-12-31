

/*

----------------------------------------------------------------------

 MODULE...............	ddt.h
 PROJECT..............	3DM - TEXTURE-MAP-MODUL

 DESCRIPTION..........	This module includes all the DDT
			data structures, typedef`s and define`s.

 DATE OF CREATION.....	15. Juli 1994
 LAST CHANGE..........	15. Juli 1994
 AUTHOR...............	JÅrgen Friedrich
 LANGUAGE.............	Turbo C++ 1.0 fÅr PCompatible

----------------------------------------------------------------------

*/



	typedef	  signed char	SIBYTE;
	typedef	unsigned char	UNBYTE;
	typedef	  signed short	SISHORT;
	typedef	unsigned short	UNSHORT;
	typedef	  signed long	SILONG;
	typedef	unsigned long	UNLONG;
	typedef		 float	SIFLOAT;
	typedef		 double	SIDOUBLE;

	typedef	SISHORT *	SISHORT_P;
	typedef	UNSHORT	*	UNSHORT_P;
	typedef	SILONG	*	SILONG_P;
	typedef	UNLONG	*	UNLONG_P;
	typedef	SIFLOAT  *	SIFLOAT_P;
	typedef	SIDOUBLE *	SIDOUBLE_P;



	#define	First_Wall		101

	#define	OBJECTS_PER_GROUP	8

	#define	OBJECT_DISTORTION	(1 << 2)

	#define	MASKED_WALL		(1 << 5)
	#define	TRANSPARENT_WALL	(1 << 6)



	typedef	struct

	{	UNLONG	Floor_bits;
		UNBYTE	Nr_frames;
		UNBYTE	_pad01;
		UNSHORT	Graphics_nr;
	}

	Floor_3D;


	typedef	struct

	{	SISHORT	X,Y,Z;
		UNSHORT	Number;
	}

	Object_in_group_3D;


	typedef	struct

	{	UNSHORT			Automapper_icon;
		Object_in_group_3D	object[ OBJECTS_PER_GROUP];
	}

	Object_group_3D;


	typedef	struct

	{	UNLONG	Status_bits;
		UNSHORT	Graphics_nr;
		UNBYTE  Nr_frames;
		UNBYTE	_pad01;
		UNSHORT	Graphics_width;
		UNSHORT	Graphics_height;
		UNSHORT	Object_width;
		UNSHORT	Object_height;
	}

	Object_3D;


	typedef	struct

	{	UNLONG	Status_bits;
		UNSHORT	Graphics_nr;
		UNBYTE  Nr_frames;
		UNBYTE	Automapper_icon;
		UNSHORT	Transparent_color;
                UNSHORT	Graphics_width;
		UNSHORT	Graphics_height;
		UNSHORT	Nr_overlays;
	}

	Wall_3D;


	typedef	struct

	{	UNSHORT	Graphics_nr;
		UNBYTE  Nr_frames;
		UNBYTE	Mask_flag;
		UNSHORT	X,Y;
                UNSHORT	Graphics_width;
		UNSHORT	Graphics_height;
		UNSHORT	Nr_overlays;
	}

	Overlay_3D;


	typedef	struct

	{	UNLONG	X,Z;
		UNSHORT	Group_nr;
		UNSHORT	NPC_nr;
	}

	NPC_3D;

