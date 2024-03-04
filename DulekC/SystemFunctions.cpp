#include "SystemFunctions.h"	
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include "DuFunctions.h"
void SystemFunctions::generatePrintNumberFunction()
{
	llvm::FunctionType* printFunctionType = llvm::FunctionType::get(m_builder->getInt32Ty(), m_builder->getInt32Ty(), false);
	auto functionPtr = llvm::Function::Create(printFunctionType, llvm::Function::LinkageTypes::ExternalLinkage, "DuDisplayNumber", m_module);
	auto printfFunc = llvm::FunctionCallee(functionPtr);
	m_functions.insert({ getSysFunctionName(SysFunctionID::DISPLAY), printfFunc });
}
void SystemFunctions::generateAllocateFunction()
{
	llvm::FunctionType* allocateFunctionType = llvm::FunctionType::get(m_builder->getInt8Ty()->getPointerTo(), m_builder->getInt64Ty(), false);
	auto functionPtr = llvm::Function::Create(allocateFunctionType, llvm::Function::LinkageTypes::ExternalLinkage, "DuAllocate", m_module);
	auto allocateFunc = llvm::FunctionCallee(functionPtr);
	m_functions.insert({ getSysFunctionName(SysFunctionID::ALLOCATE_MEMORY), allocateFunc });
}
void SystemFunctions::generateDeallocateFunction()
{
	llvm::FunctionType* allocateFunctionType = llvm::FunctionType::get(m_builder->getVoidTy(), m_builder->getInt8Ty()->getPointerTo(), false);
	auto functionPtr = llvm::Function::Create(allocateFunctionType, llvm::Function::LinkageTypes::ExternalLinkage, "DuAllocate", m_module);
	auto allocateFunc = llvm::FunctionCallee(functionPtr);
	m_functions.insert({ getSysFunctionName(SysFunctionID::DEALLOCATE_MEMORY), allocateFunc });
}

llvm::FunctionCallee* SystemFunctions::findFunction(Identifier id)
{
	auto it = m_functions.find(id.getName().data());
	if (m_functions.end() == it)
	{
		return nullptr;
	}
	return &it->second;
}