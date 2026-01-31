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

namespace nodepp { class https_t : public ssocket_t, public generator_t {
protected:

    generator::file::line line; string_t raw;
    
public:

    uint      status=200;
    string_t  version;
    header_t  headers;

    string_t  search;
    string_t  method;
    string_t  path;
    
    /*─······································································─*/

    template< class... T > 
    https_t( const T&... args ) noexcept : ssocket_t( args... ) {}

    /*─······································································─*/

    void     set_version( const string_t& msg ) noexcept { version = msg; }
    string_t get_version() const noexcept { return version; }

    /*─······································································─*/

    int read_header() noexcept {  

        if( process::millis() > get_conn_timeout() ){ return -1; }
        thread_local static ptr_t<regex_t> reg({
            regex_t( "[^ \r]+" ),
            regex_t( "^[^?#]+" ),
            regex_t( "?[^#]+"  )
        });
        
    bool b=1; coBegin
    
        if( !is_available() ) /*--*/ { coEnd; } coWait( line( this )==1 ); 
        if( line.state <= 0 ) /*--*/ { coEnd; } raw = line.data;
        if( raw.find("HTTP").null() ){ coEnd; }

        do{ coWait( line( this )==1 ); if( line.state<=0 ){ coEnd; } do {
            auto x = line.data; auto y = x.find( ": " ); 
        if( y.null() ){ b=0; break; }
            headers[ x.slice( 0, y[0] ).to_capital_case() ] = x.slice( y[1], -2 );
        } while(0); } while(b); 

        do{ auto base=reg[0].match_all(raw);
        if( base.size() < 3 ){ return -1; }
        if( !string::is_digit(base[1][0]) ){

            version= base[2]; method =base[0]; 
            search = reg [2].match( base[1] );
            path   = reg [1].match( base[1] );

        } else { version = base[0]; status = string::to_uint( base[1] ); }
        } while(0); coStay(0);

    coFinish }
    
    /*─······································································─*/

    void write_header( const string_t& method, const string_t& path, const string_t& version, const header_t& headers ) const noexcept { 
         string_t res = string::format("%s %s %s\r\n",(char*)method,(char*)path,(char*)version);
         for( auto x:headers.data() ){ res += string::format("%s: %s\r\n",(char*)x.first.to_capital_case(),(char*)x.second); }
         /*-------------------------*/ res += "\r\n"; write( res ); if( method=="HEAD" ){ close(); }
    }
    
    /*─······································································─*/

    void write_header( uint status, const header_t& headers ) const noexcept { 
         string_t res = string::format("%s %u %s\r\n",(char*)version,status,(char*)HTTP_NODEPP::_get_http_status(status));
         for( auto x:headers.data() ){ res += string::format("%s: %s\r\n",(char*)x.first.to_capital_case(),(char*)x.second); }
         /*-------------------------*/ res += "\r\n"; write( res ); if( method=="HEAD" ){ close(); } 
    }
    
    /*─······································································─*/

    template< class T > void write_header( const T& fetch, const string_t& path ) const noexcept {

        bool b = !fetch->body.empty() || fetch->file.is_available();
        string_t res = string::format( "%s %s %s\r\n", fetch->method.get(), path.get(), fetch->version.get() );

        for( auto x:fetch->headers.data() ){ res += string::format("%s: %s\r\n",(char*)x.first.to_capital_case(),(char*)x.second); }
        if ( !b ) /*--------------------*/ { res += "\r\n"; } 
        if ( fetch->method == "HEAD" )/**/ { write(res); close(); return; }
        if ( !b ) /*--------------------*/ { res += "\r\n"; write( res ); return; }
        
        if( !fetch->file.is_closed() ) { 
            res += string::format("Content-Length: %lu\r\n\r\n",fetch->file.size()); write( res );
            while( fetch->file.is_available() ){ write( fetch->file.read() ); } //write( "\r\n" ); 
        } elif( !fetch->body.empty() ) { 
            res += string::format("Content-Length: %lu\r\n\r\n",fetch->body.size());
            res += fetch->body; /*res+="\r\n";*/ write( res );
        } else { res += "\r\n"; write( res ); }

    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace https {

    inline tls_t server( function_t<void,https_t> cb, ssl_t* ssl=nullptr, agent_t* opt=nullptr ){
        return tls_t([=]( https_t cli ){

            int c=0; while((c=cli.read_header())==1){ /*unused*/ }
            if( c==0 ){ cb(cli); return; }
        
        cli.close(); }, ssl, opt );
    }
    
    /*─······································································─*/
    
    inline promise_t<https_t,except_t> fetch ( const fetch_t& args, ssl_t* ssl=nullptr, agent_t* opt=nullptr ) {
           auto agent = type::bind( opt  ); auto cert = type::bind( ssl );
           auto fetch = type::bind( args ); /*--------------------------*/ 
    return promise_t<https_t,except_t>([=]( res_t<https_t> res, rej_t<except_t> rej ){

        if( !url::is_valid( fetch->url ) ){ rej(except_t("invalid URL")); return; }
             url_t uri = url::parse( fetch->url );

        if( !fetch->query.empty() ){ uri.search=query::format(fetch->query); }
        string_t dip = uri.hostname ; fetch->headers["Connection"] = "close";
        /*-------------------------*/ fetch->headers["Host"] = dip;
        string_t dir = uri.pathname + uri.search + uri.hash;

        auto skt = tls_t([=]( https_t cli ){

            cli.set_timeout( fetch->timeout ); cli.write_header( fetch, dir );
            int c=0; while((c=cli.read_header())==1){ /*unused*/ }

            if( c==0 ){ res(cli); return; } cli.close();
            rej(except_t("Could not connect to server"));
            
        }, &cert, &agent );

        skt.onError([=]( except_t error ){ rej(error); });
        skt.connect( dip, uri.port );

    }); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/