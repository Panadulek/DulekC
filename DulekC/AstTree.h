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
#include "TypeContainer.h"
#include "Interfaces.h"
extern DuObject* s_GlobalScope;
class AstTree
{
	Scope* m_root;
	std::vector<Scope*> m_scopes;
	std::stack<Scope*> m_stack;
	void createSysFunction()
	{
		m_scopes.emplace_back(new Function(SystemFunctions::getSysFunctionName<SystemFunctions::SysFunctionID::DISPLAY>(), TypeContainer::instance().getType(SimpleNumericType::generateId(ObjectInByte::DWORD, true)), {}, {}, true, false));
	}
	AstTree()
	{
		m_root = new Scope(Identifier("GLOBAL_SCOPE"));
		s_GlobalScope = m_root;
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
		auto predicate = [&obj](const DuObject* _obj)
			{ 	
				return (!_obj->isStatement() && !dynamic_cast<ISelfGeneratedScope*>(obj) && _obj->getIdentifier() == obj->getIdentifier());
			
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
		ISelfGeneratedScope* isgs = dynamic_cast<ISelfGeneratedScope*>(scope);
		if (!isgs && !scope->isIfScope())
		{
			auto predicate = [&scope](const DuObject* _scope) { return _scope->getIdentifier() == scope->getIdentifier(); };
			auto filteredView = std::views::filter(m_scopes, predicate);
			if (std::ranges::distance(filteredView) > 0)
			{
				assert(0);
			}
			m_scopes.push_back(scope);
		}
		m_stack.push(scope);
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
	bool checkGlobalVisibility(DuObject* obj2)
	{
		return obj2->getParent() == m_root;
	}
	bool checkVisibility(DuObject* obj1, DuObject* obj2)
	{
		Scope* s1 = static_cast<Scope*>(obj1->getParent());
		Scope* s2 = static_cast<Scope*>(obj2->getParent());
		if (s1 == s2)
		{
			return true;
		}
		DuObject* parent = s1->getParent();
		while (parent)
		{
			if (parent == s2)
				return true;
			parent = parent->getParent();
		}

		return false;
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
			Scope* scope = m_stack.top();
			while (true)
			{
				if (!scope)
					break;
				else if (scope->isScope())
				{
					auto it = std::find_if(scope->begin(), scope->end(), [&id](const DuObject* obj)
						{
							return id == obj->getIdentifier();
						}
					);
					if (scope->end() != it)
					{
						ret = *it;
						break;
					}
					if (scope == m_root)
						break;
					scope = static_cast<Scope*>(scope->getParent());
				}
			}
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

	bool isGlobal(DuObject * d)
	{
		return d == m_root;
	}


	void replaceObject(Variable* var)
	{
		Scope* curr = getCurrentScope();
		DuObject* obj = curr->findObject(var->getIdentifier());
		if (obj)
		{
			curr->replace(var);
			
		}
		else
		{
			while (true)
			{
				curr = static_cast<Scope*>(curr->getParent());
				if (!curr)
					break;
				obj = curr->findObject(var->getIdentifier());
				if (obj)
				{
					getCurrentScope()->replace(var);
					break;
				}
			}
		}
	}

	~AstTree()
	{
			for (auto it : m_scopes)
			{
				delete it;
			}
	}
};

