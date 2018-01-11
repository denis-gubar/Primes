#pragma once
#include "RWLock.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>
#include <type_traits>
#include <vector>

using std::enable_if;
using std::is_integral;
using std::lower_bound;
using std::max;
using std::size_t;
using std::thread;
using std::vector;

namespace PrimeNumbers
{
    template<typename Int,
        //Enforcing usage of integral types only
        typename = typename enable_if<is_integral<Int>::value, Int>::type> 
    class Sieve
    {
        public:
            static Sieve& getInstance()
            {
                static Sieve instance;
                return instance;
            }
            void clear()
            {
                isPrime.clear();
                primes.clear();
            }
            void buildSieve( Int n )
            {
                if (calculatedN >= n || n < Int(2))
                    return;
                if (n <= Int(10000) )
                {
                    basicSieve(n);
                    return;
                }
                lock.writeLock();
                clear();                
                Int sqrt_n = static_cast<Int>(sqrt( n ));
                isPrime = vector<char>( static_cast<size_t>(n + Int(1)), char(1) );
                primes = { Int(2) };
                //First sqrt(n) numbers are being sieved
                //Skip reading even numbers
                for (Int k = Int(3); k <= sqrt_n; k += Int(2))
                {
                    if (isPrime[static_cast<size_t>(k)] )
                        //Skip reading even numbers
                        for (Int x = Int(3) * k; x <= sqrt_n; x += Int(2) * k)
                            isPrime[static_cast<size_t>(x)] = char(0);
                }
                //Skip reading even numbers
                for (Int k = Int(3); k <= sqrt_n; k += Int(2))
                    if (isPrime[static_cast<size_t>(k)])
                        primes.push_back( k );
                //SKip reading even numbers
                Int begin = (sqrt_n + Int(1)) / Int(2) * Int(2) + Int(1), end = n;
                Int pageSize = (end - begin) / THREAD_COUNT, shift = (end - begin) % THREAD_COUNT;
                Int pageBegin = begin + shift;
                vector<thread> workerThreads;
                auto markCompoundNumbers = [&]( Int begin, Int end ) -> void
                                {
                                    //Skip prime number 2
                                    for (size_t i = 1; i < primes.size(); ++i)
                                    {
                                        //Skip reading even numbers
                                        //Processing only [begin; end] interval
                                        for (Int k = (begin + primes[i] - Int( 1 )) / (Int( 2 ) * primes[i]) * Int( 2 ) * primes[i] + primes[i];
                                              k <= end; k += Int( 2 ) * primes[i])
                                            isPrime[static_cast<size_t>(k)] = char( 0 );
                                    }
                                };
                workerThreads.emplace_back( markCompoundNumbers, begin, pageBegin + pageSize);
                for (int pageNumber = 1; pageNumber < THREAD_COUNT; ++pageNumber)
                {
                    //this_thread::sleep_for( 0.1s );
                    pageBegin += pageSize;
                    workerThreads.emplace_back( markCompoundNumbers, pageBegin + Int(1), pageBegin + pageSize );
                }
                for (auto& workerThread : workerThreads)
                    if ( workerThread.joinable() )
                        workerThread.join();
                //Skip reading even numbers
                for (Int k = (sqrt_n + Int(1)) / Int(2) * Int(2) + Int(1) ; k <= n; k += Int(2))
                    if (isPrime[static_cast<size_t>(k)])
                        primes.push_back( k );
                calculatedN = n;
                lock.writeUnlock();
            }
            void readLock()
            {
                lock.readLock();
            }
            void readUnlock()
            {
                lock.readUnlock();
            }
            auto findCalculatedPrime(Int n)
            {
                return lower_bound( primes.cbegin(), primes.cend(), n );
            }
            Int largestPrime()
            {
                return primes.back();
            }
        private:
            Sieve(): calculatedN(Int(0)), THREAD_COUNT( max( thread::hardware_concurrency(), size_t(2) ) )
            {
            }
            vector<char> isPrime{};
            vector<Int> primes{};
            Int calculatedN;
            int THREAD_COUNT;
            RWLock lock;
            void basicSieve( Int n )
            {
                lock.writeLock();
                clear();
                Int sqrt_n = static_cast<Int>(sqrt( n ));
                isPrime = vector<char>( static_cast<size_t>(n + 1), static_cast<char>(1) );
                //Skip reading even numbers
                for (Int k = Int(3); k <= sqrt_n; k += Int(2))
                {
                    if (isPrime[static_cast<size_t>(k)])
                    {
                        for (Int x = Int(3) * k; x <= n; x += Int(2) * k)
                            isPrime[static_cast<size_t>(x)] = char(0);
                    }
                }
                primes.push_back( Int(2) );
                //Skip reading even numbers
                for (Int i = Int(3); i <= n; i += Int(2))
                    if (isPrime[static_cast<size_t>(i)])
                        primes.push_back( i );
                calculatedN = n;
                lock.writeUnlock();
            }
        public: //For better error message about deleted special methods
            Sieve( const Sieve& ) = delete;
            void operator=( const Sieve& ) = delete;
    };
}