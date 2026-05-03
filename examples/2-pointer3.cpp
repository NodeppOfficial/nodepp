#include <nodepp/nodepp.h>

using namespace nodepp;

class my_class { public:

    int variable = 10;

    my_class() { 
        auto self = type::bind( this ); // ptr_t<my_class> ( 0UL, *this );    
    process::add( [=]() -> int {
        console::log( self->variable ); // 10
        self->variable = 20;
        console::log( self->variable ); // 20
    return -1; }); }

};

void onMain() {

    my_class obj;
    console::log( obj.variable );

}