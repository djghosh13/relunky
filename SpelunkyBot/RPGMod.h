#pragma once

#include "stdafx.h"
#include "basictools.h"


extern DLLExport void levelUp(stat_t);

// Injected functions

extern DLLExport void updateLoop(LPVOID);

extern DLLExport void AIUpdate(LPVOID, LPVOID);

extern DLLExport void onDamage(LPVOID, LPVOID, int);

extern DLLExport void collectGem(LPVOID, LPVOID);

extern DLLExport void onKillEnemy(entity_t);

extern DLLExport void onReset(UINT);

extern DLLExport void entitySpawn(LPVOID);

extern DLLExport void setTiles(LPVOID, LPVOID);

extern DLLExport void changeEntities(LPVOID);

extern DLLExport void collectItem(UINT);

extern DLLExport void collectJelly();