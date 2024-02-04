#pragma once
#include "Expression.h"
#include "AstTree.h"
#include "Scope.h"
#include <string_view>
class IfScope : public Scope
{
	Expression* m_expr;
	llvm::BasicBlock* m_mergeBlock;
	llvm::BasicBlock* m_elseBlock;
	class ElseSplitter : public DuObject
	{
	public:
		ElseSplitter() : DuObject(Identifier("begin else")) {}
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
	};

	void updatePhi(std::map<std::shared_ptr<KeyType>, llvm::PHINode*>&phiMap, Function* parent)
	{
		if (!parent)
			return;
		for (auto it : phiMap)
		{
			DuObject* obj = findObject(it.first);
			DuObject* var = parent->findObject(obj->getObject()->getIdentifier());
			var->updateByLLVM(it.second, it.second->getType());
		}
	}
	void defaultCopiedValues(Function* parent, const size_t maxOrginalSize)
	{
		for (auto it = m_childs.begin() + maxOrginalSize; it != m_childs.end(); ++it )
		{
			DuObject*& _it = *it;
			_it->getLLVMValue(nullptr)->print(llvm::outs());
			*it = parent->findObject(_it->getIdentifier());
			(*it)->getLLVMValue(nullptr)->print(llvm::outs());
		}
	}
public:
	IfScope(Expression* expr) : Scope(Identifier("If")), m_expr(expr),  m_mergeBlock(nullptr), m_elseBlock(nullptr)
	{
	}
	virtual llvm::Type* getLLVMType(llvm::LLVMContext& context) const override
	{
		assert(0);
		return 0;
	}
	virtual llvm::Value* getLLVMValue(llvm::Type* type) const override
	{
		assert(0);
		return nullptr;
	}

	void generateLLVMIf(llvm::LLVMContext& c, llvm::Module* m, llvm::IRBuilder<>& b, std::function<void(DuObject*, Scope*)> m_cb) 
	{
		DuObject* it = this;
		bool isParentGlobal = false;
		auto& tree = AstTree::instance();	
		Function* parentFun;
		while (!it->isFunction())
		{
			if (tree.isGlobal(it))
			{
				isParentGlobal = true;
				break;
			}
			it = it->getParent();
		}
		parentFun = static_cast<Function*>(it);
		if (!isParentGlobal)
		{
			bool ifReturn = false;
			bool elseReturn = false;
			bool isElse = false;
			Function* fnc = static_cast<Function*>(it);
			llvm::Function* llvmfnc = fnc->getLLVMFunction(c, m, b);
			llvm::BasicBlock* then = getBasicBlock(c, llvmfnc);
			m_expr->processExpression(m, b, c, false);
			auto variable = m_expr->getRes();
			if (!variable)
			{
				assert(0);         
			}
			llvm::Value* condition = variable->toBoolean(c, b);
			bool foundElseSplitter = false;
			for (auto it : m_childs)
			{
				if (dynamic_cast<ElseSplitter*>(it))
				{
					foundElseSplitter = true;
				}
				if (it && it->isStatement() && static_cast<ReturnStatement*>(it)->isReturnStatement())
				{
					if (!foundElseSplitter)
						ifReturn = true;
					else
						elseReturn = true;
				}
			}
			const bool isNeedMergeBlock = !(elseReturn && ifReturn);
			if (!m_elseBlock && foundElseSplitter)
				m_elseBlock = llvm::BasicBlock::Create(c, "else_block", llvmfnc);
			if (!m_mergeBlock && isNeedMergeBlock)
				m_mergeBlock = llvm::BasicBlock::Create(c, "merge_block", llvmfnc);
			if(isNeedMergeBlock)
				b.CreateCondBr(condition, then, m_elseBlock ? m_elseBlock : m_mergeBlock);
			else
			{
				b.CreateCondBr(condition, then, m_elseBlock);
			}
			std::map<std::shared_ptr<KeyType>, llvm::PHINode*> m_phiMap;
			std::map<std::shared_ptr<KeyType>, DuObject*> orginalMap;
			b.SetInsertPoint(then);
			for (auto i = 0; i < m_childs.size(); i++)
			{
				DuObject* it = m_childs[i];
				if (dynamic_cast<ElseSplitter*>(it) && !isElse)
				{
					b.CreateBr(m_mergeBlock);
					isElse = true;
					b.SetInsertPoint(m_elseBlock);
					for (auto mapIt : orginalMap)
					{
						DuObject* fo = findObject(mapIt.second->getIdentifier());
						llvm::Value* val = mapIt.second->getLLVMValue(nullptr);
						llvm::Type* type = mapIt.second->getLLVMType(b.getContext());
						fo->updateByLLVM(val, type);
					}
					continue;
				}


				DuObject* child = findObject(it->getObject()->getIdentifier());
				if(!child)
				{
					child = AstTree::instance().findObject(it->getObject()->getIdentifier());
				
					orginalMap.insert({ it->getObject()->getKey(), child});
					child = nullptr;
				}
				m_cb(it, this);
				if (!child)
				{
					addChild(it->getObject());
					Identifier id = it->getObject()->getIdentifier();
					it->getLLVMValue(nullptr)->print(llvm::outs());
					orginalMap[it->getObject()->getKey()]->getLLVMValue(nullptr)->print(llvm::outs());	
				}
				else
				{
					DuObject* ptr = it->getObject();
					findObject(it->getObject()->getIdentifier())->updateByLLVM(it->getObject()->getLLVMValue(nullptr), ptr->getLLVMType(b.getContext()));
				}
				if (isNeedMergeBlock)
				{
					auto found = m_phiMap.find(it->getKey());
					if (found != m_phiMap.end())
					{
						llvm::PHINode*& node = found->second;

						size_t numIncomingValues = node->getNumIncomingValues();
						if (numIncomingValues == 1 && b.GetInsertBlock() == then)
						{
							node->eraseFromParent();
							llvm::BasicBlock* block = b.GetInsertBlock();
							b.SetInsertPoint(m_mergeBlock);
							m_phiMap.erase(it->getKey());
							m_phiMap.insert({ it->getKey(), b.CreatePHI(llvm::Type::getInt32Ty(b.getContext()), 2, "phi")});
							b.SetInsertPoint(block);
						}
						else if (numIncomingValues == 2)
						{
							llvm::BasicBlock* thenBlock = node->getIncomingBlock(0);
							llvm::Value* thenValue = node->getIncomingValue(0);
							llvm::BasicBlock* block = b.GetInsertBlock();
							node->eraseFromParent();
							b.SetInsertPoint(m_mergeBlock);
							m_phiMap.erase(it->getKey());
							m_phiMap.insert({ it->getKey(), b.CreatePHI(llvm::Type::getInt32Ty(b.getContext()), 2, "phi")});
							b.SetInsertPoint(block);
							m_phiMap[it->getKey()]->addIncoming(thenValue, thenBlock);

						}
						it->getLLVMValue(nullptr)->print(llvm::outs());
						m_phiMap[it->getKey()]->addIncoming(it->getLLVMValue(nullptr), b.GetInsertBlock());
					}
					else
					{
						llvm::BasicBlock* block = b.GetInsertBlock();
						b.SetInsertPoint(m_mergeBlock);
						m_phiMap.insert({ it->getKey(), b.CreatePHI(llvm::Type::getInt32Ty(b.getContext()), 2, "phi")});
						b.SetInsertPoint(block);
						m_phiMap[it->getKey()]->addIncoming(it->getLLVMValue(nullptr), b.GetInsertBlock());
					}

				}
			}
			updatePhi(m_phiMap, parentFun);
			if (isNeedMergeBlock)
			{
				b.CreateBr(m_mergeBlock);
				b.SetInsertPoint(m_mergeBlock);
			}
		}
	}
	virtual llvm::BasicBlock* getBasicBlock(llvm::LLVMContext& context, llvm::Function* fn) override
	{
		if (!m_llvmBlock)
		{
			m_llvmBlock = llvm::BasicBlock::Create(context, "then", fn);
		}
		return m_llvmBlock;
	}
	virtual bool isIfScope() const override { return true; }
	Function* getFunctionParent()
	{
		DuObject* ret = this;
		if (!ret->isFunction())
		{
			ret = ret->getParent();
			if (!ret)
				assert(0);
		}
		return static_cast<Function*>(ret);
	}
	~IfScope()
	{
		delete m_expr;
	}

	void beginElse()
	{
		addChild(new ElseSplitter());
	}

};