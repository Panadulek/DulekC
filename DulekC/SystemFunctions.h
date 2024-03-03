#pragma once
#include <cstdint>
#include <llvm/IR/IRBuilder.h>
#include <map>
#include "DuObject.h"
class SystemFunctions final
{
	llvm::Module* m_module;
	llvm::IRBuilder<>* m_builder;
	llvm::LLVMContext* m_context;
	std::map<std::string, llvm::FunctionCallee> m_functions;
	void generatePrintNumberFunction();
	void generateAllocateFunction();
	void generateDeallocateFunction();
	SystemFunctions(llvm::Module* m, llvm::IRBuilder<>* b, llvm::LLVMContext* c) : m_module(m), m_builder(b), m_context(c)
	{
		generatePrintNumberFunction();
	}
public:
	static SystemFunctions* GetSystemFunctions(llvm::Module* m, llvm::IRBuilder<>* b, llvm::LLVMContext* c)
	{
		static SystemFunctions sf(m, b, c);
		return &sf;
	}
	enum class SysFunctionID
	{
		DISPLAY = 0,
		ALLOCATE_MEMORY,
		DEALLOCATE_MEMORY,
		LAST
	};
	template<SysFunctionID ID>
	static std::string getSysFunctionName()
	{
		switch (ID)
		{
		case SysFunctionID::DISPLAY:
			return "$display";
		case SysFunctionID::ALLOCATE_MEMORY:
			return "$allocate";
		case SysFunctionID::DEALLOCATE_MEMORY:
			return "$deallocate";
		case SysFunctionID::LAST:
		default:
			return std::string();
		}
	}
	llvm::FunctionCallee* findFunction(Identifier id);
};
