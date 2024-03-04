#pragma once
#include "DuObject.h"
#include "Variable.h"
#include <llvm/IR/IRBuilder.h>
#include "AstTree.h"
#include <llvm/IR/Instructions.h>
#include "TypeContainer.h"
#include "GenTmpVariables.h"
#include "Expression.h"
#include <format>
#define DELETE_TMP_VARIABLE(X)	if(X && X->getIdentifier().getName().empty() && X->isTmp()) delete X;

class Statement : public DuObject
{
protected:
	mutable bool m_beforeProcess{ true };


public:
	Statement(Identifier id) : DuObject(id) {}
	virtual bool isStatement() const override { return true; }
	virtual bool isAssigmentStatement() const { return false;  }
	virtual bool isReturnStatement() const { return false;  }
	virtual bool isCallFunctionStatement() const { return false;  }
	virtual DuObject* copy() const override
	{
		assert(0);
		return nullptr;
	}
	virtual void processStatement(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Module* module) const { assert(0); }
	virtual ~Statement() {}
};


class AssigmentStatement : public Statement
{
	mutable Variable* m_left;
	Variable* m_right;
	mutable Expression* m_expr;
	bool m_hasExpr;
	bool m_copyOpt;
	void _processStatement(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Module* module) const
	{
		if (m_right && m_right->getIdentifier().getName().empty())
		{
			auto value = m_right->loadValue();
			if (value->isNumericValue())
			{
				llvm::Constant* c = llvm::ConstantInt::get(m_left->getLLVMType(context), static_cast<NumericValue*>(value)->loadValue());
				m_left = LlvmBuilder::assigmentValue(builder,  m_left, c);
			}
			else
				assert(0);
		}
		else if (m_right && (AstTree::instance().checkVisibility(m_left, m_right) || AstTree::instance().checkGlobalVisibility(m_right)))
		{
			llvm::Value* val = nullptr;
			if (!m_right->isGlobalVariable())
			{
				val = builder.CreateLoad(m_right->getLLVMType(context), m_right->getAlloca(), m_right->getIdentifier().getName().data());
			}
			else
			{
				llvm::GlobalVariable* gv = module->getGlobalVariable(m_right->getIdentifier().getName());
				val = builder.CreateLoad(gv->getValueType(), gv, "");
			}
			if (m_right->getLLVMType(context) != m_left->getLLVMType(context)) {
				val = m_left->getType()->convertValueBasedOnType(builder, val, m_right->getLLVMType(context), context);
			}
			m_left = LlvmBuilder::assigmentValue(builder, m_left, val);
		}
		else
		{
			const std::string format = "Undeclared variable\n";
			printf(format.c_str());
			std::exit(15);

		}
	}



	void _processStatementExpr(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Module* module) const
	{
		if (m_expr)
		{
			if (m_left->getType()->isSimpleNumericType())
			{
				m_expr->processExpression(module, builder, context, static_cast<SimpleNumericType*>(m_left->getType())->isSigned());
			}
			else if (PointerType* pt = dynamic_cast<PointerType*>(m_left->getType()))
			{
				m_expr->processExpression(module, builder, context, false);
			}
			llvm::Value* val = LlvmBuilder::loadValue(builder, m_expr->getRes());

			if (val->getType() != m_left->getLLVMType(context))
			{
				val = m_left->getType()->convertValueBasedOnType(builder, val, val->getType(), context);
			}
			m_left = LlvmBuilder::assigmentValue(builder, m_left, val);
		}
		else
		{
			const std::string format = "null expr\n";
			printf(format.c_str());
			std::exit(15);

		}
	}


public:
	AssigmentStatement(Variable* l, Variable* r) : Statement(Identifier("assigment statement")), m_left(l), m_right(r), m_expr(nullptr), m_hasExpr(false), m_copyOpt(false)
	{}
	AssigmentStatement(Variable* l, Expression* r) : Statement(Identifier("assigment statement")), m_left(l), m_right(nullptr), m_expr(r), m_hasExpr(true), m_copyOpt(false)
	{}
	void setRightElement(Variable* r)
	{
		assert(!m_right && !m_hasExpr);
		m_right = r;
	}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		assert(0);
		return nullptr;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		if (m_beforeProcess)
		{
			assert(0);
			return nullptr;
		}
		m_left->getLLVMValue(type);
	}
	virtual void processStatement(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Module* module) const  override
	{
		if (!m_hasExpr)
			_processStatement(builder, context, module);
		else
			_processStatementExpr(builder, context, module);
		m_beforeProcess = false;
	}

	virtual bool isAssigmentStatement() const override { return true; }

	virtual ~AssigmentStatement() 
	{
		DELETE_TMP_VARIABLE(m_right)
		if (m_copyOpt)
			delete m_left;
	}
	virtual std::shared_ptr<KeyType>getKey() const
	{
		assert(m_left);
		return m_left->getKey();
	}



	virtual DuObject* getObject()
	{
		return m_left;
	}

	void setCopyOpt()
	{
		m_copyOpt = true;
		m_left = static_cast<Variable*>( m_left->copy() );
		
	}

};

class ReturnStatement : public Statement
{
	Variable* m_var;
	Type* m_retType;
	mutable llvm::ReturnInst* m_retInstance;
public:
	ReturnStatement(DuObject* var, DuObject* retType) : Statement(Identifier("return_stmt")), m_var(nullptr), m_retType(nullptr), m_retInstance(nullptr)
	{
		if (var && var->isVariable())
			m_var = static_cast<Variable*>(var);
		if (retType && retType->isType())
			m_retType = static_cast<Type*>(retType);
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
	virtual void processStatement(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Module*) const  override
	{
		if (m_retInstance)
			return;
		llvm::ReturnInst* retInstance = nullptr;
		if (m_retType)
		{
			if (m_var && m_retType == m_var->getType())
			{
				auto lt = m_var->getLLVMType(context);
				auto lt2 = m_retType->getLLVMType(context);
				llvm::Value* llvmRetVal = m_retType->convertValueBasedOnType(builder, m_var->getLLVMValue(m_var->getLLVMType(context)), m_var->getLLVMType(context), context);
				m_retInstance = builder.CreateRet(llvmRetVal);
			}
			else
			{
				printf("Type nie pasuja\n");
				exit(15);
			}
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

	virtual DuObject* getObject()
	{
		return m_var;
	}
	virtual bool isReturnStatement() const override { return true; }
	virtual ~ReturnStatement()
	{
		DELETE_TMP_VARIABLE(m_var)
	}
};



class CallFunction : public Statement
{
	std::unique_ptr<CallFunctionExpression> m_cfe;
public:
	CallFunction(CallFunctionExpression* cfe) : Statement(Identifier("call_fnc_stmt"))
	{
		m_cfe.reset(cfe);
	}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		return m_cfe->getLLVMType(context);
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		return m_cfe->getLLVMValue(type);
	}
	virtual void processStatement(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Module* m) const override
	{ 
		m_cfe->processExpression(m, builder, context, false);
	}
	Identifier getFunctionName()
	{
 		return m_cfe->getIdentifier();
	}
	virtual bool isCallFunctionStatement() const override { return true; }
	virtual ~CallFunction() {}
};