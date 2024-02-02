#pragma once
#include "Type.h"
#include "Value.h"
#include "llvm/IR/IRBuilder.h"
#include "TypeContainer.h"
#include "LexerContext.h"
#include "MessageEngine.h"
class AstTree;
#define DECLARELLVM(X) mutable llvm::##X* m_llvm##X
extern void Error(MessageEngine::Code code, const char* additionalMsg);
extern void Warning(MessageEngine::Code code, const char* additionalMsg);
extern void Info(MessageEngine::Code code, const char* additionalMsg);
class Variable : public DuObject
{
	Type* m_type;
	Value* m_value;
	DECLARELLVM(Type);
	DECLARELLVM(Value);
	DECLARELLVM(AllocaInst);
	bool m_isGlobal;
	bool m_isTmp = false;
	bool m_hasBooleanValue;
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
		m_llvmType(nullptr), m_llvmValue(nullptr), m_llvmAllocaInst(nullptr), m_hasBooleanValue(false) {}
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
		if (m_llvmAllocaInst)
			return;
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
	bool isGlobalVariable() const
	{
		return m_isGlobal;
	}
	bool update(Variable* var, llvm::Value* newLLVMValue = nullptr)
	{
		if (typeid(var->getType()) == typeid(m_type))
		{	
			Value* val = var->getValue();
			delete m_value;
			m_value = nullptr;
			m_llvmValue = newLLVMValue;
			if (val)
			{
				m_value = val ? static_cast<Value*>(val->copy()) : var->getType()->getDefaultValue();
				m_value->setNewValue(newLLVMValue);
				return true;
			}
			else if (!m_value && !val && newLLVMValue)
			{
				m_llvmValue = newLLVMValue;
				return true;
			}
			else
				return false;
		}
	}
	void forceUpdateByLLVM(llvm::Value* val, llvm::Type* type)
	{
		m_llvmValue = val;
		m_llvmType = type;
	}

	bool updateByLLVM(llvm::Value* val, llvm::Type* type)
	{
		if ( type == getLLVMType(type->getContext()) )
		{
			m_llvmValue = val;
			m_llvmType = type;
			return true;
		}
		return false;
	}

	Type* getType()
	{
		return m_type;
	}
	Value* getValue()
	{
		return m_value;
	}
	virtual DuObject* copy() const override
	{
		auto variable = new Variable(getIdentifier(), m_type ? m_type : nullptr, m_value ? static_cast<Value*>(m_value->copy()) : nullptr, isGlobalVariable());
		if (m_hasBooleanValue)
			variable->setBooleanValue();
		if (m_llvmValue && m_llvmType)
		{
			variable->updateByLLVM(m_llvmValue, m_llvmType);
		}
		return variable;
	}
	void setTmp()
	{
		m_isTmp = true;
	}
	const bool isTmp() const
	{
		return m_isTmp;
	}

	void setBooleanValue()
	{
		m_hasBooleanValue = true;
		m_type = TypeContainer::instance().getType(Type::getName(Type::ID::BOOL));
		m_llvmType = nullptr;
	}

	bool isBooleanValue()
	{
		return m_hasBooleanValue;
	}
	llvm::Value* toBoolean(llvm::LLVMContext& c, llvm::IRBuilder<>& b)
	{
		if (m_hasBooleanValue)
			return getLLVMValue(getLLVMType(c));
		if (m_type->isSimpleNumericType())
		{
			return b.CreateICmpNE(getLLVMValue(getLLVMType(c)), b.getInt32(0), "int to bool");
		}
		Error(MessageEngine::Code::CannotConvertToBoolean, getIdentifier().getName().data());
		return nullptr;
	}
	virtual ~Variable() 
	{
		delete m_value;
	}
};


