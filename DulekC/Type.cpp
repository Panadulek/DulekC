#include "Type.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Type.h>
#include <string_view>
#include <llvm-c/Core.h>
#include <memory>

#include "DuObject.h"

namespace LLVM_GEN
{
	class LLVMSimpleNumericType
	{
		LLVMSimpleNumericType() {}
		llvm::Type* genType(const ObjectInByte size, bool isSigned, llvm::LLVMContext& context)
		{
			switch (size)
			{
				case ObjectInByte::BYTE:
					return llvm::Type::getInt8Ty(context);
				case ObjectInByte::WORD:
					return llvm::Type::getInt16Ty(context);
				case ObjectInByte::DWORD:
					return llvm::Type::getInt32Ty(context);
				case ObjectInByte::QWORD:
					return llvm::Type::getInt64Ty(context);
			}
			assert(0);
			return nullptr;
		}
	public:
		friend class SimpleNumericType;
	};
}
llvm::Type* SimpleNumericType::getLLVMType(llvm::LLVMContext& context)  const
{
	LLVM_GEN::LLVMSimpleNumericType l;
	return l.genType(m_size, m_isSigned, context);
}
