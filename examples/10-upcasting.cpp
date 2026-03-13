#include <nodepp/nodepp.h>

using namespace nodepp;

class a_t { public:
    int value = 10;
    a_t() /*----*/ { console::log( "AAA" ); }
    virtual ~a_t() { console::log( "-A-" ); }
};

class b_t : public a_t { public:
    template< class... A >
    b_t( const A&... args ) : a_t( args... ) { console::log( "BBB" ); }
   ~b_t() /*------------------------------*/ { console::log( "-B-" ); }
};

void onMain(){

    a_t a; auto b = type::bind( b_t(a) ); 

    console::log( a .value );
    console::log( b->value );

}