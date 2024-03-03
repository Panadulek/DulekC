#pragma once

#include "DuObject.h"
#include "AstTree.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include "GenTmpVariables.h"
#include "Statement.h"
#include "SystemFunctions.h"
#include "LLvmBuilder.h"
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
	DuObject* copy() const
	{
		assert(0);
		return nullptr;
	}
	virtual std::shared_ptr<KeyType> getKey() const
	{
		assert(m_res);
		return m_res->getKey();
	}
	virtual ~Expression()
	{
		delete m_res;
	}
};



class AdvancedExpression : public Expression
{
	Expression* m_l;
	Expression* m_r;

	void processMathematicalExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context, bool s, char op)
	{
		Variable* var1 = m_l->getRes();
		Variable* var2 = m_r->getRes();
		llvm::Value* lVal = LlvmBuilder::loadValue(builder, var1);
		llvm::Value* rVal = LlvmBuilder::loadValue(builder, var2);
		rVal = var1->getType()->convertValueBasedOnType(builder, rVal , rVal->getType(), context);
		llvm::Value* result = nullptr;
		if (op == '+')
		{
			result = builder.CreateAdd(lVal, rVal);
		}
		else if (op == '-')
		{
			result = builder.CreateSub(lVal, rVal);
		}
		else if (op == '*')
		{
			result = builder.CreateMul(lVal, rVal);
		}
		else if (op == '/')
		{
			if (s)
				result = builder.CreateSDiv(lVal, rVal);
			else
				result = builder.CreateUDiv(lVal, rVal);
		}
		Variable* newVar = new Variable("res+", var1->getType(), nullptr, false);
		newVar = LlvmBuilder::assigmentValue(builder, newVar, result);
		setRes(newVar);
	}
	char isMathematicalExpression()
	{
		const std::string_view view = getIdentifier().getName();
		if (!view.compare("+") || !view.compare("-") || !view.compare("*") || !view.compare("/"))
		{
			return view[0];
		}
		return '\0';
	}

	void processBooleanExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context, bool s, char op)
	{
		Variable* var1 = m_l->getRes();
		Variable* var2 = m_r->getRes();
		llvm::Value* lVal = LlvmBuilder::loadValue(builder, var1);
		llvm::Value* rVal = LlvmBuilder::loadValue(builder, var2);
		rVal = var1->getType()->convertValueBasedOnType(builder, rVal, rVal->getType(), context);
		llvm::Value* result = nullptr;
		switch (op)
		{
		case '>':
			if(!s)
				result = builder.CreateICmpUGT(lVal, rVal, ">");
			else
				result = builder.CreateICmpSGT(lVal, rVal, ">");
			break;
		case '<':
			if(!s)
				result = builder.CreateICmpULT(lVal, rVal, "<");
			else
				result = builder.CreateICmpSLT(lVal, rVal, "<");
			break;
		default:
			{
				std::string_view  opStr = getIdentifier().getName();
				if (!opStr.compare("=="))
				{
					result = builder.CreateICmpEQ(lVal, rVal, "==");
				}
				else
					assert(0);
			}
		}
		assert(result->getType()->isIntegerTy(1));
		Variable* res = new Variable("", TypeContainer::instance().getType(Type::getName(Type::ID::BOOL)), nullptr, false);
		res->setBooleanValue();
		res = LlvmBuilder::assigmentValue(builder,  res, result);
		setRes(res);
	}

	char isBooleanExpression()
	{
		const std::string_view view = getIdentifier().getName();
		if (!view.compare("<") || !view.compare(">") || !view.compare("=="))
		{
			return view[0];
		}
		return '\0';
	}

public:
	AdvancedExpression(Identifier op, Expression* l, Expression* r) : Expression(op), m_l(l), m_r(r)
	{}

	virtual void processExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context, bool s) override
	{
		m_l->processExpression(module, builder, context, s);
		m_r->processExpression(module, builder, context, s);
		assert(m_l && m_r);
		char op = isMathematicalExpression();
		if (op)
		{
			processMathematicalExpression(module, builder, context, s, op);
			return;
		}
		op = isBooleanExpression();
		if (op)
		{
			auto lt = m_l->getRes()->getType();
			if (lt && lt->isSimpleNumericType())
			{
				s = static_cast<SimpleNumericType*>(lt)->isSigned();
			}
			processBooleanExpression(module, builder, context, s, op);
			return;
		}
		assert(0);
	}
	virtual ~AdvancedExpression() 
	{
		delete m_l;
		delete m_r;
	}
};



class CallFunctionExpression : public Expression
{
	std::vector<Identifier> m_args;
	Function* m_fun;

	llvm::Value* processUserFunc(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Module* m) const 
	{
		AstTree& tree = AstTree::instance();
		std::vector<llvm::Value*> args;
		for (auto it : m_args)
		{
			auto [isNumber, val] = it.toNumber();
			auto arg = tree.findObject(it);
			llvm::Type* _type = nullptr;
			if (!arg && isNumber)
			{
				std::unique_ptr<Variable> uniqueArg = GeneratorTmpVariables::generateI32Variable(it, val);
				_type = uniqueArg->getLLVMType(context);
				args.push_back(uniqueArg->getLLVMValue(_type));
			}
			if (arg && arg->isVariable())
			{
				Variable* _arg = static_cast<Variable*>(arg);
				args.push_back(arg->getLLVMValue(arg->getLLVMType(context)));
			}
		}
		return builder.CreateCall(m_fun->getLLVMFunction(context, m, builder), args);
	}
	llvm::Value* processSystemFunc(llvm::FunctionCallee* fc, llvm::IRBuilder<>& builder, llvm::LLVMContext& context)
	{
		AstTree& tree = AstTree::instance();
		std::vector<llvm::Value*> args;
		for (auto it : m_args)
		{
			auto [isNumber, val] = it.toNumber();
			auto arg = tree.findObject(it);
			llvm::Type* _type = nullptr;
			if (!arg && isNumber)
			{
				std::unique_ptr<Variable> uniqueArg = GeneratorTmpVariables::generateI32Variable(it, val);
				_type = uniqueArg->getLLVMType(context);
				args.push_back(uniqueArg->getLLVMValue(_type));
			}
			if (arg && arg->isVariable())
			{
				Variable* _arg = static_cast<Variable*>(arg);
				args.push_back(LlvmBuilder::loadValue(builder, _arg));
			}
		}
		return builder.CreateCall(*fc, args);
	}


public:
	CallFunctionExpression(std::vector<Identifier>&& args, Function* fun) : Expression(Identifier("CallFunctionExpr")), m_args(std::move(args)), m_fun(fun)
	{
		AstTree& tree = AstTree::instance();
		for (auto it : m_args)
		{
			auto arg = tree.findObject(it);
			auto [isNumber, val] = it.toNumber();
			if (!arg && !isNumber)
			{
				std::cout << "nie znaleziono obiektu: " << it.getName() << std::endl;
				std::exit(-1);
			}
		}
	}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		return m_fun->getLLVMType(context);
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		return getRes()->getLLVMValue(type);
	}

	virtual void processExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context, bool) override
	{
		bool isSystemFun = m_fun->getIdentifier().getName().data()[0] == '$';
		llvm::Value* result = nullptr;
		if (isSystemFun)
		{
			SystemFunctions* sf = SystemFunctions::GetSystemFunctions(module, &builder, &context);
			auto callee = sf->findFunction(m_fun->getIdentifier());
			if (callee)
				result = processSystemFunc(callee,  builder, context);
			else
				isSystemFun = false;
		}
		if(!isSystemFun)
		{
			result = processUserFunc(builder, context, module);
		}
		if (m_fun->isProcedure())
			return;
		if (getLLVMType(context))
		{
			Variable* res = new Variable(Identifier(""), m_fun->getType(), nullptr, false);
			res = LlvmBuilder::assigmentValue(builder, res, result);
			setRes(res);
		}
		else
		{
			setRes(nullptr);
		}
		
	}
};

class BasicExpression : public Expression
{
public:
	BasicExpression(Identifier id) : Expression(id) {}

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
				auto toDelete = GeneratorTmpVariables::generateI32Variable(getIdentifier(), val);
				llvm::Value* initVal = toDelete->init(builder.CreateAlloca(toDelete->getLLVMType(context), toDelete->getLLVMValue(toDelete->getLLVMType(context))), builder);
				LlvmBuilder::assigmentValue(builder, toDelete.get(), initVal);
				setRes(toDelete.get());
				toDelete.release();
			}
		}
		
	}
	virtual ~BasicExpression() {}

};