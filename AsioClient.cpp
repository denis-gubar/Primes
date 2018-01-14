#include "AsioClient.h"

namespace PrimeNumbers
{
    AsioClient::AsioClient( const string& outputFileName ): pendingRequestCV(), activeRequests(0)
    {
        syncOutput.reset( new SyncOutput( outputFileName ) );
        work.reset( new io_service::work( ioService ) );
        workingThread.reset( new thread( [this]() { ioService.run(); } ) );
    }
    AsioClient::~AsioClient()
    {

    }
    void AsioClient::sendShutDown( const string& rawIPAddress, uint16_t port, Callback callback, unsigned int requestId )
    {
        string request = "SHUTDOWN\n";
        shared_ptr<ClientSession> session = shared_ptr<ClientSession>( new ClientSession( ioService, rawIPAddress, port, request, requestId, callback ) );
        session->socket_.open( session->endPoint.protocol() );

        //Add new session to the list of active sessions so that we can access if the user decides to cancel
        //the corresponding request before completion. To access active session list from different threads
        //we guard it with the next mutex
        unique_lock<mutex> lock( activeSessionsGuard );
        while (activeRequests > 0)
            pendingRequestCV.wait( lock );
        activeSessions[requestId] = session;
        lock.unlock();

        session->socket_.async_connect( session->endPoint,
                                        [this, session]( const error_code& ec )
                                        {
                                            if (ec != 0)
                                            {
                                                session->ec = ec;
                                                return;
                                            }

                                            async_write( session->socket_, buffer( session->request ),
                                                         [this, session]( const error_code& ec, size_t bytesTransferred )
                                                         {
                                                             if (ec != 0)
                                                             {
                                                                 session->ec = ec;
                                                                 return;
                                                             }
                                                         } );
                                        } );
    }
    void AsioClient::sendPrimeRequest( Integral low, Integral high, const string& rawIPAddress, uint16_t port, Callback callback, unsigned int requestId )
    {
        string request = "GETPRIMES " + to_string( low ) + " " + to_string( high ) + "\n";
        shared_ptr<ClientSession> session = shared_ptr<ClientSession>( new ClientSession( ioService, rawIPAddress, port, request, requestId, callback ) );
        session->socket_.open( session->endPoint.protocol() );

        //Add new session to the list of active sessions so that we can access if the user decides to cancel
        //the corresponding request before completion. To access active session list from different threads
        //we guard it with the next mutex
        unique_lock<mutex> lock( activeSessionsGuard );
        activeSessions[requestId] = session;
        ++activeRequests;
        lock.unlock();

        session->socket_.async_connect( session->endPoint,
                                        [this, session]( const error_code& ec )
                                        {
                                            if (ec != 0)
                                            {
                                                session->ec = ec;
                                                onRequestComplete( session );
                                                return;
                                            }
                                            //Synchronize cancel using cancelGuard mutex
                                            unique_lock<mutex> cancelLock( session->cancelGuard );

                                            if (session->wasCancelled)
                                            {
                                                onRequestComplete( session );
                                                return;
                                            }

                                            async_write( session->socket_, buffer( session->request ),
                                                         [this, session]( const error_code& ec, size_t bytesTransferred )
                                                         {
                                                             if (ec != 0)
                                                             {
                                                                 session->ec = ec;
                                                                 onRequestComplete( session );
                                                                 return;
                                                             }
                                                             //Synchronize cancel using cancelGuard mutex
                                                             unique_lock<mutex> cancelLock( session->cancelGuard );

                                                             if (session->wasCancelled)
                                                             {
                                                                 onRequestComplete( session );
                                                                 return;
                                                             }

                                                             async_read_until( session->socket_, session->responseBuffer, '\n',
                                                                               [this, session]( const error_code& ec, size_t bytesTransferred )
                                                                               {
                                                                                   if (ec != 0)
                                                                                   {
                                                                                       session->ec = ec;
                                                                                       return;
                                                                                   }
                                                                                   istream stream( &session->responseBuffer );
                                                                                   getline( stream, session->response );

                                                                                   onRequestComplete( session );
                                                                               }
                                                             );
                                                         } );
                                        } );
    }
    void AsioClient::cancelRequest( unsigned int requestId )
    {
        //To access active session list from different threads
        //we guard it with the next mutex
        unique_lock<mutex> lock( activeSessionsGuard );
        auto it = activeSessions.find( requestId );
        if (it != activeSessions.end())
        {
            //Synchronize cancel using cancelGuard mutex
            unique_lock<mutex> cancelLock( it->second->cancelGuard );

            it->second->wasCancelled = true;
            it->second->socket_.cancel();
        }
        lock.unlock();
    }
    void AsioClient::close()
    {
        syncOutput->close();
        //Destroy work object. This allows the I/O thread to exit the event loop
        //when there ane no more pendinge asynchronous operations
        work.reset( nullptr );

        //Wait for I/O thread to exit
        workingThread->join();
    }
    void AsioClient::onRequestComplete( shared_ptr<ClientSession> session )
    {
        //Shutting down the connection. This method may fail in case socket is not connected.
        //We ignore error code if this function fails
        error_code ignored_ec;

        session->socket_.shutdown( boost::asio::ip::tcp::socket::shutdown_both, ignored_ec );

        //To access active session list from different threads
        //we guard it with the next mutex
        unique_lock<mutex> lock( activeSessionsGuard );

        auto it = activeSessions.find( session->requestId );
        if (it != activeSessions.end())
            activeSessions.erase( it );
        lock.unlock();

        error_code ec;
        if (session->ec == 0 && session->wasCancelled)
            ec = boost::asio::error::operation_aborted;
        else
            ec = session->ec;

        //Call the user callback
        session->callback( syncOutput.get(), session->requestId, session->response, ec );

        //Callback is finished, possible pending SHUTDOWN can proceed
        lock.lock();
        --activeRequests;
        pendingRequestCV.notify_all();
        lock.unlock();
    }
}