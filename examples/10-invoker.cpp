#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    string_t addr1 = process::invoke([=]( any_t raw ){
        auto data = raw.as<ptr_t<ulong>>();
        console::log( "<-->", data, *data, process::now() );
    return 1; });

    string_t addr2 = process::invoke([=]( any_t raw ){
        static ulong x=1; static ulong y=1; ulong z=0;
        auto data = raw.as<ptr_t<ulong>>();
        z = x; x = z + y; y = z; *data = x;
    //  console::log( "<-->", data, *data, process::now(), x );
    return 1; });

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){ do {
            auto x = ptr_t<ulong>( 0UL, 0 );
            process::call( addr2, x );
            process::call( addr1, x );
        } while(0); coDelay(1000); }

    coFinish
    }));

}