#include "stdafx.h"
#include "enemycontrol.h"
#include "basictools.h"

void AIUpdate(address_t entity)
{
	address_t gameBase = offsetM("Spelunky.exe", 0x1384B4);
	address_t plr = offset(gameBase, 0x440684);

	BYTE &isControlled = offset<BYTE>(entity, 0x15C);
	if (!isControlled) return;

	// Make enemy harmless to player
	offset<BYTE>(entity, 0x1F3) = 1;
	// Stun player
	offset<BYTE>(plr, 0x211) = 1;
	offset<INT>(plr, 0x184) = 6;
	// Set camera target and remove offscreen timer
	offset(offsetM("Spelunky.exe", 0x138558), 0x30) = entity;
	offset<INT>(plr, 0x27C) = 0;

	// Check if entity is dead
	if (offset<INT>(entity, 0x140) <= 0)
	{
		isControlled = FALSE;
		offset(offsetM("Spelunky.exe", 0x138558), 0x30) = plr;
		return;
	}

	entity_t eType = offset<entity_t>(entity, 0x0C);

	if (eType == SNAKE)
	{
		offset<FLOAT>(entity, 0x244) = 0.0f;
		if (abs(controller.x) > 20)
		{
			offset<BYTE>(entity, 0x9D) = controller.x < 0;
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
		address_t target = offset(entity, 0x26C);
		if (offset(entity, 0x26C) == nullptr || offset(entity, 0x26C) == plr)
		{
			target = spawnEntity(offset<FLOAT>(entity, 0x30), offset<FLOAT>(entity, 0x34), WEB, 1);
			offset<BYTE>(target, 0x1F1) = 0;
			offset(entity, 0x26C) = target;
		}
		// Move target based on controls
		offset<FLOAT>(target, 0x30) = offset<FLOAT>(entity, 0x30) + 0.5f * controller.x;
		offset<FLOAT>(target, 0x34) = offset<FLOAT>(entity, 0x34) + 0.5f * controller.y;
		// If spider, has jump
		if (eType == SPIDER)
		{
			int &timer = offset<INT>(entity, 0x154);
			timer = (timer > 30) ? 30 : (timer < 10) ? 10 : timer;
			if (controller.input.jump && timer == 10)
			{
				timer = 0;
			}
			if (controller.y > 20 && offset<FLOAT>(entity, 0x248) > 0.199)
			{
				offset<FLOAT>(entity, 0x244) *= 0.5f;
			}
			if (controller.y < -20 && offset<FLOAT>(entity, 0x248) > 0.199)
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
	if (controller.input.bomb)
	{
		triggerExplosion(offset<FLOAT>(entity, 0x30), offset<FLOAT>(entity, 0x34), 0, BOMB);
	}
}