#pragma once
#include "SyncOutput.h"
#include <boost/asio.hpp>
#include <mutex>
#include <string>
#include <thread>

using boost::asio::io_service;
using boost::asio::streambuf;
using boost::system::error_code;
using std::mutex;
using std::string;

namespace PrimeNumbers
{
    //Function pointer type that points to the callback
    //function which is called on a request completion
    typedef void( *Callback ) (SyncOutput* syncOutput, unsigned int requestId, const string& response, const error_code& ec);

    //Structure represents a context of a single request
    struct ClientSession
    {
        ClientSession( io_service& ioService, const string& rawIPAddress, uint16_t port, const string& request, unsigned int requestId, Callback callback );
        boost::asio::ip::tcp::socket socket_;
        boost::asio::ip::tcp::endpoint endPoint;
        string request;
        streambuf responseBuffer;
        string response;

        error_code ec;
        // Unique ID assigned to the request
        unsigned int requestId;
        Callback callback;
        
        bool wasCancelled;
        mutex cancelGuard;
    };
}