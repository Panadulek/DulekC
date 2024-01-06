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
	std::string_view getName() const { return m_id; }
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
	Identifier m_id;
protected:
	DuObject* m_parent;
	DuObject(const Identifier& identfier) : m_id(identfier), m_parent(nullptr) {}

public:
	virtual bool isNumericValue() const { return false; }
	virtual bool isSimpleNumericType() const { return false; }
	virtual bool isVariable() const { return false; }
	virtual bool isFunction() const { return false; }
	virtual bool isStatement() const { return false;  }
	virtual bool isType() const  { return false; }
	virtual bool isScope() const { return false; }
	virtual bool isConstValue() const { return false; }
	std::string_view getName() { return m_id.getName(); }
	virtual llvm::Type* getLLVMType(llvm::LLVMContext&) const = 0;
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const = 0;
	const Identifier& getIdentifier() const { return m_id; }
	void setParent(DuObject* p)
	{
		assert(p->isScope());
		m_parent = p;
	}
	DuObject* getParent() { return m_parent; }
	virtual ~DuObject() {}


};

using DuPtr = DuObject*;
using weakDuPtr = std::weak_ptr<DuObject>;