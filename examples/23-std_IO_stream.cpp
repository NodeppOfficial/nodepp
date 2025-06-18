#include <nodepp/nodepp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain() {

    console::log( "write something asynchronously" );

    auto output = fs::std_output(); // writable file stream
    auto input  = fs::std_input();  // readable file stream
    auto error  = fs::std_error();  // writable file stream

    input.onData([=]( string_t data ){
        output.write( "your input is: " );
        output.write( data );
        output.write( "\n" );
    });

    stream::pipe( input );

}