#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    queue_t<int> que ({ 10, 20, 30, 40, 50 });
    console::log( array_t<int>( que.data() ).join("-") );

    que.move( nullptr, que.first()->next );
    console::log( array_t<int>( que.data() ).join("-") );

    que.move( nullptr, que.first() );
    console::log( array_t<int>( que.data() ).join("-") );

    que.move( que.first()->next->next, que.first() );
    console::log( array_t<int>( que.data() ).join("-") );

}