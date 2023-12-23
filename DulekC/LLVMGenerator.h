#pragma once
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <string>
#include "Scope.h"
#include "AstTree.h"
#include "Variable.h"
#include <exception>
#include <llvm/Support/TargetSelect.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/TargetSelect.h>


extern void not_implemented_feature();

class LLVMGen
{
	static llvm::LLVMContext& getContext()
	{
		static llvm::LLVMContext s_context;
		return s_context;
	}
	std::unique_ptr<llvm::Module> m_module;
	llvm::IRBuilder<> m_builder;

	void generateLocalVariableIrInfo(Variable* v, Scope* scope)
	{
		llvm::FunctionType* fnType = llvm::FunctionType::get(scope->getLLVMType(getContext()), false);
		llvm::Function* fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, scope->getIdentifier().getName(), m_module.get());
		std::string blockEntryName = scope->getIdentifier().getName().data();
		blockEntryName += "_du_fun_entry";
		llvm::BasicBlock* block = llvm::BasicBlock::Create(getContext(), blockEntryName.c_str(), fn);
		auto savedIP = m_builder.saveIP();
		m_builder.SetInsertPoint(block);
		auto& list = scope->getList();
		for (auto it : list)
		{
			if (it)
				llvm::AllocaInst* localVariable = m_builder.CreateAlloca(it->getLLVMType(getContext()), 0, it->getIdentifier().getName());
		}
		
	}
	void genIRForVariable(Variable* v, Scope* scope)
	{
		assert(v);
		auto& context = getContext();
		llvm::Type* type = v->getLLVMType(context);
		llvm::Value* value = v->getLLVMValue(type);
		if (v->isGlobalVariable())
		{
			llvm::Constant* _const = llvm::dyn_cast<llvm::Constant>(value);
			llvm::GlobalVariable* global = new llvm::GlobalVariable(*m_module, type, false, llvm::GlobalValue::ExternalLinkage, _const, v->getIdentifier().getName().data());
		}
		else if(!v->isGlobalVariable() && scope->isFunction())
		{
			generateLocalVariableIrInfo(v, scope);
		}
	}

	void genIRForElement(DuObject* obj, Scope* scope)
	{
		if (obj->isVariable())
		{
			Variable* v = static_cast<Variable*>(obj);
			genIRForVariable(v, scope);
		}
	}
	void genIRForScope(Scope* scope)
	{
		for (auto it = scope->begin(); it != scope->end(); it++)
		{
			genIRForElement(*it, scope);
		}
	}
public:
	LLVMGen(const std::string& modulename) : m_builder(getContext())
	{
		m_module = std::make_unique<llvm::Module>(modulename, getContext());
		llvm::InitializeNativeTarget();
		llvm::InitializeNativeTargetAsmPrinter();
	}


	void genIRForFile(const AstTree::Iterator begin, const AstTree::Iterator end)
	{
		for (auto it = begin; it != end; it++)
		{
			genIRForScope(*it);
		}
	}

	void execute()
	{
		m_module->print(llvm::outs(), nullptr);
	}

	~LLVMGen()
	{
		
	}

};