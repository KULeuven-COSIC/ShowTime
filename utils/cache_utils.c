/*
  These cache utils have been stitched together over time, including modifications.
  We try to attribute them to original sources where we can.
*/

#include <stdint.h>
#include "cache_utils.h"

////////////////////////////////////////////////////////////////////////////////

inline
void
// Attribution: https://github.com/IAIK/flush_flush/blob/master/sc/cacheutils.h
clflush(void *p)
{
	__asm__ volatile ("clflush 0(%0)" : : "c" (p) : "rax");
}

inline
void
// Attribution: https://github.com/IAIK/flush_flush/blob/master/sc/cacheutils.h
clflushopt(void *p)
{
	__asm__ volatile ("clflushopt 0(%0)" : : "c" (p) : "rax");
}

inline
void 
// Attribution: https://github.com/jovanbulck/sgx-tutorial-space18/blob/master/common/cacheutils.h
clflush_f(void *p)
{
  asm volatile (
    "mfence\n"
    "clflush 0(%0)\n"
    "mfence\n"
    :
    : "D" (p)
    : "rax");
}

////////////////////////////////////////////////////////////////////////////////

inline
uint64_t
// https://github.com/cgvwzq/evsets/blob/master/micro.h
rdtsc()
{
	unsigned a, d;
	__asm__ volatile ("lfence\n"
	"rdtscp\n"
	"mov %%edx, %0\n"
	"mov %%eax, %1\n"
	: "=r" (a), "=r" (d)
	:: "%rax", "%rbx", "%rcx", "%rdx");
	return ((uint64_t)a << 32) | d;
}

inline
uint64_t
// modified from https://github.com/cgvwzq/evsets/blob/master/micro.h 
rdtsc_cripple(uint64_t cycle_granularity)
{
	unsigned a, d;
	__asm__ volatile ("lfence\n"
	"rdtscp\n"
	"mov %%edx, %0\n"
	"mov %%eax, %1\n"
	: "=r" (a), "=r" (d)
	:: "%rax", "%rbx", "%rcx", "%rdx");
	return (uint64_t) ((uint64_t) ((uint64_t) ((uint64_t)a << 32)) | d) / cycle_granularity;
}

// Attribution: https://cs.adelaide.edu.au/~yval/Mastik/
uint64_t rdtscp64() {
  uint32_t low, high;
  asm volatile ("rdtscp": "=a" (low), "=d" (high) :: "ecx");
  return (((uint64_t)high) << 32) | low;
}

uint32_t rdtscp32() {
  uint32_t low, high;
  asm volatile ("rdtscp": "=a" (low), "=d" (high) :: "ecx");
  return low;
}

////////////////////////////////////////////////////////////////////////////////

inline
void
// Attribution: https://github.com/IAIK/flush_flush/blob/master/sc/cacheutils.h
maccess(void* p)
{
	__asm__ volatile ("movq (%0), %%rax\n" : : "c" (p) : "rax");
}

inline 
void 
mwrite(void *v)
{
  asm volatile (
    "mfence\n\t"
    "lfence\n\t"
    "movl $0, (%0)\n\t"
    "mfence\n\t"
    : 
    : "D" (v)
    : );
}

inline 
void 
mwrite_v(void *v, int value)
{
  asm volatile (
    "mfence\n\t"
    "lfence\n\t"
    "movl %1, (%0)\n\t"
    "mfence\n\t"
    : 
    : "D" (v), "r" (value)
    : );
}

inline 
int 
// Attribution: https://cs.adelaide.edu.au/~yval/Mastik/
mread(void *v) 
{
  int rv = 0;
  asm volatile("mov (%1), %0": "+r" (rv): "r" (v):);
  return rv;
}

inline
int 
// Attribution: https://cs.adelaide.edu.au/~yval/Mastik/
time_mread(void *adrs) 
{
  volatile unsigned long time;

  asm volatile (
    // "lfence\n"
    "mfence\n"
    "rdtscp\n"
    "lfence\n"
    "mov %%eax, %%esi\n"
    "mov (%1), %%eax\n"
    "rdtscp\n"
    "sub %%esi, %%eax\n"
    : "=&a" (time)          // output
    : "r" (adrs)            // input
    : "ecx", "edx", "esi"); // clobber registers

  return (int) time;
}

inline
int 
// Attribution: https://cs.adelaide.edu.au/~yval/Mastik/ (modified)
time_mread_nofence(void *adrs) 
{
  volatile unsigned long time;

  asm volatile (
    "rdtscp\n"
    "movl %%eax, %%esi\n"
    "movl (%1), %%eax\n"
    "rdtscp\n"
    "sub %%esi, %%eax\n"
    : "=&a" (time)          // output
    : "r" (adrs)            // input
    : "ecx", "edx", "esi"); // clobber registers

  return (int) time;
}

inline
int 
time_prefetch(void *adrs) 
{
  volatile unsigned long time;

  asm volatile (
    // "lfence\n"
    "mfence\n"
    "rdtscp\n"
    "lfence\n"
    "mov %%eax, %%esi\n"
    "prefetchnta (%0)\n"
    "rdtscp\n"
    "sub %%esi, %%eax\n"
    : "=&a" (time)          // output
    : "r" (adrs)            // input
    : "ecx", "edx", "esi"); // clobber registers

  return (int) time;
}

void prefetchNTANoFence(void* p)
{
  asm volatile ("prefetchnta (%0)\n" : : "r" (p));
}

void prefetchT2(void* p)
{
  asm volatile ("prefetcht2 (%0)\n" : : "r" (p));
}

void prefetchNTA(void* p)
{
  asm volatile ("lfence\nmfence\nprefetchnta (%0)\n" : : "r" (p));
}

void prefetchNTADouble(void* p, void* p2)
{
  asm volatile ("lfence\nmfence\nprefetchnta (%0)\nlfence\nmfence\nprefetchnta (%1)\n" : : "r" (p), "r" (p2));
}

void prefetchRUN(void* p, void* p2)
{

asm volatile (
  ".rept 1000\n"
    "mfence\n"
    "prefetchnta (%0)\n"
    "mfence\n"
    "prefetchnta (%1)\n"
  ".endr\n"
  : : "r" (p), "r" (p2));

}
