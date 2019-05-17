///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////


#ifndef PERFTESTTHREAD_H
#define PERFTESTTHREAD_H

#include <eathread/eathread_thread.h>
#include <benchmarkenvironment/results.h>
#include <benchmarkenvironment/statistics.h>
#include <EAIO/EAFileStream.h>
#include <EAStdC/EASprintf.h>

typedef intptr_t(*ThreadEntryFunction)(void*);

void PerfTestThreadAtomic(benchmarkenvironment::Results &results, EA::IO::FileStream* pLogFileStream);
void PerfTestThreadSemaphore(benchmarkenvironment::Results &results, EA::IO::FileStream* pLogFileStream);

inline void WriteToLogFile(EA::IO::FileStream* pLogFile, const char* formatString, ...)
{
	if(pLogFile)
	{
		const int kLogBufferSize = 256;
		char8_t buffer[kLogBufferSize];

		va_list arguments;
		va_start(arguments, formatString);

		int numCharsWritten = EA::StdC::Vsnprintf(buffer, kLogBufferSize, formatString, arguments);

		va_end(arguments);

		pLogFile->Write(buffer, numCharsWritten);
	}
}

inline void AddRowToResults(benchmarkenvironment::Results& results, benchmarkenvironment::Sample& sample, eastl::string testName)
{
	results.Begin();
	results.Add(testName.c_str());
	results.Add(sample.GetMean());
	results.Add(sample.GetMin());
	results.Add(sample.GetMax());
	results.Add(sample.GetVariance());
	results.End();
}

#endif
