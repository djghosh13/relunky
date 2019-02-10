#include "stdafx.h"
#include "modloader.h"
#include "basictools.h"

namespace mod
{
	using namespace tools;

	static list<mod_t *> loadedMods;

	static void cmdShowHelp(list<LPCWSTR>);
	static void cmdShowMods(list<LPCWSTR>);
	static void cmdEnableMod(list<LPCWSTR>);
	static void cmdDisableMod(list<LPCWSTR>);

	void initLoader()
	{
		static bool loaded = false;
		if (loaded) return;

		dev::registerCommand({ L"HELP", &cmdShowHelp, L"Shows this help menu" });
		dev::registerCommand({ L"SHOW", &cmdShowMods, L"Lists all available mods" });
		dev::registerCommand({ L"ENABLE", &cmdEnableMod, L"Enables one or more mods" });
		dev::registerCommand({ L"DISABLE", &cmdDisableMod, L"Disables one or more mods" });

		loaded = true;
	}

	void load(mod_t *mod)
	{
		mod->active = false;
		for (auto it = loadedMods.begin(); it != loadedMods.end(); ++it)
		{
			if ((*it)->name == mod->name)
			{
				*it = mod;
				return;
			}
		}
		loadedMods.push_back(mod);
	}

	bool enable(LPCWSTR modName)
	{
		for (auto it = loadedMods.begin(); it != loadedMods.end(); ++it)
		{
			auto mod = *it;
			if (mod->name == modName)
			{
				if (mod->active) return true;
				// Activate mod
				for (auto jt = mod->scripts.begin(); jt != mod->scripts.end(); ++jt)
				{
					addUpdateScript(*jt);
				}
				for (auto jt = mod->injections.begin(); jt != mod->injections.end(); ++jt)
				{
					(*jt)->activate();
				}
				for (auto jt = mod->commands.begin(); jt != mod->commands.end(); ++jt)
				{
					dev::registerCommand(*jt);
				}
				mod->active = true;
				return true;
			}
		}
		return false;
	}

	bool disable(LPCWSTR modName)
	{
		for (auto it = loadedMods.begin(); it != loadedMods.end(); ++it)
		{
			auto mod = *it;
			if (mod->name == modName)
			{
				if (!mod->active) return true;
				// Deactivate mod
				for (auto jt = mod->scripts.begin(); jt != mod->scripts.end(); ++jt)
				{
					removeUpdateScript(*jt);
				}
				for (auto jt = mod->injections.begin(); jt != mod->injections.end(); ++jt)
				{
					(*jt)->deactivate();
				}
				for (auto jt = mod->commands.begin(); jt != mod->commands.end(); ++jt)
				{
					dev::unregisterCommand(*jt);
				}
				mod->active = false;
				return true;
			}
		}
		return false;
	}

	// Make it easier to initialize mods
	list<script_t> scripts(size_t nArgs, ...)
	{
		list<script_t> allScripts = list<script_t>();
		va_list args;
		va_start(args, nArgs);
		for (size_t i = 0; i < nArgs; i++)
		{
			allScripts.push_back(va_arg(args, script_t));
		}
		va_end(args);
		return allScripts;
	}

	list<LPInjection> injections(size_t nArgs, ...)
	{
		list<LPInjection> allInjections = list<LPInjection>();
		va_list args;
		va_start(args, nArgs);
		for (size_t i = 0; i < nArgs; i++)
		{
			allInjections.push_back(va_arg(args, LPInjection));
		}
		va_end(args);
		return allInjections;
	}

	list<command_t> commands(size_t nArgs, ...)
	{
		list<command_t> allCommands = list<command_t>();
		va_list args;
		va_start(args, nArgs);
		for (size_t i = 0; i < nArgs; i++)
		{
			allCommands.push_back(va_arg(args, command_t));
		}
		va_end(args);
		return allCommands;
	}

	// COMMANDS
	static void cmdShowHelp(list<LPCWSTR> args)
	{
		using namespace dev;

		println(L"Currently available commands:");
		for (auto it = validCommands.begin(); it != validCommands.end(); ++it)
		{
			if (it->docstring == nullptr)
			{
				println(it->name.c_str());
			}
			else
			{
				println(L"%s: %s", it->name.c_str(), it->docstring);
			}
		}
	}
	
	static void cmdShowMods(list<LPCWSTR> args)
	{
		using namespace dev;

		println(L"Currently loaded mods:");
		for (auto it = loadedMods.begin(); it != loadedMods.end(); ++it)
		{
			println(L"%s: %s", (*it)->name.c_str(), (*it)->active ? L"ON" : L"OFF");
		}
	}

	static void cmdEnableMod(list<LPCWSTR> args)
	{
		using namespace dev;

		if (args.empty()) println(L"Usage: ENABLE <MOD NAME> ...");

		for (auto it = args.begin(); it != args.end(); ++it)
		{
			if (enable(*it))
			{
				println(L"Enabled mod: %s", *it);
			}
			else
			{
				println(L"Could not locate mod: %s", *it);
			}
		}
	}

	static void cmdDisableMod(list<LPCWSTR> args)
	{
		using namespace dev;

		if (args.empty()) println(L"Usage: DISABLE <MOD NAME> ...");

		for (auto it = args.begin(); it != args.end(); ++it)
		{
			if (disable(*it))
			{
				println(L"Disabled mod: %s", *it);
			}
			else
			{
				println(L"Could not locate mod: %s", *it);
			}
		}
	}
}