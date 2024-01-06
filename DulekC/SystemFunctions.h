#pragma once




class SystemFunctions final
{
	llvm::Module* m_module;
	llvm::IRBuilder<>* m_builder;
	llvm::LLVMContext* m_context;
	std::map<std::string, llvm::FunctionCallee> m_functions;
	void generatePrintNumberFunction()
	{
		llvm::FunctionType* printFunctionType = llvm::FunctionType::get(m_builder->getInt32Ty(), m_builder->getInt8Ty()->getPointerTo(), true);
		llvm::FunctionCallee printfFunc = m_module->getOrInsertFunction("printf", printFunctionType);
		m_functions.insert({ getSysFunctionName<SysFunctionID::DISPLAY>(), printfFunc });
	}



public:
	SystemFunctions(llvm::Module* m, llvm::IRBuilder<>* b, llvm::LLVMContext* c) : m_module(m), m_builder(b), m_context(c)
	{
		generatePrintNumberFunction();
	}
	enum class SysFunctionID
	{
		DISPLAY = 0,
		LAST
	};
	template<SysFunctionID ID>
	static std::string getSysFunctionName()
	{
		switch (ID)
		{
		case SysFunctionID::DISPLAY:
			return "$display";
		case SysFunctionID::LAST:
		default:
			return std::string();
		}
	}
	llvm::FunctionCallee* findFunction(Identifier id)
	{
		auto it = m_functions.find(id.getName().data());
		if (m_functions.end() == it)
		{
			return nullptr;
		}
		return &it->second;
	}
};
