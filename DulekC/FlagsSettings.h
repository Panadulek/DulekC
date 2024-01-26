#pragma once
#include <string>
#include <iostream>
#include <stack>
#include <cstdint>
/*
	TODO:
		Stworzyc klase zarzadzajaca lexerem.
		Stworzyc klase odpowiadajaca za errory.

*/
class ErrorManager
{

};

class ContextManager
{
public:
	enum class Context : uint8_t
	{
		EMPTY = 0,
		GLOBAL,
		FUNCTION,
		END_CONTEXT
	};
private:
	std::stack<Context> m_contextQueue;
	Context m_next;
public:
	ContextManager() : m_next(Context::EMPTY)
	{
		m_contextQueue.push(Context::GLOBAL);
	}

	void setNextContext(Context c)
	{
		m_next = c;
	}




};