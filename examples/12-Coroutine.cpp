#include <nodepp/nodepp.h>
#include <nodepp/fs.h>

/*────────────────────────────────────────────────────────────────────────────*/

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

void onMain(){

    auto file = fs::readable( "LICENSE" );
    auto x    = type::bind( file );

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( x->is_available() ){
            console::log( ":>", x->read_line().slice(0,-1) );
            coNext;
        }

    coFinish
    }));

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( x->is_available() ){
            console::done("Hello World");
            coNext;
        }

    coFinish
    }));

}

/*────────────────────────────────────────────────────────────────────────────*/