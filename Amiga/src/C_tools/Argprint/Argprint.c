/****************************************************************************
 Argument printer
 Written by J.Horneman (In Tune With The Universe)
 Start : 22-3-1994
 Syntax : Argprint {arguments...}
****************************************************************************/

main(int argc,char *argv[])
{
	int i;
	
	for(i=1;i<argc;i++) printf("%s\n",argv[i]);
}
