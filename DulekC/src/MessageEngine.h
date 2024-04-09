#pragma once
#include "OutputEngine.h"
#include <format>



class MessageEngine
{

public:
	enum class MessageType : uint8_t
	{
		INFO = 0,
		WARNING,
		ERROR,
	};


	enum class Code : uint32_t
	{
		CREATED_FUNCTION_IN_SCOPE,
		ERROR_TOKEN,
		BRACE_COUNTER,
		FunctionInsideScope,
		ExecuteGlobalExpression,
		NeedToOpenScope,
		ElseNeedsIfContext,
		CannotConvertToBoolean,
		INVALID_NUMBER_OF_ARGUMENTS,
		INVALID_ARGUMENT_TYPE,
	};
private:
	std::string getErrorMessage(Code code)
	{
		switch (code)
		{
		case Code::CREATED_FUNCTION_IN_SCOPE:
			return "Function was created in scope";
		case Code::ERROR_TOKEN:
		case Code::BRACE_COUNTER:
			return "Syntax error";
		case Code::FunctionInsideScope:
			return "Cannot create function in scoped";
		case Code::ExecuteGlobalExpression:
			return "Cannot execute expression in global context";
		case Code::NeedToOpenScope:
			return "Scope needs to be open with token \"{\"";
		case Code::ElseNeedsIfContext:
			return "Else statement needs if statement before";
		case Code::CannotConvertToBoolean:
			return "Variable cannot been convert to boolean type";
		case Code::INVALID_NUMBER_OF_ARGUMENTS:
			return "Invalid number of arguments";
		case Code::INVALID_ARGUMENT_TYPE:
			return "Invalid argument type";
		default:
			return "Not implemented message";
		}
	}
	std::string getTypeString(MessageType mt)
	{
		switch (mt)
		{
		case MessageType::INFO:
			return "Info";
		case MessageType::WARNING:
			return "Warning";
		case MessageType::ERROR:
			return "Error";
		default: return "";
		}
	}
protected:
	std::string getStandardMessage(MessageType mt, Code c )
	{
		return std::format("{} code: {}\t\"{}\"", getTypeString(mt), static_cast<uint32_t>(c), getErrorMessage(c));
	}
	void getStandardMessageWithAdditionalInfo(std::string& format, const char* additionalMsg)
	{
		if (!additionalMsg)
			return;
		format = std::format("{}\t\"{}\"", format, additionalMsg);
	}
public:
	virtual void printError(Code c, const char* additionalMsg) = 0;
	virtual void printWarning(Code c, const char* additionalMsg) = 0;
	virtual void printInfo(Code c, const char* additionalMsg) = 0;
	virtual void printNewLine() = 0;
};

class TerminalMessageEngine final : public MessageEngine
{
	StandardOutput<OUTPUT_MODE::STD_OUT> m_output;
public:
	virtual void printError(Code c, const char* additionalMsg)  override
	{
		std::string message = getStandardMessage(MessageType::ERROR, c);
		getStandardMessageWithAdditionalInfo(message, additionalMsg);
		m_output.changeToErrorOutput();
		m_output.printInBytes(message.c_str(), message.size() + 1);
		printNewLine();
	}

	virtual void printWarning(Code c, const char* additionalMsg) override
	{
		std::string message = getStandardMessage(MessageType::WARNING, c);
		getStandardMessageWithAdditionalInfo(message, additionalMsg);
		m_output.changeToErrorOutput();
		m_output.printInBytes(message.c_str(), message.size() + 1);
		printNewLine();
	}

	virtual void printInfo(Code c, const char* additionalMsg) override
	{
		std::string message = getStandardMessage(MessageType::INFO, c);
		getStandardMessageWithAdditionalInfo(message, additionalMsg);
		m_output.changeToStandardOutput();
		m_output.printInBytes(message.c_str(), message.size() + 1);
		printNewLine();
	}

	virtual void printNewLine() override
	{
		const char newLine[] = "\n\0";
		m_output.printInBytes("\n\0", sizeof(newLine)/sizeof(newLine[0]));
	}
};


