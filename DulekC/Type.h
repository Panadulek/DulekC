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
#include <llvm/IR/IRBuilder.h>
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
	virtual bool isType() const override { return true; }
	virtual llvm::Type* getLLVMType(llvm::LLVMContext&) const override
	{
		return nullptr;
	};
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		assert(0);
		return nullptr;
	}
	virtual llvm::Value* convertValueBasedOnType(llvm::IRBuilder<>& builder, llvm::Value* value, llvm::Type* type, llvm::LLVMContext& context)
	{
		assert(0);
		return nullptr;
	}
	virtual size_t getSizeInBytes() const = 0;
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
	virtual size_t getSizeInBytes() const override
	{
		switch (m_size)
		{
		case ObjectInByte::BYTE:
			return sizeof(uint8_t);
		case ObjectInByte::WORD:
			return sizeof(uint16_t);
		case ObjectInByte::DWORD:
			return sizeof(uint32_t);
		case ObjectInByte::QWORD:
			return sizeof(uint64_t);
		}
	}
	virtual llvm::Value* convertValueBasedOnType(llvm::IRBuilder<>& builder, llvm::Value* value, llvm::Type* type, llvm::LLVMContext& context) override
	{
		auto myType = getLLVMType(context);

		if (type->isIntegerTy() && myType->isIntegerTy()) {
			const unsigned typeBits = type->getIntegerBitWidth();
			const unsigned myTypeBits = myType->getIntegerBitWidth();
			if (typeBits > myTypeBits) 
			{
				return builder.CreateTrunc(value, myType, "truncVal");
			}
			else if (typeBits < myTypeBits)
			{
				return builder.CreateZExtOrBitCast(value, myType, "extVal");
			}
			else if (typeBits == myTypeBits) 
			{
				return builder.CreateBitCast(value, myType, "bitcastVal");
			}
		}
		return value;

	}

	DuObject* copy() const override
	{
		return new SimpleNumericType(getIdentifier(), m_size, isSigned());
	}
	virtual ~SimpleNumericType() {}
};