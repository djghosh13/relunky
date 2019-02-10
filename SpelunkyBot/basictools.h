#pragma once

#include "stdafx.h"
#include "assembly.h"

namespace tools
{
	using std::list;
	using assembly::address_t;
	using assembly::offset;
	using assembly::offsetM;
	using assembly::Injection;
	using assembly::LPInjection;

	typedef void(*script_t)();
	struct command_t
	{
		std::wstring name;
		void(*command)(list<LPCWSTR>);
		LPCWSTR docstring = NULL;

		inline bool operator==(command_t other)
		{
			return name == other.name && command == other.command;
		}
	};

	// Initiates the toolset
	address_t initTools();

	// Easier register access
	namespace registers
	{
		using assembly::register_t;
		using assembly::EAX;
		using assembly::ECX;
		using assembly::EDX;
		using assembly::EBX;
		using assembly::ESP;
		using assembly::EBP;
		using assembly::ESI;
		using assembly::EDI;
	}

	// Defines entity types
	enum entity_t : UINT
	{
#define X(name, str, id) name = id,
#include "allentities.txt"
#undef X
		UNKNOWN
	};

	inline LPCWSTR getEntityName(entity_t eType)
	{
		switch (eType)
		{
#define X(name, str, id) case id: return str;
#include "allentities.txt"
#undef X
		default: return L" ";
		}
	}

	// Affects main update loop
	extern list<script_t> updateScripts;
	void addUpdateScript(script_t);
	void removeUpdateScript(script_t);
	void _mainUpdateLoop();

	// Scripts that call in-game methods
	address_t spawnEntity(float, float, entity_t, char);

	void triggerExplosion(float, float, UINT, entity_t);

	void writeText(LPCWSTR, float, float, char, float);
	void writeText(LPCWSTR, float, float, char, float, int, int, int);

	// Developer tools
	namespace dev
	{
		extern list<command_t> validCommands;

		void registerCommand(command_t);
		void unregisterCommand(command_t);
		void println(LPCWSTR, ...);
		void updateDevConsole();
		void updateGetInput();
		void processInput(LPCWSTR);
	}
}