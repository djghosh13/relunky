#pragma once

#include "stdafx.h"
#include "basictools.h"

using namespace tools;

namespace mod
{
	using namespace tools;

	struct mod_t
	{
		std::wstring name;
		list<script_t> scripts;
		list<LPInjection> injections;
		list<command_t> commands;
		bool active = false;
	};

	void initLoader();

	void load(mod_t *);
	bool enable(LPCWSTR);
	bool disable(LPCWSTR);

	list<script_t> scripts(size_t, ...);
	list<LPInjection> injections(size_t, ...);
	list<command_t> commands(size_t, ...);
}
// Load/Unload Mods
//class Mod
//{
//	std::wstring name;
//	list<script_t> scripts;
//	list<LPInjection> injections;
//	bool active = false;
//public:
//	Mod(LPCWSTR, size_t, size_t, ...);
//	inline LPCWSTR getName() { return name.c_str(); }
//	inline bool isActive() { return active; }
//	bool operator==(LPCWSTR);
//	void activate();
//	void deactivate();
//};
//
//extern list<Mod *> loadedMods;
//
//void loadMod(Mod *);
//bool activateMod(LPCWSTR);
//bool deactivateMod(LPCWSTR);