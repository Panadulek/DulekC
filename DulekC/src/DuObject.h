#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <llvm/IR/Module.h>
#include <charconv>
#include <cstdint>
class Identifier
{
	const std::string m_id;
public:
	Identifier(const std::string& id) : m_id(id) 
	{}
	Identifier(const char* id) : m_id(id) {}
	std::string_view getName() const { return m_id; }
	void operator =(const Identifier& other)
	{
		const_cast<std::string&>(m_id) = other.m_id;
	}
	bool operator==(const Identifier& otherId) const
	{
		return m_id.compare(otherId.m_id) == 0;
	}
	const std::pair<bool, uint64_t> toNumber() const
	{
		uint64_t val;
		auto[ptr, errcode] = std::from_chars(m_id.data(), m_id.data() + m_id.size(), val);
		return { errcode == std::errc() && ptr == (m_id.data() + m_id.size()), val };
	}
};


class DuObject
{
public:
	using KeyType = int;
private:

	Identifier m_id;
protected:
	
	DuObject* m_parent;
	mutable std::shared_ptr<KeyType> m_key;
	bool m_isCopy;
	DuObject(const Identifier& identfier) : m_id(identfier), m_parent(nullptr), m_isCopy(false)
	{
		m_key = std::make_shared<KeyType>(1);
	}

public:
	virtual bool isNumericValue() const { return false; }
	virtual bool isSimpleNumericType() const { return false; }
	virtual bool isVariable() const { return false; }
	virtual bool isFunction() const { return false; }
	virtual bool isStatement() const { return false;  }
	virtual bool isType() const  { return false; }
	virtual bool isScope() const { return false; }
	virtual bool isConstValue() const { return false; }
	virtual bool isIfScope() const { return false;  }
	void setIdentifier(const Identifier id)
	{
		m_id = id;
	}
	std::string_view getName() { return m_id.getName(); }
	virtual llvm::Type* getLLVMType(llvm::LLVMContext&) const
	{
		assert(0);
		return nullptr;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const
	{
		assert(0);
		return nullptr;
	}
	virtual DuObject* copy() const
	{
		assert(0);
		return nullptr;
	}
	virtual DuObject* getObject()
	{
		return this;
	}
	const Identifier& getIdentifier() const { return m_id; }
	virtual void setParent(DuObject* p)
	{
		assert(p->isScope());
		m_parent = p;
	}
	DuObject* getParent() { return m_parent; }
	virtual std::shared_ptr<KeyType>getKey() const
	{
		return m_key;
	}
	void setKey(std::shared_ptr<KeyType>key) const
	{
		m_key = key;
	}
	virtual ~DuObject() {}
	void setCopy()
	{
		m_isCopy = true;
	}
	bool isCopy()
	{
		return m_isCopy;
	}
	virtual bool isValueWrapper() const
	{
		return false;
	}
};

using DuPtr = DuObject*;
using weakDuPtr = std::weak_ptr<DuObject>;

extern DuObject* s_GlobalScope;