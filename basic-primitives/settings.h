/////////////////////////////////
// Experimental parameters
#define AMP_ITERATIONS  100         // Number of iterations to construct the histograms
#define AMPLIFIER_LLC_CONGRUENT 0   // Amplification Factor is higher if lines are LLC-congruent
#define NUMBER_CONGRUENT_L1 16      // Number of L1-congruent addresses to find in preparation
#define SILENT_MODE 1               // Don't report helper stuff
#define RANDOM_TARGET 1             // Determine the index of the target cache line randomly

// L1 PLRU amplifier
#define PLRU_TRAVERSE 1000          // Number of traversals of the L1 pattern
#define DISTANCE 1                  // Distance-1 or Distance-2 or Distance-3 patterns
#define REFRESH 0                   // Enable or disable refresh patterns
#define REFRESH_RATE ((1 << 8) - 1) // Rate at which refreshes are executed, if enabled

// LLC Prefetch amplifier
#define PREFETCH_TRAVERSE 100       // Number of traversals (*1K) of the LLC pattern
/////////////////////////////////

//////////////////////////////
// Which experiments to run
#define ENABLE_L1_EVICTION      1
#define ENABLE_L1_ORDERING      1
#define ENABLE_L1_INVALIDATION  1
#define ENABLE_LLC_EVICTION     1
#define ENABLE_LLC_ORDER_SCOPE  1
#define ENABLE_LLC_PREFETCH     1
#define ENABLE_L1_TO_LLC        1
#define ENABLE_LLC_TO_PREF      1
//////////////////////////////
