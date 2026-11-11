// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0502	// Change this to the appropriate value to target other versions of Windows.
#endif						

// We ask windows.h not to define its min() and max() macros since we're using std::min() and std::max()
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "windows.h"

#pragma warning(disable: 4996) // stdlib

// Reference additional headers your program requires here