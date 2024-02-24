#pragma once
#include "Variable.h"
#include <llvm/IR/IRBuilder.h>
class LlvmBuilder
{
public:
	static Variable* assigmentValue(llvm::IRBuilder<>& b, Variable* l, llvm::Value* r);
	static llvm::Value* loadValue(llvm::IRBuilder<>& b, Variable* var);
	static llvm::PHINode* initPhiNode(llvm::IRBuilder<>& b, Variable* var);
};
