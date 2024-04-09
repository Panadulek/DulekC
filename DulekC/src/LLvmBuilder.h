#pragma once
#include "Variable.h"
#include <llvm/IR/IRBuilder.h>
#include "Type.h"
class LlvmBuilder
{
public:
	static Variable* assigmentValue(llvm::IRBuilder<>& b, Variable* l, llvm::Value* r);
	static llvm::Value* loadValue(llvm::IRBuilder<>& b, Variable* var);
	static llvm::Value* allocate(llvm::IRBuilder<>& b, llvm::Value* sizeofElement, llvm::Value* counts, llvm::FunctionCallee*);
	static llvm::Value* deallocate(llvm::IRBuilder<>& b, llvm::Value* Pointer, llvm::FunctionCallee* deallocateFunc);
};
