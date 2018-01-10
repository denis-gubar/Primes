#pragma once
#include "configuration.h"
#include <boost/asio.hpp>
#include <fstream>
#include <string>
#include <thread>

using boost::asio::io_service;
using std::move;
using std::ofstream;
using std::string;
using std::thread;
using std::unique_ptr;

namespace PrimeNumbers
{
    class SyncOutput
    {
    public:
        SyncOutput( const string& filename );
        ~SyncOutput();
        void post( const string& text );
        void post( Integral number );
        void close();
    private:
        void write( const string& text );
        void write( Integral number );
        void writeHeader();
        void writeFooter();
        ofstream output;
        io_service ioService;
        io_service::strand strand_;
        unique_ptr<io_service::work> work;
        unique_ptr<thread> workingThread;
    };
}