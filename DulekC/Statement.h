#pragma once
#include "DuObject.h"
#include "Variable.h"
#include <llvm/IR/IRBuilder.h>
#include "AstTree.h"
#include <llvm/IR/Instructions.h>
#include "TypeContainer.h"
#define DELETE_TMP_VARIABLE(X)	if(X->getIdentifier().getName().empty() && X->isTmp()) delete X;

class Statement : public DuObject
{
protected:

public:
	Statement(Identifier id) : DuObject(id) {}
	virtual bool isStatement() const override { return true; }
	virtual bool isAssigmentStatement() const { return false;  }
	virtual bool isReturn() const { return false;  }
	virtual bool isCallFunctionStatement() const { return false;  }
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
				m_left->update(m_right, store->getValueOperand());
			}
			else
				assert(0);
		}
		else if ( AstTree::instance().checkVisibility(m_left, m_right) )
		{
			llvm::Value* val = builder.CreateLoad(m_right->getLLVMType(context), m_right->getAlloca(), m_right->getIdentifier().getName().data());
			if (m_right->getLLVMType(context) != m_left->getLLVMType(context)) {
				val = m_left->getType()->convertValueBasedOnType(builder, val, m_right->getLLVMType(context), context);
			}
			
			auto store = builder.CreateStore(val, m_left->getAlloca());
			store->setAlignment(m_left->getAlligment());
			m_left->update(m_right, store->getValueOperand());
		}
	}

	virtual bool isAssigmentStatement() const override { return true; }

	virtual ~AssigmentStatement() 
	{
		DELETE_TMP_VARIABLE(m_right)
	}

};

class ReturnStatement : public Statement
{
	Variable* m_var;
	Type* m_retType;
	mutable llvm::ReturnInst* m_retInstance;
public:
	ReturnStatement(DuObject* var, DuObject* retType) : Statement(Identifier("return_stmt")), m_var(nullptr), m_retType(nullptr)
	{
		if (var && var->isVariable())
			m_var = static_cast<Variable*>(var);
		if (retType && retType->isType())
			m_retType = static_cast<Type*>(retType);
		m_retInstance = nullptr;
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
		if (m_retInstance)
			return;
		llvm::ReturnInst* retInstance = nullptr;
		if (m_retType)
		{
			llvm::Value* llvmRetVal = m_retType->convertValueBasedOnType(builder, m_var->getLLVMValue(nullptr), m_var->getLLVMType(context), context);
			m_retInstance = builder.CreateRet(llvmRetVal);
		}
		else
		{
			m_retInstance = builder.CreateRetVoid();
		}
	}

	llvm::ReturnInst* _return()
	{
		return m_retInstance;
	}
	virtual bool isReturn() const override { return true; }
	virtual ~ReturnStatement()
	{
		DELETE_TMP_VARIABLE(m_var)
	}
};



class CallFunction : public Statement
{
	std::vector<Identifier> m_args;
	Function* m_fun;
	bool m_isSysFunction;
public:
	CallFunction(std::vector<Identifier>&& args, Function* fun) : Statement(Identifier("call_fnc_stmt")), m_args(std::move(args)), m_fun(fun), m_isSysFunction(false)
	{
		m_isSysFunction = m_fun->getIdentifier().getName()[0] == '$';
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
		assert(0);
		return nullptr;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		assert(0);
		return nullptr;
	}
	virtual void processStatement(llvm::IRBuilder<>& builder, llvm::LLVMContext& context) const
	{ 
		
	}
	void processSystemFunc(llvm::FunctionCallee* fc, llvm::Value* str, llvm::IRBuilder<>& builder, llvm::LLVMContext& context)
	{
		AstTree& tree = AstTree::instance();
		std::vector<llvm::Value*> args;
		args.push_back(str);
		for (auto it : m_args)
		{
			auto [isNumber, val] = it.toNumber();
			auto arg = tree.findObject(it);
			llvm::Type* _type = nullptr;
			if (!arg && isNumber)
			{
 				Identifier id = SimpleNumericType::generateId(ObjectInByte::DWORD, true);
				TypeContainer::instance().insert<SimpleNumericType>(id, id, ObjectInByte::DWORD, true);
				Type* type = TypeContainer::instance().getType(SimpleNumericType::generateId(ObjectInByte::DWORD, true));
				arg = new Variable(it, type , new NumericValue(val), AstTree::instance().inGlobal());
				_type = arg->getLLVMType(context);
				args.push_back(arg->getLLVMValue(_type));
				delete arg;
				arg = nullptr;
			}
			if (arg && arg->isVariable())
			{
				Variable* _arg = static_cast<Variable*>(arg);
				args.push_back(arg->getLLVMValue(_type));
			}	
		}
		builder.CreateCall(*fc, args);
	}
	Identifier getFunctionName()
	{
 		return m_fun->getIdentifier();
	}
	virtual bool isCallFunctionStatement() const override { return true; }
	virtual ~CallFunction() {}
};