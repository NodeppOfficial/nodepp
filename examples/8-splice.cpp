#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    array_t<int> arr ({ 0, 1, 2, 3, 4, 5 });
    queue_t<int> que ({ 0, 1, 2, 3, 4, 5 });
    string_t     str = "012345";

    console::log( "str:", str.slice(0,2) );
    console::log( "arr:", arr.slice(0,2).join() );
    console::log( "que:", array_t<int>(que.slice(0,2).data()).join() );

    console::log( "---" );

    console::log( "str:", str.splice(2,2,"98") );
    console::log( "arr:", arr.splice(2,2,{9,8}).join() );
    console::log( "que:", array_t<int>(que.splice(2,2,{9,8}).data()).join() );

    console::log( "---" );

    console::log( "str:", str );
    console::log( "arr:", arr.join() );
    console::log( "que:", array_t<int>(que.data()).join() );

}
