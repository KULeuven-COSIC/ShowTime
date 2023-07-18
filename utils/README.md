# Description

This folder (i.e., `utils`) contains basic utils for cache measurements, mapping memory, and miscellaneous functions.

## Main configuration file
This folder contains `configuration.h`, which is the configuration file used in most of the experiments.
Starting with this file is essential. You must input the cache information of the target machine correctly, and set the execution parameters.
If your platform is one of the listed platforms, you can (probably) just set it and ignore the details. In particular, this will ensure that the LLC associativity is set correctly.

To help users, the `configuration.h` file is commented thoroughly. Please pay special attention to the following parameters:

1. `{ATTACKER,HELPER,VICTIM}_CORE`

    Core affinities of the threads. In case of a hyperthreaded machine, logical siblings can be learned, e.g., through `cat /`sys/devices/system/cpu/cpu0/topology/thread_siblings_list`

2. `HUGE_PAGES_AVAILABLE`

    If hugepages are available on the machine, indicate it. All PoCs in this repository should work with small pages too (hence, this is the default setting), but enabling huge pages may increase the robustness on your system.