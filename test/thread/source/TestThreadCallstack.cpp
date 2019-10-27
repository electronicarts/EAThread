///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include "TestThread.h"
#include <EATest/EATest.h>
#include <eathread/eathread.h>
#include <eathread/eathread_callstack.h>
#include <eathread/eathread_callstack_context.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/fixed_string.h>
#include <EATest/EATest.h>
#include <EAStdC/EAStopwatch.h>
#include <EAStdC/EASprintf.h>
#include <EASTL/set.h>
#include <eathread/eathread_thread.h>
#include <eathread/eathread_sync.h>
#include <eathread/eathread_semaphore.h>


#ifdef EA_PLATFORM_MICROSOFT
EA_DISABLE_ALL_VC_WARNINGS()
#include <Windows.h>
EA_RESTORE_ALL_VC_WARNINGS()
#endif


struct CallstackTestInfo
{
	eastl::fixed_vector<void*, 32> mAddressSet;
};

struct CallstackTestThreadContext
{
	CallstackTestInfo* 				mCallstackTestInfo;
	int*							mnErrorCount;
	EA::Thread::CallstackContext	mCallstackContext;
	EA::Thread::Semaphore			mThreadControlSema;
	EA::Thread::Semaphore			mTestStartSema;
};

#define EACALLSTACK_TEST_FUNCTION_LINKAGE
EA_NO_INLINE void TestCallstack01(CallstackTestInfo& callstackInfo, int& nErrorCount);
EA_NO_INLINE void TestCallstack02(CallstackTestInfo& callstackInfo, int& nErrorCount);
EA_NO_INLINE void TestCallstack03(CallstackTestInfo& callstackInfo, int& nErrorCount);
EA_NO_INLINE void TestCallstack04(CallstackTestInfo& callstackInfo, int& nErrorCount);
EA_NO_INLINE void TestCallstackWithContext02(CallstackTestThreadContext* context);
EA_NO_INLINE void TestCallstackWithContext01(CallstackTestThreadContext* context);
EA_NO_INLINE intptr_t TestCallstackContextThreadFunc(void* context);


static void VerifyCallstack(CallstackTestInfo& callstackInfo, EA::Thread::CallstackContext* context, int& nErrorCount)
{
	// We don't do a rigorous per entry ordered match because we've found that 
	// compiler optimizations get in the way of testing that reliably.
	void*        addressArray[24] = {};
	size_t       addressCount = EA::Thread::GetCallstack(addressArray, EAArrayCount(addressArray), context);
	eastl_size_t matchCount = 0;
	
	for(eastl_size_t i = 0, iEnd = callstackInfo.mAddressSet.size(); i != iEnd; i++)
	{
		void*  p = callstackInfo.mAddressSet[i];
		size_t j;

		for(j = 0; j < addressCount; j++)
		{
			if(abs((int)((intptr_t)addressArray[j] - (intptr_t)p)) < 512)
			{
				matchCount++;
				break;
			}
		}
	}

	if(matchCount != callstackInfo.mAddressSet.size())
	{
		eastl::fixed_string<char, 256> sExpectedCallstack;
		eastl::fixed_string<char, 256> sReportedCallstack;

		for(eastl_size_t i = 0; i < callstackInfo.mAddressSet.size(); i++)
			sExpectedCallstack.append_sprintf("%p ", callstackInfo.mAddressSet[i]);

		for(size_t i = 0; i < addressCount; i++)
			sReportedCallstack.append_sprintf("%p ", addressArray[i]);

			EATEST_VERIFY_F(matchCount == callstackInfo.mAddressSet.size(), "VerifyCallstack failure. Each member from the expected callstack should be present (+/- 512 bytes) in the reported callstack.\n    Expected callstack %s\n    Reported callstack %s", sExpectedCallstack.c_str(), sReportedCallstack.c_str());
	}

	EA::UnitTest::NonInlinableFunction();
}

EA_DISABLE_VC_WARNING(4740);	// flow in or out of inline asm code suppresses global optimization

EA_NO_INLINE EACALLSTACK_TEST_FUNCTION_LINKAGE void TestCallstackWithContext02(CallstackTestThreadContext* context)
{
	int nErrorCount = 0;
	EA::UnitTest::NonInlinableFunction();
	
	EATEST_VERIFY(EA::Thread::GetCallstackContextSysThreadId(context->mCallstackContext, (intptr_t)EA::Thread::GetSysThreadId()));
	context->mnErrorCount += nErrorCount;

	context->mTestStartSema.Post();
	context->mThreadControlSema.Wait();
}

EA_NO_INLINE EACALLSTACK_TEST_FUNCTION_LINKAGE void TestCallstackWithContext01(CallstackTestThreadContext* context)
{
	void* pAddress;

	EA::UnitTest::NonInlinableFunction();
	EAGetInstructionPointer(pAddress);
	context->mCallstackTestInfo->mAddressSet.push_back(pAddress);

	// calling out function through a conditionally set functor to help with optimizations from inlining this code
	TestCallstackWithContext02(context);
}

EA_NO_INLINE intptr_t TestCallstackContextThreadFunc(void* context)
{
	CallstackTestThreadContext* threadContext = (CallstackTestThreadContext*)context;

	threadContext->mThreadControlSema.Wait();

	void* pAddress;
	EAGetInstructionPointer(pAddress);
	threadContext->mCallstackTestInfo->mAddressSet.push_back(pAddress);

	EA::UnitTest::NonInlinableFunction();
	TestCallstackWithContext01(threadContext);

	return 0;
}

EA_NO_INLINE EACALLSTACK_TEST_FUNCTION_LINKAGE void TestCallstack04(CallstackTestInfo& callstackInfo, int& nErrorCount)
{
	void* pAddress;
	EAGetInstructionPointer(pAddress);
	callstackInfo.mAddressSet.push_back(pAddress);

	VerifyCallstack(callstackInfo, nullptr, nErrorCount);

	EA::UnitTest::NonInlinableFunction();
}

EA_NO_INLINE EACALLSTACK_TEST_FUNCTION_LINKAGE void TestCallstack03(CallstackTestInfo& callstackInfo, int& nErrorCount)
{
	void* pAddress;
	EAGetInstructionPointer(pAddress);
	callstackInfo.mAddressSet.push_back(pAddress);

	VerifyCallstack(callstackInfo, nullptr, nErrorCount);
	TestCallstack04(callstackInfo, nErrorCount);
	EA::UnitTest::NonInlinableFunction();
}

EA_NO_INLINE EACALLSTACK_TEST_FUNCTION_LINKAGE void TestCallstack02(CallstackTestInfo& callstackInfo, int& nErrorCount)
{
	void* pAddress;
	EAGetInstructionPointer(pAddress);
	callstackInfo.mAddressSet.push_back(pAddress);

	VerifyCallstack(callstackInfo, nullptr, nErrorCount);
	TestCallstack03(callstackInfo, nErrorCount);
	EA::UnitTest::NonInlinableFunction();
}

EA_NO_INLINE EACALLSTACK_TEST_FUNCTION_LINKAGE void TestCallstack01(CallstackTestInfo& callstackInfo, int& nErrorCount)
{
	void* pAddress;
	EAGetInstructionPointer(pAddress);
	callstackInfo.mAddressSet.push_back(pAddress);

	VerifyCallstack(callstackInfo, nullptr, nErrorCount);
	TestCallstack02(callstackInfo, nErrorCount);
	EA::UnitTest::NonInlinableFunction();
}

EA_RESTORE_VC_WARNING();


/// EATHREAD_GETCALLSTACK_RELIABLE
///
/// Defined as 0 or 1
/// Identifies whether we can rely on the results of GetCallstack for the purposes
/// of this unit test. 
#if !defined(EATHREAD_GETCALLSTACK_RELIABLE)
	#if EATHREAD_GETCALLSTACK_SUPPORTED
		#if defined(EA_PLATFORM_WINRT) && defined(EA_PROCESSOR_X86)  // WinRT-x86 does not provide usable callstacks so we avoid tracing them on this platform.
			#define EATHREAD_GETCALLSTACK_RELIABLE 0
		#else
			#define EATHREAD_GETCALLSTACK_RELIABLE 1
		#endif
	#else
		#define EATHREAD_GETCALLSTACK_RELIABLE 0
	#endif
#endif

static bool IsRoughlyEqualAddress(void* a, void* b)
{
	static const uintptr_t kMaxBytesDist = 512;
	if ( ((uintptr_t)a -(uintptr_t)b)  <= kMaxBytesDist)
	{
		return true;
	}
	else if (((uintptr_t)b - (uintptr_t)a) <= kMaxBytesDist)
	{
		return true;
	}
	else
	{
		return false;
	}
}

EA_NO_INLINE EACALLSTACK_TEST_FUNCTION_LINKAGE  int TestRemoteThreadContextVsCallstack()
{
	int nErrorCount(0);

	struct CallstackTestThread : public EA::Thread::IRunnable
	{
		EA::Thread::Thread           mThread;
		EA::Thread::ThreadParameters mParameters;
		EA::Thread::Semaphore        mEndSemaphore;
		EA::Thread::Semaphore		mStartSemaphore;
		char                         mThreadName[16];
		uint64_t                     mCounter;
		volatile bool                mbShouldRun;
		void*					mAddressCallstackArray[64];

		CallstackTestThread() : mThread(), mParameters(), mEndSemaphore(0), mStartSemaphore(0), mCounter(0), mbShouldRun(true) 
		{
			eastl::fill(eastl::begin(mAddressCallstackArray), eastl::end(mAddressCallstackArray), (void*)nullptr);
		}

		CallstackTestThread(const CallstackTestThread&) {}
		void operator=(const CallstackTestThread&) {}


		intptr_t Run(void*)
		{
			EA::Thread::GetCallstack(mAddressCallstackArray, EAArrayCount(mAddressCallstackArray));
			mStartSemaphore.Post();
			mEndSemaphore.Wait();
			return 0;
		}
	};


	CallstackTestThread remoteThread;
	remoteThread.mThread.Begin(&remoteThread, NULL, &remoteThread.mParameters);

	// make sure this thread is running
	remoteThread.mStartSemaphore.Wait();

	// context
	void*        addressContextArray[64] = {nullptr};
	auto threadId = remoteThread.mThread.GetId();

	{
#if (defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_XBOXONE)) && !defined(EA_PLATFORM_MINGW)
		// suspend the target thread to make sure we get a coherent callstack
		bool wasSuspended = (::SuspendThread(threadId) != ((DWORD)-1)); // fail is (DWORD)-1
#endif

		EA::Thread::CallstackContext callstackContext;
		if (EA::Thread::GetCallstackContext(callstackContext, (intptr_t)threadId))
		{
			EA::Thread::GetCallstack(addressContextArray, EAArrayCount(addressContextArray), &callstackContext);
		}

#if (defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_XBOXONE)) && !defined(EA_PLATFORM_MINGW)
		// resume the target thread as needed
		if (wasSuspended)
		{
			::ResumeThread(threadId);
		}
#endif
	}
	remoteThread.mEndSemaphore.Post();

	// make sure every address in the local callstack is in the remote. (remote is a superset of function calls because of suspended in kernel)
	for (void* localAddress: remoteThread.mAddressCallstackArray)
	{
		if (eastl::find_if(eastl::begin(addressContextArray), eastl::end(addressContextArray),
			[=](void* a) { return  IsRoughlyEqualAddress(a, localAddress); }) == eastl::end(addressContextArray))
		{
			nErrorCount++;
		}
	}

	remoteThread.mThread.WaitForEnd();

	return nErrorCount;
}

int TestThreadCallstack()
{
	int nErrorCount(0);

	#if EATHREAD_GETCALLSTACK_RELIABLE
		{
			EA::Thread::InitCallstack();

			CallstackTestInfo info;
			TestCallstack01(info, nErrorCount);
			EA::Thread::ShutdownCallstack();
		}

		#if defined(EA_PLATFORM_WIN64) || defined(EA_PLATFORM_XBOXONE) ||  defined(EA_PLATFORM_PS4)
		// This test will spawn a thread which will grab its own context and provide it to the main thread 
		// to use when generating a callstack. We use semaphores to control the created thread to ensure 
		// the thread is alive while we call GetCallstack() inside of VerifyCallstack()
		{
			EA::Thread::InitCallstack();

			EA::Thread::Thread testThread;
			CallstackTestInfo info;
			CallstackTestThreadContext threadContext;
			threadContext.mCallstackTestInfo = &info;
			threadContext.mnErrorCount = &nErrorCount;

			testThread.Begin(TestCallstackContextThreadFunc, (void*)&threadContext);
			while (testThread.GetStatus() != EA::Thread::Thread::kStatusRunning)
			{
				EA::Thread::ThreadSleep();
			}
			
			// Let the test thread proceed in generating test data
			threadContext.mThreadControlSema.Post();

			// Wait until the test thread is done entering test data
			threadContext.mTestStartSema.Wait();

			// Grab the context of the testThread and verify the callstack is what we expect
			VerifyCallstack(*threadContext.mCallstackTestInfo, &threadContext.mCallstackContext, nErrorCount);

			// Let the test thread finish
			threadContext.mThreadControlSema.Post();
			testThread.WaitForEnd();

			EA::Thread::ShutdownCallstack();
		}

		{
			EA::Thread::InitCallstack();

			const int numErrorInRemoteTest = TestRemoteThreadContextVsCallstack();
			#if defined(EA_PLATFORM_PS4) // We know that kettle cannot do remote callstacks. This is just to check that it does not crash when attempting to do a remote callstack
				EATEST_VERIFY(numErrorInRemoteTest != 0);
			#else
				nErrorCount += numErrorInRemoteTest;
			#endif

			EA::Thread::ShutdownCallstack();
		}
		#endif
	#endif


	#if defined(EA_PLATFORM_MICROSOFT)
		// bool ThreadHandlesAreEqual(intptr_t threadId1, intptr_t threadId2);
		// uint32_t GetThreadIdFromThreadHandle(intptr_t threadId);
	#endif

	// To do: Implement tests for the following for supporting platforms.
	// bool GetCallstackContext(CallstackContext& context, intptr_t threadId = 0);
	// bool GetCallstackContextSysThreadId(CallstackContext& context, intptr_t sysThreadId = 0);
	// void GetCallstackContext(CallstackContext& context, const Context* pContext = NULL);
	// size_t GetModuleFromAddress(const void* pAddress, char* pModuleFileName, size_t moduleNameCapacity);
	// ModuleHandle GetModuleHandleFromAddress(const void* pAddress);

	// EA::Thread::CallstackContext context;
	// EA::Thread::GetCallstackContext(context, EA::Thread::GetThreadId());
	// EATEST_VERIFY(context.mRIP != 0);
	// EATEST_VERIFY(context.mRSP != 0);

	// To consider: Test SetStackBase. It's not simple because SetStackBase is a backup for 
	// when GetStackBase's default functionality doesn't happen to work.
	// void  SetStackBase(void* pStackBase);
	// void  SetStackBase(uintptr_t pStackBase){ SetStackBase((void*)pStackBase); }

	void* pStackBase  = EA::Thread::GetStackBase();
	void* pStackLimit = EA::Thread::GetStackLimit();

	if(pStackBase && pStackLimit)
	{
		EATEST_VERIFY((uintptr_t)&nErrorCount < (uintptr_t)pStackBase);
		EATEST_VERIFY((uintptr_t)&nErrorCount > (uintptr_t)pStackLimit);
	}

	return nErrorCount;
}
