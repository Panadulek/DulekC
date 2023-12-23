#pragma once
#include "DuObject.h"

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
	virtual ~Scope() 
	{
		for (auto& it : m_childs)
		{
			delete it;
			it = nullptr;
		}
	
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
};
