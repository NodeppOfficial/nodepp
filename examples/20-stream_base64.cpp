#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>
#include <nodepp/encoder.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain() {

    auto fd = file_t( "b64.txt", "r" );
    auto fo = file_t( "b64.png", "w" );
    auto ft = crypto::decoder::BASE64();

    fd.onData([=]( string_t data ){ ft.update(data); });
    ft.onData([=]( string_t data ){ fo.write (data); });

    stream::pipe( fd );

}