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

namespace nodepp { class https_t : public ssocket_t { public:

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

    int read_header() noexcept { try { if( !is_available() ){ throw ""; }

        auto base= regex::match_all( read_line(), "\\S+" ); 
        if ( base.size() != 3 ){ throw ""; } protocol = "HTTPS";

        if ( !regex::test( base[1], "^\\d+" ) ) {
            string_t host=!headers.has("Host")? "localhost": headers["Host"];
                     
            url    = string::format("http://%s%s", host.get(), base[1].get() );
            path   = nodepp::url::path  ( url );
            search = nodepp::url::search( url );
            query  = nodepp::url::query ( url );
            version= base[2]; method = base[0]; 
            
        } else { version = base[0]; status = string::to_uint( base[1] ); }

        for(;;){
            auto line= read_line(); auto raw = regex::search( line,": " ); 
            if ( raw.empty() ){ break; }
            headers[ line.slice(0,raw[0]) ]=line.slice(raw[0],-2);
        }

    } catch(...){ return -1; } return 0; }
    
    /*─······································································─*/

    void write_header( const string_t& method, const string_t& path, const string_t& version, const header_t& headers ) const noexcept { 
         string_t res = string::format("%s %s %s\r\n",(char*)method,(char*)path,(char*)version);
         for( auto x:headers.data() ){ res += string::format("%s: %s\r\n",(char*)x.first.to_capital_case(),(char*)x.second); }
                                       res += "\r\n"; write( res ); if( memcmp( method.get(), "HEAD", 4 )==0 ){ close(); }
    }
    
    /*─······································································─*/

    void write_header( uint status, const header_t& headers ) const noexcept { 
         string_t res = string::format("%s %u %s\r\n",(char*)version,status,(char*)HTTP_NODEPP::_get_http_status(status));
         for( auto x:headers.data() ){ res += string::format("%s: %s\r\n",(char*)x.first.to_capital_case(),(char*)x.second); }
                                       res += "\r\n"; write( res ); if( memcmp( method.get(), "HEAD", 4 )==0 ){ close(); } 
    }
    
    /*─······································································─*/

    template< class T > void write_header( const T& fetch, const string_t& path ) const noexcept {

        bool b = !fetch->body.empty() || fetch->file.is_available();
        string_t res = string::format( "%s %s %s\r\n", fetch->method.get(), path.get(), fetch->version.get() );

        for( auto x:fetch->headers.data() ){ res += string::format("%s: %s\r\n",(char*)x.first.to_capital_case(),(char*)x.second); }
        if ( !b )                          { res += "\r\n"; } 
        if ( memcmp( fetch->method.get(), "HEAD", 4 )==0 ){ write(res); close(); return; }
        if ( !b )                          { res += "\r\n"; write( res ); return; }
        
        if( !fetch->file.is_closed() ) { 
            res += string::format("Content-Length: %lu\r\n\r\n",fetch->file.size()); write( res );
            while( fetch->file.is_available() ){ write( fetch->file.read() ); } write( "\r\n" ); 
        } elif( !fetch->body.empty() ) { 
            res += string::format("Content-Length: %lu\r\n\r\n",fetch->body.size());
            res += fetch->body; res += "\r\n"; write( res );
        } else { res += "\r\n"; write( res ); }

    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace https {

    template< class T > tls_t server( T cb, const ssl_t& ssl, agent_t* opt=nullptr ){
        return tls_t([=]( https_t cli ){
              if( cli.read_header()==0 ){ cb( cli ); } 
            else{ cli.close(); }
        }, ssl, opt );
    }

    /*─······································································─*/

    promise_t<https_t,except_t> fetch ( const fetch_t& args, const ssl_t& ssl, agent_t* opt=nullptr ) {
           auto agent = type::bind( opt ); auto fetch = type::bind( args ); 
    return promise_t<https_t,except_t>([=]( function_t<void,https_t> res, function_t<void,except_t> rej ){

        if( !url::is_valid( fetch->url ) ){ rej(except_t("invalid URL")); return; }
             url_t uri = url::parse( fetch->url );

        if( !fetch->query.empty() ){ uri.search=query::format(fetch->query); }
        string_t dip = uri.hostname ; fetch->headers["Host"] = dip;
        string_t dir = uri.pathname + uri.search + uri.hash;
       
        auto skt = tls_t([=]( https_t cli ){ 

            cli.set_timeout( fetch->timeout ); cli.write_header( fetch, dir );
            if( cli.read_header()==0 ){ res( cli ); } else { 
                rej(except_t("Could not connect to server"));
                cli.close();
            }
            
        }, ssl, &agent );

        skt.onError([=]( except_t error ){ rej(error); });
        skt.connect( dip, uri.port );

    }); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif