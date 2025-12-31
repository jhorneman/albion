/************
 * NAME     :  Binder.C         AUTOR :  PL , BlueByte
 * PROJECT  :  TCT
 * DATE     :  09-09-93 10:26am   START :  09-09-93 10:26am
 * FUNCTION :  Implementation des Binders aus der SGOS32 Library als
 *				Stand-Alone Programm
 * SYNOPSIS :
 * NOTES    :  Implementierte Funktionen:
 *				- Volle Konvertierung == Zusammenbinden mehrerer Dateien
 *				    zu einer Library. Durch die Tatsache, da· mehrere
 *					Inputfiles existieren, ist das Initialisieren des Filters
 *					nicht durch einen Standardzugriff durchzufÅhren
 *
 *				ACHTUNG:
 *				Im Augenblick kînnen nur Files mit einer Extension bearbeitet
 *				werden. FÅr eine Verbesserung ist ein neuer Spezifizierer
 *				zu definieren, der die öbergabe einer ganzen Fileliste
 *				ermîglicht.
 *				Besser noch: Den ganzen MÅll wegwerfen und diese Funktion
 *				direkt in den TCT implementieren.
 * BUGS     :
 * SEE ALSO :
 * UPDATE   :
 ************/



#include <stdio.h>

/* Bluebyte includes */
#include <LIB\INCLUDE\bb.h>
#include <LIB\INCLUDE\bbport.h>
#include <LIB\INCLUDE\bbvesa.h>
#include <LIB\INCLUDE\sgosbase.h>


 /*
--- Project Includes ---------------------------------------------------------
 */
#include <lib\tct.h>

 /*
--- Externe Variablen --------------------------------------------------------
 */
extern struct bbport BB;





 /*
--- Globale Variablen --------------------------------------------------------
 */
char	zs_InputPath[MAXPATH+1], zs_OutputPath[MAXPATH+1], zs_Library[MAXPATH+1];
void	*p_CommandStream=NULL, *p_ProtokollStream;
struct s_fileheader		InputDirectory, OutputDirectory;
unsigned long	ul_Processor = 0;




 /*
    Standardstrukturen und Variablen --------------------------------------------
 */
struct Version_struct{
	unsigned long	Type;
	unsigned long	Version;
	unsigned long	Release;
	unsigned long	Day;
	unsigned long	Month;
	unsigned long	Year;
	char			Author[MAXAUTHOR];
} Version = { 2, 1, 0, 9, 9, 93, "COOLHAND"};


char	KnownCommands[] 		= "1:3:4:5:";
char	KnownSpecifiers[] 		= "FULLCONVERT:";
char	Sourcetext[] 			= "Beliebige Dateien";
char	Sourceext[]				= ".*  ";
char	Sourceshort[]			= "Binder";
char	Targettext[] 			= "Blue Byte Library";
char	Targetext[]				= ".LIB";
char	Targetshort[]			= "LIBRARY";


/*
******************************************************************************
* #FUNCTION HEADER BEGIN#
* NAME      : IdentifyYourself
* FUNCTION  : Schreibt die Identifizierungsdaten des Filters ins Protokollfile
*
* INPUTS    : char *lpzs_Command	// Name des Steuerungsfiles
*			  char *lpzs_Protokoll  // Name des Protokollfiles
*
* RESULT    : int    0=Ok, -1=Fileerror
*
* FILE      : TEST.C
* AUTHOR    : COOLHAND
* FIRST     : 09-09-93 12:05:11pm
* LAST      : 09-09-93 12:05:11pm
* BUGS      :
* NOTES     :
* SEE ALSO  :
* VERSION   : 0.1
* #FUNCTION HEADER END#
*/

/* #FUNCTION BEGIN# */

int IdentifyYourself( void *p_ProtokollStream )
{

	// Schreiben der generellen RÅckmeldung
	if( P_WriteResult( p_ProtokollStream, 0, 0 ) != 0 )
		return( -1 );

	// Schreiben der Versionsinformation
	if( P_WriteVersion( p_ProtokollStream, Version.Type, Version.Version, Version.Release, Version.Day, Version.Month, Version.Year, Version.Author ) != 0 )
		return( -1 );

	if( P_WriteKnownCommands( p_ProtokollStream, KnownCommands ) != 0 )
		return( -1 );

	if( P_WriteKnownSpecifiers( p_ProtokollStream, KnownSpecifiers ) != 0 )
		return( -1 );

	if( P_WriteSourceText( p_ProtokollStream, Sourcetext ) != 0 )
		return( -1 );

	if( P_WriteSourceExt( p_ProtokollStream, Sourceext ) != 0 )
		return( -1 );

	if( P_WriteSourceShort( p_ProtokollStream, Sourceshort ) != 0 )
		return( -1 );


	if( P_WriteTargetText( p_ProtokollStream, Targettext ) != 0 )
		return( -1 );

	if( P_WriteTargetExt( p_ProtokollStream, Targetext ) != 0 )
		return( -1 );

	if( P_WriteTargetShort( p_ProtokollStream, Targetshort ) != 0 )
		return( -1 );

	return( 0 );
}

/* #FUNCTION END# */












/*
******************************************************************************
* #FUNCTION HEADER BEGIN#
* NAME      : BindObjects
* FUNCTION  :
*
* INPUTS    :
*
* RESULT    : int
*				0=Ok, -1=Fileerror, -2=Kein Speicher, -3=Filename fehlerhaft
*
* FILE      : TEST.C
* AUTHOR    :
* FIRST     : 09-09-93 01:09:35pm
* LAST      : 09-09-93 01:09:35pm
* BUGS      :
* NOTES     :
* SEE ALSO  :
* VERSION   : 0.1
* #FUNCTION HEADER END#
*/

/* #FUNCTION BEGIN# */

int BindObjects( char	*pzs_InputPath, struct s_fileheader *p_InputDirectory,char	*pzs_OutputPath, struct s_fileheader *p_OutputDirectory,   void *p_ProtokollStream, unsigned long ul_Command )
{

	struct s_file			*Datei;
	int						i;

/*
	// Dateien mit der LÑnge 0 sind TôDLICH fÅr die FIL_SAVE - Funktion,die
	// im Binder verwendet wird. Also raus damit!!
	// Au·erdem werden nur die Files mit der richtigen Extension dringelassen
	i = 0;
	while( i<p_InputDirectory->count )
	{
		Datei = S_GotoEntry( p_InputDirectory, i );
		if( Datei->length == 0 )
			S_RemoveEntry( p_InputDirectory, Datei );
		else
			i++;
	};
*/

	if( p_InputDirectory->count == 0 )
		return(0);

	// Zusammenbau des Namens der Ziellibrary
	strncpy( zs_Library, pzs_OutputPath, MAXPATH );
	if( (Datei = S_GotoEntry( p_OutputDirectory, 0 )) == NULL )
		return( -3 );
	if( zs_Library[ strlen(zs_Library) -1] != 92 );
		strcat( zs_Library, "\\" );
	strncat( zs_Library, Datei->name, MAXPATH-strlen(zs_Library));


	// Aufruf des Binders
	if( BND_BindTogether( p_InputDirectory, pzs_InputPath,zs_Library, ul_Command, ul_Processor) == 0)
	{
		// Schreiben der positiven RÅckmeldung
		if( P_WriteResult( p_ProtokollStream, 0, 0 ) != 0 )
			return( -1 );
		return( 0 );
	}else{
		// Fehler
		if( P_WriteResult( p_ProtokollStream, -1, 0 ) != 0 )
			return( -1 );

		return( 0 );
	};
}

/* #FUNCTION END# */













#ifdef DEBUG
	int dargc=5;
	char s0[] = "pit", s1[]= "/c:5", s2[] = "/i:D:\\testdata\\*.*  ", s3[] = "/o:C:\\TEMP\\Total.LIB", s4[] = "/s:D:\\TESTDATA\\STEUERUNG.$S$", s5[] = "/p:Protokoll.txt";
//	char s0[] = "pit", s1[]= "/c:5", s4[] = "/s:c:\\temp\\steuerun.TXT", s5[] = "/p:Protokoll.txt", s2[]="", s3[]="";
	char *dargv[] = { s0, s1, s4 ,s5, s2, s3 };
#endif


int main( int	argc, char *argv[] )
{

	// Lokale Variablen
	unsigned long	ul_Command;
	int				n_Error, i, n_Input=0, n_Output;





	// ôffnen der PCLIB32

#ifdef DEBUG
	if( BS_OpenPClib( "D:\\PRO\\SGOS32\\lib\\data\\") == -1 )
	{
		printf("Library open error\n");
		return(1);
	};
#else
	if( BS_OpenPClib( "Data\\") == -1 )
	{
		printf("Library open error\n");
		return(1);
	};

#endif






	// Auswerten der Kommandozeilenparameter
	if( (n_Error = BS_OpenFilterFiles( argc, argv, &ul_Command, zs_InputPath, &InputDirectory, zs_OutputPath, &OutputDirectory, &p_CommandStream, &p_ProtokollStream)) != 0)
		// Hier kînnte noch eine Auswertung des Fehlers erfolgen.
		return( n_Error );




	// Hier erfolgt eine Analyse des Kommandos und entsprechende Verzweigung
	switch( ul_Command )
	{
	case 1:
		// Identifikationslauf
		IdentifyYourself(  p_ProtokollStream );
		break;

	case 100:
	case 200:
	case 300:
		// Objekte zusammenbinden






		// Ggfs in Graphikmodus umschalten
		if( ul_Command >= 300 )
		{
			BS_ClosePClib();



#ifdef DEBUG
    		if( S_InitSGOS("D:\\pro\\sgos32\\lib\\data\\",640,480,SGOS_NORMAL ) == -1 )
			{
				printf("Library open error\n");
				return(1);
			};
#else
			if( S_InitSGOS("data\\",640,480,SGOS_NORMAL ) == -1 )
			{
				printf("Library open error\n");
				return(1);
			};
#endif
			S_HideM();
		};



		//  Backslash an Sourcepath anhÑngen
		if( zs_InputPath[ strlen(zs_InputPath) -1] != 92 );
			strcat( zs_InputPath, "\\" );


		// Hier wird das Steuerungsfile nach Spezifizierern abgesucht, die den Arbeitsmodus
		// und den Prozessortyp beschreiben. Werden keine gefunden, werden die Vorgaben aus
		// den Variableninitialisierungen verwendet: Entpacken / Intel-Format
		if( S_GotoFirst( p_CommandStream ) != 0 )
		{
			// Error
			P_WriteResult( p_ProtokollStream, -1, 4 );
			if( p_CommandStream )
				S_CloseStream( p_CommandStream );
			P_CloseStream( p_ProtokollStream );
			break; // switch verlassen
		};


		do{
			switch( S_GetSpecifierType( p_CommandStream ))
			{
			case MOTOROLA: ul_Processor = 1; break;
			case INTEL   : ul_Processor = 0; break;
			};
		}while( S_GotoNext( p_CommandStream ) == 0);


		if( ul_Command == 200 )
			printf("Making %&s Library\n", (ul_Processor==0)?("Intel"):("Motorola"));

		if( ul_Command >= 300 )
			if( ul_Processor == 0 )
				GS_Footer( "Making Intel Library", 1);
			else
				GS_Footer( "Making Motorola Library",1);



		BindObjects( zs_InputPath, &InputDirectory, zs_OutputPath, &OutputDirectory, p_ProtokollStream, ul_Command );
		break;

	default:
		// Schreiben der generellen RÅckmeldung
		if( P_WriteResult( p_ProtokollStream, 5, 0 ) != 0 )
		{
			P_CloseStream( p_ProtokollStream );
			return( -1 );
		};

		break;

	};


	if( ul_Command != 1 )
	{
		S_KillList( &InputDirectory );
		S_KillList( &OutputDirectory );
	};


	// Schlie·en der Streams

	P_CloseStream( p_ProtokollStream );
	if( p_CommandStream )
		S_CloseStream( p_CommandStream );


	if( ul_Command >= 300 )
		S_ExitSGOS();
	else
		BS_ClosePClib();

	return( 0 );
};


