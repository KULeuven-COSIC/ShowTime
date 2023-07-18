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
#define ASSERT(x) assert(x != -1)

// Consider this file only if the target machine has inclusive caches 
// according to configuration.h
#include "../utils/configuration.h"
#include "settings.h"
#ifdef LLC_INCLUSIVE 

#include "../utils/cache_utils.h"
#include "../utils/memory_utils.h"
#include "../utils/misc_utils.h"

// Evset functions
#include "../evsets/list/list_utils.h"
#include "../evsets/ps_evset.h"



////////////////////////////////////////////////////////////////////////////////
// Memory Allocations
extern volatile uint64_t *shared_mem;
extern volatile uint64_t *synchronization;
extern volatile uint64_t *synchronization_params;

////////////////////////////////////////////////////////////////////////////////
// Function declarations
void humanize();
void attacker_helper();

////////////////////////////////////////////////////////////////////////////////

extern uint64_t *evict_mem;
uint64_t *evict_mem_LLC;

void attacker() {

  // mmap some memory regions
  ASSERT(mem_map_private(&evict_mem_LLC, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));

   // spin up helper thread to simulate cross-core access by a victim
  if (fork() == 0) {
    set_core(HELPER_CORE, "Attacker Helper");
    attacker_helper();
    return;
  }

  humanize();

  // ummap some memory regions
  ASSERT(munmap(evict_mem_LLC,  EVICT_LLC_SIZE));

  // Shut Down
  *synchronization = -1; sleep(1);
}

////////////////////////////////////////////////////////////////////////////////

void humanize() {

  int i, j, it, lit, yit, kit, xit;       // Loop variables

  //////////////////////////////////////////////////////////////////////////////
  // Include the function macros
  #include "macros.h"

  /*
    Make abstraction of the preparation stages
      - pick a random target_address from shared_mem
      - set cache access thresholds
      - construct eviction sets: evsetLLC and evsetLLC2
  */
  #include "preparation_humanize.h"
  //////////////////////////////////////////////////////////////////////////////

  *(uint64_t*)target_addr = 0;
  uint8_t ground_truths[ITERATIONS];
  #define P_A target_addr
  #define P_B evsetLLC[7]
  
  ////////////////////////////////////////
  // Calibration: 3H-3M-1H-1M
  for(lit=0; lit<3; lit++)         { ground_truths[lit] = 1; }
  for(lit=3; lit<6; lit++)         { ground_truths[lit] = 0; }
  for(lit=6; lit<7; lit++)         { ground_truths[lit] = 1; }
  for(lit=7; lit<8; lit++)         { ground_truths[lit] = 0; }
  // For real: random
  for(lit=8; lit<ITERATIONS; lit++){ ground_truths[lit] = rand() & 0x1;}
  ////////////////////////////////////////

  printf("\n\n\n\n\n\n");
  printf("   SHOWTIME\n     DEMO\n"); 
  getchar();  LMFENCE;

  for(lit=0; lit<ITERATIONS; lit++){

    ////////////////////////////////////////
    // Interact with human
    LMFENCE; getchar();  
    if (lit == 0){
      LMFENCE; printf("   CALIBRATE\n\n");  getchar();  LMFENCE;
      printf("\n\n");
    }
    if (lit == 8){
      LMFENCE;
      printf("   LET'S GO\n\n");
      getchar();  LMFENCE;
      printf("\n\n");
    }

    // Warm up again after interacting with slow, slow human
    for (kit = 0; kit < 10000; kit++){ PRIME_EVSETLLC(); }

    // Make every line in the LLC set get the insertion age
    FLUSH_EVSETLLC(); FLUSH(P_A);  LMFENCE;
    PRIME_EVSETLLC2(); 

    /*
      Toss a coin:
        - to your witcher
        - to determine whether the line of interest P_A is cached in L1 or not at all
    */
    if (ground_truths[lit]){ LMFENCE; READ(P_A); LMFENCE; }

    ////////////////////////////////////////
    printf("     Show\n");  

    // Run the amplifier
    for (kit=0; kit<(AMP_TRAVERSE/1000); kit++){
      prefetchRUN((void*) P_A, (void*) P_B); // each prefetchRUN performs a thousand prefetches of P_A and P_B
    }

    printf("     Time\n"); 
    ////////////////////////////////////////

    ////////////////////////////////////////
    // Interact with human
    getchar(); getchar(); 
    if (ground_truths[lit] == 1){ printf("\n     HIT\n\n"); }
    else                        { printf("\n     MISS\n\n");}
    ////////////////////////////////////////

  }

}

#endif
