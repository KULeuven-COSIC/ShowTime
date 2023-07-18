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

#include "settings.h"
#include "../utils/cache_utils.h"
#include "../utils/memory_utils.h"
#include "../utils/misc_utils.h"

////////////////////////////////////////////////////////////////////////////////
// Memory Allocations
extern volatile uint64_t *shared_mem;
extern volatile uint64_t *synchronization;
extern volatile uint64_t *synchronization_params;

////////////////////////////////////////////////////////////////////////////////
// Function declarations
void poc_timer_free();
void attacker_helper();
////////////////////////////////////////////////////////////////////////////////


void attacker() {

  // Spin up helper thread
  if (fork() == 0) {
    set_core(HELPER_CORE, "Attacker Helper");
    attacker_helper();
    return;
  }

  poc_timer_free();

  // Shut Down
  *synchronization = -1;
  sleep(1);
}

////////////////////////////////////////////////////////////////////////////////

void poc_timer_free() {

  //////////////////////////////////////////////////////////////////////////////
  // Include helper macros
  #include "macros.h"
  //////////////////////////////////////////////////////////////////////////////

  #define EXP_ITERATIONS 10000              // Number of iterations
  int i, j, it;                             // Loop variables
  uint64_t FFs[2]; FFs[0] = 0; FFs[1] = 0;  // Bookkeeping

  //////////////////////////////////////////////////////////////////////////////
  // The cache line we will measure

  int seed = time(NULL); srand(seed);
  int target_index = (rand() & 0x3F); // Chosen by fair dice roll
                        // Guaranteed to be random
  uint64_t X = (uint64_t)&shared_mem[target_index*8];
  *(uint64_t*) X = 0;

  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  // The experiment

  READ_ACCESS(X); LMFENCE;

  for (it=0; it < 2*EXP_ITERATIONS; it++){

    // In odd iterations X is uncached
    // In even iterations X is still cached from previous iteration
    if (it & 0x1){ FLUSH(X); LMFENCE;}

    // Let other thread write 0xFF
    HELPER_WRITE();

    // Write 0x00 with this thread
      // If X is cached, this should happen quickly, i.e., before the other thread writes
      // If X is uncached, this should happen slowly, i.e., after the other thread writes
    shared_mem[mread((void*) X)] = 0x00;

    // Wait for other thread to finish
    while(*synchronization != 0);

    // Look at who won the race and keep track
    if (shared_mem[0] == 0xFFFFFFFF) {FFs[it & 0x1]++;}

  }

  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  // The prints

  printf("\n======================");
  printf("\n Timer-free timer PoC");
  printf("\n======================\n");

  printf("\nCached, should see lots of 0xFFs : \t%5ld / %ld (%lf%%)\n", FFs[0], (uint64_t)EXP_ITERATIONS, (FFs[0] * 100) / (double)EXP_ITERATIONS);
  printf("Not cached, should see few 0xFFs : \t%5ld / %ld (%lf%%)\n\n", FFs[1], (uint64_t)EXP_ITERATIONS, (FFs[1] * 100) / (double)EXP_ITERATIONS);

  //////////////////////////////////////////////////////////////////////////////


}
