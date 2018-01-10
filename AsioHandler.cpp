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
        AsioHandler( io_service& service ) : ioService( service ), socket_( service ), writeStrand( service )
        {
        }
        boost::asio::ip::tcp::socket& socket()
        {
            return socket_;
        }
        void start()
        {
            readPacket();
        }
        void disconnect()
        {
            //Initiate graceful connection closure
            error_code ignored_ec;
            socket_.shutdown( boost::asio::ip::tcp::socket::shutdown_both, ignored_ec );
        }
        void send(string msg)
        {
            ioService.post( writeStrand.wrap([me=shared_from_this(), msg]() -> void
                {
                    me->queueMessage( msg );
                } ) );
        }
    private:
        io_service& ioService;
        boost::asio::ip::tcp::socket socket_;
        io_service::strand writeStrand;
        streambuf inPacket;
        deque<string> sendPacketQueue;
        void readPacket()
        {
            async_read_until( socket_, inPacket, '\n',
                              [me=shared_from_this()](const error_code& ec, size_t bytesTransfer)
                            {
                                me->readPacketDone( ec, bytesTransfer );
                            });
        }
        void calculatePrimes( Integral low, Integral high )
        {
            Sieve<Integral>& sieve = Sieve<Integral>::getInstance();
            sieve.buildSieve( high );
            sieve.readLock();
            auto it = sieve.findCalculatedPrime( low );
            string message = "PRIMESRESULT";
            Integral end = std::min( high, sieve.largestPrime() );
            while (*it <= end)
            {
                message.append( " " ).append( to_string( *it ) );
                if (*it >= end)
                    break;
                ++it;
            }
            send( message );
            sieve.readUnlock();
        }
        void readPacketDone(const error_code& ec, size_t bytesTransferred)
        {
            if (ec)
                return;
            istream stream( &inPacket );
            string packetString;
            std::getline( stream, packetString );
            std::this_thread::sleep_for( 0.5s );
            auto split = []( const string& s, char delimeter = ' ' ) -> vector<string>
            {
                vector<string> result;
                istringstream ss( s );
                for (string token; !ss.eof(); )
                {
                    getline( ss, token, delimeter );
                    result.push_back( token );
                }
                return result;
            };
            vector<string> S = split( packetString, ' ' );
            if (!S.empty() && S[0] == "GETPRIMES")
            {
                cout << "Command: " << S[0] << " " << S[1] << " " << S[2] << "\n";
                istringstream ss( S[1] + " " + S[2]);
                Integral low, high;
                ss >> low >> high;
                calculatePrimes( low, high );
            }
            else if (!S.empty() && S[0] == "SHUTDOWN")
            {
                cout << "Command: " << S[0] << "\n";
                ioService.stop();
                return;
            }

            //handle packetString
            //TODO:

            readPacket();
        }
        void queueMessage( string message )
        {
            sendPacketQueue.push_back( move( message ) );
            if (!sendPacketQueue.empty())
                startPacketSend();
        }
        void startPacketSend()
        {
            sendPacketQueue.front() += '\n';
            async_write( socket_, buffer( sendPacketQueue.front() ), writeStrand.wrap(
                [me = shared_from_this()]( const error_code& ec, size_t )
                {
                    me->packetSendDone( ec );
                }
            ) );
        }
        void packetSendDone( const error_code& ec )
        {
            if (ec)
                return;
            sendPacketQueue.pop_front();
            if (!sendPacketQueue.empty())
                startPacketSend();            
        }
    };
}