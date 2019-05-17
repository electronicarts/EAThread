////////////////////////////////////////////////////////////////////////
// PerfTestThread.cpp
// 
// Copyright (c) 2014, Electronic Arts Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////

#include <benchmarkenvironment/test.h>
#include <coreallocator/icoreallocator_interface.h>
#include <EAMain/EAEntryPointMain.inl>
#include <EAStdC/EAString.h>
#include <EATest/EATest.h>
#include <eathread/eathread.h>
#include <MemoryMan/CoreAllocator.inl>
#include <MemoryMan/MemoryMan.inl>

#include "PerfTestThread.h"

using namespace benchmarkenvironment;

// TODO: Releases of benchmarkenvironmrnt with version numbers higher than 4.00
//       will include a new macro that will replace most of this code. Presently, that macro
//       cannot be used because it redefines operator new*.

// The "BENCHMARKENVIRONMENT_TESTFUNCTION" macro redefines the new and new[] operators here, which causes a compiler error.
// This code has been removed.
EA_PREFIX_ALIGN(128) char gWorkMemory[BENCHMARKENVIRONMENT_WORKMEMORY_SIZE] EA_POSTFIX_ALIGN(128);
EA_PREFIX_ALIGN(128) char gResultMemory[BENCHMARKENVIRONMENT_RESULTMEMORY_SIZE] EA_POSTFIX_ALIGN(128);

int EAMain(int argc, char **argv)
{
	
	Initialize(argc, argv, BENCHMARKENVIRONMENT_STRINGIZE(BENCHMARKENVIRONMENT_DEFAULT_TABLE_IDENTIFIER));
	SetFlagsValid();
	BenchmarkEnvironmentTestFunction(gWorkMemory, sizeof(gWorkMemory), gResultMemory, sizeof(gResultMemory));
	Complete(BENCHMARKENVIRONMENT_RESULTPASSED);
	return 0;
}

void BenchmarkEnvironmentTestFunction(Address /*workMemory*/, unsigned int /*workSize*/, Address resultMemory,	unsigned int resultSize)
{

	typedef void(*PerfTestFunction)(Results&, EA::IO::FileStream*);
	typedef eastl::vector<PerfTestFunction> PerfTestFunctions;

	PerfTestFunctions perfTestFunctions;
	perfTestFunctions.push_back(&PerfTestThreadAtomic);
	perfTestFunctions.push_back(&PerfTestThreadSemaphore);

	// EATHREAD_PERFORMANCE_LOG_FILENAME is set in the build file. Right now it should be ${config}-performance_log.txt
	EA::IO::FileStream performanceLog(EATHREAD_PERFORMANCE_LOG_FILENAME);

	if (!gIsAutomatedDeferredRun)
	{
		// If this is a local run, create a performance log for the evaluation function to look at.
		performanceLog.Open(EA::IO::kAccessFlagWrite, EA::IO::kCDCreateAlways);

		// This loggin is mostly here so that we have a way to know that this code is NOT running
		// in the build farm context. It could be removed once that has been confirmed.
		EA::UnitTest::Report("Local Execution -> Performance Logging Enabled\n");
	}

	// Outline the structure of the results table
	// We can do this out here because all of the tests will submit to the same table.
	Results resultsTable(resultMemory, resultSize);
	const int kNumColumns = 5;
	resultsTable.DescribeTableBegin(kNumColumns);
	resultsTable.AddStringField("Test Name");
	resultsTable.AddDoubleField("Mean", "s");
	resultsTable.AddDoubleField("Min", "s");
	resultsTable.AddDoubleField("Max", "s");
	resultsTable.AddDoubleField("Var", "s^2");
	resultsTable.DescribeTableEnd();

	for (unsigned int i = 0; i < perfTestFunctions.size(); ++i)
	{
		if (gIsAutomatedDeferredRun)
			perfTestFunctions[i](resultsTable, NULL);
		else
			perfTestFunctions[i](resultsTable, &performanceLog);
	}

	if (!gIsAutomatedDeferredRun)
	{
		performanceLog.Close();

		EA::UnitTest::Report("Log file written.\n");
	}

	return;
}


