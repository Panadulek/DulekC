#pragma once
#include <map>
#include "Type.h"
#include <memory>
class TypeContainer
{
	bool m_isInited = false;
	struct TypeContainerHash
	{
		size_t operator()(const Identifier& id)const {
			std::hash<std::string>h;
			return h(id.getName().data());
		}
	};
	using TypeMap = std::unordered_map < Identifier, std::unique_ptr<Type>, TypeContainerHash>;
	TypeMap m_typeMap;
	
public:
	using Iterator = TypeMap::iterator;
	template<typename T, typename... Args>
	void insert(const Identifier id, Args&&... args )
	{
		static_assert(std::is_base_of<Type, T>::value, "T must be a type derived from Type");
		auto it = m_typeMap.find(id);
		if (it != m_typeMap.end())
			return;
		m_typeMap.insert({ id, std::make_unique<T>(std::forward<Args>(args)...) });
	}

	static std::string generatePointerType(std::vector<Type::ID> ids)
	{
		std::string name = "pointer<";
		Type::ID id = ids[0];
		ids.erase(ids.begin());
		if (id == Type::ID::POINTER)
		{
			name += generatePointerType(ids);
		}
		else
		{
			name += Type::getName(id);
		}
		name += ">";
		return name;
	}

	void init()
	{
		if (m_isInited)
			return;
		m_isInited = true;
		insert<SimpleNumericType>(Identifier(Type::getName(Type::ID::BOOL)), Identifier(Type::getName(Type::ID::BOOL)), ObjectInByte::BOOLEAN, false);

		insert<SimpleNumericType>(Type::getName(Type::ID::U8), Identifier(Type::getName(Type::ID::U8)), ObjectInByte::BYTE, false);
		insert<SimpleNumericType>(Type::getName(Type::ID::U16), Identifier(Type::getName(Type::ID::U16)), ObjectInByte::WORD, false);
		insert<SimpleNumericType>(Type::getName(Type::ID::U32), Identifier(Type::getName(Type::ID::U32)), ObjectInByte::DWORD, false);
		insert<SimpleNumericType>(Type::getName(Type::ID::U64), Identifier(Type::getName(Type::ID::U64)), ObjectInByte::QWORD, false);

		insert<SimpleNumericType>(Identifier(Type::getName(Type::ID::I8)), Identifier(Type::getName(Type::ID::I8)), ObjectInByte::BYTE, true);
		insert<SimpleNumericType>(Identifier(Type::getName(Type::ID::I16)), Identifier(Type::getName(Type::ID::I16)), ObjectInByte::WORD, true);
		insert<SimpleNumericType>(Identifier(Type::getName(Type::ID::I32)), Identifier(Type::getName(Type::ID::I32)), ObjectInByte::DWORD, true);
		insert<SimpleNumericType>(Identifier(Type::getName(Type::ID::I64)), Identifier(Type::getName(Type::ID::I64)), ObjectInByte::QWORD, true);
		insert<PointerType>(generatePointerType({ Type::ID::U8 }), getType(Type::getName(Type::ID::U8)));
		

	}

	Type* getType(const Identifier id)
	{
		auto ret = m_typeMap.find(id);
		if (ret != m_typeMap.end())
			return ret->second.get();
		else
			return nullptr;
	}
	static TypeContainer& instance()
	{
		static TypeContainer _tc;
		_tc.init();
		return _tc;
	}

};

