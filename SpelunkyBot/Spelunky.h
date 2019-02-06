#pragma once

typedef BYTE *Address;
struct FunctionCall;

class Spelunky
{
	bool connected;
	Address baseAddress;
	HANDLE process;

	void findHooks(void);
public:
	Spelunky();
	~Spelunky();

	bool isConnected(void);
	const Address &BASE;
	void setBaseAddress(Address);

	BYTE *read_memory(Address, size_t, BYTE *);
	void write_memory(Address, size_t, BYTE *, bool = false);
	Address alloc_memory(size_t);

	void inject_call(Address, size_t, FunctionCall, char = 1);
};

BYTE *jmp_code(Address, Address, BYTE *);
BYTE *call_code(Address, Address, BYTE *);

struct FunctionCall
{
	Address function;
	UINT nArgs;
	BYTE args[8];
	BOOL baseParam = FALSE;
};

enum Register
{
	EAX = 0,
	ECX = 1,
	EDX = 2,
	EBX = 3,
	ESP = 4,
	EBP = 5,
	ESI = 6,
	EDI = 7
};