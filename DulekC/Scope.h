#pragma once
#include "DuObject.h"
#include "Variable.h"
#include <list>
class Scope : public DuObject
{
protected:
	std::list<DuPtr> m_childs;
public:
	using Iterator = decltype(m_childs)::iterator;
	Scope(Identifier id) : DuObject(id) 
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

	Iterator begin()
	{
		return m_childs.begin();
	}
	Iterator end()
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
public:
	Function(Identifier id, Type* returnType) : Scope(id), m_returnType(returnType), m_returnValue(nullptr)
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
	virtual bool isFunction() const { return true; }
};