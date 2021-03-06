#include <iostream>
#include "gtest/gtest.h"
#include "../Sieve.cpp"
#include  <algorithm>

using namespace PrimeNumbers;
using std::cout;

TEST( PrimeTestCase, Sieve1 ) {
    Sieve<long long>& sieve = Sieve<long long>::getInstance();
    sieve.clear();
    sieve.buildSieve( 100 );
    EXPECT_EQ( 97, sieve.largestPrime() );
}

TEST( PrimeTestCase, Sieve2 ) {
    Sieve<long long>& sieve = Sieve<long long>::getInstance();
    sieve.clear();
    sieve.buildSieve( 100 );
    long long end = 97;
    auto it = sieve.findCalculatedPrime( 1 );
    long long checkSum = 0;
    while (*it <= end)
    {
        checkSum += *it;
        if (*it >= end)
            break;
        ++it;
    }
    EXPECT_EQ( 1060, checkSum );
}

TEST( PrimeTestCase, Sieve3 ) {
    Sieve<long long>& sieve = Sieve<long long>::getInstance();
    sieve.clear();
    sieve.buildSieve( 100 );
    long long end = 97;
    auto it = sieve.findCalculatedPrime( 1 );
    long long checkSum = 0;
    vector<long long> realResult;
    while (*it <= end)
    {
        realResult.push_back( *it );
        if (*it >= end)
            break;
        ++it;
    }
    vector<long long> expectedResult{ 2LL, 3LL, 5LL, 7LL, 11LL, 13LL, 17LL, 19LL, 23LL, 29LL, 31LL, 37LL, 41LL, 43LL, 47LL, 53LL, 59LL, 61LL, 67LL, 71LL, 73LL, 79LL, 83LL, 89LL, 97LL };
    EXPECT_EQ( expectedResult, realResult );
}

TEST( PrimeTestCase, Sieve4 ) {
    Sieve<long long>& sieve = Sieve<long long>::getInstance();
    sieve.clear();
    sieve.buildSieve( 100000 );
    EXPECT_EQ( 99991LL, sieve.largestPrime() );
}

TEST( PrimeTestCase, Sieve5 ) {
    Sieve<long long>& sieve = Sieve<long long>::getInstance();
    sieve.clear();
    sieve.buildSieve( 100000 );
    long long end = 99991LL;
    auto it = sieve.findCalculatedPrime( 1 );
    long long checkSum = 0;
    while (*it <= end)
    {
        checkSum += *it;
        if (*it >= end)
            break;
        ++it;
    }
    EXPECT_EQ( 454396537LL, checkSum );
}

TEST( PrimeTestCase, Sieve6 ) {
    Sieve<long long>& sieve = Sieve<long long>::getInstance();
    sieve.clear();
    sieve.buildSieve( 100000000 );
    auto it = sieve.findCalculatedPrime( 99999000LL );
    long long end = 99999200LL;
    long long checkSum = 0;
    vector<long long> realResult;
    while (*it <= end)
    {
        realResult.push_back( *it );
        if (*it >= end)
            break;
        ++it;
    }
    vector<long long> expectedResult{ 99999043LL, 99999073LL, 99999077LL, 99999079LL, 99999089LL, 99999103LL, 99999113LL, 99999131LL, 99999157LL, 99999167LL, 99999187LL };
    EXPECT_EQ( expectedResult, realResult );
}


int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    RUN_ALL_TESTS();
}
