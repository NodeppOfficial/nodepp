#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    crypto::encrypt::RSA ppt; ppt.generate_keys();
    string_t msg = "Hello World!";

    auto data = ppt.public_encrypt( msg );
    console::log( data.size(), data );

    auto decp = ppt.private_decrypt( data );
    console::log( decp.size(), decp );

}