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
#include "Value.h"
enum class ObjectInByte : unsigned char
{
	BOOLEAN,
	POINTER,
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
	virtual Value* getDefaultValue() const = 0;
	virtual Value* convertLLVMToValue(llvm::Value* lv) const = 0;
	virtual llvm::Type* getLLVMType(llvm::LLVMContext&) const override
	{
		return nullptr;
	};

	static Identifier generateId(ObjectInByte id, bool isSigned)
	{
		switch (id)
		{
		case ObjectInByte::BOOLEAN:
			return Identifier(Type::getName(Type::ID::BOOL));
		case ObjectInByte::BYTE:
			if (isSigned)
				return Identifier(Type::getName(Type::ID::I8));
			else
				return Identifier(Type::getName(Type::ID::U8));
		case ObjectInByte::WORD:
			if (isSigned)
				return Identifier(Type::getName(Type::ID::I16));
			else
				return Identifier(Type::getName(Type::ID::U16));
		case ObjectInByte::DWORD:
			if (isSigned)
				return Identifier(Type::getName(Type::ID::I32));
			else
				return Identifier(Type::getName(Type::ID::U32));
		case ObjectInByte::QWORD:
			if (isSigned)
				return Identifier(Type::getName(Type::ID::I64));
			else
				return Identifier(Type::getName(Type::ID::U64));
		case ObjectInByte::POINTER:
			return Identifier(Type::getName(Type::ID::POINTER));
		default:
			return Identifier("");
		}
	}

	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		assert(0);
		return nullptr;
	}
	virtual llvm::Value* convertValueBasedOnType(llvm::IRBuilder<>& builder, llvm::Value* value, llvm::Type* type, llvm::LLVMContext& context)
	{
		return nullptr;
	}
	virtual size_t getSizeInBytes() const = 0;
	virtual ~Type() {}

	enum ID
	{
		BOOL = 0,
		POINTER,
		I8,
		U8,
		I16,
		U16,
		I32,
		U32,
		I64,
		U64,
		END_TYPE
	};

	static const char* getName(Type::ID id)
	{
		switch (id)
		{
		case ID::POINTER:
			return "pointer";
		case ID::BOOL:
			return "bool";
		
		case ID::U8:
			return "u8";
		case ID::U16:
			return "u16";
		case ID::U32:
			return "u32";
		case ID::U64:
			return "u64";

		case ID::I8:
			return "i8";
		case ID::I16:
			return "i16";
		case ID::I32:
			return "i32";
		case ID::I64:
			return "i64";
		default:
			return nullptr;
		}
	}

};


class SimpleNumericType final : public Type
{
private:
	ObjectInByte m_size;
	bool m_isSigned;
public:

	virtual bool isSimpleNumericType() const override { return true; }
	SimpleNumericType(const Identifier& id, const ObjectInByte oib, const bool isSigned) : Type(id), m_size(oib), m_isSigned(isSigned) {}
	virtual llvm::Type* getLLVMType (llvm::LLVMContext&) const override;
	const bool isSigned() const { return m_isSigned; }
	virtual size_t getSizeInBytes() const override
	{
		switch (m_size)
		{
		case ObjectInByte::BOOLEAN:
		case ObjectInByte::BYTE:
			return sizeof(uint8_t);
		case ObjectInByte::WORD:
			return sizeof(uint16_t);
		case ObjectInByte::DWORD:
			return sizeof(uint32_t);
		case ObjectInByte::POINTER:
		case ObjectInByte::QWORD:
			return sizeof(uint64_t);
		default:
			return 0;
		}
	}
	ObjectInByte getObjectInByte()
	{
		return m_size;
	}
	virtual llvm::Value* convertValueBasedOnType(llvm::IRBuilder<>& builder, llvm::Value* value, llvm::Type* type, llvm::LLVMContext& context) override
	{
		auto myType = getLLVMType(context);

		if ( type->isIntegerTy() && myType->isIntegerTy() && type != myType) 
		{
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
	virtual Value* getDefaultValue() const override
	{
		return new NumericValue();
	}
	virtual Value* convertLLVMToValue(llvm::Value* lv) const override
	{
		return nullptr;
	}
	DuObject* copy() const override
	{
		return new SimpleNumericType(getIdentifier(), m_size, isSigned());
	}
	virtual ~SimpleNumericType() {}
};



class PointerType : public Type
{
private:
	class Iterator
	{
		Type* m_curr;
	public:
		Iterator(Type* begin) : m_curr(begin) {}
		Iterator& getNext()
		{
			if (PointerType* pt = dynamic_cast<PointerType*>(m_curr))
			{
				m_curr = pt->getPtrType();
			}
			else
				m_curr = nullptr;
			return *this;
		}
		bool isEnd()
		{
			return m_curr == nullptr;
		}
		Type*& getValue()
		{
			return m_curr;
		}

	};
private:
	Type* m_ptrType;
public:
	using PtrIterator = Iterator;
	PointerType(Type* ptrType) : m_ptrType(ptrType), Type("")
	{
		setIdentifier(getTypeName());
	}
	PointerType(Identifier id, Type* ptrType) : Type(id), m_ptrType(ptrType) {}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext&) const override;
	const Identifier getTypeName() const;
	virtual Value* getDefaultValue() const override
	{
		return nullptr;
	}
	virtual Value* convertLLVMToValue(llvm::Value* lv) const override
	{
		return nullptr;
	}
	DuObject* copy() const override
	{
		return nullptr;
	}
	virtual size_t getSizeInBytes() const override
	{
		return sizeof(void*);
	}
	Type* getPtrType()
	{
		return m_ptrType;
	}

	PtrIterator begin()
	{
		return PtrIterator(m_ptrType);
	}
	virtual ~PointerType() {}
};