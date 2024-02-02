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
			llvm::Value* condition = variable->getLLVMValue(variable->getLLVMType(c));
			if(!variable->isBooleanValue())
				condition = b.CreateICmpNE(condition, b.getInt32(0), "int to bool");
			if (!m_mergeBlock)
				m_mergeBlock = llvm::BasicBlock::Create(c, "merge_block", llvmfnc);
			if(!m_elseBlock)
				m_elseBlock = llvm::BasicBlock::Create(c, "else_block", llvmfnc, m_mergeBlock);
			bool foundElseSplitter = false;
			for (auto it : m_childs)
			{
				if (dynamic_cast<ElseSplitter*>(it))
				{
					foundElseSplitter = true;
				}
			}
			if (!foundElseSplitter)
			{
				m_elseBlock->eraseFromParent();
				m_elseBlock = nullptr;
			}
			b.CreateCondBr(condition, then, m_elseBlock ? m_elseBlock : m_mergeBlock);
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
					m_cb(*it, this);
				else
					elseStatements.push_back(*it);
			}
			b.CreateBr(m_mergeBlock);
			if(isElse)
			{
				b.SetInsertPoint(m_elseBlock);
				for (auto it : elseStatements)
				{
					m_cb(it, this);
				}
				b.CreateBr(m_mergeBlock);
			}

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
	~IfScope()
	{
		delete m_expr;
	}

	void beginElse()
	{
		addChild(new ElseSplitter());
	}

};