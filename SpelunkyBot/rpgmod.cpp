#include "stdafx.h"
#include "rpgmod.h"
#include "basictools.h"

using namespace tools;

struct
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

void levelUp(stat_t stat)
{
	player.stat[stat]++;
	player.LEVEL++;
	player.ON_LEVEL_UP = FALSE;
}

void updateLoop()
{
	address_t gameBase = offsetM("Spelunky.exe", 0x1384B4);
	float *camera = offsetM<float *>("Spelunky.exe", 0x138558);
	address_t *entities = offset<address_t *>(gameBase, 0x30);
	address_t entity = NULL;

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
			writeText(label, xpos, ypos, 0, 0.5, 255, 50, 50);
		}
	}
}

void updateRPGText()
{
	float *camera = offsetM<float *>("Spelunky.exe", 0x138558);
	// Write Player Stats
	LPWSTR buffer = new WCHAR[80];
	swprintf(buffer, 80, L"O");
	writeText(buffer, 0.9f, 1.0f, 0, 1.8f, 0, 0, 0);
	swprintf(buffer, 80, L"%d", player.NEXTLEVEL);
	writeText(buffer, 0.8f, 1.0f, 0, 1.0f, 220, 240, 220);
	swprintf(buffer, 80, L"%d / %d", player.MANA, 100);
	writeText(buffer, 0.75f, 1.8f, 1, 0.6f, 100, 120, 240);
	swprintf(buffer, 80, L"%d / %d", player.EXP, expNeeded[player.NEXTLEVEL - 1]);
	writeText(buffer, 0.75f, 0.45f, 1, 0.5f, 170, 200, 180);
	// On Level Up Screens
	if (player.ON_LEVEL_UP)
	{
		swprintf(buffer, 80, L"Level %d", player.LEVEL + 1);
		writeText(buffer, 8 - camera[0] + 10, camera[1] - (102 - 9) + 5.5f, 0, 1.8f, 255, 255, 255);
		LPCWSTR labels[] = { L"CON", L"STR", L"DEX", L"MAG", L"CHR", L"PER" };
		for (int i = 0; i < 6; i++)
		{
			writeText(labels[i], 15 + 5 * i - camera[0] + 10, camera[1] - (102 - 6) + 5.5f, 0, 1.0f, 230, 230, 230);
		}
	}
}

void onDamage(address_t entity, int expectedHealth)
{
	int &health = offset<int>(entity, 0x140);
	if (offset<UINT>(entity, 0x08) == 2)
	{
		address_t gameBase = offsetM("Spelunky.exe", 0x1384B4);
		int multiplier = 3 + (offset<INT>(gameBase, 0x4405D4) - 1) / 4;
		int damageDone = health - expectedHealth;
		health = health - multiplier * damageDone;
	}
	else
	{
		health = expectedHealth;
	}
}

void collectGem(address_t plr, address_t gem)
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
		offset<int>(gem, 0x144) = 0;
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
		offset<int>(gem, 0x144) = 0;
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

void onKillEnemy(entity_t enemy)
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

void onReset(address_t gameBase)
{
	// FIX THIS
	offset<INT>(gameBase, 0x440694) = 40;

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

void entitySpawn(address_t stackPtr)
{
	UINT &eType = offset<UINT>(stackPtr, 0x2C);

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

void setTiles(address_t stackptr)
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
				INT &curLevel = offset<INT>(offsetM("Spelunky.exe", 0x1384B4), 0x4405D4);
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

void changeEntities()
{
	address_t *entities = offset<address_t *>(offsetM("Spelunky.exe", 0x1384B4), 0x30);
	address_t *tiles = offset<address_t *>(offsetM("Spelunky.exe", 0x1384B4), 0x3C);
	address_t entity = NULL;

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
		spawnEntity(15.0, 73.0, WEB, 1);
		spawnEntity(20.0, 73.0, WEB, 1);
		spawnEntity(25.0, 73.0, WEB, 1);
		spawnEntity(30.0, 73.0, WEB, 1);
		spawnEntity(35.0, 73.0, WEB, 1);
		spawnEntity(40.0, 73.0, WEB, 1);
		// Spawn in level up items
		if (player.stat[CON] < 3) spawnEntity(15.0, 75.0, ROYALJELLY, 1);
		if (player.stat[STR] == 1) spawnEntity(20.0, 75.0, PITCHERSMITT, 1);
		if (player.stat[STR] == 2) spawnEntity(20.0, 75.0, SPIKESHOES, 1);
		if (player.stat[DEX] == 1) spawnEntity(25.0, 75.0, SPRINGSHOES, 1);
		if (player.stat[MAG] < 3) spawnEntity(30.0, 75.0, SPECTACLES, 1);
		if (player.stat[CHR] < 3) spawnEntity(35.0, 75.0, DIAMOND, 1);
		if (player.stat[PER] == 1) spawnEntity(40.0, 75.0, COMPASS, 1);
		if (player.stat[PER] == 2) spawnEntity(40.0, 75.0, UDJATEYE, 1);
	}
}

void collectItem(entity_t itemID)
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

void collectJelly()
{
	levelUp(CON);
}