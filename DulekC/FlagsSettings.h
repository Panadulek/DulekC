#pragma once
#include <string>
#include <iostream>
class CompilerCommunicate
{
	static const std::string  FunctionInFunction()
	{
		return "Not allowed to declare Function in Function";
	}
	CompilerCommunicate() {}
	friend class CompilerParserFlags;
};

class CompilerParserFlags
{

	bool m_inFunction;
	bool m_inStruct;
	bool m_inGlobal;


	void err(const std::string& str)
	{
		std::cout << str << std::endl;
	}
public:
	static CompilerParserFlags& instance() 
	{
		static CompilerParserFlags _cpf;
		return _cpf;
	}

	CompilerParserFlags() : m_inFunction(false), m_inStruct(false), m_inGlobal(false)
	{}

	bool enterFunction()
	{
		if (m_inFunction)
		{
			err(CompilerCommunicate::FunctionInFunction());
			return false;
		}

		m_inFunction = true;
		m_inGlobal = false;
		return true;
	}

	void exitFunction()
	{
		m_inFunction = false;
		m_inGlobal = true;
	}

	bool isInFunction()
	{
		return (m_inFunction && !m_inGlobal);
	}

};