#pragma once
#include <llvm/IR/IRBuilder.h>

class Scope;
class DuObject;
struct ISelfGeneratedScope
{
	virtual void generateLLVM(llvm::IRBuilder<>& b, llvm::Module* m, std::function<void(Scope*, DuObject*)> cb) = 0;
	virtual void initParentFun() = 0;
	virtual llvm::BasicBlock* getMergeBlock() = 0;
	virtual const bool HasBranchedRet() const = 0;
	virtual void callCallback(std::function<void(Scope*, DuObject*)> cb, Scope* scope) = 0;
};
