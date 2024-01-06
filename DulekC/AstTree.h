#pragma once
#include "DuObject.h"
#include "Scope.h"
#include <map>
#include <llvm/IR/Module.h>
#include <stack>
#include <cassert>
#include <iostream>
#include <ranges>
#include "SystemFunctions.h"


class AstTree
{
	Scope* m_root;
	std::vector<Scope*> m_scopes;
	std::stack<Scope*> m_stack;
	void createSysFunction()
	{
		m_scopes.emplace_back(new Function(SystemFunctions::getSysFunctionName<SystemFunctions::SysFunctionID::DISPLAY>(), nullptr));
	}
	AstTree()
	{
		m_root = new Scope(Identifier("GLOBAL_SCOPE"));
		m_stack.push(m_root);
		m_scopes.push_back(m_root);
		createSysFunction();
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
		static auto predicate = [&obj](const DuObject* _obj)
			{ 
				
				return (!_obj->isStatement() &&  _obj->getIdentifier() == obj->getIdentifier());
			
			};
		assert(!m_stack.empty());
		Scope* top = m_stack.top();
		auto filteredView = std::views::filter(*top, predicate);
		if (std::ranges::distance(filteredView) > 0)
		{
			assert(0);
		}
		if (top != m_root)
		{
			filteredView = std::views::filter(*m_root, predicate);
			if (std::ranges::distance(filteredView) > 0)
			{
				assert(0);
			}
		}
		obj->setParent(m_stack.top());
		top->addChild(obj);
	}

	void beginScope(Scope* scope)
	{
		static auto predicate = [&scope](const DuObject* _scope) { return _scope->getIdentifier() == scope->getIdentifier(); };
		auto filteredView = std::views::filter(m_scopes, predicate);
		if (std::ranges::distance(filteredView) > 0)
		{
			assert(0);
		}
		m_scopes.push_back(scope);
		m_stack.push(m_scopes.back());
	}
	void endScope()
	{
		assert(m_stack.top() != m_root);
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
		Scope* s1 = static_cast<Scope*>(obj1->getParent());
		Scope* s2 = static_cast<Scope*>(obj2->getParent());
		return ((s1 == s2) || s2 == m_root);
	}
	DuObject* _findObject(Identifier id, bool global)
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

	DuObject* findObject(Identifier id)
	{
		DuObject* ret = _findObject(id, false);
		if (!ret)
			ret = _findObject(id, true);
		return ret;
	}
	Scope* getCurrentScope()
	{
		return m_stack.top();
	}
	bool setCurrentScope(Scope* sc)
	{
		if (sc == m_root)
			return false;
		auto it = std::ranges::find_if(m_scopes, [&](Scope* _sc)->bool
			{
				return _sc == sc;
			});

		if (it != m_scopes.end())
		{
			m_stack.push(*it);
			return true;
		}
		return false;
	}
	Function* findFunction(Identifier id)
	{
		auto filtredView = m_scopes | std::views::filter([&](const Scope* s)
			{
				return s->getIdentifier() == id;
			});
		auto size = std::ranges::distance(filtredView);
		if (size == 0)
		{
			return nullptr;
		}
		else if (size == 1)
		{
			auto begin = filtredView.begin();
			Function* fn = (*begin)->isFunction() ? static_cast<Function*>(*begin) : nullptr;
			return fn;
		}
		else
			return 0;
	}
	bool inGlobal()
	{
		auto top = m_stack.top();
		return (top == m_root);
	}
	~AstTree()
	{
			for (auto it : m_scopes)
			{
				delete it;
			}
	}
};

