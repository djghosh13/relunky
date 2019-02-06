// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <cmath>
#include <stdlib.h>
#include <time.h>

// reference additional headers your program requires here
#define DLLExport __declspec(dllexport)
#define DLLImport __declspec(dllimport)