///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include <EABase/eabase.h>
#include <eathread/eathread_semaphore.h>


#if EA_OPENKODE_THREADS_AVAILABLE
	#include <time.h>
	#include <errno.h>
	#include <string.h>
	#include <limits.h>
	#include <KD/kd.h>


	EASemaphoreData::EASemaphoreData()
	  : mpSemaphore(NULL),
		mnCount(0),
		mnMaxCount(INT_MAX)
	{
	}


	EA::Thread::SemaphoreParameters::SemaphoreParameters(int initialCount, bool bIntraProcess, const char* /*pName*/)
	  : mInitialCount(initialCount),
		mMaxCount(INT_MAX),
		mbIntraProcess(bIntraProcess) // OpenKODE doesn't support inter-process semaphores.
	{
	}


	EA::Thread::Semaphore::Semaphore(const SemaphoreParameters* pSemaphoreParameters, bool bDefaultParameters)
	{
		if(!pSemaphoreParameters && bDefaultParameters)
		{
			SemaphoreParameters parameters;
			Init(&parameters);
		}
		else
			Init(pSemaphoreParameters);
	}


	EA::Thread::Semaphore::Semaphore(int initialCount)
	{
		SemaphoreParameters parameters(initialCount);
		Init(&parameters);
 
	}


	EA::Thread::Semaphore::~Semaphore()
	{
		const KDint result = kdThreadSemFree(mSemaphoreData.mpSemaphore); (void)result;
		EAT_ASSERT(result == 0);
	}


	bool EA::Thread::Semaphore::Init(const SemaphoreParameters* pSemaphoreParameters)
	{
		if(pSemaphoreParameters)
		{
			mSemaphoreData.mnCount        = pSemaphoreParameters->mInitialCount;
			mSemaphoreData.mnMaxCount     = pSemaphoreParameters->mMaxCount;
			mSemaphoreData.mpSemaphore    = kdThreadSemCreate((KDuint)mSemaphoreData.mnCount);

			return (mSemaphoreData.mpSemaphore != NULL);
		}

		return false;
	}


	int EA::Thread::Semaphore::Wait(const ThreadTime& timeoutAbsolute)
	{
		KDint result = kdThreadSemWait(mSemaphoreData.mpSemaphore);

		if(result != 0)
		{
			EAT_ASSERT(false); // This is an error condition.
			return kResultError;
		}

		EAT_ASSERT(mSemaphoreData.mnCount > 0);
		return (int)mSemaphoreData.mnCount.Decrement(); // AtomicInt32 operation. Note that the value of the semaphore count could change from the returned value by the time the caller reads it. This is fine but the user should understand this.
	}


	int EA::Thread::Semaphore::Post(int count)
	{
		// Some systems have a sem_post_multiple which we could take advantage 
		// of here to atomically post multiple times.
		EAT_ASSERT(mSemaphoreData.mnCount >= 0);

		// It's hard to correctly implement mnMaxCount here, given that it 
		// may be modified by multiple threads during this execution. So if you want
		// to use max-count with an IntraProcess semaphore safely then you need to 
		// post only from a single thread, or at least a single thread at a time.
		
		int currentCount = mSemaphoreData.mnCount;

		// If count would cause an overflow exit early
		if ((mSemaphoreData.mnMaxCount - count) < currentCount)
			return kResultError;

				currentCount += count;

		while(count-- > 0)
		{
			++mSemaphoreData.mnCount;     // AtomicInt32 operation.

			if(kdThreadSemPost(mSemaphoreData.mpSemaphore) != 0)
			{
				--mSemaphoreData.mnCount; // AtomicInt32 operation.
				EAT_ASSERT(false);
				return kResultError;        
			}
		}

		// If all count posts occurred...
		return currentCount; // It's possible that another thread may have modified this value since we changed it, but that's not important.
	}


	int EA::Thread::Semaphore::GetCount() const
	{
		return (int)mSemaphoreData.mnCount;
	}


#endif // EA_PLATFORM_XXX








