#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    auto ppt = crypto::encrypt::AES_192_CBC( "key1234567890" );
         ppt.update("Hello World!");
    auto p = ppt.get();

    console::log( ":>", p.size(), p );

    auto ttp = crypto::decrypt::AES_192_CBC( "key1234567890" );
         ttp.update( p );
    auto q = ttp.get();

    console::log( ":>", q.size(), q );

}