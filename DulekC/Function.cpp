#include "AstTree.h"

#include "Scope.h"
llvm::Function* Function::getLLVMFunction(llvm::LLVMContext& context, llvm::Module* m, llvm::IRBuilder<>& b)
{
	if (!m_llvmFunction)
	{
		m_llvmFunction = llvm::Function::Create(getFunctionType(context), llvm::Function::ExternalLinkage, getIdentifier().getName().data(), m);
		b.SetInsertPoint(getBasicBlock(context, m_llvmFunction));
		if (!m_args.empty())
		{
			if (m_llvmFunction->arg_size() == m_args.size())
			{
				for (size_t i = 0; i < m_args.size(); i++)
				{
					auto arg = m_llvmFunction->getArg(i);
					Variable* v = static_cast<Variable*>(AstTree::instance().findObject(m_args[i]));
					v->getLLVMValueOnStack(b, v->getLLVMType(context));
					v->init(b.CreateAlloca(v->getLLVMType(context), nullptr, v->getIdentifier().getName()), b);
					auto ret = b.CreateStore(arg, v->getAlloca());
					Variable* _arg = new Variable(Identifier(""), m_typesArgs[i], m_typesArgs[i]->convertLLVMToValue(arg), false);
					v->updateByLLVM(ret->getValueOperand(), ret->getValueOperand()->getType());
					delete _arg;
					v->setParent(this);
				}
			}
			else
			{
				printf("nie zgadzaja sie argumenty");
				exit(20);
			}
		}
	}
	return m_llvmFunction;
}