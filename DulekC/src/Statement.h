#pragma once
#include "DuObject.h"
#include "Variable.h"
#include "llvm/IR/IRBuilder.h"
#include "AstTree.h"
#include "llvm/IR/Instructions.h"
#include "TypeContainer.h"
#include "GenTmpVariables.h"
#include "Expression.h"
#include <format>
#include <memory>
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
	mutable DuObject* m_left;
	DuObject* m_right;
	bool m_hasExpr;
	bool m_copyOpt;
	void _processStatement(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Module* module) const
	{
		if (Variable* right = dynamic_cast<Variable*>(m_right))
		{
			if (Variable* left = dynamic_cast<Variable*>(m_left))
			{
				if (right && right->getIdentifier().getName().empty())
				{
					auto value = right->loadValue();
					if (value->isNumericValue())
					{
						llvm::Constant* c = llvm::ConstantInt::get(left->getLLVMType(context), static_cast<NumericValue*>(value)->loadValue());
						m_left = LlvmBuilder::assigmentValue(builder, left, c);
					}
					else
						assert(0);
				}
				else if (m_right && (AstTree::instance().checkVisibility(left, right) || AstTree::instance().checkGlobalVisibility(right)))
				{
					llvm::Value* val = nullptr;
					if (!right->isGlobalVariable())
					{
						val = builder.CreateLoad(m_right->getLLVMType(context), right->getAlloca(), right->getIdentifier().getName().data());
					}
					else
					{
						llvm::GlobalVariable* gv = module->getGlobalVariable(right->getIdentifier().getName());
						val = builder.CreateLoad(gv->getValueType(), gv, "");
					}
					if (m_right->getLLVMType(context) != m_left->getLLVMType(context)) {
						val = left->getType()->convertValueBasedOnType(builder, val, right->getLLVMType(context), context);
					}
					m_left = LlvmBuilder::assigmentValue(builder, left, val);
				}
				else
				{
					const std::string format = "Undeclared variable\n";
					printf(format.c_str());
					std::exit(15);

				}
			}
		}
	}

	void _processStatementExpr(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Module* module) const
	{
		if (Expression* expr = dynamic_cast<Expression*>(m_right))
		{
			if (Variable* left = dynamic_cast<Variable*>(m_left))
			{
				if (left->getType()->isSimpleNumericType())
				{
					expr->processExpression(module, builder, context, static_cast<SimpleNumericType*>(left->getType())->isSigned());
				}
				else if (PointerType* pt = dynamic_cast<PointerType*>(left->getType()))
				{
					expr->processExpression(module, builder, context, false);
				}
				llvm::Value* val = nullptr;

				if (expr->isExprValueWrapper())
				{
					if (ArrayOperatorExprerssion* aoe = dynamic_cast<ArrayOperatorExprerssion*>(expr))
					{
						DuObject* res = aoe->getResWrapper();
						Variable* var = dynamic_cast<ValueWrapper*>(res)->generateVariableValAsAlloca(builder);
						val = LlvmBuilder::loadValue(builder, var);
						delete var;
					}
					else
						val = expr->getResWrapper()->getLLVMValue(nullptr);
				}
				else
					val = LlvmBuilder::loadValue(builder, expr->getRes());

				if (val->getType() != m_left->getLLVMType(context))
				{
					val = left->getType()->convertValueBasedOnType(builder, val, val->getType(), context);
				}
				m_left = LlvmBuilder::assigmentValue(builder, left, val);
			}
			else if (ArrayOperatorExprerssion* left = dynamic_cast<ArrayOperatorExprerssion*>(m_left))
			{
				left->processExpression(module, builder, context, false);
				llvm::Value* lVal = nullptr;
				llvm::Value* val = nullptr;
				if (left->isExprValueWrapper())
				{
					m_left = left->getResWrapper();
					m_left = dynamic_cast<ValueWrapper*>(m_left)->generateVariableValAsAlloca(builder);					
				}
				else
				{
					m_left = left->getRes();
				}
				if (Variable* varl = dynamic_cast<Variable*>(m_left))
				{
					if (varl->getType()->isSimpleNumericType())
					{
						expr->processExpression(module, builder, context, static_cast<SimpleNumericType*>(varl->getType())->isSigned());
					}
					else if (PointerType* pt = dynamic_cast<PointerType*>(varl->getType()))
					{
						expr->processExpression(module, builder, context, false);
					}
					if (expr->isExprValueWrapper())
					{
						val = expr->getResWrapper()->getLLVMValue(nullptr);
					}
					else
						val = LlvmBuilder::loadValue(builder, expr->getRes());
					m_left = LlvmBuilder::assigmentValue(builder, varl, val);
				}
				
			}
			else
			{
				const std::string format = "null expr\n";
				printf(format.c_str());
				std::exit(15);

			}
		}
	}


public:
	AssigmentStatement(Variable* l, Variable* r) : Statement(Identifier("assigment statement")), m_left(l), m_right(r), m_hasExpr(false), m_copyOpt(false)
	{}
	AssigmentStatement(Variable* l, Expression* r) : Statement(Identifier("assigment statement")), m_left(l), m_right(r), m_hasExpr(true), m_copyOpt(false)
	{}
	AssigmentStatement(Expression* l, Expression* r) : Statement(Identifier("assigment statement")), m_left(l), m_right(r), m_hasExpr(true), m_copyOpt(false)
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
		if(Variable* rVar = dynamic_cast<Variable*>(m_right))
			DELETE_TMP_VARIABLE(rVar)
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


class ExpressionStmtWrapper : public Statement
{
	Expression *m_expr;
public:
	ExpressionStmtWrapper(Expression* expr) : m_expr(expr), Statement("Expression_Wrapper") {}
	virtual void processStatement(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Module* m) const override
	{
		m_expr->processExpression(m, builder, context, false);
	}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		return m_expr->getLLVMType(context);
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		return m_expr->getLLVMValue(type);
	}
	~ExpressionStmtWrapper()
	{
		delete m_expr;
	}
};