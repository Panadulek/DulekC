#include "IfManager.h"
#include "LLvmBuilder.h"


void IfManager::merge(llvm::IRBuilder<>& b, Scope* ifs, std::map<std::shared_ptr<KeyType>, llvm::PHINode*>& map)
{
	if (!ifs)
		return;
	std::span view = ifs->getList();
	for (auto it : view)
	{
		if (Variable* var = dynamic_cast<Variable*>(it))
		{

			Variable* _var = static_cast<Variable*>(static_cast<Scope*>(ifs)->findUpperObject(var->getKey()));
			auto _it = map.find(_var->getKey());
			if ( _it != map.end())
			{
				llvm::Value* varval = LlvmBuilder::loadValue(b, var);
				_it->second->addIncoming(varval, ifs->getBasicBlock(b.getContext(), nullptr));
			}
			else
			{
				llvm::PHINode* n = LlvmBuilder::initPhiNode(b, var);
				map.insert({ _var->getKey(), n });
			}
			
		}
	}
}