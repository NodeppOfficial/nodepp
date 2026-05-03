#include <nodepp/nodepp.h>
#include <nodepp/dns.h>

using namespace nodepp;

void onMain(){

    auto list = dns::lookup( "www.google.com" );
    console::log( "<size>", list.size() );

    for( auto x: list ){
        console::log( "<dns>", 
            x.address, x.hostname,
            x.family== AF_INET   ? "IPv4" :
            x.family== AF_INET6  ? "IPv6" : "unknown"
        );
    }

}