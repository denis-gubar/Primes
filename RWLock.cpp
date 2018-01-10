#include "RWLock.h"

namespace PrimeNumbers
{
    RWLock::RWLock() : shared(), readerCV(), writerCV(), activeReaders(0), activeWriters(0), waitingWriters(0)
    {
    }
    void RWLock::readLock()
    {
        unique_lock<mutex> lock( shared );
        while (waitingWriters > 0)
            readerCV.wait( lock );
        ++activeReaders;
        lock.unlock();
    }
    void RWLock::readUnlock()
    {
        unique_lock<mutex> lock( shared );
        --activeReaders;
        lock.unlock();
        writerCV.notify_one();
    }
    void RWLock::writeLock()
    {
        unique_lock<mutex> lock( shared );
        ++waitingWriters;
        while (activeReaders > 0 || activeWriters > 0)
            writerCV.wait( lock );
        ++activeWriters;
        lock.unlock();
    }
    void RWLock::writeUnlock()
    {
        unique_lock<mutex> lock( shared );
        --waitingWriters;
        --activeWriters;
        if (waitingWriters > 0)
            writerCV.notify_one();
        else
            readerCV.notify_all();
        lock.unlock();
    }
}