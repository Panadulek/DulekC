#include "FlagsSettings.h"
#include "parser.hpp" 
int __cdecl yylex();
static ContextManager s_sm;
static enum 
{
	IDX_BRACE = 0,
	IDX_BUCKLE,
	IDX_END
} BRACES;


static void calculateBraces(int token, int arr[IDX_END])
{
	switch (token)
	{
	case LBRACE:
		arr[IDX_BRACE]++;
		break;
	case RBRACE:
		arr[IDX_BRACE]--;
		break;
	case LBUCKLE:
		arr[IDX_BUCKLE]++;
		break;
	case RBUCKLE:
		arr[IDX_BUCKLE]--;
	default:
		break;
	}
}


static void analyzeBraces(int token, int arr[IDX_END])
{
	if (arr[IDX_BUCKLE] < 0)
	{}
	else if (arr[IDX_BUCKLE] > 0 && token == SEMICOLON)
	{}
	if (arr[IDX_BRACE] < 0)
	{}
	else if (arr[IDX_BRACE] > 0 && token == SEMICOLON)
	{}
}


static ContextManager::Context findNextContext(int token)
{



}



int __cdecl lex(void)
{
	int token = yylex();
	static int braces[IDX_END];
	ContextManager::Context nextContext = ContextManager::Context::EMPTY;
	for (auto& it : braces)
	{
		it = 0;
	}
	calculateBraces(token, braces);


	analyzeBraces(token, braces);
	return token;
}