#include <nodepp/nodepp.h>
#include <nodepp/fs.h>

/*────────────────────────────────────────────────────────────────────────────*/

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

void onMain(){

    auto f = fs::readable( "LICENSE" );
    auto x = type::bind( f );

    process::add([=](){
    coStart

        while( x->is_available() ){
            console::log( ":>", x->read_line().slice(0,-1) );
            coNext;
        }

    coStop
    });

    process::add([=](){
    coStart

        while( x->is_available() ){
            console::done("Hello World");
            coNext;
        }

    coStop
    });

}

/*────────────────────────────────────────────────────────────────────────────*/