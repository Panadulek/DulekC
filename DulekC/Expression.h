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
	llvm::Value* m_res;
protected:
	Expression(Identifier id) : DuObject(id), m_res(nullptr) {}
	void setRes(llvm::Value* res)
	{
		m_res = res;
	}
public:
	virtual void processExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context, bool s = true) = 0;
	virtual llvm::Type* getLLVMType(llvm::LLVMContext&) 
	{
		assert(0);
		return nullptr;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const
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

	virtual void processExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context, bool s = true) override
	{
		assert(m_l && m_r);
		std::string_view view = getIdentifier().getName();
		m_l->processExpression(module, builder, context, s);
		m_r->processExpression(module, builder, context, s);
		if (!view.compare("+"))
		{
			setRes(builder.CreateAdd(m_l->getLLVMValue(nullptr), m_r->getLLVMValue(nullptr)));
		}
		else if (!view.compare("-"))
		{
			setRes(builder.CreateSub(m_l->getLLVMValue(nullptr), m_r->getLLVMValue(nullptr)));
		}
		else if (!view.compare("*"))
		{
			setRes(builder.CreateMul(m_l->getLLVMValue(nullptr), m_r->getLLVMValue(nullptr)));
		}
		else if (!view.compare("/"))
		{
			if(s)
				setRes(builder.CreateSDiv(m_l->getLLVMValue(nullptr), m_r->getLLVMValue(nullptr)));
			else
				setRes(builder.CreateUDiv(m_l->getLLVMValue(nullptr), m_r->getLLVMValue(nullptr)));
		}
		else
		{
			setRes(nullptr);
		}
	}
	virtual ~MatematicalExpression() 
	{
		delete m_l;
		delete m_r;
	}
};


class BasicExpression : public Expression
{
public:
	BasicExpression(Identifier id) : Expression(id) {}

	virtual void processExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context, bool s = true)
	{

	}
	virtual ~BasicExpression() {}

};