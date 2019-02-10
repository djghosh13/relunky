#include "stdafx.h"
#include "basictools.h"
#include "inputcapture.h"

namespace tools
{
	// Initiates the toolset
	address_t initTools()
	{
		using assembly::Injection;
		using namespace controller;
		using namespace keyboard;
		using namespace registers;

		static bool alreadyCalled = false;
		// Create main loop
		address_t BASE = assembly::getAddress("Spelunky.exe");
		if (alreadyCalled) return BASE;
		Injection mainLoop(&_mainUpdateLoop, BASE + 0xB2CA6, 5);
		mainLoop.activate();

		// Create debug input
		addUpdateScript(&dev::updateDevConsole);
		addUpdateScript(&dev::updateGetInput);

		// Activate controller input
		controls = offset(offsetM("Spelunky.exe", 0x1384B4), 0x40);
		addUpdateScript(&updateController);

		// Activate keyboard input
		static address_t textFrmwk = assembly::getAddress("TextInputFramework.dll") + 0xFA90;
		static Injection keyEvent(&onKeyEvent, textFrmwk + 0xE612, 5, 1, 1, EBP);
		keyEvent.activate();
		addUpdateScript(&updateKeyInput);

		alreadyCalled = true;
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
		static list<LPCWSTR> consoleOutput = list<LPCWSTR>();
		static WCHAR consoleInput[80];
		static size_t consoleInputCursor = 0;
		static size_t consoleDisplayTimer = 0;

		list<command_t> validCommands = list<command_t>();

		void registerCommand(command_t cmd)
		{
			for (auto it = validCommands.begin(); it != validCommands.end(); ++it)
			{
				if (it->name == cmd.name)
				{
					// Do not overwrite existing commands
					return;
				}
			}
			validCommands.push_back(cmd);
		}

		void unregisterCommand(command_t cmd)
		{
			validCommands.remove(cmd);
		}

		void println(LPCWSTR text, ...)
		{
			va_list args;
			va_start(args, text);
			LPWSTR buffer = new WCHAR[BUFSIZ];
			vswprintf(buffer, BUFSIZ, text, args);
			va_end(args);

			consoleOutput.push_front(buffer);
			if (consoleOutput.size() > 15)
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
			using namespace keyboard;

			static bool consoleActive = false;
			static Injection eloop(&stopEntityLoop, getAddress("Spelunky.exe") + 0x6B76C, 6, 0);
			static Injection tloop(&stopTimeLoop, getAddress("Spelunky.exe") + 0x661DF, 6, 1);

			if (consoleActive)
			{
				// Process input
				if (pressed[0x0D])
				{
					println(consoleInput);
					processInput(consoleInput);
				}
				// Delete input
				if (pressed[0x08] && consoleInputCursor > 0)
				{
					consoleInput[--consoleInputCursor] = 0;
				}
				// Cancel input
				if (pressed[0x0D] || pressed[0x1B] || consoleInputCursor == 0)
				{
					memset(consoleInput, 0, sizeof(consoleInput));
					consoleInputCursor = 0;
					eloop.deactivate();
					tloop.deactivate();
					consoleActive = false;
					return;
				}
				// Get input
				if (pressed[0x20] && consoleInputCursor < 80)
				{
					consoleInput[consoleInputCursor++] = L' ';
				}
				for (WCHAR chr = L'0'; chr <= L'9'; chr++)
				{
					if (pressed[chr] && consoleInputCursor < 80)
					{
						consoleInput[consoleInputCursor++] = chr;
					}
				}
				for (WCHAR chr = L'A'; chr <= L'Z'; chr++)
				{
					if (pressed[chr] && consoleInputCursor < 80)
					{
						consoleInput[consoleInputCursor++] = chr;
					}
				}
			}
			else
			{
				if (pressed[0xBF])
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
			LPWSTR tokens = new WCHAR[BUFSIZ];
			lstrcpyW(tokens, input + 1);
			LPWSTR nextToken = NULL;
			LPWSTR token = wcstok_s(tokens, L" ", &nextToken);
			if (token == nullptr) return;

			std::wstring command = token;
			list<LPCWSTR> args = list<LPCWSTR>();

			while (token != nullptr)
			{
				token = wcstok_s(NULL, L" ", &nextToken);
				if (token != nullptr)
				{
					args.push_back(token);
				}
			}

			// Run command
			for (auto it = validCommands.begin(); it != validCommands.end(); ++it)
			{
				if (it->name == command)
				{
					it->command(args);
					return;
				}
			}
			println(L"Command not recognized: %s", command.c_str());
		}
	}
}