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

namespace spelunky
{
#pragma pack (push, 1)
	typedef struct _INPUT
	{
		BYTE jump, bomb, action, rope;
		BYTE unusedLB, door;
		SHORT __0x06;
		INT __0x08;
		BYTE unusedLT, run;
		SHORT __0x0E;
		BYTE pause, journal;
		SHORT __0x12;
		INT __0x14, __0x18;
		INT movex, movey;
	} INPUT, *PINPUT;
#pragma pack (pop)
}

struct controller
{
	spelunky::PINPUT inputs;
	spelunky::INPUT prevInputs;
	BYTE jump, bomb, action, rope, door, run, pause, journal;
	INT x, y;

	controller(spelunky::PINPUT inputs) : inputs(inputs), prevInputs(*inputs) { }

	void update()
	{
		x = inputs->movex;
		y = inputs->movey;
		jump = inputs->jump && !prevInputs.jump;
		bomb = inputs->bomb && !prevInputs.bomb;
		action = inputs->action && !prevInputs.action;
		rope = inputs->rope && !prevInputs.rope;
		door = inputs->door && !prevInputs.door;
		run = inputs->run && !prevInputs.run;
		pause = inputs->pause && !prevInputs.pause;
		journal = inputs->journal && !prevInputs.journal;

		prevInputs = *inputs;
	}
};

template<typename T = LPVOID>
DLLExport T &offset(LPVOID base, int offset)
{
	return *(T *)((BYTE *)base + offset);
}

extern DLLExport LPCWSTR getEntityName(entity_t);

extern DLLExport LPVOID spawnEntity(LPVOID, float, float, entity_t, char);

extern DLLExport void triggerExplosion(LPVOID, float, float, UINT, entity_t);

// Writing methods

extern DLLExport float *writeText(LPVOID, LPCWSTR, float, float, char, float);

extern DLLExport void setTextColor(float *, int, int, int);