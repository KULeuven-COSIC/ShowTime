#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>
#include <assert.h>

#include "settings.h"                 // tunable knobs for this experiment
#include "../utils/configuration.h"
#include "../utils/cache_utils.h"
#include "../utils/memory_utils.h"
#include "../utils/misc_utils.h"

// Evset functions
#include "../evsets/list/list_traverse.h"
#include "../evsets/list/list_utils.h"
#include "../evsets/ps_evset.h"

#define ASSERT(x) assert(x != -1)
#define FLUSH(x)  ({ flush((void*)x); })

/* 
  Given two addresses a and b, and a restricted timer
    determine whether they are LLC-congruent
  This function uses the prefetchNTA amplifier.
*/
int determine_congruence(uint64_t* a, uint64_t* b, uint64_t* evict_pool){

  uint64_t GRANULARITY_CYCLES = (uint64_t)BASE_FREQ_MHZ*(uint64_t)GRANULARITY_USEC;
  #define LMFENCE asm volatile ("lfence\nmfence\n");
  uint64_t start, end, kit, lit, xit; int early_abort_flag = 0;

  for (lit=0; lit<CONGRUENCE_CONFIDENCE; lit++){

    if (early_abort_flag) {break;}

    uint64_t traversals = 0;

    #if GRANULARITY_USEC == 1000000 // use UNIX Epoch 
      LMFENCE; start = time(NULL); LMFENCE; 
      while(time(NULL) == start){
    #else // use artificially crippled timer
      start = rdtsc_cripple(GRANULARITY_CYCLES); LMFENCE;
      while(rdtsc_cripple(GRANULARITY_CYCLES) == start){
    #endif
      #if EVICTION_LOOP == 1
        LMFENCE; FLUSH(a); FLUSH(b); LMFENCE;
        for (xit = 0; xit < 2048; xit++){ maccess((void*)evict_pool[xit]); }
        LMFENCE;
        for (kit = 0; kit < 128; kit++){
          traversals++;
          prefetchRUN((void*) a, (void*) b);
        }
      #else
          traversals++;
          prefetchRUN((void*) a, (void*) b);
      #endif

      #if EARLY_ABORT
        if (traversals >= ITERATION_THRESHOLD){ early_abort_flag = 1; break; }
      #endif

    }

    #if PRINT_TRAVERSE_COUNTERS == 1
      printf("%ld - %ld\n", lit, traversals);
    #endif

    if (traversals >= ITERATION_THRESHOLD) { return 0; };

  }

  return 1; // Addresses deemed LLC-congruent
  ////////////////////////////////////////

}

////////////////////////////////////////////////////////////////////////////////

void attacker() {

  // mmap some memory regions
  uint64_t *shared_mem, *evict_mem, *evict_mem_preev, *evict_mem_evict;
  ASSERT(mem_map_private(&shared_mem, SHARED_MEM_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem_preev, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem_evict, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));


  //////////////////////////////////////////////////////////////////////////////
  // Pick a random target_addr from shared_mem
  // Cache Access Thresholds
  // Construct eviction sets to act as ground truth
  #include "preparation_evset.h"
  //////////////////////////////////////////////////////////////////////////////

  uint64_t GRANULARITY_CYCLES = (uint64_t)BASE_FREQ_MHZ*(uint64_t)GRANULARITY_USEC;
  int i, j, it, lit, yit, kit, xit;       // Loop variables
  uint64_t start, end; int nb_elems; 
  uint64_t evsetLOWRES[EV_LLC];

  // Ensure all pages are mapped
  ps_evset_premap(evict_mem_evict); ps_evset_premap(evict_mem_preev);

  printf("\n\tGranularity [ms]          %.4f ms", (double) ((double)GRANULARITY_USEC / 1000));
  printf("\n\tGranularity [cycles]  %10ld cycles\n", (uint64_t) GRANULARITY_CYCLES);
  printf("\tIteration threshold   %10d \n", ITERATION_THRESHOLD);
  printf("\tCongruence confidence %10d in a row\n\n", CONGRUENCE_CONFIDENCE);

  #if GRANULARITY_USEC < 100000 && EARLY_ABORT == 1
    printf("Relatively fine-grained timer, consider disabling EARLY_ABORT for better results\n");
  #elif GRANULARITY_USEC >= 100000 && EARLY_ABORT == 0
    printf("Relatively coarse-grained timer, consider enabling EARLY_ABORT for better results\n");
  #endif
  #if GRANULARITY_USEC < 100000 && EVICTION_LOOP == 1
    printf("Relatively fine-grained timer, consider disabling EVICTION_LOOP for better results\n");
  #elif GRANULARITY_USEC >= 100000 && EVICTION_LOOP == 0
    printf("Relatively coarse-grained timer, consider enabling EVICTION_LOOP for better results\n");
  #endif

  ///////////////////////////////////////////////////////////////////////
  // Wall-clock time to perform end-to-end eviction set construction
  ///////////////////////////////////////////////////////////////////////
  struct timespec tstart={0,0}, tend={0,0}; double timespan;
  clock_gettime(CLOCK_MONOTONIC, &tstart);

  //////////////
  // Guess pool
  //////////////
  #define START_POOL_SIZE MAX_POOL_SIZE_SMALL
  uint64_t guess;
  uint64_t guess_pool[START_POOL_SIZE];
  for (i=0; i<START_POOL_SIZE; i++)  {
    guess_pool[i] = ((uint64_t)evict_mem_preev + ((uint64_t)target_addr & (SMALLPAGE_PERIOD-1)) + i*SMALLPAGE_PERIOD);
  }

  //////////////////////////////
  // Eviction pool if necessary
  //////////////////////////////
  uint64_t evict_pool[MAX_POOL_SIZE_SMALL];
  #if EVICTION_LOOP == 1
  for (i=0; i<MAX_POOL_SIZE_SMALL; i++)  {
    evict_pool[i] = ((uint64_t)evict_mem_evict + ((uint64_t)target_addr & (SMALLPAGE_PERIOD-1)) + i*SMALLPAGE_PERIOD);
  }
  #endif

  //////////////////////////
  // EV Construction itself
  //////////////////////////
  nb_elems = 0; 
  uint64_t pool_size = START_POOL_SIZE;
  int counter=0;
  while (nb_elems < LLC_WAYS){

    // Get guess, increment counter
    guess = guess_pool[counter];

    // Demonstrate congruence -> add to eviction set
    if (determine_congruence((uint64_t*) guess, (uint64_t*) target_addr, evict_pool)){
      evsetLOWRES[nb_elems++] = guess;

      #if STATUS_UPDATE == 1
        printf("Found %2d - %5d - 0x%lx\n", nb_elems, counter, guess);
      #endif

    }

    counter = (counter + 1) % pool_size;
    if (counter == 0) {printf("At end of list\n"); return;}
  }

  ///////////////
  // Ending time
  ///////////////
  clock_gettime(CLOCK_MONOTONIC, &tend);
  printf("Eviction set constructed in %.0f ms\n", time_diff_ms(tstart, tend));

  /////////////////////////////
  // Evaluate quality of evset
  /////////////////////////////
  uint64_t correct = 0; int nb_congruent_correct = 0;

  // Congruent lines should be congruent
  for(kit=0; kit<LLC_WAYS; kit++){
    correct = 0;
    for(lit=0; lit<VERIFY_ITERATIONS; lit++){
      correct += ground_truth((uint64_t*) evsetLOWRES[kit], evsetLLC, LLC_WAYS, thrDET);
    }
    if (correct > 0.5*VERIFY_ITERATIONS){nb_congruent_correct++;}
    #if SILENT_MODE == 0
    printf("Correct %d - %d\n", kit, correct);
    #endif
  }

  printf("Evset correctness: %2d/%2d\n", nb_congruent_correct, LLC_WAYS);

  // Sanity check: non-congruent lines should be non-congruent
  correct = 0;
  for(lit=0; lit<VERIFY_ITERATIONS; lit++){
    correct += ground_truth((uint64_t*) (target_addr + 170), evsetLLC, LLC_WAYS, thrDET);
  }
  if (correct > 1){printf("Error: it said a non-congruent line was congruent");}

  ////////////////
  // Collect data 
  ////////////////
  FILE *fp; fp = fopen ("./log/results.log","a");
  fprintf(fp, "%d %d %ld %.0f\n", LLC_WAYS, nb_congruent_correct, (uint64_t) GRANULARITY_USEC, time_diff_ms(tstart, tend)); // total ways,correct elems,granularity in usec, time in ms

  // ummap some memory regions
  ASSERT(munmap(shared_mem, SHARED_MEM_SIZE));
  ASSERT(munmap(evict_mem,  EVICT_LLC_SIZE));
  ASSERT(munmap(evict_mem_preev,  EVICT_LLC_SIZE));
  ASSERT(munmap(evict_mem_evict,  EVICT_LLC_SIZE));

}