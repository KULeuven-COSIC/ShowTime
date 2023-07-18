  ///////////////////////////
  // Select target addresses
  ///////////////////////////
  int seed = time(NULL); srand(seed);
  #if RANDOM_TARGET == 1
    int target_index = (rand() & 0xFF)*8;
  #else
    int target_index = 17*8;
  #endif
  
  uint64_t target_addr = (uint64_t)&shared_mem[target_index];
  mwrite((void*)target_addr); // when in doubt, CoW is the culprit

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
  printf("\nEviction set 1 is constructed successfully\n");
  #endif

  // Convert the eviction set link-list to an array
  uint64_t evsetLLC[EV_LLC]; list_to_array(evsetList, evsetLLC);