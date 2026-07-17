#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    queue_t<int> que ({ 0, 10, 20 });

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){ do {
            if( que.empty() ){ break; } que.next(); 
            console::log( ">>", que.get()->data );
        } while(0); coDelay(1000); } 

    coFinish
    }));

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){ do {
            if( que.empty() ){ break; } 
            auto val = que.first()->data; que.shift();
            console::log( ">------> removed:", val );
        } while(0); coDelay(10000); }

    coFinish
    }));

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){ do {
            auto val = que.empty() ? 0 : que.last()->data; 
            que.push( val + 10 );
            console::log( ">------> added:", val + 10 );
        } while(0); coDelay(5000); } 

    coFinish
    }));

}