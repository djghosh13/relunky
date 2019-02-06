// SpelunkyMemory.cpp: This file handles writing and reading from the program memory

// #include "pch.h"
#include "stdafx.h"
#include "Spelunky.h"

Spelunky::Spelunky() : process(GetCurrentProcess()), baseAddress(NULL), BASE(baseAddress)
{
	findHooks();
	isConnected();
}

Spelunky::~Spelunky()
{
	CloseHandle(process);
}

void Spelunky::findHooks()
{
	Address hook = NULL;
	MEMORY_BASIC_INFORMATION memInfo;

	for (int i = 0; i < 1000; i++)
	{
		VirtualQueryEx(process, hook, &memInfo, sizeof(memInfo));

		if (memInfo.AllocationProtect == PAGE_NOACCESS)
		{
			hook = (Address)memInfo.BaseAddress + memInfo.RegionSize;
		}
		else
		{
			baseAddress = hook;
			if (isConnected()) break;
			hook = hook + 0x00010000;
		}
	}
}

bool Spelunky::isConnected()
{
	connected = true;

	BYTE *mem = read_memory(baseAddress + 0x1000, 8, NULL);
	BYTE *expected = new BYTE[8]{ 0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xC0, 0xD9, 0x45 };
	if (memcmp(mem, expected, 8) != 0)
		connected = false;

	return connected;
}

void Spelunky::setBaseAddress(Address newAddress)
{
	baseAddress = newAddress;
	if (newAddress == nullptr)
		connected = false;
	else isConnected();
}

BYTE *Spelunky::read_memory(Address from, size_t len, BYTE *readTo)
{
	if (!connected) return NULL;
	if (readTo == nullptr)
	{
		readTo = new BYTE[len];
	}

	ReadProcessMemory(process, from, readTo, len, NULL);

	return readTo;
}

void Spelunky::write_memory(Address to, size_t len, BYTE *readFrom, bool unlock)
{
	if (!connected) return;
	DWORD oldProtect;
	if (unlock)
	{
		VirtualProtectEx(process, to, len, PAGE_EXECUTE_READWRITE, &oldProtect);
	}
	WriteProcessMemory(process, to, readFrom, len, NULL);
}

Address Spelunky::alloc_memory(size_t minSize)
{
	if (!connected) return NULL;
	return (Address)VirtualAllocEx(process, NULL, minSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

void Spelunky::inject_call(Address from, size_t fromSize, FunctionCall fncall, char codeLocation)
{
	if (!connected) return;
	if (fromSize < 5) return;

	size_t N = fncall.nArgs;
	size_t toSize = 1 + N + 5 + N + 1 + 5 + (codeLocation == 0 ? 0 : fromSize) + (fncall.baseParam ? 6 : 0);
	Address callCode = alloc_memory(toSize);
	BYTE *jumpToCode = new BYTE[fromSize];
	BYTE *jumpBackCode = new BYTE[toSize];

	// Create jump to allocated memory
	jmp_code(from, callCode, jumpToCode);
	for (size_t i = 5; i < fromSize; i++)
	{
		jumpToCode[i] = 0x90;
	}

	// Create function call
	size_t index = 0;
	// Run old code if location == -1
	if (codeLocation == -1)
	{
		read_memory(from, fromSize, jumpBackCode + index);
		index += fromSize;
	}
	// Push all registers
	jumpBackCode[index++] = 0x60;
	// Push register arguments
	for (size_t i = 0; i < N; i++)
	{
		jumpBackCode[index++] = 0x50 + fncall.args[N - i - 1];
	}
	// Push base address if required
	if (fncall.baseParam)
	{
		jumpBackCode[index++] = 0x68;
		memcpy(jumpBackCode + index, &baseAddress, sizeof(PDWORD));
		index += sizeof(PDWORD);
	}
	// Call function
	call_code(callCode + index, fncall.function, jumpBackCode + index);
	index += 5;
	// Pop base address if required
	if (fncall.baseParam)
	{
		jumpBackCode[index++] = 0x58;
	}
	// Pop register arguments
	for (size_t i = 0; i < N; i++)
	{
		jumpBackCode[index++] = 0x58 + fncall.args[i];
	}
	// Pop all registers
	jumpBackCode[index++] = 0x61;
	// Run old code if location == 1
	if (codeLocation == 1)
	{
		read_memory(from, fromSize, jumpBackCode + index);
		index += fromSize;
	}
	// Jump back
	jmp_code(callCode + toSize - 5, from + fromSize, jumpBackCode + index);

	write_memory(from, fromSize, jumpToCode, true);
	write_memory(callCode, toSize, jumpBackCode);
}

// Other functions

BYTE *jmp_code(Address from, Address to, BYTE *writeTo)
{
	if (writeTo == nullptr)
	{
		writeTo = new BYTE[5];
	}
	writeTo[0] = 0xE9; // Jump near, relative
	int diff = to - (from + 5);
	memcpy(writeTo + 1, &diff, sizeof(int));
	return writeTo;
}

BYTE *call_code(Address from, Address to, BYTE *writeTo)
{
	if (writeTo == nullptr)
	{
		writeTo = new BYTE[5];
	}
	writeTo[0] = 0xE8; // Call near, relative
	int diff = to - (from + 5);
	memcpy(writeTo + 1, &diff, sizeof(int));
	return writeTo;
}