#pragma once

void     clflush            (void *p);
void     clflushopt         (void *p);
void     clflush_f          (void *p);

uint64_t rdtsc              (void);
uint64_t rdtsc_cripple      (uint64_t cycle_granularity);
uint64_t rdtscp64           (void);
uint32_t rdtscp32           (void);

void     maccess            (void *p);
void     mwrite             (void *v);
void     mwrite_v           (void *v, int val);
int      mread              (void *v);
int      time_mread         (void *adrs);
int      time_prefetch      (void *adrs);
int      time_mread_nofence (void *adrs);

void prefetchNTA(void* p);
void prefetchNTANoFence(void* p);
void prefetchT2(void* p);
void prefetchNTADouble(void* p, void* p2);
void prefetchRUN(void* p, void* p2);

#define  flush(x)            clflush_f(x)
#define  flush_nofence(x)    clflush(x)
#define  memwrite(x)         mwrite(x)
#define  memread(x)          mread(x)
