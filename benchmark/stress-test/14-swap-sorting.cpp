#include <nodepp/nodepp.h>
#include <nodepp/json.h>

using namespace nodepp;

void onMain(){

    queue_t<int>   que ({ 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 });
    queue_t<void*> mem;

    auto x = que.last(); while( x!=nullptr ){
         mem.push( (void*) x ); 
    x = x->prev; }

    auto y = mem.last(); while( y!=nullptr ){
        que.swap( que.get( os::rand() ), que.as( y->data ) );
    //  ^swap cause a seg fault error - bug fixed   
    y = y->prev; }

    console::log( string::join( que ) );

}