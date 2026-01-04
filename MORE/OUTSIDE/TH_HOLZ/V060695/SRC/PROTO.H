//------------------------------------------------------------------------------
//Proto.h - gesammelte Vorabdeklarationen fÅr's setup-programm:
//------------------------------------------------------------------------------

//Debug.c:
void	InternalError (SISHORT ErrNum, ...);
void	DoDebugBreak (void);
char *THMalloc (size_t Size);

//grafix.c:
void Rectangle3d (struct OPM *Opm, SILONG x1, SILONG y2, SILONG x2, SILONG y1, SILONG c1, SILONG c2, SILONG c3);

//io.c
void ChangeDirAndDriveTo (char *String);
void CopyOneFile (char *Source, char *Target);
BOOL IfExists (char *Filename);
void CreateFile (char *Filename);
void ExecuteShell (SILONG AnzParameters, char **Parameter);
void MakeDirs (char *DirString);

//Setup.c:

//Script.c:
void 	 SKRIPT_Init (SCRIPT *Script, char *ScriptFilename);
SILONG SCRIPT_GetTokenNumber (SCRIPT *Script, SILONG Linenumber, TOKEN *TokenTable);
SILONG SCRIPT_FindLabelLine (SCRIPT *Script, char *Label);
SILONG SCRIPT_ProcessScriptLine (SCRIPT *Script, SILONG Line, SILONG *AnzOpenIf);
char **SCRIPT_ParseScriptLine (SCRIPT *Script, SILONG Line, SILONG *AnzParameter);
SILONG SCRIPT_GetIntParameter (char *Parameter, SILONG Line);
SILONG SCRIPT_ScanForIfComplement (SCRIPT *Script, SILONG Line, SILONG AllowedToken);
