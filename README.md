# The Sieve of Erastosthenes 

## Algorithm
The Sieve of Eratosthenes is a simple algorithm to find the prime numbers up to a given number n.

Consider the following implementations:
1. sequential, on a single CPU-core;
1. parallel, on a shared memory system, using OpenMP;
1. parallel, on a distributed memory system using only MPI and MPI with the shared memory version.

The following steps describe the algorithm:
1. Create list of unmarked natural numbers 2, 3, …, n
2. k ← 2
3. Repeat
    1. Mark all multiples of k between 2k and n
    1. k ← smallest unmarked number > k
    1. // until k^2 > n
4. The unmarked numbers are primes.


The time complexity of the algorithm is O(n ln ln n).
Data range to consider (n): from 2^25 to 2^32.


## Building 

1. `cd src`
1. `make` will compile all versions (1: sequential, 2: OMP, 3: MPI)

## Running

### Sequential version

`./SoE_seq <max_number> <print=0>`

### OMP version

`./SoE_omp <max_number> <print=0>`


## Authors
* Daniel Silva
* Fábio Gaspar
