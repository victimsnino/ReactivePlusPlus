## Benchmark for observer:

Observer has non-trivial way to store functions for callbacks. It used to increase perfomance and reduce amount of memory.

Benchmark results:

Approach           | Full construction | Subscribe only
-------------------|-------------------|---------------
std::function      | 248ns             | 102ns
Storage with casts | 141ns             | 20ns