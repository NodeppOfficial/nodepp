#include <nodepp/nodepp.h>

/*────────────────────────────────────────────────────────────────────────────*/

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

void onMain(){

    //return 1 -> will repeat the task
    process::add([=](){
        console::done("repeat mode");
        return  1;
    });

    //return -1 -> will kill the task
    process::add([=](){
        console::error("once mode");
        return -1;
    });

    //return 0 -> will block until != 0
    process::add([=](){
        console::error("block mode");
        return  0;
    });

}

/*────────────────────────────────────────────────────────────────────────────*/
