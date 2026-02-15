#include <nodepp/nodepp.h>
#include <nodepp/tcp.h>

using namespace nodepp;

void onMain() {
    auto srv = tcp::server();

    srv.onConnect([=]( socket_t cli ){
        // We do nothing and let the cli object go out of scope.
        // This tests if the epoll anchor is cleaned up correctly
        // even if the user doesn't 'read' or 'write'.
    });

    srv.listen( "0.0.0.0", 8000 );
}