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

#include "promise.h"
#include "encoder.h"
#include "query.h"
#include "url.h"
#include "tcp.h"
#include "map.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using header_t = map_t< string_t, string_t >; namespace HTTP_NODEPP {

    string_t _get_http_status( uint status ){ switch( status ){
        case 100:  return "Continue";                                           break;
        case 101:  return "Switching Protocols";                                break;
        case 102:  return "Processing";                                         break;
        case 103:  return "Early Hints";                                        break;
        
        case 200:  return "OK";                                                 break;
        case 201:  return "Created";                                            break;
        case 202:  return "Accepted";                                           break;
        case 203:  return "Non-Authoritative Information";                      break;
        case 204:  return "No Content";                                         break;
        case 205:  return "Reset Content";                                      break;
        case 206:  return "Partial Content";                                    break;
        case 207:  return "Multi-Status";                                       break;
        case 208:  return "Already Reported";                                   break;
        case 226:  return "IM Used";                                            break;

        case 300:  return "Multiple Choices";                                   break;
        case 301:  return "Moved Permanently";                                  break;
        case 302:  return "Found";                                              break;
        case 303:  return "See Other";                                          break;
        case 304:  return "Not Modified";                                       break;
        case 305:  return "Use Proxy";                                          break;
        case 307:  return "Temporary Redirect";                                 break;
        case 308:  return "Permanent Redirect";                                 break;

        case 400:  return "Bad Request";                                        break;
        case 401:  return "Unauthorized";                                       break;
        case 402:  return "Payment Required";                                   break;
        case 403:  return "Forbidden";                                          break;
        case 404:  return "Not Found";                                          break;
        case 405:  return "Method Not Allowed";                                 break;
        case 406:  return "Not Acceptable";                                     break;
        case 407:  return "Proxy Authentication Required";                      break;
        case 408:  return "Request Timeout";                                    break;
        case 409:  return "Conflict";                                           break;
        case 410:  return "Gone";                                               break;
        case 411:  return "Length Required";                                    break;
        case 412:  return "Precondition Failed";                                break;
        case 413:  return "Payload Too Large";                                  break;
        case 414:  return "URI Too Long";                                       break;
        case 415:  return "Unsupported Media Type";                             break;
        case 416:  return "Range Not Satisfiable";                              break;
        case 417:  return "Expectation Failed";                                 break;
        case 418:  return "I'm a Teapot";                                       break;
        case 421:  return "Misdirected Request";                                break;
        case 422:  return "Unprocessable Entity";                               break;
        case 423:  return "Locked";                                             break;
        case 424:  return "Failed Dependency";                                  break;
        case 425:  return "Too Early";                                          break;
        case 426:  return "Upgrade Required";                                   break;
        case 428:  return "Precondition Required";                              break;
        case 429:  return "Too Many Requests";                                  break;
        case 431:  return "Request Header Fields Too Large";                    break;
        case 451:  return "Unavailable For Legal Reasons";                      break;

        case 500:  return "Internal Server Error";                              break;
        case 501:  return "Not Implemented";                                    break;
        case 502:  return "Bad Gateway";                                        break;
        case 503:  return "Service Unavailable";                                break;
        case 504:  return "Gateway Timeout";                                    break;
        case 505:  return "HTTP Version Not Supported";                         break;
        case 506:  return "Variant Also Negotiates";                            break;
        case 507:  return "Insufficient Storage";                               break;
        case 508:  return "Loop Detected";                                      break;
        case 509:  return "Bandwidth Limit Exceeded";                           break;
        case 510:  return "Not Extended";                                       break;
        case 511:  return "Network Authentication Required";                    break;
        default: throw except_t(string::format("Status %d Not Found", status)); break;
    } return nullptr; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { struct fetch_t {

    file_t    file ;
    string_t  body ;
    query_t   query;
    
    /*─······································································─*/

    header_t  headers;
    ulong     timeout = 0;
    
    /*─······································································─*/

    string_t     url ;
    string_t  method = "GET";
    string_t version = "HTTP/1.0";
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class http_t : public socket_t, public generator_t {
protected:

    generator::file::line line; string_t raw;
    
public:

    uint      status=200;
    string_t  version;
    header_t  headers;
    query_t   query;

    string_t  protocol;
    string_t  search;
    string_t  method;
    string_t  path;
    string_t  hash;
    string_t  url;
    
    /*─······································································─*/

    template< class... T > 
    http_t( const T&... args ) noexcept : socket_t( args... ) {}

    virtual ~http_t() noexcept { /*-------------------------*/ }

    /*─······································································─*/

    void     set_version( const string_t& msg ) noexcept { version = msg; }

    string_t get_version() const noexcept { return version; }

    /*─······································································─*/

    int read_header() noexcept {

        static regex_t reg0=regex_t( "^([^ ]+) ([^ ]+) ([^\r]+)" );
        static regex_t reg1=regex_t( "^\\d+"   );
        static regex_t reg2=regex_t( "^[^?#]+" );
        static regex_t reg3=regex_t( "?[^#]+"  );
        static regex_t reg4=regex_t( "#\\w+"   );

    bool b=1; coBegin
    
        if( !is_available() ){ coEnd; } coWait( line( this )==1 ); 
        if( line.state <= 0 ){ coEnd; } raw = line.data;

        do{ coWait( line( this )==1 ); if( line.state<=0 ){ coEnd; } do {
            auto x = line.data; auto y = x.find( ": " ); 
        if( y.null() ){ b=0; break; }
            headers[ x.slice( 0, y[0] ).to_capital_case() ] = x.slice( y[1], -2 );
        } while(0); } while(b); 

        do{ reg0.search_all(raw); auto base=reg0.get_memory(); 
            reg0.clear_memory( ); protocol = "HTTP";
        if( base.size() != 3 ){ break; } /*-------------*/

        if( !reg1.test( base[1] ) ){
            string_t host = headers.has("Host")? headers["Host"] : "localhost";
            url    = string::format("http://%s%s", host.get(), base[1].get() );
            path   = reg2.match( base[1] );
            search = reg3.match( base[1] );
            hash   = reg4.match( base[1] );
            query  = query::parse( search );
            version= base[2]; method=base[0]; 

        } else { version = base[0]; status = string::to_uint( base[1] ); }
        } while(0);
        
        coStay(0);

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

namespace nodepp { namespace http {

    template< class T > tcp_t server( T cb, agent_t* opt=nullptr ){
        return tcp_t([=]( http_t cli ){

            int c=0; while((c=cli.read_header())==1)
            /*------*/{ process::next(); }
            if( c==0 ){ cb(cli); return; }
            
        cli.close(); }, opt );
    }

    /*─······································································─*/

    promise_t<http_t,except_t> fetch ( const fetch_t& args, agent_t* opt=nullptr ) { 
           auto agent = type::bind( opt  ); 
           auto fetch = type::bind( args ); 
    return promise_t<http_t,except_t>([=]( res_t<http_t> res, rej_t<except_t> rej ){

        if( !url::is_valid( fetch->url ) ){ rej(except_t("invalid URL")); return; }
             url_t uri = url::parse( fetch->url );

        if( !fetch->query.empty() ){ uri.search=query::format(fetch->query); }
        string_t dip = uri.hostname ; fetch->headers["Connection"] = "close";
        /*-------------------------*/ fetch->headers["Host"] = dip;
        string_t dir = uri.pathname + uri.search + uri.hash;
       
        auto skt = tcp_t([=]( http_t cli ){

            cli.set_timeout( fetch->timeout ); cli.write_header( fetch, dir );
            int c=0; while((c=cli.read_header())==1){ process::next(); }

            if( c==0 ){ res( cli ); return; } cli.close();
            rej(except_t("Could not connect to server"));

        }, &agent );

        skt.onError([=]( except_t error ){ rej(error); });
        skt.connect( dip, uri.port );

    }); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif