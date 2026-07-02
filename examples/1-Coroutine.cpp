#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    auto co1 = coroutine::add<int,int,int>( COROUTINE_ARG( int x, int y, int z ){
        console::log( ">>", x, y, z );
    return 1; });

    auto co2 = coroutine::add( COROUTINE(){
        console::log( "hello world" );
    return 1; });

    co1( 10, 20, 30 );
    co2();

}