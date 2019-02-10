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
		extern list<LPCWSTR> consoleOutput;
		extern WCHAR consoleInput[80];
		extern size_t consoleInputCursor;
		extern size_t consoleDisplayTimer;
		void println(LPCWSTR, ...);
		void updateDevConsole();
		void updateGetInput();
		void processInput(LPCWSTR);
	}

	// Load/Unload Mods
	class Mod
	{
		std::wstring name;
		list<script_t> scripts;
		list<LPInjection> injections;
		bool active = false;
	public:
		Mod(LPCWSTR, size_t, size_t, ...);
		inline LPCWSTR getName() { return name.c_str(); }
		inline bool isActive() { return active; }
		bool operator==(LPCWSTR);
		void activate();
		void deactivate();
	};

	extern list<Mod *> loadedMods;

	void loadMod(Mod *);
	bool activateMod(LPCWSTR);
	bool deactivateMod(LPCWSTR);
}