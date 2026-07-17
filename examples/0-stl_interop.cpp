#include <nodepp/nodepp.h>
#include <nodepp/event.h>

#include <string>
#include <queue>
#include <vector>
#include <functional>

using namespace nodepp;

void onMain(){

    ptr_t<int> ptr ( 0UL, 10 ); // as value
//  ptr_t<int> ptr ( 1UL, 10 ); // as buffer

    /*-----*/

    std::function<void(int)> clb ([=]( int value ){
        console::log( value, *ptr );
    });

    /*-----*/

    event_t<int> ev; ev.once( clb );
    ev.emit( 100 );

    /*-----*/

    std::string str = "hello world";
    console::log( ">>", str );

    /*-----*/

    std::vector<int> arr ({ 10, 20, 30, 40, 50 });
    console::log( ">>", array_t<int>( arr ).join() );

    /*-----*/

    std::queue<int> st ({ 10, 20, 30, 40 });
    queue_t<int> que ( st );
    que.map([=]( int value ){ console::log( "<>", value ); });

    /*-----*/

    std::string str2 = "hello world";
    ptr_t<std::string> ptr1 ( 0UL ); 
    *ptr1 = type::move( str2 );
    console::log( *ptr1 );

    /*----*/

    std::vector<int> vec ({ 10, 20, 30 });
    ptr_t<int> ptr2 ( 3UL );
    type::move( vec.begin(), vec.end(), ptr2.begin() );
    console::log( array_t<int>( ptr2 ).join() ); // <- cheap for the CPU

}