#pragma once

#include "stdafx.h"
#include "basictools.h"

using namespace tools;

enum stat_t
{
	CON, STR, DEX, MAG, CHR, PER
};

void levelUp(stat_t);

// Injected functions

void updateLoop();

// Actual RPG stuff

void updateRPGText();

void onDamage(address_t, int);

void collectGem(address_t, address_t);

void onKillEnemy(entity_t);

void onReset(address_t);

void entitySpawn(address_t);

void setTiles(address_t);

void changeEntities();

void collectItem(entity_t);

void collectJelly();