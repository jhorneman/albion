
/* enumerations */

enum Parser_FSM_char_type {
	SPACE_FSM_CHAR_TYPE,
	EOL_FSM_CHAR_TYPE,
	COMMENT_FSM_CHAR_TYPE,
	ALPHA_FSM_CHAR_TYPE,
	MAX_FSM_CHAR_TYPES
};

enum Parser_FSM_state {
	SPACE_FSM_STATE,
	COMMENT_FSM_STATE,
	TOKEN_FSM_STATE,
	EOL_FSM_STATE,
	MAX_FSM_STATES
};

/* type definitions */

typedef enum Parser_FSM_char_type FSM_CHAR_TYPE_T;
typedef enum Parser_FSM_state FSM_STATE_T;

void
Find_token(void)
{
	FSM_STATE_T State;
	BOOLEAN End;
	UNCHAR Char;
	UNCHAR *Ptr;

	/* Get pointer to current position in script */
	Ptr = (UNCHAR *) (MEM_Claim_pointer(Script_handle) + Script_offset);

	/* Get current FSM state */
	State = Current_FSM_state;

	/* While the script hasn't ended */
	End = FALSE;
	while (Script_offset < Script_length)
	{
		/* Read character from script */
		Char = *Ptr;

		/* Analyse this character */
		State = Analyse_script_char(State, Char);

		/* Act depending on state */
		switch (State)
		{
			case SPACE_FSM_STATE:
			{
				break;
			}
			case COMMENT_FSM_STATE:
			{
				End = TRUE;
				break;
			}
			case TOKEN_FSM_STATE:
			{
				End = TRUE;
				break;
			}
			case EOL_FSM_STATE:
			{
				End = TRUE;
				break;
			}
		}

		if (!End)
			break;

		Script_offset++;
	}

	if ((State == COMMENT_FSM_STATE) || (State == EOL_FSM_STATE))
		return FALSE;

	if (State == TOKEN_FSM_STATE)
		return TRUE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Analyse_script_char
 * FUNCTION  : Analyse a script character using a finite state machine.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.08.95 16:28
 * LAST      : 01.08.95 16:28
 * INPUTS    : FSM_STATE_T Input_state - Input state.
 *             UNCHAR Char - Character that must be analysed.
 * RESULT    : FSM_STATE_T : Output state.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

FSM_STATE_T
Analyse_script_char(FSM_STATE_T Input_state, UNCHAR Char)
{
	static FSM_STATE_T FSM[MAX_FSM_CHAR_TYPES][MAX_FSM_STATES] =
	{
		{SPACE_FSM_STATE,		COMMENT_FSM_STATE,	SPACE_FSM_STATE,		EOL_FSM_STATE},
		{EOL_FSM_STATE,		EOL_FSM_STATE,			EOL_FSM_STATE,			EOL_FSM_STATE},
		{COMMENT_FSM_STATE,  COMMENT_FSM_STATE,	COMMENT_FSM_STATE,	EOL_FSM_STATE},
		{TOKEN_FSM_STATE,		COMMENT_FSM_STATE,	TOKEN_FSM_STATE,		EOL_FSM_STATE}
	};

	FSM_CHAR_TYPE_T FSM_char_type;

	/* Determine character type */
	switch (Char)
	{
		/* Space */
		case ' ':
		{
			FSM_char_type = SPACE_FSM_CHAR_TYPE;
			break;
		}
		/* Semicolon */
		case ';':
		{
			FSM_char_type = COMMENT_FSM_CHAR_TYPE;
			break;
		}
		/* EOL */
		case 0x0d:
		case 0x0a:
		{
			FSM_char_type = EOL_FSM_CHAR_TYPE;
			break;
		}
		/* Aplhanumeric character */
		default:
		{
			FSM_char_type = ALPHA_FSM_CHAR_TYPE;
			break;
		}
	}

	/* Return next FSM state */
	return FSM[FSM_char_type][Input_state];
}


