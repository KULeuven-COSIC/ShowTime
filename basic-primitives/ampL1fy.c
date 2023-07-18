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

////////////////////////////////////////////////////////////////////////////////
// Helpers
#include "../utils/cache_utils.h"       // Cache manipulation utils
#include "../evsets/list/list_utils.h"  // list conversions
#include "../utils/memory_utils.h"      // mmap and such
#include "../utils/misc_utils.h"        // Misc utils
#include "../utils/configuration.h"     // Configure for your host system
#include "settings.h"                   // Settings for this specific experiment

////////////////////////////////////////////////////////////////////////////////
// Memory Allocations
uint64_t *shared_mem, *evict_mem, *evict_mem2, *evict_mem_L1, *evict_mem_L1_m, *evict_mem_L1_c, *evict_mem_LLC, *evict_mem_LLC_c, *evict_mem_LLC2_c;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void ampl1fy() {

  // Variable declarations
  int i, j, z, xit, yit; uint64_t start, end; volatile int j0=0,j3=0,j4=0; FILE *fp; 
  uint64_t L1_EVICTION[AMP_ITERATIONS], L1_ORDERING[AMP_ITERATIONS], L1_INVALIDATION[AMP_ITERATIONS];
  uint64_t LLC_EVICTION[AMP_ITERATIONS], LLC_ORDER_SCOPE[AMP_ITERATIONS], L1_TO_LLC[AMP_ITERATIONS];
  uint64_t LLC_PREFETCH[AMP_ITERATIONS], X_EVICTED[AMP_ITERATIONS];

  //////////////////////////////////////////////////////////////////////////////
  // Pick a random target_addr from shared_mem
  // Cache Access Thresholds
  // Eviction Set Construction - LLC
  // Eviction Set Construction - S_L1 - L1 sets to which the scope line is also mapped
  #include "preparation_ampL1fy.h"

  // Define macros to assign names to cache lines 
  #include "cache_line_macros.h"

  // Define macros for traversal patterns
  #include "traverse_macros.h" 
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  // Set values to zero to enable the use of data dependencies 
  for (i=0; i<EV_LLC; i++)                     { WRITE_VALUE(evsetLLC[i], 0); }
  for (i=0; i<EV_LLC; i++)                     { WRITE_VALUE(evsetLLC_c[i], 0); }
  for (i=0; i<EV_LLC; i++)                     { WRITE_VALUE(evsetLLC2[i], 0); }
  for (i=0; i<EV_LLC; i++)                     { WRITE_VALUE(evsetLLC2_c[i], 0); }
  for (i=0; i<NUMBER_CONGRUENT_L1; i++)        { WRITE_VALUE(s_evsetL1[i], 0); }
  for (i=0; i<NUMBER_CONGRUENT_L1; i++)        { WRITE_VALUE(s_evsetL1_c[i], 0); }
  for (i=0; i<NUMBER_CONGRUENT_L1; i++)        { WRITE_VALUE(s_evsetL1_m[i], 0); }
  WRITE_VALUE(target_addr, 0);
  WRITE_VALUE(target_addr_m, 0);
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  // Pack all data values in a single L1 set to minimize pollution
    // Create local page-aligned array "fixed_array"
  uint64_t local_array[4096];
  uint64_t offset_for_set_0 = 0x1000 - ((uint64_t)local_array & 0xFFF);
  uint64_t *fixed_array = &local_array[offset_for_set_0/8];

  // Create offsets that will land in the same L1
  #define L1a_OFFSET (target_index+7*8+(0<<9))
  #define L1b_OFFSET (target_index+7*8+(1<<9))
  #define L1c_OFFSET (target_index+7*8+(2<<9))
  #define LLC_OFFSET (target_index+7*8+(3<<9))
  #define LLC2_OFFSET (target_index+7*8+(4<<9))
  #define VAR_OFFSET (target_index+7*8+(5<<9))

  #define L1a_LOCATION (&fixed_array[L1a_OFFSET])
  #define L1b_LOCATION (&fixed_array[L1b_OFFSET])
  #define L1c_LOCATION (&fixed_array[L1c_OFFSET])
  #define LLC_LOCATION (&fixed_array[LLC_OFFSET])
  #define LLC2_LOCATION (&fixed_array[LLC2_OFFSET])
  #define VAR_LOCATION (&fixed_array[VAR_OFFSET])

  // 8 pointers (8 byte each) fit in one cache line (64 bytes)
  for (i = 0; i < 8; i++){ L1a_LOCATION[i] = s_evsetL1[i]; }
  for (i = 0; i < 8; i++){ L1b_LOCATION[i] = s_evsetL1_c[i]; }
  for (i = 0; i < 8; i++){ L1c_LOCATION[i] = s_evsetL1[8+i]; }
  for (i = 0; i < 8; i++){ LLC_LOCATION[i] = evsetLLC[i]; }
  for (i = 0; i < 8; i++){ LLC2_LOCATION[i] = evsetLLC[8+i]; }

  VAR_LOCATION[6] = target_addr_m;
  VAR_LOCATION[1] = 1;
  VAR_LOCATION[2] = 0;
  VAR_LOCATION[3] = target_addr_m2;
  VAR_LOCATION[4] = target_addr;
  VAR_LOCATION[5] = 0;
  VAR_LOCATION[0] = 0;
  VAR_LOCATION[7] = REFRESH_RATE;
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  // EXPERIMENTS

  ///////////////////////
  //// PLRU Amplifier
  // 1. Event: L1 eviction (as in leaky.page)
  // 2. Event: Reordering of L1 accesses 
  // 3. Event: L1 Back-invalidation
  // 4. Event: LLC eviction (CONVERT: LLC -> L1)
  // 5. Event: LLC eviction (repeatable/ordering)
  ///////////////////////
          
  ///////////////////////////
  //// Prefetch Amplifier
  // 6. Event: LLC eviction
  // 7. Event: L1 eviction (CONVERT: L1 -> LLC)
  ///////////////////////////

  //////////////////////////////////////////////
  // 1. Amplify an L1 eviction (leaky.page and variations)
  #if ENABLE_L1_EVICTION == 1
  for (ITERATOR = 0; ITERATOR < AMP_ITERATIONS; ITERATOR++){

    EVICT_L1();                                     // Reset L1 with L1_WAYS cache misses
    X_PLRU_PREPARE_ABC();                           // Prepare the tree to contain (AB-CD)-(EF-GH) 

    j0 &= READ(X_Y + 789*(ITERATOR & 0x1) + j0);    // Access to X_Y in odd iterations
                                                    // j0 is used for the data dependency chain throughout

    L1_EVICTION[ITERATOR] = MEASURE_PLRU(); // Traverse pattern
  }
  #endif // ENABLE_L1_EVICTION
  //////////////////////////////////////////////

  //////////////////////////////////////////////
  // 2. Amplify L1 reordering 
  #if ENABLE_L1_ORDERING == 1
  for (ITERATOR = 0; ITERATOR < AMP_ITERATIONS; ITERATOR++){
    
    // Prepare L1 PLRU state
    EVICT_L1();                                     // Reset L1 with L1_WAYS cache misses
    X_PLRU_PREPARE_ABC();                           // Prepare the tree to contain (AB-CD)-(EF-GH) or some equivalent permutation

    // Difference in access order for odd and even iterations
    if (ITERATOR & 0x1) {LMFENCE; TOUCH(X_B); TOUCH(X_Y); LMFENCE} 
    else {LMFENCE; TOUCH(X_Y); TOUCH(X_B); LMFENCE}  

    // Amplify: traverse L1 PLRU pattern for which the runtime is conditioned on L1 replacement status
    // Measure: turn this time difference into architectural difference
    X_TRAVERSE_ORDER_CONVERSION();          // Adaptor pattern FHB so we can use the original traversal
    L1_ORDERING[ITERATOR] = MEASURE_PLRU(); // Traverse pattern

  }
  #endif // ENABLE_L1_ORDERING
  //////////////////////////////////////////////

  //////////////////////////////////////////////
  // 3. Amplify L1 Back-Invalidation
  #if ENABLE_L1_INVALIDATION == 1

  #define SCOPE X_A // Scope line for Prime+Scope

  for (ITERATOR = 0; ITERATOR < AMP_ITERATIONS; ITERATOR++){

    #if AMPLIFIER_LLC_CONGRUENT == 1
      // Will not work if the lines for the PLRU amplifier are itself LLC-congruent
      printf("\nWarning: did not run back-invalidation test, results are invalid.\n");
      break;
    #endif

    // Prime step of Prime+Scope
    FLUSH(X_PS); FLUSH(SCOPE); LMFENCE;
    for (int it=0; it<LLC_WAYS; it++){ TOUCH(evsetLLC_c[it]); }
    LMFENCE; prefetchNTA((void*) SCOPE); LMFENCE; // add reliability (see Leaky Way, Guo et al.)

    // Prepare the L1 tree to contain (AB-CD)-(EF-GH), and make E the EVC
    TOUCH(SCOPE); TOUCH(X_E); TOUCH(X_C); TOUCH(X_G);
    TOUCH(X_B); TOUCH(X_F); TOUCH(X_D); TOUCH(X_H);
    TOUCH(SCOPE);
    // Simulate cross-core access for LLC eviction (cache line fill in LLC but not in L1)
    if (ITERATOR & 0x1) {
      LMFENCE;
      prefetchT2((void*) X_PS);
    }
    LMFENCE;

    // If the LLC fill evicted the scope line X_A, X_A will be back-invalidated in L1
      // In that case, X_X will take the position X_A used to occupy
      // Otherwise, X_X will take the position of X_E, which was the EVC
    TOUCH(X_X);

    X_TRAVERSE_ORDER_CONVERSION();              // Adaptor pattern
    L1_INVALIDATION[ITERATOR] = MEASURE_PLRU(); // Traverse pattern
  }

  #endif // ENABLE_L1_INVALIDATION
  //////////////////////////////////////////////

  //////////////////////////////////////////////
  // 4. Convert LLC state difference to L1 state difference, for use with the L1 amplifier
  #if ENABLE_LLC_EVICTION == 1
  for (ITERATOR = 0; ITERATOR < AMP_ITERATIONS; ITERATOR++){

    // Just for the experiment: target is uncached in odd runs
    TOUCH(X_T2);
    if (ITERATOR & 0x1) {LMFENCE; FLUSH(X_T2); LMFENCE;}

    // Prepare L1 PLRU state
    EVICT_L1();                                     // Reset L1 with L1_WAYS cache misses
    X_PLRU_PREPARE_ABC();                           // Prepare the tree to contain (AB-CD)-(EF-GH) or some equivalent permutation

    ////////////////////////////////////////////////////////////////////////////////////
    // Convert: Turn LLC status into L1 replacement status through reordering
    j3 = READ(X_T2^j0);                              // The target line access we want to measure
                                                    // If target cached     -> D-H -> A is the EVC -> traversal slow
                                                    // If target not cached -> H-D -> E is the EVC -> traversal fast
    j3 = READ(X_D^j3);

                                        #ifdef KABYLAKE12
                                        for (z = 20^j0; z > 0; z--) {}
                                        //z = j0/X_Z;  z = z/X_Z;  z = z/X_Z; 
                                        #else
                                        for (z = 17^j0; z > 0; z--) {}
                                        #endif
                                        j4 = READ(X_H^z);
    j0 = READ(X_X^j4^j3);  
    ////////////////////////////////////////////////////////////////////////////////////

    // Amplify: traverse L1 PLRU pattern for which the runtime is conditioned on L1 replacement status
    // Measure: turn this time difference into architectural difference
    X_TRAVERSE_ORDER_CONVERSION();           // Adaptor pattern 
    LLC_EVICTION[ITERATOR] = MEASURE_PLRU(); // Traverse pattern
  }
  #endif // ENABLE_LLC_EVICTION
  //////////////////////////////////////////////

  //////////////////////////////////////////////
  // 5. Amplify LLC order scope
  #if ENABLE_LLC_ORDER_SCOPE == 1
  for (ITERATOR = 0; ITERATOR < AMP_ITERATIONS; ITERATOR++){

    READ_ACCESS(&X_Z);
    READ_ACCESS(&z);
    READ_ACCESS(X_T);
    READ_ACCESS(X_T2);
    
    // Prepare L1 PLRU state
    EVICT_L1();                                     // Reset L1 with L1_WAYS cache misses
    X_PLRU_PREPARE_ABC();                           // Prepare the tree to contain (AB-CD)-(EF-GH) or some equivalent permutation


    uint64_t start = rdtsc();
    for (int lit=0; lit<100; lit++){

      // For odd traversals, X_T2 is evicted in loop iteration 4 and X_T in 43 (so X_T2 first, then X_T)
      // For even traversals, X_T2 is evicted in loop iteration 43 and X_T in 4 (so X_T first, then X_T2)
      // This is the ordering diference we are converting

      if (lit == 4){
        if (ITERATOR & 0x1){
          LMFENCE; FLUSH(X_T2); LMFENCE;
        } else {
          LMFENCE; FLUSH(X_T); LMFENCE;
        }
      }

      if (lit == 43){
        if (ITERATOR & 0x1){
          LMFENCE; FLUSH(X_T); LMFENCE;
        } else {
          LMFENCE; FLUSH(X_T2); LMFENCE;
        }
      }

      #ifdef KABYLAKE16
      #define NB_LOOPS_ORDER_SCOPE 11
      #else
      // may need to tweak for your platform
      #define NB_LOOPS_ORDER_SCOPE 11
      #endif
      #define LOOP_METHOD 1
      #if LOOP_METHOD == 1
        // Loop method
        #define RHS_DELAY_ORDER_SCOPE() ({ \
          for (z = NB_LOOPS_ORDER_SCOPE^j0; z > 0; z--) {} \
        })
      #else
        // Division method
        #define RHS_DELAY_ORDER_SCOPE() ({ \
          z = j0/X_Z; \
        })
      #endif

      ////////////////////////////////////////////////////////////////////////////////////
      // Time to Order: Process Scope line 1
      TOUCH(X_B); TOUCH(X_C); TOUCH(X_F); TOUCH(X_A); TOUCH(X_B);
      j3 = READ(X_T^j0);                             // The target line access we want to measure
                                                      // If target cached     -> D-H -> A is the EVC -> traversal slow
                                                      // If target not cached -> H-D -> E is the EVC -> traversal fast
      j3 = READ(X_C^j3);
                                          RHS_DELAY_ORDER_SCOPE();
                                          j4 = READ(X_G^z);
      j0 = READ(X_X^j4^j3);  
      ////////////////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////////////////
      // Time to Order: Process Scope line 2
      TOUCH(X_B); TOUCH(X_C); TOUCH(X_F); TOUCH(X_A); TOUCH(X_B);
      j3 = READ(X_T2^j0);                              // The target line access we want to measure
                                                      // If target cached     -> D-H -> A is the EVC -> traversal slow
                                                      // If target not cached -> H-D -> E is the EVC -> traversal fast
      j3 = READ(X_C^j3);
                                          RHS_DELAY_ORDER_SCOPE();
                                          j4 = READ(X_G^z);
      j0 = READ(X_Y^j4^j3);  
      ////////////////////////////////////////////////////////////////////////////////////
    }
    uint64_t end = rdtsc()-start;

    X_TRAVERSE_ORDER_CONVERSION();              // Adaptor pattern
    LLC_ORDER_SCOPE[ITERATOR] = MEASURE_PLRU(); // Traverse pattern
  }
  #endif // ENABLE_LLC_ORDER_SCOPE
  //////////////////////////////////////////////

  //////////////////////////////////////////////
  // 6. Amplify LLC presence with prefetch amplifier
  #if ENABLE_LLC_PREFETCH == 1
  j0=0;
  WRITE_VALUE(M_A,0);
  WRITE_VALUE(M_B,0);
  for (int it=0; it<LLC_WAYS; it++){ WRITE_VALUE(evsetLLC2[it],0); }
  for (int it=2; it<LLC_WAYS; it++){ WRITE_VALUE(evsetLLC2_c[it],0); }
  for (ITERATOR = 0; ITERATOR < AMP_ITERATIONS; ITERATOR++){

    for (int it=0; it<LLC_WAYS; it++){ FLUSH(evsetLLC2[it]); LMFENCE;}
    for (int it=0; it<LLC_WAYS; it++){ FLUSH(evsetLLC2_c[it]); LMFENCE;}
    for (int it=0; it<LLC_WAYS; it++){ TOUCH(evsetLLC2[it]); LMFENCE;}
    
    if (ITERATOR & 0x1){ LMFENCE; READ(M_A); } LMFENCE; // Access on odd iterations

    LLC_PREFETCH[ITERATOR] = MEASURE_PREFETCH(M_A, M_B); // Traverse pattern
  }
  #endif // ENABLE_L1_EVICTION
  //////////////////////////////////////////////
  
  //////////////////////////////////////////////
  // 7. Convert L1 vs L2 state difference to LLC state difference
  #if ENABLE_L1_TO_LLC == 1

  #ifdef KABYLAKE16 // Intel Core i7-7700K
    #define RHS_DELAY_L1_LLC() ({ \
      for (z = 29^j0; z > 0; z--) {} \
    })
  #else
    // may need to tweak for your platform
    #define RHS_DELAY_L1_LLC() ({ \
      for (z = 26^j0; z > 0; z--) {} \
    })
  #endif

  WRITE_VALUE(X_Y + 128, 0);

  for (ITERATOR = 0; ITERATOR < AMP_ITERATIONS; ITERATOR++){
    EVICT_L1();                                      // Reset L1 with L1_WAYS cache misses
    X_PLRU_PREPARE_ABC();                            // Prepare the tree to contain (AB-CD)-(EF-GH) 

    TOUCH(X_Y + 128*(ITERATOR & 0x1));               // Access in odd iterations
    LMFENCE;

    FLUSH(M_A); FLUSH(M_B);
    for (int it=0; it<LLC_WAYS; it++){ TOUCH(evsetLLC2[it]); }
    LMFENCE;

    // Time to Order
    // Left: (L1)^8-RAM or (L2/LLC)^8-RAM
    j3 = READ(X_A^j0);
    j3 = READ(X_E^j3);
    j3 = READ(X_C^j3);
    j3 = READ(X_G^j3);
    j3 = READ(X_F^j3);
    j3 = READ(X_D^j3);
    j3 = READ(X_H^j3);
    j3 = READ(X_B^j3);
                                    RHS_DELAY_L1_LLC();
                                    prefetchNTANoFence((void*) (M_A^z));
    j3 = READ(M_A^j3);
                                    
    LMFENCE;
    prefetchNTANoFence((void*) (M_B^j3^z));

    L1_TO_LLC[ITERATOR] = MEASURE_PREFETCH(M_A, M_B); // Traverse pattern
  }

  #endif // ENABLE_L1_TO_LLC
  //////////////////////////////////////////////

  #if ENABLE_L1_EVICTION == 1
    fp = fopen ("./log/L1_EV_NA.log","w");
    for (i=0; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", L1_EVICTION[i]); }
    fclose (fp); fp = fopen ("./log/L1_EV_A.log","w");
    for (i=1; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", L1_EVICTION[i]); }
    fclose (fp);
  #endif // ENABLE_L1_EVICTION
  
  #if ENABLE_L1_ORDERING == 1
    fp = fopen ("./log/L1_ORD_NA.log","w");
    for (i=0; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", L1_ORDERING[i]); }
    fclose (fp); fp = fopen ("./log/L1_ORD_A.log","w");
    for (i=1; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", L1_ORDERING[i]); }
    fclose (fp);
  #endif // ENABLE_L1_ORDERING

  #if ENABLE_L1_INVALIDATION == 1
    fp = fopen ("./log/L1_INV_NA.log","w");
    for (i=0; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", L1_INVALIDATION[i]); }
    fclose (fp); fp = fopen ("./log/L1_INV_A.log","w");
    for (i=1; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", L1_INVALIDATION[i]); }
    fclose (fp);
  #endif // ENABLE_L1_INVALIDATION

  #if ENABLE_LLC_EVICTION == 1
    fp = fopen ("./log/LLC_EV_NA.log","w");
    for (i=0; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", LLC_EVICTION[i]); }
    fclose (fp); fp = fopen ("./log/LLC_EV_A.log","w");
    for (i=1; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", LLC_EVICTION[i]); }
    fclose (fp);
  #endif // ENABLE_LLC_EVICTION

  #if ENABLE_LLC_ORDER_SCOPE == 1
    fp = fopen ("./log/LLC_ORD_NA.log","w");
    for (i=0; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", LLC_ORDER_SCOPE[i]); }
    fclose (fp); fp = fopen ("./log/LLC_ORD_A.log","w");
    for (i=1; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", LLC_ORDER_SCOPE[i]); }
    fclose (fp);
  #endif // ENABLE_LLC_ORDER_SCOPE

  #if ENABLE_L1_TO_LLC == 1
    fp = fopen ("./log/L1_LLC_NA.log","w");
    for (i=0; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", L1_TO_LLC[i]); }
    fclose (fp); fp = fopen ("./log/L1_LLC_A.log","w");
    for (i=1; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", L1_TO_LLC[i]); }
    fclose (fp);
  #endif // ENABLE_L1_TO_LLC

  #if ENABLE_LLC_PREFETCH == 1
    fp = fopen ("./log/LLC_PREF_NA.log","w");
    for (i=0; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", LLC_PREFETCH[i]); }
    fclose (fp); fp = fopen ("./log/LLC_PREF_A.log","w");
    for (i=1; i<AMP_ITERATIONS; i+=2) { fprintf (fp, "%lu\n", LLC_PREFETCH[i]); }
    fclose (fp);
  #endif // ENABLE_LLC_PREFETCH

  fp = fopen ("./log/metadata.log","w");
  fprintf(fp, "%d\n%d\n%d\n%d\n%d\n%d\n",PLRU_TRAVERSE,PREFETCH_TRAVERSE,AMPLIFIER_LLC_CONGRUENT,HUGE_PAGES_AVAILABLE, DISTANCE, REFRESH);
  fclose(fp);

}

int main(int argc, char **argv){

  // mmap some memory regions
  ASSERT(mem_map_private(&shared_mem, SHARED_MEM_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem2, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem_L1, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem_L1_c, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem_L1_m, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem_LLC, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem_LLC_c, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem_LLC2_c, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));

  //////////////////
      ampl1fy(); 
  //////////////////

  // ummap some memory regions
  ASSERT(munmap(shared_mem, SHARED_MEM_SIZE)); 
  ASSERT(munmap(evict_mem,  EVICT_LLC_SIZE));
  ASSERT(munmap(evict_mem2,  EVICT_LLC_SIZE));
  ASSERT(munmap(evict_mem_L1,  EVICT_LLC_SIZE));
  ASSERT(munmap(evict_mem_L1_c,  EVICT_LLC_SIZE));
  ASSERT(munmap(evict_mem_L1_m,  EVICT_LLC_SIZE));
  ASSERT(munmap(evict_mem_LLC,  EVICT_LLC_SIZE));
  ASSERT(munmap(evict_mem_LLC_c,  EVICT_LLC_SIZE));
  ASSERT(munmap(evict_mem_LLC2_c,  EVICT_LLC_SIZE));

  return 0;

}
