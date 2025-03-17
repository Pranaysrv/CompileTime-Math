// test_CNTCL.cpp - Test suite for the CNTCL library
#include "CNTCL.hpp"
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <future>

// Helper function for timing
template<typename F, typename... Args>
double measure_time(F&& func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    std::forward<F>(func)(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> duration = end - start;
    return duration.count();
}

// Test compile-time functions
void test_compile_time_functions() {
    std::cout << "Testing compile-time functions...\n";
    
    // These assertions are evaluated at compile time
    static_assert(CNTCL::gcd(56, 98) == 14, "GCD test failed");
    static_assert(CNTCL::lcm(12, 18) == 36, "LCM test failed");
    static_assert(CNTCL::modpow(4, 13, 497) == 445, "ModPow test failed");
    static_assert(CNTCL::is_prime(997), "Prime test failed for 997");
    static_assert(!CNTCL::is_prime(999), "Prime test failed for 999");
    
    // Test extended GCD
    static_assert(CNTCL::extended_gcd(120, 23).first * 120 + 
                 CNTCL::extended_gcd(120, 23).second * 23 == 1, 
                 "Extended GCD test failed");
    
    // Test mod inverse
    static_assert(CNTCL::mod_inverse(3, 11) * 3 % 11 == 1, "Mod inverse test failed");
    
    std::cout << "All compile-time tests passed!\n";
}

// Test runtime functions
void test_runtime_functions() {
    std::cout << "Testing runtime functions...\n";
    
    // Test prime factorization
    auto factors = CNTCL::prime_factors(840ULL);  // Use ULL suffix to ensure uint64_t type
    std::vector<uint64_t> expected_factors = {2, 2, 2, 3, 5, 7};
    assert(factors == expected_factors);
    
    // Test SIMD sieve
    auto primes = CNTCL::simd_sieve(30);
    std::vector<uint32_t> expected_primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    
    // Debug output to see what's happening
    std::cout << "Expected primes: ";
    for (auto p : expected_primes) std::cout << p << " ";
    std::cout << "\nActual primes: ";
    for (auto p : primes) std::cout << p << " ";
    std::cout << std::endl;
    
    assert(primes == expected_primes);
    
    std::cout << "All runtime tests passed!\n";
}

// Test coroutines
void test_coroutines() {
    std::cout << "Testing coroutines...\n";
    
    // Test Fibonacci generator
    auto fib = CNTCL::fibonacci_sequence(10);
    std::vector<uint64_t> expected_fibs = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    std::vector<uint64_t> actual_fibs;
    
    for (int i = 0; i < 10 && !fib.done(); i++) {
        actual_fibs.push_back(fib.next());
    }
    
    assert(actual_fibs == expected_fibs);// Test prime generator
    auto prime_gen = CNTCL::generate_primes(10);
    std::vector<uint64_t> expected_primes_gen = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    std::vector<uint64_t> actual_primes_gen;
    
    for (int i = 0; i < 10 && !prime_gen.done(); i++) {
        actual_primes_gen.push_back(prime_gen.next());
    }
    
    assert(actual_primes_gen == expected_primes_gen);
    
    std::cout << "All coroutine tests passed!\n";
}

// Test thread-local cache
void test_thread_local_cache() {
    std::cout << "Testing thread-local cache...\n";
    
    // First call - uncached
    auto time_uncached = measure_time([](){ 
        for (int i = 0; i < 1000; i++) {
            CNTCL::PrimeChecker::is_prime_cached(1000003);
        }
    });
    
    // Second call - should be faster due to caching
    auto time_cached = measure_time([](){ 
        for (int i = 0; i < 1000; i++) {
            CNTCL::PrimeChecker::is_prime_cached(1000003);
        }
    });
    
    std::cout << "Uncached time: " << time_uncached << "ms\n";
    std::cout << "Cached time: " << time_cached << "ms\n";
    std::cout << "Speed improvement: " << time_uncached / time_cached << "x\n";
    
    assert(time_cached < time_uncached);
    std::cout << "Thread-local cache test passed!\n";
}

// Test concurrent prime counter
void test_concurrency() {
    std::cout << "Testing concurrent prime counter...\n";
    
    CNTCL::ConcurrentPrimeCounter counter;
    
    // Count primes up to 100,000 using multiple threads
    auto time_multi = measure_time([&counter](){
        counter.count_primes(1, 100000);
    });
    
    // Get the expected count (verified externally)
    uint64_t expected_count = 9592; // Number of primes <= 100,000
    
    // Single-threaded count for comparison
    uint64_t single_count = 0;
    auto time_single = measure_time([&single_count](){
        for (uint64_t i = 1; i <= 100000; i++) {
            if (CNTCL::is_prime(i)) {
                single_count++;
            }
        }
    });
    
    uint64_t multi_count = counter.count_primes(1, 100000, 1); // Force single thread for comparison
    
    std::cout << "Single-threaded time: " << time_single << "ms\n";
    std::cout << "Multi-threaded time: " << time_multi << "ms\n";
    std::cout << "Speed improvement: " << time_single / time_multi << "x\n";
    
    assert(single_count == expected_count);
    assert(multi_count == expected_count);
    
    std::cout << "Concurrency test passed!\n";
}

// Stress test
void stress_test() {
    std::cout << "Running stress tests...\n";
    
    // Test with large numbers
    auto large_factors = CNTCL::prime_factors(1234567890);
    std::cout << "Prime factors of 1234567890: ";
    for (auto factor : large_factors) {
        std::cout << factor << " ";
    }
    std::cout << "\n";
    
    // Test SIMD sieve with larger range
    std::cout << "Generating primes up to 1,000,000 using SIMD...\n";
    auto time_simd = measure_time([](){
        auto primes = CNTCL::simd_sieve(1000000);
        std::cout << "Found " << primes.size() << " primes\n";
    });
    std::cout << "SIMD sieve time: " << time_simd << "ms\n";
    
    // Test concurrent prime counting with larger range
    std::cout << "Counting primes up to 1,000,000 concurrently...\n";
    CNTCL::ConcurrentPrimeCounter counter;
    auto time_concurrent = measure_time([&counter](){
        uint64_t count = counter.count_primes(1, 1000000);
        std::cout << "Found " << count << " primes\n";
    });
    std::cout << "Concurrent prime counting time: " << time_concurrent << "ms\n";
    
    std::cout << "Stress tests completed!\n";
}

// Main test function
int main() {
    std::cout << "=== CNTCL Library Test Suite ===\n\n";
    
    test_compile_time_functions();
    std::cout << "\n";
    
    test_runtime_functions();
    std::cout << "\n";
    
    test_coroutines();
    std::cout << "\n";
    
    test_thread_local_cache();
    std::cout << "\n";
    
    test_concurrency();
    std::cout << "\n";
    
    stress_test();
    std::cout << "\n";
    
    std::cout << "All tests completed successfully!\n";
    return 0;
}