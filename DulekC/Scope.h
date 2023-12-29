#pragma once
#include "DuObject.h"
#include "Variable.h"
#include <list>
class Scope : public DuObject
{
protected:
	std::list<DuPtr> m_childs;
	llvm::BasicBlock* m_llvmBlock;
	std::string m_blockEntryName;
public:
	using Iterator = decltype(m_childs)::iterator;
	Scope(Identifier id) : DuObject(id), m_llvmBlock(nullptr)
	{}
	void addChild(DuPtr child)
	{
		m_childs.push_back(child);
	}
	DuPtr findObject(Identifier id)
	{
		for (auto it : m_childs)
		{
			if (it->getIdentifier() == id)
				return it;
		}
		return nullptr;
	}
	std::list<DuPtr>& getList()
	{
		return m_childs;
	}

	auto begin()
	{
		return m_childs.begin();
	}
	
	auto begin() const
	{
		return m_childs.begin();
	}
	auto end()
	{
		return m_childs.end();
	}
	auto end() const
	{
		return m_childs.end();
	}

	
	virtual llvm::Type* getLLVMType(llvm::LLVMContext&) const override
	{
		assert(0);
		return nullptr;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		assert(0);
		return nullptr;
	}
	llvm::BasicBlock* getBasicBlock(llvm::LLVMContext& context, llvm::Function* fn)
	{
		if (!m_llvmBlock)
		{
			if (m_blockEntryName.empty())
			{
				m_blockEntryName = getIdentifier().getName().data();
				m_blockEntryName += "_du_fun_entry";
			}
			m_llvmBlock  = llvm::BasicBlock::Create(context, getEntryName(), fn);
		}
		return m_llvmBlock;
	}

	std::string_view getEntryName()
	{
		return m_blockEntryName;
	}

	virtual ~Scope()
	{
		for (auto& it : m_childs)
		{
			delete it;
			it = nullptr;
		}

	}
};


class Function : public Scope
{
protected:
	std::vector<Variable*> m_args;
	Type* m_returnType;
	Value* m_returnValue;
	llvm::FunctionType* m_llvmType;
	llvm::Function* m_llvmFunction;
	llvm::FunctionType* getFunctionType(llvm::LLVMContext& context)
	{
		if(!m_llvmType)
			m_llvmType = llvm::FunctionType::get(getLLVMType(context), false);
		return m_llvmType;
	}

public:
	Function(Identifier id, Type* returnType) : Scope(id), m_returnType(returnType), m_returnValue(nullptr), m_llvmType(nullptr), m_llvmFunction(nullptr)
	{}
	void addArgs(Variable* arg)
	{
		m_args.push_back(arg);
	}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		return m_returnType->getLLVMType(context);
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		if (m_returnValue->isNumericValue() && m_returnType->isSimpleNumericType())
		{
			static_cast<NumericValue*>(m_returnValue)->setSigned(static_cast<SimpleNumericType*>(m_returnType)->isSigned());
		}
		return m_returnValue->getLLVMValue(type);
	}
	llvm::Function* getLLVMFunction(llvm::LLVMContext& context, llvm::Module* m)
	{
		if(!m_llvmFunction)
			m_llvmFunction = llvm::Function::Create(getFunctionType(context), llvm::Function::ExternalLinkage, getIdentifier().getName(), m);
		return m_llvmFunction;
	}

	virtual bool isFunction() const { return true; }
};


/*
		
		m_builder.SetInsertPoint(block);
		auto& list = scope->getList();
		for (auto it : list)
		{
			if (it)
				llvm::AllocaInst* localVariable = m_builder.CreateAlloca(it->getLLVMType(getContext()), 0, it->getIdentifier().getName());
		}

*/