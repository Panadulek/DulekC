#pragma once
#include <map>
#include "Type.h"
#include <memory>
class TypeContainer
{
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
		m_typeMap[id] = std::make_unique<T>(std::forward<Args>(args)...);
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
		return _tc;
	}

};

