#pragma once
#include "DuObject.h"
#include "Variable.h"
#include <list>
#include <span>
class Scope : public DuObject
{
protected:
	std::vector<DuPtr> m_childs;
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
	std::span<DuPtr> getList()
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

	virtual bool isScope() const override { return true; }
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
	virtual DuObject* copy() const override
	{
		assert(0);
		return nullptr;
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
	std::vector<Identifier> m_args;
	std::vector<Type*> m_typesArgs;
	Type* m_returnType;
	llvm::FunctionType* m_llvmType;
	llvm::Function* m_llvmFunction;
	bool m_isSystemFunction;
	bool m_isProcedure;
	llvm::FunctionType* getFunctionType(llvm::LLVMContext& context)
	{
		if(!m_llvmType)
			m_llvmType = llvm::FunctionType::get(getLLVMType(context), false);
		return m_llvmType;
	}

public:
	Function(Identifier id, Type* returnType, std::vector<Identifier>&& args, std::vector<Type*>&& types, bool systemFunction, bool isProcedure) : Scope(id), m_args(std::move(args)), m_typesArgs(std::move(types)), m_returnType(returnType), m_llvmType(nullptr), m_llvmFunction(nullptr), m_isSystemFunction(systemFunction), m_isProcedure(isProcedure)
	{}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		if (m_returnType)
			return m_returnType->getLLVMType(context);
		else
			llvm::Type::getVoidTy(context);
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		assert(0);
		return nullptr;
	}
	llvm::Function* getLLVMFunction(llvm::LLVMContext& context, llvm::Module* m)
	{
		if(!m_llvmFunction)
			m_llvmFunction = llvm::Function::Create(getFunctionType(context), llvm::Function::ExternalLinkage, getIdentifier().getName(), m);
		return m_llvmFunction;
	}
	Type* getType() const
	{
		return m_returnType;
	}
	void generateReturn(llvm::LLVMContext& context, llvm::Module* m)
	{
		
	}
	void setSystemFunction(bool flag = true)
	{
		m_isSystemFunction = flag;
	}
	const bool isSystemFunction() const { return m_isSystemFunction;  }
	const bool isProcedure() const { return m_isProcedure;  }
	virtual bool isFunction() const { return true; }
};

