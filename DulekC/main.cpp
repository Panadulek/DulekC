#include <cstdlib>
#include <cstdio>
#include "LLVMGenerator.h"
#include "Variable.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include "MessageEngine.h"
#include <memory>
int yyparse(void);
extern FILE* yyin;
#define NOT_IMPLEMENTED_FEATURE_
void not_implemented_feature()
{
#ifdef NOT_IMPLEMENTED_FEATURE
	throw std::runtime_error("NOT_IMPLEMENTED");
#endif
}

std::unique_ptr<MessageEngine> s_messageEngine;


extern void initlex(void);


int main(int argc, char* argv[])
{
	errno_t code;
	if(argc > 1)
		code = fopen_s(&yyin, argv[1], "r");
	else
		code = fopen_s(&yyin, "Main.du", "r");
	
	s_messageEngine = std::make_unique<TerminalMessageEngine>();
	initlex();
	yyparse();
	LLVMGen generator("test");
	generator.genIRForFile(AstTree::instance().begin(), AstTree::instance().end());
	//generator.print();
	generator.executeCodeToByteCode();
	return 0;
}
