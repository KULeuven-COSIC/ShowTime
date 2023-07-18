# Architectural Reordering

This PoC encodes the presence of a cache line into the architectural state by using two parallel threads.
More details about its working principles can be found in Section 5.3 of the paper.

## Configuration

For this PoC, very few parameters need to be configured - they need to be set in `settings.h`.
The main file is `timer_free.c`, the helper thread is `attacker_helper.c`.

## Execution

To execute the PoC:

1. Compile with `make`
2. Run with `make run`

### Expected Output
```
Core  2 for Attacker
Core  0 for Attacker Helper

======================
 Timer-free timer PoC
======================

Cached, should see lots of 0xFFs : 	10000 / 10000 (100.000000%)
Not cached, should see few 0xFFs : 	    1 / 10000 (0.010000%)
```

### Running multiple experiments
Optional: the script `batch.sh` can be used to conduct multiple runs of the experiment.
If the output of such a batched execution is saved in a file via `./batch.sh > filename.txt`, the command `./analyze.py filename.txt` can be used to aggregate the results.