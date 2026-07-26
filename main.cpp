#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>
#include <nodepp/encoder.h>
#include <nodepp/handler.h>

using namespace nodepp;

void onMain(){

    handler_t<int> hdl;
    auto x = hdl.create(); auto t = (char*) &x;
    auto y = ptr_t<char>({ t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7], (char)0x00 });

    auto b = crypto::encoder::BASE58(); b.update( y );
    auto c = string::split( b.get (), 5 );
    console::log( b.get(), c.size(), c.join("-") );

    auto d = crypto::decoder::BASE58(); d.update( b.get() );
    auto e = d.get();
    console::log( "->", y.size(), e.size()  );

    for( int w=0; w<y.size(); w++ ){
         console::log( (uchar) y[w], (uchar)e.ptr()[w] );
    }

}