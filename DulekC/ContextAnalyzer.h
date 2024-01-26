#pragma once
#include "LexerContext.h"

class ContextAnalyzer
{
	static LexerContext m_context;
	using LCCONTEXT = LexerContext::Context;
public:
	ContextAnalyzer()
	{}

	bool InGlobalContext()
	{
		return m_context.m_contextQueue.top() == LCCONTEXT::GLOBAL;
	}

	LexerContext& getRefToContext()
	{
		return m_context;
	}

};
