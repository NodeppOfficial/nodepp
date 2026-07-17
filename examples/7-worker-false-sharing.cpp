#include <nodepp/nodepp.h>
#include <nodepp/worker.h>

using namespace nodepp;

int shared = 100; //<- false sharing

int worker_task( int wid ) {
coStart

    while( shared>0 ){
        console::log( wid, "->", shared-- );
    coNext; }

coStop
}

void onMain(){

    worker ::add([=](){ return worker_task(0); });
    worker ::add([=](){ return worker_task(1); });
    worker ::add([=](){ return worker_task(2); });

}