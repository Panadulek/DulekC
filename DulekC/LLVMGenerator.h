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
#include <algorithm>
#include <ranges>
#include "Statement.h"

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
		if (scope->isFunction())
		{
			Function* fn = static_cast<Function*>(scope);
			llvm::Function* llvmFn = fn->getLLVMFunction(getContext(), m_module.get());
			llvm::BasicBlock* bb = fn->getBasicBlock(getContext(), llvmFn);
			m_builder.SetInsertPoint(bb);
			auto& list = fn->getList();
			v->init(m_builder.CreateAlloca(v->getLLVMType(getContext()), nullptr , v->getIdentifier().getName()), m_builder);
			
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
	void genIRForStatement(Statement* s, Scope* scope)
	{
		s->processStatement(m_builder, getContext());
	}
	void genIRForElement(DuObject* obj, Scope* scope)
	{
		if (obj->isVariable())
		{
			Variable* v = static_cast<Variable*>(obj);
			genIRForVariable(v, scope);
		}
		else if (obj->isStatement())
		{
			Statement* s = static_cast<Statement*>(obj);
			genIRForStatement(s, scope);
		}
	}
	void genIRForScope(Scope* scope)
	{
		std::for_each(scope->begin(), scope->end(), [&](DuObject* it) ->void
		{
			genIRForElement(it, scope);
		});
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