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
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include "SystemFunctions.h"
#define NO_CLEAR_MEMORY
extern void not_implemented_feature();



class LLVMGen final
{

	std::unique_ptr<llvm::Module> m_module;
	llvm::IRBuilder<> m_builder;
	llvm::LLVMContext& getContext()
	{
		static llvm::LLVMContext s_context;
		return s_context;
	}
	void generateMemoryForFunction(Scope* scope)
	{
		if (!scope->isFunction() || static_cast<Function*>(scope)->isSystemFunction())
			return;
		Function* fn = static_cast<Function*>(scope);
		llvm::Function* llvmFn = fn->getLLVMFunction(getContext(), m_module.get(), m_builder);
	}

	void generateDefaultReturnForProcedure(Scope* scope)
	{
		if (!scope->isFunction() || !static_cast<Function*>(scope)->isProcedure())
			return;
		m_builder.CreateRetVoid();
	}

	void generateLocalVariableIrInfo(Variable* v, Scope* scope)
	{
		if (scope->isFunction())
		{
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
		if (s->isCallFunctionStatement())
		{
			CallFunction* cfs = static_cast<CallFunction*>(s);
			if (cfs->isCallFunctionStatement())
			{
					cfs->processStatement(m_builder, getContext(), m_module.get());
			}
			else
			{
				assert(0);
			}
		}
		else
			s->processStatement(m_builder, getContext(), m_module.get());
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
		const bool isSettedScope = AstTree::instance().setCurrentScope(scope);
		generateMemoryForFunction(scope);
		std::for_each(scope->begin(), scope->end(), [&](DuObject* it) ->void
		{
			genIRForElement(it, scope);
		});
		if(isSettedScope)
			AstTree::instance().endScope();
		generateDefaultReturnForProcedure(scope);
		
	}
	void genfile()
	{
		std::error_code EC;
		llvm::raw_fd_ostream OS("output.ll", EC, llvm::sys::fs::OF_None);

		if (EC) {
			std::cerr << "B³¹d podczas otwierania pliku 'output.ll': " << EC.message() << std::endl;
			return;
		}

		m_module->print(OS, nullptr);
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
	void executeCodeToByteCode()
	{
		genfile();
		std::string ErrStr;

		llvm::Function* F = m_module->getFunction("main");
		if (!F)
		{
			llvm::errs() << "cannot find fun.\n";
		}

		std::unique_ptr<llvm::ExecutionEngine> EE(
			llvm::EngineBuilder(std::move(m_module))
			.setErrorStr(&ErrStr)
			.create());

		if (!EE)
		{
			llvm::errs() << "error ExecutionEngine: " << ErrStr << "\n";
		}
		
		std::vector<llvm::GenericValue> Args;
		llvm::GenericValue gv = EE->runFunction(F, Args);
	}
	void print()
	{
		m_module->print(llvm::outs(), nullptr);
	}


	~LLVMGen()
	{
	}

};