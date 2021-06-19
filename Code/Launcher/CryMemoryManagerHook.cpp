#include <malloc.h>  // _msize
#include <stdlib.h>
#include <string.h>

#include "DLL.h"
#include "System.h"

#include "CryMemoryManagerHook.h"

namespace
{
	void *CryMalloc_hook(size_t size, size_t & allocatedSize)
	{
		void *result = NULL;

		if (size)
		{
			result = malloc(size);
		}

		allocatedSize = size;

		return result;
	}

	void *CryRealloc_hook(void *mem, size_t size, size_t & allocatedSize)
	{
		void *result = NULL;

		if (size)
		{
			result = realloc(mem, size);
		}
		else
		{
			if (mem)
			{
				free(mem);
			}
		}

		allocatedSize = size;

		return result;
	}

	size_t CryFree_hook(void *mem)
	{
		size_t size = 0;

		if (mem)
		{
			size = _msize(mem);

			free(mem);
		}

		return size;
	}

	void *CrySystemCrtMalloc_hook(size_t size)
	{
		void *result = NULL;

		if (size)
		{
			result = malloc(size);
		}

		return result;
	}

	void CrySystemCrtFree_hook(void *mem)
	{
		if (mem)
		{
			free(mem);
		}
	}

	void Hook(void *pFunc, void *pNewFunc)
	{
		if (!pFunc)
		{
			return;
		}

	#ifdef BUILD_64BIT
		unsigned char code[] = {
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov rax, 0x0
			0xFF, 0xE0                                                   // jmp rax
		};

		memcpy(&code[2], &pNewFunc, 8);
	#else
		unsigned char code[] = {
			0xB8, 0x00, 0x00, 0x00, 0x00,  // mov eax, 0x0
			0xFF, 0xE0                     // jmp eax
		};

		memcpy(&code[1], &pNewFunc, 4);
	#endif

		System::FillMem(pFunc, code, sizeof code);
	}
}

void CryMemoryManagerHook::Init(const DLL & CrySystem)
{
	Hook(CrySystem.getSymbolAddress("CryMalloc"), CryMalloc_hook);
	Hook(CrySystem.getSymbolAddress("CryRealloc"), CryRealloc_hook);
	Hook(CrySystem.getSymbolAddress("CryFree"), CryFree_hook);
	Hook(CrySystem.getSymbolAddress("CrySystemCrtMalloc"), CrySystemCrtMalloc_hook);
	Hook(CrySystem.getSymbolAddress("CrySystemCrtFree"), CrySystemCrtFree_hook);
}
