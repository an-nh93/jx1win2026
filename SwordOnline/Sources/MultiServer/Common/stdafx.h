#ifndef __INCLUDE_COMMON_STDAFX_H__
#define __INCLUDE_COMMON_STDAFX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// winsock2.h MUST be before windows.h to avoid C2011/C2375 redefinition errors
#include <winsock2.h>
#include "windows.h"

#endif // __INCLUDE_COMMON_STDAFX_H__