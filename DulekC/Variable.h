#pragma once
#include "Type.h"
#include "Value.h"
#include "llvm/IR/IRBuilder.h"
#include "TypeContainer.h"
class AstTree;
#define DECLARELLVM(X) mutable llvm::##X* m_llvm##X

class Variable : public DuObject
{
	Type* m_type;
	Value* m_value;
	DECLARELLVM(Type);
	DECLARELLVM(Value);
	DECLARELLVM(AllocaInst);
	bool m_isGlobal;
	bool m_isTmp = false;
	llvm::Value* _getLLVMValue(llvm::Type* type) const
	{
		if (m_value->isNumericValue() && m_type->isSimpleNumericType())
		{
			static_cast<NumericValue*>(m_value)->setSigned(static_cast<SimpleNumericType*>(m_type)->isSigned());
		}
		return m_value->getLLVMValue(type);
	}
public:
	Variable(Identifier id, Type* type, Value* val, bool globalScope) : DuObject(id), m_type(type), m_value(val), m_isGlobal(globalScope), 
		m_llvmType(nullptr), m_llvmValue(nullptr), m_llvmAllocaInst(nullptr) {}
	virtual bool isVariable() const override { return true; }
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		if (!m_llvmType)
			m_llvmType = m_type->getLLVMType(context);
		return m_llvmType;
		
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		if (!m_llvmValue)
			m_llvmValue = _getLLVMValue(type);
		return m_llvmValue;

	}

	void init(llvm::AllocaInst* inst, llvm::IRBuilder<>& builder)
	{
		auto align = getAlligment();
		inst->setAlignment(align);
		auto store = builder.CreateStore(m_llvmValue, inst, false);
		store->setAlignment(align);
		m_llvmAllocaInst = inst;
	}

	llvm::AllocaInst* getAlloca()
	{
		return m_llvmAllocaInst;
	}
	const llvm::Align getAlligment() const
	{
		return llvm::Align(m_type->getSizeInBytes());
	}
	bool isGlobalVariable()
	{
		return m_isGlobal;
	}
	void update(Variable* var, llvm::Value* newLLVMValue = nullptr)
	{
		if (typeid(var->getType()) == typeid(m_type))
		{
			m_value = var->getValue()->copy();
			m_llvmValue = newLLVMValue;
		}
	}
	Type* getType()
	{
		return m_type;
	}
	Value* getValue()
	{
		return m_value;
	}
	Variable* copy()
	{

	}
	void setTmp()
	{
		m_isTmp = true;
	}
	const bool isTmp() const
	{
		return m_isTmp;
	}
	virtual ~Variable() 
	{
		delete m_value;
	}
};