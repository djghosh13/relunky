// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "assembly.h"
#include "basictools.h"
#include "enemycontrol.h"
#include "rpgmod.h"
#include "enemyai.h"
#include "modloader.h"

using namespace tools;
using namespace registers;

address_t BASE;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		srand((UINT)time(0));

		BASE = initTools();

		mod::initLoader();

		mod::load(new mod::mod_t {
			L"ENEMYNAMES",
			{ updateLoop },
			{ },
			{ }
		});

		mod::load(new mod::mod_t {
			L"RPGMODE",
			{ updateRPGText },
			{
				new Injection(&onDamage, BASE + 0x159C1, 6, 0, 2, ESI, ECX),
				new Injection(&collectGem, BASE + 0x52E54, 6, 1, 2, EBX, EDI),
				new Injection(&onKillEnemy, BASE + 0x6C2E1, 6, 1, 1, ESI),
				new Injection(&onReset, BASE + 0x63BAC, 7, 1, 1, ESI),
				new Injection(&entitySpawn, BASE + 0x6FD10, 8, 1, 1, ESP),
				new Injection(&setTiles, BASE + 0xD3FB4, 8, 1, 1, ESP),
				new Injection(&changeEntities, BASE + 0x6B708, 6, 1),
				new Injection(&collectItem, BASE + 0x56A6B, 5, 1, 1, EAX),
				new Injection(&collectJelly, BASE + 0x52D81, 7, 1)
			},
			{ }
		});

		mod::load(new mod::mod_t {
			L"POSSESSION",
			{ },
			{
				new Injection(&AIUpdate, BASE + 0x3B655, 7, 1, 1, EDI)
			},
			{
				{ L"POSSESS", possessNearestEnemy, L"Possess the nearest enemy" },
				{ L"UNPOSSESS", releaseAllEnemies, L"Releases control of all enemies" }
			}
		});
		/*loadMod(new Mod(L"UPDATELOOP", 1, 0,
			updateLoop
		));
		loadMod(new Mod(L"RPGMODE", 1, 9,
			updateRPGText,
			new Injection(&onDamage, BASE + 0x159C1, 6, 0, 2, ESI, ECX),
			new Injection(&collectGem, BASE + 0x52E54, 6, 1, 2, EBX, EDI),
			new Injection(&onKillEnemy, BASE + 0x6C2E1, 6, 1, 1, ESI),
			new Injection(&onReset, BASE + 0x63BAC, 7, 1, 1, ESI),
			new Injection(&entitySpawn, BASE + 0x6FD10, 8, 1, 1, ESP),
			new Injection(&setTiles, BASE + 0xD3FB4, 8, 1, 1, ESP),
			new Injection(&changeEntities, BASE + 0x6B708, 6, 1),
			new Injection(&collectItem, BASE + 0x56A6B, 5, 1, 1, EAX),
			new Injection(&collectJelly, BASE + 0x52D81, 7, 1)
		));
		loadMod(new Mod(L"POSSESSION", 0, 1,
			new Injection(&AIUpdate, BASE + 0x3B655, 7, 1, 1, EDI)
		));
		activateMod(L"UPDATELOOP");*/

		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//void injectMod(Spelunky *game)
//{
//	game->inject_call(game->BASE + 0xB2CA6, 5, { (Address)&updateLoop, 0, { }, TRUE });
//	game->inject_call(game->BASE + 0x3B655, 7, { (Address)&AIUpdate, 1, { EDI }, TRUE });
//	game->inject_call(game->BASE + 0x159C1, 6, { (Address)&onDamage, 2, { ESI, ECX }, TRUE }, 0);
//	game->inject_call(game->BASE + 0x52E54, 6, { (Address)&collectGem, 2, { EBX, EDI } });
//	game->inject_call(game->BASE + 0x6C2E1, 6, { (Address)&onKillEnemy, 1, { ESI } });
//	game->inject_call(game->BASE + 0x63BAC, 7, { (Address)&onReset, 1, { ESI } });
//	game->inject_call(game->BASE + 0x6FD10, 8, { (Address)&entitySpawn, 1, { ESP } });
//	game->inject_call(game->BASE + 0xD3FB4, 8, { (Address)&setTiles, 1, { ESP }, TRUE });
//	game->inject_call(game->BASE + 0x6B708, 6, { (Address)&changeEntities, 0, { }, TRUE });
//	game->inject_call(game->BASE + 0x56A6B, 5, { (Address)&collectItem, 1, { EAX } });
//	game->inject_call(game->BASE + 0x52D81, 7, { (Address)&collectJelly, 0, { } });
//}
//
//void injectAllAI(Spelunky *game)
//{
//	game->inject_call(game->BASE + 0x3B679, 5, { (Address)&snakeAI, 1, { EDI }, TRUE });
//	game->inject_call(game->BASE + 0x3B913, 5, { (Address)&snakeAI, 1, { EDI }, TRUE });
//}