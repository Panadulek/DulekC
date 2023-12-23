#pragma once
#include "DuObject.h"
#include <list>
class Specializator : public DuObject
{
	using SpecializedArgs = std::list<DuPtr>*;
	using Iterator = std::list<DuPtr>::iterator;
	SpecializedArgs m_specializedArgs;
	Iterator m_currentMember;
	

public:
	Specializator(SpecializedArgs specializedArgs) : DuObject("Specializator"), m_specializedArgs(specializedArgs), m_currentMember(specializedArgs->begin()) {}
	DuPtr next()
	{
		if (m_currentMember == m_specializedArgs->end())
			return nullptr;
		DuPtr ret = *m_currentMember;
		m_currentMember++;
		return ret;
	}

};