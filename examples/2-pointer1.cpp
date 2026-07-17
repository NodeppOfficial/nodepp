#include <nodepp/nodepp.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    class my_object { public: int value = 10;
        my_object(){ console::log( "constructor" ); }
       ~my_object(){ console::log( "destructor"  ); }
    };

    ptr_t<my_object> obj ( 0UL  ); // <- creat only one pointer
//  ptr_t<my_object> obj ( 10UL ); // <- create a buffer

    timer::timeout([=](){
        console::log( "done", obj->value );
    },3000);

}