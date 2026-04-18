#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    event_t<> ev;

    worker::add([=](){
        auto krn = type::bind( process::kernel() );
        process::add([=](){
            console::log( ">>", process::size() );
        return 1; });
        ev.on([=](){ 
            krn->loop_add([=](){
                console::log( "hello world" );
            return 1; });
        });
        console::log( "real0:", &process::kernel() );
    process::wait(); return -1; });

    worker::add([=](){
        ev.on([=](){ console::log( "<B>", &process::kernel() ); });
        console::log( "real1:", &process::kernel() );
    return -1; });

    worker::add([=](){
        ev.on([=](){ console::log( "<C>", &process::kernel() ); });
        console::log( "real2:", &process::kernel() );
    return -1; });

    timer::timeout([=](){
        ev.emit();
        console::log( "<D>", &process::kernel() );
    },1000);

}