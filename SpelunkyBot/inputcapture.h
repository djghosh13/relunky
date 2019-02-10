#pragma once

#include "stdafx.h"
#include "assembly.h"

// Controller input
namespace controller
{
	using assembly::address_t;

	extern address_t controls;
	extern BYTE jump, bomb, action, rope, door, run, pause, journal;
	extern int x, y;
	
	void updateController();
}

// Keyboard input
namespace keyboard
{
	using assembly::address_t;

	extern BYTE pressed[256];

	void onKeyEvent(address_t);
	void updateKeyInput();
}