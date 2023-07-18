# ShowTime: CPU Timing Attacks with the Human Eye
This repository is the open-source component of our ShowTime [paper](https://antoonpurnal.github.io/files/pdf/ShowTime.pdf) (AsiaCCS '23).

In particular, it comprises the following artifacts:
- [`basic-primitives`](./basic-primitives/): basic amplifiers and conversions
- [`humanize`](./humanize): distinguishing cache hits and misses with the human eye
- [`slow-evsets`](./slow-evsets): LLC eviction set construction with arbitrarily coarse-grained timers (including, e.g., the UNIX epoch)
- [`browser-evsets`](./browser-evsets): LLC eviction sets in Chrome
- [`architectural-reordering`](./architectural-reordering): measure a timing difference without using a timer
- [`utils`](./utils) and [`evsets`](./evsets): helper code for, respectively, general utilities and eviction set construction

Configuration, build and execution instructions are outlined in the relevant folders.

## Attribution
If you found the code in this repository useful, please cite our paper:

```
@inproceedings{Purnal2023showtime,
  author    = {Purnal, Antoon and Bognar, Marton and Piessens, Frank and Verbauwhede, Ingrid},
  title     = {ShowTime: Amplifying Arbitrary CPU Timing Side Channels},
  booktitle = {ACM SIGSAC Asia Conference on Computer and Communications Security (AsiaCCS)},
  year      = {2023},
}
```