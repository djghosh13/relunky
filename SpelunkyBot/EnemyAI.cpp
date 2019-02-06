#include "stdafx.h"
#include "EnemyAI.h"
#include "RPGMod.h"

typedef struct _extraInfoSnake {
	int dashTimer = 0;
} extraInfoSnake, *pExtraInfoSnake;

DLLExport void snakeAI(LPVOID BASE, LPVOID entity)
{
	pExtraInfoSnake &info = offset<pExtraInfoSnake>(entity, 0x1C);
	if (info == nullptr)
	{
		info = new extraInfoSnake;
	}
	if (info->dashTimer > 0)
	{
		offset<FLOAT>(entity, 0x244) *= 1.5;
		info->dashTimer--;
	}
	else
	{
		float chance = 0.3f / 60;
		if ((rand() % 1000) / 1000.0f < chance)
		{
			info->dashTimer = 40;
		}
	}
}