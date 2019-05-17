///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include <eathread/internal/config.h>
#include <eathread/eathread_atomic.h>


#if EATHREAD_HAS_EMULATED_AND_NATIVE_ATOMICS

#include <eathread/internal/atomic.h>
#include <eathread/eathread_futex.h>
#include <stdio.h>

#if __APPLE__
	#include <libkern/OSAtomic.h>
#endif

// Currently iPhone/iOS defaults to emulated atomics. The reason for this is that
// some older iPhone firmware versions have broken 64 bit atomics which are useless.
// The only other platform where it is possible to switch between native and emulated
// atomics is desktop OS X, and we default to native atomics there.
#if !defined(EATHREAD_DEFAULT_TO_EMULATED_ATOMIC64)
	#define EATHREAD_DEFAULT_TO_EMULATED_ATOMIC64 0
#endif


namespace EA
{
	namespace Thread
	{
		static Futex gEmulatedAtomicFutex;


		static inline void PrintEmulationWarningMessage()
		{
			#if EAT_ASSERT_ENABLED
				static bool gHavePrintedEmulationWarningMessage = false;

				if (!gHavePrintedEmulationWarningMessage)
				{
					printf("WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING\n"
							"    EAThread is currently configured to use emulated 64-bit atomics.\n"
							"    This can be a performance hazard on architectures that natively support\n"
							"    these instructions. If you know that this platform supports native 64-bit\n"
							"    atomics, call EA::Thread::SetAtomic64Implementation(kAtomic64Native).\n"
							"    If the platform does not currently support 64-bit atomics then disregard\n"
							"    this message.\n");
					gHavePrintedEmulationWarningMessage = true;
				}
			#endif
		}


		static int64_t AtomicAdd64Emulated(volatile int64_t *ptr, int64_t value)
		{
			AutoFutex autoFutex(gEmulatedAtomicFutex);
			PrintEmulationWarningMessage();

			const int64_t oldValue = *ptr;
			const int64_t newValue = oldValue + value;
			*ptr = newValue;

			return newValue;
		}


		static int64_t AtomicGetValue64Emulated(volatile int64_t *ptr)
		{
			AutoFutex autoFutex(gEmulatedAtomicFutex);
			PrintEmulationWarningMessage();

			return *ptr;
		}


		static int64_t AtomicSetValue64Emulated(volatile int64_t *ptr, int64_t value)
		{
			AutoFutex autoFutex(gEmulatedAtomicFutex);
			PrintEmulationWarningMessage();

			const int64_t oldValue = *ptr;
			*ptr = value;

			return oldValue;
		}


		static bool AtomicSetValueConditional64Emulated(volatile int64_t *ptr, int64_t value, int64_t condition)
		{
			AutoFutex autoFutex(gEmulatedAtomicFutex);
			PrintEmulationWarningMessage();

			const int64_t oldValue = *ptr;
			if (oldValue == condition)
			{
				*ptr = value;
				return true;
			}

			return false;
		}


		#if __APPLE__
			static int64_t AtomicAdd64Native(volatile int64_t *ptr, int64_t value)
			{
				return OSAtomicAdd64(value, ptr);
			}


			static int64_t AtomicSetValue64Native(volatile int64_t *ptr, int64_t value)
			{
				int64_t old;
				do
				{
					old = *ptr;
				} while (!OSAtomicCompareAndSwap64(old, value, ptr));
				return old;
			}


			static bool AtomicSetValueConditional64Native(volatile int64_t *ptr, int64_t value, int64_t condition)
			{
				return OSAtomicCompareAndSwap64(condition, value, ptr);
			}


			static int64_t AtomicGetValue64Native(volatile int64_t *ptr)
			{
				return AtomicAdd64Native(ptr, 0);
			}
		#endif


		#if EATHREAD_DEFAULT_TO_EMULATED_ATOMIC64
			AtomicAdd64Function                 AtomicAdd64                 = AtomicAdd64Emulated;
			AtomicGetValue64Function            AtomicGetValue64            = AtomicGetValue64Emulated;
			AtomicSetValue64Function            AtomicSetValue64            = AtomicSetValue64Emulated;
			AtomicSetValueConditional64Function AtomicSetValueConditional64 = AtomicSetValueConditional64Emulated;
		#else
			AtomicAdd64Function                 AtomicAdd64                 = AtomicAdd64Native;
			AtomicGetValue64Function            AtomicGetValue64            = AtomicGetValue64Native;
			AtomicSetValue64Function            AtomicSetValue64            = AtomicSetValue64Native;
			AtomicSetValueConditional64Function AtomicSetValueConditional64 = AtomicSetValueConditional64Native;
		#endif


		void SetAtomic64Implementation(Atomic64Implementation implementation)
		{
			if (implementation == kAtomic64Emulated)
			{
				AtomicAdd64                 = AtomicAdd64Emulated;
				AtomicGetValue64            = AtomicGetValue64Emulated;
				AtomicSetValue64            = AtomicSetValue64Emulated;
				AtomicSetValueConditional64 = AtomicSetValueConditional64Emulated;
			}
			else
			{
				AtomicAdd64                 = AtomicAdd64Native;
				AtomicGetValue64            = AtomicGetValue64Native;
				AtomicSetValue64            = AtomicSetValue64Native;
				AtomicSetValueConditional64 = AtomicSetValueConditional64Native;
			}
		}
	}
}

#endif

