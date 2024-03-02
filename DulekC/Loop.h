#pragma once
#include "Scope.h"
#include "Expression.h"
#include "Interfaces.h"

class Loop : public Scope, public ISelfGeneratedScope
{
	llvm::BasicBlock* m_loopBlock;
	llvm::BasicBlock* m_mergeBlock;
protected:
	Expression* m_cond;
	Function* m_function;
	llvm::Function* m_llvmFun;
	bool m_hasRet;
	Loop(Identifier id, Expression* cond) : m_cond(cond), Scope(id), m_function(nullptr), m_llvmFun(nullptr), m_hasRet(false), m_loopBlock(nullptr), m_mergeBlock(nullptr)
	{
	}
	virtual void initParentFun() override
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



	virtual const bool HasBranchedRet() const
	{
		return false;
	}


	void createMergeBlock(llvm::IRBuilder<>& b)
	{
		if (!m_mergeBlock)
			m_mergeBlock = llvm::BasicBlock::Create(b.getContext(), "while_merge", m_llvmFun);
	}
	virtual void callCallback(std::function<void(Scope*, DuObject*)> cb, Scope* scope)
	{
		AstTree::instance().beginScope(scope);
		for (auto it = scope->begin(); it != scope->end(); it++)
		{
			cb(scope, *it);
			ISelfGeneratedScope* ifm = dynamic_cast<ISelfGeneratedScope*>(*it);
			if (ifm)
			{
				llvm::BasicBlock* newBB = ifm->getMergeBlock();
				scope->setBlock(newBB);
				if (ifm->HasBranchedRet())
				{
					break;
				}
			}
		}
		AstTree::instance().endScope();
	}
public:
	virtual void generateLLVM(llvm::IRBuilder<>& b, llvm::Module* m, std::function<void(Scope*, DuObject*)> cb) override
	{
		initParentFun();
		m_llvmFun = m_function->getLLVMFunction(b.getContext(), m, b);
	}
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
	virtual llvm::BasicBlock* getBasicBlock(llvm::LLVMContext& context, llvm::Function* fn) override
	{
		if (!m_llvmBlock)
		{
			m_llvmBlock = llvm::BasicBlock::Create(context, "loop_body", fn);
		}
		return m_llvmBlock;
	}

	virtual llvm::BasicBlock* getMergeBlock()
	{
		return m_mergeBlock;
	}
};



class WhileScope final : public Loop
{

public:

	WhileScope(Expression* cond) : Loop("WHILE_SCOPE", cond)
	{}
	virtual void generateLLVM(llvm::IRBuilder<>& b, llvm::Module* m, std::function<void(Scope*, DuObject*)> cb) override
	{
		Loop::generateLLVM(b, m, cb);
		llvm::BasicBlock* entry = llvm::BasicBlock::Create(b.getContext(), "while_entry", m_llvmFun);

		createMergeBlock(b);
		llvm::BasicBlock* merge = getMergeBlock();
		llvm::BasicBlock* loop = getBasicBlock(b.getContext(), m_llvmFun);
		b.CreateBr(entry);
		b.SetInsertPoint(entry);
		m_cond->processExpression(m, b, b.getContext(), false);
		b.CreateCondBr(getCondValue(b), loop, merge);

		b.SetInsertPoint(loop);
		callCallback(cb, this);
		if(!m_hasRet)
		{
			b.CreateBr(entry);
		}
		b.SetInsertPoint(merge);
	}
};