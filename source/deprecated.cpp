///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include <eathread/internal/config.h>
#include "eathread/internal/deprecated.h"
#include <eathread/eathread.h>
#include <stdio.h>

namespace EA {
namespace Thread {
		
EATHREADLIB_API void WarnOnce(bool* pHasTriggered, const char* message)
{
	EA_UNUSED(pHasTriggered);
	EA_UNUSED(message);
#if EAT_ASSERT_ENABLED
	if (*pHasTriggered == false)
	{
		*pHasTriggered = true;
		// TODO: redirect to debug printing in EAStdC once we have a dependency
		printf("[EAThread] ***Warning*** %s\n", message);
	}
#endif
}

EATHREADLIB_API void ErrorOnce(bool* pHasTriggered, const char* message)
{
	EA_UNUSED(pHasTriggered);
	EA_UNUSED(message);
#if EAT_ASSERT_ENABLED
	if (*pHasTriggered == false)
	{
		*pHasTriggered = true;
		EAT_FAIL_MSG(message);
	}
#endif
}

}} // end namespace EA::Thread


