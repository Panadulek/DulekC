#pragma once
#include "DuObject.h"
#include <cstdint>

class Value : public DuObject
{
public:
	Value() : DuObject(Identifier("Value")) {}
	virtual bool isNumericValue() const { return false;  }
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override { return nullptr; }
	virtual llvm::Type* getLLVMType(llvm::LLVMContext&) const override
	{
		assert(0);
		return nullptr;
	}
};

class NumericValue : public Value
{
	uint64_t m_value;
	bool m_isSigned = false;
public:
	NumericValue(const uint64_t i = 0) : Value()
	{
		m_value = i;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		return llvm::ConstantInt::get(type, m_value, m_isSigned);
	}
	void setSigned(bool flag) { m_isSigned = flag; }
	virtual bool isNumericValue() const { return true; }
};