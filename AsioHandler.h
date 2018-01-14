#pragma once
#include "Sieve.cpp"
#include "configuration.h"
#include <algorithm>
#include <boost/asio.hpp>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

using boost::asio::async_read_until;
using boost::asio::async_write;
using boost::asio::buffer;
using boost::asio::io_service;
using boost::asio::streambuf;
using boost::system::error_code;
using std::cout;
using std::deque;
using std::enable_shared_from_this;
using std::endl;
using std::getline;
using std::istream;
using std::istringstream;
using std::move;
using std::ostringstream;
using std::size_t;
using std::string;
using std::to_string;
using std::vector;

using namespace std::chrono_literals;

namespace PrimeNumbers
{
    class AsioHandler : public enable_shared_from_this<AsioHandler>
    {
    public:
        AsioHandler( io_service& service );
        boost::asio::ip::tcp::socket& socket();
        void start();
        void disconnect();
        void send( string msg );
    private:
        io_service & ioService;
        boost::asio::ip::tcp::socket socket_;
        io_service::strand writeStrand;
        streambuf inPacket;
        deque<string> sendPacketQueue;
        void readPacket();
        void calculatePrimes( Integral low, Integral high );
        void readPacketDone( const error_code& ec, size_t bytesTransferred );
        void queueMessage( string message );
        void startPacketSend();
        void packetSendDone( const error_code& ec );
    };
}