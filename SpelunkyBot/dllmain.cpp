// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "spelunky.h"
#include "basictools.h"
#include "rpgmod.h"
#include "enemyai.h"

BOOL LOADED = FALSE;

void injectMod(Spelunky *);
void injectAllAI(Spelunky *);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	Spelunky *game;
	MODULEINFO moduleInfo;
	HMODULE tifModule;

	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		if (LOADED) break;

		LOADED = TRUE;
		srand((UINT)time(0));
		game = new Spelunky();

		if (!game->isConnected())
		{
			MessageBoxA(NULL, "Could not find Spelunky.exe base", "Spelunky RPG", MB_ICONEXCLAMATION | MB_OK);
			break;
		}

		injectMod(game);
		injectAllAI(game);

		// Find TextInputFramework.dll
		tifModule = GetModuleHandleA("Spelunky.exe");
		if (tifModule == NULL)
		{
			debug::print(L"Could not find TextInputFramework.dll");
		}
		else
		{
			debug::print(L"Found TextInputFramework.dll!");
			GetModuleInformation(GetCurrentProcess(), tifModule, &moduleInfo, sizeof(moduleInfo));

			LPWSTR buffer = new WCHAR[40];
			swprintf(buffer, 40, L"Located at 0x%.8X", (int)moduleInfo.lpBaseOfDll);
			debug::print(buffer);
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void injectMod(Spelunky *game)
{
	game->inject_call(game->BASE + 0xB2CA6, 5, { (Address)&updateLoop, 0, { }, TRUE });
	game->inject_call(game->BASE + 0x3B655, 7, { (Address)&AIUpdate, 1, { EDI }, TRUE });
	game->inject_call(game->BASE + 0x159C1, 6, { (Address)&onDamage, 2, { ESI, ECX }, TRUE }, 0);
	game->inject_call(game->BASE + 0x52E54, 6, { (Address)&collectGem, 2, { EBX, EDI } });
	game->inject_call(game->BASE + 0x6C2E1, 6, { (Address)&onKillEnemy, 1, { ESI } });
	game->inject_call(game->BASE + 0x63BAC, 7, { (Address)&onReset, 1, { ESI } });
	game->inject_call(game->BASE + 0x6FD10, 8, { (Address)&entitySpawn, 1, { ESP } });
	game->inject_call(game->BASE + 0xD3FB4, 8, { (Address)&setTiles, 1, { ESP }, TRUE });
	game->inject_call(game->BASE + 0x6B708, 6, { (Address)&changeEntities, 0, { }, TRUE });
	game->inject_call(game->BASE + 0x56A6B, 5, { (Address)&collectItem, 1, { EAX } });
	game->inject_call(game->BASE + 0x52D81, 7, { (Address)&collectJelly, 0, { } });
}

void injectAllAI(Spelunky *game)
{
	game->inject_call(game->BASE + 0x3B679, 5, { (Address)&snakeAI, 1, { EDI }, TRUE });
	game->inject_call(game->BASE + 0x3B913, 5, { (Address)&snakeAI, 1, { EDI }, TRUE });
}