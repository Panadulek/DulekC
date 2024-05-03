#pragma once
#include "DuObject.h"
#include "Type.h"
class Variable;
class ValueWrapper : public DuObject
{
	llvm::Value* m_val;
	Type* m_type;
public:
	ValueWrapper(Identifier id, llvm::Value* val, Type* t) : m_val(val), m_type(t), DuObject(id) {}
	bool isValueWrapper() const override { return true; }
	llvm::Value* getValue()
	{
		return m_val;
	}
	Type* getType()
	{
		return m_type;
	}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& c) const
	{
		return m_type ? m_type->getLLVMType(c) : nullptr;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const
	{
		return m_val;
	}
	Type* getDirectType()
	{
		Type* it = m_type;
		if(it)
		{
			while (PointerType* pt = dynamic_cast<PointerType*>(it))
			{
				it = pt->getPtrType();
			}
			return it;
		}
		else
			return nullptr;
	}
	Variable* generateVariable(llvm::IRBuilder<>& b)
	{
		Variable* var = new Variable("", m_type, nullptr, false);
		var->getLLVMType(b.getContext());
		LlvmBuilder::assigmentValue(b, var, m_val);
		return var;
	}
	Variable* generateVariableValAsAlloca(llvm::IRBuilder<>& b)
	{
		Variable* var = new Variable("", m_type, nullptr, false);
		var->getLLVMType(b.getContext());
		var->setAlloca(m_val);
		return var;
	}
	void setType(Type* type)
	{
		m_type = type;
	}

	virtual ~ValueWrapper() {}
};