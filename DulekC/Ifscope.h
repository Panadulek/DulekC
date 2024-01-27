#pragma once
#include "Expression.h"
#include "AstTree.h"
#include "Scope.h"
class IfScope : public Scope
{
	Expression* m_expr;
	llvm::BasicBlock* m_mergeBlock;
public:
	IfScope(Expression* expr) : Scope(Identifier("If")), m_expr(expr),  m_mergeBlock(nullptr)
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
			Function* fnc = static_cast<Function*>(it);
			llvm::Function* llvmfnc = fnc->getLLVMFunction(c, m, b);
			llvm::BasicBlock* then = getBasicBlock(c, llvmfnc);
			m_expr->processExpression(m, b, c, false);
			auto variable = m_expr->getRes();
			if (!variable)
			{
				assert(0);         //tu ma byc error
			}
			llvm::Value* condition = variable->getLLVMValue(variable->getLLVMType(c));
			condition = b.CreateICmpNE(condition, b.getInt32(0), "int to bool");
			if (!m_mergeBlock)
				m_mergeBlock = llvm::BasicBlock::Create(c, "merge_block", llvmfnc);
			b.CreateCondBr(condition, then, m_mergeBlock);
			b.SetInsertPoint(then);
			for (auto it : m_childs)
			{
				m_cb(it, this);
			}
			then = b.GetInsertBlock();
			b.CreateBr(m_mergeBlock);
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
};