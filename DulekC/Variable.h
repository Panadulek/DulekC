#pragma once
#include "Type.h"
#include "Value.h"
#include "llvm/IR/IRBuilder.h"
class Variable : public DuObject
{
	Type* m_type;
	Value* m_value;
	bool m_isGlobal;
public:
	Variable(Identifier id, Type* type, Value* val, bool globalScope) : DuObject(id), m_type(type), m_value(val), m_isGlobal(globalScope) {}
	virtual bool isVariable() const override { return true; }
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		return m_type->getLLVMType(context);
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		if (m_value->isNumericValue() && m_type->isSimpleNumericType())
		{
			static_cast<NumericValue*>(m_value)->setSigned(static_cast<SimpleNumericType*>(m_type)->isSigned());
		}
		return m_value->getLLVMValue(type);
	}
	bool isGlobalVariable()
	{
		return m_isGlobal;
	}
	virtual ~Variable() 
	{
		delete m_value;
	}
};