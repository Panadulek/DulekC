#pragma once

#include "DuObject.h"
#include "AstTree.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include "GenTmpVariables.h"
class Expression : public DuObject
{
	llvm::Value* m_res;
protected:
	Expression(Identifier id) : DuObject(id), m_res(nullptr) {}
	void setRes(llvm::Value* res)
	{
		m_res = res;
	}
public:
	virtual void processExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context) = 0;
	virtual llvm::Type* getLLVMType(llvm::LLVMContext&) 
	{
		assert(0);
		return nullptr;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const
	{
		return m_res;
	}
};



class MatematicalExpression : public Expression
{
	Identifier m_l;
	Identifier m_r;
	bool m_isLNumber;
	bool m_isRNumber;
	bool m_signed;
public:
	MatematicalExpression(Identifier op, Identifier l, Identifier r, bool s) : Expression(op),  m_l(l), m_r(r), m_isLNumber(false), m_isRNumber(false), m_signed(s)
	{
		auto [isLNumber, lValue] = l.toNumber();
		auto [isRNumber, isLValue] = r.toNumber();
		auto& tree = AstTree::instance();
		if (!isLNumber)
		{
			auto foundObject = tree.findObject(m_l);
			if (!foundObject)
			{
				std::cout << "nie znaleziono l obiektu" << std::endl;
				assert(0);
			}
		}
		if (!isRNumber)
		{
			auto foundObject = tree.findObject(m_r);
			if (!foundObject)
			{
				std::cout << "nie znaleziono r obiektu" << std::endl;
				assert(0);
			}
		}

		m_isLNumber = isLNumber;
		m_isRNumber = isRNumber;
	}

	virtual void processExpression(llvm::Module* module, llvm::IRBuilder<>& builder, llvm::LLVMContext& context) override
	{
		std::unique_ptr<DuObject> m_objs[2]{ nullptr, nullptr };
		auto& tree = AstTree::instance();
		if (m_isLNumber)
			m_objs[0] = GeneratorTmpVariables::generateI32Variable(m_l, m_l.toNumber().second);
		if(m_isRNumber)
			m_objs[1] = GeneratorTmpVariables::generateI32Variable(m_r, m_r.toNumber().second);
		

		DuObject* obj[2]{ nullptr, nullptr };
		if (!m_objs[0])
		{
			obj[0] = tree.findObject(m_l);
		}
		else
			obj[0] = m_objs[0].get();

		if (!m_objs[1])
		{
			obj[1] = tree.findObject(m_r);
		}
		else
			obj[1] = m_objs[1].get();

		if (!obj[0] || !obj[1])
		{
			std::cout << "nie znaleziono prawej strony wartosci " << std::endl;
			assert(0);
		}

		std::string_view view = getIdentifier().getName();
		if (!view.compare("+"))
		{
			setRes(builder.CreateAdd(obj[0]->getLLVMValue(obj[0]->getLLVMType(context)), obj[1]->getLLVMValue(obj[1]->getLLVMType(context))));
		}
		else if (!view.compare("-"))
		{
			setRes(builder.CreateSub(obj[0]->getLLVMValue(obj[0]->getLLVMType(context)), obj[1]->getLLVMValue(obj[1]->getLLVMType(context))));
		}
		else if (!view.compare("*"))
		{
			setRes(builder.CreateMul(obj[0]->getLLVMValue(obj[0]->getLLVMType(context)), obj[1]->getLLVMValue(obj[1]->getLLVMType(context))));
		}
		else if (!view.compare("/"))
		{
			if(m_signed)
				setRes(builder.CreateSDiv(obj[0]->getLLVMValue(obj[0]->getLLVMType(context)), obj[1]->getLLVMValue(obj[1]->getLLVMType(context))));
			else
				setRes(builder.CreateUDiv(obj[0]->getLLVMValue(obj[0]->getLLVMType(context)), obj[1]->getLLVMValue(obj[1]->getLLVMType(context))));
		}
		else
		{
			setRes(nullptr);
		}
	}

	
};