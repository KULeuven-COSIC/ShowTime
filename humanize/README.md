# Humanize: Timing attacks with human eyes
This folder contains the code used for live audience demos (e.g., [RuhrSec 2023](https://youtu.be/CP1bcche7oA?t=1140)), and is also the framework that was used for our human study. 
The PoC implements the LLC prefetch amplification of a cache line that is either present or not present in the LLC.

This PoC was developed on an Intel Core i7-7700K (Kaby Lake, 16-way LLC), on which it works very reliably.
We obtain similar results for the Intel Core i7-6700 (Skylake, 16-way LLC), another one of our development machines. Incidentally, these are also the machines for which the [Leaky Way paper](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=9923849) reports the results.

We note that the prefetchNTA amplifier also works on, e.g, the Core i5-7500 (Kaby Lake, 12-way LLC) or Core i5-6500 (Skylake, 12-way LLC), but in its current form it may not be robust enough for the human eye demonstration. We have not hashed out why it is less robust, we simply note that its reliability can be improved using a modified prefetch pattern.
For some earlier processor generations, e.g., i7-4790, we do not observe a signal at all.

We also want to point out that, even though prefetchNTA should not (?) be serialized by any of the fences, the presence of the correct fence in `prefetchRUN` is crucial.
The prefetch amplifier does not work without separating the prefetches with an `mfence`, nor does it work when the `mfence` is replaced by, e.g., `lfence`.
We leave an exploration of this behavior to future work.

## Configuration
Ensure that the global configuration parameters are [set correctly](../utils/README.md).

Experiment-specific parameters are configured in `settings.h`.

The main parameter of interest is `AMP_TRAVERSE`, which reflects the total number of repetitions of the traversal pattern.

## Execution
To execute the PoC:

1. Compile with `make`
2. Run with `make run`

### Expected output
Please refer to our [live audience demo](https://youtu.be/CP1bcche7oA?t=1140).