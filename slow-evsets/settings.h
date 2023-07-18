#define VERIFY_ITERATIONS 30    // sample size for checking soundness of eviction set

#define BASE_FREQ_MHZ    4200   // base frequency of the cpu
#define GRANULARITY_USEC 1000  // artificial timer granularity in microseconds
                                // only tested for >=100

#define RANDOM_TARGET 1         // random target address?
#define CONGRUENCE_CONFIDENCE 3 // consecutive successes of the congruence test
                                    // in order to accept the address as being congruent
#define EARLY_ABORT 0           // stop congruence test if already exceeded number of iterations
                                    // recommended for very coarse-grained timers (>=100000)
#define EVICTION_LOOP 0         // every so often, perform untargeted eviction of the cache set
                                    // recommended for very coarse-grained timers
                                        // (otherwise, might take up too large a portion of the epoch)

#define ITERATION_THRESHOLD (GRANULARITY_USEC/100) 
                                // threshold for how many iterations of the amplifier one 
                                // can do within one timer period
                                // if the eviction set construction fails, 
                                // enable PRINT_TRAVERSE_COUNTERS and see how
                                // it needs to be adjusted

// Tune verbosity
#define PRINT_TRAVERSE_COUNTERS 0 // print how many traversals were performed every period
#define STATUS_UPDATE 1         // print immediately when a congruent address is found
#define SILENT_MODE 1           // suppress some more prints


