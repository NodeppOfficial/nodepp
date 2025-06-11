#include <nodepp/nodepp.h>
#include <nodepp/url.h>

/*────────────────────────────────────────────────────────────────────────────*/

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

void onMain(){
	
    string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
    auto     ppt = url::parse( uri ); 

    console::log( "hostname:", ppt.hostname );
    console::log( "protocol:", ppt.protocol );
    console::log( "pathname:", ppt.pathname );
    console::log( "search:",   ppt.search );
    console::log( "origin:",   ppt.origin );
    console::log( "port:",     ppt.port );
    console::log( "host:",     ppt.host );
    console::log( "hash:",     ppt.hash );
    console::log( "user:",     ppt.user );
    console::log( "pass:",     ppt.pass );
    console::log( "auth:",     ppt.auth );
    console::log( "href:",     ppt.href );

    for( auto x : ppt.query.data() )
         console::log( x.first, ":>", x.second );

}

/*────────────────────────────────────────────────────────────────────────────*/