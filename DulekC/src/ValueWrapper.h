#pragma once
#include "DuObject.h"
#include "Type.h"

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

	virtual ~ValueWrapper() {}
};