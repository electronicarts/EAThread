///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif

/////////////////////////////////////////////////////////////////////////////
// Defines functionality for thread-safe primitive operations.
/////////////////////////////////////////////////////////////////////////////


#ifndef EATHREAD_POWERPC_EATHREAD_ATOMIC_POWERPC_H
#define EATHREAD_POWERPC_EATHREAD_ATOMIC_POWERPC_H


#ifndef INCLUDED_eabase_H
	#include <EABase/eabase.h>
#endif
#ifndef EATHREAD_EATHREAD_SYNC_H
	#include <eathread/eathread_sync.h>
#endif
#include <stddef.h>


///////////////////////////////////////////////////////////////////////////////
// EATHREAD_XTL_H_ENABLED
//
// Defined as 0 or 1. Default is 1, for backward compatibility.
// If enabled then xtl.h is #included below, merely for backward compatibility.
//
#ifndef EATHREAD_XTL_H_ENABLED
	#define EATHREAD_XTL_H_ENABLED 1
#endif
///////////////////////////////////////////////////////////////////////////////




#ifdef _MSC_VER
	 #pragma warning(push)
	 #pragma warning(disable: 4146)  // unary minus operator applied to unsigned type, result still unsigned
#endif


#if defined(EA_PROCESSOR_POWERPC)
	#define EA_THREAD_ATOMIC_IMPLEMENTED
	#define EA_THREAD_ATOMIC_LLR_SUPPORTED (0)

	namespace EA
	{
		namespace Thread
		{
			/* To do
			inline int32_t AtomicGetValue(volatile int32_t*)
				{ }
			inline void AtomicSetValue(volatile int32_t*, int32_t value)
				{ }
			inline int32_t AtomicIncrement(volatile int32_t*)
				{ }
			inline int32_t AtomicDecrement(volatile int32_t*)
				{ }
			inline int32_t AtomicAdd(volatile int32_t*, int32_t value)
				{ }
			inline int32_t AtomicOr(volatile int32_t*, int32_t value)
				{ }
			inline int32_t AtomicAnd(volatile int32_t*, int32_t value)
				{ }
			inline int32_t AtomicXor(volatile int32_t*, int32_t value)
				{ }
			inline int32_t AtomicSwap(volatile int32_t*, int32_t value)
				{ }
			inline bool AtomicSetValueConditional(volatile int32_t*, int32_t value, int32_t condition)
				{ }

			inline uint32_t AtomicGetValue(volatile uint32_t*)
				{ }
			inline void AtomicSetValue(volatile uint32_t*, uint32_t value)
				{ }
			inline uint32_t AtomicIncrement(volatile uint32_t*)
				{ }
			inline uint32_t AtomicDecrement(volatile uint32_t*)
				{ }
			inline uint32_t AtomicAdd(volatile uint32_t*, uint32_t value)
				{ }
			inline uint32_t AtomicOr(volatile uint32_t*, uint32_t value)
				{ }
			inline uint32_t AtomicAnd(volatile uint32_t*, uint32_t value)
				{ }
			inline uint32_t AtomicXor(volatile uint32_t*, uint32_t value)
				{ }
			inline uint32_t AtomicSwap(volatile uint32_t*, uint32_t value)
				{ }
			inline bool AtomicSetValueConditional(volatile uint32_t*, uint32_t value, uint32_t condition)
				{ }

			inline int64_t AtomicGetValue(volatile int64_t*)
				{ }
			inline void AtomicSetValue(volatile int64_t*, int64_t value)
				{ }
			inline int64_t AtomicIncrement(volatile int64_t*)
				{ }
			inline int64_t AtomicDecrement(volatile int64_t*)
				{ }
			inline int64_t AtomicAdd(volatile int64_t*, int64_t value)
				{ }
			inline int64_t AtomicOr(volatile int64_t*, int64_t value)
				{ }
			inline int64_t AtomicAnd(volatile int64_t*, int64_t value)
				{ }
			inline int64_t AtomicXor(volatile int64_t*, int64_t value)
				{ }
			inline int64_t AtomicSwap(volatile int64_t*, int64_t value)
				{ }
			inline bool AtomicSetValueConditional(volatile int64_t*, int64_t value, int64_t condition)
				{ }

			inline uint64_t AtomicGetValue(volatile uint64_t*)
				{ }
			inline void AtomicSetValue(volatile uint64_t*, uint64_t value)
				{ }
			inline uint64_t AtomicIncrement(volatile uint64_t*)
				{ }
			inline uint64_t AtomicDecrement(volatile uint64_t*)
				{ }
			inline uint64_t AtomicAdd(volatile uint64_t*, uint64_t value)
				{ }
			inline uint64_t AtomicOr(volatile uint64_t*, uint64_t value)
				{ }
			inline uint64_t AtomicAnd(volatile uint64_t*, uint64_t value)
				{ }
			inline uint64_t AtomicXor(volatile uint64_t*, uint64_t value)
				{ }
			inline uint64_t AtomicSwap(volatile uint64_t*, uint64_t value)
				{ }
			inline bool AtomicSetValueConditional(volatile uint64_t*, uint64_t value, uint64_t condition)
				{ }
			*/


			template <class T>
			class AtomicInt
			{
			public:
				typedef AtomicInt<T> ThisType;
				typedef T            ValueType;

				/// AtomicInt
				/// Empty constructor. Intentionally leaves mValue in an unspecified state.
				/// This is done so that an AtomicInt acts like a standard built-in integer.
				AtomicInt()
					{}

				AtomicInt(ValueType n) 
					{ SetValue(n); }

				AtomicInt(const ThisType& x)
					: mValue(x.GetValue()) {}

				AtomicInt& operator=(const ThisType& x)
					{ mValue = x.GetValue(); return *this; }

				ValueType GetValueRaw() const
					{ return mValue; }

				ValueType GetValue() const;
				ValueType SetValue(ValueType n);
				bool      SetValueConditional(ValueType n, ValueType condition);
				ValueType Increment();
				ValueType Decrement();
				ValueType Add(ValueType n);

			#if EA_THREAD_ATOMIC_LLR_SUPPORTED
				ValueType Reserve(void* spuScratch = NULL);
				bool      StoreConditionalReserved(ValueType n, void* spuScratch = NULL);
			#endif

				// operators
				inline            operator const ValueType() const { return GetValue(); }  // Should this be provided? Is it safe enough? Return value of 'const' attempts to make this safe from misuse.
				inline ValueType  operator =(ValueType n)          {        SetValue(n); return n; }
				inline ValueType  operator+=(ValueType n)          { return Add(n);}
				inline ValueType  operator-=(ValueType n)          { return Add(-n);}
				inline ValueType  operator++()                     { return Increment();}
				inline ValueType  operator++(int)                  { return Increment() - 1;}
				inline ValueType  operator--()                     { return Decrement(); }
				inline ValueType  operator--(int)                  { return Decrement() + 1;}

			protected:
				volatile ValueType mValue;
			};

				// Template specializations for Generic PowerPC: Macintosh OSX, etc.
			#if defined(CS_UNDEFINED_STRING) || defined(EA_COMPILER_GNUC) 

				template <> inline
				AtomicInt<int32_t>::ValueType AtomicInt<int32_t>::GetValue() const
				{
					// The version below uses lwarx directly and not a lwarx/stwcx loop. 
					// You would want the loop if you are on an SMP system and want the 
					// returned value to be reflective of the last store to the address
					// (which would be our store). The downside to the loop is that it 
					// would be slower due to the extra instruction and due to an extra
					// memory synchronization event.
					ValueType nValue;
					#if (EA_MEMORY_BARRIERS_REQUIRED == 0)
						__asm__ __volatile__("lwarx  %0,0,%1"
											: "=&b" (nValue) : "b" (&mValue) : "cc", "memory");
					#else
						__asm__ __volatile__("1: lwarx  %0,0,%1\n\
												 stwcx. %0,0,%1\n\
												  bne 1b"
											   : "=&b" (nValue) : "b" (&mValue) : "cc", "memory");
					#endif
					return nValue;
				}

				template <> inline
				AtomicInt<uint32_t>::ValueType AtomicInt<uint32_t>::GetValue() const
				{
					ValueType nValue;
					#if (EA_MEMORY_BARRIERS_REQUIRED == 0)
						__asm__ __volatile__("lwarx  %0,0,%1"
											: "=&b" (nValue) : "b" (&mValue) : "cc", "memory");
					#else
						__asm__ __volatile__("1: lwarx  %0,0,%1\n\
												 stwcx. %0,0,%1\n\
												 bne 1b"
											   : "=&b" (nValue) : "b" (&mValue) : "cc", "memory");
					#endif
					return nValue;
				}

				template <> inline
				AtomicInt<int32_t>::ValueType AtomicInt<int32_t>::SetValue(ValueType n)
				{
					ValueType nOriginalValue;
					__asm__ __volatile__("1: lwarx  %0,0,%2\n\
											 stwcx. %1,0,%2\n\
											 bne-    1b"
										  : "=&b" (nOriginalValue) : "r" (n), "b" (&mValue) : "cc", "memory");
					return nOriginalValue;
				}

				template <> inline
				AtomicInt<uint32_t>::ValueType AtomicInt<uint32_t>::SetValue(ValueType n)
				{
					ValueType nOriginalValue;
					__asm__ __volatile__("1: lwarx  %0,0,%2\n\
											 stwcx. %1,0,%2\n\
											 bne-    1b" 
										   : "=&b" (nOriginalValue) : "r" (n), "b" (&mValue) : "cc", "memory");
					return nOriginalValue;
				}

				template <> inline
				bool AtomicInt<int32_t>::SetValueConditional(ValueType n, ValueType condition)
				{
					ValueType nOriginalValue;
					__asm__ __volatile__("\n\
										  1: lwarx  %0,0,%1 \n\
											 cmpw    0,%0,%2 \n\
											 bne     2f \n\
											 stwcx. %3,0,%1 \n\
											 bne-    1b\n"
										  "2:"
											: "=&b" (nOriginalValue)
											: "b" (&mValue), "r" (condition), "r" (n)
											: "cc", "memory");
					return (condition == nOriginalValue);
				}

				template <> inline
				bool AtomicInt<uint32_t>::SetValueConditional(ValueType n, ValueType condition)
				{
					ValueType nOriginalValue;
					__asm__ __volatile__("\n\
										 1: lwarx  %0,0,%1 \n\
											cmpw    0,%0,%2 \n\
											bne     2f \n\
											stwcx. %3,0,%1 \n\
											bne-    1b\n"
										"2:"
											: "=&b" (nOriginalValue)
											: "b" (&mValue), "r" (condition), "r" (n)
											: "cc", "memory");
					return (condition == nOriginalValue);
				}

				template <> inline
				AtomicInt<int32_t>::ValueType AtomicInt<int32_t>::Increment()
				{
					ValueType nNewValue;
					__asm__ __volatile__("1: lwarx  %0,0,%1\n\
											 addi    %0,%0,1\n\
											 stwcx. %0,0,%1\n\
											 bne-    1b"
										   : "=&b" (nNewValue) : "b" (&mValue) : "cc", "memory");
					return nNewValue;
				}

				template <> inline
				AtomicInt<uint32_t>::ValueType AtomicInt<uint32_t>::Increment()
				{
					ValueType nNewValue;
					__asm__ __volatile__("1: lwarx  %0,0,%1\n\
											 addi    %0,%0,1\n\
											 stwcx. %0,0,%1\n\
											 bne-    1b"
										   : "=&b" (nNewValue) : "b" (&mValue) : "cc", "memory");
					return nNewValue;
				}

				template <> inline
				AtomicInt<int32_t>::ValueType AtomicInt<int32_t>::Decrement()
				{
					ValueType nNewValue;
					__asm__ __volatile__("1: lwarx  %0,0,%1\n\
											 addi    %0,%0,-1\n\
											 stwcx. %0,0,%1\n\
											 bne-    1b"
										   : "=&b" (nNewValue) : "b" (&mValue) : "cc", "memory");
					return nNewValue;
				}

				template <> inline
				AtomicInt<uint32_t>::ValueType AtomicInt<uint32_t>::Decrement()
				{
					ValueType nNewValue;
					__asm__ __volatile__("1: lwarx  %0,0,%1\n\
											 addi    %0,%0,-1\n\
											 stwcx. %0,0,%1\n\
											 bne-    1b"
										   : "=&b" (nNewValue) : "b" (&mValue) : "cc", "memory");
					return nNewValue;
				}

				template <> inline
				AtomicInt<int32_t>::ValueType AtomicInt<int32_t>::Add(ValueType n)
				{
					ValueType nNewValue;
					__asm__ __volatile__("1: lwarx    %0,0,%2\n\
											 add      %0,%1,%0\n\
											 stwcx.  %0,0,%2\n\
											 bne-     1b"
										   : "=&b" (nNewValue) : "r" (n), "b" (&mValue) : "cc", "memory");
					return nNewValue;
				}

				template <> inline
				AtomicInt<uint32_t>::ValueType AtomicInt<uint32_t>::Add(ValueType n)
				{
					ValueType nNewValue;
					__asm__ __volatile__("1: lwarx    %0,0,%2\n\
											 add      %0,%1,%0\n\
											 stwcx.  %0,0,%2\n\
											 bne-     1b"
										   : "=&b" (nNewValue) : "r" (n), "b" (&mValue) : "cc", "memory");
					return nNewValue;
				}

			#endif // EA_COMPILER_GNUC

			#if (defined(EA_PLATFORM_WORD_SIZE) && (EA_PLATFORM_WORD_SIZE >= 8)) // If we have PowerPC64...

				#if defined(EA_COMPILER_GNUC)

					template <> inline
					AtomicInt<int64_t>::ValueType AtomicInt<int64_t>::GetValue() const
					{
						// The version below uses lwarx directly and not a ldarx/stdcx loop. 
						// You would want the loop if you are on an SMP system and want the 
						// returned value to be reflective of the last store to the address
						// (which would be our store). The downside to the loop is that it 
						// would be slower due to the extra instruction and due to an extra
						// memory synchronization event.
						ValueType nValue;
						#if (EA_MEMORY_BARRIERS_REQUIRED == 0)
							 __asm__ __volatile__("ldarx  %0,0,%1"
												: "=&b" (nValue) : "b" (&mValue) : "cc", "memory");
						#else
							 __asm__ __volatile__("1: ldarx  %0,0,%1\n\
													  stdcx. %0,0,%1\n\
													  bne 1b"
													: "=&b" (nValue) : "b" (&mValue) : "cc", "memory");
						#endif
						return nValue;
					}

					template <> inline
					AtomicInt<uint64_t>::ValueType AtomicInt<uint64_t>::GetValue() const
					{
						ValueType nValue;
						#if (EA_MEMORY_BARRIERS_REQUIRED == 0)
							 __asm__ __volatile__("ldarx  %0,0,%1"
												   : "=&b" (nValue) : "b" (&mValue) : "cc", "memory");
						#else
							 __asm__ __volatile__("1: ldarx  %0,0,%1\n\
													  stdcx. %0,0,%1\n\
													  bne 1b"
													: "=&b" (nValue) : "b" (&mValue) : "cc", "memory");
						#endif
						return nValue;
					}

					template <> inline
					AtomicInt<int64_t>::ValueType AtomicInt<int64_t>::SetValue(ValueType n)
					{
						ValueType nOriginalValue;
						__asm__ __volatile__("1: ldarx  %0,0,%2\n\
												 stdcx. %1,0,%2\n\
												 bne-    1b" 
											   : "=&b" (nOriginalValue) : "r" (n), "b" (&mValue) : "cc", "memory");
						return nOriginalValue;
					}

					template <> inline
					AtomicInt<uint64_t>::ValueType AtomicInt<uint64_t>::SetValue(ValueType n)
					{
						ValueType nOriginalValue;
						__asm__ __volatile__("1: ldarx  %0,0,%2\n\
												 stdcx. %1,0,%2\n\
												 bne-    1b" 
											   : "=&b" (nOriginalValue) : "r" (n), "b" (&mValue) : "cc", "memory");
						return nOriginalValue;
					}

					template <> inline
					bool AtomicInt<int64_t>::SetValueConditional(ValueType n, ValueType condition)
					{
						ValueType nOriginalValue;
						__asm__ __volatile__("\n\
											1: ldarx  %0,0,%1 \n\
												cmpd    0,%0,%2 \n\
												bne     2f \n\
												stdcx. %3,0,%1 \n\
												bne-    1b\n"
											"2:"
												: "=&b" (nOriginalValue)
												: "b" (&mValue), "r" (condition), "r" (n)
												: "cc", "memory");
						return (condition == nOriginalValue);
					}

					template <> inline
					bool AtomicInt<uint64_t>::SetValueConditional(ValueType n, ValueType condition)
					{
						ValueType nOriginalValue;
						__asm__ __volatile__("\n\
											1: ldarx  %0,0,%1 \n\
												cmpd    0,%0,%2 \n\
												bne     2f \n\
												stdcx. %3,0,%1 \n\
												bne-    1b\n"
											"2:"
												: "=&b" (nOriginalValue)
												: "b" (&mValue), "r" (condition), "r" (n)
												: "cc", "memory");
						return (condition == nOriginalValue);
					}

					template <> inline
					AtomicInt<int64_t>::ValueType AtomicInt<int64_t>::Increment()
					{
						ValueType nNewValue;
						__asm__ __volatile__("1: ldarx  %0,0,%1\n\
												 addi    %0,%0,1\n\
												 stdcx. %0,0,%1\n\
												 bne-    1b"
											   : "=&b" (nNewValue) : "b" (&mValue) : "cc", "memory");
						return nNewValue;
					}

					template <> inline
					AtomicInt<uint64_t>::ValueType AtomicInt<uint64_t>::Increment()
					{
						ValueType nNewValue;
						__asm__ __volatile__("1: ldarx  %0,0,%1\n\
												 addi    %0,%0,1\n\
												 stdcx. %0,0,%1\n\
												 bne-    1b"
											   : "=&b" (nNewValue) : "b" (&mValue) : "cc", "memory");
						return nNewValue;
					}

					template <> inline
					AtomicInt<int64_t>::ValueType AtomicInt<int64_t>::Decrement()
					{
						ValueType nNewValue;
						__asm__ __volatile__("1: ldarx  %0,0,%1\n\
												 addi    %0,%0,-1\n\
												 stdcx. %0,0,%1\n\
												 bne-    1b"
											   : "=&b" (nNewValue) : "b" (&mValue) : "cc", "memory");
						return nNewValue;
					}

					template <> inline
					AtomicInt<uint64_t>::ValueType AtomicInt<uint64_t>::Decrement()
					{
						ValueType nNewValue;
						__asm__ __volatile__("1: ldarx  %0,0,%1\n\
												 addi    %0,%0,-1\n\
												 stdcx. %0,0,%1\n\
												 bne-    1b"
											   : "=&b" (nNewValue) : "b" (&mValue) : "cc", "memory");
						return nNewValue;
					}

					template <> inline
					AtomicInt<int64_t>::ValueType AtomicInt<int64_t>::Add(ValueType n)
					{
						ValueType nNewValue;
						__asm__ __volatile__("1: ldarx    %0,0,%2\n\
												 add      %0,%1,%0\n\
												 stdcx.  %0,0,%2\n\
												 bne-     1b"
											   : "=&b" (nNewValue) : "r" (n), "b" (&mValue) : "cc", "memory");
						return nNewValue;
					}

					template <> inline
					AtomicInt<uint64_t>::ValueType AtomicInt<uint64_t>::Add(ValueType n)
					{
						ValueType nNewValue;
						__asm__ __volatile__("1: ldarx    %0,0,%2\n\
												 add      %0,%1,%0\n\
												 stdcx.  %0,0,%2\n\
												 bne-     1b"
											   : "=&b" (nNewValue) : "r" (n), "b" (&mValue) : "cc", "memory");
						return nNewValue;
					}

				#endif

			#endif

		} // namespace Thread

	} // namespace EA


#endif // EA_PROCESSOR_XXXX


#ifdef _MSC_VER
	 #pragma warning(pop)
#endif


#endif // EATHREAD_POWERPC_EATHREAD_ATOMIC_POWERPC_H








