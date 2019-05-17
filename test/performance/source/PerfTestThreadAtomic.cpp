////////////////////////////////////////////////////////////////////////
// PerfTestThreadAtomic.cpp
// 
// Copyright (c) 2014, Electronic Arts Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////

#include "benchmarkenvironment/results.h"
#include "benchmarkenvironment/statistics.h"
#include "benchmarkenvironment/timer.h"

#include "eathread/eathread_atomic.h"
#include "eathread/eathread_thread.h"
#include "EATest/EATest.h"

#include "PerfTestThread.h"

using namespace EA::Thread;

#if EA_THREADS_AVAILABLE

// ------------------------------------------------------------------------
// Each thread that is involved in these tests will get its own independent timer, and a 
// shared atomic variable. With each thread doing its own timing, we should be able to 
// remove scheduling noise to the greatest degree possible.
//
struct AtomicAndTimer 
{
	static AtomicInt32 mAtomicInteger;
	benchmarkenvironment::Timer mLocalTimer;
};

AtomicInt32 AtomicAndTimer::mAtomicInteger = 0;

// ------------------------------------------------------------------------
//
static intptr_t AtomicIntMath(void* pWorkStructure) 
{
	AtomicAndTimer& at = *static_cast<AtomicAndTimer*>(pWorkStructure);

	// A series of atomic operations copied from the atomic unit test
	at.mLocalTimer.Start();

	++(at.mAtomicInteger);
	--(at.mAtomicInteger);
	(at.mAtomicInteger) += 5;
	(at.mAtomicInteger) -= 5;
	(at.mAtomicInteger)++;
	(at.mAtomicInteger)--;

	at.mLocalTimer.Stop();

	return 0;
}

// --------------------------------------------------------------------------
//
// todo:  come up with a performance test for CAS operations.
// static intptr_t AtomicIntCompareAndSwap(void* pArgs)
// {
//     AtomicAndTimer& at = static_cast<AtomicAndTimer&>(*pArgs);
// 
//     toCAS->mLocalTimer.Start();
//     toCAS->mAtomicInteger.SetValueConditional(0, 1);
//     toCAS->mLocalTimer.Stop();
// 
//     return 0;
// }

// ---------------------------------------------------------------------------
//
void AtomicIntPerfTest(ThreadEntryFunction ptestFunc, benchmarkenvironment::Sample &sample)
{
	static const int kNumThreads = 8;

	EA::Thread::Thread::Status threadExitStatus;
	AtomicAndTimer threadTimers[kNumThreads];
	float totalTime = 0.0;

	Thread threads[kNumThreads];

	for (int i = 0; i < kNumThreads; ++i) 
		threads[i].Begin(ptestFunc, &threadTimers[i]);

	for (int i = 0; i < kNumThreads; ++i)
	{
		threadExitStatus = threads[i].WaitForEnd(GetThreadTime() + 30000);
		
		// Only take the results if the thread exited properly.
		if (threadExitStatus != Thread::kStatusRunning)
			totalTime += threadTimers[i].mLocalTimer.AsSeconds();
	}

	sample.AddElement(totalTime);
}

// ------------------------------------------------------------------------------
// NOTE If you wish to add tests here. You will need to add your test function to the 
//      testFunctions vector, as well as the name of the test to the testNames vector.
void PerfTestThreadAtomic(benchmarkenvironment::Results &results, EA::IO::FileStream* pPerformanceLog)
{
	using namespace eastl;
	using namespace benchmarkenvironment;

	const int kNumSamples = 50;

	vector<ThreadEntryFunction> testFunctions;
	testFunctions.push_back(&AtomicIntMath);
	// The compare-and-swap test has been deactivated until we can think of a better way to test that functionality.
	//testFunctions.push_back(&AtomicIntCompareAndSwap));

	vector<string> testNames;
	testNames.push_back("Atomic Math Test");
	//testNames.push_back("Atomic Compare and Swap Test");

	vector<Sample> samples;
	for (unsigned int i = 0; i < testFunctions.size(); ++i)
		samples.push_back(Sample(kNumSamples));

	for (unsigned int i = 0; i < testFunctions.size(); ++i)
	{
		for (int j = 0; j < kNumSamples; ++j)
			AtomicIntPerfTest(testFunctions[i], samples[i]);

		AddRowToResults(results, samples[i], testNames.at(i));
		WriteToLogFile(pPerformanceLog, "%s,%g,%g\r\n", testNames[i].c_str(), samples[i].GetMean(), samples[i].GetVariance());
	}
}

#endif // EA_THREADS_AVAILABLE
