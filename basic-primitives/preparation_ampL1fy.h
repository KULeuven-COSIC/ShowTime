  ///////////////////////////
  // Select target addresses
  ///////////////////////////

  // Minimize the number of L1 cache sets that are polluted by the container arrays
  uint64_t s_evsetL1[64*8];
  uint64_t s_evsetL1_c[64*8];
  uint64_t s_evsetL1_m[64*8];

  int seed = time(NULL); srand(seed);
  #if RANDOM_TARGET == 1
    int target_index = (rand() & 0x3F)*8;
  #else
    int good_target_index = (((uint64_t)s_evsetL1 & 0xFFF) >> 3);
    int target_index = good_target_index+17*8;
  #endif
  
  uint64_t target_addr             = (uint64_t)&shared_mem[target_index];
  uint64_t target_addr_m           = (uint64_t)&shared_mem[target_index+17*8];
  uint64_t target_addr_m2          = (uint64_t)&shared_mem[target_index+20*8];

  ///////////////////////////////
  // Configure access thresholds
  ///////////////////////////////

  int thrL1, thrRAM, thrDET;
  configure_thresholds(target_addr, &thrL1, &thrRAM, &thrDET);

  printf("\nThresholds Configured\n\n");
  printf("\tL1/L2    : %u\n", thrL1   );
  printf("\tRAM      : %u\n", thrRAM  );
  printf("\tTHRESHOLD: %u\n", thrDET  );

  // Remind the user how many LLC ways we are assuming
  printf("\nConfiguration: %d LLC ways\n", LLC_WAYS);

  /////////////////////
  // LLC Eviction Sets
  /////////////////////

#if PREMAP_PAGES == 1
  ps_evset_premap(evict_mem);
#endif

  Elem  *evsetList;  Elem **evsetList_ptr = &evsetList;
  *evsetList_ptr=NULL;

repeat_evset:
  if (PS_SUCCESS != ps_evset( evsetList_ptr,
                              (char*)target_addr,
                              EV_LLC,
                              evict_mem,
                              HUGE_PAGES_AVAILABLE,
                              thrDET))
    goto repeat_evset;

  #if SILENT_MODE == 0
  printf("\nEviction set is constructed successfully");
  #endif

  // Convert the eviction set link-list to an array
  uint64_t evsetLLC[EV_LLC]; list_to_array(evsetList, evsetLLC);

#if PREMAP_PAGES == 1
  ps_evset_premap(evict_mem2);
#endif

  Elem  *evsetList2;  Elem **evsetList2_ptr = &evsetList2;
  *evsetList2_ptr=NULL;

repeat_evset2:
  if (PS_SUCCESS != ps_evset( evsetList2_ptr,
                              (char*)target_addr_m2,
                              EV_LLC,
                              evict_mem2,
                              HUGE_PAGES_AVAILABLE,
                              thrDET))
    goto repeat_evset2;

  #if SILENT_MODE == 0
  printf("\nEviction set is constructed successfully");
  #endif

  // Convert the eviction set link-list to an array
  uint64_t evsetLLC2[EV_LLC]; list_to_array(evsetList2, evsetLLC2);

#if PREMAP_PAGES == 1
  ps_evset_premap(evict_mem_LLC_c);
#endif

  Elem  *evsetList3;  Elem **evsetList3_ptr = &evsetList3;
  *evsetList3_ptr=NULL;

repeat_evset3:
  if (PS_SUCCESS != ps_evset( evsetList3_ptr,
                              (char*)target_addr,
                              EV_LLC,
                              evict_mem_LLC_c,
                              HUGE_PAGES_AVAILABLE,
                              thrDET))
    goto repeat_evset3;

  #if SILENT_MODE == 0
  printf("\nEviction set is constructed successfully");
  #endif

  // Convert the eviction set link-list to an array
  uint64_t evsetLLC_c[EV_LLC]; list_to_array(evsetList3, evsetLLC_c);


#if PREMAP_PAGES == 1
  ps_evset_premap(evict_mem_LLC2_c);
#endif

  Elem  *evsetList4;  Elem **evsetList4_ptr = &evsetList4;
  *evsetList4_ptr=NULL;

repeat_evset4:
  if (PS_SUCCESS != ps_evset( evsetList4_ptr,
                              (char*)target_addr_m2,
                              EV_LLC,
                              evict_mem_LLC2_c,
                              HUGE_PAGES_AVAILABLE,
                              thrDET))
    goto repeat_evset4;

  #if SILENT_MODE == 0
  printf("\nEviction set is constructed successfully");
  #endif

  // Convert the eviction set link-list to an array
  uint64_t evsetLLC2_c[EV_LLC]; list_to_array(evsetList4, evsetLLC2_c);

  ////////////////////
  // L1 Eviction Sets
  ////////////////////

#if PREMAP_PAGES == 1
  ps_evset_premap(evict_mem_L1);
  ps_evset_premap(evict_mem_L1_c);
  ps_evset_premap(evict_mem_L1_m);
#endif

construct_L1_eviction_set(s_evsetL1, evict_mem_L1, target_addr, NUMBER_CONGRUENT_L1, SMALLPAGE_PERIOD, MAX_POOL_SIZE_SMALL);
construct_L1_eviction_set(s_evsetL1_c, evict_mem_L1_c, target_addr, NUMBER_CONGRUENT_L1, SMALLPAGE_PERIOD, MAX_POOL_SIZE_SMALL);
construct_L1_eviction_set(s_evsetL1_m, evict_mem_L1_m, target_addr_m2, NUMBER_CONGRUENT_L1, SMALLPAGE_PERIOD, MAX_POOL_SIZE_SMALL);