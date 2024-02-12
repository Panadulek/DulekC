#pragma once
#include "DuObject.h"
#include <cstdint>
#include <iostream>
class Value : public DuObject
{
protected:
	Value(Identifier id) : DuObject(Identifier(id)) {}
public:
	
	virtual bool isNumericValue() const { return false;  }
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override { return nullptr; }
	virtual llvm::Type* getLLVMType(llvm::LLVMContext&) const override
	{
		assert(0);
		return nullptr;
	}
	virtual DuObject* copy() const override
	{
		assert(0);
		return nullptr;
	}
	virtual void setNewValue(llvm::Value*)
	{
		assert(0);
		return;
	}
	virtual llvm::Value* getLLVMValueOnStack(llvm::IRBuilder<>& builder, llvm::Type* type) const
	{
		assert(0);
		return nullptr;
	}
	virtual ~Value() {}
};

class NumericValue : public Value
{
	uint64_t m_value;
	bool m_isSigned = false;
public:
	NumericValue(const uint64_t i = 0) : Value(Identifier(std::to_string(i)))
	{
		m_value = i;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		return llvm::ConstantInt::get(type, m_value, m_isSigned);
	}
	virtual llvm::Value* getLLVMValueOnStack(llvm::IRBuilder<>& builder, llvm::Type* type) const override
	{
		llvm::Value* var = builder.CreateAlloca(type, nullptr, "numeric_value");
		llvm::Value* valueToStore = llvm::ConstantInt::get(type, m_value, m_isSigned);
		builder.CreateStore(valueToStore, var);
		llvm::Value* load = builder.CreateLoad(type, var);
		return load;
	}
	uint64_t getValue() const { return m_value; }
	void setSigned(bool flag) { m_isSigned = flag; }
	virtual bool isNumericValue() const { return true; }
	virtual DuObject* copy() const override
	{
		return new NumericValue(getValue());
	}
	virtual void setNewValue(llvm::Value* val) override
	{
		if (auto* constInt = llvm::dyn_cast<llvm::ConstantInt>(val)) {
			m_value = m_isSigned ? constInt->getSExtValue() : constInt->getZExtValue();
		}
		return;
	}
	virtual ~NumericValue() {}
};



