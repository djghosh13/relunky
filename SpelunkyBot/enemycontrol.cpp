#include "stdafx.h"
#include "enemycontrol.h"
#include "basictools.h"
#include "inputcapture.h"

void AIUpdate(address_t entity)
{
	address_t gameBase = offsetM("Spelunky.exe", 0x1384B4);
	address_t plr = offset(gameBase, 0x440684);

	BYTE &isControlled = offset<BYTE>(entity, 0x15C);
	if (!isControlled) return;

	// Make enemy harmless to player
	offset<BYTE>(entity, 0x1F3) = TRUE;
	// Stun player
	offset<BYTE>(plr, 0x211) = TRUE;
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
		if (abs(controller::x) > 20)
		{
			offset<BYTE>(entity, 0x9D) = controller::x < 0;
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
		offset<FLOAT>(target, 0x30) = offset<FLOAT>(entity, 0x30) + 0.5f * controller::x;
		offset<FLOAT>(target, 0x34) = offset<FLOAT>(entity, 0x34) + 0.5f * controller::y;
		// If spider, has jump
		if (eType == SPIDER)
		{
			int &timer = offset<INT>(entity, 0x154);
			timer = (timer > 30) ? 30 : (timer < 10) ? 10 : timer;
			if (controller::jump && timer == 10)
			{
				timer = 0;
			}
			if (controller::y > 20 && offset<FLOAT>(entity, 0x248) > 0.199)
			{
				offset<FLOAT>(entity, 0x244) *= 0.5f;
			}
			if (controller::y < -20 && offset<FLOAT>(entity, 0x248) > 0.199)
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
	if (controller::bomb)
	{
		triggerExplosion(offset<FLOAT>(entity, 0x30), offset<FLOAT>(entity, 0x34), 0, BOMB);
	}
}

void possessNearestEnemy(list<LPCWSTR> args)
{
	address_t gameBase = offsetM("Spelunky.exe", 0x1384B4);
	address_t plr = offset(gameBase, 0x440684);

	if (offset<BYTE>(plr, 0x211)) return;

	address_t *entities = offset<address_t *>(gameBase, 0x30);
	address_t nearest = NULL;
	float nearestDist = 0.0;
	address_t entity = NULL;

	for (size_t i = 0; i < offset<size_t>(entities, 0x7810); i++)
	{
		entity = entities[i];
		float dx = offset<float>(entity, 0x30) - offset<float>(plr, 0x30);
		float dy = offset<float>(entity, 0x34) - offset<float>(plr, 0x34);
		float dist = sqrtf(dx * dx + dy * dy);

		if (offset<UINT>(entity, 0x08) == 3 && (nearest == NULL || dist < nearestDist))
		{
			nearest = entity;
			nearestDist = dist;
		}
	}

	if (nearest == nullptr)
	{
		dev::println(L"Could not find any enemies nearby");
	}
	else
	{
		dev::println(L"Possessing nearby %s", getEntityName(offset<entity_t>(nearest, 0x0C)));
		offset<BYTE>(nearest, 0x15C) = TRUE;
	}
}

void releaseAllEnemies(list<LPCWSTR> args)
{
	address_t gameBase = offsetM("Spelunky.exe", 0x1384B4);
	address_t plr = offset(gameBase, 0x440684);
	address_t *entities = offset<address_t *>(gameBase, 0x30);
	address_t entity = NULL;
	offset(offsetM("Spelunky.exe", 0x138558), 0x30) = plr;
	long numReleased = 0;

	for (size_t i = 0; i < offset<size_t>(entities, 0x7810); i++)
	{
		entity = entities[i];

		if (offset<UINT>(entity, 0x08) == 3 && offset<BYTE>(entity, 0x15C))
		{
			offset<BYTE>(entity, 0x15C) = FALSE;
			numReleased++;
		}
	}

	if (numReleased == 0)
	{
		dev::println(L"No enemies currently possessed");
	}
	else
	{
		dev::println(L"Released control of %ld enemies", numReleased);
	}
}