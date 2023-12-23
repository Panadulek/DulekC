#pragma once
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <string>
#include "Scope.h"
#include "AstTree.h"
#include "Variable.h"
#include <exception>
#include <llvm/IR/GlobalVariable.h>

class LLVMGen
{
	static llvm::LLVMContext& getContext()
	{
		static llvm::LLVMContext s_context;
		return s_context;
	}
	llvm::Module m_module;
	llvm::IRBuilder<> m_builder;


	void genIRForVariable(Variable* v)
	{
		assert(v);
		auto& context = getContext();
		llvm::Type* type = v->getLLVMType(context);
		llvm::Value* value = v->getLLVMValue(type);
		llvm::Module& refToModule = m_module;
		if (v->isGlobalVariable())
		{
			llvm::Constant* _const = llvm::dyn_cast<llvm::Constant>(value);
			llvm::GlobalVariable* global = new llvm::GlobalVariable(refToModule, type, false, llvm::GlobalValue::ExternalLinkage, _const, v->getIdentifier().getName().data());
		}
		else
		{
			throw std::runtime_error("NOT_IMPLEMENTED");
		}
	}

	void genIRForElement(DuObject* obj)
	{
		if (obj->isVariable())
		{
			Variable* v = static_cast<Variable*>(obj);
			genIRForVariable(v);
		}
	}
	void genIRForScope(Scope* scope)
	{
		for (auto it = scope->begin(); it != scope->end(); it++)
		{
			genIRForElement(*it);
		}
	}
public:
	LLVMGen(const std::string& modulename) : m_module(modulename, getContext()), m_builder(getContext())
	{
	
	}


	void genIRForFile(const AstTree::Iterator begin, const AstTree::Iterator end)
	{
		for (auto it = begin; it != end; it++)
		{
			genIRForScope(*it);
		}
	}
	~LLVMGen()
	{
		m_module.print(llvm::outs(), nullptr);
	}

};