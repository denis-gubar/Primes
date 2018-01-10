#include "SyncOutput.h"

namespace PrimeNumbers
{    
    SyncOutput::SyncOutput( const string& filename ) : output( ofstream( filename ) ), strand_(ioService)
    {
        work.reset( new io_service::work( ioService ) );
        workingThread.reset( new thread( [this]() { ioService.run(); } ) );
        writeHeader();
    }
    SyncOutput::~SyncOutput()
    {
        writeFooter();
        output.close();
    }
    void SyncOutput::write( const string& text )
    {
        output << text;
    }
    void SyncOutput::write( Integral number )
    {
        output << number << ' ';
    }
    void SyncOutput::post( const string& text )
    {
        strand_.post( [this, text]() {write( text );} );
    }
    void SyncOutput::post( Integral number )
    {
        strand_.post( [this, number]() {write( number );} );
    }
    void SyncOutput::writeHeader()
    {
        output << "<root>\n";
        output << "    <primes>";
    }
    void SyncOutput::writeFooter()
    {
        output << "</primes>\n";
        output << "</root>";
    }
    void SyncOutput::close()
    {
        //Destroy work object. This allows the I/O thread to exit the event loop
        //when there ane no more pendinge asynchronous operations
        work.reset( nullptr );

        //Wait for I/O thread to exit
        workingThread->join();
    }
}