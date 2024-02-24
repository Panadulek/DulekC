#pragma once
#include "DuObject.h"
#include "Scope.h"
#include <llvm/IR/IRBuilder.h>
#include "LLvmBuilder.h"
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
		bool m_hasNoMerge;
		IfManager* m_manager;
		void setManager(IfManager* m)
		{
			m_manager = m;
		}
	public:
		IfScope() : Scope("IfScope"), m_hasRet(false), m_manager(nullptr), m_hasNoMerge(false) {}
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

		void setNoMerge()
		{
			m_hasNoMerge = true;
		}
		const bool getNoMerge() const
		{
			return m_hasNoMerge;
		}
	};
	void assigmentMemory(llvm::IRBuilder<>& b)
	{
		_assigmentMemory(m_ifelse.first, b);
		_assigmentMemory(m_ifelse.second, b);
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

	llvm::Value* getCondValue(llvm::IRBuilder<>& b)
	{
		auto condVar = m_cond->getRes();
		if (!condVar)
		{
			assert(0);
		}
		condVar->toBoolean(b.getContext(), b);
		return LlvmBuilder::loadValue(b, condVar);
	}


	void generateBlock(IfScope* ifs, const char* blockName, std::function<void(Scope*, DuObject*)> cb, llvm::BasicBlock* block, llvm::IRBuilder<>& b)
	{
		block->setName(blockName);
		b.SetInsertPoint(block);
		AstTree::instance().beginScope(ifs);
		callCallback(cb, ifs);
		if (!ifs->hasRet())
			b.CreateBr(m_mergeBlock);
		AstTree::instance().endScope();
	}
	void _assigmentMemory(IfScope* ifm, llvm::IRBuilder<>& b)
	{
		if (!ifm)
			return;
		auto span = ifm->getList();
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

	void merge(llvm::IRBuilder<>& b, Scope* ifs, std::map<std::shared_ptr<KeyType>, llvm::PHINode*>& map);
	void merge(llvm::IRBuilder<>& b)
	{

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
				if (ifm->hasBothRet())
				{
					scope->setNoMerge();
					break;
				}
			}
		}
		AstTree::instance().endScope();
	}

	

	void generateLLVM(llvm::IRBuilder<>& b, llvm::Module* m, std::function<void(Scope*, DuObject*)> cb)
	{
		std::map<std::shared_ptr<KeyType>, llvm::PHINode*> map;
		assert(m_ifelse.first);
		initParentFun();
		m_llvmFun = m_function->getLLVMFunction(b.getContext(), m, b);
		m_cond->processExpression(m, b, b.getContext(), false);

		m_defaultInsert = b.GetInsertBlock();
		llvm::BasicBlock* then = m_ifelse.first->getBasicBlock(b.getContext(), m_function->getLLVMFunction(b.getContext(), m, b));
		llvm::BasicBlock* _else = nullptr;
		if(m_ifelse.second)
			_else = m_ifelse.second->getBasicBlock(b.getContext(), m_function->getLLVMFunction(b.getContext(), m, b));



		m_hasBothRet = m_ifelse.first->hasRet() && m_ifelse.second && m_ifelse.second->hasRet();
		if (!m_hasBothRet)
		{
			m_mergeBlock = llvm::BasicBlock::Create(b.getContext(), "merge_block", m_llvmFun);
			b.CreateCondBr(getCondValue(b), then, _else ? _else : m_mergeBlock);
			
		}		
		generateBlock(m_ifelse.first, "then", cb, then, b);
		if (_else)
		{
			generateBlock(m_ifelse.second, "else", cb, _else, b);
			
		}
		b.SetInsertPoint(m_mergeBlock);
		merge(b, m_ifelse.first, map);
		merge(b, _else ? m_ifelse.second : static_cast<Scope*>(getParent()), map);
		if (!m_hasBothRet)
		{			
			Scope* parent = static_cast<Scope*>(getParent());
			auto view = parent->getList();
			for (auto it : view)
			{
				if (it->isVariable())
				{
					auto _it = map.find(it->getKey());
					if (_it != map.end())
					{
						LlvmBuilder::assigmentValue(b, static_cast<Variable*>(it), _it->second);
					}
				}
			}

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

