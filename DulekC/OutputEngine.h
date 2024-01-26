#pragma once
#include <cstdio>
#include <cstdint>
#include <string>
enum class OUTPUT_MODE
{
	STD_OUT,
	FILE,
};
static constexpr const char outputfilename[] = "dulekc_error.err";

template<OUTPUT_MODE mode>
class StandardOutput
{
	std::FILE* m_output;
	std::string m_filename;
	StandardOutput() : m_output(nullptr), m_filename(outputfilename)
	{
		changeToStandardOutput();
		openFile(m_filename.c_str());
	}
	void changeToErrorOutput()
	{
		if (mode == OUTPUT_MODE::STD_OUT)
			m_output = stderr;
	}
	void changeToStandardOutput()
	{
		if (mode == OUTPUT_MODE::STD_OUT)
			m_output = stdout;
	}
	void openFile(const char* filename)
	{
		if (mode == OUTPUT_MODE::FILE && !m_output)
		{
			m_filename = filename;
			m_output = fopen(filename, "w+");
		}
	}
	void printInBytes(const void* msg, size_t size)
	{
		fwrite(msg, sizeof(uint8_t), size, m_output);
	}
	~StandardOutput()
	{
		if (mode == OUTPUT_MODE::FILE)
		{
			fclose(m_output);
			m_output = nullptr;
		}
	}
	friend class MessageEngine;
	friend class TerminalMessageEngine;
};