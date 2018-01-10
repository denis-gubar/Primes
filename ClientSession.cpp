#include "ClientSession.h"

namespace PrimeNumbers
{
    ClientSession::ClientSession( io_service & ioService, const string & rawIPAddress, uint16_t port,
                                  const string & request, unsigned int requestId, Callback callback ) :
        socket_( ioService ), endPoint( boost::asio::ip::address::from_string( rawIPAddress ), port ), 
        request( request ), requestId( requestId ), callback( callback ), wasCancelled( false )
    {

    }
}