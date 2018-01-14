#pragma once
#include "ClientSession.h"
#include "configuration.h"
#include <boost/asio.hpp>
#include <iostream>
#include <map>

using boost::asio::async_read_until;
using boost::asio::async_write;
using boost::asio::buffer;
using boost::asio::io_service;
using std::condition_variable;
using std::getline;
using std::istream;
using std::map;
using std::shared_ptr;
using std::size_t;
using std::string;
using std::thread;
using std::to_string;
using std::unique_lock;
using std::unique_ptr;

namespace PrimeNumbers
{
    class AsioClient
    {
    public:
        AsioClient( const string& outputFileName );
        ~AsioClient();
        void sendShutDown( const string& rawIPAddress, uint16_t port, Callback callback, unsigned int requestId );
        void sendPrimeRequest( Integral low, Integral high, const string& rawIPAddress, uint16_t port, Callback callback, unsigned int requestId );
        void cancelRequest( unsigned int requestId );
        void close();
    private:
        io_service ioService;
        map<int, shared_ptr<ClientSession>> activeSessions;
        mutex activeSessionsGuard;
        unique_ptr<io_service::work> work;
        unique_ptr<thread> workingThread;
        unique_ptr<SyncOutput> syncOutput;
        condition_variable pendingRequestCV;
        int activeRequests;
        void onRequestComplete( shared_ptr<ClientSession> session );
    public: //For better error message about deleted special methods
        AsioClient( const AsioClient& ) = delete;
        void operator=( const AsioClient& ) = delete;
    };
}