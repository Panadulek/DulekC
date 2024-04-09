#pragma once
#include "Variable.h"
#include <memory>
#include "AstTree.h"
namespace GeneratorTmpVariables
{
	static std::unique_ptr<Variable> generateI32Variable(Identifier _id, uint64_t val)
	{
		Identifier id = SimpleNumericType::generateId(ObjectInByte::DWORD, true);
		TypeContainer::instance().insert<SimpleNumericType>(id, id, ObjectInByte::DWORD, true);
		Type* type = TypeContainer::instance().getType(SimpleNumericType::generateId(ObjectInByte::DWORD, true));
		return std::make_unique<Variable>(_id, type, new NumericValue(val), AstTree::instance().inGlobal());
	}
	static std::unique_ptr<Variable> generateI64Variable(Identifier _id, uint64_t val)
	{
		Identifier id = SimpleNumericType::generateId(ObjectInByte::QWORD, true);
		TypeContainer::instance().insert<SimpleNumericType>(id, id, ObjectInByte::QWORD, true);
		Type* type = TypeContainer::instance().getType(SimpleNumericType::generateId(ObjectInByte::QWORD, true));
		return std::make_unique<Variable>(_id, type, new NumericValue(val), AstTree::instance().inGlobal());
	}
};




