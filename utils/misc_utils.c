#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "cache_utils.h"
#include "misc_utils.h"

// Pin thread to specific core
void set_core(int core, char *print_info) {

  // Define your cpu_set bit mask
  cpu_set_t my_set;

  // Initialize it all to 0, i.e. no CPUs selected
  CPU_ZERO(&my_set);

  // Set the bit that represents core
  CPU_SET(core, &my_set);

  // Set affinity of tihs process to the defined mask
  sched_setaffinity(0, sizeof(cpu_set_t), &my_set);

  // Print this thread's CPU
  printf("Core %2d for %s\n", sched_getcpu(), print_info);
}



// Measure time elapsed for experiment (not used for cache timing measurements)
double time_diff_ms(struct timespec begin, struct timespec end)
{
	double timespan;
	if ((end.tv_nsec-begin.tv_nsec)<0) {
		timespan  = (end.tv_sec -begin.tv_sec  - 1				   )*1.0e3 ;
		timespan += (end.tv_nsec-begin.tv_nsec + 1000000000UL)*1.0e-6;
	} else {
		timespan  = (end.tv_sec -begin.tv_sec                )*1.0e3 ;
		timespan += (end.tv_nsec-begin.tv_nsec               )*1.0e-6;
	}
	return timespan;
}

int comp(const void * a, const void * b) {
  return ( *(uint64_t*)a - *(uint64_t*)b );
}

int comp_double(const void * a, const void * b) {
  if (*(double*)a > *(double*)b)
    return 1;
  else if (*(double*)a < *(double*)b)
    return -1;
  else
    return 0;  
}

inline int median(int *array, int len) {
  qsort(array, len, sizeof(int), comp);
  return array[len/2];
}

uint64_t get_new_random_index(uint64_t* rands, uint64_t MAX_RAND, int nb_so_far){
  // Avoiding duplicates
  
  uint8_t goodRand = 0; int j; uint64_t r;

  while (!goodRand){
    goodRand = 1;
    r = rand() % MAX_RAND;
    for (j=0; j<nb_so_far; j++){
      if (rands[j] == r){ goodRand = 0; break; }
    }
  }

  rands[nb_so_far] = r;

  return r;

}

void construct_L1_eviction_set(uint64_t* evset, uint64_t* page, uint64_t target, uint8_t SIZE, uint64_t STRIDE, uint64_t MAX_RAND){

  uint64_t i, r;
  uint64_t rands[SIZE];
  
  for (i=0; i<SIZE; i++)  {

    r = get_new_random_index(rands, MAX_RAND, i);

    evset[i] = ((uint64_t)page + (target & (STRIDE-1)) + r*STRIDE);
  }

}

void configure_thresholds(uint64_t target_addr, int* thrL1, int* thrRAM, int* thrDET) {

  #define THRESHOLD_TEST_COUNT 1000

  int timing[10][THRESHOLD_TEST_COUNT];
  int access_time;

  for (int t=0; t<THRESHOLD_TEST_COUNT; t++) {
    flush((void*) target_addr);
    timing[0][t] = time_mread((void*) target_addr); // time0: DRAM
    timing[1][t] = time_mread((void*) target_addr); // time1: L1/L2
  }
  qsort(timing[0], THRESHOLD_TEST_COUNT, sizeof(int), comp);
  qsort(timing[1], THRESHOLD_TEST_COUNT, sizeof(int), comp);
  *thrRAM = timing[0][(int)0.50*THRESHOLD_TEST_COUNT];
  *thrL1  = timing[1][(int)0.10*THRESHOLD_TEST_COUNT];
  *thrDET = (2*(*thrRAM) + (*thrL1))/3;
}


int ground_truth(uint64_t* a, uint64_t* evset, uint8_t LLC_ASSOCIATIVITY, uint64_t thrDET){

  #define NB_TESTS_GROUND_TRUTH 10

  #define READ_ACCESS(x)  ({ maccess((void*) (x)); })
  #define TIME_READ_ACCESS(x)  ({ time_mread((void*) (x)); })
  #define MFENCE asm volatile ("mfence\n");
  uint64_t kit, lit, xit;

  int counter = 0;

  for (lit = 0; lit < NB_TESTS_GROUND_TRUTH; lit++){

    prefetchNTA(a); MFENCE;
    for (xit=0; xit < 5; xit++){
      for (kit = 0; kit < LLC_ASSOCIATIVITY; kit++){
        READ_ACCESS(evset[kit]); MFENCE;
      }
    }
    if (TIME_READ_ACCESS(a) > thrDET){ counter++; }

  }

  return (counter > 0.7* (NB_TESTS_GROUND_TRUTH));

}

