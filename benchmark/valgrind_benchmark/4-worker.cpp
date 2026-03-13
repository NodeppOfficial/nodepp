#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/channel.h>

using namespace nodepp;

atomic_t<ulong> done = false;

void onMain() {
    
    console::log("Worker Stress Test Started (2 Workers)...");
    channel_t<string_t> ch;

    for( int x=2; x-->0; ){ 
    worker::add( [=](){

        ptr_t<string_t> memory;

        if   ( done.get() ) { return -1; }
        while( ch._read( memory ) == -2 ){ 
            process::delay(1); return 1;
        }

        if( memory.null() ) { return  1; }
        console::log( *memory );

    return 1; }); }

    ptr_t<int> idx ( 0UL,100000 );

    process::add( [=](){

        // Send 100,000 tasks across workers
        while( *idx >= 0 ){
            ch.write( string::format( "Task_Data_Payload_Stress %d", *idx ) );
            process::delay(1); *idx -= 1; return 1; 
        }

        done = true; console::log("done"); 

    return -1; });

}