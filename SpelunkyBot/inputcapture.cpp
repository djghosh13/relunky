#include "stdafx.h"
#include "inputcapture.h"

namespace controller
{
	using assembly::address_t;
	using assembly::offset;
	using assembly::offsetM;

	address_t controls = NULL;
	BYTE jump, bomb, action, rope, door, run, pause, journal;
	int x, y;

	static BYTE pjump, pbomb, paction, prope, pdoor, prun, ppause, pjournal;
	
	void updateController()
	{
		x = offset<int>(controls, 0x1C);
		y = offset<int>(controls, 0x20);

		jump = offset<BYTE>(controls, 0x00) && !pjump;
		bomb = offset<BYTE>(controls, 0x01) && !pbomb;
		action = offset<BYTE>(controls, 0x02) && !paction;
		rope = offset<BYTE>(controls, 0x03) && !prope;
		door = offset<BYTE>(controls, 0x05) && !pdoor;
		run = offset<BYTE>(controls, 0x0D) && !prun;
		pause = offset<BYTE>(controls, 0x10) && !ppause;
		journal = offset<BYTE>(controls, 0x11) && !pjournal;

		pjump = offset<BYTE>(controls, 0x00);
		pbomb = offset<BYTE>(controls, 0x01);
		paction = offset<BYTE>(controls, 0x02);
		prope = offset<BYTE>(controls, 0x03);
		pdoor = offset<BYTE>(controls, 0x05);
		prun = offset<BYTE>(controls, 0x0D);
		ppause = offset<BYTE>(controls, 0x10);
		pjournal = offset<BYTE>(controls, 0x11);
	}
}

namespace keyboard
{
	using assembly::address_t;
	using assembly::offset;

	BYTE pressed[256];
	static std::queue<BYTE> eventQueue = std::queue<BYTE>();

	void onKeyEvent(address_t stackptr)
	{
		BYTE key = offset<BYTE>(stackptr, 0x10);
		BYTE pressed = offset<BYTE>(stackptr, 0x14);
		if (pressed & 0x03)
		{
			eventQueue.push(key);
		}
	}

	void updateKeyInput()
	{
		memset(pressed, FALSE, sizeof(pressed));
		while (!eventQueue.empty())
		{
			pressed[eventQueue.front()] = TRUE;
			eventQueue.pop();
		}
	}
}