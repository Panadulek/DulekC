#include "LexerContext.h"
#include "ContextAnalyzer.h"
#include "parser.hpp" 
#include "MessageEngine.h"
#include <memory>
int __cdecl yylex();



LexerContext* s_lc = nullptr;




extern std::unique_ptr<MessageEngine> s_messageEngine;
extern char* yytext;
static enum 
{
	IDX_BRACE = 0,
	IDX_BUCKLE,
	IDX_END
} BRACES;

using CMContext = LexerContext::Context;
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
		break;
	default:
		break;
	}
}


static void analyzeBraces(int token, int arr[IDX_END])
{
	if ( ( arr[IDX_BUCKLE] < 0 ) ||  ( arr[IDX_BRACE] < 0 )  )
	{
		s_messageEngine->printError(MessageEngine::Code::BRACE_COUNTER, yytext);
	}
}


static CMContext findNextContext(int token)
{
	if (token == FUNCTION_KEYWORD)
		return CMContext::FUNCTION;
	else if (token == IF_KEYWORD)
		 return CMContext::IF;
	return CMContext::EMPTY;
}

static void changeActualState(int token, CMContext context)
{
	
	if (token == LBUCKLE)
	{
		s_lc->pushContext();
	}
	else if (token == RBUCKLE)
	{
		AstTree::instance().endScope();
		s_lc->popContext();
	}
	else
	{
		s_lc->setNextContext(context);
	}
}

void initlex(void)
{
	static bool isInited = false;
	if (!isInited)
	{
		ContextAnalyzer ca;
		s_lc = &ca.getRefToContext();
		isInited = true;
	}
}

int* getBraces()
{
	static int braces[IDX_END];
	static bool firstCall = true;
	if (firstCall)
	{
		for (unsigned i = 0; i < IDX_END; i++)
		{
			braces[i] = 0;
		}
		firstCall = false;
	}
	return braces;
}

int __cdecl lex(void)
{
	int token = yylex();

	if (s_lc->isExpectedOpenBuckle())
	{
		if (token != LBUCKLE)
		{
			s_messageEngine->printError(MessageEngine::Code::NeedToOpenScope, nullptr);
			exit(static_cast<uint8_t>(MessageEngine::Code::NeedToOpenScope));
		}
		else
			s_lc->setNeedOpenBuckle(false);
	}
	static int* braces = getBraces();
	LexerContext::Context nextContext = LexerContext::Context::EMPTY;
	calculateBraces(token, braces);
	analyzeBraces(token, braces);


	nextContext = findNextContext(token);
	changeActualState(token, nextContext);


	return token;
}