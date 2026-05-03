#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    queue_t<uint> que ({ 10, 20, 30 });
    que.push( 40 ); 
    que.push( 50 ); 
    que.unshift( 60 );
    que.unshift( 70 );
    que.insert( 2, 80 ); 

    auto item = que.first();
    while( item != nullptr ){
        console::log( "->", item->data );
        item = item->next;
    }

    console::log("-- --");
    for( auto &x: que.data() ){ console::log( "->", x ); }

    console::log("-- --");
    que.map([]( uint item ){ console::log( "->", item ); });

    console::log("-- --");

    que.sort([=]( uint a, uint b ){
        return a <= b;
    }) .map([]( uint item ){
        console::log( "->", item );
    });

    console::log("-- --");

    console::log( "is_empty:", que.empty() ? "true" : "false" );
    console::log( "size:", que.size() );
    console::log( "addr:", que.data() ); que.clear();
    console::log( "is_empty:", que.empty() ? "true" : "false" );

}