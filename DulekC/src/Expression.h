#pragma once

#include "DuObject.h"
#include "AstTree.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include "GenTmpVariables.h"
#include "SystemFunctions.h"
#include "LLvmBuilder.h"
#include "ValueWrapper.h"


class Expression : public DuObject
{
	union
	{
		Variable* m_res;
		ValueWrapper* m_resWrapper;
	};
protected:
	bool m_bValueWrapper{ false };
	Expression(Identifier id) : DuObject(id), m_res(nullptr) {}
	void setRes(DuObject* res)
	{
		assert(res->isVariable() || res->isValueWrapper());
		if (res->isValueWrapper())
		{
			m_resWrapper = dynamic_cast<ValueWrapper*>(res);
			m_bValueWrapper = true;
		}
		else
		{
			m_res = dynamic_cast<Variable*>(res);
		}
	}

public:
	virtual void processExpression(llvm::Module*, llvm::IRBuilder<>&, llvm::LLVMContext&, bool s) = 0;
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& c) const override
	{
		if (m_bValueWrapper)
		{
			return m_resWrapper->getType()->getLLVMType(c);
		}
		else
			return m_res->getLLVMType(c);
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		if (m_bValueWrapper)
			return m_resWrapper->getLLVMValue(type);
		else
			return m_res->getLLVMValue(type);
	}
	Variable* getRes() const
	{
		return m_res;
	}
	ValueWrapper* getResWrapper()
	{
		return m_resWrapper;
	}
	DuObject* copy() const
	{
		assert(0);
		return nullptr;
	}
	virtual std::shared_ptr<KeyType> getKey() const
	{
		assert(m_res && m_bValueWrapper);
		return m_res->getKey();
	}

	bool isValueWrapper()
	{
		return m_resWrapper;
	}
	const bool isExprValueWrapper()
	{
		return m_bValueWrapper;
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


	void processPointerExpression(llvm::Module* m, llvm::IRBuilder<>& b, char op)
	{
		assert(0);
	}

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
			Variable* var = m_l->getRes();
			if (var->isPointer())
			{
				processPointerExpression(module, builder, op);
			}
			else
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
		if (fc && fc->getFunctionType()->getNumParams() != m_args.size())
		{
			Error(MessageEngine::Code::INVALID_NUMBER_OF_ARGUMENTS, nullptr);
		}
		for (int i = 0; i < m_args.size(); i++)
		{
			auto [isNumber, val] = m_args[i].toNumber();
			auto arg = tree.findObject(m_args[i]);
			llvm::Type* _type = nullptr;
			if (!arg && isNumber)
			{
				std::unique_ptr<Variable> uniqueArg = nullptr;
				if(fc->getFunctionType()->getParamType(i) == builder.getInt32Ty())
				{
					uniqueArg = GeneratorTmpVariables::generateI32Variable(m_args[i], val);
				}
				else
				{
					uniqueArg = GeneratorTmpVariables::generateI64Variable(m_args[i], val);
				}
				_type = uniqueArg->getLLVMType(context);
				args.push_back(uniqueArg->getLLVMValue(_type));
			}
			if (arg && arg->isVariable())
			{
				Variable* _arg = static_cast<Variable*>(arg);
				args.push_back(LlvmBuilder::loadValue(builder, _arg));
			}
			if (args[i]->getType() != fc->getFunctionType()->getParamType(i))
				Error(MessageEngine::Code::INVALID_ARGUMENT_TYPE, nullptr);
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



class AllocExpression : public Expression
{
	Type* m_type;
	Expression* m_counts;
public:
	AllocExpression(Type* type, Expression* counts) : m_type(type), m_counts(counts), Expression("AllocaExpression")
	{
	}
	virtual void processExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context, bool s)
	{
		auto& tree = AstTree::instance();
		llvm::Type* type = m_type->getLLVMType(context);
		llvm::Constant* size_of = llvm::ConstantExpr::getSizeOf(type);
		m_counts->processExpression(module, builder, context, s);
		Variable* countsVar = m_counts->getRes();
		llvm::Value* counts = countsVar->getLLVMValue(countsVar->getLLVMType(context));
		SystemFunctions* sf = SystemFunctions::GetSystemFunctions(module, &builder, &context);
		llvm::FunctionCallee* callee = sf->findFunction(SystemFunctions::getSysFunctionName(SystemFunctions::SysFunctionID::ALLOCATE_MEMORY));
		llvm::Value* allocatedMemory = LlvmBuilder::allocate ( builder, size_of, counts, callee );
		setRes(new ValueWrapper("allocated_value", allocatedMemory, m_type));
	}
};


class DeallocateExpression : public Expression
{
	Variable* m_obj;
public:
	DeallocateExpression(Identifier id) : m_obj(nullptr), Expression("Deallocate") 
	{
		auto& tree = AstTree::instance();
		auto obj = tree.findObject(id);
		if (Variable* var = dynamic_cast<Variable*>(obj))
		{
			if (var->isPointer())
			{
				m_obj = var;
			}
			else
			{
				Error(MessageEngine::Code::INVALID_ARGUMENT_TYPE, "");
			}
		}
		else
			Error(MessageEngine::Code::ERROR_TOKEN, "");
	}

	virtual void processExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context, bool s)
	{
		SystemFunctions* sf = SystemFunctions::GetSystemFunctions(module, &builder, &context);
		llvm::FunctionCallee* callee = sf->findFunction(SystemFunctions::getSysFunctionName(SystemFunctions::SysFunctionID::DEALLOCATE_MEMORY));
		llvm::Value* ptrToDelete = LlvmBuilder::loadValue(builder, m_obj);
		assert(ptrToDelete && ptrToDelete->getType()->isPointerTy());
		llvm::Value* resVal = LlvmBuilder::deallocate(builder, ptrToDelete, callee);
		LlvmBuilder::assigmentValue(builder, m_obj, resVal);
	}
};