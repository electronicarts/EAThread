////////////////////////////////////////////////////////////////////////
// PerfTestThreadSemaphore.cpp
//
// Copyright (c) 2014, Electronic Arts Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////

#include "benchmarkenvironment/results.h"
#include "benchmarkenvironment/statistics.h"
#include "benchmarkenvironment/timer.h"

#include "eathread/eathread_atomic.h"
#include "eathread/eathread_semaphore.h"
#include "eathread/eathread_thread.h"

#include "EATest/EATest.h"

#include "PerfTestThread.h"

using namespace EA::Thread;
using namespace benchmarkenvironment;

// Used to set how many times the contended thread functions run.
const int kNumTestIterations = 10000;

#define THREAD_WAIT_TIMEOUT 15000

/////////////////////////////////////////////////////////////////////////////////////////////////
//                           Producer/Consumer Tests & Test Functions
/////////////////////////////////////////////////////////////////////////////////////////////////

static AtomicInt32 gThreadSyncer = 0;
void DECREMENT_AND_SPINWAIT(AtomicInt32& atomic_var) 
{
	atomic_var--;
	while (atomic_var > 0) 
		;
}

struct ProducerConsumerTestData
{
	static Semaphore* mpEmptySlots;
	static Semaphore* mpFullSlots;
	Timer mThreadLocalTimer;

	ProducerConsumerTestData()
		: mThreadLocalTimer() {}

	static void InitSemaphores(int bufferCapacity)
	{
		SemaphoreParameters temp(bufferCapacity, true, "Producer/Consumer Full");
		mpEmptySlots = new Semaphore(&temp);
		
		SemaphoreParameters temp2(0, false, "Producer/Consumer Empty");
		mpFullSlots = new Semaphore(&temp2);
	}

	static void ResetSemaphores()
	{
		if (mpEmptySlots)
		{
			delete mpEmptySlots;
			mpEmptySlots = NULL;
		}
		if (mpFullSlots)
		{
			delete mpFullSlots;
			mpFullSlots = NULL;
		}
	}
};

// Initialization of the static members...
Semaphore* ProducerConsumerTestData::mpEmptySlots = NULL;
Semaphore* ProducerConsumerTestData::mpFullSlots = NULL;

static intptr_t ProducerThreadFunction(void* pTestData)
{
	ProducerConsumerTestData& testData = *static_cast<ProducerConsumerTestData*>(pTestData);

	testData.mThreadLocalTimer.Start();
	for (int i = 0; i < kNumTestIterations; ++i)
	{
		testData.mpEmptySlots->Wait();
		testData.mpFullSlots->Post();
	}
	testData.mThreadLocalTimer.Stop();

	EAT_ASSERT(testData.mThreadLocalTimer.AsSeconds() >= 0.0);

	return 0;
}

static intptr_t ConsumerThreadFunction(void* pTestData)
{
	ProducerConsumerTestData& testData = *static_cast<ProducerConsumerTestData*>(pTestData);

	testData.mThreadLocalTimer.Start();
	for (int i = 0; i < kNumTestIterations; ++i)
	{
		testData.mpFullSlots->Wait();
		testData.mpEmptySlots->Post();
	}
	testData.mThreadLocalTimer.Stop();

	EAT_ASSERT(testData.mThreadLocalTimer.AsSeconds() >= 0.0);

	return 0;
}

void ProducerConsumerTest(
	Sample& sample,
	ThreadEntryFunction pProducer,
	ThreadEntryFunction pConsumer,
	int bufferCapacity,
	bool isContended) 
{
	const int kThreadArraySize = 12;
	const int kMinThreads = 4;

	const int kNumCores = (isContended ? eastl::min(kThreadArraySize, eastl::max(GetProcessorCount(), kMinThreads)) : 2);
	const int kThreadGroupSize = (isContended ? kNumCores / 2 : 1);

	ProducerConsumerTestData::InitSemaphores(bufferCapacity);

	eastl::vector<ProducerConsumerTestData> producerThreadTimers(kThreadGroupSize);
	eastl::vector<ProducerConsumerTestData> consumerThreadTimers(kThreadGroupSize);
	eastl::vector<Thread> producers(kThreadGroupSize);
	eastl::vector<Thread> consumers(kThreadGroupSize);

	ThreadAffinityMask affinityMask = 1;
	ThreadId newThread;
	for (int i = 0; i < kThreadGroupSize; ++i)
	{
		newThread = producers[i].Begin(pProducer, &producerThreadTimers[i]);
		EA::Thread::SetThreadAffinityMask(newThread, affinityMask);
		affinityMask = affinityMask << 1;

		newThread = consumers[i].Begin(pConsumer, &consumerThreadTimers[i]);
		EA::Thread::SetThreadAffinityMask(newThread, affinityMask);
		affinityMask = affinityMask << 1;
	}

	for (int i = 0; i < kThreadGroupSize; ++i)
	{
		EA::Thread::Thread::Status producerThreadExitStatus = producers[i].WaitForEnd(GetThreadTime() + (THREAD_WAIT_TIMEOUT * kNumCores));
		EA::Thread::Thread::Status consumerThreadExitStatus = consumers[i].WaitForEnd(GetThreadTime() + (THREAD_WAIT_TIMEOUT * kNumCores));

		EA_UNUSED(producerThreadExitStatus);
		EA_UNUSED(consumerThreadExitStatus);

		EAT_ASSERT(producerThreadExitStatus != Thread::kStatusRunning);
		EAT_ASSERT(consumerThreadExitStatus != Thread::kStatusRunning);
	}

	double totalTime = 0.0;
	for (int i = 0; i < kThreadGroupSize; ++i)
	{
		totalTime += producerThreadTimers[i].mThreadLocalTimer.AsSeconds();
		totalTime += consumerThreadTimers[i].mThreadLocalTimer.AsSeconds();
	}

	sample.AddElement(totalTime);

	ProducerConsumerTestData::ResetSemaphores();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                           Scheduler Tests & Test Functions
/////////////////////////////////////////////////////////////////////////////////////////////////

struct SemaphoreTestData
{
	static Semaphore* mpTestSemaphore;
	Timer& mThreadLocalTimer;
	AtomicInt32 mSignal;

	SemaphoreTestData(Timer &timer, int signal = 0)
		: mThreadLocalTimer(timer)
		, mSignal(signal)
	{
	}

	SemaphoreTestData operator=(const SemaphoreTestData& other)
	{
		mThreadLocalTimer = other.mThreadLocalTimer;
		mSignal = other.mSignal;

		return *this;
	}
	
	SemaphoreTestData(const SemaphoreTestData& other) 
		: mThreadLocalTimer(other.mThreadLocalTimer)
		, mSignal(other.mSignal) 
	{
	}
	
	static void setSemaphoreInitialCount(int count)
	{
		SemaphoreParameters params(count, true, "Test Semaphore");
		mpTestSemaphore = new Semaphore(&params);
	}

	static void resetSemaphore()
	{
		if (mpTestSemaphore)
		{
			delete mpTestSemaphore;
			mpTestSemaphore = NULL;
		}
	}
};

Semaphore* SemaphoreTestData::mpTestSemaphore = NULL;

// ------------------------------------------------------------------------------
//

static intptr_t SemaphoreTestSchedulerContendedFunction(void* pTestData)
{
	// In this case, each thread will have its own timer, and share the same semaphore.
	SemaphoreTestData& testData = *static_cast<SemaphoreTestData*>(pTestData);

	DECREMENT_AND_SPINWAIT(gThreadSyncer);
	testData.mThreadLocalTimer.Start();

	for (int i = 0; i < kNumTestIterations; ++i)
	{
		testData.mpTestSemaphore->Wait();
		testData.mpTestSemaphore->Post();
	}

	testData.mThreadLocalTimer.Stop();

	return 0;
}

// ------------------------------------------------------------------------------
//
static intptr_t SemaphoreTestSchedulerUncontendedWakeFunction(void* pTestData)
{
	// Initiate the timer when the wakeup signal is sent.
	SemaphoreTestData& testData = *static_cast<SemaphoreTestData*>(pTestData);

	testData.mThreadLocalTimer.Start();
	testData.mpTestSemaphore->Post();

	return 0;
}

// ------------------------------------------------------------------------------
//
static intptr_t SemaphoreTestSchedulerUncontendedWaitFunction(void* pTestData)
{
	// Immediately go to sleep waiting for the semaphore, then stop the timer 
	// once we wake up.
	SemaphoreTestData& testData = *static_cast<SemaphoreTestData*>(pTestData);

	testData.mSignal++;
	int exitResult = testData.mpTestSemaphore->Wait(GetThreadTime() + THREAD_WAIT_TIMEOUT);
	EAT_ASSERT(exitResult != Semaphore::kResultTimeout);
	EA_UNUSED(exitResult); // Silences gcc and clang warnings 

	testData.mThreadLocalTimer.Stop();

	return 0;
}

// ------------------------------------------------------------------------------
// 
void SemaphoreUncontendedPerfTest(Sample &sample, ThreadEntryFunction pWaitingFunc, ThreadEntryFunction pWakingFunc, int semaphoreInitialCount)
{
	SemaphoreParameters params(semaphoreInitialCount, true, "Uncontended");
	Timer timer;

	SemaphoreTestData sharedData(timer);
	SemaphoreTestData::setSemaphoreInitialCount(semaphoreInitialCount);

	Thread waker;
	Thread sleeper;

	sleeper.Begin(pWaitingFunc, &sharedData);

	// Spin until the sleeping thread runs and blocks on the semaphore.
	while (sharedData.mSignal == 0) {}

	waker.Begin(pWakingFunc, &sharedData);

	EA::Thread::Thread::Status waiterThreadExitStatus = sleeper.WaitForEnd(GetThreadTime() + THREAD_WAIT_TIMEOUT);
	EA::Thread::Thread::Status wakerThreadExitStatus = waker.WaitForEnd(GetThreadTime() + THREAD_WAIT_TIMEOUT);
	
	EA_UNUSED(waiterThreadExitStatus);
	EA_UNUSED(wakerThreadExitStatus);

	EAT_ASSERT(waiterThreadExitStatus != Thread::kStatusRunning && wakerThreadExitStatus != Thread::kStatusRunning);

	sample.AddElement(timer.AsSeconds());

	SemaphoreTestData::resetSemaphore();
}

// ------------------------------------------------------------------------------
// 
void SemaphoreContendedPerfTest(benchmarkenvironment::Sample &sample, ThreadEntryFunction pTestFunc, int semaphoreInitialCount)
{
	// The contended test will always use per-thread timers, since any blocks are
	// a circumstance that would arise in normal use.

	const int kThreadArraySize = 12;
	const int kMinThreads = 4;

	const int kNumCores = eastl::min(kThreadArraySize, eastl::max(GetProcessorCount(), kMinThreads));
	gThreadSyncer = kNumCores;

	SemaphoreTestData::setSemaphoreInitialCount(semaphoreInitialCount);

	eastl::vector<Thread> threads(kNumCores);
	eastl::vector<Timer> timers(kNumCores);
	eastl::vector<SemaphoreTestData> data;
	for (int i = 0; i < kNumCores; ++i)
		data.push_back(SemaphoreTestData(timers[i]));

	for (int i = 0; i < kNumCores; ++i)
	{
		ThreadId newThread = threads[i].Begin(pTestFunc, &data[i]);
		EA::Thread::SetThreadAffinityMask(newThread, ThreadAffinityMask(1 << i));
	}

	double totalTime = 0.0;
	for (int i = 0; i < kNumCores; ++i)
	{
		EA::Thread::Thread::Status threadExitStatus = threads[i].WaitForEnd(GetThreadTime() + (THREAD_WAIT_TIMEOUT * kNumCores));
		EAT_ASSERT(threadExitStatus != Thread::kStatusRunning);
		EA_UNUSED(threadExitStatus);
			
		totalTime += timers[i].AsSeconds();
	}

	sample.AddElement(totalTime);

	SemaphoreTestData::resetSemaphore();
}

// ------------------------------------------------------------------------------
//
void PerfTestThreadSemaphore(Results &results, EA::IO::FileStream* pPerformanceLog)
{
	using namespace eastl;

	const int kNumSamples = 10;
	const int kNumTests = 7;
	
	vector<Sample> samples;
	for (int i = 0; i < kNumTests; ++i)
		samples.push_back(Sample(kNumSamples));

	for (int j = 0; j < kNumSamples; ++j)
		SemaphoreUncontendedPerfTest(samples[0], &SemaphoreTestSchedulerUncontendedWaitFunction, &SemaphoreTestSchedulerUncontendedWakeFunction, 0);
	AddRowToResults(results, samples[0], "Semaphore Wakeup Time");
	WriteToLogFile(pPerformanceLog, "Semaphore Wakeup Time,%g,%g\r\n", samples[0].GetMean(), samples[0].GetVariance());  // Execution is in a local context, so output the results to the log file.

	for (int j = 0; j < kNumSamples; ++j)
		SemaphoreContendedPerfTest(samples[1], &SemaphoreTestSchedulerContendedFunction, 1);
	AddRowToResults(results, samples[1], "Semaphore as Mutex");
	WriteToLogFile(pPerformanceLog, "Semaphore as Mutex,%g,%g\r\n", samples[1].GetMean(), samples[1].GetVariance());

	for (int j = 0; j < kNumSamples; ++j)
		SemaphoreContendedPerfTest(samples[2], &SemaphoreTestSchedulerContendedFunction, 5);
	AddRowToResults(results, samples[2], "Semaphore as 5-way Mutex");
	WriteToLogFile(pPerformanceLog, "Semaphore as 5-way Mutex,%g,%g\r\n", samples[2].GetMean(), samples[2].GetVariance());

	for (int j = 0; j < kNumSamples; ++j)
		ProducerConsumerTest(samples[3], &ProducerThreadFunction, &ConsumerThreadFunction, 1, false);
	AddRowToResults(results, samples[3], "1 P/1 C (1 Thread at once)");
	WriteToLogFile(pPerformanceLog, "1 P/1 C (1 Thread at once),%g,%g\r\n", samples[3].GetMean(), samples[3].GetVariance());

	for (int j = 0; j < kNumSamples; ++j)
		ProducerConsumerTest(samples[4], &ProducerThreadFunction, &ConsumerThreadFunction, 5, false);
	AddRowToResults(results, samples[4], "1 P/1 C (5 Threads at once)");
	WriteToLogFile(pPerformanceLog, "1 P/1 C (5 Threads at once),%g,%g\r\n", samples[4].GetMean(), samples[4].GetVariance());

	for (int j = 0; j < kNumSamples; ++j)
		ProducerConsumerTest(samples[5], &ProducerThreadFunction, &ConsumerThreadFunction, 1, true);
	AddRowToResults(results, samples[5], "1+ P/1+ C (1 Thread at once)");
	WriteToLogFile(pPerformanceLog, "1+ P/1+ C (1 Thread at once),%g,%g\r\n", samples[5].GetMean(), samples[5].GetVariance());

	for (int j = 0; j < kNumSamples; ++j)
		ProducerConsumerTest(samples[6], &ProducerThreadFunction, &ConsumerThreadFunction, 5, true);
	AddRowToResults(results, samples[6], "1+ P/1+ C (5 Threads at once)");
	WriteToLogFile(pPerformanceLog, "1+ P/1+ C (5 Threads at once),%g,%g\r\n", samples[6].GetMean(), samples[6].GetVariance());

	return;
}
