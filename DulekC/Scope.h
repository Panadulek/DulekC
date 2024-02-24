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
	virtual void addChild(DuPtr child)
	{
		m_childs.push_back(child);
	}
	virtual DuPtr findObject(Identifier id)
	{
		for (auto it : m_childs)
		{
			if (it->getIdentifier() == id)
				return it;
		}
		return nullptr;
	}

	virtual DuPtr findObject(std::shared_ptr<KeyType> key)
	{
		for (auto it : m_childs)
		{
			if (it->getKey() == key)
				return it;
		}
		return nullptr;
	}
	DuObject* findUpperObject(std::shared_ptr<KeyType> key, bool orginal = false)
	{
		DuObject* parent = this;
		do
		{
			 parent = parent->getParent();
			 if (!parent || parent == s_GlobalScope)
				 return nullptr;
			 if (parent->isScope())
			 {
				 DuObject* ret = static_cast<Scope*>(parent)->findObject(key);
				 if (ret)
				 {
					 if (orginal)
					 {
						 if (!ret->isCopy())
							 return ret;
					 }
					 else
						return ret;
				 }
			 }

		} while (parent);
	}
	DuObject* findUpperObject(Identifier key, bool orginal = false)
	{
		DuObject* parent = this;
		do
		{
			parent = parent->getParent();
			if (!parent || parent == s_GlobalScope)
				return nullptr;
			if (parent->isScope())
			{
				DuObject* ret = static_cast<Scope*>(parent)->findObject(key);
				if (ret)
				{
					if (orginal)
					{
						if (!ret->isCopy())
							return ret;
					}
					else
						return ret;
				}
			}

		} while (parent);
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
	virtual llvm::BasicBlock* getBasicBlock(llvm::LLVMContext& context, llvm::Function* fn)
	{
		if (!m_llvmBlock)
		{
			m_llvmBlock  = llvm::BasicBlock::Create(context, "global", fn);
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

	bool replace(DuObject* obj)
	{
		for (auto& it : m_childs)
		{
			if (it->getIdentifier() == obj->getObject()->getIdentifier())
			{
				it = obj->getObject();
				return true;
			}
		}
		return false;
	}

	virtual ~Scope()
	{
		for (auto& it : m_childs)
		{
			auto toDelete = it;
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
		if (!m_llvmType)
		{
			if(m_args.empty())
				m_llvmType = llvm::FunctionType::get(getLLVMType(context), false);
			else
			{
				std::vector<llvm::Type*> types;
				for (auto it : m_typesArgs)
				{
					types.push_back(it->getLLVMType(context));
				}
				m_llvmType = llvm::FunctionType::get(getLLVMType(context), types, false);
			}
		}
		return m_llvmType;
	}

public:
	Function(Identifier id, Type* returnType, std::vector<Identifier>&& args, std::vector<Type*>&& types, bool systemFunction, bool isProcedure) : Scope(id), m_args(std::move(args)), m_typesArgs(std::move(types)), m_returnType(returnType), m_llvmType(nullptr), m_llvmFunction(nullptr), m_isSystemFunction(systemFunction), m_isProcedure(isProcedure)
	{
		if (m_args.size() == m_typesArgs.size())
		{
			for (size_t i = 0; i < m_args.size(); i++)
			{
				Variable* v = new Variable(m_args[i], m_typesArgs[i], m_typesArgs[i]->getDefaultValue(), false);
				addChild(v);
			}
		}
		else
		{
			printf("Nie rowna ilosc typow i argumentow");
			exit(20);
		}
	}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		if (m_returnType)
			return m_returnType->getLLVMType(context);
		else
			return llvm::Type::getVoidTy(context);
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		assert(0);
		return nullptr;
	}
	llvm::Function* getLLVMFunction(llvm::LLVMContext& context, llvm::Module* m, llvm::IRBuilder<>& b);
	Type* getType() const
	{
		return m_returnType;
	}
	void setSystemFunction(bool flag = true)
	{
		m_isSystemFunction = flag;
	}
	const bool isSystemFunction() const { return m_isSystemFunction;  }
	const bool isProcedure() const { return m_isProcedure;  }
	virtual bool isFunction() const { return true; }
	virtual llvm::BasicBlock* getBasicBlock(llvm::LLVMContext& context, llvm::Function* fn) override
	{
		if (!m_llvmBlock)
		{
			if (m_blockEntryName.empty())
			{
				m_blockEntryName = getIdentifier().getName().data();
				m_blockEntryName += "_du_fun_entry";
			}
			m_llvmBlock = llvm::BasicBlock::Create(context, getEntryName(), fn);
		}
		return m_llvmBlock;
	}
};

