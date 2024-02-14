#pragma once
#include "DuObject.h"
#include "Scope.h"
#include <llvm/IR/IRBuilder.h>
#include "Statement.h"
#include "AstTree.h"

class IfManager : public DuObject
{
protected:
	friend class IfManager;
	Type* getRetType()
	{	
		initParentFun();
		return m_function ? m_function->getType() : nullptr;
	}
public:
	class IfScope : public Scope
	{
		friend class IfManager;
		bool m_hasRet;
		IfManager* m_manager;
		void setManager(IfManager* m)
		{
			m_manager = m;
		}
	public:
		IfScope() : Scope("IfScope"), m_hasRet(false), m_manager(nullptr) {}
		virtual void addChild(DuObject* child) override
		{
			if (m_hasRet)
			{
				delete child;
				return;
			}
			if (child->isStatement())
			{
				if (static_cast<Statement*>(child)->isReturnStatement())
					m_hasRet = true;
				else if (static_cast<Statement*>(child)->isAssigmentStatement())
				{
					AssigmentStatement* as = static_cast<AssigmentStatement*>(child);
					Variable* left = static_cast<Variable*>(as->getObject());
					bool hasItem =	findObject(left->getKey()) != nullptr;
					if (!hasItem)
					{
						as->setCopyOpt();
						Variable* obj = static_cast<Variable*>(as->getObject());
						if (obj->isVariable())
						{
							static_cast<Variable*>(obj)->setAlloca(left->getAlloca());
						}
						Scope::addChild(obj);
						Scope::addChild(as);
						return;
					}
				}
			}
			m_childs.push_back(child);
		}
		const bool hasRet()
		{
			return m_hasRet;
		}
		virtual bool isIfScope() const { return true; }
		void setBlock(llvm::BasicBlock* bb)
		{
			m_llvmBlock = bb;
		}
		Type* getRetType()
		{
			return m_manager->getRetType();
		}
	};
	void assigmentMemory(llvm::IRBuilder<>& b)
	{
		auto span = m_ifelse.first->getList();
		for (auto it : span)
		{
			if (it->isVariable())
			{
				Variable* var = static_cast<Variable*>(it);
				if (var->isCopy())
				{
					Variable* _var = static_cast<Variable*>(m_ifelse.first->findUpperObject(var->getKey()));
					var->setAlloca(_var->getAlloca());
				}
				else
				{
					var->init(b.CreateAlloca(var->getLLVMType(b.getContext()), nullptr, var->getIdentifier().getName()), b);
				}
			}
		}
		if (m_ifelse.second)
		{
			span = m_ifelse.second->getList();
			for (auto it : span)
			{
				if (it->isVariable())
				{
					Variable* var = static_cast<Variable*>(it);
					if (var->isCopy())
					{
						Variable* _var = static_cast<Variable*>(m_ifelse.first->findUpperObject(var->getKey()));
						var->setAlloca(_var->getAlloca());
					}
					else
					{
						var->init(b.CreateAlloca(var->getLLVMType(b.getContext()), nullptr, var->getIdentifier().getName()), b);
					}
				}
			}
		}
	}
private:
	using IfElseStatement = std::pair<IfScope*, IfScope*>;
	IfElseStatement m_ifelse;
	llvm::BasicBlock* m_defaultInsert{ nullptr };
	Function* m_function;
	llvm::Function* m_llvmFun;
	Expression* m_cond;
	llvm::BasicBlock* m_mergeBlock{ nullptr };
	bool m_hasBothRet{ false };
	void merge(llvm::IRBuilder<>& b)
	{
		std::map<std::shared_ptr<KeyType>, llvm::PHINode*> phiMap;
		auto ifChilds = m_ifelse.first->getList();
		for (auto it : ifChilds)
		{
			if (it->isVariable())
			{
				Variable* varIt = static_cast<Variable*>(it);
				if (varIt->isCopy())
				{
					auto it = phiMap.find(varIt->getKey());
					if (it == phiMap.end())
					{
						auto _it = phiMap.insert({ varIt->getKey(), b.CreatePHI(varIt->getLLVMType(b.getContext()), 0, "")});
						_it.first->second->addIncoming(varIt->getLLVMValue(varIt->getLLVMType(b.getContext())), m_ifelse.first->getBasicBlock(b.getContext(), nullptr));
					}
				}
			}
		}

		if (m_ifelse.second)
		{
			ifChilds = m_ifelse.second->getList();
			for (auto it : ifChilds)
			{
				if (it->isVariable())
				{
					Variable* varIt = static_cast<Variable*>(it);
					if (varIt->isCopy())
					{
						auto it = phiMap.find(varIt->getKey());
						if (it == phiMap.end())
						{
							auto _it  = phiMap.insert({ varIt->getKey(), b.CreatePHI(varIt->getLLVMType(b.getContext()), 0, "") });
							_it.first->second->addIncoming(varIt->getLLVMValue(varIt->getLLVMType(b.getContext())), m_ifelse.second->getBasicBlock(b.getContext(), nullptr));
						}
						else
						{
							it->second->addIncoming(varIt->getLLVMValue(varIt->getLLVMType(b.getContext())), m_ifelse.second->getBasicBlock(b.getContext(), nullptr));
						}
					}
				}
			}
		}

		for (auto it : phiMap)
		{
			if (it.second->getNumIncomingValues() < 2)
			{
				DuObject* obj = m_ifelse.first->findUpperObject(it.first);
				if (obj)
				{
					it.second->addIncoming(obj->getLLVMValue(obj->getLLVMType(b.getContext())), m_defaultInsert);
				}
			}
		}


		for (auto it : phiMap)
		{
			DuObject* orginObj = m_ifelse.first->findUpperObject(it.first);
			orginObj->updateByLLVM(it.second, it.second->getType());
		}
	}
	IfScope* getDefault()
	{
		if (!m_ifelse.second)
			return m_ifelse.first;
		return m_ifelse.second;
	}

	IfScope* getElse()
	{
		return m_ifelse.second;
	}

	IfScope* getIf()
	{
		return m_ifelse.first;
	}
	void initParentFun()
	{
		if (m_function)
			return;
		DuObject* it = this;
		auto& tree = AstTree::instance();
		do
		{
			it = it->getParent();
			if (tree.isGlobal(it))
			{
				it = nullptr;
				break;
			}
		} while (it && !it->isFunction());
		if (it && it->isFunction())
		{
			m_function = static_cast<Function*>(it);
		}
	}


public:
	enum class ScopeFlag
	{
		Default,
		If,
		Else
	};
	IfManager(IfScope* _if, Expression* cond) : DuObject("IfManager"), m_ifelse({ _if, nullptr }), m_function(nullptr), m_cond(cond), m_llvmFun(nullptr)
	{
		m_ifelse.first->setManager(this);
		
	}

	void callCallback(std::function<void(Scope*, DuObject*)> cb, IfScope* scope)
	{
		AstTree::instance().beginScope(scope);
		for (auto it = scope->begin(); it != scope->end(); it++)
		{
			cb(scope, *it);
			IfManager* ifm = dynamic_cast<IfManager*>(*it);
			if (ifm)
			{
				llvm::BasicBlock* newBB = ifm->getMergeBlock();
				scope->setBlock(newBB);
			}
		}
		AstTree::instance().endScope();
	}

	void generateLLVM(llvm::IRBuilder<>& b, llvm::Module* m, std::function<void(Scope*, DuObject*)> cb)
	{
		
		assert(m_ifelse.first);
		initParentFun();
		m_defaultInsert = b.GetInsertBlock();
		m_cond->processExpression(m, b, b.getContext(), false);

		auto condVar = m_cond->getRes();
		if (!condVar)
		{
			assert(0);
		}
		condVar->toBoolean(b.getContext(), b);
		
		
		llvm::Value* valCond = condVar->getLLVMValue(condVar->getLLVMType(b.getContext()));
		m_llvmFun = m_function->getLLVMFunction(b.getContext(), m, b);
		
		m_hasBothRet = m_ifelse.first->hasRet() && m_ifelse.second && m_ifelse.second->hasRet();
		if (!m_hasBothRet)
			m_mergeBlock = llvm::BasicBlock::Create(b.getContext(), "merge_block", m_llvmFun);
		
		llvm::BasicBlock* then = m_ifelse.first->getBasicBlock(b.getContext(), m_function->getLLVMFunction(b.getContext(), m, b));
		then->setName("then");
		b.SetInsertPoint(then);
		AstTree::instance().beginScope(m_ifelse.first);
		callCallback(cb, getActualScope(IfManager::ScopeFlag::If));
		if (!m_hasBothRet)
			b.CreateBr(m_mergeBlock);
		AstTree::instance().endScope();

		if (m_ifelse.second)
		{
			llvm::BasicBlock* _else = m_ifelse.second->getBasicBlock(b.getContext(), m_function->getLLVMFunction(b.getContext(), m, b));
			_else->setName("else");
			b.SetInsertPoint(_else);
			AstTree::instance().beginScope(m_ifelse.second);
			callCallback(cb, getActualScope(IfManager::ScopeFlag::Else));
			if (!m_hasBothRet)
				b.CreateBr(m_mergeBlock);
			AstTree::instance().endScope();
			b.SetInsertPoint(m_defaultInsert);
			b.CreateCondBr(valCond, then, _else);

		}
		else
		{
			b.SetInsertPoint(m_defaultInsert);
			b.CreateCondBr(valCond, then, m_mergeBlock);

		}

		if (m_mergeBlock)
		{
			b.SetInsertPoint(m_mergeBlock);
			merge(b);
		}

	}
	IfScope* getActualScope(ScopeFlag sf = ScopeFlag::Default)
	{
		switch (sf)
		{
		case ScopeFlag::Default:
			return getDefault();
		case ScopeFlag::If:
			return getIf();
		case ScopeFlag::Else:
			return getElse();
		default:
			return nullptr;
		}
	}
	void beginElse()
	{
		assert(!m_ifelse.second);
		m_ifelse.second = new IfScope();
		m_ifelse.second->setParent(getParent());
		m_ifelse.second->setManager(this);
	}
	virtual bool isIfScope() const { return true; }
	virtual void setParent(DuObject* p) override
	{
		assert(p->isScope() && m_ifelse.first);
		m_parent = p;
		m_ifelse.first->setParent(p);
	}
	llvm::BasicBlock* getMergeBlock()
	{
		return m_mergeBlock;
	}

	bool hasBothRet()
	{
		return m_hasBothRet;
	}

	~IfManager()
	{
		delete m_ifelse.first;
		delete m_ifelse.second;
	}

};

