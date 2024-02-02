#pragma once
#include "Expression.h"
#include "AstTree.h"
#include "Scope.h"
class IfScope : public Scope
{
	Expression* m_expr;
	llvm::BasicBlock* m_mergeBlock;
	llvm::BasicBlock* m_elseBlock;
	class ElseSplitter : public DuObject
	{
	public:
		ElseSplitter() : DuObject(Identifier("begin else")) {}
		virtual llvm::Type* getLLVMType(llvm::LLVMContext&) const
		{
			assert(0);
			return nullptr;
		}
		virtual llvm::Value* getLLVMValue(llvm::Type* type) const
		{
			assert(0);
			return nullptr;
		}
		virtual DuObject* copy() const
		{
			assert(0);
			return nullptr;
		}
	};
public:
	IfScope(Expression* expr) : Scope(Identifier("If")), m_expr(expr),  m_mergeBlock(nullptr), m_elseBlock(nullptr)
	{
		AstTree& tree = AstTree::instance();

	}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		assert(0);
		return 0;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		assert(0);
		return nullptr;
	}

	void generateLLVMIf(llvm::LLVMContext& c, llvm::Module* m, llvm::IRBuilder<>& b, std::function<void(DuObject*, Scope*)> m_cb) 
	{
		DuObject* it = this;
		bool isParentGlobal = false;
		auto& tree = AstTree::instance();
		while (!it->isFunction())
		{
			if (tree.isGlobal(it))
			{
				isParentGlobal = true;
				break;
			}
			it = it->getParent();
		}
		if (!isParentGlobal)
		{
			bool ifReturn = false;
			bool elseReturn = false;
			bool isElse = false;
			Function* fnc = static_cast<Function*>(it);
			llvm::Function* llvmfnc = fnc->getLLVMFunction(c, m, b);
			llvm::BasicBlock* then = getBasicBlock(c, llvmfnc);
			m_expr->processExpression(m, b, c, false);
			auto variable = m_expr->getRes();
			if (!variable)
			{
				assert(0);         
			}
			llvm::Value* condition = variable->toBoolean(c, b);

			bool foundElseSplitter = false;
			for (auto it : m_childs)
			{
				if (dynamic_cast<ElseSplitter*>(it))
				{
					foundElseSplitter = true;
				}
				if (it->isStatement() && static_cast<ReturnStatement*>(it)->isReturnStatement())
				{
					if (!foundElseSplitter)
						ifReturn = true;
					else
						elseReturn = true;
				}
			}

			const bool isNeedMergeBlock = !(elseReturn && ifReturn);
			if (!m_elseBlock && foundElseSplitter)
				m_elseBlock = llvm::BasicBlock::Create(c, "else_block", llvmfnc, then);
			if (!m_mergeBlock && isNeedMergeBlock)
				m_mergeBlock = llvm::BasicBlock::Create(c, "merge_block", llvmfnc);
			if(isNeedMergeBlock)
				b.CreateCondBr(condition, then, m_elseBlock ? m_elseBlock : m_mergeBlock);
			else if (!isNeedMergeBlock)
			{
				b.CreateCondBr(condition, then, m_elseBlock);
			}
			b.SetInsertPoint(then);
			auto it = m_childs.begin();
			std::vector<DuObject*> elseStatements;
			for (; it != m_childs.end(); it++)
			{
				if (dynamic_cast<ElseSplitter*>(*it) && !isElse)
				{
					isElse = true;
					continue;
				}
				if (!isElse)
				{
					m_cb(*it, this);
				}
				else
				{
					elseStatements.push_back(*it);
				}
			}
			if(isNeedMergeBlock)
				b.CreateBr(m_mergeBlock);
			if(isElse)
			{
				b.SetInsertPoint(m_elseBlock);
				for (auto it : elseStatements)
				{
					m_cb(it, this);
				}
				if (isNeedMergeBlock)
					b.CreateBr(m_mergeBlock);
			}
			if(isNeedMergeBlock)
				b.SetInsertPoint(m_mergeBlock);
		}
	}
	virtual llvm::BasicBlock* getBasicBlock(llvm::LLVMContext& context, llvm::Function* fn) override
	{
		if (!m_llvmBlock)
		{
			m_llvmBlock = llvm::BasicBlock::Create(context, "then", fn);
		}
		return m_llvmBlock;
	}
	virtual bool isIfScope() const override { return true; }
	Function* getFunctionParent()
	{
		DuObject* ret = this;
		if (!ret->isFunction())
		{
			ret = ret->getParent();
			if (!ret)
				assert(0);
		}
		return static_cast<Function*>(ret);
	}
	~IfScope()
	{
		delete m_expr;
	}

	void beginElse()
	{
		addChild(new ElseSplitter());
	}

};