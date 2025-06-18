#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/serial.h>

using namespace nodepp;

void onMain(){

    auto device = serial::get_devices();

    if( device.empty() ){ 
        process::error("Not Serial Device Found"); 
    } else {
        console::log( device[0] );
    }

    auto x = serial::connect( device[0], 9600 );

    x.onConnect([]( serial_t cli ){

        console::log( "connected" );

        cli.onData([]( string_t chunk ){
            console::log( chunk );
        });

    });

}