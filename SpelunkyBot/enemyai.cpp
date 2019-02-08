#include "stdafx.h"
#include "enemyai.h"
#include "basictools.h"

typedef struct _extraInfoSnake {
	UINT CHECK = 1001;
	int dashTimer = 0;
	int dashCooldown = 0;
} extraInfoSnake, *pExtraInfoSnake;

DLLExport void snakeAI(LPVOID BASE, LPVOID entity)
{
	pExtraInfoSnake &info = offset<pExtraInfoSnake>(entity, 0x28);
	// Exit if dead
	if (offset<INT>(entity, 0x140) <= 0 || offset<BYTE>(entity, 0x9C))
	{
		delete info;
		return;
	}
	//
	if (info < BASE || info->CHECK != 1001)
	{
		info = new extraInfoSnake;
	}
	if (info->dashCooldown > 0)
	{
		info->dashCooldown--;
	}
	if (info->dashTimer > 0)
	{
		offset<FLOAT>(entity, 0x244) *= 4.0f;
		info->dashTimer--;
	}
	if (info->dashCooldown <= 0)
	{
		float chance = 0.1f / 60;

		LPVOID plr = offset(offset(BASE, 0x1384B4), 0x440684);
		if (plr != nullptr)
		{
			float xdiff = offset<FLOAT>(plr, 0x30) - offset<FLOAT>(entity, 0x30);
			float ydiff = offset<FLOAT>(plr, 0x34) - offset<FLOAT>(entity, 0x34) - 0.1f;
			xdiff *= offset<BYTE>(entity, 0x9D) ? -1 : 1;
			// Higher dash chance if player is about to jump on
			if (ydiff > 0.5 && ydiff < 3.0 && abs(xdiff) < 1.0)
				chance = 1.0f;
			// Higher dash chance if close to player
			if (abs(ydiff) < 0.5 && xdiff > 0 && xdiff < 3.5)
				chance = 0.4f / 6;
		}
		
		if ((rand() % 1000) / 1000.0f < chance)
		{
			info->dashTimer = 15;
			info->dashCooldown = 30 + info->dashTimer;
		}
	}
}