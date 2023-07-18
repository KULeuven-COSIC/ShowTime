# Basic Primitives

This folder is a testing ground for some of the main basic primitives of the paper.
If you want to port a PoC to another machine, you may want to start here.

| # | Event | Convert | Amplify | Paper Sections |
| :---: | :---: | :---: | :---: | :---: |
| 1 | L1 Eviction | / | L1 PLRU | Sec 4.1 |
| 2 | L1 Reordering | / | L1 PLRU | Sec 4.1 |
| 3 | LLC Presence | Back-Invalidation | L1 PLRU | Sec 4.1 and 5.1|
| 4 | LLC Presence | Time to Order (to L1 Reordering)| L1 PLRU | Sec 4.1 and 5.2 |
| 5 | LLC Presence | Time to Order Repeatable (to L1 Reordering)| L1 PLRU | Sec 4.1 and 5.2 |
| 6 | LLC Presence | / | Prefetch | Sec 4.2 |
| 7 | L1 Presence | Time to Order (to LLC Presence) | Prefetch | Sec 4.2 and 5.2|

## Configuration
- Ensure that the global configuration parameters are [set correctly](../utils/README.md).
- All experiment-specific parameters are configured in `settings.h`, or at the relevant location in the code.
	- The amplification ratio of the L1 amplifier is much higher if the lines used in the traversal are LLC-congruent (which can be set in `settings.h` with the `AMPLIFIER_CONGRUENT_LLC` macro)
		- For L1-congruent lines, the slow event after amplification takes 1.3x longer than the fast event
		- For LLC-congruent lines, the slow event after amplification takes >2x longer than the fast event
	- The robustness _will_ benefit from increasing the `DISTANCE`, ranging from 1 to 3. See the paper for more information
		-  Increasing the distance comes at the cost of reducing the amplification ratio (as a larger fraction of the memory accesses do not carry the hit/miss signal).
	- The robustness _may_ benefit from using refreshes, which can be configured using the `REFRESH` and `REFRESH_INTERVAL` macros

To (attempt to) minimize pollution of the L1d cache and increase the robustness of the PoC, data values are packed in specific L1d cache sets which are not congruent with the target

## Execution

To execute the PoC:

1. Compile with `make`
2. Run with `make run`
    - This runs the binary `amp`, as well as `histo.py` which processes the measurements.
    - In particular, it produces histograms for the data files, allowing quick visual inspection of the success of the amplifier (needs Python). The histograms are produced in `./log`

### Expected Output

This is the output on an Intel Core i7-7700K, running Rocky Linux 8.8:

```
Thresholds Configured

	L1/L2    : 39
	RAM      : 304
	THRESHOLD: 215

Configuration: 16 LLC ways

#########
 L1 PLRU
#########
1. L1_EV
	Ratio of medians is 1.31

2. L1_ORD
	Ratio of medians is 1.30

3. L1_INV
	Ratio of medians is 1.31

4. LLC_EV
	Ratio of medians is 1.30

5. LLC_ORD
	Ratio of medians is 1.31


##########
 PREFETCH
##########
6. LLC_PREF
	Ratio of medians is 9.89

7. L1_LLC
	Ratio of medians is 9.88


See ./log for histogram png files 
```

with the default parameters (distance-1 patterns, no refresh, no LLC-congruence).


### Disclaimer
These PoCs have been developed on an Intel Core i7-7700K (Kaby Lake, 16-way LLC). We expect many of them to run on other Intel processors too, but specific parameters of the PoCs may be sensitive to your specific CPU.

In particular:
- Check whether the timing thresholds for L1/LLC/RAM make sense on your platform
- Check the correct LLC associativity is configured in `../utils/configuration.h`
- For the LLC amplifier, check whether `prefetchNTA` has the expected behavior on your platform (for more details, see the [Leaky Way](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=9923849) paper by Guo et al.)
- For the Time to Order conversions: you may want to vary the length of the fixed branch 
