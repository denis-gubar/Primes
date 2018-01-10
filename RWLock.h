#pragma once
#include <condition_variable>
#include <mutex>

using std::condition_variable;
using std::mutex;
using std::unique_lock;

namespace PrimeNumbers
{
    class RWLock
    {
    public:
        RWLock();
        void readLock();
        void readUnlock();
        void writeLock();
        void writeUnlock();
    private:
        mutex shared;
        condition_variable readerCV;
        condition_variable writerCV;
        int activeReaders;
        int activeWriters;
        int waitingWriters;
    };
}