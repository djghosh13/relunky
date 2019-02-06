#pragma once

#include "stdafx.h"

enum entity_t
{
#define X(name, str, id) name = id,
#include "AllEntities.txt"
#undef X
	UNKNOWN
};

inline DLLExport LPCWSTR getEntityName(entity_t eType)
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

inline DLLExport entity_t fromInt(UINT eType)
{
	switch (eType)
	{
#define X(name, str, id) case id: return name;
#include "AllEntities.txt"
#undef X
	default:
		return (entity_t)eType;
	}
}