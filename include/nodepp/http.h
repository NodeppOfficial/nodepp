/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_HTTP
#define NODEPP_HTTP

/*────────────────────────────────────────────────────────────────────────────*/

#include "generator.h"
#include "promise.h"
#include "encoder.h"
#include "query.h"
#include "url.h"
#include "tcp.h"
#include "map.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using header_t = map_t< string_t, string_t >; namespace HTTP_NODEPP {

    inline string_t _get_http_status( uint status ){ switch( status ){
        
        case 100: return "Continue";                        break;
        case 101: return "Switching Protocols";             break;
        case 102: return "Processing";                      break;
        case 103: return "Early Hints";                     break;
        
        case 200: return "OK";                              break;
        case 201: return "Created";                         break;
        case 202: return "Accepted";                        break;
        case 203: return "Non-Authoritative Information";   break;
        case 204: return "No Content";                      break;
        case 205: return "Reset Content";                   break;
        case 206: return "Partial Content";                 break;
        case 207: return "Multi-Status";                    break;
        case 208: return "Already Reported";                break;
        case 226: return "IM Used";                         break;
 
        case 300: return "Multiple Choices";                break;
        case 301: return "Moved Permanently";               break;
        case 302: return "Found";                           break;
        case 303: return "See Other";                       break;
        case 304: return "Not Modified";                    break;
        case 305: return "Use Proxy";                       break;
        case 307: return "Temporary Redirect";              break;
        case 308: return "Permanent Redirect";              break;

        case 400: return "Bad Request";                     break;
        case 401: return "Unauthorized";                    break;
        case 402: return "Payment Required";                break;
        case 403: return "Forbidden";                       break;
        case 404: return "Not Found";                       break;
        case 405: return "Method Not Allowed";              break;
        case 406: return "Not Acceptable";                  break;
        case 407: return "Proxy Authentication Required";   break;
        case 408: return "Request Timeout";                 break;
        case 409: return "Conflict";                        break;
        case 410: return "Gone";                            break;
        case 411: return "Length Required";                 break;
        case 412: return "Precondition Failed";             break;
        case 413: return "Payload Too Large";               break;
        case 414: return "URI Too Long";                    break;
        case 415: return "Unsupported Media Type";          break;
        case 416: return "Range Not Satisfiable";           break;
        case 417: return "Expectation Failed";              break;
        case 418: return "I'm a Teapot";                    break;
        case 421: return "Misdirected Request";             break;
        case 422: return "Unprocessable Entity";            break;
        case 423: return "Locked";                          break;
        case 424: return "Failed Dependency";               break;
        case 425: return "Too Early";                       break;
        case 426: return "Upgrade Required";                break;
        case 428: return "Precondition Required";           break;
        case 429: return "Too Many Requests";               break;
        case 431: return "Request Header Fields Too Large"; break;
        case 451: return "Unavailable For Legal Reasons";   break;

        case 500: return "Internal Server Error";           break;
        case 501: return "Not Implemented";                 break;
        case 502: return "Bad Gateway";                     break;
        case 503: return "Service Unavailable";             break;
        case 504: return "Gateway Timeout";                 break;
        case 505: return "HTTP Version Not Supported";      break;
        case 506: return "Variant Also Negotiates";         break;
        case 507: return "Insufficient Storage";            break;
        case 508: return "Loop Detected";                   break;
        case 509: return "Bandwidth Limit Exceeded";        break;
        case 510: return "Not Extended";                    break;
        case 511: return "Network Authentication Required"; break;

    }   /*-----*/ return nullptr; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { struct fetch_t {

    query_t   query  ;
    string_t  body   ;
    header_t  headers;
    ulong     timeout= 60000;
    
    /*─······································································─*/

    string_t     url ;
    string_t  method = "GET";
    string_t version = "HTTP/1.0";
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class http_t : public socket_t, public generator_t {
protected:

    struct DONE { len_t size; int state; };
    struct NODE {
        generator::file::line  line ; DONE mode[2];
        generator::http::read  read ;
        generator::http::write write;
    };  ptr_t<NODE> http;

    enum FLAG {
         HTTP_FLAG_UNKNOWN = 0b00000000,
         HTTP_FLAG_CHUNKED = 0b00000001,
         HTTP_FLAG_STREAM  = 0b00000010,
    };

    void set_http_mode( DONE& mode, header_t header ) const noexcept { 
        mode.state = FLAG::HTTP_FLAG_UNKNOWN ; mode.size = 0UL;
        if( !header.has( "Content-Length"    ) ){
        if( !header.has( "Transfer-Encoding" ) ){ return; } else { 
            
            auto itm = header ["Transfer-Encoding"];
            if ( itm.to_lower_case().find( "chunked" ).null() ){ 
                     mode.state |= FLAG::HTTP_FLAG_UNKNOWN;
            } else { mode.state |= FLAG::HTTP_FLAG_CHUNKED; }

        }} else { 
            mode.size  = string::to_u64( header["Content-Length"] );
            mode.state|= FLAG::HTTP_FLAG_STREAM;
        }
    }

    void set_recv_mode( header_t header ) const noexcept { 
         set_http_mode( http->mode[0], header ); 
    }

    void set_send_mode( header_t header ) const noexcept { 
         set_http_mode( http->mode[1], header ); 
    }

public:

    uint      status = 200;
    string_t  version;
    header_t  headers;

    string_t  body  ;
    string_t  search;
    string_t  method;
    string_t  path  ;
    
    /*─······································································─*/

    template< class... T > 
    http_t( const T&... args ) noexcept : socket_t( args... ), http( new NODE() ) {}

    /*─······································································─*/

    int read_header() noexcept { if( is_closed() ){ return -1; } 
        
        thread_local static ptr_t<regex_t> reg({
            regex_t( "[^ \r]+" ),
            regex_t( "^[^?#]+" ),
            regex_t( "?[^#]+"  )
        });
        
    bool b=1; coBegin

        set_recv_mode( nullptr ); if( is_server() ) { set_send_mode( nullptr ); }
    
        if( !is_available() ) /*--------------*/ { coEnd; } coWait( http->line( this )==1 ); 
        if( http->line.state <= 0 ) /*--------*/ { coEnd; }
        if( http->line.data.find("HTTP").null() ){ coEnd; }

        do{ auto base=reg[0].match_all( http->line.data );
        if( base.size() < 3 ){ return -1; }
        if( !string::is_digit(base[1][0]) ){

            version= base[2]; method =base[0]; 
            search = reg [2].match( base[1] );
            path   = reg [1].match( base[1] );

        } else { version = base[0]; status = string::to_uint( base[1] ); }
        } while(0); 

        do{ coWait( http->line( this )==1 ); if( http->line.state<=0 ){ coEnd; } 
        do{ auto x= http->line.data; auto y = x.find( ": " ); 
        if( y.null() ){ b=0; break; }
            headers[ x.slice( 0, y[0] ).to_capital_case() ] = x.slice( y[1], -2 );
        } while(0); } while(b);

        http->read.borrow = type::move( get_borrow( ) ); 
        set_recv_mode( headers ); /*-----*/ coStay(0);

    coFinish }
    
    /*─······································································─*/

    promise_t<http_t,except_t> read_body( ulong timeout=60000UL ) const noexcept {

        auto self = type::bind( this ); set_recv_timeout( timeout );

    return promise_t<http_t,except_t> ([=](
        res_t<http_t> res, rej_t<except_t> rej
    ){

        auto task = self->onDrain.once([&self,res](){ res( *self ); });

        process::poll( *self, POLL_STATE::READ | POLL_STATE::EDGE, coroutine::add( COROUTINE(){
        coBegin

            if( self->http->mode[0].state & FLAG::HTTP_FLAG_STREAM ){

                while ( self->http->mode[0].size != 0 ){
                coWait( self->http->read( self.get(), 
                        self->get_buffer().data   (), 
                        self->get_buffer().size   (), self->http->mode[0]
                )==1 );
                    self->body += string_t( self->get_buffer().data(), self->http->read.data );
                }

            } else { 
                
                while ( self->is_available() ){
                coWait( self->http->read( self.get(), 
                        self->get_buffer().data   (), 
                        self->get_buffer().size   (), self->http->mode[0]
                )==1 );
                    self->body += string_t( self->get_buffer().data(), self->http->read.data );
                }

            }
            
            self->onDrain.off(task); res( *self ); 

        coFinish
        }), 0UL );

    }); }
    
    /*─······································································─*/

    void write_header( const string_t& method, const string_t& path, const string_t& version, const header_t& headers ) const noexcept { 
        
        queue_t<string_t> out; set_send_mode( nullptr );

        out.push( string::format( "%s %s %s" , method.get(), path.get(), version.get() ) );

        auto x = headers.raw().first(); while( x!=nullptr ){ 
        auto y = x->next; auto &z = x->data;
               out.push( string::format( "%s: %s", z.first.to_capital_case().get(), z.second.get() ) );
        x=y; } out.push( "\r\n" ); write( array_t<string_t>( out.data() ).join("\r\n") );
        
        if( method=="HEAD" ){ close(); return; } set_send_mode( headers );

    }

    /*─······································································─*/

    void write_header( uint status, const header_t& headers ) const noexcept { 
        
        queue_t<string_t> out; set_send_mode( nullptr );

        out.push( string::format( "%s %u %s", version.get(), status, HTTP_NODEPP::_get_http_status(status).get() ) );

        auto x = headers.raw().first(); while( x!=nullptr ){ 
        auto y = x->next; auto &z = x->data;
               out.push( string::format( "%s: %s", z.first.to_capital_case().get(), z.second.get() ) );
        x=y; } out.push( "\r\n" ); write( array_t<string_t>( out.data() ).join("\r\n") );
        
        if( method=="HEAD" ){ close(); return; } set_send_mode( headers );

    }
    
    /*─······································································─*/

    template< class T > void write_header( const T& fetch, const string_t& path ) const noexcept {
        
        queue_t<string_t> out; set_send_mode( nullptr );

        out.push( string::format( "%s %s %s", fetch.method.get(), path.get(), fetch.version.get() ) );
        if( !fetch.body.empty() ){ 
             fetch.headers["Content-Length"] = string::to_string( fetch.body.size() );
        }

        auto x = fetch.headers.raw().first(); while( x!=nullptr ){ 
        auto y = x->next; auto &z = x->data;
               out.push( string::format( "%s: %s", z.first.to_capital_case().get(), z.second.get() ) );
        x=y; } out.push( "\r\n" + fetch.body ); 

        write( array_t<string_t>( out.data() ).join("\r\n") );
        if( fetch.method == "HEAD" ){ close(); return; } set_send_mode( fetch.headers );

    }
    
    /*─······································································─*/

    virtual int _write( char* bf, const ulong& sx ) const noexcept override {
        if( is_closed() ){ return -1; } if( sx==0 ){ return  0; } auto &md = http->mode[1];
        while( http->write( this, bf, sx, md )==1 ){ return -2; }
        return http->write.data==0 ? -1 : http->write.data;
    }

    virtual int _read ( char* bf, const ulong& sx ) const noexcept override {
        if( is_closed() ){ return -1; } if( sx==0 ){ return  0; } auto &md = http->mode[0];
        while( http->read( this, bf, sx, md ) ==1 ){ return -2; }
        return http->read.data==0 ? -1 : http->read.data;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace http {

    inline tcp_t server( function_t<void,http_t> cb, agent_t* opt=nullptr ){
    return tcp_t([=]( http_t cli ){ int c =0; 
        
        while((c=cli.read_header())==1){ 
        if   ( cli.is_waiting()){ process::next(); }}
        if( c!=0 ){ cli.close(); return; } 
        
    cb(cli); }, opt ); }

    /*─······································································─*/

    inline promise_t<http_t,except_t> fetch ( const fetch_t& fetch, agent_t* opt=nullptr ) {
    auto   agent = type::bind( opt==nullptr ? agent_t() : *opt );
    return promise_t<http_t,except_t>([=]( res_t<http_t> res, rej_t<except_t> rej ){

        if( !url::is_valid( fetch.url ) ){ rej(except_t("invalid URL")); return; }
             url_t uri = url::parse( fetch.url );

        if( !fetch.query.empty() ){ uri.search=query::format(fetch.query); }
        string_t dip = uri.hostname ; fetch.headers["Connection"] = "close";
        /*-------------------------*/ fetch.headers["Host"] = dip;
        string_t dir = uri.pathname + uri.search + uri.hash;

        auto skt = tcp_t([=]( http_t cli ){

            cli.set_timeout ( fetch.timeout ); 
            cli.write_header( fetch, dir  );

        stream::readable( cli, 0UL ).then([=]( http_t cli ){ int c=0;
                
            while((c=cli.read_header())==1){ 
            if   ( cli.is_waiting()){ process::next(); }}

            if( c==0 ){ res(cli); return; } cli.close();

            rej(except_t("Could not connect to server"));

        }).fail([=]( except_t /*unused*/ ){
            rej(except_t("Could not connect to server"));
        }); }, &agent );

        skt.onError([=]( except_t error ){ rej(error); });
        skt.connect( uri.rawname, uri.port );

    }); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/