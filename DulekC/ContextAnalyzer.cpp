#include "ContextAnalyzer.h"
#include "MessageEngine.h"
LexerContext ContextAnalyzer::m_context;
extern std::unique_ptr<MessageEngine> s_messageEngine(nullptr);

void initMessageEngine(void)
{
	if(!s_messageEngine)
		s_messageEngine = std::make_unique<TerminalMessageEngine>();
}