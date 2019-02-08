#pragma once

#include "stdafx.h"

namespace debug
{
	extern DLLExport std::queue<LPCWSTR> log;
	extern DLLExport void print(LPCWSTR);
}

enum stat_t
{
	CON, STR, DEX, MAG, CHR, PER
};

enum entity_t
{
#define X(name, str, id) name = id,
#include "AllEntities.txt"
#undef X
	UNKNOWN
};

template<typename T = LPVOID>
DLLExport T &offset(LPVOID base, int offset)
{
	return *(T *)((BYTE *)base + offset);
}

extern DLLExport LPCWSTR getEntityName(entity_t);

extern DLLExport LPVOID spawnEntity(LPVOID, float, float, entity_t, char);

extern DLLExport void triggerExplosion(LPVOID, float, float, UINT, entity_t);

extern DLLExport float *writeText(LPVOID, LPCWSTR, float, float, char, float);

extern DLLExport void setTextColor(float *, int, int, int);