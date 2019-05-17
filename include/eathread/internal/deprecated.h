///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#ifndef EATHREAD_INTERNAL_DEPRECATED_H
#define EATHREAD_INTERNAL_DEPRECATED_H

#include <EABase/eabase.h>

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif

////////////////////////////////////////////////////////////////////////////////
// This header provides facilities for nudging users off of deprecated code.
// 
// The goal is to provide a gradual migration where users become aware of the
// accumulated technical debt considerably before they are required to address
// the problem. To this end, once a feature has been deprecated, we may escalate
// from warnings, to assertions, to build warnings before actual removal.
//
// EATHREAD_REMOVE_DEPRECATED_API				can be defined in client code to force build time errors
// EATHREAD_DEPRECATED_MEMBER_WARN_ON_USE		generate runtime warnings on write to deprecated members
// EATHREAD_DEPRECATED_MEMBER_ASSERT_ON_USE		generate runtime assertions on write to deprecated members
// EATHREAD_DEPRECATED_MEMBER_WARN_ON_BUILD		generate runtime assertions and build warnings on access
//
// TODO: consider migrating these facilities to a shared location once they're stable (EAStdC)

///////////////////////////////////////////////////////////////////////////////
// EATHREAD_REMOVE_DEPRECATED_API
//
// Defining this macro in client code will remove any deprecated API from the
// EAThread public headers. This can be useful to temporarily define locally
// in dependent modules to find and eliminate any contained code that depends
// on any deprecated EAThread features.
//
// Another approach is to enable broader deprecate culling by defining
// EA_REMOVE_DEPRECATED_API within the build for the module you wish to
// eliminate deprecated code. This should remove deprecates for all libraries
// that support it.
//
// Note: Deprecated API culling macros should not be defined globally for a
// build. Doing so will flag all use of deprecated API across all modules
// in a game which is typically more noise than desired and makes a piecewise
// approach more difficult. Instead, define the flags only when building the
// module where you wish to eliminate use of deprecated code.
#if defined(EA_REMOVE_DEPRECATED_API)
	// respect the master control if it has been provided
	#define EATHREAD_REMOVE_DEPRECATED_API
#endif

////////////////////////////////////////////////////////////////////////////////
// EATHREAD_DEPRECATED_MEMBER_WARN_ON_USE
//
// Simplifies the process of disabling public members of EAThread classes when
// building with deprecated code removed. This macro renames the member variable
// when deprecate culling is enabled to avoid changing the size of the structure
// which can cause binary incompatibility issues.
#if defined(EATHREAD_REMOVE_DEPRECATED_API)
	// rename deprecated members to trigger a build error
	#define EATHREAD_DEPRECATED_MEMBER_WARN_ON_USE(Type, Name) EA_DEPRECATED Type EA_PREPROCESSOR_JOIN2(name, _deprecated)
#else
	// member enabled, but use runtime deprecation warnings only
	#define EATHREAD_DEPRECATED_MEMBER_WARN_ON_USE(Type, Name) EA::Thread::DeprecatedMemberWarn<Type> Name
#endif

////////////////////////////////////////////////////////////////////////////////
// EATHREAD_DEPRECATED_MEMBER_ASSERT_ON_USE
//
// This is similar to recently deprecated member except that it will generate
// an assertion failure on assignment.
#if defined(EATHREAD_REMOVE_DEPRECATED_API)
	// rename deprecated members to trigger a build error
	#define EATHREAD_DEPRECATED_MEMBER_ASSERT_ON_USE(Type, Name) EA_DEPRECATED Type EA_PREPROCESSOR_JOIN2(name, _deprecated)
#else
	// member enabled, but use runtime assertions only
	#define EATHREAD_DEPRECATED_MEMBER_ASSERT_ON_USE(Type, Name) EA::Thread::DeprecatedMemberError<Type> Name
#endif

////////////////////////////////////////////////////////////////////////////////
// EATHREAD_DEPRECATED_MEMBER_WARN_ON_BUILD
//
// This is similar to deprecated member except that it additionally
// add deprecation markup which will trigger warnings during the build. Note,
// this will often get converted into a build error with warnings as error
// enabled. For this reason, consider using the other macros first.
#if defined(EATHREAD_REMOVE_DEPRECATED_API)
	// rename deprecated members to trigger a build error
	#define EATHREAD_DEPRECATED_MEMBER_WARN_ON_BUILD(Type, Name) EATHREAD_DEPRECATED_MEMBER_ASSERT_ON_USE(Type, Member)
#else
	// member enabled, assert on set but also use build-time deprecation warnings
	#define EATHREAD_DEPRECATED_MEMBER_WARN_ON_BUILD(Type, Name) EA_DEPRECATED EATHREAD_DEPRECATED_MEMBER_ASSERT_ON_USE(Type, Name)
#endif

namespace EA {
namespace Thread {

// Issues the given warning if the flag is unset (and sets it), otherwise does nothing.  This can be use to limit the
// amount of message spam coming from a specific usage.
EATHREADLIB_API void WarnOnce(bool* pHasTriggered, const char* message);
EATHREADLIB_API void ErrorOnce(bool* pHasTriggered, const char* message);

// This template allows the creation of classes that implicitly convert to the wrapped type but will warn on assignment.
// This is useful in removing public member variables from our public API.  The goal here is to provide a softer nudge
// than a build error.  Deprecation markup on the member is a similar approach but will often trigger a build failure
// as warnings as errors is commonly enabled.
// TODO: does not work for types that support dereferencing
// TODO: also missing other operator forwarding
template <typename T>
class DeprecatedMemberWarn
{
public:
#ifdef EA_COMPILER_NO_DEFAULTED_FUNCTIONS
	DeprecatedMemberWarn(){}
#else
	DeprecatedMemberWarn() = default;
#endif
	DeprecatedMemberWarn(T rhs): mValue(rhs) {}

	//DeprecatedMemberWarn& operator=(DeprecatedMemberWarn&&) = default; // TODO: Why doesn't this work
#ifdef EA_COMPILER_NO_DEFAULTED_FUNCTIONS
	DeprecatedMemberWarn& operator=(const DeprecatedMemberWarn& rhs)
	{
		mValue = rhs.mValue;
		return *this;
	}
#else
	DeprecatedMemberWarn& operator=(const DeprecatedMemberWarn& rhs) = default;
#endif

	DeprecatedMemberWarn& operator=(const T& rhs)
	{
#if EAT_ASSERT_ENABLED
		static bool hasTriggered = false;
		WarnOnce(&hasTriggered, "Client code is accessing a deprecated structure member.");
#endif
		this->mValue = rhs;
		return *this;
	}

	DeprecatedMemberWarn& operator=(T&& rhs)
	{
#if EAT_ASSERT_ENABLED
		static bool hasTriggered = false;
		WarnOnce(&hasTriggered, "Client code is accessing a deprecated structure member.");
#endif
		this->mValue = rhs;
		return *this;
	}

	operator T() const
	{
#if EAT_ASSERT_ENABLED
		static bool hasTriggered = false;
		WarnOnce(&hasTriggered, "Client code is accessing a deprecated structure member.");
#endif
		return mValue;
	}

	// accessor for fetching the value without tripping the error
	const T& GetValue() const { return mValue; }

	// TODO: use sfinae to enable/disable when the wrapped type supports dereferencing
	//auto operator->() const
	//{
		//return T::operator->(mValue);
	//}


private:
	T mValue;
	int foo;
};

// This template allows the creation of classes that implicitly convert to the wrapped type but will assert on assignment.
// This is useful in removing public member variables from our public API.  The goal here is to provide a softer nudge
// than a build error.  Deprecation markup on the member is a similar approach but will often trigger a build failure
// as warnings as errors is commonly enabled.
// TODO: does not work for types that support dereferencing
template <typename T>
class DeprecatedMemberError
{
public:
#ifdef EA_COMPILER_NO_DEFAULTED_FUNCTIONS
	DeprecatedMemberError(){};
#else
	DeprecatedMemberError() = default;
#endif
	DeprecatedMemberError(T rhs): mValue(rhs) {}

	//DeprecatedMemberError& operator=(DeprecatedMemberError&&) = default; // TODO: Why doesn't this work
#ifdef EA_COMPILER_NO_DEFAULTED_FUNCTIONS
	DeprecatedMemberError& operator=(const DeprecatedMemberError& rhs)
	{
		mValue = rhs.mValue;
		return *this;
	};
#else
	DeprecatedMemberError& operator=(const DeprecatedMemberError& rhs) = default;
#endif

	DeprecatedMemberError& operator=(const T& rhs)
	{
#if EAT_ASSERT_ENABLED
		static bool hasTriggered = false;
		ErrorOnce(&hasTriggered, "Client code is accessing a deprecated structure member.");
#endif
		this->mValue = rhs;
		return *this;
	}

	DeprecatedMemberError& operator=(T&& rhs)
	{
#if EAT_ASSERT_ENABLED
		static bool hasTriggered = false;
		ErrorOnce(&hasTriggered, "Client code is accessing a deprecated structure member.");
#endif
		this->mValue = rhs;
		return *this;
	}

	operator T() const
	{
#if EAT_ASSERT_ENABLED
		static bool hasTriggered = false;
		ErrorOnce(&hasTriggered, "Client code is accessing a deprecated structure member.");
#endif
		return mValue;
	}

	// accessor for fetching the value without tripping the error
	const T& GetValue() const { return mValue; }

private:
	T mValue;
};

}} // end namespace EA::Thread

#endif

