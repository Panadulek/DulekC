#pragma once

#include "DuObject.h"
#include "AstTree.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include "GenTmpVariables.h"
class Expression : public DuObject
{

	Variable* m_res;
protected:
	Expression(Identifier id) : DuObject(id), m_res(nullptr) {}
	void setRes(DuObject* res)
	{
		assert(res->isVariable());
		m_res = static_cast<Variable*>(res);
	}

public:
	virtual void processExpression(llvm::Module*, llvm::IRBuilder<>&, llvm::LLVMContext&, bool s) = 0;
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& c) const override
	{
		return m_res->getLLVMType(c);
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		return m_res->getLLVMValue(type);
	}
	Variable* getRes() const
	{
		return m_res;
	}
	virtual ~Expression() {}
};



class MatematicalExpression : public Expression
{
	Expression* m_l;
	Expression* m_r;
public:
	MatematicalExpression(Identifier op, Expression* l, Expression* r) : Expression(op), m_l(l), m_r(r)
	{}

	virtual void processExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context, bool s) override
	{
		assert(m_l && m_r);
		std::string_view view = getIdentifier().getName();
		m_l->processExpression(module, builder, context, s);
		m_r->processExpression(module, builder, context, s);
		
		auto var1 = m_l->getRes();

		auto lVal = m_l->getLLVMValue(m_l->getLLVMType(context));
		auto rVal = var1->getType()->convertValueBasedOnType(builder, m_r->getLLVMValue(m_l->getLLVMType(context)), m_r->getLLVMType(context), context);
		
		llvm::Value* result = nullptr;
		
		if (!view.compare("+"))
		{
			result = builder.CreateAdd(lVal, rVal);
		}
		else if (!view.compare("-"))
		{
			result = builder.CreateSub(lVal, rVal);
		}
		else if (!view.compare("*"))
		{
			result = builder.CreateMul(lVal, rVal);
		}
		else if (!view.compare("/"))
		{
			if(s)
				result = builder.CreateSDiv(lVal, rVal);
			else
				result = builder.CreateUDiv(lVal, rVal);
		}
		var1->update(m_r->getRes(), result);
		setRes(var1);
	}
	virtual ~MatematicalExpression() 
	{
		delete m_l;
		delete m_r;
	}
};


class BasicExpression : public Expression
{
	std::unique_ptr<Variable> m_toDelete;
public:
	BasicExpression(Identifier id) : Expression(id), m_toDelete(nullptr) {}

	virtual void processExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context, bool s)
	{ 
		auto& tree = AstTree::instance();
		auto ret = tree.findObject(getIdentifier());
		llvm::Value* res = nullptr;
		if(ret && ret->isVariable())
		{
			setRes(ret);
		}
		else 
		{
			auto [isNumber, val] = getIdentifier().toNumber();
			if (isNumber)
			{
				m_toDelete = GeneratorTmpVariables::generateI32Variable(getIdentifier(), val);
				setRes(m_toDelete.get());
			}
		}
		
	}
	virtual ~BasicExpression() {}

};