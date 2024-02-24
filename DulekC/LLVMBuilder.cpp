#include "LLvmBuilder.h"
#include "AstTree.h"
#include <format>
Variable* LlvmBuilder::assigmentValue(llvm::IRBuilder<>& b, Variable* l, llvm::Value* r)
{
	llvm::AllocaInst* inst = l->getAlloca();
	if (!inst)
	{
		l->init(b.CreateAlloca(l->getLLVMType(b.getContext()), nullptr, ""), b);
	}
	inst = l->getAlloca();
	b.CreateStore(r, inst);
	return l;
}


llvm::Value* LlvmBuilder::loadValue(llvm::IRBuilder<>& b, Variable* var)
{
	llvm::AllocaInst* memory = var->getAlloca();
	if (!memory)
	{
		auto fmt = std::format("\'{}'\@", var->getIdentifier().getName()); 
		var->init(b.CreateAlloca(var->getLLVMType(b.getContext()), 0, fmt), b);
	}
	memory = var->getAlloca();
	return b.CreateLoad(var->getLLVMType(b.getContext()), memory);

}

llvm::PHINode* LlvmBuilder::initPhiNode(llvm::IRBuilder<>& b, Variable* var)
{
	llvm::Value* val = loadValue(b, var);
	llvm::PHINode* ret = b.CreatePHI(val->getType(), 0, var->getIdentifier().getName());
	ret->addIncoming(val, static_cast<Scope*>(var->getParent())->getBasicBlock(b.getContext(), nullptr));
	return ret;
}