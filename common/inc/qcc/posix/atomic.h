/**
 * @file
 *
 * Define atomic read-modify-write memory options
 */

/******************************************************************************
 *
 *
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/
#ifndef _POSIX_QCC_ATOMIC_H
#define _POSIX_QCC_ATOMIC_H

#include <qcc/platform.h>

#if defined(QCC_OS_DARWIN)
#include <libkern/OSAtomic.h>
#endif

namespace qcc {

#if defined(QCC_OS_ANDROID)

/**
 * Fetch an int32_t and return its value atomically.
 *
 * @param mem  Pointer to int32_t
 * @return  Value of *mem
 */
inline int32_t AtomicFetch(volatile const int32_t* mem) {
    return __sync_add_and_fetch(mem, 0);
}

/**
 * Set an int32_t atomically.
 *
 * @param mem  Pointer to int32_t
 * @param val  value
 * @return void
 */
inline void AtomicSet(volatile int32_t* mem, volatile int32_t val) {
    __atomic_swap(val, mem);
    return;
}

/**
 * Increment an int32_t and return its new value atomically.
 *
 * @param mem  Pointer to int32_t to be incremented.
 * @return  New value (after increment) of *mem
 */
inline int32_t IncrementAndFetch(volatile int32_t* mem)
{
    /*
     * The __sync_fetch_and_add operation returns the previous value
     * stored in mem. This value has been changed in memory.  Since we expect
     * the value after the increment to be returned we add one to the value
     * returned by __sync_fetch_and_add.
     */
    return __sync_fetch_and_add(mem, 1) + 1;
}

/**
 * Decrement an int32_t and return its new value atomically.
 *
 * @param mem  Pointer to int32_t to be decremented.
 * @return  New value (after decrement) of *mem
 */
inline int32_t DecrementAndFetch(volatile int32_t* mem)
{
    /*
     * The __sync_fetch_and_sub operation returns the previous value
     * stored in mem. This value has been changed in memory.  Since we expect
     * the value after the decrement to be returned we subtract one from the
     * value returned by __sync_fetch_and_sub.
     */
    return __sync_fetch_and_sub(mem, 1) - 1;
}

/**
 * Performs an atomic compare-and-exchange operation on the specified values.
 * It compares two specified 32-bit values and exchanges with another 32-bit
 * value based on the outcome of the comparison.
 *
 * @param mem  Pointer to int32_t to be compared and modified.
 * @param  expectedValue Expected value of *mem.
 * @param  newValue New value of *mem after calling this function, if returning true.
 * @return  true if the initial value of *mem was expectedValue, false otherwise
 */
inline bool CompareAndExchange(volatile int32_t* mem, int32_t expectedValue, int32_t newValue)
{
    /* Use strong memory ordering model */
    return __atomic_compare_exchange_n(mem, &expectedValue, newValue, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/**
 * Performs an atomic compare-and-exchange operation on the specified pointer values.
 * It compares two specified pointer values and exchanges with another pointer
 * value based on the outcome of the comparison.
 *
 * @param mem  Pointer to the pointer value to be compared and modified.
 * @param expectedValue  Expected value of *mem.
 * @param newValue  New value of *mem after calling this function, if returning true.
 * @return  true if the initial value of *mem was expectedValue, false otherwise
 */
inline bool CompareAndExchangePointer(void* volatile* mem, void* expectedValue, void* newValue)
{
    /* Use strong memory ordering model */
    return __atomic_compare_exchange_n(mem, &expectedValue, newValue, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

#elif defined(QCC_OS_LINUX)

/**
 * Fetch an int32_t and return its value atomically.
 *
 * @param mem  Pointer to int32_t
 * @return  Value of *mem
 */
inline int32_t AtomicFetch(volatile const int32_t* mem) {
    return __sync_add_and_fetch(const_cast<volatile int32_t*>(mem), 0);
}

/**
 * Set an int32_t atomically.
 *
 * @param mem  Pointer to int32_t
 * @param val  value
 * @return void
 */
inline void AtomicSet(volatile int32_t* mem, volatile int32_t val) {
    #if 1
    __sync_synchronize();
    *mem = val;
    __sync_synchronize();
    #else
    /** 
    For TSan's race detection to understand atomic loads and stores, it really
    wants to see compiler intrinsics used to handle loads and stores to memory
    locations. There are no direct load instructions in the __sync_synchronize() 
    builtin, so you'd have to do something like __sync_fetch_and_add(x, 0).   **/ 
    int32_t oldval = __sync_fetch_and_add(mem, 0);
    __sync_val_compare_and_swap(mem, oldval, val);
    #endif
}

/**
 * Increment an int32_t and return its new value atomically.
 *
 * @param mem  Pointer to int32_t to be incremented.
 * @return  New value (after increment) of *mem
 */
inline int32_t IncrementAndFetch(volatile int32_t* mem) {
    return __sync_add_and_fetch(mem, 1);
}

/**
 * Decrement an int32_t and return its new value atomically.
 *
 * @param mem  Pointer to int32_t to be decremented.
 * @return  New value (afer decrement) of *mem
 */
inline int32_t DecrementAndFetch(volatile int32_t* mem) {
    return __sync_sub_and_fetch(mem, 1);
}

/**
 * Performs an atomic compare-and-exchange operation on the specified values.
 * It compares two specified 32-bit values and exchanges with another 32-bit
 * value based on the outcome of the comparison.
 *
 * @param mem  Pointer to int32_t to be compared and modified.
 * @param expectedValue  Expected value of *mem.
 * @param newValue  New value of *mem after calling this function, if returning true.
 * @return  true if the initial value of *mem was expectedValue, false otherwise
 */
inline bool CompareAndExchange(volatile int32_t* mem, int32_t expectedValue, int32_t newValue) {
    return __sync_bool_compare_and_swap(mem, expectedValue, newValue);
}

/**
 * Performs an atomic compare-and-exchange operation on the specified pointer values.
 * It compares two specified pointer values and exchanges with another pointer
 * value based on the outcome of the comparison.
 *
 * @param mem  Pointer to the pointer value to be compared and modified.
 * @param expectedValue  Expected value of *mem.
 * @param newValue  New value of *mem after calling this function, if returning true.
 * @return  true if the initial value of *mem was expectedValue, false otherwise
 */
inline bool CompareAndExchangePointer(void* volatile* mem, void* expectedValue, void* newValue) {
    return __sync_bool_compare_and_swap(mem, expectedValue, newValue);
}

#elif defined(QCC_OS_DARWIN)

/**
 * Fetch an int32_t and return its value atomically.
 *
 * @param mem  Pointer to int32_t
 * @return  Value of *mem
 */
inline int32_t AtomicFetch(volatile const int32_t* mem) {
    return OSAtomicAdd32Barrier(0, mem);
}

/**
 * Set an int32_t atomically.
 *
 * @param mem  Pointer to int32_t
 * @param val  value
 * @return void
 */
inline void AtomicSet(volatile int32_t* mem, volatile int32_t val) {
    OSMemoryBarrier();
    *mem = val;
    OSMemoryBarrier();
    return;
}

/**
 * Increment an int32_t and return its new value atomically.
 *
 * @param mem  Pointer to int32_t to be incremented.
 * @return  New value (after increment) of *mem
 */
inline int32_t IncrementAndFetch(volatile int32_t* mem) {
    return OSAtomicIncrement32(mem);
}

/**
 * Decrement an int32_t and return its new value atomically.
 *
 * @param mem  Pointer to int32_t to be decremented.
 * @return  New value (afer decrement) of *mem
 */
inline int32_t DecrementAndFetch(volatile int32_t* mem) {
    return OSAtomicDecrement32(mem);
}

/**
 * Performs an atomic compare-and-exchange operation on the specified values.
 * It compares two specified 32-bit values and exchanges with another 32-bit
 * value based on the outcome of the comparison.
 *
 * @param mem  Pointer to int32_t to be compared and modified.
 * @param expectedValue  Expected value of *mem.
 * @param newValue  New value of *mem after calling this function, if returning true.
 * @return  true if the initial value of *mem was expectedValue, false otherwise
 */
inline bool CompareAndExchange(volatile int32_t* mem, int32_t expectedValue, int32_t newValue) {
    return OSAtomicCompareAndSwapIntBarrier(expectedValue, newValue, mem);
}

/**
 * Performs an atomic compare-and-exchange operation on the specified pointer values.
 * It compares two specified pointer values and exchanges with another pointer
 * value based on the outcome of the comparison.
 *
 * @param mem  Pointer to the pointer value to be compared and modified.
 * @param expectedValue  Expected value of *mem.
 * @param newValue  New value of *mem after calling this function, if returning true.
 * @return  true if the initial value of *mem was expectedValue, false otherwise
 */
inline bool CompareAndExchangePointer(void* volatile* mem, void* expectedValue, void* newValue) {
    return OSAtomicCompareAndSwapPtrBarrier(expectedValue, newValue, mem);
}

#else

/**
 * Fetch an int32_t and return its value atomically.
 *
 * @param mem  Pointer to int32_t
 * @return  Value of *mem
 */
inline int32_t AtomicFetch(volatile const int32_t* mem);

/**
 * Set an int32_t atomically.
 *
 * @param mem  Pointer to int32_t 
 * @param val  value
 * @return void
 */
inline void AtomicSet(volatile int32_t* mem, volatile int32_t val);

/**
 * Increment an int32_t and return its new value atomically.
 *
 * @param mem  Pointer to int32_t to be incremented.
 * @return  New value (afer increment) of *mem
 */
int32_t IncrementAndFetch(volatile int32_t* mem);

/**
 * Decrement an int32_t and return its new value atomically.
 *
 * @param mem  Pointer to int32_t to be decremented.
 * @return  New value (afer decrement) of *mem
 */
int32_t DecrementAndFetch(volatile int32_t* mem);

/**
 * Performs an atomic compare-and-exchange operation on the specified values.
 * It compares two specified 32-bit values and exchanges with another 32-bit
 * value based on the outcome of the comparison.
 *
 * @param mem  Pointer to int32_t to be compared and modified.
 * @param expectedValue  Expected value of *mem.
 * @param newValue  New value of *mem after calling this function, if returning true.
 * @return  true if the initial value of *mem was expectedValue, false otherwise
 */
bool CompareAndExchange(volatile int32_t* mem, int32_t expectedValue, int32_t newValue);

/**
 * Performs an atomic compare-and-exchange operation on the specified pointer values.
 * It compares two specified pointer values and exchanges with another pointer
 * value based on the outcome of the comparison.
 *
 * @param mem  Pointer to the pointer value to be compared and modified.
 * @param expectedValue  Expected value of *mem.
 * @param newValue  New value of *mem after calling this function, if returning true.
 * @return  true if the initial value of *mem was expectedValue, false otherwise
 */
bool CompareAndExchangePointer(void* volatile* mem, void* expectedValue, void* newValue);

#endif

}

#endif
