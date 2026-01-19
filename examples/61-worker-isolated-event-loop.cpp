#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/os.h>

using namespace nodepp;

atomic_t<int> shared = 100;

void isolated_event_loop( uint cpu_id ){ worker::add([=](){ 

    //  this worker runs it's own event loop in parallel

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( shared-->0 ){
            console::log( cpu_id, "->", shared.get() );
        coDelay(100); }

    coFinish
    }));

    process::wait();

return -1; }); }

void onMain(){

    for( auto x=os::cpus(); x-->0; ){
         isolated_event_loop( x );
    }

}