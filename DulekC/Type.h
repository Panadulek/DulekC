#pragma once
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Type.h>
#include <string_view>
#include <llvm-c/Core.h>
#include <memory>
#include "DuObject.h"

enum class ObjectInByte : unsigned char
{
	BYTE,
	WORD,
	DWORD,
	QWORD,
};

class Type : public DuObject
{
	
public:
	Type(const Identifier& id) : DuObject(id) {};
	virtual bool isSimpleNumericType() const { return false; }
	virtual llvm::Type* getLLVMType(llvm::LLVMContext&) const override
	{
		return nullptr;
	};
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		assert(0);
		return nullptr;
	}
	virtual ~Type() {}
};


class SimpleNumericType final : public Type
{
private:
	ObjectInByte m_size;
	bool m_isSigned;
public:
	static Identifier generateId(ObjectInByte id, bool isSigned)
	{
		switch (id)
		{
		case ObjectInByte::BYTE:
			if (isSigned)
				return Identifier("i8");
			else
				return Identifier("u8");
		case ObjectInByte::WORD:
			if (isSigned)
				return Identifier("i16");
			else
				return Identifier("u16");
		case ObjectInByte::DWORD:
			if (isSigned)
				return Identifier("i32");
			else
				return Identifier("u32");
		case ObjectInByte::QWORD:
			if (isSigned)
				return Identifier("i64");
			else
				return Identifier("u64");
		}
	}
	virtual bool isSimpleNumericType() const override { return true; }
	SimpleNumericType(const Identifier& id, const ObjectInByte oib, const bool isSigned) : Type(id), m_size(oib), m_isSigned(isSigned) {}
	virtual llvm::Type* getLLVMType (llvm::LLVMContext&) const override;
	const bool isSigned() const { return m_isSigned; }
	~SimpleNumericType() {}
};