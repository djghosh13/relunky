#pragma once

#include "stdafx.h"
#include "EntityTypes.h"

enum stat_t;

template<typename T = LPVOID>
inline T &offset(LPVOID base, int os) { return *(T *)((BYTE *)base + os); }

extern DLLExport void levelUp(stat_t);

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

// Assembly-Integrated Functions

extern DLLExport int spawnEntity(LPVOID, float, float, entity_t, char);

extern DLLExport float *writeText(LPVOID, LPCWSTR, float, float, char, float);

extern DLLExport void setTextColor(float *, int, int, int);