/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_HTTPS
#define NODEPP_HTTPS

/*────────────────────────────────────────────────────────────────────────────*/

#include "http.h"
#include "tls.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class https_t : public ssocket_t {
public:

    uint      status=200;
    string_t  version;
    header_t  headers;
    query_t   query;

    string_t  protocol;
    string_t  search;
    string_t  method;
    string_t  path;
    string_t  url;
    
    /*─······································································─*/

    template< class... T > 
    https_t( const T&... args ) noexcept : ssocket_t( args... ) {}

    /*─······································································─*/

    void     set_version( const string_t& msg ) noexcept { version = msg; }

    string_t get_version() const noexcept { return version; }

    /*─······································································─*/

    int read_header() noexcept { if( !is_available() ){ return -1; }
        
        auto base = read_line(); protocol="HTTPS";
        auto init = regex::split( base, "\\s+" );

        if( !regex::test( base,"HTTP/\\d\\.\\d" ) ){ return -1; }
        if( init.size() < 4 ){ return -1; }

        if( !regex::test( init[1], "^\\d+" ) ) {
            auto idx = init[1].index_of([]( char x ){ return x=='?'; });
              
            if( idx > 0 ){
                     path= init[1].slice( 0,idx );
                   search= init[1].slice(   idx );
                    query= query::parse( search );
            } else { path= init[1]; }

                   method= init[0]; if( version.empty() ) version = init[2];
            string_t host= !headers.has("Host") ? "localhost" : headers["Host"];
                      url= string::format( "http://%s%s%s", (char*)host, (char*)path, (char*)search );
        } else { version = init[0]; status = string::to_uint( init[1] ); }

        int idx = 0; do { auto line = read_line();
            idx = line.index_of([]( char x ){ return x==':'; });
            if( idx<0 ){ break; } auto a = line.slice( 0,idx ).to_capital_case();
                                  auto b = line.slice( idx+2, -2 ); headers[a]=b;
        } while ( true ); return 0;

    }
    
    /*─······································································─*/

    void write_header( const string_t& method, const string_t& path, const string_t& version, const header_t& headers ) const noexcept {
        string_t res; res += string::format("%s %s %s\r\n",(char*)method,(char*)path,(char*)version);
        for( auto x:headers.data() ){ res += string::format("%s: %s\r\n",(char*)x.first.to_capital_case(),(char*)x.second); }
                                      res += "\r\n"; write( res ); if( method == "HEAD" ){ close(); }
    }
    
    /*─······································································─*/

    void write_header( uint status, const header_t& headers ) const noexcept { 
        string_t res; res += string::format("%s %u %s\r\n",(char*)version,status,(char*)HTTP_NODEPP::_get_http_status(status));
        for( auto x:headers.data() ){ res += string::format("%s: %s\r\n",(char*)x.first.to_capital_case(),(char*)x.second); }
                                      res += "\r\n"; write( res ); if( method == "HEAD" ){ close(); } 
    }
    
    /*─······································································─*/

    template< class T > void write_header( const T& fetch, const string_t& path ) const noexcept {

        bool b = !fetch->body.empty() || fetch->file.is_available(); string_t res; 

        res += string::format( "%s %s %s\r\n", fetch->method.get(), path.get(), fetch->version.get() );
        for( auto x:fetch->headers.data() ){ res += string::format("%s: %s\r\n",(char*)x.first.to_capital_case(),(char*)x.second); }
        if ( !b )                          { res += "\r\n"; } if( fetch->method == "HEAD" ){ write( res ); close(); return; }
        if ( !b )                          { res += "\r\n"; write( res ); return; }
        
        if( !fetch->file.is_closed() ) { 
            res += string::format("Content-Length: %lu\r\n\r\n",fetch->file.size()); write( res );
            while( fetch->file.is_available() ) { write( fetch->file.read() ); } write( "\r\n" ); 
        } elif( !fetch->body.empty() ) { 
            res += string::format("Content-Length: %lu\r\n\r\n",fetch->body.size());
            res += fetch->body; res += "\r\n"; write( res );
        } else { 
            res += "\r\n"; write( res );
        }

    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace https {

    template< class T > tls_t server( T cb, const ssl_t* ctx, agent_t* opt=nullptr ){
        return tls_t([=]( https_t cli ){ int c=0;
            if( cli.read_header()==0 ){ cb( cli ); } 
          else{ cli.close(); }
        }, ctx, opt ); 
    }
    
    /*─······································································─*/
    
    promise_t<https_t,except_t> fetch ( const fetch_t& cfg, const ssl_t* ctx, agent_t* opt=nullptr ) { 
        if( ctx == nullptr ) process::error( "Invalid SSL Context" );
           auto agn = type::bind( opt ); 
           auto gfc = type::bind( cfg );
           auto ssl = type::bind( ctx );
    return promise_t<https_t,except_t>([=]( function_t<void,https_t> res, function_t<void,except_t> rej ){

        if( !url::is_valid( gfc->url ) ){ rej(except_t("invalid URL")); return; }
        
        url_t uri = url::parse( gfc->url );
        if( !gfc->query.empty() ){ uri.search=query::format(gfc->query); }
        
        string_t dip = uri.hostname ; gfc->headers["Host"] = dip;
        string_t dir = uri.pathname + uri.search + uri.hash;
       
        auto client = tls_t ([=]( https_t cli ){ 
            cli.set_timeout( gfc->timeout ); int c = 0; cli.write_header( gfc, dir );

            if( cli.read_header()==0 ){ res( cli ); } else { 
                rej(except_t("Could not connect to server"));
                cli.close();
            }
            
        }, &ssl, &agn );

        client.onError([=]( except_t error ){ rej(error); });
        client.connect( dip, uri.port );

    }); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
