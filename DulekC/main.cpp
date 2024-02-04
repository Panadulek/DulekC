#define TEST1
#ifndef TEST
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




extern void initlex(void);
extern void initTerminalMessageEngine(void);

int main(int argc, char* argv[])
{
	errno_t code;
	if(argc > 1)
		code = fopen_s(&yyin, argv[1], "r");
	else
		code = fopen_s(&yyin, "Main.du", "r");
	

	initlex();
	initTerminalMessageEngine();
	yyparse();
	LLVMGen generator("test");
	generator.genIRForFile(AstTree::instance().begin(), AstTree::instance().end());
	generator.print();
	generator.executeCodeToByteCode();
	return 0;
}
#else


#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

using namespace llvm;

int main() {
	LLVMContext Context;
	Module* Mod = new Module("my_module", Context);
	IRBuilder<> Builder(Context);

	// Tworzenie funkcji "main_du_fun_entry" typu void ()
	FunctionType* FTy = FunctionType::get(Type::getVoidTy(Context), false);
	Function* MainFun = Function::Create(FTy, GlobalValue::InternalLinkage, "main_du_fun_entry", Mod);

	// Tworzenie bloków
	BasicBlock* EntryBB = BasicBlock::Create(Context, "entry", MainFun);
	BasicBlock* ThenBB = BasicBlock::Create(Context, "then", MainFun);
	BasicBlock* ElseBB = BasicBlock::Create(Context, "else", MainFun);
	BasicBlock* MergeBB = BasicBlock::Create(Context, "merge", MainFun);

	// Ustawienie punktu wejœcia do funkcji
	Builder.SetInsertPoint(EntryBB);

	// Tworzenie instrukcji w bloku "entry"
	Value* NumericValue = Builder.CreateAlloca(Type::getInt32Ty(Context));
	Builder.CreateStore(ConstantInt::get(Type::getInt32Ty(Context), 2), NumericValue);
	Value* NumericValue1 = Builder.CreateAlloca(Type::getInt32Ty(Context));
	Builder.CreateStore(ConstantInt::get(Type::getInt32Ty(Context), 3), NumericValue1);
	Value* Val = Builder.CreateAlloca(Type::getInt32Ty(Context));

	Value* Load0 = Builder.CreateLoad(Type::getInt32Ty(Context), NumericValue);
	Builder.CreateStore(Load0, Val);
	Value* Load1 = Builder.CreateLoad(Type::getInt32Ty(Context), NumericValue1);
	Value* Val2 = Builder.CreateAlloca(Type::getInt32Ty(Context));
	Builder.CreateStore(Load1, Val2);

	Value* Cmp = Builder.CreateICmpSLT(Load0, Load1);
	Builder.CreateCondBr(Cmp, ThenBB, ElseBB);

	// Tworzenie instrukcji w bloku "then"
	Builder.SetInsertPoint(ThenBB);
	Value* Add0 = Builder.CreateAdd(Load0, Load0);
	Value* Add1 = Builder.CreateAdd(Add0, Load0);
	Builder.CreateStore(Add1, Val);
	Builder.CreateBr(MergeBB);

	// Tworzenie instrukcji w bloku "else"
	Builder.SetInsertPoint(ElseBB);
	Builder.CreateStore(ConstantInt::get(Type::getInt32Ty(Context), 1), Val);
	Builder.CreateBr(MergeBB);

	// Tworzenie instrukcji w bloku "merge"
	Builder.SetInsertPoint(MergeBB);
	Value* LoadVal = Builder.CreateLoad(Type::getInt32Ty(Context), Val);
	Builder.CreateCall(Function::Create(FunctionType::get(Type::getVoidTy(Context), Type::getInt32Ty(Context), false), GlobalValue::ExternalLinkage, "dis", Mod), LoadVal);
	Builder.CreateRetVoid();

	// Wydrukowanie kodu LLVM IR
	Mod->print(outs(), nullptr);

	return 0;
}

#endif