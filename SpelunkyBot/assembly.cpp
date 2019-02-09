#include "stdafx.h"
#include "assembly.h"

namespace assembly
{
	// Current process
	const HANDLE process = GetCurrentProcess();
	// Hooks
	std::map<std::string, address_t> foundRegions = std::map<std::string, address_t>();
	address_t getAddress(LPCSTR moduleName)
	{
		std::string mName(moduleName);
		if (foundRegions.count(mName) != 0)
		{
			return foundRegions.at(mName);
		}
		else
		{
			HMODULE hModule = GetModuleHandleA(mName.c_str());
			if (hModule == NULL) throw 1;
			MODULEINFO mInfo;
			GetModuleInformation(process, hModule, &mInfo, sizeof(mInfo));
			foundRegions.at(mName) = (address_t)mInfo.lpBaseOfDll;
			return (address_t)mInfo.lpBaseOfDll;
		}
	}
	// Memory Access
	buffer_t readMemory(address_t src, size_t nBytes, buffer_t buffer)
	{
		if (buffer == nullptr)
		{
			buffer = new BYTE[nBytes];
		}
		ReadProcessMemory(process, src, buffer, nBytes, NULL);
		return buffer;
	}
	void writeMemory(address_t dst, size_t nBytes, buffer_t buffer, bool unlock = false)
	{
		DWORD oldProtect;
		if (unlock)
		{
			VirtualProtectEx(process, dst, nBytes, PAGE_EXECUTE_READWRITE, &oldProtect);
		}
		WriteProcessMemory(process, dst, buffer, nBytes, NULL);
	}
	address_t allocMemory(size_t nBytes)
	{
		return (address_t)VirtualAllocEx(process, NULL, nBytes, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	}
	// Opcode generation
	buffer_t jumpOpCode(address_t src, address_t dst, buffer_t buffer)
	{
		if (buffer == nullptr)
		{
			buffer = new BYTE[5];
		}
		int diff = (int)dst - (int)(src + 5);
		buffer[0] = 0xE9;
		memcpy(buffer + 1, &diff, sizeof(int));
		return buffer;
	}
	buffer_t callOpCode(address_t src, address_t dst, buffer_t buffer)
	{
		if (buffer == nullptr)
		{
			buffer = new BYTE[5];
		}
		int diff = (int)dst - (int)(src + 5);
		buffer[0] = 0xE8;
		memcpy(buffer + 1, &diff, sizeof(int));
		return buffer;
	}
	buffer_t fixOpCode(address_t oldAddress, address_t newAddress, buffer_t opCode)
	{
		if (opCode[0] == 0xE8 || opCode[0] == 0xE9)
		{
			int offset;
			memcpy(&offset, opCode + 1, sizeof(int));
			offset = offset - ((int)newAddress - (int)oldAddress);
			memcpy(opCode + 1, &offset, sizeof(int));
		}
		return opCode;
	}
	// Injections
	Injection::Injection(address_t fLoc,
		address_t srcLoc,
		size_t sLen,
		size_t nArgs,
		register_t *args,
		int reloc
	) : functionLocation(fLoc),
		srcLocation(srcLoc),
		srcLength(sLen),
		nArgs(nArgs),
		args(args),
		relocation(reloc)
	{
		if (sLen < 5) throw 1;
		originalCode = new BYTE[srcLength];
		memcpy(originalCode, srcLocation, srcLength);
	}

	void Injection::activate()
	{
		if (active) return;
		if (injectionLocation == nullptr)
		{
			// Allocate and generate a function call
			size_t dstLength = 2 * nArgs + 12 + srcLength;
			injectionLocation = allocMemory(dstLength);
			// Create function call
			size_t offset = 0;
			// Run old code if location < 0
			if (relocation < 0)
			{
				memcpy(injectionLocation + offset, originalCode, srcLength);
				fixOpCode(srcLocation, injectionLocation + offset, injectionLocation + offset);
				offset += srcLength;
			}
			// Push all registers
			injectionLocation[offset++] = 0x60;
			// Push register arguments
			for (size_t i = 0; i < nArgs; i++)
			{
				injectionLocation[offset++] = 0x50 + args[nArgs - 1 - i];
			}
			// Call function
			callOpCode(injectionLocation + offset, functionLocation, injectionLocation + offset);
			offset += 5;
			// Pop register arguments
			for (size_t i = 0; i < nArgs; i++)
			{
				injectionLocation[offset++] = 0x58 + args[i];
			}
			// Pop all registers
			injectionLocation[offset++] = 0x61;
			// Run old code if location > 0
			if (relocation > 0)
			{
				memcpy(injectionLocation + offset, originalCode, srcLength);
				fixOpCode(srcLocation, injectionLocation + offset, injectionLocation + offset);
				offset += srcLength;
			}
			// Jump back
			jumpOpCode(injectionLocation + offset, srcLocation + srcLength, injectionLocation + offset);
		}
		// Replace original code with jump to allocated memory
		buffer_t injectCode = new BYTE[srcLength];
		jumpOpCode(srcLocation, injectionLocation, injectCode);
		for (size_t i = 5; i < srcLength; i++)
		{
			injectCode[i] = 0x90;
		}
		writeMemory(srcLocation, srcLength, injectCode, true);
	}
	void Injection::deactivate()
	{
		if (!active) return;
		writeMemory(srcLocation, srcLength, originalCode);
	}
}