# LLC Eviction sets with coarse-grained timers

The code in `slow-evsets` demonstrates constructing an LLC eviction set with coarse-grained timing sources, including the second-granular Unix Epoch.

## Configuration
- Ensure that the global configuration parameters are [set correctly](../utils/README.md).
- All experiment-specific parameters are configured in `settings.h`.
    - The timing source is artificially crippled for this experiment. Please refer to the implementation of `rdtsc_cripple` [here](../utils/cache_utils.c)
		- The crippling parameter is `GRANULARITY_USEC`
		- If `GRANULARITY_USEC` is set to `1000000`, i.e., a timer granularity of 1 second, the UNIX epoch is used instead
	- Note: set the base frequency of your CPU for a reasonably accurate crippled timer.

## Execution
To execute the PoC:

1. Compile with `make`
2. Run with `make run`

### Expected output

This is a sample output on an Intel Core i7-7700K, running Rocky Linux 8.8, for a (relatively) fine-grained timer source:
- Timing source of 1 ms, i.e., `GRANULARITY_USEC` is 1000
- `EARLY_ABORT` and `EVICTION_LOOP` both **disabled**

Note that executions are random so the runtime, addresses and congruent indices will be different for a particular execution.

```
Core  2 for Attacker

Thresholds Configured

	L1/L2    : 39
	RAM      : 229
	THRESHOLD: 165

Configuration: 16 LLC ways

	Granularity [ms]          1.0000 ms
	Granularity [cycles]     4200000 cycles
	Iteration threshold           10 
	Congruence confidence          3 in a row

Found  1 -    19 - 0x7fcb019ac680
Found  2 -   195 - 0x7fcb01a5c680
Found  3 -   339 - 0x7fcb01aec680
Found  4 -   483 - 0x7fcb01b7c680
Found  5 -   563 - 0x7fcb01bcc680
Found  6 -   675 - 0x7fcb01c3c680
Found  7 -   755 - 0x7fcb01c8c680
Found  8 -   899 - 0x7fcb01d1c680
Found  9 -  1043 - 0x7fcb01dac680
Found 10 -  1219 - 0x7fcb01e5c680
Found 11 -  1363 - 0x7fcb01eec680
Found 12 -  1507 - 0x7fcb01f7c680
Found 13 -  1587 - 0x7fcb01fcc680
Found 14 -  1667 - 0x7fcb0201c680
Found 15 -  1811 - 0x7fcb020ac680
Found 16 -  1955 - 0x7fcb0213c680
Eviction set constructed in 1989 ms
Evset correctness: 16/16
```

Similar for a more coarse-grained timer example:
- Timing source of 100 ms, i.e., `GRANULARITY_USEC` is 100000
- `EARLY_ABORT` and `EVICTION_LOOP` both **enabled**


```
Core  2 for Attacker

Configuration: 16 LLC ways

	Granularity [ms]          100.0000 ms
	Granularity [cycles]   420000000 cycles
	Iteration threshold         1000 
	Congruence confidence          3 in a row

Found  1 -     0 - 0x7f0592aa7e80
[...]
Found 16 -  1888 - 0x7f0593207e80
Eviction set constructed in 39451 ms
Evset correctness: 16/16
```

If you don't get any output at all beyond the configuration parameters, enable `PRINT_TRAVERSE_COUNTERS` for useful debugging information.


### Disclaimer
This PoC has been developed on an Intel Core i7-7700K (Kaby Lake, 16-way LLC). We expect it to run on other Intel processors too, but specific parameters of the PoCs may be sensitive to your specific CPU.

In particular:
- Check whether the timing thresholds for L1/LLC/RAM make sense on your platform
- Check the correct LLC associativity is configured in `../utils/configuration.h`
- For the LLC amplifier, check whether `prefetchNTA` has the expected behavior on your platform (for more details, see the [Leaky Way](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=9923849) paper by Guo et al.)