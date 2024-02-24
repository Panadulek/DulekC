#pragma once
#include <string>
#include <iostream>
#include <stack>
#include <cstdint>
#include <cassert>

class LexerContext
{
public:
	enum class Context : uint8_t
	{
		EMPTY = 0,
		GLOBAL,
		FUNCTION,
		IF,
		ELSE,
		WHILE,
		END_CONTEXT
	};
private:
	std::stack<Context> m_contextQueue;
	Context m_next;
	bool m_expectOpenBuckle;
public:
	LexerContext() : m_next(Context::EMPTY), m_expectOpenBuckle(false)
	{
		m_contextQueue.push(Context::GLOBAL);
	}
	bool isExpectedOpenBuckle()
	{
		return m_expectOpenBuckle;
	}

	void setNeedOpenBuckle(bool s)
	{
		m_expectOpenBuckle = s;
	}

	void setNextContext(Context c)
	{
		if(c != Context::EMPTY)
			m_next = c;
	}

	void pushContext()
	{
		assert(m_next != Context::GLOBAL);
		if (m_next != Context::EMPTY)
		{
			m_contextQueue.push(m_next);
			m_next = Context::EMPTY;
		}
	}

	void popContext()
	{
		Context c = m_contextQueue.top();
		assert(c != Context::GLOBAL);
		m_contextQueue.pop();
	}

	bool isInGlobalContext()
	{
		return m_contextQueue.top() == Context::GLOBAL;
	}

	Context getCurrentContext()
	{
		return m_contextQueue.top();
	}

	friend class ContextAnalyzer;
};