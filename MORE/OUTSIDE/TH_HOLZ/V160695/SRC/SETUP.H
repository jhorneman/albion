//Die Watcom includes laden:
#include <math.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <graph.h>
#include <dos.h>
#include <malloc.h>
#include <direct.h>
#include <ctype.h>

//Die BlueByte-includes laden:
#include "lib\bbdef.h"		//Die Typendeklarationen
#include "lib\bbopm.h"     //???
#include "lib\bbaes.h"     //???
#include "lib\bbdsa.h"     //???
#include "lib\bblbm.h"		//Routinen fÅr die .LBM-Files

//Die Setup-spezifischen includes laden:
#include "defines.h"		//Dinge, die durch Defines variabel gehalten werden
#include "token.h"      //Die defines der Tokens
#include "error.h"		//Die Kennungen fÅr die Fehlermeldungen
#include "types.h"		//eigene Typen und Strukturen
#include "proto.h"		//Alle Vorabdeklarationen
#include "global.h"		//Die "extern"-Versionen der globalen Variablen
