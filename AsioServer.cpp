#include <boost/asio.hpp>
#include <thread>
#include <vector>

using boost::asio::io_service;
using boost::system::error_code;
using std::make_shared;
using std::shared_ptr;
using std::thread;
using std::vector;

namespace PrimeNumbers
{
    template <typename ConnectionHandler>
    class AsioServer
    {
        using SharedHandlerType = shared_ptr<ConnectionHandler>;
    public:
        AsioServer( int threadCount = 1 ) : threadCount( threadCount ), acceptor_( ioService ) {}
        void startServer( uint16_t port )
        {
            auto handler = make_shared<ConnectionHandler>( ioService );

            //Set up acceptor to listen on the TCP port
            boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::tcp::v4(), port );
            acceptor_.open( endpoint.protocol() );
            acceptor_.bind( endpoint );
            acceptor_.set_option( boost::asio::ip::tcp::acceptor::reuse_address( true ) );
            acceptor_.listen();

            acceptor_.async_accept( handler->socket(),
                                    [=]( auto ec )
                                    {
                                        handleNewConnection( handler, ec );
                                    } );

            //Start pool of threads to process the Asio events
            for (int i = 0; i < threadCount; ++i)
                threadPool.emplace_back( [=] {ioService.run();} );

            //Unreachable until server shutdown (to be implemented)
            for (int i = 0; i < threadCount; ++i)
                threadPool[i].join();
        }
    private:
        void handleNewConnection( SharedHandlerType handler, const error_code& ec )
        {
            if (ec)
                return;

            handler->start();
            auto newHandler = make_shared<ConnectionHandler>( ioService );
            acceptor_.async_accept( newHandler->socket(),
                                    [=]( auto ec )
                                    {
                                        handleNewConnection( newHandler, ec );
                                    } );
        }
        int threadCount;
        vector<thread> threadPool;
        io_service ioService;
        boost::asio::ip::tcp::acceptor acceptor_;
    };
}