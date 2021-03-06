#include "AsioClient.h"
#include "AsioHandler.h"
#include "AsioServer.cpp"
#include "InputFileData.cpp"
#include "Sieve.cpp"
#include "SyncOutput.h"
#include "configuration.h"
#include <chrono>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::round;
using std::cout;
using std::endl;
using std::string;
using std::this_thread::sleep_for;

using namespace std::chrono_literals;
using namespace PrimeNumbers;

const string inputFileName = "input.xml";
const string outputFileName = "output.xml";
const uint16_t SERVER_PORT = 3500;

int main()
{
    InputFileData<Integral> inputFileData = InputFileData<Integral>::loadFromXML( inputFileName );
    AsioServer<AsioHandler> server;
    thread serverThread( [&server]()
                            {
                                server.startServer( SERVER_PORT );
                            } );
    cout << "Server is started" << endl;
    sleep_for( 0.1s );
    auto responseHandler = [](SyncOutput* syncOutput, unsigned int requestId, const string& response, const error_code& ec ) -> void
    {
        if (ec == 0)
        {
            //cout << "Request #" << requestId << " has completed. Response:\n";
            //cout << response << "\n";
            size_t position = response.find( ' ' );
            if (position != string::npos)
            {
                syncOutput->post( string( response.cbegin() + position, response.cend()) );
            }                
        }
        else if (ec == boost::asio::error::operation_aborted)
        {
            cout << "Request #" << requestId << " has been cancelled by user\n";
        }
        else
        {
            cout << "Request #" << requestId << " has been failed!\n";
            cout << "Error code = " << ec.value() << ". Error message: " << ec.message() << "\n";
        }
    };
    AsioClient client(outputFileName);
    unsigned int requestId = 1000;
    for (const auto& interval : inputFileData.intervals)        
        client.sendPrimeRequest( interval.first, interval.second, "127.0.0.1", SERVER_PORT, responseHandler, ++requestId );
    //Internal pendingRequestCV condition_variable is used to wait until all request are completed
    client.sendShutDown( "127.0.0.1", SERVER_PORT, nullptr, ++requestId );
    client.close();
    cout << "Client is closed" << endl;
    serverThread.join();
    return 0;
}