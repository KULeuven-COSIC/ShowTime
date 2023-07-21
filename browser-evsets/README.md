# LLC eviction sets in the browser

This PoC demonstrates the construction of LLC eviction sets in an unmodified Google Chrome browser, in a restricted isolation setting (with a `performance.now()` precision of 100 us).
This experiment has been tested on two machines, running Chrome 107 and 108 on an Intel Core i7-8665U and an Intel Core i5-8250U.
It might not necessarily work out of the box on other systems.
We consider it very likely that the execution time and accuracy of the PoC can be improved.

## Prerequisites
- Google Chrome
- Python 3 for running the web server
- `gcc` for compiling the eviction set verification script

## Running the example

1. Compile the eviction set verification script: `gcc virt_to_phys.c -o virt_to_phys`
    - This script has been modified from [Vila's evsets repository](https://github.com/cgvwzq/evsets)
2. Run the server: `python3 -m http.server` in this directory
3. Launch Chrome with the verification script: `./run.sh`
4. Before clicking on `Start`, adjust the `associativity` setting in the form according to your CPU. The other settings should not require any changes.
5. For additional debug information while running the eviction set construction, open the developer console (`F12`).
