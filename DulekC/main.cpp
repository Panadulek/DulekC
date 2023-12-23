
#include <cstdlib>
#include <cstdio>
#include "LLVMGenerator.h"
#include "Variable.h"
int yyparse(void);
extern FILE* yyin;
#define NOT_IMPLEMENTED_FEATURE_
void not_implemented_feature()
{
#ifdef NOT_IMPLEMENTED_FEATURE
	throw std::runtime_error("NOT_IMPLEMENTED");
#endif
}

int main() 
{
	auto code = fopen_s(&yyin, "Main.du", "r");
	yyparse();
	LLVMGen generator("test");
	generator.genIRForFile(AstTree::instance().begin(), AstTree::instance().end());
	generator.execute();
	return 0;
}
