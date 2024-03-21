#include "LLvmBuilder.h"
#include "AstTree.h"
#include <format>

Variable* LlvmBuilder::assigmentValue(llvm::IRBuilder<>& b, Variable* l, llvm::Value* r)
{
	llvm::AllocaInst* inst = l->getAlloca();
	if (!inst)
	{
		l->init(b.CreateAlloca(l->getLLVMType(b.getContext()), nullptr, l->getIdentifier().getName()), b);
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


llvm::Value* LlvmBuilder::allocate(llvm::IRBuilder<>& b, llvm::Value* sizeofElement, llvm::Value* counts, llvm::FunctionCallee* allocateFunc)
{
	assert(sizeofElement->getType()->isIntegerTy() && counts->getType()->isIntegerTy());
	sizeofElement = b.CreateIntCast(sizeofElement, llvm::Type::getInt64Ty(b.getContext()), false);
	counts = b.CreateIntCast(counts, llvm::Type::getInt64Ty(b.getContext()), false);
	llvm::Value* size = b.CreateMul(counts, sizeofElement, "CalculateSizeToAllocate");
	std::vector<llvm::Value*> args(1);
	args[0] = size;
	llvm::Value* memory = b.CreateCall(*allocateFunc, args);
	return memory;
}


llvm::Value* LlvmBuilder::deallocate(llvm::IRBuilder<>& b, llvm::Value* ptr, llvm::FunctionCallee* deallocateFunc)
{
	assert(ptr && ptr->getType()->isPointerTy());
	llvm::Type* ptrType = ptr->getType();
	//ptr = b.CreateBitCast(ptr, llvm::IntegerType::getInt8Ty(b.getContext())->getPointerTo(), "cast to i8*");
	std::vector<llvm::Value*> args(1);
	args[0] = ptr;
	b.CreateCall(*deallocateFunc, args);
	return llvm::Constant::getNullValue(ptrType);

}