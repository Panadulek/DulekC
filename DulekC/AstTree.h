#pragma once
#include "DuObject.h"
#include "Scope.h"
#include <map>
#include <llvm/IR/Module.h>
#include <stack>
#include <cassert>
#include <iostream>
class AstTree
{
	std::unique_ptr<Scope> m_root;
	std::vector<Scope*> m_scopes;
	std::stack<Scope*> m_stack;
	AstTree()
	{
		m_root = std::make_unique<Scope>(Identifier("GLOBAL_SCOPE"));
		m_stack.push(m_root.get());
		m_scopes.push_back(m_root.get());
	}
public:
	using Iterator = decltype(m_scopes)::iterator;
	static AstTree& instance()
	{
		static AstTree s_tree;
		return s_tree;
	}
	void addObject(DuObject* obj)
	{
		assert(!m_stack.empty());
		auto it = std::find_if(m_stack.top()->begin(), m_stack.top()->end(), [&obj](const DuObject* _obj)
			{
				return _obj->getIdentifier() == obj->getIdentifier();
			}
		);
		if (it != m_stack.top()->end())
		{
			assert(0);
		}
		if (m_stack.top() != m_root.get())
		{
			auto it = std::find_if(m_root->begin(), m_root->end(), [&obj](const DuObject* _obj)
				{
					return _obj->getIdentifier() == obj->getIdentifier();
				}
			);
			if (it != m_root->end())
			{
				assert(0);
			}
		}
		m_stack.top()->addChild(obj);
	}

	void beginScope(Scope* scope)
	{
		m_scopes.push_back(scope);
		m_stack.push(m_scopes.back());
	}
	void endScope()
	{
		assert(m_stack.top() != m_root.get());
		m_stack.pop();
	}
	Iterator begin()
	{
		return m_scopes.begin();
	}
	Iterator end()
	{
		return m_scopes.end();
	}

	bool checkVisibility(DuObject* obj1, DuObject* obj2)
	{
		auto begin = m_stack.top()->begin();
		auto end = m_stack.top()->end();
		for (auto it = begin; it != end; it++)
		{
			if (*it == obj1)
			{
				begin = it;
				break;
			}
		}
		bool found = false;
		for (auto it = begin; it != end; it++)
		{
			if (*it == obj2)
			{
				found = true;
				break;
			}
		}
		for (auto it = m_root->begin(); it != m_root->end(); it++)
		{
			if (found)
			{
				if ((*it)->getIdentifier() == obj2->getIdentifier())
				{
					std::cout << "VARIABLE REPEAT" << std::endl;
					return false;
				}
			}
			else
			{
				if (*it == obj2)
				{
					found = true;
				}
			}
		}
		if (!found)
			std::cout << "VARIABLE DOESNT EXIST" << std::endl;
		return found;
	}
	DuObject* findObject(Identifier id, bool global)
	{
		DuObject* ret = nullptr;
		if (global)
		{
			auto it = std::find_if(m_root->begin(), m_root->end(), [&id](const DuObject* obj)
				{
					return id == obj->getIdentifier();
				}
			);
			if (it != m_root->end())
				ret = *it;
		}
		else
		{
			auto it = std::find_if(m_stack.top()->begin(), m_stack.top()->end(), [&id](const DuObject* obj)
				{
					return id == obj->getIdentifier();
				}
			);
			if (it != m_stack.top()->end())
				ret = *it;
		}
		return ret;
	}
	bool inGlobal()
	{
		auto top = m_stack.top();
		return (top == m_root.get());
	}
	~AstTree()
	{
		for (auto& it : m_scopes)
		{
			if (m_root.get() == it)
				continue;
		//	delete it;
		//	it = nullptr;
		}
	}
};

