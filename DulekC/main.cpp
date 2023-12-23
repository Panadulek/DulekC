
#include <cstdlib>
#include <cstdio>
#include "LLVMGenerator.h"
#include "Variable.h"
int yyparse(void);
extern FILE* yyin;
int main() 
{
	auto code = fopen_s(&yyin, "Main.du", "r");
	yyparse();
	LLVMGen generator("test");
	generator.genIRForFile(AstTree::instance().begin(), AstTree::instance().end());
	return 0;
}
