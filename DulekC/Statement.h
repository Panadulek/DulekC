#pragma once
#include "DuObject.h"
#include "Variable.h"
#include <llvm/IR/IRBuilder.h>
#include "AstTree.h"
class Statement : public DuObject
{
protected:

public:
	Statement(Identifier id) : DuObject(id) {}
	virtual bool isStatement() const override { return true; }
	virtual bool isAssigmentStatement() const { return false;  }
	virtual void processStatement(llvm::IRBuilder<>& builder, llvm::LLVMContext& context) const { assert(0); }
	virtual ~Statement() {}
};


class AssigmentStatement : public Statement
{
	Variable* m_left;
	Variable* m_right;
public:
	AssigmentStatement(Variable* l, Variable* r) : Statement(Identifier("assigment statement")), m_left(l), m_right(r)
	{}
	void setRightElement(Variable* r)
	{
		assert(!m_right);
		m_right = r;
	}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		assert(0);
		return nullptr;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		assert(0);
		return nullptr;
	}
	virtual void processStatement(llvm::IRBuilder<>& builder, llvm::LLVMContext& context) const  override
	{
		if ( m_right->getIdentifier().getName().empty() )
		{
			auto value = m_right->getValue();
			if (value->isNumericValue())
			{
				auto store = builder.CreateStore(llvm::ConstantInt::get(m_left->getLLVMType(context), static_cast<NumericValue*>(value)->getValue()), m_left->getAlloca());
				store->setAlignment(m_left->getAlligment());
			}
			else
				assert(0);
		}
		else if (AstTree::instance().checkVisibility(m_left, m_right) || true)
		{
			llvm::Value* val = builder.CreateLoad(m_right->getLLVMType(context), m_right->getAlloca(), m_right->getIdentifier().getName().data());
			if (m_right->getLLVMType(context) != m_left->getLLVMType(context)) {
				val = m_left->getType()->convertValueBasedOnType(builder, val, m_right->getLLVMType(context), context);
			}
			
			auto store = builder.CreateStore(val, m_left->getAlloca());
			store->setAlignment(m_left->getAlligment());
		}
	}

	virtual bool isAssigmentStatement() const override { return true; }

	virtual ~AssigmentStatement() 
	{
		if (m_right->getIdentifier().getName().empty())
			delete m_right;
	}

};

class ReturnStatement : public Statement
{

};