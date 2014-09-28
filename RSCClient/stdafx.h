// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <process.h>
#include <winsock2.h>
#include <ShellAPI.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>

// TODO: reference additional headers your program requires here
#include "..\Common\RSCSocket.h"
#include "..\Common\RSCMisc.h"
#include "..\Common\RSCPalette.h"
#include "..\Common\RSCRLE.h"
#include "..\Common\RSCRegion.h"
#include "..\Common\RSCPixtangle.h"
#include "..\Common\RSCCrypt.h"

// Entry Point Header File
#include "RSCClient.h"