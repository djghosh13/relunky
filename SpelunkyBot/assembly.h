#pragma once

#include "stdafx.h"

namespace assembly
{
	typedef BYTE *address_t, *buffer_t;
	// Current process
	extern const HANDLE process;
	// Hooks
	extern std::map<std::string, address_t> foundRegions;
	address_t getAddress(LPCSTR);
	// Memory Access
	buffer_t readMemory(address_t, size_t, buffer_t);
	void writeMemory(address_t, size_t, buffer_t, bool);
	address_t allocMemory(size_t);
	// Opcode generation
	buffer_t jumpOpCode(address_t, address_t, buffer_t);
	buffer_t callOpCode(address_t, address_t, buffer_t);
	buffer_t fixOpCode(address_t, address_t, buffer_t);
	// Injections
	enum register_t : UINT
	{
		EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
	};
	typedef class Injection
	{
		address_t functionLocation;
		address_t srcLocation;
		size_t srcLength;
		size_t nArgs;
		register_t *args;
		int relocation;
		bool active = false;
		address_t injectionLocation = NULL;
		buffer_t originalCode;
	public:
		Injection(address_t, address_t, size_t, size_t, register_t *, int);
		inline bool isActive() { return active; }
		void activate();
		void deactivate();
	} *LPInjection;
}