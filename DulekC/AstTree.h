#pragma once
#include "DuObject.h"
#include "Scope.h"
#include <map>
#include <llvm/IR/Module.h>
#include <stack>
#include <cassert>

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
			delete it;
			it = nullptr;
		}
	}
};

