# The Sieve of Erastosthenes 


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

## Authors
* Daniel Silva
* Fábio Gaspar
