#include "MessageEngine.h"
static std::unique_ptr<MessageEngine> s_messageEngine(nullptr);
void Error(MessageEngine::Code code, std::string_view additionalMsg)
{
	s_messageEngine->printError(code, additionalMsg);
	exit(static_cast<uint32_t>(code));
}

void Warning(MessageEngine::Code code, std::string_view additionalMsg)
{
	s_messageEngine->printWarning(code, additionalMsg);
}

void Info(MessageEngine::Code code, std::string_view additionalMsg)
{
	s_messageEngine->printInfo(code, additionalMsg);
}

void initTerminalMessageEngine(void)
{
	if (!s_messageEngine)
		s_messageEngine = std::make_unique<TerminalMessageEngine>();
}