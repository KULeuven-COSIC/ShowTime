#pragma once

////////////////////////////////////////////////////////////////////////////////
// Supported platforms for this PoC

#define KABYLAKE16 // Intel Core i7-7700K

//#define KABYLAKE12
//#define KABYLAKE16
// #define COFFEELAKE12
// #define SKYLAKE16
//#define SKYLAKE12
// #define HASWELL16
// #define HASWELL12
// #define IVYBRIDGE16
// #define IVYBRIDGE12
// #define SANDYBRIDGE12
// #define XEONSILVER12
// #define XEONPLATINUM12
// #define YOUROWNPLATFORM 

#ifdef YOUROWNPLATFORM
  // LLC configuration is picked among various ones defined in ./cache_info.h
  #define LLC_INCLUSIVE 
  #define LLC_WITH_12_WAY_1024_SET 
  
  // You should also define your own PRIME() function in ./prime.h
#endif

// Map platforms to their specs
#include "../utils/platforms.h"


////////////////////////////////////////////////////////////////////////////////
// Core allocation

/*
  Ensure that attacker and victim (and helper) reside on different physical cores.
  In case of a hyperthreaded machine, logical siblings can be learned, e.g.: 
  cat /sys/devices/system/cpu/cpu0/topology/thread_siblings_list 
*/
#define ATTACKER_CORE   2
#define HELPER_CORE     0
#define VICTIM_CORE     3

////////////////////////////////////////////////////////////////////////////////
// Application Specific Configuration

/* If huge pages are available, indicate it. */
#define HUGE_PAGES_AVAILABLE 0

#include "../utils/memory_sizes.h"    // For KB, MB, GB
#define EVICT_L1_SIZE         ( 8*MB)
#define EVICT_L2_SIZE         ( 8*MB)
#if LLC_SLICES > 8
  #define EVICT_LLC_SIZE      (512*MB) // especially the 28-slice machines need larger pool
#else
  #define EVICT_LLC_SIZE      (128*MB) // especially the 28-slice machines need larger pool
#endif
#define SHARED_MEM_SIZE       (128*MB)
  // Pick memory sizes. EVICT_LLC_SIZE is important as it defines the size of
  // guess pool, which consists of addresses potentially congruent with the target
 
#define TEST_LEN  10
#define MAX_EVSET_CONST_RETRY 25

// The parameters below are for tweaking the evset construction

#define RANDOMIZE_TARGET 1
  // Pick a random tests for every loop of the tests

#define RENEW_THRESHOLD 0
  // Recalculate the cache access threshold for every loop of the tests

#define PREMAP_PAGES 1
  // Ensure that all physical pages of the evset buffer (guess pool) are mapped (and not copy-on-write)
  // before the accessing them in the evset construction.
  // Important for small pages.

////////////////////////////////////////////////////////////////////////////////
// Eviction Set Construction (../evsets) Parameters

#define ENABLE_ALREADY_FOUND 1
  // Uses the premature list when finding a new congruent address.

#define ENABLE_EARLY_EVICTION_CHECK 1
  // Tests whether the premature list evicts the victim
  // before any guess is accessed.

#define ENABLE_EXTENSION 1
  // Tests whether the found list can evict the victim.
  // If this fails, extends the list until it evicts.

#define ENABLE_REDUCTION 1
  // Reduces the list length to LLC length
  // by removing the list elements if they don't help eviction
  // Should be used with ENABLE_EXTENSION.

#define RANDOMIZE_GUESS_POOL 1
  // The guess pool (addresses which might be congruent with the target) is
  // random. If disabled, an ordered list of addresses are used.

#define IGNORE_VERY_SLOW 1
  // If an access to the target is slow, the guess might have evicted it, hence
  // it should be congruent. However, if the acccess is too slow, then
  // something might have gone wrong.

/// Parameters

#define MAX_EXTENSION  32
#define MAX_ATTEMPT    20
#define EV_LLC LLC_WAYS
//#define EARLY_TEST_LEN 10

// For access to the types and return values of evset
#include "../evsets/ps_evset.h"


#ifdef LLC_INCLUSIVE 

  #define BLOCK_OFFSET      6 // 64B cache lines

  //////////////////////////////////////////////////////////////////////////////
  // Per-core L1D
  #define L1_WAYS           8
  #define L1_INDEX_BITS     (BLOCK_OFFSET + 6) // 64 sets
  #define L1_PERIOD         (1 << L1_INDEX_BITS)
  #define L1_SET_INDEX(x)   ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << L1_INDEX_BITS)-1);index;})

  //////////////////////////////////////////////////////////////////////////////
  // Per-core L2
  #define L2_WAYS           4
  #define L2_INDEX_BITS     (BLOCK_OFFSET + 10) // 1024 sets
  #define L2_PERIOD         (1 << L2_INDEX_BITS)
  #define L2_SET_INDEX(x)   ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << L2_INDEX_BITS)-1);index;})

  //////////////////////////////////////////////////////////////////////////////
  // Shared inclusive LLC

  #ifdef LLC_WITH_12_WAY_1024_SET
    #define LLC_SLICES        8
    #define LLC_WAYS          12
    #define LLC_INDEX_BITS    (BLOCK_OFFSET + 10) // 1024 sets
    #define LLC_PERIOD        (1 << LLC_INDEX_BITS)
    #define LLC_SET_INDEX(x)  ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << LLC_INDEX_BITS)-1);index;})
  #endif

  #ifdef LLC_WITH_16_WAY_1024_SET
    #define LLC_SLICES        8
    #define LLC_WAYS          16
    #define LLC_INDEX_BITS    (BLOCK_OFFSET + 10) // 1024 sets
    #define LLC_PERIOD        (1 << LLC_INDEX_BITS)
    #define LLC_SET_INDEX(x)  ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << LLC_INDEX_BITS)-1);index;})
  #endif

  #define SMALLPAGE_PERIOD    (1 << 12)

#endif // LLC_INCLUSIVE


#ifdef LLC_NONINCLUSIVE 
  // The following is tested on both Intel Xeon Silver 4208 and Platinum 8280

  #define BLOCK_OFFSET      6 // 64B cache lines

  //////////////////////////////////////////////////////////////////////////////
  // Per-core L1D
  #define L1_WAYS           8
  #define L1_INDEX_BITS     (BLOCK_OFFSET + 6) // 64 sets
  #define L1_PERIOD         (1 << L1_INDEX_BITS)
  #define L1_SET_INDEX(x)   ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << L1_INDEX_BITS)-1);index;})

  //////////////////////////////////////////////////////////////////////////////
  // Per-core L2
  #define L2_WAYS           16
  #define L2_INDEX_BITS     (BLOCK_OFFSET + 10) // 1024 sets
  #define L2_PERIOD         (1 << L2_INDEX_BITS)
  #define L2_SET_INDEX(x)   ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << L2_INDEX_BITS)-1);index;})

  //////////////////////////////////////////////////////////////////////////////
  // Shared non-inclusive LLC

  #ifdef LLC_WITH_11_WAY_2048_SET_8_SLICE
    #define LLC_SLICES        8 
    #define LLC_WAYS          11
    #define LLC_INDEX_BITS    (BLOCK_OFFSET + 11) // 2048 sets per slice
    #define LLC_PERIOD        (1 << LLC_INDEX_BITS)
    #define LLC_SET_INDEX(x)  ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << LLC_INDEX_BITS)-1);index;})
    #define CD_WAYS           12
  #endif

  #ifdef LLC_WITH_11_WAY_2048_SET_28_SLICE
    #define LLC_SLICES        28 
    #define LLC_WAYS          11
    #define LLC_INDEX_BITS    (BLOCK_OFFSET + 11) // 2048 sets per slice
    #define LLC_PERIOD        (1 << LLC_INDEX_BITS)
    #define LLC_SET_INDEX(x)  ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << LLC_INDEX_BITS)-1);index;})
    #define CD_WAYS           12
  #endif

  #define SMALLPAGE_PERIOD    (1 << 12)

#endif // LLC_NONINCLUSIVE
