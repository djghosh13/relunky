#include "stdafx.h"
#include "basictools.h"


DLLExport std::queue<LPCWSTR> debug::log = std::queue<LPCWSTR>();

DLLExport LPCWSTR getEntityName(entity_t eType)
{
	switch (eType)
	{
#define X(name, str, id) case name: return str;
#include "AllEntities.txt"
#undef X
	default:
		return L" ";
	}
}

DLLExport LPVOID spawnEntity(LPVOID BASE, float x, float y, entity_t eType, char onList)
{
	DWORD func = (DWORD)BASE + 0x6FD10;
	DWORD gameBase = offset<DWORD>(BASE, 0x1384B4);
	DWORD dwOnList = (DWORD)onList;
	LPVOID entity = NULL;
	__asm
	{
		push ecx;
		mov ecx, gameBase;
		mov eax, func;
		push dwOnList;
		push eType;
		push y;
		push x;
		call eax;
		pop ecx;
		mov entity, eax;
	}
	return entity;
}

DLLExport void triggerExplosion(LPVOID BASE, float x, float y, UINT player, entity_t bombType)
{
	DWORD func = (DWORD)BASE + 0x86CD0;
	LPVOID gameBase = offset(BASE, 0x1384B4);
	__asm
	{
		pushad;
		mov eax, func;
		push bombType;
		push player;
		push y;
		push x;
		push gameBase;
		call eax;
		popad;
	}
}


DLLExport float *writeText(LPVOID BASE, LPCWSTR pText, float x, float y, char rAlign, float fontSize)
{
	DWORD func = (DWORD)BASE + 0xE8EC0;
	DWORD graphicsBase = offset<DWORD>(offset(BASE, 0x1384B4), 0x50);
	DWORD dwAlign = (DWORD)rAlign;
	DWORD *textColor = nullptr;
	__asm
	{
		pushad;

		push 0;
		push fontSize;
		push dwAlign;
		push y;
		push x;
		push pText;
		mov edi, graphicsBase;
		mov eax, func;
		call eax;

		mov textColor, eax;
		popad;
	}

	return (float *)((BYTE *)textColor + 0x1682C);
}

DLLExport void setTextColor(float *colorPtr, int red, int green, int blue)
{
	colorPtr[0] = red / 255.0f;
	colorPtr[1] = green / 255.0f;
	colorPtr[2] = blue / 255.0f;
}

DLLExport void debug::print(LPCWSTR text)
{
	debug::log.push(text);
	if (debug::log.size() > 10)
		debug::log.pop();
}