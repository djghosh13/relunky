#include "stdafx.h"
#include "basictools.h"


namespace tools
{
	// Initiates the toolset
	address_t initTools()
	{
		using assembly::Injection;
		using namespace registers;

		// Create main loop
		static address_t BASE = assembly::getAddress("Spelunky.exe");
		static Injection mainLoop(&_mainUpdateLoop, BASE + 0xB2CA6, 5);
		mainLoop.activate();

		// Create debug input
		addUpdateScript(&dev::updateDevConsole);
		addUpdateScript(&dev::updateGetInput);

		// Activate controller input
		controller.controls = offset(offsetM("Spelunky.exe", 0x1384B4), 0x40);
		addUpdateScript(&updateController);

		// Activate keyboard input
		static address_t tiFramework = assembly::getAddress("TextInputFramework.dll") + 0xFA90;
		static Injection keyEvent(&onKeyEvent, tiFramework + 0xE612, 5, 1, 1, EBP);
		keyEvent.activate();
		addUpdateScript(&updateKeyInput);
		return BASE;
	}

	// Affects the main update loop (-ish)
	list<script_t> updateScripts = list<script_t>();

	void addUpdateScript(script_t script)
	{
		updateScripts.push_front(script);
	}

	void removeUpdateScript(script_t script)
	{
		updateScripts.remove(script);
	}

	void _mainUpdateLoop()
	{
		for (auto it = updateScripts.begin(); it != updateScripts.end(); ++it)
		{
			(*it)();
		}
	}

	// Scripts that call in-game methods
	address_t spawnEntity(float x, float y, entity_t eType, char addToList = true)
	{
		address_t func = &offsetM<BYTE>("Spelunky.exe", 0x6FD10);
		address_t gameBase = offsetM("Spelunky.exe", 0x1384B4);
		int dwAddToList = addToList;
		address_t entity = NULL;
		__asm
		{
			push ecx;
			mov ecx, gameBase;
			mov eax, func;
			push dwAddToList;
			push eType;
			push y;
			push x;
			call eax;
			pop ecx;
			mov entity, eax;
		}
		return entity;
	}

	void triggerExplosion(float x, float y, UINT player, entity_t bombType)
	{
		address_t func = &offsetM<BYTE>("Spelunky.exe", 0x86CD0);
		address_t gameBase = offsetM("Spelunky.exe", 0x1384B4);
		__asm
		{
			pushad;
			mov eax, func;
			push bombType;
			push player;
			push y;
			push x;
			push gameBase;
			call eax;
			popad;
		}
	}

	void writeText(LPCWSTR pText, float x, float y, char rAlign, float fontSize)
	{
		address_t func = &offsetM<BYTE>("Spelunky.exe", 0xE8EC0);
		address_t graphicsBase = offset(offsetM("Spelunky.exe", 0x1384B4), 0x50);
		UINT dwAlign = rAlign;
		__asm
		{
			pushad;

			push 0;
			push fontSize;
			push dwAlign;
			push y;
			push x;
			push pText;
			mov edi, graphicsBase;
			mov eax, func;
			call eax;
			popad;
		}
	}
	void writeText(LPCWSTR pText, float x, float y, char rAlign, float fontSize, int r, int g, int b)
	{
		address_t func = &offsetM<BYTE>("Spelunky.exe", 0xE8EC0);
		address_t graphicsBase = offset(offsetM("Spelunky.exe", 0x1384B4), 0x50);
		UINT dwAlign = rAlign;
		float *textColor = NULL;
		__asm
		{
			pushad;

			push 0;
			push fontSize;
			push dwAlign;
			push y;
			push x;
			push pText;
			mov edi, graphicsBase;
			mov eax, func;
			call eax;
			mov textColor, eax;
			popad;
		}
		textColor[0x5A0B + 0] = r / 255.0f;
		textColor[0x5A0B + 1] = g / 255.0f;
		textColor[0x5A0B + 2] = b / 255.0f;
	}

	// Developer tools
	namespace dev
	{
		list<LPCWSTR> consoleOutput = list<LPCWSTR>();
		WCHAR consoleInput[80];
		size_t consoleInputCursor = 0;
		size_t consoleDisplayTimer = 0;

		void println(LPCWSTR text, ...)
		{
			va_list args;
			va_start(args, text);
			LPWSTR buffer = new WCHAR[BUFSIZ];
			vswprintf(buffer, BUFSIZ, text, args);
			va_end(args);

			consoleOutput.push_front(buffer);
			if (consoleOutput.size() > 16)
			{
				consoleOutput.pop_back();
			}
			consoleDisplayTimer = 600;
		}

		void updateDevConsole()
		{
			if (consoleInputCursor == 0)
			{
				if (consoleDisplayTimer == 0) return;
				else consoleDisplayTimer--;
			}

			float y = 9.5;
			for (auto it = consoleOutput.begin(); it != consoleOutput.end(); ++it, y -= 0.5f)
			{
				if ((*it)[0] == L'/')
				{
					writeText((*it) + 1, 0.4f, y, 1, 0.6f, 210, 240, 210);
				}
				else
				{
					writeText(*it, 0.4f, y, 1, 0.6f, 200, 200, 200);
				}
			}
			writeText(consoleInput, 0.4f, 10.0f, 1, 0.65f, 215, 255, 215);
		}

		static void stopEntityLoop() { }
		static void stopTimeLoop()
		{
			address_t gameBase = offsetM("Spelunky.exe", 0x1384B4);
			offset<int>(gameBase, 0x64) = 1;
		}

		void updateGetInput()
		{
			using assembly::getAddress;
			using assembly::Injection;

			static bool consoleActive = false;
			static Injection eloop(&stopEntityLoop, getAddress("Spelunky.exe") + 0x6B76C, 6, 0);
			static Injection tloop(&stopTimeLoop, getAddress("Spelunky.exe") + 0x661DF, 6, 1);

			if (consoleActive)
			{
				// Process input
				if (mkinput.key[0x0D])
				{
					println(consoleInput);
					processInput(consoleInput);
				}
				// Delete input
				if (mkinput.key[0x08] && consoleInputCursor > 0)
				{
					consoleInput[--consoleInputCursor] = 0;
				}
				// Cancel input
				if (mkinput.key[0x0D] || mkinput.key[0x1B] || consoleInputCursor == 0)
				{
					memset(consoleInput, 0, sizeof(consoleInput));
					consoleInputCursor = 0;
					eloop.deactivate();
					tloop.deactivate();
					consoleActive = false;
					return;
				}
				// Get input
				if (mkinput.key[0x20] && consoleInputCursor < 80)
				{
					consoleInput[consoleInputCursor++] = L' ';
				}
				for (WCHAR chr = L'0'; chr <= L'9'; chr++)
				{
					if (mkinput.key[chr] && consoleInputCursor < 80)
					{
						consoleInput[consoleInputCursor++] = chr;
					}
				}
				for (WCHAR chr = L'A'; chr <= L'Z'; chr++)
				{
					if (mkinput.key[chr] && consoleInputCursor < 80)
					{
						consoleInput[consoleInputCursor++] = chr;
					}
				}
			}
			else
			{
				if (mkinput.key[0xBF])
				{
					eloop.activate();
					tloop.activate();
					consoleActive = true;
					consoleInput[consoleInputCursor++] = L'/';
				}
			}
		}

		void processInput(LPCWSTR input)
		{
			std::wstring sInput(input + 1);
			size_t cmdLen = sInput.find_first_of(L' ');
			std::wstring command = sInput.substr(0, cmdLen);
			std::wstring param;
			if (cmdLen == sInput.npos)
			{
				param = L"";
			}
			else
			{
				param = sInput.substr(command.length() + 1);
			}
			// Cases
			if (command == L"HELP")
			{
				println(L"Currently supported commands:");
				println(L"ENABLE <MOD NAME>");
				println(L"DISABLE <MOD NAME>");
				println(L"SHOW");
			}
			else if (command == L"ENABLE")
			{
				bool found = activateMod(param.c_str());
				if (found)
				{
					println(L"Enabled mod: %s", param.c_str());
				}
				else
				{
					println(L"Could not locate mod: %s", param.c_str());
				}
			}
			else if (command == L"DISABLE")
			{
				bool found = deactivateMod(param.c_str());
				if (found)
				{
					println(L"Disabled mod: %s", param.c_str());
				}
				else
				{
					println(L"Could not locate mod: %s", param.c_str());
				}
			}
			else if (command == L"SHOW")
			{
				println(L"Installed mods:");
				for (auto it = loadedMods.begin(); it != loadedMods.end(); ++it)
				{
					println(L"%s: %s", (*it)->getName(), (*it)->isActive() ? L"ON" : L"OFF");
				}
			}
			else
			{
				println(L"Command not recognized: %s", command.c_str());
			}
		}
	}

	// Mod Loading
	Mod::Mod(LPCWSTR name, size_t nScripts, size_t nInjections, ...) : name(name)
	{
		va_list args;
		va_start(args, nInjections);
		for (size_t i = 0; i < nScripts; i++)
		{
			scripts.push_back(va_arg(args, script_t));
		}
		for (size_t i = 0; i < nInjections; i++)
		{
			injections.push_back(va_arg(args, LPInjection));
		}
		va_end(args);
	}

	bool Mod::operator==(LPCWSTR term)
	{
		return name == term;
	}

	void Mod::activate()
	{
		if (active) return;
		for (auto it = scripts.begin(); it != scripts.end(); ++it)
		{
			addUpdateScript(*it);
		}
		for (auto it = injections.begin(); it != injections.end(); ++it)
		{
			(*it)->activate();
		}
		active = true;
	}

	void Mod::deactivate()
	{
		if (!active) return;
		for (auto it = scripts.begin(); it != scripts.end(); ++it)
		{
			removeUpdateScript(*it);
		}
		for (auto it = injections.begin(); it != injections.end(); ++it)
		{
			(*it)->deactivate();
		}
		active = false;
	}

	list<Mod *> loadedMods = list<Mod *>();

	void loadMod(Mod *mod)
	{
		loadedMods.push_back(mod);
	}

	bool activateMod(LPCWSTR name)
	{
		for (auto it = loadedMods.begin(); it != loadedMods.end(); ++it)
		{
			if (**it == name)
			{
				(*it)->activate();
				return true;
			}
		}
		return false;
	}

	bool deactivateMod(LPCWSTR name)
	{
		for (auto it = loadedMods.begin(); it != loadedMods.end(); ++it)
		{
			if (**it == name)
			{
				(*it)->deactivate();
				return true;
			}
		}
		return false;
	}

	// Update Controller
	_controller_t controller;
	void updateController()
	{
		address_t controls = controller.controls;
		auto &input = controller.input;
		auto &previous = controller.previous;

		controller.x = offset<int>(controls, 0x1C);
		controller.y = offset<int>(controls, 0x20);

		input.jump = offset<BYTE>(controls, 0x00) && !previous.jump;
		input.bomb = offset<BYTE>(controls, 0x01) && !previous.bomb;
		input.action = offset<BYTE>(controls, 0x02) && !previous.action;
		input.rope = offset<BYTE>(controls, 0x03) && !previous.rope;
		input.door = offset<BYTE>(controls, 0x05) && !previous.door;
		input.run = offset<BYTE>(controls, 0x0D) && !previous.run;
		input.pause = offset<BYTE>(controls, 0x10) && !previous.pause;
		input.journal = offset<BYTE>(controls, 0x11) && !previous.journal;

		previous.jump = offset<BYTE>(controls, 0x00);
		previous.bomb = offset<BYTE>(controls, 0x01);
		previous.action = offset<BYTE>(controls, 0x02);
		previous.rope = offset<BYTE>(controls, 0x03);
		previous.door = offset<BYTE>(controls, 0x05);
		previous.run = offset<BYTE>(controls, 0x0D);
		previous.pause = offset<BYTE>(controls, 0x10);
		previous.journal = offset<BYTE>(controls, 0x11);
	}

	// Update key input
	_mkinput_t mkinput;
	void onKeyEvent(address_t stackptr)
	{
		BYTE key = offset<BYTE>(stackptr, 0x10);
		BYTE pressed = offset<BYTE>(stackptr, 0x14);

		if (pressed & 0x03)
		{
			mkinput.eventQueue.push(key);
		}
	}

	void updateKeyInput()
	{
		memset(mkinput.key, FALSE, sizeof(mkinput.key));
		while (!mkinput.eventQueue.empty())
		{
			mkinput.key[mkinput.eventQueue.front()] = TRUE;
			mkinput.eventQueue.pop();
		}
	}
}