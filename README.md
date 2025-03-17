# CNTCL - Compile-time and Concurrent Number Theory Library

clear
## Features

- **Compile-time Number Theory**: GCD, LCM, modular exponentiation, primality testing, and more
- **Coroutine-based Generators**: Lazy evaluation of prime numbers and Fibonacci sequences
- **SIMD-accelerated Algorithms**: Fast prime sieve implementation with architecture-specific optimizations
- **Thread-safe Operations**: Lock-free concurrent prime counting and factorization
- **Thread-local Caching**: Optimized for repeated calculations

## Requirements

- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 19.28+)
- CMake 3.15+ (for building)
- Hardware with SIMD support (optional, falls back to scalar implementation)

## Installation

CNTCL is a header-only library. Simply include the main header file in your project:

```cpp
#include "CNTCL.hpp"


 ```
## Building and Testing
```bash
# Create build directory
mkdir -p build

# Compile tests
clang++ -std=c++20 -Wall -Wextra -O3 -fcoroutines-ts -I./include -o build/test_CNTCL tests/test_CNTCL.cpp -pthread

# Run tests
./build/test_CNTCL
 ```
```

## Usage Examples
### Compile-time Functions
```cpp
// These are evaluated at compile time
constexpr auto result1 = CNTCL::gcd(56, 98);        // 14
constexpr auto result2 = CNTCL::lcm(12, 18);        // 36
constexpr auto result3 = CNTCL::modpow(4, 13, 497); // 445
constexpr bool isPrime = CNTCL::is_prime(997);      // true
 ```

### Prime Factorization
```cpp
// Thread-safe prime factorization
auto factors = CNTCL::prime_factors(840ULL);
// Result: {2, 2, 2, 3, 5, 7}
 ```
### Coroutine-based Generators
```cpp
// Generate first 10 Fibonacci numbers
auto fib = CNTCL::fibonacci_sequence(10);
for (int i = 0; i < 10 && !fib.done(); i++) {
    std::cout << fib.next() << " ";
}
// Output: 0 1 1 2 3 5 8 13 21 34

// Generate first 10 prime numbers
auto primes = CNTCL::generate_primes(10);
for (int i = 0; i < 10 && !primes.done(); i++) {
    std::cout << primes.next() << " ";
}
// Output: 2 3 5 7 11 13 17 19 23 29
 ```

### SIMD-accelerated Prime Sieve
```cpp
// Find all primes up to 1,000,000
auto primes = CNTCL::simd_sieve(1000000);
std::cout << "Found " << primes.size() << " primes\n";
 ```

### Concurrent Prime Counting
```cpp
// Count primes in a range using multiple threads
CNTCL::ConcurrentPrimeCounter counter;
uint64_t count = counter.count_primes(1, 1000000);
std::cout << "Found " << count << " primes\n";
 ```

## Performance
CNTCL is designed for high performance:

- Compile-time evaluation eliminates runtime overhead for constant expressions
- SIMD acceleration provides up to 4x speedup on compatible hardware
- Thread-local caching improves performance for repeated calculations
- Lock-free concurrency scales efficiently with available CPU cores