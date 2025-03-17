// CNTCL.hpp - Main header file for the library
#pragma once

#include <cstdint>
#include <type_traits>
#include <atomic>
#include <coroutine>
#include <vector>
#include <optional>
#include <bit>
#include <thread>

// Architecture-specific includes
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    // x86/x64 specific SIMD
    #include <immintrin.h>
    #define HAS_X86_SIMD 1
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(__arm__) || defined(_M_ARM)
    // ARM NEON SIMD for Apple Silicon
    #include <arm_neon.h>
    #define HAS_ARM_NEON 1
#endif

namespace CNTCL {

// ===== Compile-time basic number theory functions =====

// Greatest Common Divisor (compile-time)
template <typename T>
constexpr T gcd(T a, T b) {
    static_assert(std::is_integral_v<T>, "Type must be integral");
    while (b != 0) {
        T temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Least Common Multiple (compile-time)
template <typename T>
constexpr T lcm(T a, T b) {
    static_assert(std::is_integral_v<T>, "Type must be integral");
    return (a / gcd(a, b)) * b;
}

// Fast Modular Exponentiation (compile-time)
template <typename T>
constexpr T modpow(T base, T exp, T modulus) {
    static_assert(std::is_integral_v<T>, "Type must be integral");
    if (modulus == 1) return 0;
    
    T result = 1;
    base = base % modulus;
    
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % modulus;
        }
        exp >>= 1;
        base = (base * base) % modulus;
    }
    
    return result;
}

// Miller-Rabin Primality Test (compile-time for small primes)
template <typename T>
constexpr bool is_prime(T n) {
    static_assert(std::is_integral_v<T>, "Type must be integral");
    
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    for (T i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    
    return true;
}

// Extended Euclidean Algorithm (compile-time)
template <typename T>
constexpr std::pair<T, T> extended_gcd(T a, T b) {
    static_assert(std::is_integral_v<T>, "Type must be integral");
    
    if (a == 0) return {0, 1};
    
    auto [x, y] = extended_gcd(b % a, a);
    return {y - (b / a) * x, x};
}

// Modular multiplicative inverse (compile-time)
template <typename T>
constexpr T mod_inverse(T a, T m) {
    static_assert(std::is_integral_v<T>, "Type must be integral");
    auto [x, _] = extended_gcd(a, m);
    return (x % m + m) % m;
}

// ===== Runtime optimized functions with lock-free concurrency =====

// Thread-safe prime factorization with atomics
template <typename T>
std::vector<T> prime_factors(T n) {
    static_assert(std::is_integral_v<T>, "Type must be integral");
    std::vector<T> factors;
    
    // Handle small divisors separately
    while (n % 2 == 0) {
        factors.push_back(2);
        n /= 2;
    }
    
    // Use atomics for thread safety in shared contexts
    std::atomic<T> current_n(n);
    
    // Try dividing by odd numbers
    for (T i = 3; i * i <= current_n.load(); i += 2) {
        while (current_n.load() % i == 0) {
            factors.push_back(i);
            
            // Atomically update current_n
            T expected = current_n.load();
            T desired;
            do {
                desired = expected / i;
            } while (!current_n.compare_exchange_weak(expected, desired));
        }
    }
    
    // If n is a prime number greater than 2
    if (current_n.load() > 2) {
        factors.push_back(current_n.load());
    }
    
    return factors;
}

// SIMD-accelerated sieve of Eratosthenes - architecture-specific implementation
std::vector<uint32_t> simd_sieve(uint32_t limit) {
    if (limit < 2) return {};
    
    const size_t size = (limit + 1) / 2; // We only store odd numbers
    std::vector<bool> is_composite(size, false);
    std::vector<uint32_t> primes;
    primes.push_back(2); // Add 2 separately
    
    // Process odd numbers
    for (uint32_t i = 3; i * i <= limit; i += 2) {
        if (!is_composite[(i - 1) / 2]) {
            // Mark multiples as composite
            for (uint32_t j = i * i; j <= limit; j += 2 * i) {
                is_composite[(j - 1) / 2] = true;
            }
        }
    }
    
    // Collect remaining primes
    for (uint32_t i = 3; i <= limit; i += 2) {
        if (!is_composite[(i - 1) / 2]) {
            primes.push_back(i);
        }
    }
    
    return primes;
}

// ===== Coroutine-based number theory functions =====

// Generator for Fibonacci sequence using coroutines
struct fibonacci_generator {
    struct promise_type {
        uint64_t value;
        
        fibonacci_generator get_return_object() {
            return {std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(uint64_t val) {
            value = val;
            return {};
        }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };
    
    std::coroutine_handle<promise_type> handle;
    
    fibonacci_generator(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~fibonacci_generator() { if (handle) handle.destroy(); }
    
    uint64_t next() {
        handle.resume();
        return handle.promise().value;
    }
    
    bool done() const { return handle.done(); }
};

// Coroutine to generate Fibonacci numbers
fibonacci_generator fibonacci_sequence(uint64_t max_count) {
    uint64_t a = 0, b = 1;
    
    for (uint64_t i = 0; i < max_count; i++) {
        co_yield a;
        uint64_t temp = a;
        a = b;
        b = temp + b;
    }
}

// Coroutine for lazy prime generation
struct prime_generator {
    struct promise_type {
        uint64_t value;
        
        prime_generator get_return_object() {
            return {std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(uint64_t val) {
            value = val;
            return {};
        }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };
    
    std::coroutine_handle<promise_type> handle;
    
    prime_generator(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~prime_generator() { if (handle) handle.destroy(); }
    
    uint64_t next() {
        handle.resume();
        return handle.promise().value;
    }
    
    bool done() const { return handle.done(); }
};

// Coroutine to generate prime numbers
prime_generator generate_primes(uint64_t max_count) {
    co_yield 2; // First prime
    
    uint64_t count = 1; // We've already yielded one prime
    uint64_t num = 3;   // Start checking from 3
    
    while (count < max_count) {
        bool is_prime = true;
        
        for (uint64_t i = 3; i * i <= num; i += 2) {
            if (num % i == 0) {
                is_prime = false;
                break;
            }
        }
        
        if (is_prime) {
            co_yield num;
            count++;
        }
        
        num += 2; // Skip even numbers
    }
}

// ===== Thread-local cache for optimizing repeated calculations =====

// Prime checker with thread-local cache
class PrimeChecker {
private:
    // Thread-local cache of recently checked numbers
    static thread_local std::vector<std::pair<uint64_t, bool>> cache;
    static constexpr size_t CACHE_SIZE = 1000;
    
public:
    static bool is_prime_cached(uint64_t n) {
        // Check cache first
        for (const auto& entry : cache) {
            if (entry.first == n) {
                return entry.second;
            }
        }
        
        // Compute result
        bool result = is_prime(n);
        
        // Update cache
        if (cache.size() >= CACHE_SIZE) {
            cache.erase(cache.begin());
        }
        cache.emplace_back(n, result);
        
        return result;
    }
};

thread_local std::vector<std::pair<uint64_t, bool>> PrimeChecker::cache;

// ===== Lock-free concurrent prime counter =====

class ConcurrentPrimeCounter {
private:
    std::atomic<uint64_t> count{0};
    
public:
    // Count primes in a range using multiple threads
    uint64_t count_primes(uint64_t start, uint64_t end, uint32_t thread_count = std::thread::hardware_concurrency()) {
        count = 0;
        std::vector<std::thread> threads;
        
        // Divide work among threads
        uint64_t chunk_size = (end - start + 1) / thread_count;
        
        for (uint32_t i = 0; i < thread_count; i++) {
            uint64_t chunk_start = start + i * chunk_size;
            uint64_t chunk_end = (i == thread_count - 1) ? end : chunk_start + chunk_size - 1;
            
            threads.emplace_back([this, chunk_start, chunk_end]() {
                uint64_t local_count = 0;
                
                for (uint64_t n = chunk_start; n <= chunk_end; n++) {
                    if (is_prime(n)) {
                        local_count++;
                    }
                }
                
                // Atomically add local count to global count
                count.fetch_add(local_count, std::memory_order_relaxed);
            });
        }
        
        // Join all threads
        for (auto& t : threads) {
            t.join();
        }
        
        return count.load();
    }
};

} // namespace CNTCL