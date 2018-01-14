#pragma once
#include "AsioHandler.h"

namespace PrimeNumbers
{
    AsioHandler::AsioHandler( io_service& service ) : ioService( service ), socket_( service ), writeStrand( service )
    {
    }
    boost::asio::ip::tcp::socket& AsioHandler::socket()
    {
        return socket_;
    }
    void AsioHandler::start()
    {
        readPacket();
    }
    void AsioHandler::disconnect()
    {
        //Initiate graceful connection closure
        error_code ignored_ec;
        socket_.shutdown( boost::asio::ip::tcp::socket::shutdown_both, ignored_ec );
    }
    void AsioHandler::send( string msg )
    {
        ioService.post( writeStrand.wrap( [me = shared_from_this(), msg]() -> void
        {
            me->queueMessage( msg );
        } ) );
    }
    void AsioHandler::readPacket()
    {
        async_read_until( socket_, inPacket, '\n',
                          [me = shared_from_this()]( const error_code& ec, size_t bytesTransfer )
        {
            me->readPacketDone( ec, bytesTransfer );
        } );
    }
    void AsioHandler::calculatePrimes( Integral low, Integral high )
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
    void AsioHandler::readPacketDone( const error_code& ec, size_t bytesTransferred )
    {
        if (ec)
            return;
        istream stream( &inPacket );
        string packetString;
        std::getline( stream, packetString );
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
            istringstream ss( S[1] + " " + S[2] );
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

        //Continue serving other commands
        readPacket();
    }
    void AsioHandler::queueMessage( string message )
    {
        sendPacketQueue.push_back( move( message ) );
        if (!sendPacketQueue.empty())
            startPacketSend();
    }
    void AsioHandler::startPacketSend()
    {
        sendPacketQueue.front() += '\n';
        async_write( socket_, buffer( sendPacketQueue.front() ), writeStrand.wrap(
            [me = shared_from_this()]( const error_code& ec, size_t )
        {
            me->packetSendDone( ec );
        }
        ) );
    }
    void AsioHandler::packetSendDone( const error_code& ec )
    {
        if (ec)
            return;
        sendPacketQueue.pop_front();
        if (!sendPacketQueue.empty())
            startPacketSend();
    }
}