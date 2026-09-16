#include <nodepp/nodepp.h>
#include <nodepp/event.h>

using namespace nodepp;

void onMain(){

    event_t<> event; ptr_t<task_t> ppt ( 0UL );
    ptr_t<uchar> tmp (0UL, 10);

    auto out = event.add([=](){

        event.off( ppt );
        
        while( tmp[0] --> 0 ){
             console::log( ">>", *tmp ); 
             return 1;
        }

    return -1; });

    *ppt = *out; 
    
    while( !event.empty() ){ event.emit(); }

}