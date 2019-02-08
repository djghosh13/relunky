#include "stdafx.h"
#include "rpgmod.h"
#include "basictools.h"


DLLExport struct
{
	UINT LEVEL = 1;
	UINT EXP = 0;
	int MANA = 50;

	UINT stat[6] = { 1, 1, 1, 1, 1, 1 };

	UINT NEXTLEVEL = 1;
	BOOL ON_LEVEL_UP = FALSE;

	struct
	{
		int timer = 1;
	} possessed;
}
player;

const UINT expNeeded[13] = { 100, 250, 600, 1000, 2500, 6000, 10000, 25000, 60000, 100000, 250000, 600000, 1000000 };

DLLExport void levelUp(stat_t stat)
{
	player.stat[stat]++;
	player.LEVEL++;
	player.ON_LEVEL_UP = FALSE;
}

DLLExport void updateLoop(LPVOID BASE)
{
	LPVOID gameBase = offset(BASE, 0x1384B4);
	float *camera = offset<FLOAT *>(BASE, 0x138558);
	LPVOID *entities = offset<LPVOID *>(gameBase, 0x30);
	LPVOID entity = NULL;

	if (offset<UINT>(gameBase, 0x58) == 4)
	{
		return;
	}
	// Put labels on every enemy
	for (size_t i = 0; i < offset<UINT>(entities, 0x7810); i++)
	{
		entity = entities[i];
		if (offset<UINT>(entity, 0x08) == 3)
		{
			LPCWSTR label = getEntityName(offset<entity_t>(entity, 0x0C));
			float xpos = offset<FLOAT>(entity, 0x30) - camera[0] + 10.0f;
			float ypos = -(offset<FLOAT>(entity, 0x34) - camera[1]) + 5.0f;
			float *textColor = writeText(BASE, label, xpos, ypos, 0, 0.5);
			setTextColor(textColor, 255, 50, 50);
		}
	}
	// Write Player Stats
	LPWSTR buffer = new WCHAR[80];
	swprintf(buffer, 80, L"O");
	setTextColor(writeText(BASE, buffer, 0.85f, 1.0f, 0, 1.6f), 0, 0, 0);
	swprintf(buffer, 80, L"%d", player.NEXTLEVEL);
	setTextColor(writeText(BASE, buffer, 0.8f, 1.0f, 0, 1.0f), 220, 240, 220);
	swprintf(buffer, 80, L"%d / %d", player.MANA, 100);
	setTextColor(writeText(BASE, buffer, 0.75f, 1.8f, 1, 0.6f), 100, 120, 240);
	swprintf(buffer, 80, L"%d / %d", player.EXP, expNeeded[player.NEXTLEVEL - 1]);
	setTextColor(writeText(BASE, buffer, 0.75f, 0.45f, 1, 0.5f), 170, 200, 180);
	// On Level Up Screens
	if (player.ON_LEVEL_UP)
	{
		swprintf(buffer, 80, L"Level %d", player.LEVEL + 1);
		setTextColor(writeText(BASE, buffer, 8 - camera[0] + 10, camera[1] - (102 - 9) + 5.5f, 0, 1.8f), 255, 255, 255);
		LPCWSTR labels[] = { L"CON", L"STR", L"DEX", L"MAG", L"CHR", L"PER" };
		for (int i = 0; i < 6; i++)
		{
			float *color = writeText(BASE, labels[i], 15 + 5 * i - camera[0] + 10, camera[1] - (102 - 6) + 5.5f, 0, 1.0f);
			setTextColor(color, 230, 230, 230);
		}
	}
	// Console logging
	std::queue<LPCWSTR> log = debug::log;
	float logY = 10;
	while (!log.empty())
	{
		writeText(BASE, log.front(), 0.5f, logY, 1, 0.6f);
		log.pop();
		logY -= 0.7f;
	}
}

DLLExport void AIUpdate(LPVOID BASE, LPVOID entity)
{
	LPVOID controls = offset(entity, 0x15C);
	if (player.possessed.timer == 0 || controls == nullptr)
	{
		return;
	}

	LPVOID gameBase = offset(BASE, 0x1384B4);
	LPVOID plr = offset(gameBase, 0x440684);

	// Make enemy harmless to player
	offset<BYTE>(entity, 0x1F3) = 1;
	// Stun player
	offset<BYTE>(plr, 0x211) = 1;
	offset<INT>(plr, 0x184) = 6;
	// Set camera target and remove offscreen timer
	offset(offset(BASE, 0x138558), 0x30) = entity;
	offset<INT>(plr, 0x27C) = 0;

	entity_t eType = offset<entity_t>(entity, 0x0C);

	if (eType == SNAKE)
	{
		offset<FLOAT>(entity, 0x244) = 0.0f;
		if (abs(offset<INT>(controls, 0x1C)) > 20)
		{
			offset<BYTE>(entity, 0x9D) = offset<INT>(controls, 0x1C) < 0;
		}
	}

	if (eType == BAT || eType == SPIDER)
	{
		// "Trigger" entity
		if (offset<BYTE>(entity, 0x207))
		{
			offset<BYTE>(entity, 0x207) = 0;
			offset<FLOAT>(entity, 0x34) -= 0.1f;
		}
		// Remove targeting timer
		offset<INT>(entity, 0x270) = 3600;
		// Check for target, create one if nonexistent
		LPVOID target = offset(entity, 0x26C);
		if (offset(entity, 0x26C) == nullptr || offset(entity, 0x26C) == plr)
		{
			target = spawnEntity(BASE, offset<FLOAT>(entity, 0x30), offset<FLOAT>(entity, 0x34), WEB, 1);
			offset<BYTE>(target, 0x1F1) = 0;
			offset(entity, 0x26C) = target;
		}
		// Move target based on controls
		offset<FLOAT>(target, 0x30) = offset<FLOAT>(entity, 0x30) + 0.5f * offset<INT>(controls, 0x1C);
		offset<FLOAT>(target, 0x34) = offset<FLOAT>(entity, 0x34) + 0.5f * offset<INT>(controls, 0x20);
		// If spider, has jump
		if (eType == SPIDER)
		{
			int &timer = offset<INT>(entity, 0x154);
			timer = (timer > 30) ? 30 : (timer < 10) ? 10 : timer;
			if (offset<BYTE>(controls, 0x00) && timer == 10)
			{
				timer = 0;
			}
			if (offset<INT>(controls, 0x20) > 20 && offset<FLOAT>(entity, 0x248) > 0.199)
			{
				offset<FLOAT>(entity, 0x244) *= 0.5f;
			}
			if (offset<INT>(controls, 0x20) < -20 && offset<FLOAT>(entity, 0x248) > 0.199)
			{
				offset<FLOAT>(entity, 0x244) *= 1.3f;
				offset<FLOAT>(entity, 0x248) *= 0.4f;
			}
		}
		// Check if entity is dead
		if (offset<INT>(entity, 0x140) <= 0)
		{
			offset<BYTE>(target, 0x9C) = 1;
		}
	}
	// Explode if triggered
	if (offset<BYTE>(controls, 0x01))
	{
		triggerExplosion(BASE, offset<FLOAT>(entity, 0x30), offset<FLOAT>(entity, 0x34), 0, BOMB);
	}
}

DLLExport void onDamage(LPVOID BASE, LPVOID entity, int expectedHealth)
{
	int &health = (int &)offset(entity, 0x140);
	if (offset<UINT>(entity, 0x08) == 2)
	{
		LPVOID baseGame = offset(BASE, 0x1384B4);
		int multiplier = 3 + (offset<INT>(baseGame, 0x4405D4) - 1) / 4;
		int damageDone = health - expectedHealth;
		health = health - multiplier * damageDone;
	}
	else
	{
		health = expectedHealth;
	}
}

DLLExport void collectGem(LPVOID plr, LPVOID gem)
{
	int healthPlus = 0;
	int manaPlus = 0;
	entity_t gemType = offset<entity_t>(gem, 0x0C);
	// Collect CHR
	if (gemType == DIAMOND)
	{
		levelUp(CHR);
		offset<UINT>(gem, 0x144) = 1000;
	}
	// Add health
	switch (gemType)
	{
	case LARGERUBY:
		healthPlus += 2;
	case SMALLRUBY:
		healthPlus += 2;
	case LARGEEMERALD:
		healthPlus += 1;
	case SMALLEMERALD:
		healthPlus += 1;
		offset(gem, 0x144) = 0;
	default:
		break;
	}
	// Add mana
	switch (gemType)
	{
	case LARGESAPPHIRE:
		manaPlus += 10;
	case SMALLSAPPHIRE:
		manaPlus += 7;
	case LARGEEMERALD:
		manaPlus += 2;
	case SMALLEMERALD:
		manaPlus += 1;
		offset(gem, 0x144) = 0;
	default:
		break;
	}
	//
	INT maxHealth = 40 + 20 * player.stat[CON];
	offset<INT>(plr, 0x140) = offset<INT>(plr, 0x140) + healthPlus;
	if (offset<INT>(plr, 0x140) > maxHealth) offset<INT>(plr, 0x140) = maxHealth;

	player.MANA = player.MANA + manaPlus;
	if (player.MANA > 100) player.MANA = 100;
}

DLLExport void onKillEnemy(entity_t enemy)
{
	UINT addXP = 0;
	switch (enemy)
	{
	case OLDBITEY:
	case YETIKING:
	case ALIENQUEEN:
		addXP += 900;
	case SHOPKEEPER:
	case MUMMY:
	case ALIENLORD:
	case GIANTSPIDER:
	case VLAD:
	case ANUBIS:
	case QUEENBEE:
	case CROCMAN:
	case BLACKKNIGHT:
		addXP += 70;
	case CAVEMAN:
	case MANTRAP:
	case YETI:
	case HAWKMAN:
	case VAMPIRE:
	case DEVIL:
	case GIANTFROG:
	case MAMMOTH:
	case TIKIMAN:
	case GREENKNIGHT:
	case HORSEHEAD:
	case OXFACE:
		addXP += 20;
	case SNAKE:
	case SPIDER:
	case BAT:
	case UFO:
	case SKELETON:
	case PIRANHA:
	case MONKEY:
	case JIANGSHI:
	case FIREFROG:
	case ALIEN:
	case SCORPION:
	case IMP:
	case BEE:
	case COBRA:
	case SPINNERSPIDER:
	case ALIENTANK:
	case SCORPIONFLY:
	case SNAIL:
	case GOLDENMONKEY:
	case SUCCUBUS:
		addXP += 10;
	default:
		break;
	}

	player.EXP += addXP;

	if (player.EXP >= expNeeded[player.NEXTLEVEL - 1])
	{
		player.EXP -= expNeeded[player.NEXTLEVEL - 1];
		player.NEXTLEVEL++;
	}
}

DLLExport void onReset(UINT gameBase)
{
	*(int *)(gameBase + 0x440694) = 40;

	player.LEVEL = 1;

	for (int i = 0; i < 6; i++)
	{
		player.stat[i] = 1;
	}
	player.EXP = 0;
	player.MANA = 50;

	player.NEXTLEVEL = 1; // Only temporary
	player.ON_LEVEL_UP = FALSE;
}

DLLExport void entitySpawn(LPVOID stackPtr)
{
	UINT &eType = (UINT &)offset(stackPtr, 0x2C);

	if (player.ON_LEVEL_UP)
	{
		if ((eType > 1000 && eType < 1080) || eType == 22)
		{
			eType = RUBBLE;
		}
	}
	else
	{
		switch (eType)
		{
		case SPECTACLES:
		case PITCHERSMITT:
		case SPRINGSHOES:
		case SPIKESHOES:
		case COMPASS:
		case ROYALJELLY:
		case UDJATEYE:
		case DIAMOND:
			eType = RUBBLE;
			break;
		default:
			break;
		}
	}
}

DLLExport void setTiles(LPVOID BASE, LPVOID stackptr)
{
	if (player.LEVEL < player.NEXTLEVEL)
	{
		player.ON_LEVEL_UP = TRUE;
	}

	if (player.ON_LEVEL_UP)
	{
		int chunkX = offset<INT>(stackptr, 0xF4) / 10;
		int chunkY = offset<INT>(stackptr, 0xF8) / 8;
		BYTE *chunkMap = (BYTE *)&offset(stackptr, 0x94);

		if (chunkX == 0)
		{
			if (chunkY == 0)
			{
				memcpy(chunkMap,
					"0000000000" "0000000000" "0000000000" "0000090000"
					"1111111111" "1111111111" "1111111111" "1111111111",
					80);
				// Only once
				INT &curLevel = offset<INT>(offset(BASE, 0x1384B4), 0x4405D4);
				if (curLevel > 0) curLevel--;
			}
			else
			{
				memcpy(chunkMap,
					"1111111111" "1111111111" "1111111111" "1111111111"
					"1111111111" "1111111111" "1111111111" "1111111111",
					80);
			}
		}
		else
		{
			if (chunkY == 0)
			{
				memcpy(chunkMap,
					"0000000000" "0000000000" "0000000000" "0000000000"
					"11L1111L11" "1101111011" "1101111011" "1101111011",
					80);
			}
			else if (chunkY < 3)
			{
				memcpy(chunkMap,
					"1101111011" "1101111011" "1101111011" "1101111011"
					"1101111011" "1101111011" "1101111011" "1101111011",
					80);
			}
			else
			{
				memcpy(chunkMap,
					"1101111011" "1101111011" "1101111011" "1101111011"
					"1101111011" "1191111911" "1111111111" "1111111111",
					80);
			}
		}
	}
}

DLLExport void changeEntities(LPVOID BASE)
{
	LPVOID *entities = offset<LPVOID *>(offset(BASE, 0x1384B4), 0x30);
	LPVOID *tiles = offset<LPVOID *>(offset(BASE, 0x1384B4), 0x3C);
	LPVOID entity = NULL;

	// Change shop item prices
	for (size_t i = 0; i < offset<UINT>(entities, 0x7810); i++)
	{
		entity = entities[i];
		if (offset<UINT>(entity, 0x08) == 4 && (offset<BYTE>(entity, 0x1F4) & 1) == 1)
		{
			offset<UINT>(entity, 0x144) = 10 * (int)(offset<UINT>(entity, 0x144) / 10 * (1.15 - player.stat[CHR] * 0.15));
		}
	}

	if (player.ON_LEVEL_UP)
	{
		// Remove all enemies
		for (size_t i = 0; i < offset<UINT>(entities, 0x7810); i++)
		{
			entity = entities[i];
			if (offset<UINT>(entity, 0x08) == 3)
			{
				offset<BYTE>(entity, 0x9C) |= 1;
			}
		}
		// Make tiles indestructible
		for (size_t i = 0; i < 46 * 102; i++)
		{
			entity = tiles[1235 + i];
			if (entity != NULL)
			{
				offset<BYTE>(entity, 0x9F) |= 1;
			}
		}
		// Spawn in webs
		spawnEntity(BASE, 15.0, 73.0, WEB, 1);
		spawnEntity(BASE, 20.0, 73.0, WEB, 1);
		spawnEntity(BASE, 25.0, 73.0, WEB, 1);
		spawnEntity(BASE, 30.0, 73.0, WEB, 1);
		spawnEntity(BASE, 35.0, 73.0, WEB, 1);
		spawnEntity(BASE, 40.0, 73.0, WEB, 1);
		// Spawn in level up items
		if (player.stat[CON] < 3) spawnEntity(BASE, 15.0, 75.0, ROYALJELLY, 1);
		if (player.stat[STR] == 1) spawnEntity(BASE, 20.0, 75.0, PITCHERSMITT, 1);
		if (player.stat[STR] == 2) spawnEntity(BASE, 20.0, 75.0, SPIKESHOES, 1);
		if (player.stat[DEX] == 1) spawnEntity(BASE, 25.0, 75.0, SPRINGSHOES, 1);
		if (player.stat[MAG] < 3) spawnEntity(BASE, 30.0, 75.0, SPECTACLES, 1);
		if (player.stat[CHR] < 3) spawnEntity(BASE, 35.0, 75.0, DIAMOND, 1);
		if (player.stat[PER] == 1) spawnEntity(BASE, 40.0, 75.0, COMPASS, 1);
		if (player.stat[PER] == 2) spawnEntity(BASE, 40.0, 75.0, UDJATEYE, 1);
	}
}

DLLExport void collectItem(UINT itemID)
{
	switch (itemID)
	{
	case PITCHERSMITT:
	case SPIKESHOES:
		levelUp(STR);
		break;
	case SPRINGSHOES:
		levelUp(DEX);
		break;
	case SPECTACLES:
		levelUp(MAG);
		break;
	case COMPASS:
	case UDJATEYE:
		levelUp(PER);
		break;
	default:
		break;
	}
}

DLLExport void collectJelly()
{
	levelUp(CON);
}